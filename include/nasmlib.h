/* ----------------------------------------------------------------------- *
 *
 *   Copyright 1996-2017 The NASM Authors - All Rights Reserved
 *   See the file AUTHORS included with the NASM distribution for
 *   the specific copyright holders.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following
 *   conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----------------------------------------------------------------------- */

/*
 * nasmlib.h    header file for nasmlib.c
 */

#ifndef NASM_NASMLIB_H
#define NASM_NASMLIB_H

#include "compiler.h"

#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

/*
 * tolower table -- avoids a function call on some platforms.
 * NOTE: unlike the tolower() function in ctype, EOF is *NOT*
 * a permitted value, for obvious reasons.
 */
void tolower_init(void);
extern unsigned char nasm_tolower_tab[256];
#define nasm_tolower(x) nasm_tolower_tab[(unsigned char)(x)]

/* Wrappers around <ctype.h> functions */
/* These are only valid for values that cannot include EOF */
#define nasm_isspace(x)  isspace((unsigned char)(x))
#define nasm_isalpha(x)  isalpha((unsigned char)(x))
#define nasm_isdigit(x)  isdigit((unsigned char)(x))
#define nasm_isalnum(x)  isalnum((unsigned char)(x))
#define nasm_isxdigit(x) isxdigit((unsigned char)(x))

/*
 * -------------------------
 * Error reporting functions
 * -------------------------
 */

/*
 * An error reporting function should look like this.
 */
void printf_func(2, 3) nasm_error(int severity, const char *fmt, ...);
no_return printf_func(2, 3) nasm_fatal(int flags, const char *fmt, ...);
no_return printf_func(2, 3) nasm_panic(int flags, const char *fmt, ...);
no_return nasm_panic_from_macro(const char *file, int line);
#define panic() nasm_panic_from_macro(__FILE__, __LINE__);

typedef void (*vefunc) (int severity, const char *fmt, va_list ap);
extern vefunc nasm_verror;
static inline vefunc nasm_set_verror(vefunc ve)
{
    vefunc old_verror = nasm_verror;
    nasm_verror = ve;
    return old_verror;
}

/*
 * These are the error severity codes which get passed as the first
 * argument to an efunc.
 */

#define ERR_DEBUG       0x00000000      /* put out debugging message */
#define ERR_WARNING     0x00000001      /* warn only: no further action */
#define ERR_NONFATAL    0x00000002      /* terminate assembly after phase */
#define ERR_FATAL       0x00000006      /* instantly fatal: exit with error */
#define ERR_PANIC       0x00000007      /* internal error: panic instantly
                                         * and dump core for reference */
#define ERR_MASK        0x00000007      /* mask off the above codes */
#define ERR_NOFILE      0x00000010      /* don't give source file name/line */
#define ERR_USAGE       0x00000020      /* print a usage message */
#define ERR_PASS1       0x00000040      /* only print this error on pass one */
#define ERR_PASS2       0x00000080

#define ERR_NO_SEVERITY 0x00000100      /* suppress printing severity */
#define ERR_PP_PRECOND	0x00000200	/* for preprocessor use */
#define ERR_PP_LISTMACRO 0x00000400	/* from preproc->error_list_macros() */

/*
 * These codes define specific types of suppressible warning.
 */

#define ERR_WARN_MASK   0xFFFFF000      /* the mask for this feature */
#define ERR_WARN_SHR    12              /* how far to shift right */

#define WARN(x)         ((x) << ERR_WARN_SHR)
#define WARN_IDX(x)     (((x) & ERR_WARN_MASK) >> ERR_WARN_SHR)

#define ERR_WARN_TERM           WARN( 0) /* treat warnings as errors */
#define ERR_WARN_MNP            WARN( 1) /* macro-num-parameters warning */
#define ERR_WARN_MSR            WARN( 2) /* macro self-reference */
#define ERR_WARN_MDP            WARN( 3) /* macro default parameters check */
#define ERR_WARN_OL             WARN( 4) /* orphan label (no colon, and
                                          * alone on line) */
#define ERR_WARN_NOV            WARN( 5) /* numeric overflow */
#define ERR_WARN_GNUELF         WARN( 6) /* using GNU ELF extensions */
#define ERR_WARN_FL_OVERFLOW    WARN( 7) /* FP overflow */
#define ERR_WARN_FL_DENORM      WARN( 8) /* FP denormal */
#define ERR_WARN_FL_UNDERFLOW   WARN( 9) /* FP underflow */
#define ERR_WARN_FL_TOOLONG     WARN(10) /* FP too many digits */
#define ERR_WARN_USER           WARN(11) /* %warning directives */
#define ERR_WARN_LOCK		WARN(12) /* bad LOCK prefixes */
#define ERR_WARN_HLE		WARN(13) /* bad HLE prefixes */
#define ERR_WARN_BND		WARN(14) /* bad BND prefixes */
#define ERR_WARN_ZEXTRELOC	WARN(15) /* relocation zero-extended */
#define ERR_WARN_PTR		WARN(16) /* not a NASM keyword */
#define ERR_WARN_MAX            16       /* the highest numbered one */

/*
 * Wrappers around malloc, realloc and free. nasm_malloc will
 * fatal-error and die rather than return NULL; nasm_realloc will
 * do likewise, and will also guarantee to work right on being
 * passed a NULL pointer; nasm_free will do nothing if it is passed
 * a NULL pointer.
 */
void * safe_malloc(1) nasm_malloc(size_t);
void * safe_malloc(1) nasm_zalloc(size_t);
void * safe_malloc2(1,2) nasm_calloc(size_t, size_t);
void * safe_realloc(2) nasm_realloc(void *, size_t);
void nasm_free(void *);
char * safe_alloc nasm_strdup(const char *);
char * safe_alloc nasm_strndup(const char *, size_t);

#define nasm_new(p) ((p) = nasm_zalloc(sizeof(*(p))))
#define nasm_newn(p,n) ((p) = nasm_calloc(sizeof(*(p)),(n)))
#define nasm_delete(p) do { nasm_free(p); (p) = NULL; } while (0)

/*
 * Wrapper around fwrite() which fatal-errors on output failure.
 */
void nasm_write(const void *, size_t, FILE *);

/*
 * NASM assert failure
 */
no_return nasm_assert_failed(const char *, int, const char *);
#define nasm_assert(x)                                          \
    do {                                                        \
        if (unlikely(!(x)))                                     \
            nasm_assert_failed(__FILE__,__LINE__,#x);           \
    } while (0)

/*
 * NASM failure at build time if x != 0
 */
#ifdef static_assert
# define nasm_build_assert(x) static_assert(x, #x)
#elif defined(HAVE_FUNC_ATTRIBUTE_ERROR) && defined(__OPTIMIZE__)
# define nasm_build_assert(x)                                           \
    if (!(x)) {                                                         \
        extern void __attribute__((error("assertion " #x " failed")))   \
            _nasm_static_fail(void);					\
        _nasm_static_fail();                                            \
    }
#else
# define nasm_build_assert(x) (void)(sizeof(char[1-2*!(x)]))
#endif

/*
 * ANSI doesn't guarantee the presence of `stricmp' or
 * `strcasecmp'.
 */
#if defined(HAVE_STRCASECMP)
#define nasm_stricmp strcasecmp
#elif defined(HAVE_STRICMP)
#define nasm_stricmp stricmp
#else
int pure_func nasm_stricmp(const char *, const char *);
#endif

#if defined(HAVE_STRNCASECMP)
#define nasm_strnicmp strncasecmp
#elif defined(HAVE_STRNICMP)
#define nasm_strnicmp strnicmp
#else
int pure_func nasm_strnicmp(const char *, const char *, size_t);
#endif

int pure_func nasm_memicmp(const char *, const char *, size_t);

#if defined(HAVE_STRSEP)
#define nasm_strsep strsep
#else
char *nasm_strsep(char **stringp, const char *delim);
#endif

#ifndef HAVE_DECL_STRNLEN
size_t pure_func strnlen(const char *, size_t);
#endif

/* This returns the numeric value of a given 'digit'. */
#define numvalue(c)         ((c) >= 'a' ? (c) - 'a' + 10 : (c) >= 'A' ? (c) - 'A' + 10 : (c) - '0')

/*
 * Convert a string into a number, using NASM number rules. Sets
 * `*error' to true if an error occurs, and false otherwise.
 */
int64_t readnum(char *str, bool *error);

/*
 * Convert a character constant into a number. Sets
 * `*warn' to true if an overflow occurs, and false otherwise.
 * str points to and length covers the middle of the string,
 * without the quotes.
 */
int64_t readstrnum(char *str, int length, bool *warn);

/*
 * seg_init: Initialise the segment-number allocator.
 * seg_alloc: allocate a hitherto unused segment number.
 */
void pure_func seg_init(void);
int32_t pure_func seg_alloc(void);

/*
 * many output formats will be able to make use of this: a standard
 * function to add an extension to the name of the input file
 */
void standard_extension(char *inname, char *outname, char *extension);

/*
 * Utility macros...
 *
 * This is a useful #define which I keep meaning to use more often:
 * the number of elements of a statically defined array.
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*
 * List handling
 *
 *  list_for_each - regular iterator over list
 *  list_for_each_safe - the same but safe against list items removal
 *  list_last - find the last element in a list
 */
#define list_for_each(pos, head)                        \
    for (pos = head; pos; pos = pos->next)
#define list_for_each_safe(pos, n, head)                \
    for (pos = head, n = (pos ? pos->next : NULL); pos; \
        pos = n, n = (n ? n->next : NULL))
#define list_last(pos, head)                            \
    for (pos = head; pos && pos->next; pos = pos->next) \
        ;
#define list_reverse(head, prev, next)                  \
    do {                                                \
        if (!head || !head->next)                       \
            break;                                      \
        prev = NULL;                                    \
        while (head) {                                  \
            next = head->next;                          \
            head->next = prev;                          \
            prev = head;                                \
            head = next;                                \
        }                                               \
        head = prev;                                    \
    } while (0)

/*
 * Power of 2 align helpers
 */
#undef ALIGN_MASK		/* Some BSD flavors define these in system headers */
#undef ALIGN
#define ALIGN_MASK(v, mask)     (((v) + (mask)) & ~(mask))
#define ALIGN(v, a)             ALIGN_MASK(v, (a) - 1)
#define IS_ALIGNED(v, a)        (((v) & ((a) - 1)) == 0)

/*
 * some handy macros that will probably be of use in more than one
 * output format: convert integers into little-endian byte packed
 * format in memory
 */

#if X86_MEMORY

#define WRITECHAR(p,v)                          \
    do {                                        \
        *(uint8_t *)(p) = (v);                  \
        (p) += 1;                               \
    } while (0)

#define WRITESHORT(p,v)                         \
    do {                                        \
        *(uint16_t *)(p) = (v);                 \
        (p) += 2;                               \
    } while (0)

#define WRITELONG(p,v)                          \
    do {                                        \
        *(uint32_t *)(p) = (v);                 \
        (p) += 4;                               \
    } while (0)

#define WRITEDLONG(p,v)                         \
    do {                                        \
        *(uint64_t *)(p) = (v);                 \
        (p) += 8;                               \
    } while (0)

#define WRITEADDR(p,v,s)                        \
    do {                                        \
        uint64_t _wa_v = (v);                   \
        memcpy((p), &_wa_v, (s));               \
        (p) += (s);                             \
    } while (0)

#else /* !X86_MEMORY */

#define WRITECHAR(p,v)                          \
    do {                                        \
        uint8_t *_wc_p = (uint8_t *)(p);        \
        uint8_t _wc_v = (v);                    \
        _wc_p[0] = _wc_v;                       \
        (p) = (void *)(_wc_p + 1);              \
    } while (0)

#define WRITESHORT(p,v)                         \
    do {                                        \
        uint8_t *_ws_p = (uint8_t *)(p);        \
        uint16_t _ws_v = (v);                   \
        _ws_p[0] = _ws_v;                       \
        _ws_p[1] = _ws_v >> 8;                  \
        (p) = (void *)(_ws_p + 2);              \
    } while (0)

#define WRITELONG(p,v)                          \
    do {                                        \
        uint8_t *_wl_p = (uint8_t *)(p);        \
        uint32_t _wl_v = (v);                   \
        _wl_p[0] = _wl_v;                       \
        _wl_p[1] = _wl_v >> 8;                  \
        _wl_p[2] = _wl_v >> 16;                 \
        _wl_p[3] = _wl_v >> 24;                 \
        (p) = (void *)(_wl_p + 4);              \
    } while (0)

#define WRITEDLONG(p,v)                         \
    do {                                        \
        uint8_t *_wq_p = (uint8_t *)(p);        \
        uint64_t _wq_v = (v);                   \
        _wq_p[0] = _wq_v;                       \
        _wq_p[1] = _wq_v >> 8;                  \
        _wq_p[2] = _wq_v >> 16;                 \
        _wq_p[3] = _wq_v >> 24;                 \
        _wq_p[4] = _wq_v >> 32;                 \
        _wq_p[5] = _wq_v >> 40;                 \
        _wq_p[6] = _wq_v >> 48;                 \
        _wq_p[7] = _wq_v >> 56;                 \
        (p) = (void *)(_wq_p + 8);              \
    } while (0)

#define WRITEADDR(p,v,s)                        \
    do {                                        \
        int _wa_s = (s);                        \
        uint64_t _wa_v = (v);                   \
        while (_wa_s--) {                       \
            WRITECHAR(p,_wa_v);                 \
            _wa_v >>= 8;                        \
        }                                       \
    } while(0)

#endif

/*
 * and routines to do the same thing to a file
 */
#define fwriteint8_t(d,f) putc(d,f)
void fwriteint16_t(uint16_t data, FILE * fp);
void fwriteint32_t(uint32_t data, FILE * fp);
void fwriteint64_t(uint64_t data, FILE * fp);
void fwriteaddr(uint64_t data, int size, FILE * fp);

/*
 * Binary search routine. Returns index into `array' of an entry
 * matching `string', or <0 if no match. `array' is taken to
 * contain `size' elements.
 *
 * bsi() is case sensitive, bsii() is case insensitive.
 */
int bsi(const char *string, const char **array, int size);
int bsii(const char *string, const char **array, int size);

/*
 * These functions are used to keep track of the source code file and name.
 */
void src_init(void);
void src_free(void);
const char *src_set_fname(const char *newname);
const char *src_get_fname(void);
int32_t src_set_linnum(int32_t newline);
int32_t src_get_linnum(void);
/* Can be used when there is no need for the old information */
void src_set(int32_t line, const char *filename);
/*
 * src_get gets both the source file name and line.
 * It is also used if you maintain private status about the source location
 * It return 0 if the information was the same as the last time you
 * checked, -2 if the name changed and (new-old) if just the line changed.
 */
int32_t src_get(int32_t *xline, const char **xname);

char *nasm_strcat(const char *one, const char *two);

char *nasm_skip_spaces(const char *p);
char *nasm_skip_word(const char *p);
char *nasm_zap_spaces_fwd(char *p);
char *nasm_zap_spaces_rev(char *p);
char *nasm_trim_spaces(char *p);
char *nasm_get_word(char *p, char **tail);
char *nasm_opt_val(char *p, char **opt, char **val);

/*
 * Converts a relative pathname rel_path into an absolute path name.
 *
 * The buffer returned must be freed by the caller
 */
char *nasm_realpath(const char *rel_path);

const char * pure_func prefix_name(int);

/*
 * Wrappers around fopen()... for future change to a dedicated structure
 */
enum file_flags {
    NF_BINARY	= 0x00000000,   /* Binary file (default) */
    NF_TEXT	= 0x00000001,   /* Text file */
    NF_NONFATAL = 0x00000000,   /* Don't die on open failure (default) */
    NF_FATAL    = 0x00000002,   /* Die on open failure */
    NF_FORMAP   = 0x00000004    /* Intended to use nasm_map_file() */
};

FILE *nasm_open_read(const char *filename, enum file_flags flags);
FILE *nasm_open_write(const char *filename, enum file_flags flags);

/* Probe for existence of a file */
bool nasm_file_exists(const char *filename);

#define ZERO_BUF_SIZE 65536     /* Default value */
#if defined(BUFSIZ) && (BUFSIZ > ZERO_BUF_SIZE)
# undef ZERO_BUF_SIZE
# define ZERO_BUF_SIZE BUFSIZ
#endif
extern const uint8_t zero_buffer[ZERO_BUF_SIZE];

/* Missing fseeko/ftello */
#ifndef HAVE_FSEEKO
# undef off_t                   /* Just in case it is a macro */
# ifdef HAVE__FSEEKI64
#  define fseeko _fseeki64
#  define ftello _ftelli64
#  define off_t  int64_t
# else
#  define fseeko fseek
#  define ftello ftell
#  define off_t  long
# endif
#endif

const void *nasm_map_file(FILE *fp, off_t start, off_t len);
void nasm_unmap_file(const void *p, size_t len);
off_t nasm_file_size(FILE *f);
off_t nasm_file_size_by_path(const char *pathname);
void fwritezero(off_t bytes, FILE *fp);

static inline bool const_func overflow_general(int64_t value, int bytes)
{
    int sbit;
    int64_t vmax, vmin;

    if (bytes >= 8)
        return false;

    sbit = (bytes << 3) - 1;
    vmax =  ((int64_t)2 << sbit) - 1;
    vmin = -((int64_t)1 << sbit);

    return value < vmin || value > vmax;
}

static inline bool const_func overflow_signed(int64_t value, int bytes)
{
    int sbit;
    int64_t vmax, vmin;

    if (bytes >= 8)
        return false;

    sbit = (bytes << 3) - 1;
    vmax =  ((int64_t)1 << sbit) - 1;
    vmin = -((int64_t)1 << sbit);

    return value < vmin || value > vmax;
}

static inline bool const_func overflow_unsigned(int64_t value, int bytes)
{
    int sbit;
    int64_t vmax, vmin;

    if (bytes >= 8)
        return false;

    sbit = (bytes << 3) - 1;
    vmax = ((int64_t)2 << sbit) - 1;
    vmin = 0;

    return value < vmin || value > vmax;
}

static inline int64_t const_func signed_bits(int64_t value, int bits)
{
    if (bits < 64) {
        value &= ((int64_t)1 << bits) - 1;
        if (value & (int64_t)1 << (bits - 1))
            value |= (int64_t)((uint64_t)-1 << bits);
    }
    return value;
}

int const_func idata_bytes(int opcode);

/* check if value is power of 2 */
#define is_power2(v)   ((v) && ((v) & ((v) - 1)) == 0)

/*
 * floor(log2(v))
 */
int const_func ilog2_32(uint32_t v);
int const_func ilog2_64(uint64_t v);

/*
 * v == 0 ? 0 : is_power2(x) ? ilog2_X(v) : -1
 */
int const_func alignlog2_32(uint32_t v);
int const_func alignlog2_64(uint64_t v);

#endif

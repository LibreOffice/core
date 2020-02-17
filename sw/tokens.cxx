/* C++ code produced by gperf version 3.1 */
/* Command-line: /usr/bin/gperf --compare-strncmp --readonly-tables --output-file=/home/jpakkane/bob/libreoffice/workdir/CustomTarget/sw/generated/tokens.cxx --class-name=TextBlockTokens --word-array-name=textBlockList --enum /home/jpakkane/bob/libreoffice/workdir/CustomTarget/sw/generated/TextBlockTokens.gperf  */
/* Computed positions: -k'1' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35)          \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40)      \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44)       \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48)       \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52)       \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56)       \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60)       \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65)       \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69)       \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73)       \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77)       \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81)       \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85)       \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89)       \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93)      \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98)       \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102)    \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106)   \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110)   \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114)   \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118)   \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122)   \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

struct xmltoken
{
    const sal_Char *name; XMLTokenEnum nToken;
};
enum
    {
     TOTAL_KEYWORDS = 5,
     MIN_WORD_LENGTH = 1,
     MAX_WORD_LENGTH = 16,
     MIN_HASH_VALUE = 1,
     MAX_HASH_VALUE = 16
    };

/* maximum key range = 16, duplicates = 0 */

class TextBlockTokens
{
private:
    static inline unsigned int hash (const char *str, size_t len);
public:
    static const struct xmltoken *in_word_set (const char *str, size_t len);
};

inline unsigned int
TextBlockTokens::hash (const char *str, size_t len)
{
    static const unsigned char asso_values[] =
        {
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17,  5, 17,
         0, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17,  0, 17, 17, 17,  0, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
         17, 17, 17, 17, 17, 17
        };
    return len + asso_values[static_cast<unsigned char>(str[0])];
}

static const struct xmltoken textBlockList[] =
    {
     {(char*)0, XML_TOKEN_INVALID},
     {"p",XML_P},
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {"text",XML_TEXT},
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {"document",XML_DOCUMENT},
     {"body",XML_BODY},
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {"document-content",XML_DOCUMENT_CONTENT}
    };

const struct xmltoken *
TextBlockTokens::in_word_set (const char *str, size_t len)
{
    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
        {
            unsigned int key = hash (str, len);

            if (key <= MAX_HASH_VALUE)
                {
                    const char *s = textBlockList[key].name;

                    if (s && *str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                        return &textBlockList[key];
                }
        }
    return 0;
}

/* C++ code produced by gperf version 3.1 */
/* Command-line: /usr/bin/gperf --compare-strncmp --readonly-tables -T --class-name=BlockListTokens --word-array-name=blockListList /home/jpakkane/bob/libreoffice/workdir/CustomTarget/sw/generated/BlockListTokens.gperf  */
/* Computed positions: -k'1' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35)          \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40)      \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44)       \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48)       \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52)       \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56)       \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60)       \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65)       \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69)       \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73)       \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77)       \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81)       \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85)       \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89)       \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93)      \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98)       \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102)    \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106)   \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110)   \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114)   \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118)   \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122)   \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif


#define TOTAL_KEYWORDS 7
#define MIN_WORD_LENGTH 4
#define MAX_WORD_LENGTH 16
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 21
/* maximum key range = 18, duplicates = 0 */

class BlockListTokens
{
private:
    static inline unsigned int hash (const char *str, size_t len);
public:
    static const struct xmltoken *in_word_set (const char *str, size_t len);
};

inline unsigned int
BlockListTokens::hash (const char *str, size_t len)
{
    static const unsigned char asso_values[] =
        {
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22,  5,  0, 22,
         22, 22, 22, 22, 22, 22, 22, 22,  0, 22,
         0, 22,  0, 22, 22, 22, 22,  0, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22
        };
    return len + asso_values[static_cast<unsigned char>(str[0])];
}

static const struct xmltoken blockListList[] =
    {
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {"name",XML_NAME},
     {"block",XML_BLOCK},
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {"list-name",XML_LIST_NAME},
     {"block-list",XML_BLOCK_LIST},
     {(char*)0, XML_TOKEN_INVALID},
     {"package-name",XML_PACKAGE_NAME},
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {"unformatted-text",XML_UNFORMATTED_TEXT},
     {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID}, {(char*)0, XML_TOKEN_INVALID},
     {"abbreviated-name",XML_ABBREVIATED_NAME}
    };

const struct xmltoken *
BlockListTokens::in_word_set (const char *str, size_t len)
{
    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
        {
            unsigned int key = hash (str, len);

            if (key <= MAX_HASH_VALUE)
                {
                    const char *s = blockListList[key].name;

                    if (s && *str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                        return &blockListList[key];
                }
        }
    return 0;
}

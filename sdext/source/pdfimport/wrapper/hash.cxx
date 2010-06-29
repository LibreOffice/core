/* C++ code produced by gperf version 3.0.1 */
/* Command-line: gperf -C -t -l -L C++ -m 20 -Z PdfKeywordHash keyword_list  */
/* Computed positions: -k'4-5,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "keyword_list"
struct hash_entry { const char* name; enum parseKey eKey; };

#if defined __GNUC__
#pragma GCC system_header
#endif

#define TOTAL_KEYWORDS 46
#define MIN_WORD_LENGTH 7
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 8
#define MAX_HASH_VALUE 68
/* maximum key range = 61, duplicates = 0 */

class PdfKeywordHash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static const struct hash_entry *in_word_set (const char *str, unsigned int len);
};

inline unsigned int
PdfKeywordHash::hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69,  9, 69, 38, 69,
       1, 69, 69, 69, 31, 69, 16, 35, 21, 69,
      15,  0, 69, 21, 15, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69,  0, 15, 69,
      69,  5,  8, 69,  4,  8,  0, 20,  2, 27,
      11, 31,  0, 69,  0,  9,  0,  5, 69,  5,
      69, 13, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
      69, 69, 69, 69, 69, 69, 69
    };
  return len + asso_values[(unsigned char)str[4]+1] + asso_values[(unsigned char)str[3]] + asso_values[(unsigned char)str[len - 1]];
}

const struct hash_entry *
PdfKeywordHash::in_word_set (register const char *str, register unsigned int len)
{
  static const unsigned char lengthtable[] =
    {
       0,  0,  0,  0,  0,  0,  0,  0,  7,  0,  0,  0,  8, 12,
       8, 10, 10, 12, 13,  9, 15, 16, 17, 14, 15, 15, 10, 13,
      14, 15, 14, 11, 12,  9,  9, 17, 13, 19, 11,  8, 14,  9,
       7,  9, 19, 17, 15, 10, 13, 10,  9,  8, 17, 12,  8,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10,  8
    };
  static const struct hash_entry wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 29 "keyword_list"
      {"setFont",SETFONT},
      {""}, {""}, {""},
#line 8 "keyword_list"
      {"clipPath",CLIPPATH},
#line 28 "keyword_list"
      {"setFillColor",SETFILLCOLOR},
#line 19 "keyword_list"
      {"fillPath",FILLPATH},
#line 46 "keyword_list"
      {"updateFont",UPDATEFONT},
#line 17 "keyword_list"
      {"eoClipPath",EOCLIPPATH},
#line 25 "keyword_list"
      {"restoreState",RESTORESTATE},
#line 47 "keyword_list"
      {"updateLineCap",UPDATELINECAP},
#line 39 "keyword_list"
      {"startPage",STARTPAGE},
#line 43 "keyword_list"
      {"updateFillColor",UPDATEFILLCOLOR},
#line 51 "keyword_list"
      {"updateMiterLimit",UPDATEMITERLIMIT},
#line 52 "keyword_list"
      {"updateStrokeColor",UPDATESTROKECOLOR},
#line 48 "keyword_list"
      {"updateLineDash",UPDATELINEDASH},
#line 50 "keyword_list"
      {"updateLineWidth",UPDATELINEWIDTH},
#line 41 "keyword_list"
      {"updateBlendMode",UPDATEBLENDMODE},
#line 30 "keyword_list"
      {"setLineCap",SETLINECAP},
#line 21 "keyword_list"
      {"intersectClip",INTERSECTCLIP},
#line 45 "keyword_list"
      {"updateFlatness",UPDATEFLATNESS},
#line 22 "keyword_list"
      {"intersectEoClip",INTERSECTEOCLIP},
#line 49 "keyword_list"
      {"updateLineJoin",UPDATELINEJOIN},
#line 31 "keyword_list"
      {"setLineDash",SETLINEDASH},
#line 33 "keyword_list"
      {"setLineWidth",SETLINEWIDTH},
#line 24 "keyword_list"
      {"pushState",PUSHSTATE},
#line 26 "keyword_list"
      {"saveState",SAVESTATE},
#line 44 "keyword_list"
      {"updateFillOpacity",UPDATEFILLOPACITY},
#line 16 "keyword_list"
      {"endTextObject",ENDTEXTOBJECT},
#line 53 "keyword_list"
      {"updateStrokeOpacity",UPDATESTROKEOPACITY},
#line 32 "keyword_list"
      {"setLineJoin",SETLINEJOIN},
#line 23 "keyword_list"
      {"popState",POPSTATE},
#line 36 "keyword_list"
      {"setStrokeColor",SETSTROKECOLOR},
#line 42 "keyword_list"
      {"updateCtm",UPDATECTM},
#line 15 "keyword_list"
      {"endPage",ENDPAGE},
#line 20 "keyword_list"
      {"hyperLink",HYPERLINK},
#line 14 "keyword_list"
      {"drawSoftMaskedImage",DRAWSOFTMASKEDIMAGE},
#line 37 "keyword_list"
      {"setTextRenderMode",SETTEXTRENDERMODE},
#line 13 "keyword_list"
      {"drawMaskedImage",DRAWMASKEDIMAGE},
#line 40 "keyword_list"
      {"strokePath",STROKEPATH},
#line 34 "keyword_list"
      {"setMiterLimit",SETMITERLIMIT},
#line 18 "keyword_list"
      {"eoFillPath",EOFILLPATH},
#line 10 "keyword_list"
      {"drawImage",DRAWIMAGE},
#line 9 "keyword_list"
      {"drawChar",DRAWCHAR},
#line 38 "keyword_list"
      {"setTransformation",SETTRANSFORMATION},
#line 27 "keyword_list"
      {"setBlendMode",SETBLENDMODE},
#line 12 "keyword_list"
      {"drawMask",DRAWMASK},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 35 "keyword_list"
      {"setPageNum",SETPAGENUM},
#line 11 "keyword_list"
      {"drawLink",DRAWLINK  }
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        if (len == lengthtable[key])
          {
            register const char *s = wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
              return &wordlist[key];
          }
    }
  return 0;
}

/* C++ code produced by gperf version 3.0.1 */
/* Command-line: gperf -C -t -l -L C++ -m 20 -Z AfmKeywordHash afm_keyword_list  */
/* Computed positions: -k'1,4,6,$' */

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

#line 1 "afm_keyword_list"
struct hash_entry { const char* name; enum parseKey eKey; };

#define TOTAL_KEYWORDS 56
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 18
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 57
/* maximum key range = 57, duplicates = 0 */

class AfmKeywordHash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static const struct hash_entry *in_word_set (const char *str, unsigned int len);
};

inline unsigned int
AfmKeywordHash::hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 28,  1,  0,  9,  0,
      19, 58,  2, 10, 58,  0, 28,  0, 20, 58,
      44, 58, 58,  0, 16, 10, 24,  2,  3, 58,
      58, 58, 58, 58, 58, 58, 58,  6, 58,  0,
      19,  0, 58, 25, 14,  6, 58, 58, 17, 11,
       0, 17, 39, 58,  0,  0, 10, 58, 58, 58,
      13,  4, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
      58, 58, 58, 58, 58, 58
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

const struct hash_entry *
AfmKeywordHash::in_word_set (register const char *str, register unsigned int len)
{
  static const unsigned char lengthtable[] =
    {
       0,  1,  2,  1,  2,  1,  3,  2,  3,  5, 10, 11, 12,  2,
      14, 15, 16, 11,  9, 13, 14, 12, 12, 14, 13,  9,  7,  9,
       7,  9, 14,  5,  6, 14, 12, 16, 10, 14, 11, 10,  7,  1,
      12,  8, 17, 18,  2,  3,  7,  1,  8,  8, 13,  6,  6,  8,
       0,  1
    };
  static const struct hash_entry wordlist[] =
    {
      {"",NOPE},
#line 6 "afm_keyword_list"
      {"C",CODE},
#line 7 "afm_keyword_list"
      {"CC",COMPCHAR},
#line 5 "afm_keyword_list"
      {"B",CHARBBOX},
#line 8 "afm_keyword_list"
      {"CH",CODEHEX},
#line 54 "afm_keyword_list"
      {"W",XYWIDTH},
#line 33 "afm_keyword_list"
      {"KPX",KERNPAIRXAMT},
#line 56 "afm_keyword_list"
      {"WX",XWIDTH},
#line 55 "afm_keyword_list"
      {"W0X",X0WIDTH},
#line 47 "afm_keyword_list"
      {"StdHW",STDHW},
#line 12 "afm_keyword_list"
      {"Characters",CHARACTERS},
#line 36 "afm_keyword_list"
      {"MetricsSets",METRICSSETS},
#line 23 "afm_keyword_list"
      {"EndKernPairs",ENDKERNPAIRS},
#line 16 "afm_keyword_list"
      {"Em",EM},
#line 45 "afm_keyword_list"
      {"StartKernPairs",STARTKERNPAIRS},
#line 41 "afm_keyword_list"
      {"StartComposites",STARTCOMPOSITES},
#line 40 "afm_keyword_list"
      {"StartCharMetrics",STARTCHARMETRICS},
#line 22 "afm_keyword_list"
      {"EndKernData",ENDKERNDATA},
#line 14 "afm_keyword_list"
      {"Descender",DESCENDER},
#line 44 "afm_keyword_list"
      {"StartKernData",STARTKERNDATA},
#line 18 "afm_keyword_list"
      {"EndCharMetrics",ENDCHARMETRICS},
#line 20 "afm_keyword_list"
      {"EndDirection",ENDDIRECTION},
#line 11 "afm_keyword_list"
      {"CharacterSet",CHARACTERSET},
#line 42 "afm_keyword_list"
      {"StartDirection",STARTDIRECTION},
#line 19 "afm_keyword_list"
      {"EndComposites",ENDCOMPOSITES},
#line 49 "afm_keyword_list"
      {"TrackKern",TRACKKERN},
#line 15 "afm_keyword_list"
      {"Descent",DESCENT},
#line 9 "afm_keyword_list"
      {"CapHeight",CAPHEIGHT},
#line 13 "afm_keyword_list"
      {"Comment",COMMENT},
#line 10 "afm_keyword_list"
      {"CharWidth",CHARWIDTH},
#line 46 "afm_keyword_list"
      {"StartTrackKern",STARTTRACKKERN},
#line 48 "afm_keyword_list"
      {"StdVW",STDVW},
#line 38 "afm_keyword_list"
      {"Notice",NOTICE},
#line 21 "afm_keyword_list"
      {"EndFontMetrics",ENDFONTMETRICS},
#line 24 "afm_keyword_list"
      {"EndTrackKern",ENDTRACKKERN},
#line 43 "afm_keyword_list"
      {"StartFontMetrics",STARTFONTMETRICS},
#line 29 "afm_keyword_list"
      {"IsBaseFont",ISBASEFONT},
#line 17 "afm_keyword_list"
      {"EncodingScheme",ENCODINGSCHEME},
#line 31 "afm_keyword_list"
      {"ItalicAngle",ITALICANGLE},
#line 25 "afm_keyword_list"
      {"FamilyName",FAMILYNAME},
#line 58 "afm_keyword_list"
      {"XHeight",XHEIGHT},
#line 37 "afm_keyword_list"
      {"N",CHARNAME},
#line 30 "afm_keyword_list"
      {"IsFixedPitch",ISFIXEDPITCH},
#line 27 "afm_keyword_list"
      {"FontName",FONTNAME},
#line 50 "afm_keyword_list"
      {"UnderlinePosition",UNDERLINEPOSITION},
#line 51 "afm_keyword_list"
      {"UnderlineThickness",UNDERLINETHICKNESS},
#line 32 "afm_keyword_list"
      {"KP",KERNPAIR},
#line 39 "afm_keyword_list"
      {"PCC",COMPCHARPIECE},
#line 53 "afm_keyword_list"
      {"Version",VERSION},
#line 52 "afm_keyword_list"
      {"V",VVECTOR},
#line 28 "afm_keyword_list"
      {"FullName",FULLNAME},
#line 26 "afm_keyword_list"
      {"FontBBox",FONTBBOX},
#line 35 "afm_keyword_list"
      {"MappingScheme",MAPPINGSCHEME},
#line 57 "afm_keyword_list"
      {"Weight",WEIGHT},
#line 4 "afm_keyword_list"
      {"Ascent",ASCENT},
#line 3 "afm_keyword_list"
      {"Ascender",ASCENDER},
      {"",NOPE},
#line 34 "afm_keyword_list"
      {"L",LIGATURE}
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

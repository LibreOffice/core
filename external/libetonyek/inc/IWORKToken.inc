/* C++ code produced by gperf version 3.0.4 */
/* Command-line: gperf --compare-strncmp --enum --null-strings --readonly-tables --language C++ IWORKToken.gperf  */
/* Computed positions: -k'1,3,5-6,8,22,$' */

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

#line 10 "IWORKToken.gperf"

#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

using namespace IWORKToken;
#line 18 "IWORKToken.gperf"
struct Token
{
  const char *name;
  int id;
};
#include <string.h>
/* maximum key range = 1049, duplicates = 0 */

class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static const struct Token *in_word_set (const char *str, unsigned int len);
};

inline unsigned int
Perfect_Hash::hash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050,   35, 1050,    0,    5,    0,
         0,   25,   15,    5,   15,   15,   35,   10,    0, 1050,
      1050, 1050, 1050, 1050, 1050,    5,   35,    0,    5, 1050,
         0, 1050, 1050,   30,    0, 1050,   35,    0,   10, 1050,
        10, 1050,    0,   65,   25, 1050, 1050,    0,   15,   10,
      1050, 1050, 1050, 1050, 1050, 1050,    5,   10,   50,   60,
        40,    0,  180,    5,  250,  245,   20,   15,  100,   65,
        25,   50,  120,  250,    5,   10,    0,  115,  115,   55,
       195,  120,  270, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1050, 1050, 1050, 1050, 1050, 1050, 1050
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[21]];
      /*FALLTHROUGH*/
      case 21:
      case 20:
      case 19:
      case 18:
      case 17:
      case 16:
      case 15:
      case 14:
      case 13:
      case 12:
      case 11:
      case 10:
      case 9:
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]+1];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

const struct Token *
Perfect_Hash::in_word_set (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 290,
      MIN_WORD_LENGTH = 1,
      MAX_WORD_LENGTH = 41,
      MIN_HASH_VALUE = 1,
      MAX_HASH_VALUE = 1049
    };

  static const struct Token wordlist[] =
    {
      {(char*)0},
#line 26 "IWORKToken.gperf"
      {"1",_1},
#line 256 "IWORKToken.gperf"
      {"st",st},
      {(char*)0}, {(char*)0},
#line 33 "IWORKToken.gperf"
      {"IDREF",IDREF},
#line 25 "IWORKToken.gperf"
      {"0",_0},
#line 32 "IWORKToken.gperf"
      {"ID",ID},
#line 113 "IWORKToken.gperf"
      {"fmt",fmt},
      {(char*)0}, {(char*)0},
#line 241 "IWORKToken.gperf"
      {"s",s},
#line 237 "IWORKToken.gperf"
      {"rt",rt},
      {(char*)0},
#line 91 "IWORKToken.gperf"
      {"data",data},
#line 250 "IWORKToken.gperf"
      {"shape",shape},
#line 232 "IWORKToken.gperf"
      {"r",r},
#line 139 "IWORKToken.gperf"
      {"fs",fs},
      {(char*)0},
#line 263 "IWORKToken.gperf"
      {"star",star},
#line 234 "IWORKToken.gperf"
      {"right",right},
      {(char*)0}, {(char*)0},
#line 249 "IWORKToken.gperf"
      {"sfa",sfa},
      {(char*)0},
#line 162 "IWORKToken.gperf"
      {"ident",ident},
#line 269 "IWORKToken.gperf"
      {"stroke",stroke},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 259 "IWORKToken.gperf"
      {"spaceAfter",spaceAfter},
#line 126 "IWORKToken.gperf"
      {"format-type",format_type},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 163 "IWORKToken.gperf"
      {"image",image},
#line 242 "IWORKToken.gperf"
      {"scalar",scalar},
#line 236 "IWORKToken.gperf"
      {"rn",rn},
      {(char*)0},
#line 262 "IWORKToken.gperf"
      {"span",span},
#line 195 "IWORKToken.gperf"
      {"miter",miter},
#line 90 "IWORKToken.gperf"
      {"d",d},
#line 89 "IWORKToken.gperf"
      {"ct",ct},
      {(char*)0}, {(char*)0},
#line 112 "IWORKToken.gperf"
      {"firstPageMaster",firstPageMaster},
#line 268 "IWORKToken.gperf"
      {"string",string},
      {(char*)0},
#line 127 "IWORKToken.gperf"
      {"format-string",format_string},
#line 191 "IWORKToken.gperf"
      {"mark",mark},
#line 31 "IWORKToken.gperf"
      {"1414088262",_1414088262},
#line 252 "IWORKToken.gperf"
      {"shearYAngle",shearYAngle},
#line 258 "IWORKToken.gperf"
      {"so",so},
#line 41 "IWORKToken.gperf"
      {"amt",amt},
      {(char*)0},
#line 28 "IWORKToken.gperf"
      {"1299148630",_1299148630},
#line 251 "IWORKToken.gperf"
      {"shearXAngle",shearXAngle},
#line 114 "IWORKToken.gperf"
      {"fo",fo},
#line 116 "IWORKToken.gperf"
      {"fontName",fontName},
      {(char*)0},
#line 42 "IWORKToken.gperf"
      {"angle",angle},
#line 38 "IWORKToken.gperf"
      {"a",a},
#line 161 "IWORKToken.gperf"
      {"id",id},
      {(char*)0},
#line 57 "IWORKToken.gperf"
      {"butt",butt},
#line 29 "IWORKToken.gperf"
      {"1346651680",_1346651680},
#line 130 "IWORKToken.gperf"
      {"format-negative-style",format_negative_style},
      {(char*)0}, {(char*)0},
#line 196 "IWORKToken.gperf"
      {"mode",mode},
#line 27 "IWORKToken.gperf"
      {"1246774599",_1246774599},
#line 118 "IWORKToken.gperf"
      {"footer",footer},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 94 "IWORKToken.gperf"
      {"datasource",datasource},
#line 199 "IWORKToken.gperf"
      {"n",n},
#line 119 "IWORKToken.gperf"
      {"footers",footers},
      {(char*)0},
#line 239 "IWORKToken.gperf"
      {"rows",rows},
      {(char*)0},
#line 124 "IWORKToken.gperf"
      {"format-base",format_base},
#line 87 "IWORKToken.gperf"
      {"cornerRadius",cornerRadius},
      {(char*)0},
#line 99 "IWORKToken.gperf"
      {"drawable-shape",drawable_shape},
#line 111 "IWORKToken.gperf"
      {"firstLineIndent",firstLineIndent},
#line 164 "IWORKToken.gperf"
      {"image-media",image_media},
      {(char*)0}, {(char*)0},
#line 98 "IWORKToken.gperf"
      {"drawables",drawables},
#line 192 "IWORKToken.gperf"
      {"media",media},
#line 128 "IWORKToken.gperf"
      {"format-decimal-places",format_decimal_places},
#line 64 "IWORKToken.gperf"
      {"cb",cb},
      {(char*)0},
#line 183 "IWORKToken.gperf"
      {"line",line},
#line 30 "IWORKToken.gperf"
      {"1347307366",_1347307366},
#line 235 "IWORKToken.gperf"
      {"rightIndent",rightIndent},
#line 86 "IWORKToken.gperf"
      {"content",content},
#line 125 "IWORKToken.gperf"
      {"format-base-places",format_base_places},
#line 88 "IWORKToken.gperf"
      {"crbr",crbr},
#line 129 "IWORKToken.gperf"
      {"format-currency-code",format_currency_code},
#line 131 "IWORKToken.gperf"
      {"format-show-thousands-separator",format_show_thousands_separator},
#line 255 "IWORKToken.gperf"
      {"sl",sl},
#line 194 "IWORKToken.gperf"
      {"metadata",metadata},
      {(char*)0},
#line 74 "IWORKToken.gperf"
      {"chart-name",chart_name},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 186 "IWORKToken.gperf"
      {"link",link},
#line 107 "IWORKToken.gperf"
      {"false",false_},
#line 49 "IWORKToken.gperf"
      {"b",b},
#line 136 "IWORKToken.gperf"
      {"frame-w",frame_w},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 276 "IWORKToken.gperf"
      {"t",t},
#line 100 "IWORKToken.gperf"
      {"du",du},
      {(char*)0},
#line 200 "IWORKToken.gperf"
      {"name",name},
#line 293 "IWORKToken.gperf"
      {"title",title},
      {(char*)0},
#line 48 "IWORKToken.gperf"
      {"authors",authors},
      {(char*)0},
#line 189 "IWORKToken.gperf"
      {"lnbr",lnbr},
#line 270 "IWORKToken.gperf"
      {"style",style},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 202 "IWORKToken.gperf"
      {"number",number},
      {(char*)0}, {(char*)0},
#line 68 "IWORKToken.gperf"
      {"characterstyle",characterstyle},
      {(char*)0}, {(char*)0},
#line 81 "IWORKToken.gperf"
      {"comment",comment},
#line 120 "IWORKToken.gperf"
      {"footnote",footnote},
      {(char*)0},
#line 273 "IWORKToken.gperf"
      {"stylesheet",stylesheet},
#line 201 "IWORKToken.gperf"
      {"naturalSize",naturalSize},
      {(char*)0}, {(char*)0},
#line 115 "IWORKToken.gperf"
      {"fontColor",fontColor},
#line 122 "IWORKToken.gperf"
      {"footnotebr",footnotebr},
#line 272 "IWORKToken.gperf"
      {"styles",styles},
      {(char*)0}, {(char*)0},
#line 123 "IWORKToken.gperf"
      {"footnotes",footnotes},
      {(char*)0},
#line 275 "IWORKToken.gperf"
      {"superscript",superscript},
      {(char*)0},
#line 73 "IWORKToken.gperf"
      {"chart-model-object",chart_model_object},
      {(char*)0},
#line 77 "IWORKToken.gperf"
      {"color",color},
#line 96 "IWORKToken.gperf"
      {"displayname",displayname},
      {(char*)0},
#line 121 "IWORKToken.gperf"
      {"footnote-mark",footnote_mark},
      {(char*)0},
#line 71 "IWORKToken.gperf"
      {"chart-row_names",chart_row_names},
      {(char*)0}, {(char*)0},
#line 70 "IWORKToken.gperf"
      {"chart-column_names",chart_column_names},
#line 110 "IWORKToken.gperf"
      {"filtered-image",filtered_image},
      {(char*)0}, {(char*)0},
#line 165 "IWORKToken.gperf"
      {"intratopicbr",intratopicbr},
      {(char*)0},
#line 171 "IWORKToken.gperf"
      {"kind",kind},
#line 72 "IWORKToken.gperf"
      {"chart-info",chart_info},
#line 302 "IWORKToken.gperf"
      {"v",v},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 138 "IWORKToken.gperf"
      {"frame-y",frame_y},
      {(char*)0},
#line 278 "IWORKToken.gperf"
      {"tabs",tabs},
      {(char*)0}, {(char*)0},
#line 248 "IWORKToken.gperf"
      {"sf",sf},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 106 "IWORKToken.gperf"
      {"f",f},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 59 "IWORKToken.gperf"
      {"calc-engine",calc_engine},
#line 305 "IWORKToken.gperf"
      {"vector-style",vector_style},
#line 206 "IWORKToken.gperf"
      {"oddPageMaster",oddPageMaster},
#line 187 "IWORKToken.gperf"
      {"liststyle",liststyle},
      {(char*)0},
#line 197 "IWORKToken.gperf"
      {"movie-media",movie_media},
#line 133 "IWORKToken.gperf"
      {"format-use-accounting-style",format_use_accounting_style},
#line 109 "IWORKToken.gperf"
      {"filtered",filtered},
      {(char*)0},
#line 173 "IWORKToken.gperf"
      {"layer",layer},
#line 58 "IWORKToken.gperf"
      {"cached-data",cached_data},
      {(char*)0},
#line 240 "IWORKToken.gperf"
      {"row-label-formulas",row_label_formulas},
#line 55 "IWORKToken.gperf"
      {"bold",bold},
      {(char*)0}, {(char*)0},
#line 37 "IWORKToken.gperf"
      {"SFTStrokeProperty", SFTStrokeProperty},
      {(char*)0},
#line 108 "IWORKToken.gperf"
      {"fill",fill},
      {(char*)0},
#line 190 "IWORKToken.gperf"
      {"locked",locked},
#line 103 "IWORKToken.gperf"
      {"element",element},
      {(char*)0}, {(char*)0},
#line 75 "IWORKToken.gperf"
      {"chart-type",chart_type},
#line 175 "IWORKToken.gperf"
      {"layers",layers},
#line 193 "IWORKToken.gperf"
      {"menu-choices",menu_choices},
#line 277 "IWORKToken.gperf"
      {"tab",tab},
#line 105 "IWORKToken.gperf"
      {"evenPageMaster",evenPageMaster},
      {(char*)0},
#line 97 "IWORKToken.gperf"
      {"double",double_},
      {(char*)0},
#line 172 "IWORKToken.gperf"
      {"language",language},
#line 65 "IWORKToken.gperf"
      {"cell-date",cell_date},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 63 "IWORKToken.gperf"
      {"category-title",category_title},
      {(char*)0},
#line 95 "IWORKToken.gperf"
      {"date-format",date_format},
      {(char*)0}, {(char*)0},
#line 297 "IWORKToken.gperf"
      {"true",true_},
      {(char*)0},
#line 260 "IWORKToken.gperf"
      {"spaceBefore",spaceBefore},
#line 204 "IWORKToken.gperf"
      {"numrows", numrows},
      {(char*)0},
#line 298 "IWORKToken.gperf"
      {"type",type},
#line 247 "IWORKToken.gperf"
      {"self-contained-movie",self_contained_movie},
      {(char*)0}, {(char*)0},
#line 92 "IWORKToken.gperf"
      {"data-formulas",data_formulas},
      {(char*)0}, {(char*)0},
#line 56 "IWORKToken.gperf"
      {"bottom",bottom},
#line 79 "IWORKToken.gperf"
      {"columns",columns},
      {(char*)0},
#line 180 "IWORKToken.gperf"
      {"left",left},
      {(char*)0},
#line 309 "IWORKToken.gperf"
      {"w",w},
#line 137 "IWORKToken.gperf"
      {"frame-x",frame_x},
#line 238 "IWORKToken.gperf"
      {"row-span",row_span},
      {(char*)0}, {(char*)0},
#line 140 "IWORKToken.gperf"
      {"g",g},
      {(char*)0},
#line 303 "IWORKToken.gperf"
      {"val",val},
#line 35 "IWORKToken.gperf"
      {"SFTCellStylePropertyDateTimeFormat",SFTCellStylePropertyDateTimeFormat},
      {(char*)0},
#line 78 "IWORKToken.gperf"
      {"column",column},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 151 "IWORKToken.gperf"
      {"header",header},
      {(char*)0},
#line 227 "IWORKToken.gperf"
      {"pos",pos},
#line 253 "IWORKToken.gperf"
      {"size",size},
      {(char*)0},
#line 264 "IWORKToken.gperf"
      {"start-index",start_index},
#line 152 "IWORKToken.gperf"
      {"headers",headers},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 203 "IWORKToken.gperf"
      {"numcols", numcols},
#line 61 "IWORKToken.gperf"
      {"cap",cap},
      {(char*)0}, {(char*)0},
#line 43 "IWORKToken.gperf"
      {"anon-styles",anon_styles},
#line 221 "IWORKToken.gperf"
      {"pattern",pattern},
      {(char*)0},
#line 167 "IWORKToken.gperf"
      {"join",join},
#line 101 "IWORKToken.gperf"
      {"duration-format",duration_format},
#line 257 "IWORKToken.gperf"
      {"slide-style",slide_style},
#line 261 "IWORKToken.gperf"
      {"spacing",spacing},
      {(char*)0},
#line 299 "IWORKToken.gperf"
      {"underline",underline},
      {(char*)0},
#line 185 "IWORKToken.gperf"
      {"linespacing",linespacing},
#line 44 "IWORKToken.gperf"
      {"aspectRatioLocked",aspectRatioLocked},
      {(char*)0},
#line 150 "IWORKToken.gperf"
      {"head",head},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 170 "IWORKToken.gperf"
      {"keywords",keywords},
#line 271 "IWORKToken.gperf"
      {"style-run",style_run},
      {(char*)0},
#line 176 "IWORKToken.gperf"
      {"layout",layout},
#line 135 "IWORKToken.gperf"
      {"frame-h",frame_h},
      {(char*)0}, {(char*)0},
#line 181 "IWORKToken.gperf"
      {"leftIndent",leftIndent},
#line 178 "IWORKToken.gperf"
      {"layoutstyle",layoutstyle},
      {(char*)0},
#line 205 "IWORKToken.gperf"
      {"number-format",number_format},
#line 289 "IWORKToken.gperf"
      {"text",text},
#line 66 "IWORKToken.gperf"
      {"cell-style",cell_style},
#line 312 "IWORKToken.gperf"
      {"x",x},
#line 224 "IWORKToken.gperf"
      {"pm",pm},
#line 69 "IWORKToken.gperf"
      {"characterstyle-ref",characterstyle_ref},
      {(char*)0},
#line 211 "IWORKToken.gperf"
      {"pageBreakBefore",pageBreakBefore},
      {(char*)0}, {(char*)0},
#line 283 "IWORKToken.gperf"
      {"tabular-style",tabular_style},
#line 274 "IWORKToken.gperf"
      {"stylesheet-ref",stylesheet_ref},
      {(char*)0}, {(char*)0},
#line 182 "IWORKToken.gperf"
      {"leveled",leveled},
#line 76 "IWORKToken.gperf"
      {"col-span",col_span},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 177 "IWORKToken.gperf"
      {"layoutMargins",layoutMargins},
#line 85 "IWORKToken.gperf"
      {"container-hint",container_hint},
      {(char*)0},
#line 160 "IWORKToken.gperf"
      {"http://developer.apple.com/namespaces/sfa",NS_URI_SFA},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 280 "IWORKToken.gperf"
      {"tabstop",tabstop},
#line 198 "IWORKToken.gperf"
      {"mutable-array",mutable_array},
      {(char*)0},
#line 212 "IWORKToken.gperf"
      {"pagemaster",pagemaster},
#line 51 "IWORKToken.gperf"
      {"bezier",bezier},
      {(char*)0},
#line 117 "IWORKToken.gperf"
      {"fontSize",fontSize},
      {(char*)0},
#line 39 "IWORKToken.gperf"
      {"align",align},
#line 184 "IWORKToken.gperf"
      {"lineSpacing",lineSpacing},
      {(char*)0}, {(char*)0},
#line 132 "IWORKToken.gperf"
      {"format-fraction-accuracy",format_fraction_accuracy},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 295 "IWORKToken.gperf"
      {"top",top},
      {(char*)0}, {(char*)0},
#line 80 "IWORKToken.gperf"
      {"column-label-formulas",column_label_formulas},
#line 34 "IWORKToken.gperf"
      {"SFTCellStylePropertyNumberFormat",SFTCellStylePropertyNumberFormat},
#line 233 "IWORKToken.gperf"
      {"relative",relative},
#line 36 "IWORKToken.gperf"
      {"SFTCellStylePropertyDurationFormat",SFTCellStylePropertyDurationFormat},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 45 "IWORKToken.gperf"
      {"attachment",attachment},
#line 209 "IWORKToken.gperf"
      {"p",p},
#line 281 "IWORKToken.gperf"
      {"tabular-info",tabular_info},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 306 "IWORKToken.gperf"
      {"vector-style-ref",vector_style_ref},
#line 245 "IWORKToken.gperf"
      {"sectionstyle",sectionstyle},
#line 188 "IWORKToken.gperf"
      {"liststyle-ref",liststyle_ref},
#line 134 "IWORKToken.gperf"
      {"formula-chart-model",formula_chart_model},
      {(char*)0},
#line 47 "IWORKToken.gperf"
      {"attachments",attachments},
#line 285 "IWORKToken.gperf"
      {"tailAtCenter",tailAtCenter},
#line 155 "IWORKToken.gperf"
      {"hfs-type",hfs_type},
#line 24 "IWORKToken.gperf"
      {"__multilingual",__multilingual},
#line 82 "IWORKToken.gperf"
      {"connection-line",connection_line},
#line 84 "IWORKToken.gperf"
      {"connection-style",connection_style},
#line 244 "IWORKToken.gperf"
      {"section",section},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 313 "IWORKToken.gperf"
      {"y",y},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 267 "IWORKToken.gperf"
      {"strikethru",strikethru},
      {(char*)0},
#line 208 "IWORKToken.gperf"
      {"outline",outline},
      {(char*)0},
#line 216 "IWORKToken.gperf"
      {"paragraphstyle",paragraphstyle},
#line 215 "IWORKToken.gperf"
      {"paragraphStroke",paragraphStroke},
#line 166 "IWORKToken.gperf"
      {"italic",italic},
#line 168 "IWORKToken.gperf"
      {"keepLinesTogether",keepLinesTogether},
#line 296 "IWORKToken.gperf"
      {"tracking",tracking},
#line 213 "IWORKToken.gperf"
      {"paragraphBorderType",paragraphBorderType},
      {(char*)0},
#line 243 "IWORKToken.gperf"
      {"scalar-path",scalar_path},
      {(char*)0},
#line 93 "IWORKToken.gperf"
      {"data-ref",data_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 174 "IWORKToken.gperf"
      {"layer-ref",layer_ref},
      {(char*)0},
#line 266 "IWORKToken.gperf"
      {"sticky-note",sticky_note},
#line 292 "IWORKToken.gperf"
      {"text-storage",text_storage},
      {(char*)0},
#line 40 "IWORKToken.gperf"
      {"alignment",alignment},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 282 "IWORKToken.gperf"
      {"tabular-model",tabular_model},
#line 290 "IWORKToken.gperf"
      {"textBackground",textBackground},
      {(char*)0},
#line 254 "IWORKToken.gperf"
      {"sizesLocked",sizesLocked},
#line 310 "IWORKToken.gperf"
      {"widowControl",widowControl},
#line 50 "IWORKToken.gperf"
      {"baselineShift",baselineShift},
#line 158 "IWORKToken.gperf"
      {"href",href},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 207 "IWORKToken.gperf"
      {"other-datas",other_datas},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 304 "IWORKToken.gperf"
      {"value-title",value_title},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 300 "IWORKToken.gperf"
      {"unfiltered",unfiltered},
#line 294 "IWORKToken.gperf"
      {"title-placeholder-ref",title_placeholder_ref},
      {(char*)0}, {(char*)0},
#line 62 "IWORKToken.gperf"
      {"capitalization",capitalization},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 284 "IWORKToken.gperf"
      {"tail",tail},
#line 54 "IWORKToken.gperf"
      {"body-placeholder-ref",body_placeholder_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 219 "IWORKToken.gperf"
      {"parent-ref",parent_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 231 "IWORKToken.gperf"
      {"proxy-master-layer",proxy_master_layer},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 179 "IWORKToken.gperf"
      {"layoutstyle-ref",layoutstyle_ref},
#line 149 "IWORKToken.gperf"
      {"h",h},
      {(char*)0},
#line 214 "IWORKToken.gperf"
      {"paragraphFill",paragraphFill},
#line 67 "IWORKToken.gperf"
      {"cell-style-ref",cell_style_ref},
#line 225 "IWORKToken.gperf"
      {"point",point},
      {(char*)0}, {(char*)0},
#line 104 "IWORKToken.gperf"
      {"equal-columns",equal_columns},
#line 220 "IWORKToken.gperf"
      {"path",path},
#line 159 "IWORKToken.gperf"
      {"http://developer.apple.com/namespaces/sf",NS_URI_SF},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 53 "IWORKToken.gperf"
      {"bezier-ref",bezier_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 218 "IWORKToken.gperf"
      {"parent-ident",parent_ident},
      {(char*)0},
#line 144 "IWORKToken.gperf"
      {"grid",grid},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 148 "IWORKToken.gperf"
      {"group",group},
      {(char*)0}, {(char*)0},
#line 141 "IWORKToken.gperf"
      {"geometry",geometry},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 46 "IWORKToken.gperf"
      {"attachment-ref",attachment_ref},
      {(char*)0}, {(char*)0},
#line 229 "IWORKToken.gperf"
      {"property-map",property_map},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 246 "IWORKToken.gperf"
      {"sectionstyle_ref",sectionstyle_ref},
      {(char*)0},
#line 228 "IWORKToken.gperf"
      {"position",position},
#line 291 "IWORKToken.gperf"
      {"text-body",text_body},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 60 "IWORKToken.gperf"
      {"callout2-path",callout2_path},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 210 "IWORKToken.gperf"
      {"padding",padding},
#line 279 "IWORKToken.gperf"
      {"tabs-ref",tabs_ref},
      {(char*)0},
#line 265 "IWORKToken.gperf"
      {"stop-index",stop_index},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 217 "IWORKToken.gperf"
      {"paragraphstyle-ref",paragraphstyle_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 301 "IWORKToken.gperf"
      {"unfiltered-ref",unfiltered_ref},
      {(char*)0}, {(char*)0},
#line 307 "IWORKToken.gperf"
      {"verticalFlip",verticalFlip},
      {(char*)0},
#line 308 "IWORKToken.gperf"
      {"vertical-gridline-styles",vertical_gridline_styles,},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 157 "IWORKToken.gperf"
      {"horizontal-gridline-styles",horizontal_gridline_styles},
#line 169 "IWORKToken.gperf"
      {"keepWithNext",keepWithNext},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 153 "IWORKToken.gperf"
      {"headline-style",headline_style},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 222 "IWORKToken.gperf"
      {"placeholder-style",placeholder_style},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 83 "IWORKToken.gperf"
      {"connection-path",connection_path},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 146 "IWORKToken.gperf"
      {"grid-row",grid_row},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 288 "IWORKToken.gperf"
      {"tailSize",tailSize},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 287 "IWORKToken.gperf"
      {"tailPositionY",tailPositionY},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 286 "IWORKToken.gperf"
      {"tailPositionX",tailPositionX},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 156 "IWORKToken.gperf"
      {"horizontalFlip",horizontalFlip},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 52 "IWORKToken.gperf"
      {"bezier-path",bezier_path},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 145 "IWORKToken.gperf"
      {"grid-column",grid_column},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 311 "IWORKToken.gperf"
      {"width",width},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 154 "IWORKToken.gperf"
      {"height",height},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 102 "IWORKToken.gperf"
      {"editable-bezier-path",editable_bezier_path},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 230 "IWORKToken.gperf"
      {"proxied-cell-ref",proxied_cell_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 226 "IWORKToken.gperf"
      {"point-path",point_path},
      {(char*)0}, {(char*)0},
#line 142 "IWORKToken.gperf"
      {"graphic-style",graphic_style},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 223 "IWORKToken.gperf"
      {"placeholder-style-ref",placeholder_style_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 143 "IWORKToken.gperf"
      {"graphic-style-ref",graphic_style_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 147 "IWORKToken.gperf"
      {"gridline-index",gridline_index,}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (s && *str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
#line 314 "IWORKToken.gperf"


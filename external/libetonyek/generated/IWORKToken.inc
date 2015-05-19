/* C++ code produced by gperf version 3.0.4 */
/* Command-line: /usr/bin/gperf --compare-strncmp --enum --null-strings --readonly-tables --language C++ IWORKToken.gperf  */
/* Computed positions: -k'1-2,5-7,21,27,$' */

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
/* maximum key range = 480, duplicates = 0 */

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
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481,  75, 481,   5, 105,  60,
      481, 481, 481, 481, 481, 481, 481, 481,   5, 481,
      481, 481, 481, 481, 481,  10,  30,   0,   5, 481,
        0, 481,   5,   0, 481, 481, 110, 481,   0, 481,
       15, 481,  55,  10,  15, 481,   0,  10,  10,   0,
      481, 481, 481, 481, 481,   5, 481,  20, 235,  40,
      150,   5,  70, 200,  30,  75, 481,  40,  10, 140,
       25,   0,  15, 481,  10,   0,   5, 105, 100, 115,
       45, 145,  20, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481, 481, 481, 481, 481,
      481, 481, 481, 481, 481, 481
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[26]];
      /*FALLTHROUGH*/
      case 26:
      case 25:
      case 24:
      case 23:
      case 22:
      case 21:
        hval += asso_values[(unsigned char)str[20]];
      /*FALLTHROUGH*/
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
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
      case 3:
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

const struct Token *
Perfect_Hash::in_word_set (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 212,
      MIN_WORD_LENGTH = 1,
      MAX_WORD_LENGTH = 50,
      MIN_HASH_VALUE = 1,
      MAX_HASH_VALUE = 480
    };

  static const struct Token wordlist[] =
    {
      {(char*)0},
#line 180 "IWORKToken.gperf"
      {"s",s},
#line 193 "IWORKToken.gperf"
      {"so",so},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 28 "IWORKToken.gperf"
      {"IDREF",IDREF},
#line 203 "IWORKToken.gperf"
      {"t",t},
#line 27 "IWORKToken.gperf"
      {"ID",ID},
      {(char*)0},
#line 179 "IWORKToken.gperf"
      {"rows",rows},
      {(char*)0},
#line 201 "IWORKToken.gperf"
      {"styles",styles},
      {(char*)0},
#line 171 "IWORKToken.gperf"
      {"pos",pos},
#line 216 "IWORKToken.gperf"
      {"text",text},
#line 200 "IWORKToken.gperf"
      {"style",style},
#line 175 "IWORKToken.gperf"
      {"r",r},
      {(char*)0}, {(char*)0},
#line 222 "IWORKToken.gperf"
      {"true",true_},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 205 "IWORKToken.gperf"
      {"tabs",tabs},
#line 169 "IWORKToken.gperf"
      {"point",point},
#line 159 "IWORKToken.gperf"
      {"p",p},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 211 "IWORKToken.gperf"
      {"tail",tail},
      {(char*)0},
#line 46 "IWORKToken.gperf"
      {"a",a},
      {(char*)0}, {(char*)0},
#line 196 "IWORKToken.gperf"
      {"span",span},
#line 187 "IWORKToken.gperf"
      {"shape",shape},
#line 133 "IWORKToken.gperf"
      {"layers",layers},
      {(char*)0}, {(char*)0},
#line 148 "IWORKToken.gperf"
      {"lnbr",lnbr},
      {(char*)0},
#line 154 "IWORKToken.gperf"
      {"n",n},
#line 81 "IWORKToken.gperf"
      {"ct",ct},
#line 214 "IWORKToken.gperf"
      {"tailPositionY",tailPositionY},
      {(char*)0},
#line 131 "IWORKToken.gperf"
      {"layer",layer},
      {(char*)0},
#line 114 "IWORKToken.gperf"
      {"headers",headers},
      {(char*)0}, {(char*)0},
#line 50 "IWORKToken.gperf"
      {"angle",angle},
#line 112 "IWORKToken.gperf"
      {"h",h},
#line 212 "IWORKToken.gperf"
      {"tailAtCenter",tailAtCenter},
#line 213 "IWORKToken.gperf"
      {"tailPositionX",tailPositionX},
#line 80 "IWORKToken.gperf"
      {"crbr",crbr},
#line 72 "IWORKToken.gperf"
      {"color",color},
#line 189 "IWORKToken.gperf"
      {"shearYAngle",shearYAngle},
#line 207 "IWORKToken.gperf"
      {"tabstop",tabstop},
      {(char*)0},
#line 166 "IWORKToken.gperf"
      {"path",path},
      {(char*)0},
#line 195 "IWORKToken.gperf"
      {"spaceBefore",spaceBefore},
#line 173 "IWORKToken.gperf"
      {"property-map",property_map},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 188 "IWORKToken.gperf"
      {"shearXAngle",shearXAngle},
#line 208 "IWORKToken.gperf"
      {"tabular-info",tabular_info},
#line 178 "IWORKToken.gperf"
      {"row-span",row_span},
      {(char*)0}, {(char*)0},
#line 116 "IWORKToken.gperf"
      {"height",height},
#line 167 "IWORKToken.gperf"
      {"placeholder-style",placeholder_style},
#line 210 "IWORKToken.gperf"
      {"tabular-style",tabular_style},
#line 190 "IWORKToken.gperf"
      {"size",size},
#line 47 "IWORKToken.gperf"
      {"align",align},
#line 181 "IWORKToken.gperf"
      {"scalar",scalar},
#line 78 "IWORKToken.gperf"
      {"content",content},
#line 209 "IWORKToken.gperf"
      {"tabular-model",tabular_model},
      {(char*)0}, {(char*)0},
#line 234 "IWORKToken.gperf"
      {"x",x},
#line 100 "IWORKToken.gperf"
      {"footers",footers},
#line 186 "IWORKToken.gperf"
      {"sfa",sfa},
#line 141 "IWORKToken.gperf"
      {"line",line},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 97 "IWORKToken.gperf"
      {"fontColor",fontColor},
      {(char*)0},
#line 155 "IWORKToken.gperf"
      {"naturalSize",naturalSize},
      {(char*)0}, {(char*)0},
#line 118 "IWORKToken.gperf"
      {"horizontalFlip",horizontalFlip},
#line 91 "IWORKToken.gperf"
      {"false",false_},
#line 192 "IWORKToken.gperf"
      {"slide-style",slide_style},
#line 219 "IWORKToken.gperf"
      {"text-storage",text_storage},
#line 71 "IWORKToken.gperf"
      {"col-span",col_span},
#line 30 "IWORKToken.gperf"
      {"SFTCellStylePropertyParagraphStyle",SFTCellStylePropertyParagraphStyle},
#line 74 "IWORKToken.gperf"
      {"connection-line",connection_line},
#line 76 "IWORKToken.gperf"
      {"connection-style",connection_style},
      {(char*)0}, {(char*)0},
#line 119 "IWORKToken.gperf"
      {"href",href},
      {(char*)0},
#line 45 "IWORKToken.gperf"
      {"TableCellStylePropertyFormatNegativeStyle",TableCellStylePropertyFormatNegativeStyle},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 194 "IWORKToken.gperf"
      {"spaceAfter",spaceAfter},
#line 26 "IWORKToken.gperf"
      {"1",_1},
#line 79 "IWORKToken.gperf"
      {"cornerRadius",cornerRadius},
      {(char*)0},
#line 24 "IWORKToken.gperf"
      {"__multilingual",__multilingual},
      {(char*)0},
#line 121 "IWORKToken.gperf"
      {"http://developer.apple.com/namespaces/sfa",NS_URI_SFA},
      {(char*)0},
#line 172 "IWORKToken.gperf"
      {"position",position},
#line 144 "IWORKToken.gperf"
      {"link",link},
      {(char*)0},
#line 177 "IWORKToken.gperf"
      {"rightIndent",rightIndent},
      {(char*)0}, {(char*)0},
#line 66 "IWORKToken.gperf"
      {"capitalization",capitalization},
#line 75 "IWORKToken.gperf"
      {"connection-path",connection_path},
#line 51 "IWORKToken.gperf"
      {"anon-styles",anon_styles},
#line 183 "IWORKToken.gperf"
      {"section",section},
      {(char*)0}, {(char*)0},
#line 67 "IWORKToken.gperf"
      {"cell-style",cell_style},
#line 90 "IWORKToken.gperf"
      {"f",f},
#line 185 "IWORKToken.gperf"
      {"sf",sf},
#line 215 "IWORKToken.gperf"
      {"tailSize",tailSize},
      {(char*)0},
#line 184 "IWORKToken.gperf"
      {"self-contained-movie",self_contained_movie},
      {(char*)0},
#line 125 "IWORKToken.gperf"
      {"intratopicbr",intratopicbr},
      {(char*)0},
#line 152 "IWORKToken.gperf"
      {"mode",mode},
#line 170 "IWORKToken.gperf"
      {"point-path",point_path},
#line 134 "IWORKToken.gperf"
      {"layout",layout},
#line 128 "IWORKToken.gperf"
      {"keepWithNext",keepWithNext},
      {(char*)0},
#line 69 "IWORKToken.gperf"
      {"characterstyle",characterstyle},
      {(char*)0},
#line 137 "IWORKToken.gperf"
      {"layoutstyle",layoutstyle},
#line 164 "IWORKToken.gperf"
      {"parent-ident",parent_ident},
      {(char*)0},
#line 223 "IWORKToken.gperf"
      {"type",type},
      {(char*)0},
#line 156 "IWORKToken.gperf"
      {"number",number},
      {(char*)0}, {(char*)0},
#line 115 "IWORKToken.gperf"
      {"headline-style",headline_style},
      {(char*)0},
#line 136 "IWORKToken.gperf"
      {"layoutStyle",layoutStyle},
      {(char*)0},
#line 49 "IWORKToken.gperf"
      {"amt",amt},
      {(char*)0},
#line 198 "IWORKToken.gperf"
      {"strikethru",strikethru},
#line 202 "IWORKToken.gperf"
      {"superscript",superscript},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 120 "IWORKToken.gperf"
      {"http://developer.apple.com/namespaces/sf",NS_URI_SF},
      {(char*)0}, {(char*)0},
#line 221 "IWORKToken.gperf"
      {"tocStyle",tocStyle},
#line 77 "IWORKToken.gperf"
      {"container-hint",container_hint},
#line 135 "IWORKToken.gperf"
      {"layoutParagraphStyle",layoutParagraphStyle},
#line 64 "IWORKToken.gperf"
      {"calc-engine",calc_engine},
      {(char*)0}, {(char*)0},
#line 93 "IWORKToken.gperf"
      {"filtered-image",filtered_image},
      {(char*)0},
#line 182 "IWORKToken.gperf"
      {"scalar-path",scalar_path},
      {(char*)0},
#line 99 "IWORKToken.gperf"
      {"fontSize",fontSize},
#line 113 "IWORKToken.gperf"
      {"head",head},
#line 150 "IWORKToken.gperf"
      {"media",media},
#line 29 "IWORKToken.gperf"
      {"SFTCellStylePropertyLayoutStyle",SFTCellStylePropertyLayoutStyle},
      {(char*)0},
#line 206 "IWORKToken.gperf"
      {"tabs-ref",tabs_ref},
#line 83 "IWORKToken.gperf"
      {"data",data},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 227 "IWORKToken.gperf"
      {"v",v},
      {(char*)0}, {(char*)0},
#line 132 "IWORKToken.gperf"
      {"layer-ref",layer_ref},
#line 139 "IWORKToken.gperf"
      {"leftIndent",leftIndent},
      {(char*)0},
#line 228 "IWORKToken.gperf"
      {"vector-style",vector_style},
#line 176 "IWORKToken.gperf"
      {"relative",relative},
#line 68 "IWORKToken.gperf"
      {"cell-style-ref",cell_style_ref},
      {(char*)0},
#line 25 "IWORKToken.gperf"
      {"0",_0},
#line 73 "IWORKToken.gperf"
      {"columns",columns},
#line 65 "IWORKToken.gperf"
      {"callout2-path",callout2_path},
#line 48 "IWORKToken.gperf"
      {"alignment",alignment},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 165 "IWORKToken.gperf"
      {"parent-ref",parent_ref},
#line 168 "IWORKToken.gperf"
      {"placeholder-style-ref",placeholder_style_ref},
#line 158 "IWORKToken.gperf"
      {"outline",outline},
#line 70 "IWORKToken.gperf"
      {"characterstyle-ref",characterstyle_ref},
      {(char*)0},
#line 138 "IWORKToken.gperf"
      {"layoutstyle-ref",layoutstyle_ref},
      {(char*)0},
#line 101 "IWORKToken.gperf"
      {"frame-h",frame_h},
      {(char*)0}, {(char*)0},
#line 123 "IWORKToken.gperf"
      {"image",image},
#line 231 "IWORKToken.gperf"
      {"w",w},
      {(char*)0}, {(char*)0},
#line 226 "IWORKToken.gperf"
      {"unfiltered-ref",unfiltered_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 224 "IWORKToken.gperf"
      {"underline",underline},
#line 122 "IWORKToken.gperf"
      {"ident",ident},
#line 126 "IWORKToken.gperf"
      {"italic",italic},
      {(char*)0},
#line 98 "IWORKToken.gperf"
      {"fontName",fontName},
      {(char*)0},
#line 111 "IWORKToken.gperf"
      {"group",group},
#line 55 "IWORKToken.gperf"
      {"attachments",attachments},
      {(char*)0}, {(char*)0},
#line 146 "IWORKToken.gperf"
      {"liststyle",liststyle},
#line 53 "IWORKToken.gperf"
      {"attachment",attachment},
#line 157 "IWORKToken.gperf"
      {"other-datas",other_datas},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 233 "IWORKToken.gperf"
      {"width",width},
      {(char*)0},
#line 103 "IWORKToken.gperf"
      {"frame-x",frame_x},
      {(char*)0},
#line 145 "IWORKToken.gperf"
      {"listStyle",listStyle},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 204 "IWORKToken.gperf"
      {"tab",tab},
#line 217 "IWORKToken.gperf"
      {"textBackground",textBackground},
      {(char*)0}, {(char*)0},
#line 230 "IWORKToken.gperf"
      {"verticalFlip",verticalFlip},
      {(char*)0},
#line 129 "IWORKToken.gperf"
      {"kind",kind},
      {(char*)0},
#line 58 "IWORKToken.gperf"
      {"bezier",bezier},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 229 "IWORKToken.gperf"
      {"vector-style-ref",vector_style_ref},
      {(char*)0},
#line 117 "IWORKToken.gperf"
      {"hfs-type",hfs_type},
      {(char*)0}, {(char*)0},
#line 197 "IWORKToken.gperf"
      {"sticky-note",sticky_note},
#line 127 "IWORKToken.gperf"
      {"keepLinesTogether",keepLinesTogether},
      {(char*)0},
#line 162 "IWORKToken.gperf"
      {"paragraphstyle",paragraphstyle},
#line 95 "IWORKToken.gperf"
      {"followingLayoutStyle",followingLayoutStyle},
      {(char*)0},
#line 52 "IWORKToken.gperf"
      {"aspectRatioLocked",aspectRatioLocked},
#line 161 "IWORKToken.gperf"
      {"paragraphFill",paragraphFill},
#line 160 "IWORKToken.gperf"
      {"paragraphBorderType",paragraphBorderType},
#line 85 "IWORKToken.gperf"
      {"datasource",datasource},
#line 235 "IWORKToken.gperf"
      {"y",y},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 82 "IWORKToken.gperf"
      {"d",d},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 44 "IWORKToken.gperf"
      {"SFTableStylePropertyVectorStyle",SFTableStylePropertyVectorStyle},
#line 43 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderVectorStyle",SFTableStylePropertyHeaderVectorStyle},
      {(char*)0}, {(char*)0},
#line 225 "IWORKToken.gperf"
      {"unfiltered",unfiltered},
#line 38 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderColumnCellStyle",SFTableStylePropertyHeaderColumnCellStyle},
      {(char*)0},
#line 33 "IWORKToken.gperf"
      {"SFTableStylePropertyCellParagraphStyle",SFTableStylePropertyCellParagraphStyle},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 36 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderColumnCellLayoutStyle",SFTableStylePropertyHeaderColumnCellLayoutStyle},
#line 147 "IWORKToken.gperf"
      {"liststyle-ref",liststyle_ref},
#line 54 "IWORKToken.gperf"
      {"attachment-ref",attachment_ref},
#line 37 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderColumnCellParagraphStyle",SFTableStylePropertyHeaderColumnCellParagraphStyle},
#line 149 "IWORKToken.gperf"
      {"locked",locked},
      {(char*)0},
#line 92 "IWORKToken.gperf"
      {"filtered",filtered},
      {(char*)0}, {(char*)0},
#line 42 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderSeperatorVectorStyle",SFTableStylePropertyHeaderSeperatorVectorStyle},
#line 232 "IWORKToken.gperf"
      {"widowControl",widowControl},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 143 "IWORKToken.gperf"
      {"linespacing",linespacing},
#line 31 "IWORKToken.gperf"
      {"SFTableStylePropertyBorderVectorStyle",SFTableStylePropertyBorderVectorStyle},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 220 "IWORKToken.gperf"
      {"title-placeholder-ref",title_placeholder_ref},
#line 140 "IWORKToken.gperf"
      {"leveled",leveled},
#line 84 "IWORKToken.gperf"
      {"data-ref",data_ref},
      {(char*)0}, {(char*)0},
#line 142 "IWORKToken.gperf"
      {"lineSpacing",lineSpacing},
      {(char*)0},
#line 35 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderBorderVectorStyle",SFTableStylePropertyHeaderBorderVectorStyle},
      {(char*)0}, {(char*)0},
#line 191 "IWORKToken.gperf"
      {"sizesLocked",sizesLocked},
      {(char*)0},
#line 151 "IWORKToken.gperf"
      {"metadata",metadata},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 163 "IWORKToken.gperf"
      {"paragraphstyle-ref",paragraphstyle_ref},
      {(char*)0},
#line 94 "IWORKToken.gperf"
      {"firstLineIndent",firstLineIndent},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 109 "IWORKToken.gperf"
      {"grid-column",grid_column},
#line 88 "IWORKToken.gperf"
      {"du",du},
#line 41 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderRowCellStyle",SFTableStylePropertyHeaderRowCellStyle},
#line 108 "IWORKToken.gperf"
      {"grid",grid},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 130 "IWORKToken.gperf"
      {"language",language},
#line 39 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderRowCellLayoutStyle",SFTableStylePropertyHeaderRowCellLayoutStyle},
      {(char*)0},
#line 59 "IWORKToken.gperf"
      {"bezier-path",bezier_path},
#line 40 "IWORKToken.gperf"
      {"SFTableStylePropertyHeaderRowCellParagraphStyle",SFTableStylePropertyHeaderRowCellParagraphStyle},
#line 107 "IWORKToken.gperf"
      {"graphic-style",graphic_style},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 106 "IWORKToken.gperf"
      {"geometry",geometry},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 57 "IWORKToken.gperf"
      {"baselineShift",baselineShift},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 62 "IWORKToken.gperf"
      {"bold",bold},
      {(char*)0},
#line 153 "IWORKToken.gperf"
      {"movie-media",movie_media},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 102 "IWORKToken.gperf"
      {"frame-w",frame_w},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 105 "IWORKToken.gperf"
      {"g",g},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 60 "IWORKToken.gperf"
      {"bezier-ref",bezier_ref},
      {(char*)0}, {(char*)0},
#line 174 "IWORKToken.gperf"
      {"proxy-master-layer",proxy_master_layer},
      {(char*)0}, {(char*)0},
#line 86 "IWORKToken.gperf"
      {"displayname",displayname},
      {(char*)0},
#line 110 "IWORKToken.gperf"
      {"grid-row",grid_row},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0},
#line 61 "IWORKToken.gperf"
      {"body-placeholder-ref",body_placeholder_ref},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 96 "IWORKToken.gperf"
      {"followingParagraphStyle",followingParagraphStyle},
#line 87 "IWORKToken.gperf"
      {"drawables",drawables},
      {(char*)0},
#line 199 "IWORKToken.gperf"
      {"string",string},
      {(char*)0}, {(char*)0},
#line 34 "IWORKToken.gperf"
      {"SFTableStylePropertyCellStyle",SFTableStylePropertyCellStyle},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 32 "IWORKToken.gperf"
      {"SFTableStylePropertyCellLayoutStyle",SFTableStylePropertyCellLayoutStyle},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 104 "IWORKToken.gperf"
      {"frame-y",frame_y},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 124 "IWORKToken.gperf"
      {"image-media",image_media},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 89 "IWORKToken.gperf"
      {"editable-bezier-path",editable_bezier_path},
#line 56 "IWORKToken.gperf"
      {"b",b},
      {(char*)0}, {(char*)0},
#line 218 "IWORKToken.gperf"
      {"text-body",text_body},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0},
#line 63 "IWORKToken.gperf"
      {"bulletListStyle",bulletListStyle}
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
#line 236 "IWORKToken.gperf"


/***************************************************************************
    smathlexer.lxx  -  rules for reading smath formula text
    - scanner generation file for flex
                             -------------------
    begin                : Wed May 21 2008
    copyright            : (C) 2021 by Jan Rheinlaender
    email                : jrheinlaender@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%{
#if defined INSIDE_SM
  class SmDocShell;
  class documentObject;
  #define FORMULAOBJECT SmDocShell
  #define DOCUMENTOBJECT documentObject
  namespace imath { class smathparser; }
  // Announce to Flex the prototype we want for lexing function. This must match the declaration of lex_param at the top of smathparser.yxx
  # define YY_DECL imath::smathparser::token::yytokentype imathlex (imath::smathparser::semantic_type* yylval, \
              imath::smathparser::location_type* yylloc, std::shared_ptr<eqc> compiler, \
              unsigned include_level)
  // These includes are required for smathparser.hxx
  #include <imath/option.hxx>
  #include <imath/printing.hxx>
  #include <imath/imathutils.hxx>
  #include <imath/iFormulaLine.hxx>
  #include <imath/unit.hxx>
  #include <imath/func.hxx>
  #include <imath/msgdriver.hxx>
#else
  #include "iFormula.hxx"
  #define FORMULAOBJECT iFormula
  #define DOCUMENTOBJECT iMathDoc
  #include "unit.hxx"
  #include "func.hxx"
  #include "msgdriver.hxx"
#endif
#include "smathparser.hxx"

// This can also be done by calling flex with --nounistd
#ifdef _MSC_VER
  #define YY_NO_UNISTD_H
  #include <codecvt>
#endif

/* By default yylex returns int, we use token_type.
   Unfortunately yyterminate by default returns 0, which is
   not of token_type.  */
#define yyterminate() BEGIN(INITIAL); return token::ENDSTRING

// ----------------- Helper functions ----------------
// TODO: Is there any way to define this as a function??? The problem is accessing last_token etc, and yyless
#define CHECK_IMPLICIT_MUL(thetoken) \
  if (implicit_mul_might_follow) { \
    if ((last_token == token::FUNC)  && (thetoken == token::BOPEN)) { \
      checkresult = false; /* No implicit mul may exist between these two tokens */ \
    } else { \
      implicit_mul_might_follow = false; \
      last_token = token_type(' '); \
      yylloc->columns(-yyleng); \
      yyless(0); \
      checkresult = true; \
    } \
  } else { \
    checkresult = false;\
  }\

  typedef imath::smathparser::token token; // just an abbreviation
  typedef imath::smathparser::token_type token_type;

  // Note: An implicit multiplication occurs when the multiplication sign '*' is omitted, i.e.
  // '3x+4y' has two implicit multiplications. Parsing this is difficult because adding a rule
  // like exp exp -> exp leads to numerous shift/reduce conflicts. The way to avoid it is to add
  // some ugly hacks to the scanner. The logic behind these hacks is:
  // An implicit multiplication might follow the tokens SYMBOL, DIGIT, UNIT or any kind of closing bracket
  // It will only follow if the next token is SYMBOL, DIGIT, UNIT, FUNC, FRAC, VALUE or any kind
  // of opening bracket.
  // There are some exceptions:
  // - No implicit mul may occur between a function name and an opening bracket. Note that this
  //   case is ambiguous! EQC assumes that any opening bracket after a function name belongs to
  //   the function
  // In case an implicit mul is detected, the scanned token is put back on the input stream and IMPMUL returned instead.

  // Variables for keeping track of the scanner state
  bool implicit_mul_might_follow;
  token_type last_token;
  std::string last_attribute = "";
  std::string argument = "";
  bool checkresult;
%}

/* Because there is no `#include'-like feature we don't need `yywrap',and we parse an actual file, this is not an
   interactive session with the user.  Finally we enable the scanner
   tracing features
*/
%option noyywrap batch
/*%option debug*/
/* required for MSVC compile. How can we access config.h: HAVE_UNISTD_H here? */
%option never-interactive
/* %option bison-locations */

/* Unicode, see
   http://groups.google.com/group/comp.compilers/browse_thread/thread/2d2bff9c324d8e05
   added in 1.4.3 */
UTF8_BYTE_ORDER_MARK   [\xEF][\xBB][\xBF]

ASCII       [\x0-\xFF]{-}[⁰¹²³⁴⁵⁶⁷⁸⁹]
/* Excludes ²³¹ = C2 B2, C2 B3, C2 B9 */
U2_1        \xC2[\x80-\xB1]
U2_2        \xC2[\xB4-\xB8]
U2_3        \xC2[\xBA-\xBF]
U2          {U2_1}|{U2_2}|{U2_3}|[\xC3-\xDF][\x80-\xBF]
U3          [\xE0][\xA0-\xBF][\x80-\xBF]
/* Excludes ⁰⁴⁵⁶⁷⁸⁹ = E2 81 B0, E2 81 B4, E2 81 B5, E2 81 B6, E2 81 B7, E2 81 B8 */
U4_1        \xE1[\x80-\xBF][\x80-\xBF]
U4_2        \xE2\x80[\x80-\xBF]
U4_3        \xE2\x81[\x80-\xAF]
U4_4        \xE2\x81[\xB1-\xB3]
U4_5        \xE2\x81[\xBA-\xBF]
U4          {U4_1}|{U4_2}|{U4_3}|{U4_4}|{U4_5}|[\xE3-\xEC][\x80-\xBF][\x80-\xBF]
U5          [\xED][\x80-\x9F][\x80-\xBF]
U6          [\xEE-\xEF][\x80-\xBF][\x80-\xBF]
U7          [\xF0][\x90-\xBF][\x80-\xBF][\x80-\xBF]
U8          [\xF1-\xF3][\x80-\xBF][\x80-\xBF][\x80-\xBF]
U9          [\xF4][\x80-\x8F][\x80-\xBF][\x80-\xBF]

UNICODE     {ASCII}|{U2}|{U3}|{U4}|{U5}|{U6}|{U7}|{U8}|{U9}
/* Adding [a-zA-Z] at the end might seem superfluous, but it isn't (why? */
LASCII      [\x0-\xFF]{-}[[:blank:]]{-}[[:cntrl:]]{-}[[:digit:]]{-}[[:graph:]]{-}[[:print:]]{-}[[:punct:]]{-}[[:space:]]{-}[⁰¹²³⁴⁵⁶⁷⁸⁹]{+}[a-zA-Z]
LUNICODE    {LASCII}|{U2}|{U3}|{U4}|{U5}|{U6}|{U7}|{U8}|{U9}

LETTER  [a-zA-Z]
ULETTER {LUNICODE}
DIGIT   [0-9]
NAME    {ULETTER}+
/* [] brackets are only for indexing matrices, {} brackets are for grouping! */
SEPARATOR [*"=<&;{[}\]#]
BLANK     [ \t]

/* A subscript is enclosed in {} and can contain everything except closing brackets.
   TODO: This definition should correspond with what starmath allows in a subscript!!!
   For example, "a_3 rsub 3" is illegal (double right subscript) or "a rsup 3 rsup 4" */
SUBSCRIPT _\{[^}]*\}|_({ULETTER}|{DIGIT})+
SUBPSCRIPT {BLANK}+([lcr]su[bp])(\{[^}]*\}|{BLANK}+{NAME}|{BLANK}+{DIGIT}+)
SUBSUPERSCRIPT {SUBSCRIPT}{SUBPSCRIPT}*|{SUBPSCRIPT}
IDENTIFIER \%?{NAME}{SUBSUPERSCRIPT}*

ATTRIBUTE acute|grave|check|breve|circle|vec|tilde|hat|bar|dot|widevec|widetilde|widehat|ddot|overline|underline|overstrike|dddot|bold|ital|font\ sans|font\ serif|font\ fixed|color\ black|color\ blue|color\ green|color\ red|color\ cyan|color\ magenta|color\ yellow|color\ teal|color\ silver|color\ gray|color\ maroon|color\ purple|color\ lime|color\ olive|color\ navy|color\ aqua|color\ fuchsia|phantom


/* The following paragraph suffices to track locations accurately.  Each
   time `yylex' is invoked, the begin position is moved onto the end
   position.  Then when a pattern is matched, the end position is advanced
   of its width.  In case it matched ends of lines, the end cursor is
   adjusted, and each time blanks are matched, the begin cursor is moved
   onto the end cursor to effectively ignore the blanks preceding tokens.
   Comments would be treated equally.
*/
%{
  #define YY_USER_ACTION  yylloc->columns(yyleng);
%}

/* define iMath parsing states */
%x II
%x SCANSTR
%x SCANUSERSTR
%x SCANCOMMENT
%x SCANUSERTEXT
%x SCANSUPERSCRIPT
%x END_OF_INPUT

%%
%{
  yylloc->step ();
%}
BEGIN(INITIAL);

{UTF8_BYTE_ORDER_MARK}

%{
  // Beginning and end -----------------------------------------
%}
^{BLANK}*\%\%ii  { // iFormula marker
          BEGIN(II);
          implicit_mul_might_follow = false;
        }
<END_OF_INPUT><<EOF>>  {
          BEGIN(INITIAL);
          implicit_mul_might_follow = false;
          last_token = token_type('\n');
          return token::ENDSTRING;
        }

%{
  // auto-generated lines
%}
^[^\n]*\%\%gg[^\n]*$ { // auto-generated line (discard)
         MSG_INFO(4, "Found generated line" << endline);
         return (token::GENERATED);
        }

%{
  // Arbitrary user text -----------------------------------------
%}
^{BLANK}*\%\%   { // user comment
         MSG_INFO(4,  "Found comment line" << endline);
         BEGIN(SCANCOMMENT);
         yylval->str = new std::string("%%");
       }
<SCANCOMMENT>[^\n]*$ { // scan to end of line. Note we cannot combine the two rules for comments because that would also match lines beginning with %%ii
          *(yylval->str) += std::string(yytext);
          BEGIN(INITIAL);
          return (token::COMMENT);
        }
\n      { MSG_INFO(4, "Found \\n" << endline);
          if (include_level > 0) yylloc->lines(1); // line count for include files
          implicit_mul_might_follow = false;
          last_token = token_type('\n');
          return token_type('\n');
        }
.       { MSG_INFO(4,  "Found user text" << endline); // arbitrary user text
          yylloc->columns(-yyleng);
          yyless(0); // Scan again in SCANUSERTEXT state
          yylval->str = new std::string();
          BEGIN(SCANUSERTEXT);
        }
<SCANUSERTEXT>newline { // user-defined newline
          MSG_INFO(4,  "newline found" << endline);
          if ((yylval->str != NULL) && (*(yylval->str) != "")) {
            yylloc->columns(-yyleng);
            yyless(0);
            return (token::STRING);
          } else {
            yylval->str = new std::string(); //prepare str for next part of usertext
            return (token::NEWLINE);
          }
        }
<SCANUSERTEXT>\\\"    { // escaped quotes are OK
          *(yylval->str) += std::string(yytext);
        }
<SCANUSERTEXT>\n     { // end of user text
          MSG_INFO(4,  "\\n found" << endline);
          if ((yylval->str != NULL) && (*(yylval->str) != "")) {
            yylloc->columns(-yyleng);
            yyless(0);
            return (token::STRING);
          } else {
            BEGIN(INITIAL);
            if (include_level > 0) yylloc->lines(1); // line count for include files
            implicit_mul_might_follow = false;
            last_token = token_type('\n');
            return token_type('\n');
          }
         }
<SCANUSERTEXT><<EOF>>    { MSG_INFO(4,  "EOF found" << endline); // end of user text
          if ((yylval->str != NULL) && (*(yylval->str) != "")) {
            BEGIN(END_OF_INPUT);
            return (token::STRING);
          } else {
            BEGIN(INITIAL);
            implicit_mul_might_follow = false;
            last_token = token_type('\n');
            return token::ENDSTRING;
          }
         }
<SCANUSERTEXT>=|<>|>|<|>=|<=|EQUIV|equiv {
          MSG_INFO(4,  "Found operator sign '" << yytext << "'" << endline); // equal sign
          if ((yylval->str != NULL) && (*(yylval->str) != "")) {
            MSG_INFO(4,  "Returning accumulated string '"  << *(yylval->str) << "'" << endline);
            yylloc->columns(-yyleng);
            yyless(0);
            return (token::STRING);
          } else {
            yylval->str = new std::string(yytext);
            BEGIN(INITIAL);
            return token::OPERATOR;
          }
        }
<SCANUSERTEXT>_ii_ {
          if ((yylval->str != NULL) && (*(yylval->str) != "")) {
            yylloc->columns(-yyleng);
            yyless(0);
            return (token::STRING);
          } else {
            BEGIN(II);
            implicit_mul_might_follow = false;
            MSG_INFO(1, "Found magic token in usertext" << endline);
            return (token::MAGIC);
          }
        }
<SCANUSERTEXT>\" { // begin string inside usertext (important because equal signs in here are not aligned)
          if ((yylval->str != NULL) && (*(yylval->str) != "")) {
            yylloc->columns(-yyleng);
            yyless(0);
            return (token::STRING);
          } else {
            BEGIN(SCANUSERSTR);
            yylval->str = new std::string("\"");
          }
        }
<SCANUSERSTR>\\\" { // allow escaped quotes
                 *(yylval->str) += "\\\"";
               }
<SCANUSERSTR>[^"\n] { // If we add '*' to the rule then the preceding rule will never be matched!
                 *(yylval->str) += std::string(yytext);
               }
<SCANUSERSTR>\"   { // end string
                 *(yylval->str) += "\"";
                 BEGIN(INITIAL);
                 return (token::STRING);
                 // Problem: SCANUSERTEXT expects yylval->str = new std::string() !
              }
<SCANUSERTEXT>. { MSG_INFO(4, "'" << yytext << "' found" << endline);// any other text
                 *(yylval->str) += std::string(yytext);
                }

%{
// iMath file operations -----------------------------------------
%}
<II>readfile|READFILE { return (token::READFILE);  }

%{
// iMath options
%}
<II>options|OPTIONS { return (token::OPTIONS); }

%{
// iMath namespaces
%}
<II>BEGIN|begin { implicit_mul_might_follow = false; return (token::BEGIN_NS); }
<II>END|end { implicit_mul_might_follow = false; return (token::END_NS); }

%{
// iMath declarations ----------------------------------
%}
<II>FUNCTION|function   { implicit_mul_might_follow = false; return (token::FUNCTION); }
<II>UNITDEF|unitdef   { implicit_mul_might_follow = false; return (token::UNITDEF); }
<II>PREFIXDEF|prefixdef   { implicit_mul_might_follow = false; return (token::PREFIXDEF); }
<II>VECTORDEF|vectordef   { implicit_mul_might_follow = false; return (token::VECTORDEF); }
<II>MATRIXDEF|matrixdef   { implicit_mul_might_follow = false; return (token::MATRIXDEF); }
<II>REALVARDEF|realvardef { implicit_mul_might_follow = false; return (token::REALVARDEF); }
<II>POSVARDEF|posvardef { implicit_mul_might_follow = false; return (token::POSVARDEF); }
<II>CLEAREQUATIONS|clearequations { return (token::CLEAREQUATIONS); }
<II>DELETE|delete   { return (token::DELETE); }
<II>UPDATE|update { return (token::UPDATE); }
%{
// iMath definitions
%}
<II>EXDEF|exdef  { implicit_mul_might_follow = false; return (token::EXDEF); }
<II>EQDEF|eqdef { implicit_mul_might_follow = false; return (token::EQDEF); }
<II>CONSTDEF|constdef { implicit_mul_might_follow = false; return (token::CONSTDEF); }
<II>FUNCDEF|funcdef { implicit_mul_might_follow = false; return (token::FUNCDEF); }
%{
// Extended printing
%}
<II>PRINTVAL|printval|PRINTAVAL|printaval {
              implicit_mul_might_follow = false;
              std::string kw = std::string(yytext);
              std::transform(kw.begin(), kw.end(), kw.begin(), ::toupper);
              yylval->str = new std::string(kw);
              return (token::PRINTVALUE);
            }
<II>PRINTVALWITH|printvalwith|PRINTAVALWITH|printavalwith {
              implicit_mul_might_follow = false;
              std::string kw = std::string(yytext);
              std::transform(kw.begin(), kw.end(), kw.begin(), ::toupper);
              yylval->str = new std::string(kw);
              return (token::PRINTVALUEWITH);
            }
<II>EXPLAINVAL|explainval { implicit_mul_might_follow = false; return (token::EXPLAINVAL); }
%{
// Graphics and text
%}
<II>TEXT[ ]|text[ ]|TEXT|text {
              implicit_mul_might_follow = false;
              yylval->str = new std::string();
              BEGIN(SCANUSERTEXT);
              return (token::TEXT);
            }
<II>CHART|chart { implicit_mul_might_follow = false; return (token::CHART); }
%{
// Data exchange
%}
<II>SETTABLECELL|settablecell { implicit_mul_might_follow = false; return (token::SETTABLECELL); }
<II>SETCALCCELLS|setcalccells { implicit_mul_might_follow = false; return (token::SETCALCCELLS); }
%{
// Names for key-value pairs
%}
<II>units          yylval->opt = o_units;        return(token::OPT_L);
<II>precision      yylval->opt = o_precision;    return(token::OPT_U);
<II>fixedpoint     yylval->opt = o_fixeddigits;  return(token::OPT_B);
<II>suppress_units yylval->opt = o_suppress_units; return(token::OPT_B);
<II>showlabels     yylval->opt = o_showlabels;   return(token::OPT_B);
<II>underflow      yylval->opt = o_underflow;    return(token::OPT_B);
<II>realroots      yylval->opt = o_evalf_real_roots; return(token::OPT_B);
<II>exponent       yylval->opt = o_exponent;     return(token::OPT_I);
<II>difftype       yylval->opt = o_difftype;     return(token::OPT_S);
<II>autoformat     yylval->opt = o_eqraw;        return(token::OPT_B);
<II>implicitmul    yylval->opt = o_implicitmul;  return(token::OPT_B);
<II>autoalign      yylval->opt = o_eqalign;      return(token::OPT_B);
<II>autochain      yylval->opt = o_eqchain;      return(token::OPT_B);
<II>autofraction   yylval->opt = o_autofraction; return(token::OPT_B);
<II>minimumtextsize   yylval->opt = o_minimumtextsize; return(token::OPT_U);
<II>autotextmode   yylval->opt = o_autotextmode; return(token::OPT_B);
<II>echo           yylval->opt = o_echoformula;  return(token::OPT_B);
<II>lowsclimit     yylval->opt = o_lowsclimit;   return(token::OPT_I);
<II>highsclimit    yylval->opt = o_highsclimit;  return(token::OPT_I);

%{/*<II>eqsplit        yylval->opt = o_eqsplit;      return(token::OPT_E);
<II>eqsplittext    yylval->opt = o_eqsplittext;  return(token::OPT_S);
<II>vecautosize    yylval->opt = o_vecautosize;  return(token::OPT_E);
<II>save           return(SAVE);
<II>restore        return(RESTORE);*/
%}

%{
// smath operators ------------------------------------------------
// These must come BEFORE the symbols etc., otherwise they will not be matched
%}
<II>left { /* Opening sized brackets */
        CHECK_IMPLICIT_MUL(token::BOPEN)
        if (checkresult) return (token::IMPMUL);
        implicit_mul_might_follow = false;
        yylval->str = new std::string(yytext);
        last_token = token::LEFT;
        return (token::LEFT);
      }
<II>right { /* Closing sized brackets */
        implicit_mul_might_follow = false;
        yylval->str = new std::string(yytext);
        last_token = token::RIGHT;
        return (token::RIGHT);
      }
<II>stack|STACK {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::STACK;
              return (token::STACK);
            }
<II>matrix|MATRIX {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::MATRIX;
              return (token::MATRIX);
            }
<II>over|OVER|div|DIV|wideslash|WIDESLASH|widebslash|WIDEBSLASH|÷|∕|⁄|⧸ { /* U+00F7 U+2215 U+2044 U+29F8 */
              implicit_mul_might_follow = false;
              return token_type('/'); /* Treat as division symbol */
            }
<II>cdot|CDOT|·|∙ { /* U+00B7 U+2219 */
              implicit_mul_might_follow = false;
              return token_type('*'); /* Treat as multiplication symbol */
           }
<II>times|TIMES|×  { /* U+00D7 */
              implicit_mul_might_follow = false;
              return (token::TIMES);
           }
<II>[⁰¹²³⁴⁵⁶⁷⁸⁹] {
             BEGIN(SCANSUPERSCRIPT);
             yylval->str = new std::string();
             yylloc->columns(-yyleng);
             yyless(0);
           }
<SCANSUPERSCRIPT>⁰ { yylval->str->append("0"); }
<SCANSUPERSCRIPT>¹ { yylval->str->append("1"); }
<SCANSUPERSCRIPT>² { yylval->str->append("2"); }
<SCANSUPERSCRIPT>³ { yylval->str->append("3"); }
<SCANSUPERSCRIPT>⁴ { yylval->str->append("4"); }
<SCANSUPERSCRIPT>⁵ { yylval->str->append("5"); }
<SCANSUPERSCRIPT>⁶ { yylval->str->append("6"); }
<SCANSUPERSCRIPT>⁷ { yylval->str->append("7"); }
<SCANSUPERSCRIPT>⁸ { yylval->str->append("8"); }
<SCANSUPERSCRIPT>⁹ { yylval->str->append("9"); }
<SCANSUPERSCRIPT>.|\n {
             implicit_mul_might_follow = (*yytext != '\n');
             yylloc->columns(-yyleng);
             yyless(0);
             BEGIN(II);
             last_token = token::SUPERSCRIPT;
             return (token::SUPERSCRIPT);
           }
<SCANSUPERSCRIPT><<EOF>> {
             BEGIN(END_OF_INPUT);
             return (token::SUPERSCRIPT);
           }
<II>⁺      { implicit_mul_might_follow = false; return token::SUPERPLUS; } /* U+207A */
<II>⁻      { implicit_mul_might_follow = false; return token::SUPERMINUS; } /* U+207B */
<II>\.\*|⊗|otimes|OTIMES|odot|ODOT { /* U+2297 */
             implicit_mul_might_follow = false;
             return (token::HPRODUCT);
           }
<II>\.\/|⊘|odivide|ODIVIDE { /* U+2298 */
             implicit_mul_might_follow = false;
             return (token::HDIVISION);
           }
<II>\.\^|ⓔ { /* U+24D4 */
             implicit_mul_might_follow = false;
             return (token::HPOWER);
           }
<II>-|‒|–|—  { /* U+2012 U+2013 U+2014 */
              implicit_mul_might_follow = false;
              return token_type('-'); /* Treat as minus sign */
            }
<II>-\+     { implicit_mul_might_follow = false;
              return (token::MINUSPLUS);
            }
<II>\+-     { implicit_mul_might_follow = false;
              return (token::PLUSMINUS);
            }
<II>nroot|NROOT {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::NROOT;
              return (token::NROOT);
           }
<II>quo|QUO { CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::QUO;
              return (token::QUO);
           }
<II>rem|REM { CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::REM;
              return (token::REM);
           }
<II>gcd|GCD { CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::GCD;
              return (token::GCD);
           }
<II>lcm|LCM { CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::LCM;
              return (token::LCM);
           }
<II>and|AND { implicit_mul_might_follow = false;
              last_token = token::AND;
              return (token::AND);
            }
<II>or|OR { implicit_mul_might_follow = false;
              last_token = token::OR;
              return (token::OR);
            }
<II>neg|NEG { implicit_mul_might_follow = false;
              last_token = token::NEG;
              return (token::NEG);
            }
<II>EQUIV|equiv {
              implicit_mul_might_follow = false;
              last_token = token::EQUIV;
              return (token::EQUIV);
            }
<II>MOD|mod|\"MOD\"|\"mod\" {
              implicit_mul_might_follow = false;
              last_token = token::MOD;
              return (token::MOD);
            }
<II>\({BLANK}*MOD|\({BLANK}*mod|\({BLANK}*\"MOD\"|\({BLANK}*\"mod\" {
              implicit_mul_might_follow = false;
              last_token = token::BMOD;
              return (token::BMOD);
            }
<II>from|FROM { implicit_mul_might_follow = false;
              return (token::FROM); /* sum from .. to .. */
           }
<II>to|TO { implicit_mul_might_follow = false;
              return (token::TO); /* sum from .. to .. */
           }
<II>lhs|LHS {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::LHS;
              yylval->str = new std::string("LHS");
              return (token::LHS);
            }
<II>rhs|RHS {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::RHS;
              yylval->str = new std::string("RHS"); // not really necessary because smathparser.yxx tests for "LHS" only
              return (token::RHS);
            }
<II>rev|REV {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::REVERSE;
              return (token::REVERSE);
            }
<II>wild    {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::WILD;
              return (token::WILD);
            }
<II>val|VAL|aval|AVAL|numval|NUMVAL|unit|UNIT|quantity|QUANTITY {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::VALUE;
              std::string kw = std::string(yytext);
              std::transform(kw.begin(), kw.end(), kw.begin(), ::toupper);
              yylval->str = new std::string(kw);
              return (token::VALUE);
            }
<II>valwith|VALWITH|avalwith|AVALWITH|numvalwith|NUMVALWITH|unitwith|UNITWITH|quantitywith|QUANTITYWITH {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::VALUEWITH;
              std::string kw = std::string(yytext);
              std::transform(kw.begin(), kw.end(), kw.begin(), ::toupper);
              yylval->str = new std::string(kw);
              return (token::VALUEWITH);
            }
<II>iterate|ITERATE {
              CHECK_IMPLICIT_MUL(token::FUNC)
              if (checkresult) return (token::IMPMUL);
              implicit_mul_might_follow = false;
              last_token = token::ITERATE;
              return (token::ITERATE);
            }
<II>solve|SOLVE {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::SOLVE;
             return (token::SOLVE);
           }
<II>subst|SUBST {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::SUBST;
             return (token::SUBST);
           }
<II>substc|SUBSTC {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::SUBST;
             return (token::SUBSTC);
           }
<II>substv|SUBSTV {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::SUBSTV;
             return (token::SUBSTV);
           }
<II>substvc|SUBSTVC {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::SUBSTVC;
             return (token::SUBSTVC);
           }
<II>simplify|SIMPLIFY {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::SIMPLIFY;
             return (token::SIMPLIFY);
           }
<II>collect|COLLECT {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::COLLECT;
             return (token::COLLECT);
           }
<II>differentiate|DIFFERENTIATE {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::DIFFERENTIATE;
             return (token::DIFFERENTIATE);
           }
<II>pdifferentiate|PDIFFERENTIATE {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::PDIFFERENTIATE;
             return (token::PDIFFERENTIATE);
           }
<II>integrate|INTEGRATE {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::INTEGRATE;
             return (token::INTEGRATE);
           }
<II>tseries|TSERIES {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::TSERIES;
             return (token::TSERIES);
           }
%{
// smath formatting -------------------------------------------------------
%}
<II>newline { /* TODO: Can or should this occur at all ? */ }
<II>size|SIZE {
             CHECK_IMPLICIT_MUL(token::FUNC)
             if (checkresult) return (token::IMPMUL);
             implicit_mul_might_follow = false;
             last_token = token::SIZE;
             return (token::SIZE);
           }
<II>alignc|ALIGNC|alignl|ALIGNL|alignr|ALIGNR { /* skip */ }
<II>\~|` { /* skip */ }
<II>func|FUNC { /* Not handled yet */ }
%{
// ------------------------------------------------------
// Brackets
// Note: Extended brackets must come before names, otherwise they will not be recognized
%}
<II>[\{\(\[] { /* Normal opening brackets */
        if (((last_token == token::MSYMBOL) || (last_token == token::VSYMBOL)) && (yytext[0] == '[')) {
          checkresult = false; // Special case for matrix indexing
        } else {
          CHECK_IMPLICIT_MUL(token::BOPEN)
        }
        if (checkresult) return (token::IMPMUL);
        implicit_mul_might_follow = false;
        last_token = token::BOPEN;
        return (token_type(yytext[0]));
      }
<II>[\}\)\]] { /* Normal closing brackets */
        implicit_mul_might_follow = true;
        last_token = token::BCLOSE;
        return (token_type(yytext[0]));
      }
<II>\\\{|ldbracket|lline|ldline|lbrace|langle { /* Extended opening brackets */
        CHECK_IMPLICIT_MUL(token::BOPEN)
        if (checkresult) return (token::IMPMUL);
        implicit_mul_might_follow = false;
        last_token = token::BOPEN;
        yylval->str = new std::string(yytext);
        return (token::BOPEN);
      }
<II>\\\}|rdbracket|rline|rdline|rbrace|rangle { /* Extended closing brackets */
        implicit_mul_might_follow = true;
        last_token = token::BCLOSE;
        yylval->str = new std::string(yytext);
        return (token::BCLOSE);
      }
%{
// ------------------------------------------------------
// normal symbols, unit or function names, incomplete subscripts
%}
<II>true  { yylval->boolean = true; return token::BOOL; }
<II>false { yylval->boolean = false; return token::BOOL; }
<II>@prev@ {
        // equation label referencing the previous equation
        CHECK_IMPLICIT_MUL(token::LABEL)
        if (checkresult) return (token::IMPMUL);
        implicit_mul_might_follow = true;
        last_token = token::LABEL;
        yylval->str = new std::string("prev");
        return (token::LABEL);
      }
<II>@prev[[:digit:]]+@ {
        // equation label referencing the nth previous equation
        CHECK_IMPLICIT_MUL(token::LABEL)
        if (checkresult) return (token::IMPMUL);
        implicit_mul_might_follow = true;
        last_token = token::LABEL;
        std::string label(yytext);

        try {
          yylval->str = new std::string(compiler->getPreviousEquationLabel(std::stoi(label.substr(5, label.size() - 6))));
          if (yylval->str->empty()) {
            delete yylval->str;
            yylval->str = new std::string(yytext); // nth previous equation does not exist
          }
        } catch (std::exception& e) {
          (void)e;
          yylval->str = new std::string(yytext); // Invalid string for conversion to integer
        }

        return (token::LABEL);
      }
<II>@[^@]*@ {
        // an equation label
        std::string* label = new std::string(yytext);
        label->erase(0,1);
        label->erase(label->length()-1, 1);
        auto toktype = (compiler->is_expression_label(compiler->exlabel_ns(*label, true)) ? token::EXLABEL : token::LABEL);
        CHECK_IMPLICIT_MUL(toktype)
        if (checkresult) return (token::IMPMUL);
        implicit_mul_might_follow = true;
        last_token = toktype;
        yylval->str = label;
        // must_autoformat is set in smathparser
        return (toktype);
      }

<II>SUM{BLANK}*FROM|sum{BLANK}*from {
         CHECK_IMPLICIT_MUL(token::FUNC)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_token = token::FUNC;
         return (token::SUMFROM);
       }
<II>PRODUCT|product {
         CHECK_IMPLICIT_MUL(token::FUNC)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_token = token::FUNC;
         return (token::PRODUCT);
       }
<II>INT|int {
         CHECK_IMPLICIT_MUL(token::FUNC)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_token = token::FUNC;
         return (token::INTEGRAL);
       }
<II>DIFFERENTIAL|differential|PARTIAL|partial {
         CHECK_IMPLICIT_MUL(token::FUNC)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_token = token::FUNC;
         std::string kw = std::string(yytext);
         std::transform(kw.begin(), kw.end(), kw.begin(), ::toupper);
         yylval->str = new std::string(kw);
         return (token::DIFFERENTIAL);
       }
<II>TEXTFIELD|textfield {
         CHECK_IMPLICIT_MUL(token::FUNC)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_token = token::FUNC;
         return (token::TEXTFIELD);
       }
<II>TABLECELL|tablecell {
         CHECK_IMPLICIT_MUL(token::FUNC)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_token = token::FUNC;
         return (token::TABLECELL);
       }
<II>CALCCELL|calccell {
         CHECK_IMPLICIT_MUL(token::FUNC)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_token = token::FUNC;
         return (token::CALCCELL);
       }

<II>{ATTRIBUTE} {
         CHECK_IMPLICIT_MUL(token::IDENTIFIER)
         if (checkresult) return (token::IMPMUL);
         implicit_mul_might_follow = false;
         last_attribute = last_attribute + (last_attribute.empty() ? "" : " ") + yytext;
         last_token = token::ATTRIBUTE;
         // don't return the attribute because it must be concatenated with the identifier
       }
<II>{IDENTIFIER}|nospace\{{IDENTIFIER}{BLANK}+{IDENTIFIER}\}|{IDENTIFIER}::{IDENTIFIER}|{IDENTIFIER}::nospace\{{IDENTIFIER}{BLANK}+{IDENTIFIER}\}|::{IDENTIFIER}|::nospace\{{IDENTIFIER}{BLANK}+{IDENTIFIER}\} {
         // Note: f \rhs{"prev"} does not mean f * rhs{"prev"}, but f(\rhs{"prev"})
         CHECK_IMPLICIT_MUL(token::IDENTIFIER)
         if (checkresult) return (token::IMPMUL);
         std::string name(yytext);

         MSG_INFO(4,  "Found string "  << name << endline);

         if ((name[0] == '%') && (name.length() > 1) && compiler->unitmgr.isUnit(name.substr(1))) {
           // The name is a unit
           name.erase(0,1);
           implicit_mul_might_follow = true;
           last_token = token::UNIT;
           yylval->str = new std::string(name);
           return (token::UNIT);
         }

         if (last_token == token::ATTRIBUTE) // Concatenate the attribute with the name
           name = "{" + last_attribute + " " + name + "}";
         last_attribute = ""; // Clear attribute

         yylval->str = new std::string(name);
         name = compiler->varname_ns(name);

         if (compiler->getsymtype(name) == t_function) {
           MSG_INFO(4,  "Found existing function " << name << endline);
           // Note that the function might or might not be followed by an argument!
           // The case FUNC BOPEN was handled in CHECK_IMPLICIT_MUL
           if (last_token == token::FUNC) {
                // Check a second time to handle the case FUNC FUNC
                CHECK_IMPLICIT_MUL(token::FUNC)
                if (checkresult) return (token::IMPMUL);
           }
           last_token = token::FUNC;
           *(yylval->str) = name; // Handle namespace in function name
         } else if (compiler->getsymprop(name) == p_vector) {
           MSG_INFO(4,  "Found existing vector variable " << name << endline);
           last_token = token::VSYMBOL;
           yylval->expr = compiler->getsymp(name);
         } else if (compiler->getsymprop(name) == p_matrix) {
           MSG_INFO(4,  "Found existing matrix variable " << name << endline);
           last_token = token::MSYMBOL;
           yylval->expr = compiler->getsymp(name);
         } else if (compiler->getsymtype(name) == t_variable) {
           MSG_INFO(4,  "Found existing variable " << name << endline);
           last_token = token::SYMBOL;
           yylval->expr = compiler->getsymp(name);
         } else if (compiler->getsymtype(name) == t_constant) {
           MSG_INFO(4,  "Found existing constant " << name << endline);
           last_token = token::SYMBOL;
           yylval->expr = compiler->getsymp(name);
         } else {
           MSG_INFO(4,  "Found new identifier " << name << endline);
           last_token = token::IDENTIFIER;
         }

         implicit_mul_might_follow = true;
         return (last_token);
       }
%{
// ------------------------------------------------------
// Numbers, strings
%}
<II>{DIGIT}+ { CHECK_IMPLICIT_MUL(token::DIGITS)
          if (checkresult) return (token::IMPMUL);
          implicit_mul_might_follow = true;
          last_token = token::DIGITS;
          yylval->str = new std::string(yytext);
          return (token::DIGITS);
        }
<II>\"   { // begin string
          BEGIN(SCANSTR);
          yylval->str = new std::string();
        }
<SCANSTR>\\\"   { // allow escaped quotes
                 *(yylval->str) += "\"";
               }
<SCANSTR>[^"\n] { // Note: If we add '*' to the rule then the preceding rule will never be matched
                 *(yylval->str) += std::string(yytext);
               }
<SCANSTR>\"    { // end string. This might also be a UNIT, therefore we do the IMPMUL checks
                 CHECK_IMPLICIT_MUL(token::STRING)
                 if (checkresult) return (token::IMPMUL);
                 BEGIN(II);
                 implicit_mul_might_follow = true;
                 last_token = token::STRING;
                 return (token::STRING);
                }

%{
// ------------------------------------------------------
// One-letter tokens and the rest
%}
<II>\%\%.*$ { // Comment in a line starting with %%ii
              BEGIN(INITIAL);
              yylval->str = new std::string(yytext);
              return (token::COMMENT);
            }
<II>\n%%ii\+ { /* extended formula */ }
<II>\n   { // smath formulas cannot extend over line breaks
              if (include_level > 0) yylloc->lines (1); // line count for include files
              implicit_mul_might_follow = false;
              last_token = token_type(' ');
              BEGIN(INITIAL);
              return (token_type('\n'));
            }
<II>{BLANK}+    {
              // Eat whitespace
              if (last_token == token::DIGITS) // for cases like 5 1/%s
                last_token = token_type(' ');
            }
<II>##      { // Matrix row separator
              implicit_mul_might_follow = false;
              last_token = token::DOUBLEHASH;
              return (token::DOUBLEHASH);
            }
<II>\^T     { // Matrix or vector transposition
              implicit_mul_might_follow = true;
              last_token = token::TRANSPOSE;
              return (token::TRANSPOSE);
            }
<II>!=|<>   {
              implicit_mul_might_follow = false;
              last_token = token::NEQ;
              return (token::NEQ);
            }

<II>{SEPARATOR} {
              MSG_INFO(4,  "Found separator: " << yytext << endline);
              implicit_mul_might_follow = false;
              last_token = token_type(*yytext);
              return (token_type(*yytext));
            }
<II>.           { /* everything else */
              MSG_INFO(4,  "Found token: '" << yytext << "'" << endline);
              if (yytext[0] == '!')
                implicit_mul_might_follow = true; // factorial
              else
                implicit_mul_might_follow = false;
              // register the last token for correct parsing of x^y_n -> (x_n)^y, not x^(y_n)
              last_token = token_type(*yytext);
              return token_type(*yytext);
            }
<II>_ii_ {
              MSG_INFO(1, "Found magic token in iFormula" << endline);
              yylval->str = new std::string();
              BEGIN(SCANUSERTEXT);
              return (token::MAGIC);
            }
<II><<EOF>> {
              implicit_mul_might_follow = false;
              last_token = token_type('\n');
              BEGIN(INITIAL);
              return token::ENDSTRING;
            }
%%

// -------------------------------------------------------------
// Because the scanner related driver's member function depend on
// the scanner's data, it is simpler to implement them in this file.
namespace smathlexer {
  void scan_begin(const std::string& input) {
    // Note: Apparently yypush/yypop don't work with yy_scan_string
    static YY_BUFFER_STATE scan_buffer; // This is required for scanning to work!
    implicit_mul_might_follow = false;
    last_token = imath::smathparser::token_type(' ');
    BEGIN(INITIAL);
    //yy_flex_debug = trace_scanning;
    scan_buffer = yy_scan_string(input.c_str());
  } // smathlexer::scan_begin()

  void scan_end() {
    // Note: Apparently yypush/yypop don't work with yy_scan_string
    static YY_BUFFER_STATE scan_buffer;
    if (YY_CURRENT_BUFFER != NULL)
      yy_delete_buffer(scan_buffer); // This is required for scanning to work!
    BEGIN(INITIAL);
  } // smathlexer::scan_end()

  bool begin_include(const std::string &fname) {
    YY_BUFFER_STATE new_buffer;
    // TODO: Must we clean up the old yyin before re-assigning it?
  #ifdef _MSC_VER
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wfname = converter.from_bytes(fname);
    errno_t err = _wfopen_s(&yyin, wfname.c_str(), L"r");
    MSG_INFO(1, "Trying to open Windows file " << fname << " with result " << err << endline);
  #else
    yyin = fopen(fname.c_str(), "r");
  #endif

    if (yyin == NULL) {
      return (false);
    } else {
      new_buffer = yy_create_buffer(yyin, YY_BUF_SIZE);
      yypush_buffer_state(new_buffer);
      implicit_mul_might_follow = false;
      last_token = imath::smathparser::token_type(' ');
      BEGIN(INITIAL);
      return (true);
    }
  } // smathlexer::begin_include()

  bool finish_include() {
    if (YY_CURRENT_BUFFER != NULL)
      yypop_buffer_state();
    implicit_mul_might_follow = false;
    last_token = imath::smathparser::token_type(' ');
    BEGIN(II); // We can't go to INITIAL because we still have to scan the \n or <<EOF>> after a READFILE command
    //infomsg("Ending include file");
    return (YY_CURRENT_BUFFER == NULL);
  } // smathlexer::finish_include()
}


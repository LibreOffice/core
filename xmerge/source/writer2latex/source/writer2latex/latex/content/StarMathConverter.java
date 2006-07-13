/************************************************************************
 *
 *  StarMathConverter.java
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3i (2004-12-28)
 *
 */

package writer2latex.latex.content;

import java.util.Hashtable;
import writer2latex.util.*;
import writer2latex.latex.style.I18n;
import writer2latex.latex.LaTeXDocumentPortion;

/* TO DO:
 * better font handling, currently nested font attributes (eg. italic font fixed) don't work
 * text inherits font from text surrounding formula - should this be changed?
 * implement implicit left alignment of expressions starting with text
 * improve subsup: \sideset should possibly be replaced by \multiscripts
 *   \multiscripts should only be used when absolutely neccessary
 *   maybe \leftidx should also be used sometimes
 * alignment: how to make stack{{alignr x} % xxx} work??
 * alignment: fractions and binoms?
 * ...
 */

//////////////////////////////////////////////////////////////////
/* The converted formula requires LaTeX2e with the packages
 * amsmath.sty, amssymb.sty, amsfonts.sty, color.sty.
 * Also the following control sequences must be defined for starmath symbols
 * and constructions that are not provided by standard LaTeX2e+amsmath:
 * \defeq, \lambdabar, \ddotsup, \multimapdotbothA, \multimapdotbothB,
 * \llbracket, \rrbracket, \oiint, \oiiint,
 * \mathoverstrike, \wideslash, \widebslash, \boldsubformula,
 * \normalsubformula.
 * These should be declared in the preamble.
 * Some are defined in the packages stmaryrd.sty, pxfonts.sty, txfonts.sty,
 * but fallback definitions are provided in writer.sty if these packages are
 * not used. More info in the documentation on writer.sty.
 */

//////////////////////////////////////////////////////////////////
// Helper Classes

// Some Character classifications
class CharClasses{

    static final char[] aDelimiterTable =
    { ' ', '\t' , '\n', '\r', '+', '-', '*', '/', '=', '#',
      '%', '\\', '"', '~', '`', '>', '<', '&', '|', '(',
      ')', '{', '}', '[', ']', '^', '_', '\0'};

    static boolean isDelimiter(char cChar){
        // return true iff cChar is '\0' or a delimiter
        if (cChar=='\0') return false;
        int i=0;
        for (i=0;aDelimiterTable[i]!='\0';i++)
            if (aDelimiterTable[i]==cChar)
                break;
        return aDelimiterTable[i]!='\0';
    }

    static boolean isEndOrLineEnd(char cChar){
        switch (cChar){
            case '\0':
            case '\n':
            case '\r':
                return true;
            default:
                return false;
        }
    }

    static boolean isDigitOrDot(char cChar){
        return (cChar>='0' && cChar<='9') || cChar=='.' || cChar==',';
    }
}

// enumerate Tokens
class Token{
    public static final Token END=new Token();
    public static final Token LGROUP=new Token();
    public static final Token RGROUP=new Token();
    public static final Token LPARENT=new Token();
    public static final Token RPARENT=new Token();
    public static final Token LBRACKET=new Token();
    public static final Token RBRACKET=new Token();
    public static final Token PLUS=new Token();
    public static final Token MINUS=new Token();
    public static final Token MULTIPLY=new Token();
    public static final Token DIVIDEBY=new Token();
    public static final Token ASSIGN=new Token();
    public static final Token POUND=new Token();
    public static final Token SPECIAL=new Token();
    public static final Token SLASH=new Token();
    public static final Token BACKSLASH=new Token();
    public static final Token BLANK=new Token();
    public static final Token SBLANK=new Token();
    public static final Token RSUB=new Token();
    public static final Token RSUP=new Token();
    public static final Token CSUB=new Token();
    public static final Token CSUP=new Token();
    public static final Token LSUB=new Token();
    public static final Token LSUP=new Token();
    public static final Token GT=new Token();
    public static final Token LT=new Token();
    public static final Token AND=new Token();
    public static final Token OR=new Token();
    public static final Token INTERSECT=new Token();
    public static final Token UNION=new Token();
    public static final Token NEWLINE=new Token();
    public static final Token BINOM=new Token();
    public static final Token FROM=new Token();
    public static final Token TO=new Token();
    public static final Token INT=new Token();
    public static final Token SUM=new Token();
    public static final Token OPER=new Token();
    public static final Token ABS=new Token();
    public static final Token SQRT=new Token();
    public static final Token FACT=new Token();
    public static final Token NROOT=new Token();
    public static final Token OVER=new Token();
    public static final Token TIMES=new Token();
    public static final Token GE=new Token();
    public static final Token LE=new Token();
    public static final Token GG=new Token();
    public static final Token LL=new Token();
    public static final Token DOTSAXIS=new Token();
    public static final Token DOTSLOW=new Token();
    public static final Token DOTSVERT=new Token();
    public static final Token DOTSDIAG=new Token();
    public static final Token DOTSUP=new Token();
    public static final Token DOTSDOWN=new Token();
    public static final Token ACUTE=new Token();
    public static final Token BAR=new Token();
    public static final Token BREVE=new Token();
    public static final Token CHECK=new Token();
    public static final Token CIRCLE=new Token();
    public static final Token DOT=new Token();
    public static final Token DDOT=new Token();
    public static final Token DDDOT=new Token();
    public static final Token GRAVE=new Token();
    public static final Token HAT=new Token();
    public static final Token TILDE=new Token();
    public static final Token VEC=new Token();
    public static final Token UNDERLINE=new Token();
    public static final Token OVERLINE=new Token();
    public static final Token OVERSTRIKE=new Token();
    public static final Token ITALIC=new Token();
    public static final Token NITALIC=new Token();
    public static final Token BOLD=new Token();
    public static final Token NBOLD=new Token();
    public static final Token PHANTOM=new Token();
    public static final Token FONT=new Token();
    public static final Token SIZE=new Token();
    public static final Token COLOR=new Token();
    public static final Token ALIGNL=new Token();
    public static final Token ALIGNC=new Token();
    public static final Token ALIGNR=new Token();
    public static final Token LEFT=new Token();
    public static final Token RIGHT=new Token();
    public static final Token LANGLE=new Token();
    public static final Token LBRACE=new Token();
    public static final Token LLINE=new Token();
    public static final Token LDLINE=new Token();
    public static final Token LCEIL=new Token();
    public static final Token LFLOOR=new Token();
    public static final Token NONE=new Token();
    public static final Token MLINE=new Token();
    public static final Token RANGLE=new Token();
    public static final Token RBRACE=new Token();
    public static final Token RLINE=new Token();
    public static final Token RDLINE=new Token();
    public static final Token RCEIL=new Token();
    public static final Token RFLOOR=new Token();
    public static final Token SIN=new Token();
    public static final Token COS=new Token();
    public static final Token TAN=new Token();
    public static final Token COT=new Token();
    public static final Token FUNC=new Token();
    public static final Token STACK=new Token();
    public static final Token MATRIX=new Token();
    public static final Token DPOUND=new Token();
    public static final Token PLACE=new Token();
    public static final Token TEXT=new Token();
    public static final Token NUMBER=new Token();
    public static final Token CHARACTER=new Token();
    public static final Token IDENT=new Token();
    public static final Token NEQ=new Token();
    public static final Token EQUIV=new Token();
    public static final Token DEF=new Token();
    public static final Token PROP=new Token();
    public static final Token SIM=new Token();
    public static final Token SIMEQ=new Token();
    public static final Token APPROX=new Token();
    public static final Token PARALLEL=new Token();
    public static final Token ORTHO=new Token();
    public static final Token IN=new Token();
    public static final Token NOTIN=new Token();
    public static final Token SUBSET=new Token();
    public static final Token SUBSETEQ=new Token();
    public static final Token SUPSET=new Token();
    public static final Token SUPSETEQ=new Token();
    public static final Token PLUSMINUS=new Token();
    public static final Token MINUSPLUS=new Token();
    public static final Token OPLUS=new Token();
    public static final Token OMINUS=new Token();
    public static final Token DIV=new Token();
    public static final Token OTIMES=new Token();
    public static final Token ODIVIDE=new Token();
    public static final Token TRANSL=new Token();
    public static final Token TRANSR=new Token();
    public static final Token IINT=new Token();
    public static final Token IIINT=new Token();
    public static final Token LINT=new Token();
    public static final Token LLINT=new Token();
    public static final Token LLLINT=new Token();
    public static final Token PROD=new Token();
    public static final Token COPROD=new Token();
    public static final Token FORALL=new Token();
    public static final Token EXISTS=new Token();
    public static final Token LIM=new Token();
    public static final Token NABLA=new Token();
    public static final Token TOWARD=new Token();
    public static final Token SINH=new Token();
    public static final Token COSH=new Token();
    public static final Token TANH=new Token();
    public static final Token COTH=new Token();
    public static final Token ASIN=new Token();
    public static final Token ACOS=new Token();
    public static final Token ATAN=new Token();
    public static final Token LN=new Token();
    public static final Token LOG=new Token();
    public static final Token UOPER=new Token();
    public static final Token BOPER=new Token();
    public static final Token BLACK=new Token();
    public static final Token WHITE=new Token();
    public static final Token RED=new Token();
    public static final Token GREEN=new Token();
    public static final Token BLUE=new Token();
    public static final Token CYAN=new Token();
    public static final Token MAGENTA=new Token();
    public static final Token YELLOW=new Token();
    public static final Token FIXED=new Token();
    public static final Token SANS=new Token();
    public static final Token SERIF=new Token();
    public static final Token POINT=new Token();
    public static final Token ASINH=new Token();
    public static final Token ACOSH=new Token();
    public static final Token ATANH=new Token();
    public static final Token ACOTH=new Token();
    public static final Token ACOT=new Token();
    public static final Token EXP=new Token();
    public static final Token CDOT=new Token();
    public static final Token ODOT=new Token();
    public static final Token LESLANT=new Token();
    public static final Token GESLANT=new Token();
    public static final Token NSUBSET=new Token();
    public static final Token NSUPSET=new Token();
    public static final Token NSUBSETEQ=new Token();
    public static final Token NSUPSETEQ=new Token();
    public static final Token PARTIAL=new Token();
    public static final Token NEG=new Token();
    public static final Token NI=new Token();
    public static final Token BACKEPSILON=new Token();
    public static final Token ALEPH=new Token();
    public static final Token IM=new Token();
    public static final Token RE=new Token();
    public static final Token WP=new Token();
    public static final Token EMPTYSET=new Token();
    public static final Token INFINITY=new Token();
    public static final Token ESCAPE=new Token();
    public static final Token LIMSUP=new Token();
    public static final Token LIMINF=new Token();
    public static final Token NDIVIDES=new Token();
    public static final Token DRARROW=new Token();
    public static final Token DLARROW=new Token();
    public static final Token DLRARROW=new Token();
    public static final Token UNDERBRACE=new Token();
    public static final Token OVERBRACE=new Token();
    public static final Token CIRC=new Token();
    //public static final Token TOP=new Token();
    public static final Token HBAR=new Token();
    public static final Token LAMBDABAR=new Token();
    public static final Token LEFTARROW=new Token();
    public static final Token RIGHTARROW=new Token();
    public static final Token UPARROW=new Token();
    public static final Token DOWNARROW=new Token();
    public static final Token DIVIDES=new Token();
    public static final Token SETN=new Token();
    public static final Token SETZ=new Token();
    public static final Token SETQ=new Token();
    public static final Token SETR=new Token();
    public static final Token SETC=new Token();
    public static final Token WIDEVEC=new Token();
    public static final Token WIDETILDE=new Token();
    public static final Token WIDEHAT=new Token();
    public static final Token WIDESLASH=new Token();
    public static final Token WIDEBACKSLASH=new Token();
    public static final Token LDBRACKET=new Token();
    public static final Token RDBRACKET=new Token();
    public static final Token UNKNOWN=new Token();
}

// enumerate Token groups
class TGroup{
    public static final TGroup NONE=new TGroup();
    public static final TGroup OPER=new TGroup();
    public static final TGroup RELATION=new TGroup();
    public static final TGroup SUM=new TGroup();
    public static final TGroup PRODUCT=new TGroup();
    public static final TGroup UNOPER=new TGroup();
    public static final TGroup POWER =new TGroup();
    public static final TGroup ATTRIBUT=new TGroup();
    public static final TGroup ALIGN =new TGroup();
    public static final TGroup FUNCTION=new TGroup();
    public static final TGroup BLANK =new TGroup();
    public static final TGroup LBRACES=new TGroup();
    public static final TGroup RBRACES=new TGroup();
    public static final TGroup COLOR =new TGroup();
    public static final TGroup FONT=new TGroup();
    public static final TGroup STANDALONE=new TGroup();
    public static final TGroup LIMIT=new TGroup();
    public static final TGroup FONTATTR=new TGroup();
}

// Token tables
class SmTokenTableEntry{ // This is simply a struct
        String  sIdent;
        Token   eType;
        String  sLaTeX;
        TGroup  eGroup1, eGroup2;
        int     nLevel;

        SmTokenTableEntry(String sIdent, Token eType, String sLaTeX,
        TGroup eGroup1, TGroup eGroup2, int nLevel){
            this.sIdent=sIdent;
            this.eType=eType;
            this.sLaTeX=sLaTeX;
            this.eGroup1=eGroup1;
            this.eGroup2=eGroup2;
            this.nLevel=nLevel;
        }

        SmTokenTableEntry(String sIdent, Token eType, String sLaTeX,
        TGroup eGroup1, int nLevel){
            this(sIdent,eType,sLaTeX,eGroup1,TGroup.NONE,nLevel);
        }

        SmTokenTableEntry(String sIdent, Token eType, String sLaTeX,
        int nLevel){
            this(sIdent,eType,sLaTeX,TGroup.NONE,TGroup.NONE,nLevel);
        }
}

class SmTokenTable{
    private SmTokenTableEntry[] table;

    SmTokenTable(SmTokenTableEntry[] table){this.table=table;}

    boolean lookup(String sIdent, boolean bIgnoreCase, SmToken token){
        for (int i=0; i<table.length; i++){
            if (bIgnoreCase ? table[i].sIdent.equalsIgnoreCase(sIdent) : table[i].sIdent.equals(sIdent)){
                token.assign(table[i].eType, table[i].sLaTeX, table[i].eGroup1, table[i].eGroup2, table[i].nLevel);
                return true;
            }
        }
        return false;
    }

    static final SmTokenTableEntry[] keywords=
        {new SmTokenTableEntry( "Im",Token.IM, "\\Im ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "Re" , Token.RE, "\\Re ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "abs", Token.ABS, "", TGroup.UNOPER, 5),
         new SmTokenTableEntry( "arcosh", Token.ACOSH, "\\operatorname{arcosh}", TGroup.FUNCTION, 5 ),
         new SmTokenTableEntry( "arcoth", Token.ACOTH, "\\operatorname{arsinh}", TGroup.FUNCTION, 5 ),
         new SmTokenTableEntry( "acute", Token.ACUTE, "\\acute", TGroup.ATTRIBUT, 5 ),
         new SmTokenTableEntry( "aleph" , Token.ALEPH, "\\aleph ", TGroup.STANDALONE, 5 ),
         new SmTokenTableEntry( "alignc", Token.ALIGNC, "", TGroup.ALIGN, 0),
         new SmTokenTableEntry( "alignl", Token.ALIGNL, "", TGroup.ALIGN, 0),
         new SmTokenTableEntry( "alignr", Token.ALIGNR, "", TGroup.ALIGN, 0),
         new SmTokenTableEntry( "and", Token.AND, "\\wedge ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "approx", Token.APPROX, "\\approx ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "arccos", Token.ACOS, "\\arccos ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "arccot", Token.ACOT, "\\arccot ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "arcsin", Token.ASIN, "\\arcsin ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "arctan", Token.ATAN, "\\arctan ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "arsinh", Token.ASINH, "\\operatorname{arsinh}", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "artanh", Token.ATANH, "\\operatorname{artanh}", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "backepsilon" , Token.BACKEPSILON, "\\backepsilon ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "bar", Token.BAR, "\\bar", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "binom", Token.BINOM, "" , 5 ),
         new SmTokenTableEntry( "black", Token.BLACK, "black", TGroup.COLOR, 0),
         new SmTokenTableEntry( "blue", Token.BLUE, "blue", TGroup.COLOR, 0),
         new SmTokenTableEntry( "bold", Token.BOLD, "\\boldsubformula", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "boper", Token.BOPER, "", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "breve", Token.BREVE, "\\breve", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "bslash", Token.BACKSLASH, "\\backslash ", TGroup.PRODUCT, 0 ),
         new SmTokenTableEntry( "cdot", Token.CDOT, "\\cdot ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "check", Token.CHECK, "\\check", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "circ" , Token.CIRC, "\\circ ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "circle", Token.CIRCLE, "\\overset{\\circ}", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "color", Token.COLOR, "", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "coprod", Token.COPROD, "\\coprod ", TGroup.OPER, 5),
         new SmTokenTableEntry( "cos", Token.COS, "\\cos ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "cosh", Token.COSH, "\\cosh ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "cot", Token.COT, "\\cot ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "coth", Token.COTH, "\\coth ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "csub", Token.CSUB, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "csup", Token.CSUP, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "cyan", Token.CYAN, "cyan", TGroup.COLOR, 0),
         new SmTokenTableEntry( "dddot", Token.DDDOT, "\\dddot", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "ddot", Token.DDOT, "\\ddot", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "def", Token.DEF, "\\defeq ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "div", Token.DIV, "\\div ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "divides", Token.DIVIDES, "|", TGroup.RELATION, 0),
         new SmTokenTableEntry( "dlarrow" , Token.DLARROW, "\\Leftarrow ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "dlrarrow" , Token.DLRARROW, "\\Leftrightarrow ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "dot", Token.DOT, "\\dot", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "dotsaxis", Token.DOTSAXIS, "\\cdots ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "dotsdiag", Token.DOTSDIAG, "\\ddots ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "dotsdown", Token.DOTSDOWN, "\\ddots ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "dotslow", Token.DOTSLOW, "\\ldots ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "dotsup", Token.DOTSUP, "\\ddotsup ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "dotsvert", Token.DOTSVERT, "\\vdots ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "downarrow" , Token.DOWNARROW, "\\downarrow ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "drarrow" , Token.DRARROW, "\\Rightarrow ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "emptyset" , Token.EMPTYSET, "\\emptyset ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "equiv", Token.EQUIV, "\\equiv ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "exists", Token.EXISTS, "\\exists ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "exp", Token.EXP, "\\exp ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "fact", Token.FACT, "!", TGroup.UNOPER, 5),
         new SmTokenTableEntry( "fixed", Token.FIXED, "\\mathtt", TGroup.FONT, 0),
         new SmTokenTableEntry( "font", Token.FONT, "", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "forall", Token.FORALL, "\\forall ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "from", Token.FROM, "", TGroup.LIMIT, 0),
         new SmTokenTableEntry( "func", Token.FUNC, "", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "ge", Token.GE, "\\geq ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "geslant", Token.GESLANT, "\\geqslant ", TGroup.RELATION, 0 ),
         new SmTokenTableEntry( "gg", Token.GG, "\\gg ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "grave", Token.GRAVE, "\\grave", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "green", Token.GREEN, "green", TGroup.COLOR, 0),
         new SmTokenTableEntry( "gt", Token.GT, ">", TGroup.RELATION, 0),
         new SmTokenTableEntry( "hat", Token.HAT, "\\hat", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "hbar" , Token.HBAR, "\\hbar ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "iiint", Token.IIINT, "\\iiint ", TGroup.OPER, 5),
         new SmTokenTableEntry( "iint", Token.IINT, "\\iint ", TGroup.OPER, 5),
         new SmTokenTableEntry( "in", Token.IN, "\\in ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "infinity" , Token.INFINITY, "\\infty ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "infty" , Token.INFINITY, "\\infty ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "int", Token.INT, "\\int ", TGroup.OPER, 5),
         new SmTokenTableEntry( "intersection", Token.INTERSECT, "\\cap ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "ital", Token.ITALIC, "\\normalsubformula", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "italic", Token.ITALIC, "\\normalsubformula", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "lambdabar" , Token.LAMBDABAR, "\\lambdabar ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "langle", Token.LANGLE, "\\langle ", TGroup.LBRACES, 5),
         new SmTokenTableEntry( "lbrace", Token.LBRACE, "\\{", TGroup.LBRACES, 5),
         new SmTokenTableEntry( "lceil", Token.LCEIL, "\\lceil ", TGroup.LBRACES, 5),
         new SmTokenTableEntry( "ldbracket", Token.LDBRACKET, "\\llbracket ", TGroup.LBRACES, 5),
         new SmTokenTableEntry( "ldline", Token.LDLINE, "\\|", TGroup.LBRACES, 5),
         new SmTokenTableEntry( "le", Token.LE, "\\leq ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "left", Token.LEFT, "", 5),
         new SmTokenTableEntry( "leftarrow" , Token.LEFTARROW, "\\leftarrow ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "leslant", Token.LESLANT, "\\leqslant ", TGroup.RELATION, 0 ),
         new SmTokenTableEntry( "lfloor", Token.LFLOOR, "\\lfloor ", TGroup.LBRACES, 5),
         new SmTokenTableEntry( "lim", Token.LIM, "\\lim ", TGroup.OPER, 5),
         new SmTokenTableEntry( "liminf", Token.LIMINF, "\\liminf ", TGroup.OPER, 5),
         new SmTokenTableEntry( "limsup", Token.LIMSUP, "\\limsup ", TGroup.OPER, 5),
         new SmTokenTableEntry( "lint", Token.LINT, "\\oint ", TGroup.OPER, 5),
         new SmTokenTableEntry( "ll", Token.LL, "\\ll ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "lline", Token.LLINE, "|", TGroup.LBRACES, 5),
         new SmTokenTableEntry( "llint", Token.LLINT, "\\oiint ", TGroup.OPER, 5),
         new SmTokenTableEntry( "lllint", Token.LLLINT, "\\oiiint ", TGroup.OPER, 5),
         new SmTokenTableEntry( "ln", Token.LN, "\\ln ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "log", Token.LOG, "\\log ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "lsub", Token.LSUB, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "lsup", Token.LSUP, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "lt", Token.LT, "<", TGroup.RELATION, 0),
         new SmTokenTableEntry( "magenta", Token.MAGENTA, "magenta", TGroup.COLOR, 0),
         new SmTokenTableEntry( "matrix", Token.MATRIX, "", 5),
         new SmTokenTableEntry( "minusplus", Token.MINUSPLUS, "\\mp ", TGroup.UNOPER, TGroup.SUM, 5),
         new SmTokenTableEntry( "mline", Token.MLINE, "", 0),
         new SmTokenTableEntry( "nabla", Token.NABLA, "\\nabla ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "nbold", Token.NBOLD, "\\normalsubformula", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "ndivides", Token.NDIVIDES, "\\nmid ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "neg", Token.NEG, "\\neg ", TGroup.UNOPER, 5 ),
         new SmTokenTableEntry( "neq", Token.NEQ, "\\neq ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "newline", Token.NEWLINE, "", 0),
         new SmTokenTableEntry( "ni", Token.NI, "\\ni ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "nitalic", Token.NITALIC, "\\mathrm", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "none", Token.NONE, ".", TGroup.LBRACES, TGroup.RBRACES, 0),
         new SmTokenTableEntry( "notin", Token.NOTIN, "\\notin ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "nsubset", Token.NSUBSET, "\\not\\subset ", TGroup.RELATION, 0 ),
         new SmTokenTableEntry( "nsupset", Token.NSUPSET, "\\not\\supset ", TGroup.RELATION, 0 ),
         new SmTokenTableEntry( "nsubseteq", Token.NSUBSETEQ, "\\nsubseteq ", TGroup.RELATION, 0 ),
         new SmTokenTableEntry( "nsupseteq", Token.NSUPSETEQ, "\\nsupseteq ", TGroup.RELATION, 0 ),
         new SmTokenTableEntry( "nroot", Token.NROOT, "", TGroup.UNOPER, 5),
         new SmTokenTableEntry( "odivide", Token.ODIVIDE, "\\oslash ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "odot", Token.ODOT, "\\odot ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "ominus", Token.OMINUS, "\\ominus ", TGroup.SUM, 0),
         new SmTokenTableEntry( "oper", Token.OPER, "", TGroup.OPER, 5),
         new SmTokenTableEntry( "oplus", Token.OPLUS, "\\oplus ", TGroup.SUM, 0),
         new SmTokenTableEntry( "or", Token.OR, "\\vee ", TGroup.SUM, 0),
         new SmTokenTableEntry( "ortho", Token.ORTHO, "\\perp ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "otimes", Token.OTIMES, "\\otimes ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "over", Token.OVER, "", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "overbrace", Token.OVERBRACE, "", TGroup.PRODUCT, 5),
         new SmTokenTableEntry( "overline", Token.OVERLINE, "\\overline", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "overstrike", Token.OVERSTRIKE, "\\mathoverstrike", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "owns", Token.NI, "\\ni ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "parallel", Token.PARALLEL, "\\parallel ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "partial", Token.PARTIAL, "\\partial ", TGroup.STANDALONE, 5 ),
         new SmTokenTableEntry( "phantom", Token.PHANTOM, "\\phantom", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "plusminus", Token.PLUSMINUS, "\\pm ", TGroup.UNOPER, TGroup.SUM, 5),
         new SmTokenTableEntry( "prod", Token.PROD, "\\prod ", TGroup.OPER, 5),
         new SmTokenTableEntry( "prop", Token.PROP, "\\propto ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "rangle", Token.RANGLE, "\\rangle ", TGroup.RBRACES, 0),
         new SmTokenTableEntry( "rbrace", Token.RBRACE, "\\}", TGroup.RBRACES, 0),
         new SmTokenTableEntry( "rceil", Token.RCEIL, "\\rceil ", TGroup.RBRACES, 0),
         new SmTokenTableEntry( "rdbracket", Token.RDBRACKET, "\\rrbracket ", TGroup.RBRACES, 0),
         new SmTokenTableEntry( "rdline", Token.RDLINE, "\\|", TGroup.RBRACES, 0),
         new SmTokenTableEntry( "red", Token.RED, "red", TGroup.COLOR, 0),
         new SmTokenTableEntry( "rfloor", Token.RFLOOR, "\\rfloor", TGroup.RBRACES, 0),
         new SmTokenTableEntry( "right", Token.RIGHT, "", 0),
         new SmTokenTableEntry( "rightarrow" , Token.RIGHTARROW, "\\rightarrow ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "rline", Token.RLINE, "|", TGroup.RBRACES, 0),
         new SmTokenTableEntry( "rsub", Token.RSUB, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "rsup", Token.RSUP, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "sans", Token.SANS, "\\mathsf", TGroup.FONT, 0),
         new SmTokenTableEntry( "serif", Token.SERIF, "\\mathrm", TGroup.FONT, 0),
         new SmTokenTableEntry( "setC" , Token.SETC, "\\mathbb{C}", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "setN" , Token.SETN, "\\mathbb{N}", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "setQ" , Token.SETQ, "\\mathbb{Q}", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "setR" , Token.SETR, "\\mathbb{R}", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "setZ" , Token.SETZ, "\\mathbb{Z}", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "setminus", Token.BACKSLASH, "\\setminus ", TGroup.PRODUCT, 0 ),
         new SmTokenTableEntry( "sim", Token.SIM, "\\sim ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "simeq", Token.SIMEQ, "\\simeq ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "sin", Token.SIN, "\\sin ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "sinh", Token.SINH, "\\sinh ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "size", Token.SIZE, "", TGroup.FONTATTR, 5),
         new SmTokenTableEntry( "slash", Token.SLASH, "/", TGroup.PRODUCT, 0 ),
         new SmTokenTableEntry( "sqrt", Token.SQRT, "", TGroup.UNOPER, 5),
         new SmTokenTableEntry( "stack", Token.STACK, "", 5),
         new SmTokenTableEntry( "sub", Token.RSUB, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "subset", Token.SUBSET, "\\subset ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "subseteq", Token.SUBSETEQ, "\\subseteq ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "sum", Token.SUM, "\\sum ", TGroup.OPER, 5),
         new SmTokenTableEntry( "sup", Token.RSUP, "", TGroup.POWER, 0),
         new SmTokenTableEntry( "supset", Token.SUPSET, "\\supset ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "supseteq", Token.SUPSETEQ, "\\supseteq ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "tan", Token.TAN, "\\tan ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "tanh", Token.TANH, "\\tanh ", TGroup.FUNCTION, 5),
         new SmTokenTableEntry( "tilde", Token.TILDE, "\\tilde ", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "times", Token.TIMES, "\\times ", TGroup.PRODUCT, 0),
         new SmTokenTableEntry( "to", Token.TO, "", TGroup.LIMIT, 0),
         new SmTokenTableEntry( "toward", Token.TOWARD, "\\to ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "transl", Token.TRANSL, "\\multimapdotbothA ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "transr", Token.TRANSR, "\\multimapdotbothB ", TGroup.RELATION, 0),
         new SmTokenTableEntry( "underbrace", Token.UNDERBRACE, "", TGroup.PRODUCT, 5),
         new SmTokenTableEntry( "underline", Token.UNDERLINE, "\\underline", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "union", Token.UNION, "\\cup ", TGroup.SUM, 0),
         new SmTokenTableEntry( "uoper", Token.UOPER, "", TGroup.UNOPER, 5),
         new SmTokenTableEntry( "uparrow" , Token.UPARROW, "\\uparrow ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "vec", Token.VEC, "\\vec", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "white", Token.WHITE, "white", TGroup.COLOR, 0),
         new SmTokenTableEntry( "widebslash", Token.WIDEBACKSLASH, "", TGroup.PRODUCT, 0 ),
         new SmTokenTableEntry( "widehat", Token.WIDEHAT, "\\widehat ", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "widetilde", Token.WIDETILDE, "\\widetilde", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "wideslash", Token.WIDESLASH, "", TGroup.PRODUCT, 0 ),
         new SmTokenTableEntry( "widevec", Token.WIDEVEC, "\\overrightarrow", TGroup.ATTRIBUT, 5),
         new SmTokenTableEntry( "wp" , Token.WP, "\\wp ", TGroup.STANDALONE, 5),
         new SmTokenTableEntry( "yellow", Token.YELLOW, "yellow", TGroup.COLOR, 0)};

    static final SmTokenTableEntry[] symbols=
        {new SmTokenTableEntry("ALPHA",Token.SPECIAL,"A",5),
        new SmTokenTableEntry("BETA",Token.SPECIAL,"B",5),
        new SmTokenTableEntry("CHI",Token.SPECIAL,"X",5),
        new SmTokenTableEntry("DELTA",Token.SPECIAL,"\\Delta ",5),
        new SmTokenTableEntry("EPSILON",Token.SPECIAL,"E",5),
        new SmTokenTableEntry("ETA",Token.SPECIAL,"H",5),
        new SmTokenTableEntry("GAMMA",Token.SPECIAL,"\\Gamma ",5),
        new SmTokenTableEntry("IOTA",Token.SPECIAL,"I",5),
        new SmTokenTableEntry("KAPPA",Token.SPECIAL,"K",5),
        new SmTokenTableEntry("LAMBDA",Token.SPECIAL,"\\Lambda ",5),
        new SmTokenTableEntry("MU",Token.SPECIAL,"M",5),
        new SmTokenTableEntry("NU",Token.SPECIAL,"N",5),
        new SmTokenTableEntry("OMEGA",Token.SPECIAL,"\\Omega ",5),
        new SmTokenTableEntry("OMICRON",Token.SPECIAL,"O",5),
        new SmTokenTableEntry("PHI",Token.SPECIAL,"\\Phi ",5),
        new SmTokenTableEntry("PI",Token.SPECIAL,"\\Pi ",5),
        new SmTokenTableEntry("PSI",Token.SPECIAL,"\\Psi ",5),
        new SmTokenTableEntry("RHO",Token.SPECIAL,"P",5),
        new SmTokenTableEntry("SIGMA",Token.SPECIAL,"\\Sigma ",5),
        new SmTokenTableEntry("TAU",Token.SPECIAL,"T",5),
        new SmTokenTableEntry("THETA",Token.SPECIAL,"\\Theta ",5),
        new SmTokenTableEntry("UPSILON",Token.SPECIAL,"\\Upsilon ",5),
        new SmTokenTableEntry("XI",Token.SPECIAL,"\\Xi ",5),
        new SmTokenTableEntry("ZETA",Token.SPECIAL,"Z",5),
        new SmTokenTableEntry("alpha",Token.SPECIAL,"\\alpha ",5),
        new SmTokenTableEntry("beta",Token.SPECIAL,"\\beta ",5),
        new SmTokenTableEntry("chi",Token.SPECIAL,"\\chi ",5),
        new SmTokenTableEntry("delta",Token.SPECIAL,"\\delta ",5),
        new SmTokenTableEntry("epsilon",Token.SPECIAL,"\\epsilon ",5),
        new SmTokenTableEntry("eta",Token.SPECIAL,"\\eta ",5),
        new SmTokenTableEntry("gamma",Token.SPECIAL,"\\gamma ",5),
        new SmTokenTableEntry("iota",Token.SPECIAL,"\\iota ",5),
        new SmTokenTableEntry("kappa",Token.SPECIAL,"\\kappa ",5),
        new SmTokenTableEntry("lambda",Token.SPECIAL,"\\lambda ",5),
        new SmTokenTableEntry("mu",Token.SPECIAL,"\\mu ",5),
        new SmTokenTableEntry("nu",Token.SPECIAL,"\\nu ",5),
        new SmTokenTableEntry("omega",Token.SPECIAL,"\\omega ",5),
        new SmTokenTableEntry("omicron",Token.SPECIAL,"o",5),
        new SmTokenTableEntry("phi",Token.SPECIAL,"\\phi ",5),
        new SmTokenTableEntry("pi",Token.SPECIAL,"\\pi ",5),
        new SmTokenTableEntry("psi",Token.SPECIAL,"\\psi ",5),
        new SmTokenTableEntry("rho",Token.SPECIAL,"\\rho ",5),
        new SmTokenTableEntry("sigma",Token.SPECIAL,"\\sigma ",5),
        new SmTokenTableEntry("tau",Token.SPECIAL,"\\tau ",5),
        new SmTokenTableEntry("theta",Token.SPECIAL,"\\theta ",5),
        new SmTokenTableEntry("upsilon",Token.SPECIAL,"\\upsilon ",5),
        new SmTokenTableEntry("varepsilon",Token.SPECIAL,"\\varepsilon ",5),
        new SmTokenTableEntry("varphi",Token.SPECIAL,"\\varphi ",5),
        new SmTokenTableEntry("varpi",Token.SPECIAL,"\\varpi ",5),
        new SmTokenTableEntry("varrho",Token.SPECIAL,"\\varrho ",5),
        new SmTokenTableEntry("varsigma",Token.SPECIAL,"\\varsigma ",5),
        new SmTokenTableEntry("vartheta",Token.SPECIAL,"\\vartheta ",5),
        new SmTokenTableEntry("xi",Token.SPECIAL,"\\xi ",5),
        new SmTokenTableEntry("zeta",Token.SPECIAL,"\\zeta ",5),
        new SmTokenTableEntry("and",Token.SPECIAL,"\\wedge ",5),
        new SmTokenTableEntry("angle",Token.SPECIAL,"\\sphericalangle ",5),
        new SmTokenTableEntry("element",Token.SPECIAL,"\\in ",5),
        new SmTokenTableEntry("identical",Token.SPECIAL,"\\equiv ",5),
        new SmTokenTableEntry("infinite",Token.SPECIAL,"\\infty ",5),
        new SmTokenTableEntry("noelement",Token.SPECIAL,"\\notin ",5),
        new SmTokenTableEntry("notequal",Token.SPECIAL,"\\neq ",5),
        new SmTokenTableEntry("or",Token.SPECIAL,"\\vee ",5),
        new SmTokenTableEntry("perthousand",Token.SPECIAL,"\\text{\\textperthousand}",5),
        new SmTokenTableEntry("strictlygreaterthan",Token.SPECIAL,"\\gg ",5),
        new SmTokenTableEntry("strictlylessthan",Token.SPECIAL,"\\ll ",5),
        new SmTokenTableEntry("tendto",Token.SPECIAL,"\\to ",5)};
}

// Token (this is simply a struct)
class SmToken{
    Token   eType;
    String  sLaTeX;
    TGroup  eGroup1, eGroup2;
    int     nLevel;

    SmToken(){
        this.assign(Token.UNKNOWN,"",0);
    }

    void assign(Token eType, String sLaTeX, TGroup eGroup1, TGroup eGroup2, int nLevel){
        this.eType=eType;
        this.sLaTeX=sLaTeX;
        this.eGroup1=eGroup1;
        this.eGroup2=eGroup2;
        this.nLevel=nLevel;
    }

    void assign(Token eType, String sLaTeX, TGroup eGroup1, int nLevel){
        assign(eType, sLaTeX, eGroup1, TGroup.NONE, nLevel);
    }

    void assign(Token eType, String sLaTeX, int nLevel){
        assign(eType, sLaTeX, TGroup.NONE, TGroup.NONE, nLevel);
    }
}


///////////////////////////////////////////////////////////////////
// The converter class
public final class StarMathConverter {

    // Variables
    private SmTokenTable keywords=new SmTokenTable(SmTokenTable.keywords);
    private SmTokenTable symbols=new SmTokenTable(SmTokenTable.symbols);
    private Hashtable configSymbols;
    private boolean bUseColor;
    private SmToken curToken=new SmToken(); // contains the data of the current token
    private SimpleInputBuffer buffer; // contains the starmath formula
    private Float fBaseSize; // base size for the formula (usually 12pt)
    private I18n i18n;

    // Flags to track need for ooomath.sty definitions
    private boolean bDefeq = false;
    private boolean bLambdabar = false;
    private boolean bDdotsup = false;
    private boolean bMultimapdotbothA = false;
    private boolean bMultimapdotbothB = false;
    private boolean bLlbracket = false;
    private boolean bRrbracket = false;
    private boolean bOiint = false;
    private boolean bOiiint = false;
    private boolean bWideslash = false;
    private boolean bWidebslash = false;
    private boolean bBoldsubformula = false;
    private boolean bNormalsubformula = false;
    private boolean bMultiscripts = false;
    private boolean bMathoverstrike = false;

    StarMathConverter(I18n i18n, Config config){
        this.i18n = i18n;
        configSymbols = config.getMathSymbols();
        bUseColor = config.useColor();
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (bDefeq) {
            decl.append("\\newcommand\\defeq{\\stackrel{\\mathrm{def}}{=}}").nl();
        }
        if (bLambdabar) {
            decl.append("\\newcommand\\lambdabar{\\mathchar'26\\mkern-10mu\\lambda}").nl();
        }
        if (bDdotsup) {
            decl.append("\\newcommand\\ddotsup{\\mathinner{\\mkern1mu\\raise1pt\\vbox{\\kern7pt\\hbox{.}}\\mkern2mu\\raise4pt\\hbox{.}\\mkern2mu\\raise7pt\\hbox{.}\\mkern1mu}}").nl();
        }
        if (bMultimapdotbothA) {
            decl.append("\\providecommand\\multimapdotbothA{\\bullet\\kern-0.4em-\\kern-0.4em\\circ}").nl();
        }
        if (bMultimapdotbothB) {
            decl.append("\\providecommand\\multimapdotbothB{\\circ\\kern-0.4em-\\kern-0.4em\\bullet}").nl();
        }
        if (bLlbracket) {
            decl.append("\\providecommand\\llbracket{[}").nl();
        }
        if (bRrbracket) {
            decl.append("\\providecommand\\rrbracket{]}").nl();
        }
        if (bOiint) {
            decl.append("\\providecommand\\oiint{\\oint}").nl();
        }
        if (bOiiint) {
            decl.append("\\providecommand\\oiiint{\\oint}").nl();
        }
        if (bWideslash) {
            decl.append("\\newcommand\\wideslash[2]{{}^{#1}/_{#2}}").nl();
        }
        if (bWidebslash) {
            decl.append("\\newcommand\\widebslash[2]{{}_{#1}\\backslash^{#2}}").nl();
        }
        if (bBoldsubformula) {
            decl.append("\\newcommand\\boldsubformula[1]{\\text{\\mathversion{bold}$#1$}}").nl();
        }
        if (bNormalsubformula) {
            decl.append("\\newcommand\\normalsubformula[1]{\\text{\\mathversion{normal}$#1$}}").nl();
        }
        if (bMultiscripts || bMathoverstrike) {
            decl.append("\\newlength{\\idxmathdepth}\\newlength{\\idxmathtotal}\\newlength{\\idxmathwidth}\\newlength{\\idxraiseme}").nl();
            decl.append("\\newcommand{\\idxdheight}[1]{\\protect\\settoheight{\\idxmathtotal}{\\(\\displaystyle#1\\)}\\protect\\settodepth{\\idxmathdepth}{\\(\\displaystyle#1\\)}\\protect\\settowidth{\\idxmathwidth}{\\(\\displaystyle#1\\)}\\protect\\addtolength{\\idxmathtotal}{\\idxmathdepth}\\protect\\setlength{\\idxraiseme}{\\idxmathtotal/2-\\idxmathdepth}}").nl();
            decl.append("\\newcommand{\\idxtheight}[1]{\\protect\\settoheight{\\idxmathtotal}{\\(\\textstyle #1\\)}\\protect\\settodepth{\\idxmathdepth}{\\(\\textstyle #1\\)}\\protect\\settowidth{\\idxmathwidth}{\\(\\textstyle#1\\)}\\protect\\addtolength{\\idxmathtotal}{\\idxmathdepth}\\protect\\setlength{\\idxraiseme}{\\idxmathtotal/2-\\idxmathdepth}}").nl();
            decl.append("\\newcommand{\\idxsheight}[1]{\\protect\\settoheight{\\idxmathtotal}{\\(\\scriptstyle #1\\)}\\protect\\settodepth{\\idxmathdepth}{\\(\\scriptstyle #1\\)}\\protect\\settowidth{\\idxmathwidth}{\\(\\scriptstyle#1\\)}\\protect\\addtolength{\\idxmathtotal}{\\idxmathdepth}\\protect\\setlength{\\idxraiseme}{\\idxmathtotal/2-\\idxmathdepth}}").nl();
            decl.append("\\newcommand{\\idxssheight}[1]{\\protect\\settoheight{\\idxmathtotal}{\\(\\scriptscriptstyle #1\\)}\\protect\\settodepth{\\idxmathdepth}{\\(\\scriptscriptstyle #1\\)}\\protect\\settowidth{\\idxmathwidth}{\\(\\scriptscriptstyle#1\\)}\\protect\\addtolength{\\idxmathtotal}{\\idxmathdepth}\\protect\\setlength{\\idxraiseme}{\\idxmathtotal/2-\\idxmathdepth}}").nl();
        }
        if (bMultiscripts) {
            decl.append("\\newcommand\\multiscripts[5]{\\mathchoice")
                .append("{\\idxdheight{#4}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#1\\underset{#2}{\\overset{#3}{#4}}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#5}")
                .append("{\\idxtheight{#4}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#1\\underset{#2}{\\overset{#3}{#4}}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#5}")
                .append("{\\idxsheight{#4}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#1\\underset{#2}{\\overset{#3}{#4}}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#5}")
                .append("{\\idxssheight{#4}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#1\\underset{#2}{\\overset{#3}{#4}}\\rule[-\\idxmathdepth]{0mm}{\\idxmathtotal}#5}}")
                .nl();
        }
        if (bMathoverstrike) {
            decl.append("\\newcommand\\mathoverstrike[1]{\\mathchoice")
                .append("{\\idxdheight{#1}\\rlap{\\rule[\\idxraiseme]{\\idxmathwidth}{0.4pt}}{#1}}")
                .append("{\\idxtheight{#1}\\rlap{\\rule[\\idxraiseme]{\\idxmathwidth}{0.4pt}}{#1}}")
                .append("{\\idxsheight{#1}\\rlap{\\rule[\\idxraiseme]{\\idxmathwidth}{0.4pt}}{#1}}")
                .append("{\\idxssheight{#1}\\rlap{\\rule[\\idxraiseme]{\\idxmathwidth}{0.4pt}}{#1}}}")
                .nl();
        }
    }

    /////////////////////////////////////////
    // Tokenizer

    private boolean tokenInGroup(TGroup eGroup){
        return curToken.eGroup1==eGroup || curToken.eGroup2==eGroup;
    }

    private void skipWhiteSpaces(){
        while (Character.isWhitespace(buffer.peekChar())) buffer.getChar();
    }

    private void skipComment(){
        if (buffer.peekChar()!='%' || buffer.peekFollowingChar()!='%') return;
        while (!CharClasses.isEndOrLineEnd(buffer.peekChar())) buffer.getChar();
    }

    private void nextToken(){
        // read next token from buffer and update curToken
        int nStart;
        do // move to first significant character
        { nStart=buffer.getIndex();
          skipWhiteSpaces();
          skipComment();
        } while (nStart<buffer.getIndex());
        if (buffer.peekChar()=='\0'){ // End of input
            curToken.assign(Token.END,"",0);
            return;
        }
        else if (buffer.peekChar()=='"'){ // Text
            String sText="";
            buffer.getChar();
            while (buffer.peekChar()!='"' && buffer.peekChar()!='\0'){
                sText+=buffer.getChar(); // temporary...
            }
            buffer.getChar();
            // TODO: Language should be inherited from surrounding text
            curToken.assign(Token.TEXT, i18n.convert(sText,false,"en"), 5);
            return;
        }
        else if (buffer.peekChar()=='%'){ // Special
            buffer.getChar();
            String sIdent=buffer.getIdentifier();
            if (configSymbols.containsKey(sIdent)) { // symbol defined in configuration
                curToken.assign(Token.SPECIAL, (String) configSymbols.get(sIdent), 5);
            }
            else if (!symbols.lookup(sIdent,false,curToken))
                curToken.assign(Token.IDENT, i18n.convert(sIdent,true,"en"), 5);
        }
        else if (Character.isLetter(buffer.peekChar())){ // Identifier
            String sIdent=buffer.getIdentifier();
            if (!keywords.lookup(sIdent,true,curToken))
                curToken.assign(Token.IDENT, i18n.convert(sIdent,true,"en"), 5);
            return;
        }
        else if (CharClasses.isDigitOrDot(buffer.peekChar())){ // Number
            String sNumber=buffer.getNumber();
            curToken.assign(Token.NUMBER, sNumber, 5);
            return;
        }
        else {
            char cChar=buffer.getChar();
            switch (cChar) {
            case '<':
                if (buffer.peekChar()=='<'){
                    buffer.getChar();
                    curToken.assign(Token.LL, "\\ll ", TGroup.RELATION, 0);
                }
                else if (buffer.peekChar()=='='){
                    buffer.getChar();
                    curToken.assign(Token.LE, "\\le ", TGroup.RELATION, 0);
                }
                else if (buffer.peekChar()=='>'){
                    buffer.getChar();
                    curToken.assign(Token.NEQ, "\\neq ", TGroup.RELATION, 0);
                }
                else if (buffer.peekChar()=='?' && buffer.peekFollowingChar()=='>'){
                    buffer.getChar(); buffer.getChar();
                    curToken.assign(Token.PLACE, "\\Box ", TGroup.STANDALONE, 5); // no group in parse.cxx
                }
                else {
                    curToken.assign(Token.LT, "<", TGroup.RELATION, 0);
                }
                return;
            case '>':
                if (buffer.peekChar()=='>'){
                    buffer.getChar();
                    curToken.assign(Token.GG, "\\gg ", TGroup.RELATION, 0);
                }
                else if (buffer.peekChar()=='='){
                    buffer.getChar();
                    curToken.assign(Token.GE, "\\ge ", TGroup.RELATION, 0);
                }
                else {
                    curToken.assign(Token.GT, ">", TGroup.RELATION, 0);
                }
                return;
            case '[':
                curToken.assign(Token.LBRACKET, "[", TGroup.LBRACES, 5);
                return;
            case '\\':
                curToken.assign(Token.ESCAPE, "", 5);
                return;
            case ']':
                curToken.assign(Token.RBRACKET, "]", TGroup.RBRACES, 0);
                return;
            case '^':
                curToken.assign(Token.RSUP, "", TGroup.POWER, 0);
                return;
            case '_':
                curToken.assign(Token.RSUB, "", TGroup.POWER, 0);
                return;
            case '`':
                curToken.assign(Token.SBLANK, "\\;", TGroup.BLANK, 5);
                return;
            case '{':
                curToken.assign(Token.LGROUP, "{", 5);
                return;
            case '|':
                curToken.assign(Token.OR, "\\vee ", TGroup.SUM, 0);
                return;
            case '}':
                curToken.assign(Token.RGROUP, "}", 0);
                return;
            case '~':
                curToken.assign(Token.BLANK, "\\ ", TGroup.BLANK, 5);
                return;
            case '#':
                if (buffer.peekChar()=='#'){
                    buffer.getChar();
                    curToken.assign(Token.DPOUND, "", 0);
                }
                else {
                    curToken.assign(Token.POUND, "", 0);
                }
                return;
            case '&':
                curToken.assign(Token.AND, "\\wedge ", TGroup.PRODUCT, 0);
                return;
            case '(':
                curToken.assign(Token.LPARENT, "(", TGroup.LBRACES, 5);
                return;
            case ')':
                curToken.assign(Token.RPARENT, ")", TGroup.RBRACES, 0);
                return;
            case '*':
                curToken.assign(Token.MULTIPLY, "\\ast ", TGroup.PRODUCT, 0);
                return;
            case '+':
                if (buffer.peekChar()=='-'){
                    buffer.getChar();
                    curToken.assign(Token.PLUSMINUS, "\\pm ", TGroup.UNOPER, TGroup.SUM, 5);
                }
                else {
                    curToken.assign(Token.PLUS, "+", TGroup.UNOPER, TGroup.SUM, 5);
                }
                return;
            case '-':
                if (buffer.peekChar()=='+'){
                    buffer.getChar();
                    curToken.assign(Token.MINUSPLUS, "\\mp ", TGroup.UNOPER, TGroup.SUM, 5);
                }
                else {
                    curToken.assign(Token.MINUS, "-", TGroup.UNOPER, TGroup.SUM, 5);
                }
                return;
            //case '.': // not relevant...    ??
            //    curToken.assign(Token.POINT, ".", 0);
            //    return;
            case '/':
                curToken.assign(Token.DIVIDEBY, "/", TGroup.PRODUCT, 0);
                return;
            case '=':
                curToken.assign(Token.ASSIGN, "=", TGroup.RELATION, 0);
                return;
            default:
                Character cCharObject=new Character(cChar);
                curToken.assign(Token.CHARACTER,i18n.convert(cCharObject.toString(),true,"en"),5);
                return;
            }

        }
    }

    ////////////////////////////////////////////////
    // Grammar

    private String table(float fSize, Token eAlign){
        StringBuffer bufTable=new StringBuffer();
        String sLine=line(fSize,eAlign);
        if (curToken.eType==Token.NEWLINE){ // more than one line
            bufTable.append("\\begin{gathered}").append(sLine);
            while (curToken.eType==Token.NEWLINE){
                nextToken();
                bufTable.append("\\\\").append(line(fSize,eAlign));
            }
            return bufTable.append("\\end{gathered}").toString();
        }
        else { // only one line
            return sLine;
        }
    }

    private String align(float fSize, Token eAlign,boolean bUseAlignment,boolean bNeedNull){
        // Alignment works very different in StarMath and LaTeX:
        // In LaTeX alignment is accomplished using suitable \hfill's in appropriate spots.
        // Hence we need to pass on the current alignment as a parameter to decide where to \hfill.
        // bUseAlignment requires us to add a suitable \hfill (set true by table, matrix and stack).
        // bNeedNull requires us to add \null (an empty hbox) at the end (the matrix environment
        // needs this).
        // Currently fractions and binoms are *not* aligned.
        // In the other constructions alignment doesn't work if it's put inside a group:
        // stack{{alignl a}#aaaa} does not work, while stack{alignl a#aaaa} does work.

        if (tokenInGroup(TGroup.ALIGN)){
            eAlign=curToken.eType;
            nextToken();
        }
        if (bUseAlignment && eAlign==Token.ALIGNL){
            if (bNeedNull){
                return expression(fSize,eAlign)+"\\hfill\\null ";
            }
            else {
                return expression(fSize,eAlign)+"\\hfill ";
            }
        }
        else if (bUseAlignment && eAlign==Token.ALIGNR){
            return "\\hfill "+expression(fSize,eAlign);
        }
        else { // center alignment (default!) or no alignment
            return expression(fSize,eAlign);
        }
    }

    private String line(float fSize, Token eAlign){
        if (curToken.eType!=Token.NEWLINE && curToken.eType!=Token.END){
            // Add implicit left alignment for expressions starting with text
            // (Note: Don't pass on this alignment to subexpressions!)
            if (curToken.eType==Token.TEXT) {
                return expression(fSize,eAlign)+"\\hfill ";
            }
            else {
                return align(fSize,eAlign,true,false);
            }
        }
        else { // empty line
            return "{}"; // LaTeX doesn't like empty lines in gather
        }
    }

    private String expression(float fSize, Token eAlign){
        StringBuffer bufExpression=new StringBuffer().append(relation(fSize,eAlign));
        while (curToken.nLevel>=5){
            bufExpression.append(relation(fSize,eAlign));
        }
        return bufExpression.toString();
    }

    private String relation(float fSize, Token eAlign){
        StringBuffer bufRelation=new StringBuffer().append(sum(fSize,eAlign));
        while (tokenInGroup(TGroup.RELATION)){
            if (curToken.eType==Token.TRANSL) { bMultimapdotbothA=true; }
            else if (curToken.eType==Token.TRANSR) { bMultimapdotbothB=true; }
            else if (curToken.eType==Token.DEF) { bDefeq=true; }
            bufRelation.append(opsubsup(fSize,eAlign)).append(sum(fSize,eAlign));
        }
        return bufRelation.toString();
    }

    private String sum(float fSize, Token eAlign){
        StringBuffer bufSum=new StringBuffer().append(product(fSize,eAlign));
        while (tokenInGroup(TGroup.SUM)){
            bufSum.append(opsubsup(fSize,eAlign)).append(product(fSize,eAlign));
        }
        return bufSum.toString();
    }

    private String product(float fSize, Token eAlign){
        String sProduct=power(fSize,eAlign,true);
        sProduct=sProduct.substring(1,sProduct.length()-1);
        // a small hack to avoid double {}: Require {}, then remove them
        // and add them below if they are needed.
        while (tokenInGroup(TGroup.PRODUCT)){
            if (curToken.eType==Token.OVER){
                 nextToken();
                 sProduct="\\frac{"+sProduct+"}"+power(fSize,eAlign,true);
            } else if (curToken.eType==Token.BOPER){
                 nextToken();
                 sProduct+=special()+power(fSize,eAlign,false);
            } else if (curToken.eType==Token.OVERBRACE){
                 nextToken();
                 sProduct="\\overbrace{"+sProduct+"}^"+power(fSize,eAlign,true);
            } else if (curToken.eType==Token.UNDERBRACE){
                 nextToken();
                 sProduct="\\underbrace{"+sProduct+"}_"+power(fSize,eAlign,true);
            } else  if (curToken.eType==Token.WIDESLASH){
                 bWideslash=true;
                 nextToken();
                 sProduct="\\wideslash{"+sProduct+"}"+power(fSize,eAlign,true);
            } else if (curToken.eType==Token.WIDEBACKSLASH){
                 bWidebslash=true;
                 nextToken();
                 sProduct="\\widebslash{"+sProduct+"}"+power(fSize,eAlign,true);
            } else {
                 sProduct+=opsubsup(fSize,eAlign)+power(fSize,eAlign,false);
            }
        }
        return sProduct;

    }

    private String tosub(String s){
        return s!=null ? "_"+s : "";
    }

    private String tosup(String s){
        return s!=null ? "^"+s : "";
    }

    private String subsup(float fSize, Token eAlign,String sBody, TGroup eActiveGroup){
        // sBody is the string to attach scripts to
        // eActiveGroup must be TGroup.LIMIT or TGroup.POWER
        // in the former case sBody must contain a large operator (sum, int...)
        Token eScriptType;
        String sLsub=null, sLsup=null, sCsub=null, sCsup=null, sRsub=null, sRsup=null;
        while (tokenInGroup(eActiveGroup)){
            eScriptType=curToken.eType;
            nextToken();
            if (eScriptType==Token.FROM) sCsub="{"+relation(fSize,eAlign)+"}";
            else if (eScriptType==Token.TO) sCsup="{"+relation(fSize,eAlign)+"}";
            else if (eScriptType==Token.LSUB) sLsub=term(fSize,eAlign,true);
            else if (eScriptType==Token.LSUP) sLsup=term(fSize,eAlign,true);
            else if (eScriptType==Token.CSUB) sCsub=term(fSize,eAlign,true);
            else if (eScriptType==Token.CSUP) sCsup=term(fSize,eAlign,true);
            else if (eScriptType==Token.RSUB) sRsub=term(fSize,eAlign,true);
            else if (eScriptType==Token.RSUP) sRsup=term(fSize,eAlign,true);
        }
        if (sLsub==null && sLsup==null && sCsub==null && sCsup==null && sRsub==null && sRsup==null){
            return sBody;
        }
        if (eActiveGroup==TGroup.LIMIT){
            if (sLsub==null && sLsup==null && sRsub==null && sRsup==null){
                // ordinary limits
                return sBody+tosub(sCsub)+tosup(sCsup);
            }
            else { // nontrivial case: use \sideset
                // problem: always typesets the operator in \displaystyle
                // solution: use \multiscripts instead???
                return "\\sideset{"+tosub(sLsub)+tosup(sLsup)+"}{"+tosub(sRsub)+tosup(sRsup)+"}"
                +sBody+"\\limits"+tosub(sCsub)+tosup(sCsup);
            }
        }
        else {
            if (sLsub==null && sLsup==null && sCsub==null && sCsup==null){
                // ordinary scripts
                return sBody+tosub(sRsub)+tosup(sRsup);
            }
            else if (sLsub==null && sLsup==null && sRsub==null && sRsup==null){
                // scripts above/below
                if (sCsub==null){
                    return "\\overset"+sCsup+"{"+sBody+"}";
                }
                else if (sCsup==null){
                    return "\\underset"+sCsub+"{"+sBody+"}";
                }
                else {
                    return "\\overset"+sCsup+"{\\underset"+sCsub+"{"+sBody+"}}";
                }
            }
            else {// general case: use \multiscripts
                bMultiscripts=true;
                if (sCsub==null) {sCsub="{}";}
                if (sCsup==null) {sCsup="{}";}
                return "\\multiscripts{"+tosub(sLsub)+tosup(sLsup)+"}"+sCsub+sCsup
                +"{"+sBody+"}{"+tosub(sRsub)+tosup(sRsup)+"}";
            }
        }
    }

    private String opsubsup(float fSize, Token eAlign){
        String sOperator=curToken.sLaTeX;
        nextToken();
        return subsup(fSize,eAlign,sOperator,TGroup.POWER);
    }

    private String power(float fSize, Token eAlign,boolean bNeedGroup){
        // bNeedGroup is true, if the power needs to be enclosed in braces
        // Since we don't want to add unnecessary braces the responsibility
        // is delegated to power - we need to look ahead to determine if {}
        // should be added.
        boolean bTermIsGroup=curToken.eType==Token.LGROUP;
        String sTerm=term(fSize,eAlign);
        if (bNeedGroup && (!bTermIsGroup || tokenInGroup(TGroup.POWER))){
            return "{"+subsup(fSize,eAlign,sTerm,TGroup.POWER)+"}";
        }
        else {
            return subsup(fSize,eAlign,sTerm,TGroup.POWER);
        }
    }

    private String blank(){
        StringBuffer bufBlank=new StringBuffer();
        while (tokenInGroup(TGroup.BLANK)){
            bufBlank.append(curToken.sLaTeX);
            nextToken();
        }
        return bufBlank.toString();
    }

    private String term(float fSize, Token eAlign, boolean bNeedGroup){
        // Special version of term used to avoid double {{grouping}}
        // if bNeedGroup=true we must return {term} in braces
        if (bNeedGroup && !(curToken.eType==Token.LGROUP)){
            return "{"+term(fSize,eAlign)+"}";
        }
        /*else if (!bNeedGroup && curToken.eType==Token.LGROUP){
            String sTerm=term(fSize,eAlign);
            return sTerm.substring(1,sTerm.length()-2); // renove unwanted {}
        }*/
        else {
            return term(fSize,eAlign);
        }
    }

    private String term(float fSize, Token eAlign){
        String sContent;
        if (curToken.eType==Token.ESCAPE)
            return escape();
        else if (curToken.eType==Token.LGROUP){
            nextToken();
            if (curToken.eType!=Token.RGROUP)
                sContent=align(fSize,eAlign,false,false);
            else
                sContent=""; // empty group
            if (curToken.eType==Token.RGROUP)
                nextToken();
            // otherwise there is an error in the formula
            // we close the group anyway to make us TeX'able.
            return "{"+sContent+"}";
        }
        else if (curToken.eType==Token.LEFT)
            return scalebrace(fSize,eAlign);
        else if (tokenInGroup(TGroup.BLANK))
            return blank();
        else if (curToken.eType==Token.TEXT){
            sContent=curToken.sLaTeX;
            nextToken();
            return "\\text{"+sContent+"}";
        }
        else if (curToken.eType==Token.CHARACTER || curToken.eType==Token.NUMBER
                 || tokenInGroup(TGroup.STANDALONE)){
            if (curToken.eType==Token.LAMBDABAR) { bLambdabar=true; }
            if (curToken.eType==Token.DOTSUP) { bDdotsup=true; }
            sContent=curToken.sLaTeX;
            nextToken();
            return sContent;
        }
        else if (curToken.eType==Token.IDENT){
            sContent=curToken.sLaTeX;
            nextToken();
            return sContent.length()>1 ? "\\mathit{"+sContent+"}" : sContent;
        }
        else if (curToken.eType==Token.SPECIAL)
            return special();
        else if (curToken.eType==Token.BINOM)
            return binom(fSize,eAlign);
        else if (curToken.eType==Token.STACK)
            return stack(fSize,eAlign);
        else if (curToken.eType==Token.MATRIX)
            return matrix(fSize,eAlign);
        else if (tokenInGroup(TGroup.LBRACES))
            return brace(fSize,eAlign);
        else if (tokenInGroup(TGroup.OPER))
            return operator(fSize,eAlign);
        else if (tokenInGroup(TGroup.UNOPER))
            return unoper(fSize,eAlign);
        else if (tokenInGroup(TGroup.ATTRIBUT) || tokenInGroup(TGroup.FONTATTR))
            return attributes(fSize,eAlign);
        else if (tokenInGroup(TGroup.FUNCTION))
            return function();
        else { // error in formula
            nextToken();
            return "?";
        }
    }

    private String escape(){
        String sEscape;
        nextToken();
        if ((tokenInGroup(TGroup.LBRACES) || tokenInGroup(TGroup.RBRACES))
        && curToken.eType!=Token.NONE){
            sEscape=curToken.sLaTeX;
            nextToken();
        }
        else if (curToken.eType==Token.LGROUP){
            sEscape="\\{";
            nextToken();
        }
        else if (curToken.eType==Token.RGROUP){
            sEscape="\\}";
            nextToken();
        }
        else { // error in formula
            sEscape="";
        }
        return sEscape;
    }

    private String operator(float fSize, Token eAlign){
        String sOperator=oper();
        if (tokenInGroup(TGroup.LIMIT) || tokenInGroup(TGroup.POWER)){
            // Note: TGroup.LIMIT and TGroup.POWER are always in eGroup1, so this is OK:
            return subsup(fSize,eAlign,sOperator,curToken.eGroup1)+power(fSize,eAlign,false);
        }
        else {
            return sOperator+power(fSize,eAlign,false);
        }
    }

    private String oper(){
        if (curToken.eType==Token.LLINT) { bOiint=true; }
        else if (curToken.eType==Token.LLLINT) { bOiiint=true; }
        String sOper;
        if (curToken.eType==Token.OPER){
            nextToken();
            if (curToken.eType==Token.SPECIAL)
                sOper="\\operatornamewithlimits{"+curToken.sLaTeX+"}";
            else
                sOper="\\operatornamewithlimits{?}"; // error in formula
        }
        else {
            sOper=curToken.sLaTeX;
        }
        nextToken();
        return sOper;
    }

    private String unoper(float fSize, Token eAlign){
        if (curToken.eType==Token.ABS){
            nextToken();
            return "\\left|"+power(fSize,eAlign,false)+"\\right|";
        }
        else if (curToken.eType==Token.SQRT){
            nextToken();
            return "\\sqrt"+power(fSize,eAlign,true);
        }
        else if (curToken.eType==Token.NROOT){
            nextToken();
            return "\\sqrt["+power(fSize,eAlign,false)+"]"+power(fSize,eAlign,true);
        }
        else if (curToken.eType==Token.UOPER){
            nextToken();
            return special()+power(fSize,eAlign,false);
        }
        else if (curToken.eType==Token.FACT){
            String sOperator=opsubsup(fSize,eAlign);
            return power(fSize,eAlign,false)+sOperator;
        }
        else { // must be PLUS, MINUS, PLUSMINUS, MINUSPLUS or NEG
            return opsubsup(fSize,eAlign)+power(fSize,eAlign,false);
        }
    }

    private String attributes(float fSize, Token eAlign){
        String sAttribute;
        if (curToken.eType==Token.FONT){
            nextToken();
            if (tokenInGroup(TGroup.FONT)){
                sAttribute=curToken.sLaTeX;
                nextToken();
                return sAttribute+"{"+term(fSize,eAlign)+"}";
            }
            else { // error in formula
                return "?";
            }
        }
        else if (curToken.eType==Token.COLOR){
            nextToken();
            if (tokenInGroup(TGroup.COLOR)){
                sAttribute=curToken.sLaTeX; // the color name
                nextToken();
                if (bUseColor) {
                    return "\\textcolor{"+sAttribute+"}{"+term(fSize,eAlign)+"}";
                    // note: despite the name, \textcolor also works in math mode!
                }
                else {
                    return term(fSize,eAlign);
                }
            }
            else { // error in formula
                return "?";
            }
        }
        else if (curToken.eType==Token.SIZE){
            nextToken();
            if (curToken.eType==Token.PLUS){
                nextToken();
                if (curToken.eType==Token.NUMBER){
                    fSize+=Misc.getFloat(curToken.sLaTeX,0);
                    nextToken();
                } // else error in formula: ignore
            }
            else if(curToken.eType==Token.MINUS){
                nextToken();
                if (curToken.eType==Token.NUMBER){
                    fSize-=Misc.getFloat(curToken.sLaTeX,0);
                    nextToken();
                } // else error in formula: ignore
            }
            else if(curToken.eType==Token.MULTIPLY){
                nextToken();
                if (curToken.eType==Token.NUMBER){
                    fSize*=Misc.getFloat(curToken.sLaTeX,1);
                    nextToken();
                } // else error in formula: ignore
            }
            else if(curToken.eType==Token.DIVIDEBY){
                nextToken();
                if (curToken.eType==Token.NUMBER){
                    float f=Misc.getFloat(curToken.sLaTeX,1);
                    if (f!=0) {fSize/=f;}
                    nextToken();
                } // else error in formula: ignore
            }
            else if (curToken.eType==Token.NUMBER){
                fSize=Misc.getFloat(curToken.sLaTeX,fSize);
                nextToken();
            } // else error in formula: ignore
            return term(fSize,eAlign);
            // currently only reads the size, it is not used
            // should use fSize/fBaseSize to change to
            // \displaystyle, \textstyle, \scriptstyle, \scriptscriptstyle
        }
        else { // must be ATTRIBUT or FONTATTR
            if (curToken.eType == Token.OVERSTRIKE) { bMathoverstrike=true; }
            else if (curToken.eType == Token.BOLD) { bBoldsubformula=true; }
            else if (curToken.eType == Token.NBOLD) { bNormalsubformula=true; }
            sAttribute=curToken.sLaTeX;
            nextToken();
            return sAttribute+"{"+term(fSize,eAlign)+"}";
        }
    }

    private String scalebrace(float fSize, Token eAlign){
        String sLeft, sRight, sBody;
        nextToken();
        if (tokenInGroup(TGroup.LBRACES) || tokenInGroup(TGroup.RBRACES)){
            if (curToken.eType==Token.LDBRACKET) { bLlbracket=true; }
            else if (curToken.eType==Token.RDBRACKET) { bRrbracket=true; }
            sLeft=new String(curToken.sLaTeX);
            nextToken();
            sBody=scalebracebody(fSize,eAlign);
            if (curToken.eType==Token.RIGHT) {
                nextToken();
                if (tokenInGroup(TGroup.LBRACES) || tokenInGroup(TGroup.RBRACES)){
                    if (curToken.eType==Token.LDBRACKET) { bLlbracket=true; }
                    else if (curToken.eType==Token.RDBRACKET) { bRrbracket=true; }
                    sRight=new String(curToken.sLaTeX);
                    nextToken();
                }
                else { // no brace after right!
                    sRight=".";
                }
            }
            else { // no right!
                return ".";
            }
            return "\\left"+sLeft+sBody+"\\right"+sRight;
        }
        else { // no brace after left!
            return "?";
        }
    }

    private String brace(float fSize, Token eAlign){
        String sLeft, sRight, sBody;
        if (curToken.eType==Token.LDBRACKET) { bLlbracket=true; }
        sLeft=new String(curToken.sLaTeX);
        nextToken();
        sBody=bracebody(fSize,eAlign);
        if (tokenInGroup(TGroup.RBRACES)){
            if (curToken.eType==Token.RDBRACKET) { bRrbracket=true; }
            sRight=new String(curToken.sLaTeX);
            nextToken();
            return sLeft+sBody+sRight;
        }
        else { // no right brace! (This is an error, we don't care);
            return sLeft+sBody;
        }
    }


    private String scalebracebody(float fSize, Token eAlign){
        if (curToken.eType==Token.MLINE){
            nextToken();
            return "\\left|"+scalebracebody(fSize,eAlign)+"\\right.";
        }
        else if (curToken.eType!=Token.RIGHT && curToken.eType!=Token.END){
            return align(fSize,eAlign,false,false)+scalebracebody(fSize,eAlign);
        }
        else {  // Finished recursion
            return "";
        }
    }

    private String bracebody(float fSize, Token eAlign){
        if (curToken.eType==Token.MLINE){
            nextToken();
            return "|"+bracebody(fSize,eAlign);
        }
        else if (!tokenInGroup(TGroup.RBRACES) && curToken.eType!=Token.END){
            return align(fSize,eAlign,false,false)+bracebody(fSize,eAlign);
        }
        else { // Finished recursion
            return "";
        }
    }

    private String function(){
        String sFunction;
        if (curToken.eType==Token.FUNC){
            nextToken();
            if (curToken.eType==Token.IDENT){
                sFunction="\\operatorname{"+curToken.sLaTeX+"}";
                nextToken();
            }
            else { // error in formula
                sFunction="";
            }
        }
        else {
            sFunction=curToken.sLaTeX;
            nextToken();
        }
        return sFunction;
    }

    private String binom(float fSize, Token eAlign){
        nextToken();
        return "\\genfrac{}{}{0pt}{0}{"+sum(fSize,eAlign)+"}{"+sum(fSize,eAlign)+"}";
    }

    private String stack(float fSize, Token eAlign){
        nextToken();
        if (curToken.eType==Token.LGROUP){
            StringBuffer bufStack=new StringBuffer().append("\\begin{matrix}");
            do {
                nextToken();
                bufStack.append(align(fSize,eAlign,true,true));
                if (curToken.eType==Token.POUND) bufStack.append("\\\\");
            } while (curToken.eType==Token.POUND);
            if (curToken.eType==Token.RGROUP) nextToken(); // otherwise error in formula - ignore
            return bufStack.append("\\end{matrix}").toString();
        }
        else { // error in formula
            return "";
        }
    }

    private String matrix(float fSize, Token eAlign){
        nextToken();
        if (curToken.eType==Token.LGROUP){
            StringBuffer bufMatrix=new StringBuffer().append("\\begin{matrix}");
            do {
                nextToken();
                bufMatrix.append(align(fSize,eAlign,true,true));
                if (curToken.eType==Token.POUND) bufMatrix.append("&");
                else if (curToken.eType==Token.DPOUND) bufMatrix.append("\\\\");
            } while (curToken.eType==Token.POUND || curToken.eType==Token.DPOUND);
            if (curToken.eType==Token.RGROUP) nextToken(); // otherwise error in formula- ignore
            return bufMatrix.append("\\end{matrix}").toString();
        }
        else { // error in formula
            return "";
        }
    }

    private String special() {
        String sSpecial=curToken.sLaTeX;
        nextToken();
        return sSpecial;
    }


    ////////////////////////////////////////////////
    // Finally, the converter itself
    public String convert(String sStarMath){
        String sExport="";
        buffer=new SimpleInputBuffer(sStarMath);
        nextToken();
        sExport=table(12.0F,Token.ALIGNC);
        return sExport.length()==0 ? " " : sExport; // don't return an empty formula!
        //System.out.println(sExport);
    }
}



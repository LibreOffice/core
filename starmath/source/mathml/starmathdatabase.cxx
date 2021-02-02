/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <starmathdatabase.hxx>

SmToken starmathdatabase::Identify_SmXMLOperatorContext_Impl(sal_Unicode cChar, bool bIsStretchy)
{
    switch (cChar)
    {
        case MS_COPROD:
            return SmToken(TCOPROD, MS_COPROD, "coprod", TG::Oper, 5);
        case MS_IIINT:
            return SmToken(TIIINT, MS_IIINT, "iiint", TG::Oper, 5);
        case MS_IINT:
            return SmToken(TIINT, MS_IINT, "iint", TG::Oper, 5);
        case MS_INT:
            if (bIsStretchy)
                return SmToken(TINTD, MS_INT, "intd", TG::Oper, 5);
            else
                return SmToken(TINT, MS_INT, "int", TG::Oper, 5);
        case MS_LINT:
            return SmToken(TLINT, MS_LINT, "lint", TG::Oper, 5);
        case MS_LLINT:
            return SmToken(TLLINT, MS_LLINT, "llint", TG::Oper, 5);
        case MS_LLLINT:
            return SmToken(TLLLINT, MS_LLLINT, "lllint", TG::Oper, 5);
        case MS_PROD:
            return SmToken(TPROD, MS_PROD, "prod", TG::Oper, 5);
        case MS_SUM:
            return SmToken(TSUM, MS_SUM, "sum", TG::Oper, 5);
        case MS_FACT:
            return SmToken(TFACT, MS_FACT, "!", TG::UnOper, 5);
        case MS_NEG:
            return SmToken(TNEG, MS_NEG, "neg", TG::UnOper, 5);
        case MS_OMINUS:
            return SmToken(TOMINUS, MS_OMINUS, "ominus", TG::Sum, 0);
        case MS_OPLUS:
            return SmToken(TOPLUS, MS_OPLUS, "oplus", TG::Sum, 0);
        case MS_UNION:
            return SmToken(TUNION, MS_UNION, "union", TG::Sum, 0);
        case MS_OR:
            return SmToken(TOR, MS_OR, "|", TG::Sum, 5);
        case MS_PLUSMINUS:
            return SmToken(TPLUSMINUS, MS_PLUSMINUS, "+-", TG::Sum | TG::UnOper, 5);
        case MS_MINUSPLUS:
            return SmToken(TMINUSPLUS, MS_MINUSPLUS, "-+", TG::Sum | TG::UnOper, 5);
        case 0xe083:
        case MS_PLUS:
            return SmToken(TPLUS, MS_PLUS, "+", TG::Sum | TG::UnOper, 5);
        case MS_MINUS:
            return SmToken(TMINUS, MS_MINUS, "-", TG::Sum | TG::UnOper, 5);
        case 0x2022:
        case MS_CDOT:
            return SmToken(TCDOT, MS_CDOT, "cdot", TG::Product, 0);
        case MS_DIV:
            return SmToken(TDIV, MS_DIV, "div", TG::Product, 0);
        case MS_TIMES:
            return SmToken(TTIMES, MS_TIMES, "times", TG::Product, 0);
        case MS_INTERSECT:
            return SmToken(TINTERSECT, MS_INTERSECT, "intersection", TG::Product, 0);
        case MS_ODIVIDE:
            return SmToken(TODIVIDE, MS_ODIVIDE, "odivide", TG::Product, 0);
        case MS_ODOT:
            return SmToken(TODOT, MS_ODOT, "odot", TG::Product, 0);
        case MS_OTIMES:
            return SmToken(TOTIMES, MS_OTIMES, "otimes", TG::Product, 0);
        case MS_AND:
            return SmToken(TAND, MS_AND, "&", TG::Product, 0);
        case MS_MULTIPLY:
            return SmToken(TMULTIPLY, MS_MULTIPLY, "*", TG::Product, 0);
        case MS_SLASH:
            if (bIsStretchy)
                return SmToken(TWIDESLASH, MS_SLASH, "wideslash", TG::Product, 0);
            else
                return SmToken(TSLASH, MS_SLASH, "slash", TG::Product, 0);
        case MS_BACKSLASH:
            if (bIsStretchy)
                return SmToken(TWIDEBACKSLASH, MS_BACKSLASH, "bslash", TG::Product, 0);
            else
                return SmToken(TBACKSLASH, MS_BACKSLASH, "slash", TG::Product, 0);
        case MS_DEF:
            return SmToken(TDEF, MS_DEF, "def", TG::Relation, 0);
        case MS_LINE:
            return SmToken(TDIVIDES, MS_LINE, "divides", TG::Relation, 0);
        case MS_EQUIV:
            return SmToken(TEQUIV, MS_EQUIV, "equiv", TG::Relation, 0);
        case MS_GE:
            return SmToken(TGE, MS_GE, ">=", TG::Relation, 0);
        case MS_GESLANT:
            return SmToken(TGESLANT, MS_GESLANT, "geslant", TG::Relation, 0);
        case MS_GG:
            return SmToken(TGG, MS_GG, ">>", TG::Relation, 0);
        case MS_GT:
            return SmToken(TGT, MS_GT, ">", TG::Relation, 0);
        case MS_IN:
            return SmToken(TIN, MS_IN, "in", TG::Relation, 0);
        case MS_LE:
            return SmToken(TLE, MS_LE, "<=", TG::Relation, 0);
        case MS_LESLANT:
            return SmToken(TLESLANT, MS_LESLANT, "leslant", TG::Relation, 0);
        case MS_LL:
            return SmToken(TLL, MS_LL, "<<", TG::Relation, 0);
        case MS_LT:
            return SmToken(TLT, MS_LT, "<", TG::Relation, 0);
        case MS_NDIVIDES:
            return SmToken(TNDIVIDES, MS_NDIVIDES, "ndivides", TG::Relation, 0);
        case MS_NEQ:
            return SmToken(TNEQ, MS_NEQ, "<>", TG::Relation, 0);
        case MS_NOTIN:
            return SmToken(TNOTIN, MS_NOTIN, "notin", TG::Relation, 0);
        case MS_NOTPRECEDES:
            return SmToken(TNOTPRECEDES, MS_NOTPRECEDES, "nprec", TG::Relation, 0);
        case MS_NSUBSET:
            return SmToken(TNSUBSET, MS_NSUBSET, "nsubset", TG::Relation, 0);
        case MS_NSUBSETEQ:
            return SmToken(TNSUBSETEQ, MS_NSUBSETEQ, "nsubseteq", TG::Relation, 0);
        case MS_NOTSUCCEEDS:
            return SmToken(TNOTSUCCEEDS, MS_NOTSUCCEEDS, "nsucc", TG::Relation, 0);
        case MS_NSUPSET:
            return SmToken(TNSUPSET, MS_NSUPSET, "nsupset", TG::Relation, 0);
        case MS_NSUPSETEQ:
            return SmToken(TNSUPSETEQ, MS_NSUPSETEQ, "nsupseteq", TG::Relation, 0);
        case MS_ORTHO:
            return SmToken(TORTHO, MS_ORTHO, "ortho", TG::Relation, 0);
        case MS_NI:
            return SmToken(TNI, MS_NI, "owns", TG::Relation, 0);
        case MS_DLINE:
            return SmToken(TPARALLEL, MS_DLINE, "parallel", TG::Relation, 0);
        case MS_PRECEDES:
            return SmToken(TPRECEDES, MS_PRECEDES, "prec", TG::Relation, 0);
        case MS_PRECEDESEQUAL:
            return SmToken(TPRECEDESEQUAL, MS_PRECEDESEQUAL, "preccurlyeq", TG::Relation, 0);
        case MS_PRECEDESEQUIV:
            return SmToken(TPRECEDESEQUIV, MS_PRECEDESEQUIV, "precsim", TG::Relation, 0);
        case MS_PROP:
            return SmToken(TPROP, MS_PROP, "prop", TG::Relation, 0);
        case MS_SIM:
            return SmToken(TSIM, MS_SIM, "sim", TG::Relation, 0);
        case 0x2245:
        case MS_SIMEQ:
            return SmToken(TSIMEQ, MS_SIMEQ, "simeq", TG::Relation, 0);
        case MS_SUBSET:
            return SmToken(TSUBSET, MS_SUBSET, "subset", TG::Relation, 0);
        case MS_SUBSETEQ:
            return SmToken(TSUBSETEQ, MS_SUBSETEQ, "subseteq", TG::Relation, 0);
        case MS_SUCCEEDS:
            return SmToken(TSUCCEEDS, MS_SUCCEEDS, "succ", TG::Relation, 0);
        case MS_SUCCEEDSEQUAL:
            return SmToken(TSUCCEEDSEQUAL, MS_SUCCEEDSEQUAL, "succcurlyeq", TG::Relation, 0);
        case MS_SUCCEEDSEQUIV:
            return SmToken(TSUCCEEDSEQUIV, MS_SUCCEEDSEQUIV, "succsim", TG::Relation, 0);
        case MS_SUPSET:
            return SmToken(TSUPSET, MS_SUPSET, "supset", TG::Relation, 0);
        case MS_SUPSETEQ:
            return SmToken(TSUPSETEQ, MS_SUPSETEQ, "supseteq", TG::Relation, 0);
        case MS_RIGHTARROW:
            return SmToken(TTOWARD, MS_RIGHTARROW, "toward", TG::Relation, 0);
        case MS_TRANSL:
            return SmToken(TTRANSL, MS_TRANSL, "transl", TG::Relation, 0);
        case MS_TRANSR:
            return SmToken(TTRANSR, MS_TRANSR, "transr", TG::Relation, 0);
        case MS_ASSIGN:
            return SmToken(TASSIGN, MS_ASSIGN, "=", TG::Relation, 0);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, "lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, "lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, "lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, "ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, "[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, "(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, "rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, "rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, "rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, "rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, "]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, ")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, "none", TG::RBrace | TG::LBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, "", TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_Prefix_SmXMLOperatorContext_Impl(sal_Unicode cChar)
{
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TLLINE, MS_VERTLINE, "lline", TG::LBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TLDLINE, MS_DVERTLINE, "ldline", TG::LBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, "lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, "lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, "lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, "ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, "[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, "(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, "rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, "rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, "rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, "rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, "]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, ")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, "none", TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, "", TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_Postfix_SmXMLOperatorContext_Impl(sal_Unicode cChar)
{
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TRLINE, MS_VERTLINE, "rline", TG::RBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TRDLINE, MS_DVERTLINE, "rdline", TG::RBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, "lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, "lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, "lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, "ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, "[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, "(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, "rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, "rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, "rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, "rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, "]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, ")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, "none", TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, "", TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_PrefixPostfix_SmXMLOperatorContext_Impl(sal_Unicode cChar)
{
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TLRLINE, MS_VERTLINE, "lrline", TG::LBrace | TG::RBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TLRDLINE, MS_DVERTLINE, "lrdline", TG::LBrace | TG::RBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, "langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, "lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, "lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, "lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, "ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, "[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, "(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, "rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, "rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, "rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, "rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, "rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, "]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, ")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, "none", TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, "", TG::NONE, SAL_MAX_UINT16);
    }
}

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableParse[]
    = { { "aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { "antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { "aqua", TMATHMLCOL, COL_SM_AQUA },
        { "aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { "azure", THTMLCOL, COL_SM_AZURE },
        { "beige", THTMLCOL, COL_SM_BEIGE },
        { "bisque", THTMLCOL, COL_SM_BISQUE },
        { "black", TMATHMLCOL, COL_SM_BLACK },
        { "blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { "blue", TMATHMLCOL, COL_SM_BLUE },
        { "blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { "brown", THTMLCOL, COL_SM_BROWN },
        { "burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { "cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { "chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { "chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { "coral", THTMLCOL, COL_SM_CORAL },
        { "cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { "cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { "crimson", THTMLCOL, COL_SM_CRIMSON },
        { "cyan", TMATHMLCOL, COL_SM_CYAN },
        { "darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { "darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { "darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { "darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { "darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { "darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { "darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { "darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { "darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { "darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { "darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { "darkred", THTMLCOL, COL_SM_DARKRED },
        { "darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { "darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { "darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { "darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { "darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { "darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { "darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { "debian", TICONICCOL, COL_SM_DEBIAN_MAGENTA },
        { "deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { "deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { "dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { "dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { "dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { "dvip", TDVIPSNAMESCOL, COL_SM_BLACK },
        { "firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { "floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { "forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { "fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { "gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { "ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { "gold", THTMLCOL, COL_SM_GOLD },
        { "goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { "gray", TMATHMLCOL, COL_SM_GRAY },
        { "green", TMATHMLCOL, COL_SM_GREEN },
        { "greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { "grey", THTMLCOL, COL_SM_GREY },
        { "hex", THEX, COL_SM_BLACK },
        { "honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { "hotpink", THTMLCOL, COL_SM_HOTPINK },
        { "indianred", THTMLCOL, COL_SM_INDIANRED },
        { "indigo", THTMLCOL, COL_SM_INDIGO },
        { "ivory", THTMLCOL, COL_SM_IVORY },
        { "khaki", THTMLCOL, COL_SM_KHAKI },
        { "lavender", THTMLCOL, COL_SM_LAVENDER },
        { "lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { "lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { "lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { "lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { "lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { "lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { "lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { "lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { "lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { "lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { "lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { "lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { "lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { "lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { "lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { "lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { "lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { "lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { "lime", TMATHMLCOL, COL_SM_LIME },
        { "limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { "linen", THTMLCOL, COL_SM_LINEN },
        { "lo", TICONICCOL, COL_SM_LO_GREEN },
        { "magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { "maroon", TMATHMLCOL, COL_SM_MAROON },
        { "mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { "mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { "mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { "mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { "mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { "mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { "mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { "mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { "mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { "midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { "mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { "mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { "moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { "navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { "navy", TMATHMLCOL, COL_SM_NAVY },
        { "oldlace", THTMLCOL, COL_SM_OLDLACE },
        { "olive", TMATHMLCOL, COL_SM_OLIVE },
        { "olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { "orange", THTMLCOL, COL_SM_ORANGE },
        { "orangered", THTMLCOL, COL_SM_ORANGERED },
        { "orchid", THTMLCOL, COL_SM_ORCHID },
        { "palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { "palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { "paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { "palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { "papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { "peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { "per", THTMLCOL, COL_SM_PERU },
        { "pink", THTMLCOL, COL_SM_PINK },
        { "plum", THTMLCOL, COL_SM_PLUM },
        { "powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { "purple", TMATHMLCOL, COL_SM_PURPLE },
        { "rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { "red", TMATHMLCOL, COL_SM_RED },
        { "rgb", TRGB, COL_AUTO },
        { "rgba", TRGBA, COL_AUTO },
        { "rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { "royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { "saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { "salmon", THTMLCOL, COL_SM_SALMON },
        { "sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { "seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { "seashell", THTMLCOL, COL_SM_SEASHELL },
        { "sienna", THTMLCOL, COL_SM_SIENNA },
        { "silver", TMATHMLCOL, COL_SM_SILVER },
        { "skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { "slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { "slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { "slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { "snow", THTMLCOL, COL_SM_SNOW },
        { "springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { "steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { "tan", THTMLCOL, COL_SM_TAN },
        { "teal", TMATHMLCOL, COL_SM_TEAL },
        { "thistle", THTMLCOL, COL_SM_THISTLE },
        { "tomato", THTMLCOL, COL_SM_TOMATO },
        { "turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { "ubunt", TICONICCOL, COL_SM_UBUNTU_ORANGE },
        { "violet", THTMLCOL, COL_SM_VIOLET },
        { "wheat", THTMLCOL, COL_SM_WHEAT },
        { "white", TMATHMLCOL, COL_SM_WHITE },
        { "whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { "yellow", TMATHMLCOL, COL_SM_YELLOW },
        { "yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableHTML[]
    = { { "aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { "antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { "aqua", TMATHMLCOL, COL_SM_AQUA },
        { "aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { "azure", THTMLCOL, COL_SM_AZURE },
        { "beige", THTMLCOL, COL_SM_BEIGE },
        { "bisque", THTMLCOL, COL_SM_BISQUE },
        { "black", TMATHMLCOL, COL_SM_BLACK },
        { "blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { "blue", TMATHMLCOL, COL_SM_BLUE },
        { "blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { "brown", THTMLCOL, COL_SM_BROWN },
        { "burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { "cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { "chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { "chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { "coral", THTMLCOL, COL_SM_CORAL },
        { "cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { "cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { "crimson", THTMLCOL, COL_SM_CRIMSON },
        { "cyan", TMATHMLCOL, COL_SM_CYAN },
        { "darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { "darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { "darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { "darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { "darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { "darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { "darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { "darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { "darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { "darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { "darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { "darkred", THTMLCOL, COL_SM_DARKRED },
        { "darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { "darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { "darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { "darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { "darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { "darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { "darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { "deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { "deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { "dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { "dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { "dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { "firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { "floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { "forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { "fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { "gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { "ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { "gold", THTMLCOL, COL_SM_GOLD },
        { "goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { "gray", TMATHMLCOL, COL_SM_GRAY },
        { "green", TMATHMLCOL, COL_SM_GREEN },
        { "greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { "grey", THTMLCOL, COL_SM_GREY },
        { "honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { "hotpink", THTMLCOL, COL_SM_HOTPINK },
        { "indianred", THTMLCOL, COL_SM_INDIANRED },
        { "indigo", THTMLCOL, COL_SM_INDIGO },
        { "ivory", THTMLCOL, COL_SM_IVORY },
        { "khaki", THTMLCOL, COL_SM_KHAKI },
        { "lavender", THTMLCOL, COL_SM_LAVENDER },
        { "lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { "lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { "lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { "lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { "lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { "lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { "lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { "lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { "lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { "lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { "lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { "lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { "lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { "lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { "lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { "lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { "lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { "lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { "lime", TMATHMLCOL, COL_SM_LIME },
        { "limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { "linen", THTMLCOL, COL_SM_LINEN },
        { "magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { "maroon", TMATHMLCOL, COL_SM_MAROON },
        { "mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { "mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { "mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { "mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { "mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { "mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { "mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { "mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { "mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { "midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { "mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { "mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { "moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { "navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { "navy", TMATHMLCOL, COL_SM_NAVY },
        { "oldlace", THTMLCOL, COL_SM_OLDLACE },
        { "olive", TMATHMLCOL, COL_SM_OLIVE },
        { "olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { "orange", THTMLCOL, COL_SM_ORANGE },
        { "orangered", THTMLCOL, COL_SM_ORANGERED },
        { "orchid", THTMLCOL, COL_SM_ORCHID },
        { "palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { "palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { "paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { "palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { "papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { "peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { "per", THTMLCOL, COL_SM_PERU },
        { "pink", THTMLCOL, COL_SM_PINK },
        { "plum", THTMLCOL, COL_SM_PLUM },
        { "powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { "purple", TMATHMLCOL, COL_SM_PURPLE },
        { "rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { "red", TMATHMLCOL, COL_SM_RED },
        { "rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { "royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { "saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { "salmon", THTMLCOL, COL_SM_SALMON },
        { "sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { "seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { "seashell", THTMLCOL, COL_SM_SEASHELL },
        { "sienna", THTMLCOL, COL_SM_SIENNA },
        { "silver", TMATHMLCOL, COL_SM_SILVER },
        { "skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { "slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { "slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { "slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { "snow", THTMLCOL, COL_SM_SNOW },
        { "springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { "steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { "tan", THTMLCOL, COL_SM_TAN },
        { "teal", TMATHMLCOL, COL_SM_TEAL },
        { "thistle", THTMLCOL, COL_SM_THISTLE },
        { "tomato", THTMLCOL, COL_SM_TOMATO },
        { "turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { "violet", THTMLCOL, COL_SM_VIOLET },
        { "wheat", THTMLCOL, COL_SM_WHEAT },
        { "white", TMATHMLCOL, COL_SM_WHITE },
        { "whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { "yellow", TMATHMLCOL, COL_SM_YELLOW },
        { "yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableDVIPS[]
    = { { "apricot", TDVIPSNAMESCOL, COL_SM_DIV_APRICOT },
        { "aquamarine", TDVIPSNAMESCOL, COL_SM_DIV_AQUAMARINE },
        { "bittersweet", TDVIPSNAMESCOL, COL_SM_DIV_BITTERSWEET },
        { "black", TDVIPSNAMESCOL, COL_SM_BLACK },
        { "blue", TDVIPSNAMESCOL, COL_SM_BLACK } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableMATHML[] = {
    // clang-format off
        { "aqua", TMATHMLCOL, COL_SM_AQUA },
        { "black", TMATHMLCOL, COL_SM_BLACK },
        { "blue", TMATHMLCOL, COL_SM_BLUE },
        { "fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { "gray", TMATHMLCOL, COL_SM_GRAY },
        { "green", TMATHMLCOL, COL_SM_GREEN },
        { "lime", TMATHMLCOL, COL_SM_LIME },
        { "maroon", TMATHMLCOL, COL_SM_MAROON },
        { "navy", TMATHMLCOL, COL_SM_NAVY },
        { "olive", TMATHMLCOL, COL_SM_OLIVE },
        { "purple", TMATHMLCOL, COL_SM_PURPLE },
        { "red", TMATHMLCOL, COL_SM_RED },
        { "silver", TMATHMLCOL, COL_SM_SILVER },
        { "teal", TMATHMLCOL, COL_SM_TEAL },
        { "white", TMATHMLCOL, COL_SM_WHITE },
        { "yellow", TMATHMLCOL, COL_SM_YELLOW }
    // clang-format on
};

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_Parser(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_HTML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_MATHML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableMATHML); i < std::end(aColorTokenTableMATHML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_Color_DVIPSNAMES(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_Parser(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry("", TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_HTML(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if (colorname[0] == '#')
    {
        Color col = Color::STRtoRGB(colorname);
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry("", TRGB, col));
    }
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry("", TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_DVIPSNAMES(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry("", TERROR, COL_SM_BLACK));
}

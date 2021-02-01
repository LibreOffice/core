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
    = { { u"aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { u"antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { u"aqua", TMATHMLCOL, COL_SM_AQUA },
        { u"aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { u"azure", THTMLCOL, COL_SM_AZURE },
        { u"beige", THTMLCOL, COL_SM_BEIGE },
        { u"bisque", THTMLCOL, COL_SM_BISQUE },
        { u"black", TMATHMLCOL, COL_SM_BLACK },
        { u"blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { u"blue", TMATHMLCOL, COL_SM_BLUE },
        { u"blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { u"brown", THTMLCOL, COL_SM_BROWN },
        { u"burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { u"cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { u"chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { u"chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { u"coral", THTMLCOL, COL_SM_CORAL },
        { u"cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { u"cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { u"crimson", THTMLCOL, COL_SM_CRIMSON },
        { u"cyan", TMATHMLCOL, COL_SM_CYAN },
        { u"darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { u"darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { u"darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { u"darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { u"darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { u"darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { u"darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { u"darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { u"darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { u"darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { u"darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { u"darkred", THTMLCOL, COL_SM_DARKRED },
        { u"darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { u"darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { u"darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { u"darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { u"darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { u"darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { u"darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { u"debian", TICONICCOL, COL_SM_DEBIAN_MAGENTA },
        { u"deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { u"deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { u"dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { u"dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { u"dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { u"dvip", TDVIPSNAMESCOL, COL_SM_BLACK },
        { u"firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { u"floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { u"forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { u"fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { u"ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { u"gold", THTMLCOL, COL_SM_GOLD },
        { u"goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { u"gray", TMATHMLCOL, COL_SM_GRAY },
        { u"green", TMATHMLCOL, COL_SM_GREEN },
        { u"greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { u"grey", THTMLCOL, COL_SM_GREY },
        { u"hex", THEX, COL_SM_BLACK },
        { u"honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { u"hotpink", THTMLCOL, COL_SM_HOTPINK },
        { u"indianred", THTMLCOL, COL_SM_INDIANRED },
        { u"indigo", THTMLCOL, COL_SM_INDIGO },
        { u"ivory", THTMLCOL, COL_SM_IVORY },
        { u"khaki", THTMLCOL, COL_SM_KHAKI },
        { u"lavender", THTMLCOL, COL_SM_LAVENDER },
        { u"lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { u"lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { u"lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { u"lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { u"lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { u"lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { u"lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { u"lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { u"lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { u"lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { u"lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { u"lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { u"lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { u"lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { u"lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { u"lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { u"lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { u"lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { u"lime", TMATHMLCOL, COL_SM_LIME },
        { u"limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { u"linen", THTMLCOL, COL_SM_LINEN },
        { u"lo", TICONICCOL, COL_SM_LO_GREEN },
        { u"magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { u"maroon", TMATHMLCOL, COL_SM_MAROON },
        { u"mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { u"mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { u"mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { u"mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { u"mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { u"mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { u"mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { u"mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { u"mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { u"midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { u"mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { u"mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { u"moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { u"navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { u"navy", TMATHMLCOL, COL_SM_NAVY },
        { u"oldlace", THTMLCOL, COL_SM_OLDLACE },
        { u"olive", TMATHMLCOL, COL_SM_OLIVE },
        { u"olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { u"orange", THTMLCOL, COL_SM_ORANGE },
        { u"orangered", THTMLCOL, COL_SM_ORANGERED },
        { u"orchid", THTMLCOL, COL_SM_ORCHID },
        { u"palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { u"palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { u"paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { u"palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { u"papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { u"peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { u"peru", THTMLCOL, COL_SM_PERU },
        { u"pink", THTMLCOL, COL_SM_PINK },
        { u"plum", THTMLCOL, COL_SM_PLUM },
        { u"powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { u"purple", TMATHMLCOL, COL_SM_PURPLE },
        { u"rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { u"red", TMATHMLCOL, COL_SM_RED },
        { u"rgb", TRGB, COL_AUTO },
        { u"rgba", TRGBA, COL_AUTO },
        { u"rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { u"royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { u"saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { u"salmon", THTMLCOL, COL_SM_SALMON },
        { u"sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { u"seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { u"seashell", THTMLCOL, COL_SM_SEASHELL },
        { u"sienna", THTMLCOL, COL_SM_SIENNA },
        { u"silver", TMATHMLCOL, COL_SM_SILVER },
        { u"skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { u"slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { u"slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { u"slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { u"snow", THTMLCOL, COL_SM_SNOW },
        { u"springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { u"steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { u"tan", THTMLCOL, COL_SM_TAN },
        { u"teal", TMATHMLCOL, COL_SM_TEAL },
        { u"thistle", THTMLCOL, COL_SM_THISTLE },
        { u"tomato", THTMLCOL, COL_SM_TOMATO },
        { u"turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { u"ubuntu", TICONICCOL, COL_SM_UBUNTU_ORANGE },
        { u"violet", THTMLCOL, COL_SM_VIOLET },
        { u"wheat", THTMLCOL, COL_SM_WHEAT },
        { u"white", TMATHMLCOL, COL_SM_WHITE },
        { u"whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { u"yellow", TMATHMLCOL, COL_SM_YELLOW },
        { u"yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableHTML[]
    = { { u"aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { u"antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { u"aqua", TMATHMLCOL, COL_SM_AQUA },
        { u"aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { u"azure", THTMLCOL, COL_SM_AZURE },
        { u"beige", THTMLCOL, COL_SM_BEIGE },
        { u"bisque", THTMLCOL, COL_SM_BISQUE },
        { u"black", TMATHMLCOL, COL_SM_BLACK },
        { u"blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { u"blue", TMATHMLCOL, COL_SM_BLUE },
        { u"blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { u"brown", THTMLCOL, COL_SM_BROWN },
        { u"burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { u"cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { u"chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { u"chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { u"coral", THTMLCOL, COL_SM_CORAL },
        { u"cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { u"cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { u"crimson", THTMLCOL, COL_SM_CRIMSON },
        { u"cyan", TMATHMLCOL, COL_SM_CYAN },
        { u"darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { u"darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { u"darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { u"darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { u"darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { u"darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { u"darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { u"darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { u"darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { u"darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { u"darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { u"darkred", THTMLCOL, COL_SM_DARKRED },
        { u"darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { u"darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { u"darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { u"darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { u"darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { u"darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { u"darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { u"deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { u"deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { u"dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { u"dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { u"dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { u"firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { u"floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { u"forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { u"fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { u"ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { u"gold", THTMLCOL, COL_SM_GOLD },
        { u"goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { u"gray", TMATHMLCOL, COL_SM_GRAY },
        { u"green", TMATHMLCOL, COL_SM_GREEN },
        { u"greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { u"grey", THTMLCOL, COL_SM_GREY },
        { u"honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { u"hotpink", THTMLCOL, COL_SM_HOTPINK },
        { u"indianred", THTMLCOL, COL_SM_INDIANRED },
        { u"indigo", THTMLCOL, COL_SM_INDIGO },
        { u"ivory", THTMLCOL, COL_SM_IVORY },
        { u"khaki", THTMLCOL, COL_SM_KHAKI },
        { u"lavender", THTMLCOL, COL_SM_LAVENDER },
        { u"lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { u"lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { u"lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { u"lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { u"lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { u"lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { u"lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { u"lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { u"lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { u"lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { u"lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { u"lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { u"lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { u"lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { u"lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { u"lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { u"lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { u"lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { u"lime", TMATHMLCOL, COL_SM_LIME },
        { u"limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { u"linen", THTMLCOL, COL_SM_LINEN },
        { u"magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { u"maroon", TMATHMLCOL, COL_SM_MAROON },
        { u"mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { u"mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { u"mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { u"mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { u"mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { u"mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { u"mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { u"mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { u"mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { u"midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { u"mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { u"mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { u"moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { u"navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { u"navy", TMATHMLCOL, COL_SM_NAVY },
        { u"oldlace", THTMLCOL, COL_SM_OLDLACE },
        { u"olive", TMATHMLCOL, COL_SM_OLIVE },
        { u"olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { u"orange", THTMLCOL, COL_SM_ORANGE },
        { u"orangered", THTMLCOL, COL_SM_ORANGERED },
        { u"orchid", THTMLCOL, COL_SM_ORCHID },
        { u"palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { u"palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { u"paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { u"palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { u"papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { u"peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { u"peru", THTMLCOL, COL_SM_PERU },
        { u"pink", THTMLCOL, COL_SM_PINK },
        { u"plum", THTMLCOL, COL_SM_PLUM },
        { u"powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { u"purple", TMATHMLCOL, COL_SM_PURPLE },
        { u"rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { u"red", TMATHMLCOL, COL_SM_RED },
        { u"rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { u"royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { u"saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { u"salmon", THTMLCOL, COL_SM_SALMON },
        { u"sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { u"seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { u"seashell", THTMLCOL, COL_SM_SEASHELL },
        { u"sienna", THTMLCOL, COL_SM_SIENNA },
        { u"silver", TMATHMLCOL, COL_SM_SILVER },
        { u"skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { u"slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { u"slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { u"slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { u"snow", THTMLCOL, COL_SM_SNOW },
        { u"springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { u"steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { u"tan", THTMLCOL, COL_SM_TAN },
        { u"teal", TMATHMLCOL, COL_SM_TEAL },
        { u"thistle", THTMLCOL, COL_SM_THISTLE },
        { u"tomato", THTMLCOL, COL_SM_TOMATO },
        { u"turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { u"violet", THTMLCOL, COL_SM_VIOLET },
        { u"wheat", THTMLCOL, COL_SM_WHEAT },
        { u"white", TMATHMLCOL, COL_SM_WHITE },
        { u"whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { u"yellow", TMATHMLCOL, COL_SM_YELLOW },
        { u"yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableDVIPS[]
    = { { u"apricot", TDVIPSNAMESCOL, COL_SM_DIV_APRICOT },
        { u"aquamarine", TDVIPSNAMESCOL, COL_SM_DIV_AQUAMARINE },
        { u"bittersweet", TDVIPSNAMESCOL, COL_SM_DIV_BITTERSWEET },
        { u"black", TDVIPSNAMESCOL, COL_SM_BLACK },
        { u"blue", TDVIPSNAMESCOL, COL_SM_BLACK } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableMATHML[] = {
    // clang-format off
        { u"aqua", TMATHMLCOL, COL_SM_AQUA },
        { u"black", TMATHMLCOL, COL_SM_BLACK },
        { u"blue", TMATHMLCOL, COL_SM_BLUE },
        { u"fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gray", TMATHMLCOL, COL_SM_GRAY },
        { u"green", TMATHMLCOL, COL_SM_GREEN },
        { u"lime", TMATHMLCOL, COL_SM_LIME },
        { u"maroon", TMATHMLCOL, COL_SM_MAROON },
        { u"navy", TMATHMLCOL, COL_SM_NAVY },
        { u"olive", TMATHMLCOL, COL_SM_OLIVE },
        { u"purple", TMATHMLCOL, COL_SM_PURPLE },
        { u"red", TMATHMLCOL, COL_SM_RED },
        { u"silver", TMATHMLCOL, COL_SM_SILVER },
        { u"teal", TMATHMLCOL, COL_SM_TEAL },
        { u"white", TMATHMLCOL, COL_SM_WHITE },
        { u"yellow", TMATHMLCOL, COL_SM_YELLOW }
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
            new SmColorTokenTableEntry(u"", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_HTML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_MATHML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableMATHML); i < std::end(aColorTokenTableMATHML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_Color_DVIPSNAMES(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_Parser(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
    {
        sal_Int32 matches = colorname.compareTo(i->pIdent);
        if (matches == 0)
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
        if (matches > 0)
            break;
    }
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry(u"", TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_HTML(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
    {
        sal_Int32 matches = colorname.compareTo(i->pIdent);
        if (matches == 0)
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
        if (matches > 0)
            break;
    }
    if (colorname[0] == '#')
    {
        Color col = Color::STRtoRGB(colorname);
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(u"", TRGB, col));
    }
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry(u"", TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_DVIPSNAMES(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(u"", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
    {
        sal_Int32 matches = colorname.compareTo(i->pIdent);
        if (matches == 0)
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
        if (matches > 0)
            break;
    }
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry(u"", TERROR, COL_SM_BLACK));
}

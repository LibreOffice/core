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
            return SmToken(TCOPROD, MS_COPROD, u"coprod", TG::Oper, 5);
        case MS_IIINT:
            return SmToken(TIIINT, MS_IIINT, u"iiint", TG::Oper, 5);
        case MS_IINT:
            return SmToken(TIINT, MS_IINT, u"iint", TG::Oper, 5);
        case MS_INT:
            if (bIsStretchy)
                return SmToken(TINTD, MS_INT, u"intd", TG::Oper, 5);
            else
                return SmToken(TINT, MS_INT, u"int", TG::Oper, 5);
        case MS_LINT:
            return SmToken(TLINT, MS_LINT, u"lint", TG::Oper, 5);
        case MS_LLINT:
            return SmToken(TLLINT, MS_LLINT, u"llint", TG::Oper, 5);
        case MS_LLLINT:
            return SmToken(TLLLINT, MS_LLLINT, u"lllint", TG::Oper, 5);
        case MS_PROD:
            return SmToken(TPROD, MS_PROD, u"prod", TG::Oper, 5);
        case MS_SUM:
            return SmToken(TSUM, MS_SUM, u"sum", TG::Oper, 5);
        case MS_FACT:
            return SmToken(TFACT, MS_FACT, u"!", TG::UnOper, 5);
        case MS_NEG:
            return SmToken(TNEG, MS_NEG, u"neg", TG::UnOper, 5);
        case MS_OMINUS:
            return SmToken(TOMINUS, MS_OMINUS, u"ominus", TG::Sum, 0);
        case MS_OPLUS:
            return SmToken(TOPLUS, MS_OPLUS, u"oplus", TG::Sum, 0);
        case MS_UNION:
            return SmToken(TUNION, MS_UNION, u"union", TG::Sum, 0);
        case MS_OR:
            return SmToken(TOR, MS_OR, u"|", TG::Sum, 5);
        case MS_PLUSMINUS:
            return SmToken(TPLUSMINUS, MS_PLUSMINUS, u"+-", TG::Sum | TG::UnOper, 5);
        case MS_MINUSPLUS:
            return SmToken(TMINUSPLUS, MS_MINUSPLUS, u"-+", TG::Sum | TG::UnOper, 5);
        case 0xe083:
        case MS_PLUS:
            return SmToken(TPLUS, MS_PLUS, u"+", TG::Sum | TG::UnOper, 5);
        case MS_MINUS:
            return SmToken(TMINUS, MS_MINUS, u"-", TG::Sum | TG::UnOper, 5);
        case 0x2022:
        case MS_CDOT:
            return SmToken(TCDOT, MS_CDOT, u"cdot", TG::Product, 0);
        case MS_DIV:
            return SmToken(TDIV, MS_DIV, u"div", TG::Product, 0);
        case MS_TIMES:
            return SmToken(TTIMES, MS_TIMES, u"times", TG::Product, 0);
        case MS_INTERSECT:
            return SmToken(TINTERSECT, MS_INTERSECT, u"intersection", TG::Product, 0);
        case MS_ODIVIDE:
            return SmToken(TODIVIDE, MS_ODIVIDE, u"odivide", TG::Product, 0);
        case MS_ODOT:
            return SmToken(TODOT, MS_ODOT, u"odot", TG::Product, 0);
        case MS_OTIMES:
            return SmToken(TOTIMES, MS_OTIMES, u"otimes", TG::Product, 0);
        case MS_AND:
            return SmToken(TAND, MS_AND, u"&", TG::Product, 0);
        case MS_MULTIPLY:
            return SmToken(TMULTIPLY, MS_MULTIPLY, u"*", TG::Product, 0);
        case MS_SLASH:
            if (bIsStretchy)
                return SmToken(TWIDESLASH, MS_SLASH, u"wideslash", TG::Product, 0);
            else
                return SmToken(TSLASH, MS_SLASH, u"slash", TG::Product, 0);
        case MS_BACKSLASH:
            if (bIsStretchy)
                return SmToken(TWIDEBACKSLASH, MS_BACKSLASH, u"bslash", TG::Product, 0);
            else
                return SmToken(TBACKSLASH, MS_BACKSLASH, u"slash", TG::Product, 0);
        case MS_DEF:
            return SmToken(TDEF, MS_DEF, u"def", TG::Relation, 0);
        case MS_LINE:
            return SmToken(TDIVIDES, MS_LINE, u"divides", TG::Relation, 0);
        case MS_EQUIV:
            return SmToken(TEQUIV, MS_EQUIV, u"equiv", TG::Relation, 0);
        case MS_GE:
            return SmToken(TGE, MS_GE, u">=", TG::Relation, 0);
        case MS_GESLANT:
            return SmToken(TGESLANT, MS_GESLANT, u"geslant", TG::Relation, 0);
        case MS_GG:
            return SmToken(TGG, MS_GG, u">>", TG::Relation, 0);
        case MS_GT:
            return SmToken(TGT, MS_GT, u">", TG::Relation, 0);
        case MS_IN:
            return SmToken(TIN, MS_IN, u"in", TG::Relation, 0);
        case MS_LE:
            return SmToken(TLE, MS_LE, u"<=", TG::Relation, 0);
        case MS_LESLANT:
            return SmToken(TLESLANT, MS_LESLANT, u"leslant", TG::Relation, 0);
        case MS_LL:
            return SmToken(TLL, MS_LL, u"<<", TG::Relation, 0);
        case MS_LT:
            return SmToken(TLT, MS_LT, u"<", TG::Relation, 0);
        case MS_NDIVIDES:
            return SmToken(TNDIVIDES, MS_NDIVIDES, u"ndivides", TG::Relation, 0);
        case MS_NEQ:
            return SmToken(TNEQ, MS_NEQ, u"<>", TG::Relation, 0);
        case MS_NOTIN:
            return SmToken(TNOTIN, MS_NOTIN, u"notin", TG::Relation, 0);
        case MS_NOTPRECEDES:
            return SmToken(TNOTPRECEDES, MS_NOTPRECEDES, u"nprec", TG::Relation, 0);
        case MS_NSUBSET:
            return SmToken(TNSUBSET, MS_NSUBSET, u"nsubset", TG::Relation, 0);
        case MS_NSUBSETEQ:
            return SmToken(TNSUBSETEQ, MS_NSUBSETEQ, u"nsubseteq", TG::Relation, 0);
        case MS_NOTSUCCEEDS:
            return SmToken(TNOTSUCCEEDS, MS_NOTSUCCEEDS, u"nsucc", TG::Relation, 0);
        case MS_NSUPSET:
            return SmToken(TNSUPSET, MS_NSUPSET, u"nsupset", TG::Relation, 0);
        case MS_NSUPSETEQ:
            return SmToken(TNSUPSETEQ, MS_NSUPSETEQ, u"nsupseteq", TG::Relation, 0);
        case MS_ORTHO:
            return SmToken(TORTHO, MS_ORTHO, u"ortho", TG::Relation, 0);
        case MS_NI:
            return SmToken(TNI, MS_NI, u"owns", TG::Relation, 0);
        case MS_DLINE:
            return SmToken(TPARALLEL, MS_DLINE, u"parallel", TG::Relation, 0);
        case MS_PRECEDES:
            return SmToken(TPRECEDES, MS_PRECEDES, u"prec", TG::Relation, 0);
        case MS_PRECEDESEQUAL:
            return SmToken(TPRECEDESEQUAL, MS_PRECEDESEQUAL, u"preccurlyeq", TG::Relation, 0);
        case MS_PRECEDESEQUIV:
            return SmToken(TPRECEDESEQUIV, MS_PRECEDESEQUIV, u"precsim", TG::Relation, 0);
        case MS_PROP:
            return SmToken(TPROP, MS_PROP, u"prop", TG::Relation, 0);
        case MS_SIM:
            return SmToken(TSIM, MS_SIM, u"sim", TG::Relation, 0);
        case 0x2245:
        case MS_SIMEQ:
            return SmToken(TSIMEQ, MS_SIMEQ, u"simeq", TG::Relation, 0);
        case MS_SUBSET:
            return SmToken(TSUBSET, MS_SUBSET, u"subset", TG::Relation, 0);
        case MS_SUBSETEQ:
            return SmToken(TSUBSETEQ, MS_SUBSETEQ, u"subseteq", TG::Relation, 0);
        case MS_SUCCEEDS:
            return SmToken(TSUCCEEDS, MS_SUCCEEDS, u"succ", TG::Relation, 0);
        case MS_SUCCEEDSEQUAL:
            return SmToken(TSUCCEEDSEQUAL, MS_SUCCEEDSEQUAL, u"succcurlyeq", TG::Relation, 0);
        case MS_SUCCEEDSEQUIV:
            return SmToken(TSUCCEEDSEQUIV, MS_SUCCEEDSEQUIV, u"succsim", TG::Relation, 0);
        case MS_SUPSET:
            return SmToken(TSUPSET, MS_SUPSET, u"supset", TG::Relation, 0);
        case MS_SUPSETEQ:
            return SmToken(TSUPSETEQ, MS_SUPSETEQ, u"supseteq", TG::Relation, 0);
        case MS_RIGHTARROW:
            return SmToken(TTOWARD, MS_RIGHTARROW, u"toward", TG::Relation, 0);
        case MS_TRANSL:
            return SmToken(TTRANSL, MS_TRANSL, u"transl", TG::Relation, 0);
        case MS_TRANSR:
            return SmToken(TTRANSR, MS_TRANSR, u"transr", TG::Relation, 0);
        case MS_ASSIGN:
            return SmToken(TASSIGN, MS_ASSIGN, u"=", TG::Relation, 0);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none", TG::RBrace | TG::LBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u"", TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_Prefix_SmXMLOperatorContext_Impl(sal_Unicode cChar)
{
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TLLINE, MS_VERTLINE, u"lline", TG::LBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TLDLINE, MS_DVERTLINE, u"ldline", TG::LBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none", TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u"", TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_Postfix_SmXMLOperatorContext_Impl(sal_Unicode cChar)
{
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TRLINE, MS_VERTLINE, u"rline", TG::RBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TRDLINE, MS_DVERTLINE, u"rdline", TG::RBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none", TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u"", TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_PrefixPostfix_SmXMLOperatorContext_Impl(sal_Unicode cChar)
{
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TLRLINE, MS_VERTLINE, u"lrline", TG::LBrace | TG::RBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TLRDLINE, MS_DVERTLINE, u"lrdline", TG::LBrace | TG::RBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle", TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace", TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil", TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor", TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket", TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"[", TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"(", TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle", TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace", TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil", TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor", TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket", TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]", TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")", TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none", TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u"", TG::NONE, SAL_MAX_UINT16);
    }
}

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableParse[]
    = { { "aliceblue", "aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { "antiquewhite", "antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { "aqua", "aqua", TMATHMLCOL, COL_SM_AQUA },
        { "aquamarine", "aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { "azure", "azure", THTMLCOL, COL_SM_AZURE },
        { "beige", "beige", THTMLCOL, COL_SM_BEIGE },
        { "bisque", "bisque", THTMLCOL, COL_SM_BISQUE },
        { "black", "black", TMATHMLCOL, COL_SM_BLACK },
        { "blanchedalmond", "blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { "blue", "blue", TMATHMLCOL, COL_SM_BLUE },
        { "blueviolet", "blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { "brown", "brown", THTMLCOL, COL_SM_BROWN },
        { "burlywood", "burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { "cadetblue", "cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { "chartreuse", "chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { "chocolate", "chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { "coral", "coral", THTMLCOL, COL_SM_CORAL },
        { "cornflowerblue", "cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { "cornsilk", "cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { "crimson", "crimson", THTMLCOL, COL_SM_CRIMSON },
        { "cyan", "cyan", TMATHMLCOL, COL_SM_CYAN },
        { "darkblue", "darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { "darkcyan", "darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { "darkgoldenrod", "darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { "darkgray", "darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { "darkgreen", "darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { "darkgrey", "darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { "darkkhaki", "darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { "darkmagenta", "darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { "darkolivegreen", "darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { "darkorange", "darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { "darkorchid", "darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { "darkred", "darkred", THTMLCOL, COL_SM_DARKRED },
        { "darksalmon", "darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { "darkseagreen", "darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { "darkslateblue", "darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { "darkslategray", "darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { "darkslategrey", "darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { "darkturquoise", "darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { "darkviolet", "darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { "debian", "", TICONICCOL, COL_SM_DEBIAN_MAGENTA },
        { "deeppink", "deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { "deepskyblue", "deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { "dimgray", "dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { "dimgrey", "dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { "dodgerblue", "dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { "dvip", "dvip", TDVIPSNAMESCOL, COL_SM_BLACK },
        { "firebrick", "firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { "floralwhite", "floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { "forestgreen", "forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { "fuchsia", "fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { "gainsboro", "gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { "ghostwhite", "ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { "gold", "gold", THTMLCOL, COL_SM_GOLD },
        { "goldenrod", "goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { "gray", "gray", TMATHMLCOL, COL_SM_GRAY },
        { "green", "green", TMATHMLCOL, COL_SM_GREEN },
        { "greenyellow", "greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { "grey", "grey", THTMLCOL, COL_SM_GREY },
        { "hex", "hex", THEX, COL_SM_BLACK },
        { "honeydew", "honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { "hotpink", "hotpink", THTMLCOL, COL_SM_HOTPINK },
        { "indianred", "indianred", THTMLCOL, COL_SM_INDIANRED },
        { "indigo", "indigo", THTMLCOL, COL_SM_INDIGO },
        { "ivory", "ivory", THTMLCOL, COL_SM_IVORY },
        { "khaki", "khaki", THTMLCOL, COL_SM_KHAKI },
        { "lavender", "lavender", THTMLCOL, COL_SM_LAVENDER },
        { "lavenderblush", "lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { "lawngreen", "lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { "lemonchiffon", "lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { "lightblue", "lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { "lightcoral", "lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { "lightcyan", "lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { "lightgoldenrodyellow", "lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { "lightgray", "lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { "lightgreen", "lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { "lightgrey", "lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { "lightpink", "lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { "lightsalmon", "lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { "lightseagreen", "lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { "lightskyblue", "lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { "lightslategray", "lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { "lightslategrey", "lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { "lightsteelblue", "lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { "lightyellow", "lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { "lime", "lime", TMATHMLCOL, COL_SM_LIME },
        { "limegreen", "limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { "linen", "linen", THTMLCOL, COL_SM_LINEN },
        { "lo", "", TICONICCOL, COL_SM_LO_GREEN },
        { "magenta", "magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { "maroon", "maroon", TMATHMLCOL, COL_SM_MAROON },
        { "mediumaquamarine", "mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { "mediumblue", "mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { "mediumorchid", "mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { "mediumpurple", "mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { "mediumseagreen", "mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { "mediumslateblue", "mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { "mediumspringgreen", "mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { "mediumturquoise", "mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { "mediumvioletred", "mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { "midnightblue", "midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { "mintcream", "mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { "mistyrose", "mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { "moccasin", "moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { "navajowhite", "navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { "navy", "navy", TMATHMLCOL, COL_SM_NAVY },
        { "oldlace", "oldlace", THTMLCOL, COL_SM_OLDLACE },
        { "olive", "olive", TMATHMLCOL, COL_SM_OLIVE },
        { "olivedrab", "olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { "orange", "orange", THTMLCOL, COL_SM_ORANGE },
        { "orangered", "orangered", THTMLCOL, COL_SM_ORANGERED },
        { "orchid", "orchid", THTMLCOL, COL_SM_ORCHID },
        { "palegoldenrod", "palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { "palegreen", "palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { "paleturquoise", "paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { "palevioletred", "palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { "papayawhip", "papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { "peachpuff", "peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { "peru", "peru", THTMLCOL, COL_SM_PERU },
        { "pink", "pink", THTMLCOL, COL_SM_PINK },
        { "plum", "plum", THTMLCOL, COL_SM_PLUM },
        { "powderblue", "powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { "purple", "purple", TMATHMLCOL, COL_SM_PURPLE },
        { "rebeccapurple", "rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { "red", "red", TMATHMLCOL, COL_SM_RED },
        { "rgb", "rgb", TRGB, COL_AUTO },
        { "rgba", "rgba", TRGBA, COL_AUTO },
        { "rosybrown", "rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { "royalblue", "royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { "saddlebrown", "saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { "salmon", "salmon", THTMLCOL, COL_SM_SALMON },
        { "sandybrown", "sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { "seagreen", "seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { "seashell", "seashell", THTMLCOL, COL_SM_SEASHELL },
        { "sienna", "sienna", THTMLCOL, COL_SM_SIENNA },
        { "silver", "silver", TMATHMLCOL, COL_SM_SILVER },
        { "skyblue", "skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { "slateblue", "slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { "slategray", "slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { "slategrey", "slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { "snow", "snow", THTMLCOL, COL_SM_SNOW },
        { "springgreen", "springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { "steelblue", "steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { "tan", "tan", THTMLCOL, COL_SM_TAN },
        { "teal", "teal", TMATHMLCOL, COL_SM_TEAL },
        { "thistle", "thistle", THTMLCOL, COL_SM_THISTLE },
        { "tomato", "tomato", THTMLCOL, COL_SM_TOMATO },
        { "turquoise", "turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { "ubuntu", "", TICONICCOL, COL_SM_UBUNTU_ORANGE },
        { "violet", "COL_SM_VIOLET", THTMLCOL, COL_SM_VIOLET },
        { "wheat", "wheat", THTMLCOL, COL_SM_WHEAT },
        { "white", "white", TMATHMLCOL, COL_SM_WHITE },
        { "whitesmoke", "whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { "yellow", "yellow", TMATHMLCOL, COL_SM_YELLOW },
        { "yellowgreen", "yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableHTML[]
    = { { "aliceblue", "aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { "antiquewhite", "antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { "aqua", "aqua", TMATHMLCOL, COL_SM_AQUA },
        { "aquamarine", "aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { "azure", "azure", THTMLCOL, COL_SM_AZURE },
        { "beige", "beige", THTMLCOL, COL_SM_BEIGE },
        { "bisque", "bisque", THTMLCOL, COL_SM_BISQUE },
        { "black", "black", TMATHMLCOL, COL_SM_BLACK },
        { "blanchedalmond", "blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { "blue", "blue", TMATHMLCOL, COL_SM_BLUE },
        { "blueviolet", "blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { "brown", "brown", THTMLCOL, COL_SM_BROWN },
        { "burlywood", "burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { "cadetblue", "cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { "chartreuse", "chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { "chocolate", "chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { "coral", "coral", THTMLCOL, COL_SM_CORAL },
        { "cornflowerblue", "cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { "cornsilk", "cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { "crimson", "crimson", THTMLCOL, COL_SM_CRIMSON },
        { "cyan", "cyan", TMATHMLCOL, COL_SM_CYAN },
        { "darkblue", "darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { "darkcyan", "darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { "darkgoldenrod", "darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { "darkgray", "darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { "darkgreen", "darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { "darkgrey", "darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { "darkkhaki", "darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { "darkmagenta", "darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { "darkolivegreen", "darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { "darkorange", "darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { "darkorchid", "darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { "darkred", "darkred", THTMLCOL, COL_SM_DARKRED },
        { "darksalmon", "darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { "darkseagreen", "darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { "darkslateblue", "darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { "darkslategray", "darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { "darkslategrey", "darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { "darkturquoise", "darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { "darkviolet", "darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { "deeppink", "deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { "deepskyblue", "deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { "dimgray", "dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { "dimgrey", "dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { "dodgerblue", "dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { "firebrick", "firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { "floralwhite", "floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { "forestgreen", "forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { "fuchsia", "fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { "gainsboro", "gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { "ghostwhite", "ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { "gold", "gold", THTMLCOL, COL_SM_GOLD },
        { "goldenrod", "goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { "gray", "gray", TMATHMLCOL, COL_SM_GRAY },
        { "green", "green", TMATHMLCOL, COL_SM_GREEN },
        { "greenyellow", "greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { "grey", "grey", THTMLCOL, COL_SM_GREY },
        { "honeydew", "honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { "hotpink", "hotpink", THTMLCOL, COL_SM_HOTPINK },
        { "indianred", "indianred", THTMLCOL, COL_SM_INDIANRED },
        { "indigo", "indigo", THTMLCOL, COL_SM_INDIGO },
        { "ivory", "ivory", THTMLCOL, COL_SM_IVORY },
        { "khaki", "khaki", THTMLCOL, COL_SM_KHAKI },
        { "lavender", "lavender", THTMLCOL, COL_SM_LAVENDER },
        { "lavenderblush", "lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { "lawngreen", "lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { "lemonchiffon", "lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { "lightblue", "lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { "lightcoral", "lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { "lightcyan", "lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { "lightgoldenrodyellow", "lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { "lightgray", "lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { "lightgreen", "lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { "lightgrey", "lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { "lightpink", "lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { "lightsalmon", "lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { "lightseagreen", "lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { "lightskyblue", "lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { "lightslategray", "lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { "lightslategrey", "lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { "lightsteelblue", "lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { "lightyellow", "lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { "lime", "lime", TMATHMLCOL, COL_SM_LIME },
        { "limegreen", "limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { "linen", "linen", THTMLCOL, COL_SM_LINEN },
        { "magenta", "magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { "maroon", "maroon", TMATHMLCOL, COL_SM_MAROON },
        { "mediumaquamarine", "mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { "mediumblue", "mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { "mediumorchid", "mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { "mediumpurple", "mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { "mediumseagreen", "mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { "mediumslateblue", "mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { "mediumspringgreen", "mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { "mediumturquoise", "mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { "mediumvioletred", "mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { "midnightblue", "midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { "mintcream", "mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { "mistyrose", "mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { "moccasin", "moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { "navajowhite", "navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { "navy", "navy", TMATHMLCOL, COL_SM_NAVY },
        { "oldlace", "oldlace", THTMLCOL, COL_SM_OLDLACE },
        { "olive", "olive", TMATHMLCOL, COL_SM_OLIVE },
        { "olivedrab", "olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { "orange", "orange", THTMLCOL, COL_SM_ORANGE },
        { "orangered", "orangered", THTMLCOL, COL_SM_ORANGERED },
        { "orchid", "orchid", THTMLCOL, COL_SM_ORCHID },
        { "palegoldenrod", "palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { "palegreen", "palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { "paleturquoise", "paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { "palevioletred", "palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { "papayawhip", "papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { "peachpuff", "peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { "peru", "peru", THTMLCOL, COL_SM_PERU },
        { "pink", "pink", THTMLCOL, COL_SM_PINK },
        { "plum", "plum", THTMLCOL, COL_SM_PLUM },
        { "powderblue", "powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { "purple", "purple", TMATHMLCOL, COL_SM_PURPLE },
        { "rebeccapurple", "rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { "red", "red", TMATHMLCOL, COL_SM_RED },
        { "rosybrown", "rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { "royalblue", "royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { "saddlebrown", "saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { "salmon", "salmon", THTMLCOL, COL_SM_SALMON },
        { "sandybrown", "sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { "seagreen", "seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { "seashell", "seashell", THTMLCOL, COL_SM_SEASHELL },
        { "sienna", "sienna", THTMLCOL, COL_SM_SIENNA },
        { "silver", "silver", TMATHMLCOL, COL_SM_SILVER },
        { "skyblue", "skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { "slateblue", "slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { "slategray", "slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { "slategrey", "slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { "snow", "snow", THTMLCOL, COL_SM_SNOW },
        { "springgreen", "springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { "steelblue", "steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { "tan", "tan", THTMLCOL, COL_SM_TAN },
        { "teal", "teal", TMATHMLCOL, COL_SM_TEAL },
        { "thistle", "thistle", THTMLCOL, COL_SM_THISTLE },
        { "tomato", "tomato", THTMLCOL, COL_SM_TOMATO },
        { "turquoise", "turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { "violet", "COL_SM_VIOLET", THTMLCOL, COL_SM_VIOLET },
        { "wheat", "wheat", THTMLCOL, COL_SM_WHEAT },
        { "white", "white", TMATHMLCOL, COL_SM_WHITE },
        { "whitesmoke", "whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { "yellow", "yellow", TMATHMLCOL, COL_SM_YELLOW },
        { "yellowgreen", "yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableDVIPSNAMES[]
    = { { "apricot", "Apricot", TDVIPSNAMESCOL, COL_SM_DIV_APRICOT },
        { "aquamarine", "Aquamarine", TDVIPSNAMESCOL, COL_SM_DIV_AQUAMARINE },
        { "bittersweet", "Bittersweet", TDVIPSNAMESCOL, COL_SM_DIV_BITTERSWEET },
        { "black", "Black", TDVIPSNAMESCOL, COL_SM_BLACK },
        { "blue", "Blue", TDVIPSNAMESCOL, COL_SM_BLACK } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableMATHML[]
    = { { "aqua", "aqua", TMATHMLCOL, COL_SM_AQUA },
        { "black", "black", TMATHMLCOL, COL_SM_BLACK },
        { "blue", "blue", TMATHMLCOL, COL_SM_BLUE },
        { "fuchsia", "fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { "gray", "gray", TMATHMLCOL, COL_SM_GRAY },
        { "green", "green", TMATHMLCOL, COL_SM_GREEN },
        { "lime", "lime", TMATHMLCOL, COL_SM_LIME },
        { "maroon", "maroon", TMATHMLCOL, COL_SM_MAROON },
        { "navy", "navy", TMATHMLCOL, COL_SM_NAVY },
        { "olive", "olive", TMATHMLCOL, COL_SM_OLIVE },
        { "purple", "purple", TMATHMLCOL, COL_SM_PURPLE },
        { "red", "red", TMATHMLCOL, COL_SM_RED },
        { "silver", "silver", TMATHMLCOL, COL_SM_SILVER },
        { "teal", "teal", TMATHMLCOL, COL_SM_TEAL },
        { "white", "white", TMATHMLCOL, COL_SM_WHITE },
        { "yellow", "yellow", TMATHMLCOL, COL_SM_YELLOW } };

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_Parser(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    for (auto i = std::begin(aColorTokenTableDVIPSNAMES); i < std::end(aColorTokenTableDVIPSNAMES);
         ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_HTML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_MATHML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableMATHML); i < std::end(aColorTokenTableMATHML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_Color_DVIPSNAMES(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableDVIPSNAMES); i < std::end(aColorTokenTableDVIPSNAMES);
         ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_Parser(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry("", "", TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_HTML(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if (colorname[0] == '#')
    {
        Color col = Color::STRtoRGB(colorname);
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGB, col));
    }
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry("", "", TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_DVIPSNAMES(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableDVIPSNAMES); i < std::end(aColorTokenTableDVIPSNAMES);
         ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry("", "", TERROR, COL_SM_BLACK));
}

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
            return SmToken(TCOPROD, MS_COPROD, u"coprod", TG::Largeop, 5);
        case MS_IIINT:
            return SmToken(TIIINT, MS_IIINT, u"iiint", TG::Largeop, 5);
        case MS_IINT:
            return SmToken(TIINT, MS_IINT, u"iint", TG::Largeop, 5);
        case MS_INT:
            if (bIsStretchy)
                return SmToken(TINTD, MS_INT, u"intd", TG::Largeop, 5);
            else
                return SmToken(TINT, MS_INT, u"int", TG::Largeop, 5);
        case MS_LINT:
            return SmToken(TLINT, MS_LINT, u"lint", TG::Largeop, 5);
        case MS_LLINT:
            return SmToken(TLLINT, MS_LLINT, u"llint", TG::Largeop, 5);
        case MS_LLLINT:
            return SmToken(TLLLINT, MS_LLLINT, u"lllint", TG::Largeop, 5);
        case MS_PROD:
            return SmToken(TPROD, MS_PROD, u"prod", TG::Largeop, 5);
        case MS_SUM:
            return SmToken(TSUM, MS_SUM, u"sum", TG::Largeop, 5);
        case MS_FACT:
            return SmToken(TFACT, MS_FACT, u"!", TG::UnMo, 5);
        case MS_NEG:
            return SmToken(TNEG, MS_NEG, u"neg", TG::UnMo, 5);
        case MS_OMINUS:
            return SmToken(TOMINUS, MS_OMINUS, u"ominus", TG::Sum, 0);
        case MS_OPLUS:
            return SmToken(TOPLUS, MS_OPLUS, u"oplus", TG::Sum, 0);
        case MS_UNION:
            return SmToken(TUNION, MS_UNION, u"union", TG::Sum, 0);
        case MS_OR:
            return SmToken(TOR, MS_OR, u"|", TG::Sum, 5);
        case MS_PLUSMINUS:
            return SmToken(TPLUSMINUS, MS_PLUSMINUS, u"+-", TG::Sum | TG::UnMo, 5);
        case MS_MINUSPLUS:
            return SmToken(TMINUSPLUS, MS_MINUSPLUS, u"-+", TG::Sum | TG::UnMo, 5);
        case 0xe083:
        case MS_PLUS:
            return SmToken(TPLUS, MS_PLUS, u"+", TG::Sum | TG::UnMo, 5);
        case MS_MINUS:
            return SmToken(TMINUS, MS_MINUS, u"-", TG::Sum | TG::UnMo, 5);
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
    = { { u"aliceblue", u"aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { u"antiquewhite", u"antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { u"aqua", u"aqua", TMATHMLCOL, COL_SM_AQUA },
        { u"aquamarine", u"aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { u"azure", u"azure", THTMLCOL, COL_SM_AZURE },
        { u"beige", u"beige", THTMLCOL, COL_SM_BEIGE },
        { u"bisque", u"bisque", THTMLCOL, COL_SM_BISQUE },
        { u"black", u"black", TMATHMLCOL, COL_SM_BLACK },
        { u"blanchedalmond", u"blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { u"blue", u"blue", TMATHMLCOL, COL_SM_BLUE },
        { u"blueviolet", u"blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { u"brown", u"brown", THTMLCOL, COL_SM_BROWN },
        { u"burlywood", u"burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { u"cadetblue", u"cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { u"chartreuse", u"chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { u"chocolate", u"chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { u"coral", u"coral", THTMLCOL, COL_SM_CORAL },
        { u"cornflowerblue", u"cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { u"cornsilk", u"cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { u"crimson", u"crimson", THTMLCOL, COL_SM_CRIMSON },
        { u"cyan", u"cyan", TMATHMLCOL, COL_SM_CYAN },
        { u"darkblue", u"darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { u"darkcyan", u"darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { u"darkgoldenrod", u"darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { u"darkgray", u"darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { u"darkgreen", u"darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { u"darkgrey", u"darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { u"darkkhaki", u"darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { u"darkmagenta", u"darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { u"darkolivegreen", u"darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { u"darkorange", u"darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { u"darkorchid", u"darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { u"darkred", u"darkred", THTMLCOL, COL_SM_DARKRED },
        { u"darksalmon", u"darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { u"darkseagreen", u"darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { u"darkslateblue", u"darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { u"darkslategray", u"darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { u"darkslategrey", u"darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { u"darkturquoise", u"darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { u"darkviolet", u"darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { u"debian", u"", TICONICCOL, COL_SM_DEBIAN_MAGENTA },
        { u"deeppink", u"deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { u"deepskyblue", u"deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { u"dimgray", u"dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { u"dimgrey", u"dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { u"dodgerblue", u"dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { u"dvip", u"dvip", TDVIPSNAMESCOL, COL_SM_BLACK },
        { u"firebrick", u"firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { u"floralwhite", u"floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { u"forestgreen", u"forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { u"fuchsia", u"fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gainsboro", u"gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { u"ghostwhite", u"ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { u"gold", u"gold", THTMLCOL, COL_SM_GOLD },
        { u"goldenrod", u"goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { u"gray", u"gray", TMATHMLCOL, COL_SM_GRAY },
        { u"green", u"green", TMATHMLCOL, COL_SM_GREEN },
        { u"greenyellow", u"greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { u"grey", u"grey", THTMLCOL, COL_SM_GREY },
        { u"hex", u"hex", THEX, COL_SM_BLACK },
        { u"honeydew", u"honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { u"hotpink", u"hotpink", THTMLCOL, COL_SM_HOTPINK },
        { u"indianred", u"indianred", THTMLCOL, COL_SM_INDIANRED },
        { u"indigo", u"indigo", THTMLCOL, COL_SM_INDIGO },
        { u"ivory", u"ivory", THTMLCOL, COL_SM_IVORY },
        { u"khaki", u"khaki", THTMLCOL, COL_SM_KHAKI },
        { u"lavender", u"lavender", THTMLCOL, COL_SM_LAVENDER },
        { u"lavenderblush", u"lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { u"lawngreen", u"lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { u"lemonchiffon", u"lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { u"lightblue", u"lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { u"lightcoral", u"lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { u"lightcyan", u"lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { u"lightgoldenrodyellow", u"lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { u"lightgray", u"lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { u"lightgreen", u"lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { u"lightgrey", u"lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { u"lightpink", u"lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { u"lightsalmon", u"lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { u"lightseagreen", u"lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { u"lightskyblue", u"lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { u"lightslategray", u"lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { u"lightslategrey", u"lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { u"lightsteelblue", u"lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { u"lightyellow", u"lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { u"lime", u"lime", TMATHMLCOL, COL_SM_LIME },
        { u"limegreen", u"limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { u"linen", u"linen", THTMLCOL, COL_SM_LINEN },
        { u"lo", u"", TICONICCOL, COL_SM_LO_GREEN },
        { u"magenta", u"magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { u"maroon", u"maroon", TMATHMLCOL, COL_SM_MAROON },
        { u"mediumaquamarine", u"mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { u"mediumblue", u"mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { u"mediumorchid", u"mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { u"mediumpurple", u"mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { u"mediumseagreen", u"mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { u"mediumslateblue", u"mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { u"mediumspringgreen", u"mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { u"mediumturquoise", u"mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { u"mediumvioletred", u"mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { u"midnightblue", u"midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { u"mintcream", u"mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { u"mistyrose", u"mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { u"moccasin", u"moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { u"navajowhite", u"navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { u"navy", u"navy", TMATHMLCOL, COL_SM_NAVY },
        { u"oldlace", u"oldlace", THTMLCOL, COL_SM_OLDLACE },
        { u"olive", u"olive", TMATHMLCOL, COL_SM_OLIVE },
        { u"olivedrab", u"olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { u"orange", u"orange", THTMLCOL, COL_SM_ORANGE },
        { u"orangered", u"orangered", THTMLCOL, COL_SM_ORANGERED },
        { u"orchid", u"orchid", THTMLCOL, COL_SM_ORCHID },
        { u"palegoldenrod", u"palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { u"palegreen", u"palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { u"paleturquoise", u"paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { u"palevioletred", u"palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { u"papayawhip", u"papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { u"peachpuff", u"peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { u"peru", u"peru", THTMLCOL, COL_SM_PERU },
        { u"pink", u"pink", THTMLCOL, COL_SM_PINK },
        { u"plum", u"plum", THTMLCOL, COL_SM_PLUM },
        { u"powderblue", u"powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { u"purple", u"purple", TMATHMLCOL, COL_SM_PURPLE },
        { u"rebeccapurple", u"rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { u"red", u"red", TMATHMLCOL, COL_SM_RED },
        { u"rgb", u"rgb", TRGB, COL_AUTO },
        { u"rgba", u"rgba", TRGBA, COL_AUTO },
        { u"rosybrown", u"rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { u"royalblue", u"royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { u"saddlebrown", u"saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { u"salmon", u"salmon", THTMLCOL, COL_SM_SALMON },
        { u"sandybrown", u"sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { u"seagreen", u"seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { u"seashell", u"seashell", THTMLCOL, COL_SM_SEASHELL },
        { u"sienna", u"sienna", THTMLCOL, COL_SM_SIENNA },
        { u"silver", u"silver", TMATHMLCOL, COL_SM_SILVER },
        { u"skyblue", u"skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { u"slateblue", u"slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { u"slategray", u"slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { u"slategrey", u"slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { u"snow", u"snow", THTMLCOL, COL_SM_SNOW },
        { u"springgreen", u"springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { u"steelblue", u"steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { u"tan", u"tan", THTMLCOL, COL_SM_TAN },
        { u"teal", u"teal", TMATHMLCOL, COL_SM_TEAL },
        { u"thistle", u"thistle", THTMLCOL, COL_SM_THISTLE },
        { u"tomato", u"tomato", THTMLCOL, COL_SM_TOMATO },
        { u"turquoise", u"turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { u"ubuntu", u"", TICONICCOL, COL_SM_UBUNTU_ORANGE },
        { u"violet", u"COL_SM_VIOLET", THTMLCOL, COL_SM_VIOLET },
        { u"wheat", u"wheat", THTMLCOL, COL_SM_WHEAT },
        { u"white", u"white", TMATHMLCOL, COL_SM_WHITE },
        { u"whitesmoke", u"whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { u"yellow", u"yellow", TMATHMLCOL, COL_SM_YELLOW },
        { u"yellowgreen", u"yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableHTML[]
    = { { u"aliceblue", u"aliceblue", THTMLCOL, COL_SM_ALICEBLUE },
        { u"antiquewhite", u"antiquewhite", THTMLCOL, COL_SM_ANTIQUEWHITE },
        { u"aqua", u"aqua", TMATHMLCOL, COL_SM_AQUA },
        { u"aquamarine", u"aquamarine", THTMLCOL, COL_SM_AQUAMARINE },
        { u"azure", u"azure", THTMLCOL, COL_SM_AZURE },
        { u"beige", u"beige", THTMLCOL, COL_SM_BEIGE },
        { u"bisque", u"bisque", THTMLCOL, COL_SM_BISQUE },
        { u"black", u"black", TMATHMLCOL, COL_SM_BLACK },
        { u"blanchedalmond", u"blanchedalmond", THTMLCOL, COL_SM_BLANCHEDALMOND },
        { u"blue", u"blue", TMATHMLCOL, COL_SM_BLUE },
        { u"blueviolet", u"blueviolet", THTMLCOL, COL_SM_BLUEVIOLET },
        { u"brown", u"brown", THTMLCOL, COL_SM_BROWN },
        { u"burlywood", u"burlywood", THTMLCOL, COL_SM_BURLYWOOD },
        { u"cadetblue", u"cadetblue", THTMLCOL, COL_SM_CADETBLUE },
        { u"chartreuse", u"chartreuse", THTMLCOL, COL_SM_CHARTREUSE },
        { u"chocolate", u"chocolate", THTMLCOL, COL_SM_CHOCOLATE },
        { u"coral", u"coral", THTMLCOL, COL_SM_CORAL },
        { u"cornflowerblue", u"cornflowerblue", THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { u"cornsilk", u"cornsilk", THTMLCOL, COL_SM_CORNSILK },
        { u"crimson", u"crimson", THTMLCOL, COL_SM_CRIMSON },
        { u"cyan", u"cyan", TMATHMLCOL, COL_SM_CYAN },
        { u"darkblue", u"darkblue", THTMLCOL, COL_SM_DARKBLUE },
        { u"darkcyan", u"darkcyan", THTMLCOL, COL_SM_DARKCYAN },
        { u"darkgoldenrod", u"darkgoldenrod", THTMLCOL, COL_SM_DARKGOLDENROD },
        { u"darkgray", u"darkgray", THTMLCOL, COL_SM_DARKGRAY },
        { u"darkgreen", u"darkgreen", THTMLCOL, COL_SM_DARKGREEN },
        { u"darkgrey", u"darkgrey", THTMLCOL, COL_SM_DARKGREY },
        { u"darkkhaki", u"darkkhaki", THTMLCOL, COL_SM_DARKKHAKI },
        { u"darkmagenta", u"darkmagenta", THTMLCOL, COL_SM_DARKMAGENTA },
        { u"darkolivegreen", u"darkolivegreen", THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { u"darkorange", u"darkorange", THTMLCOL, COL_SM_DARKORANGE },
        { u"darkorchid", u"darkorchid", THTMLCOL, COL_SM_DARKORCHID },
        { u"darkred", u"darkred", THTMLCOL, COL_SM_DARKRED },
        { u"darksalmon", u"darksalmon", THTMLCOL, COL_SM_DARKSALMON },
        { u"darkseagreen", u"darkseagreen", THTMLCOL, COL_SM_DARKSEAGREEN },
        { u"darkslateblue", u"darkslateblue", THTMLCOL, COL_SM_DARKSLATEBLUE },
        { u"darkslategray", u"darkslategray", THTMLCOL, COL_SM_DARKSLATEGRAY },
        { u"darkslategrey", u"darkslategrey", THTMLCOL, COL_SM_DARKSLATEGREY },
        { u"darkturquoise", u"darkturquoise", THTMLCOL, COL_SM_DARKTURQUOISE },
        { u"darkviolet", u"darkviolet", THTMLCOL, COL_SM_DARKVIOLET },
        { u"deeppink", u"deeppink", THTMLCOL, COL_SM_DEEPPINK },
        { u"deepskyblue", u"deepskyblue", THTMLCOL, COL_SM_DEEPSKYBLUE },
        { u"dimgray", u"dimgray", THTMLCOL, COL_SM_DIMGRAY },
        { u"dimgrey", u"dimgrey", THTMLCOL, COL_SM_DIMGREY },
        { u"dodgerblue", u"dodgerblue", THTMLCOL, COL_SM_DODGERBLUE },
        { u"firebrick", u"firebrick", THTMLCOL, COL_SM_FIREBRICK },
        { u"floralwhite", u"floralwhite", THTMLCOL, COL_SM_FLORALWHITE },
        { u"forestgreen", u"forestgreen", THTMLCOL, COL_SM_FORESTGREEN },
        { u"fuchsia", u"fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gainsboro", u"gainsboro", THTMLCOL, COL_SM_GAINSBORO },
        { u"ghostwhite", u"ghostwhite", THTMLCOL, COL_SM_GHOSTWHITE },
        { u"gold", u"gold", THTMLCOL, COL_SM_GOLD },
        { u"goldenrod", u"goldenrod", THTMLCOL, COL_SM_GOLDENROD },
        { u"gray", u"gray", TMATHMLCOL, COL_SM_GRAY },
        { u"green", u"green", TMATHMLCOL, COL_SM_GREEN },
        { u"greenyellow", u"greenyellow", THTMLCOL, COL_SM_GREENYELLOW },
        { u"grey", u"grey", THTMLCOL, COL_SM_GREY },
        { u"honeydew", u"honeydew", THTMLCOL, COL_SM_HONEYDEW },
        { u"hotpink", u"hotpink", THTMLCOL, COL_SM_HOTPINK },
        { u"indianred", u"indianred", THTMLCOL, COL_SM_INDIANRED },
        { u"indigo", u"indigo", THTMLCOL, COL_SM_INDIGO },
        { u"ivory", u"ivory", THTMLCOL, COL_SM_IVORY },
        { u"khaki", u"khaki", THTMLCOL, COL_SM_KHAKI },
        { u"lavender", u"lavender", THTMLCOL, COL_SM_LAVENDER },
        { u"lavenderblush", u"lavenderblush", THTMLCOL, COL_SM_LAVENDERBLUSH },
        { u"lawngreen", u"lawngreen", THTMLCOL, COL_SM_LAWNGREEN },
        { u"lemonchiffon", u"lemonchiffon", THTMLCOL, COL_SM_LEMONCHIFFON },
        { u"lightblue", u"lightblue", THTMLCOL, COL_SM_LIGHTBLUE },
        { u"lightcoral", u"lightcoral", THTMLCOL, COL_SM_LIGHTCORAL },
        { u"lightcyan", u"lightcyan", THTMLCOL, COL_SM_LIGHTCYAN },
        { u"lightgoldenrodyellow", u"lightgoldenrodyellow", THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { u"lightgray", u"lightgray", THTMLCOL, COL_SM_LIGHTGRAY },
        { u"lightgreen", u"lightgreen", THTMLCOL, COL_SM_LIGHTGREEN },
        { u"lightgrey", u"lightgrey", THTMLCOL, COL_SM_LIGHTGREY },
        { u"lightpink", u"lightpink", THTMLCOL, COL_SM_LIGHTPINK },
        { u"lightsalmon", u"lightsalmon", THTMLCOL, COL_SM_LIGHTSALMON },
        { u"lightseagreen", u"lightseagreen", THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { u"lightskyblue", u"lightskyblue", THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { u"lightslategray", u"lightslategray", THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { u"lightslategrey", u"lightslategrey", THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { u"lightsteelblue", u"lightsteelblue", THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { u"lightyellow", u"lightyellow", THTMLCOL, COL_SM_LIGHTYELLOW },
        { u"lime", u"lime", TMATHMLCOL, COL_SM_LIME },
        { u"limegreen", u"limegreen", THTMLCOL, COL_SM_LIMEGREEN },
        { u"linen", u"linen", THTMLCOL, COL_SM_LINEN },
        { u"magenta", u"magenta", TMATHMLCOL, COL_SM_MAGENTA },
        { u"maroon", u"maroon", TMATHMLCOL, COL_SM_MAROON },
        { u"mediumaquamarine", u"mediumaquamarine", THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { u"mediumblue", u"mediumblue", THTMLCOL, COL_SM_MEDIUMBLUE },
        { u"mediumorchid", u"mediumorchid", THTMLCOL, COL_SM_MEDIUMORCHID },
        { u"mediumpurple", u"mediumpurple", THTMLCOL, COL_SM_MEDIUMPURPLE },
        { u"mediumseagreen", u"mediumseagreen", THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { u"mediumslateblue", u"mediumslateblue", THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { u"mediumspringgreen", u"mediumspringgreen", THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { u"mediumturquoise", u"mediumturquoise", THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { u"mediumvioletred", u"mediumvioletred", THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { u"midnightblue", u"midnightblue", THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { u"mintcream", u"mintcream", THTMLCOL, COL_SM_MINTCREAM },
        { u"mistyrose", u"mistyrose", THTMLCOL, COL_SM_MISTYROSE },
        { u"moccasin", u"moccasin", THTMLCOL, COL_SM_MOCCASIN },
        { u"navajowhite", u"navajowhite", THTMLCOL, COL_SM_NAVAJOWHITE },
        { u"navy", u"navy", TMATHMLCOL, COL_SM_NAVY },
        { u"oldlace", u"oldlace", THTMLCOL, COL_SM_OLDLACE },
        { u"olive", u"olive", TMATHMLCOL, COL_SM_OLIVE },
        { u"olivedrab", u"olivedrab", THTMLCOL, COL_SM_OLIVEDRAB },
        { u"orange", u"orange", THTMLCOL, COL_SM_ORANGE },
        { u"orangered", u"orangered", THTMLCOL, COL_SM_ORANGERED },
        { u"orchid", u"orchid", THTMLCOL, COL_SM_ORCHID },
        { u"palegoldenrod", u"palegoldenrod", THTMLCOL, COL_SM_PALEGOLDENROD },
        { u"palegreen", u"palegreen", THTMLCOL, COL_SM_PALEGREEN },
        { u"paleturquoise", u"paleturquoise", THTMLCOL, COL_SM_PALETURQUOISE },
        { u"palevioletred", u"palevioletred", THTMLCOL, COL_SM_PALEVIOLETRED },
        { u"papayawhip", u"papayawhip", THTMLCOL, COL_SM_PAPAYAWHIP },
        { u"peachpuff", u"peachpuff", THTMLCOL, COL_SM_PEACHPUFF },
        { u"peru", u"peru", THTMLCOL, COL_SM_PERU },
        { u"pink", u"pink", THTMLCOL, COL_SM_PINK },
        { u"plum", u"plum", THTMLCOL, COL_SM_PLUM },
        { u"powderblue", u"powderblue", THTMLCOL, COL_SM_POWDERBLUE },
        { u"purple", u"purple", TMATHMLCOL, COL_SM_PURPLE },
        { u"rebeccapurple", u"rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { u"red", u"red", TMATHMLCOL, COL_SM_RED },
        { u"rosybrown", u"rosybrown", THTMLCOL, COL_SM_ROSYBROWN },
        { u"royalblue", u"royalblue", THTMLCOL, COL_SM_ROYALBLUE },
        { u"saddlebrown", u"saddlebrown", THTMLCOL, COL_SM_SADDLEBROWN },
        { u"salmon", u"salmon", THTMLCOL, COL_SM_SALMON },
        { u"sandybrown", u"sandybrown", THTMLCOL, COL_SM_SANDYBROWN },
        { u"seagreen", u"seagreen", THTMLCOL, COL_SM_SEAGREEN },
        { u"seashell", u"seashell", THTMLCOL, COL_SM_SEASHELL },
        { u"sienna", u"sienna", THTMLCOL, COL_SM_SIENNA },
        { u"silver", u"silver", TMATHMLCOL, COL_SM_SILVER },
        { u"skyblue", u"skyblue", THTMLCOL, COL_SM_SKYBLUE },
        { u"slateblue", u"slateblue", THTMLCOL, COL_SM_SLATEBLUE },
        { u"slategray", u"slategray", THTMLCOL, COL_SM_SLATEGRAY },
        { u"slategrey", u"slategrey", THTMLCOL, COL_SM_SLATEGREY },
        { u"snow", u"snow", THTMLCOL, COL_SM_SNOW },
        { u"springgreen", u"springgreen", THTMLCOL, COL_SM_SPRINGGREEN },
        { u"steelblue", u"steelblue", THTMLCOL, COL_SM_STEELBLUE },
        { u"tan", u"tan", THTMLCOL, COL_SM_TAN },
        { u"teal", u"teal", TMATHMLCOL, COL_SM_TEAL },
        { u"thistle", u"thistle", THTMLCOL, COL_SM_THISTLE },
        { u"tomato", u"tomato", THTMLCOL, COL_SM_TOMATO },
        { u"turquoise", u"turquoise", THTMLCOL, COL_SM_TURQUOISE },
        { u"violet", u"COL_SM_VIOLET", THTMLCOL, COL_SM_VIOLET },
        { u"wheat", u"wheat", THTMLCOL, COL_SM_WHEAT },
        { u"white", u"white", TMATHMLCOL, COL_SM_WHITE },
        { u"whitesmoke", u"whitesmoke", THTMLCOL, COL_SM_WHITESMOKE },
        { u"yellow", u"yellow", TMATHMLCOL, COL_SM_YELLOW },
        { u"yellowgreen", u"yellowgreen", THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableDVIPSNAMES[]
    = { { u"apricot", u"Apricot", TDVIPSNAMESCOL, COL_SM_DIV_APRICOT },
        { u"aquamarine", u"Aquamarine", TDVIPSNAMESCOL, COL_SM_DIV_AQUAMARINE },
        { u"bittersweet", u"Bittersweet", TDVIPSNAMESCOL, COL_SM_DIV_BITTERSWEET },
        { u"black", u"Black", TDVIPSNAMESCOL, COL_SM_BLACK },
        { u"blue", u"Blue", TDVIPSNAMESCOL, COL_SM_BLACK } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableMATHML[]
    = { { u"aqua", u"aqua", TMATHMLCOL, COL_SM_AQUA },
        { u"black", u"black", TMATHMLCOL, COL_SM_BLACK },
        { u"blue", u"blue", TMATHMLCOL, COL_SM_BLUE },
        { u"fuchsia", u"fuchsia", TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gray", u"gray", TMATHMLCOL, COL_SM_GRAY },
        { u"green", u"green", TMATHMLCOL, COL_SM_GREEN },
        { u"lime", u"lime", TMATHMLCOL, COL_SM_LIME },
        { u"maroon", u"maroon", TMATHMLCOL, COL_SM_MAROON },
        { u"navy", u"navy", TMATHMLCOL, COL_SM_NAVY },
        { u"olive", u"olive", TMATHMLCOL, COL_SM_OLIVE },
        { u"purple", u"purple", TMATHMLCOL, COL_SM_PURPLE },
        { u"red", u"red", TMATHMLCOL, COL_SM_RED },
        { u"silver", u"silver", TMATHMLCOL, COL_SM_SILVER },
        { u"teal", u"teal", TMATHMLCOL, COL_SM_TEAL },
        { u"white", u"white", TMATHMLCOL, COL_SM_WHITE },
        { u"yellow", u"yellow", TMATHMLCOL, COL_SM_YELLOW } };

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
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_HTML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_MATHML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableMATHML); i < std::end(aColorTokenTableMATHML); ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_Color_DVIPSNAMES(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableDVIPSNAMES); i < std::end(aColorTokenTableDVIPSNAMES);
         ++i)
        if (i->equals(cColor))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if ((cColor & 0x00FFFFFF) == cColor)
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGB, cColor));
    else
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGBA, cColor));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_Parser(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry(TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_HTML(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if (colorname[0] == '#')
    {
        Color col = Color::STRtoRGB(colorname);
        return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(TRGB, col));
    }
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry(TERROR, COL_SM_BLACK));
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_DVIPSNAMES(const OUString& colorname)
{
    if (colorname.isEmpty())
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry(TERROR, COL_SM_BLACK));
    for (auto i = std::begin(aColorTokenTableDVIPSNAMES); i < std::end(aColorTokenTableDVIPSNAMES);
         ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return std::unique_ptr<SmColorTokenTableEntry>(
        new SmColorTokenTableEntry(TERROR, COL_SM_BLACK));
}

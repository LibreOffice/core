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
    = { { "apricot", "Apricot", TDVIPSNAMESCOL, COL_SM_DIV_APRICOT },
        { "aqua", "aqua", THTMLCOL, COL_SM_AQUA },
        { "aquamarine", "Aquamarine", TDVIPSNAMESCOL, COL_SM_DIV_AQUAMARINE },
        { "bittersweet", "Bittersweet", TDVIPSNAMESCOL, COL_SM_DIV_BITTERSWEET },
        { "black", "black", THTMLCOL, COL_SM_BLACK },
        { "blue", "blue", THTMLCOL, COL_SM_BLUE },
        { "cyan", "cyan", THTMLCOL, COL_SM_BLACK },
        { "debian", "", TICONICCOL, COL_SM_DEBIAN_MAGENTA },
        { "dblack", "Black", TDVIPSNAMESCOL, COL_SM_BLACK },
        { "dblue", "Blue", TDVIPSNAMESCOL, COL_SM_BLACK },
        { "fuchsia", "fuchsia", THTMLCOL, COL_SM_FUCHSIA },
        { "gray", "gray", THTMLCOL, COL_SM_GRAY },
        { "green", "green", THTMLCOL, COL_SM_GREEN },
        { "hex", "hex", THEX, COL_SM_BLACK },
        { "lime", "lime", THTMLCOL, COL_SM_LIME },
        { "lo", "", TICONICCOL, COL_SM_LO_GREEN },
        { "magenta", "magenta", THTMLCOL, COL_SM_FUCHSIA },
        { "maroon", "maroon", THTMLCOL, COL_SM_MAROON },
        { "navy", "navy", THTMLCOL, COL_SM_NAVY },
        { "olive", "olive", THTMLCOL, COL_SM_OLIVE },
        { "purple", "purple", THTMLCOL, COL_SM_PURPLE },
        { "rebeccapurple", "rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { "red", "red", THTMLCOL, COL_SM_RED },
        { "rgb", "rgb", TRGB, COL_AUTO },
        { "rgba", "rgba", TRGBA, COL_AUTO },
        { "silver", "silver", THTMLCOL, COL_SM_SILVER },
        { "teal", "teal", THTMLCOL, COL_SM_TEAL },
        { "ubuntu", "", TICONICCOL, COL_SM_UBUNTU_ORANGE },
        { "white", "white", THTMLCOL, COL_SM_WHITE },
        { "yellow", "yellow", THTMLCOL, COL_SM_YELLOW } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableHTML[]
    = { { "aqua", "aqua", THTMLCOL, COL_SM_AQUA },
        { "black", "black", THTMLCOL, COL_SM_BLACK },
        { "blue", "blue", THTMLCOL, COL_SM_BLUE },
        { "cyan", "cyan", THTMLCOL, COL_SM_BLACK },
        { "fuchsia", "fuchsia", THTMLCOL, COL_SM_FUCHSIA },
        { "gray", "gray", THTMLCOL, COL_SM_GRAY },
        { "green", "green", THTMLCOL, COL_SM_GREEN },
        { "hex", "hex", THEX, COL_SM_BLACK },
        { "lime", "lime", THTMLCOL, COL_SM_LIME },
        { "magenta", "magenta", THTMLCOL, COL_SM_FUCHSIA },
        { "maroon", "maroon", THTMLCOL, COL_SM_MAROON },
        { "navy", "navy", THTMLCOL, COL_SM_NAVY },
        { "olive", "olive", THTMLCOL, COL_SM_OLIVE },
        { "purple", "purple", THTMLCOL, COL_SM_PURPLE },
        { "rebeccapurple", "rebeccapurple", THTMLCOL, COL_SM_REBECCAPURPLE },
        { "red", "red", THTMLCOL, COL_SM_RED },
        { "silver", "silver", THTMLCOL, COL_SM_SILVER },
        { "teal", "teal", THTMLCOL, COL_SM_TEAL },
        { "white", "white", THTMLCOL, COL_SM_WHITE },
        { "yellow", "yellow", THTMLCOL, COL_SM_YELLOW } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableDVIPSNAMES[] = {
    { "apricot", "Apricot", TDVIPSNAMESCOL, COL_SM_DIV_APRICOT },
    { "aquamarine", "Aquamarine", TDVIPSNAMESCOL, COL_SM_DIV_AQUAMARINE },
    { "bittersweet", "Bittersweet", TDVIPSNAMESCOL, COL_SM_DIV_BITTERSWEET },
    { "dblack", "Black", TDVIPSNAMESCOL, COL_SM_BLACK },
    { "dblue", "Blue", TDVIPSNAMESCOL, COL_SM_BLACK },
};

std::unique_ptr<SmColorTokenTableEntry> starmathdatabase::Identify_Color_Parser(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
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
        return nullptr;
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return nullptr;
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_HTML(const OUString& colorname)
{
    if (colorname.isEmpty())
        return nullptr;
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    if (colorname[0] == '#' && colorname.getLength() == 7)
        return std::unique_ptr<SmColorTokenTableEntry>(
            new SmColorTokenTableEntry("", "", TRGB, colorname.copy(1, 6).toUInt32(16)));
    return nullptr;
}

std::unique_ptr<SmColorTokenTableEntry>
starmathdatabase::Identify_ColorName_DVIPSNAMES(const OUString& colorname)
{
    if (colorname.isEmpty())
        return nullptr;
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(colorname))
            return std::unique_ptr<SmColorTokenTableEntry>(new SmColorTokenTableEntry(i));
    return nullptr;
}

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
#include <types.hxx>

SmToken starmathdatabase::Identify_SmXMLOperatorContext_Impl(std::u16string_view rText,
                                                             bool bIsStretchy, sal_Int32 nIndex)
{
    auto cChar = o3tl::iterateCodePoints(rText, &nIndex);
    switch (cChar)
    {
        case MS_COPROD:
            return SmToken(TCOPROD, MS_COPROD, u"coprod"_ustr, TG::Oper, 5);
        case MS_IIINT:
            return SmToken(TIIINT, MS_IIINT, u"iiint"_ustr, TG::Oper, 5);
        case MS_IINT:
            return SmToken(TIINT, MS_IINT, u"iint"_ustr, TG::Oper, 5);
        case MS_INT:
            if (bIsStretchy)
                return SmToken(TINTD, MS_INT, u"intd"_ustr, TG::Oper, 5);
            else
                return SmToken(TINT, MS_INT, u"int"_ustr, TG::Oper, 5);
        case MS_LINT:
            return SmToken(TLINT, MS_LINT, u"lint"_ustr, TG::Oper, 5);
        case MS_LLINT:
            return SmToken(TLLINT, MS_LLINT, u"llint"_ustr, TG::Oper, 5);
        case MS_LLLINT:
            return SmToken(TLLLINT, MS_LLLINT, u"lllint"_ustr, TG::Oper, 5);
        case MS_PROD:
            return SmToken(TPROD, MS_PROD, u"prod"_ustr, TG::Oper, 5);
        case MS_SUM:
            return SmToken(TSUM, MS_SUM, u"sum"_ustr, TG::Oper, 5);
        case MS_MAJ:
            return SmToken(TSUM, MS_MAJ, u"maj"_ustr, TG::Oper, 5);
        case MS_FACT:
            return SmToken(TFACT, MS_FACT, u"!"_ustr, TG::UnOper, 5);
        case MS_NEG:
            return SmToken(TNEG, MS_NEG, u"neg"_ustr, TG::UnOper, 5);
        case MS_OMINUS:
            return SmToken(TOMINUS, MS_OMINUS, u"ominus"_ustr, TG::Sum, 0);
        case MS_OPLUS:
            return SmToken(TOPLUS, MS_OPLUS, u"oplus"_ustr, TG::Sum, 0);
        case MS_UNION:
            return SmToken(TUNION, MS_UNION, u"union"_ustr, TG::Sum, 0);
        case MS_OR:
            return SmToken(TOR, MS_OR, u"|"_ustr, TG::Sum, 5);
        case MS_PLUSMINUS:
            return SmToken(TPLUSMINUS, MS_PLUSMINUS, u"+-"_ustr, TG::Sum | TG::UnOper, 5);
        case MS_MINUSPLUS:
            return SmToken(TMINUSPLUS, MS_MINUSPLUS, u"-+"_ustr, TG::Sum | TG::UnOper, 5);
        case 0xe083:
        case MS_PLUS:
            return SmToken(TPLUS, MS_PLUS, u"+"_ustr, TG::Sum | TG::UnOper, 5);
        case MS_MINUS:
            return SmToken(TMINUS, MS_MINUS, u"-"_ustr, TG::Sum | TG::UnOper, 5);
        case 0x2022:
        case MS_CDOT:
            return SmToken(TCDOT, MS_CDOT, u"cdot"_ustr, TG::Product, 0);
        case MS_DIV:
            return SmToken(TDIV, MS_DIV, u"div"_ustr, TG::Product, 0);
        case MS_TIMES:
            return SmToken(TTIMES, MS_TIMES, u"times"_ustr, TG::Product, 0);
        case MS_INTERSECT:
            return SmToken(TINTERSECT, MS_INTERSECT, u"intersection"_ustr, TG::Product, 0);
        case MS_ODIVIDE:
            return SmToken(TODIVIDE, MS_ODIVIDE, u"odivide"_ustr, TG::Product, 0);
        case MS_ODOT:
            return SmToken(TODOT, MS_ODOT, u"odot"_ustr, TG::Product, 0);
        case MS_OTIMES:
            return SmToken(TOTIMES, MS_OTIMES, u"otimes"_ustr, TG::Product, 0);
        case MS_AND:
            return SmToken(TAND, MS_AND, u"&"_ustr, TG::Product, 0);
        case MS_MULTIPLY:
            return SmToken(TMULTIPLY, MS_MULTIPLY, u"*"_ustr, TG::Product, 0);
        case MS_SLASH:
            if (bIsStretchy)
                return SmToken(TWIDESLASH, MS_SLASH, u"wideslash"_ustr, TG::Product, 0);
            else
                return SmToken(TSLASH, MS_SLASH, u"slash"_ustr, TG::Product, 0);
        case MS_BACKSLASH:
            if (bIsStretchy)
                return SmToken(TWIDEBACKSLASH, MS_BACKSLASH, u"widebslash"_ustr, TG::Product, 0);
            else
                return SmToken(TBACKSLASH, MS_BACKSLASH, u"bslash"_ustr, TG::Product, 0);
        case MS_DEF:
            return SmToken(TDEF, MS_DEF, u"def"_ustr, TG::Relation, 0);
        case MS_LINE:
            return SmToken(TDIVIDES, MS_LINE, u"divides"_ustr, TG::Relation, 0);
        case MS_EQUIV:
            return SmToken(TEQUIV, MS_EQUIV, u"equiv"_ustr, TG::Relation, 0);
        case MS_GE:
            return SmToken(TGE, MS_GE, u">="_ustr, TG::Relation, 0);
        case MS_GESLANT:
            return SmToken(TGESLANT, MS_GESLANT, u"geslant"_ustr, TG::Relation, 0);
        case MS_GG:
            return SmToken(TGG, MS_GG, u">>"_ustr, TG::Relation, 0);
        case MS_GT:
            return SmToken(TGT, MS_GT, u">"_ustr, TG::Relation, 0);
        case MS_IN:
            return SmToken(TIN, MS_IN, u"in"_ustr, TG::Relation, 0);
        case MS_LE:
            return SmToken(TLE, MS_LE, u"<="_ustr, TG::Relation, 0);
        case MS_LESLANT:
            return SmToken(TLESLANT, MS_LESLANT, u"leslant"_ustr, TG::Relation, 0);
        case MS_LL:
            return SmToken(TLL, MS_LL, u"<<"_ustr, TG::Relation, 0);
        case MS_LT:
            return SmToken(TLT, MS_LT, u"<"_ustr, TG::Relation, 0);
        case MS_NDIVIDES:
            return SmToken(TNDIVIDES, MS_NDIVIDES, u"ndivides"_ustr, TG::Relation, 0);
        case MS_NEQ:
            return SmToken(TNEQ, MS_NEQ, u"<>"_ustr, TG::Relation, 0);
        case MS_NOTIN:
            return SmToken(TNOTIN, MS_NOTIN, u"notin"_ustr, TG::Relation, 0);
        case MS_NOTPRECEDES:
            return SmToken(TNOTPRECEDES, MS_NOTPRECEDES, u"nprec"_ustr, TG::Relation, 0);
        case MS_NSUBSET:
            return SmToken(TNSUBSET, MS_NSUBSET, u"nsubset"_ustr, TG::Relation, 0);
        case MS_NSUBSETEQ:
            return SmToken(TNSUBSETEQ, MS_NSUBSETEQ, u"nsubseteq"_ustr, TG::Relation, 0);
        case MS_NOTSUCCEEDS:
            return SmToken(TNOTSUCCEEDS, MS_NOTSUCCEEDS, u"nsucc"_ustr, TG::Relation, 0);
        case MS_NSUPSET:
            return SmToken(TNSUPSET, MS_NSUPSET, u"nsupset"_ustr, TG::Relation, 0);
        case MS_NSUPSETEQ:
            return SmToken(TNSUPSETEQ, MS_NSUPSETEQ, u"nsupseteq"_ustr, TG::Relation, 0);
        case MS_ORTHO:
            return SmToken(TORTHO, MS_ORTHO, u"ortho"_ustr, TG::Relation, 0);
        case MS_NI:
            return SmToken(TNI, MS_NI, u"owns"_ustr, TG::Relation, 0);
        case MS_DLINE:
            return SmToken(TPARALLEL, MS_DLINE, u"parallel"_ustr, TG::Relation, 0);
        case MS_PRECEDES:
            return SmToken(TPRECEDES, MS_PRECEDES, u"prec"_ustr, TG::Relation, 0);
        case MS_PRECEDESEQUAL:
            return SmToken(TPRECEDESEQUAL, MS_PRECEDESEQUAL, u"preccurlyeq"_ustr, TG::Relation, 0);
        case MS_PRECEDESEQUIV:
            return SmToken(TPRECEDESEQUIV, MS_PRECEDESEQUIV, u"precsim"_ustr, TG::Relation, 0);
        case MS_PROP:
            return SmToken(TPROP, MS_PROP, u"prop"_ustr, TG::Relation, 0);
        case MS_SIM:
            return SmToken(TSIM, MS_SIM, u"sim"_ustr, TG::Relation, 0);
        case 0x2245:
        case MS_SIMEQ:
            return SmToken(TSIMEQ, MS_SIMEQ, u"simeq"_ustr, TG::Relation, 0);
        case MS_SUBSET:
            return SmToken(TSUBSET, MS_SUBSET, u"subset"_ustr, TG::Relation, 0);
        case MS_SUBSETEQ:
            return SmToken(TSUBSETEQ, MS_SUBSETEQ, u"subseteq"_ustr, TG::Relation, 0);
        case MS_SUCCEEDS:
            return SmToken(TSUCCEEDS, MS_SUCCEEDS, u"succ"_ustr, TG::Relation, 0);
        case MS_SUCCEEDSEQUAL:
            return SmToken(TSUCCEEDSEQUAL, MS_SUCCEEDSEQUAL, u"succcurlyeq"_ustr, TG::Relation, 0);
        case MS_SUCCEEDSEQUIV:
            return SmToken(TSUCCEEDSEQUIV, MS_SUCCEEDSEQUIV, u"succsim"_ustr, TG::Relation, 0);
        case MS_SUPSET:
            return SmToken(TSUPSET, MS_SUPSET, u"supset"_ustr, TG::Relation, 0);
        case MS_SUPSETEQ:
            return SmToken(TSUPSETEQ, MS_SUPSETEQ, u"supseteq"_ustr, TG::Relation, 0);
        case MS_RIGHTARROW:
            return SmToken(TTOWARD, MS_RIGHTARROW, u"toward"_ustr, TG::Relation, 0);
        case MS_TRANSL:
            return SmToken(TTRANSL, MS_TRANSL, u"transl"_ustr, TG::Relation, 0);
        case MS_TRANSR:
            return SmToken(TTRANSR, MS_TRANSR, u"transr"_ustr, TG::Relation, 0);
        case MS_ASSIGN:
            return SmToken(TASSIGN, MS_ASSIGN, u"="_ustr, TG::Relation, 0);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace"_ustr, TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil"_ustr, TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor"_ustr, TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket"_ustr, TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"["_ustr, TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"("_ustr, TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace"_ustr, TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil"_ustr, TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor"_ustr, TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket"_ustr, TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]"_ustr, TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")"_ustr, TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none"_ustr, TG::RBrace | TG::LBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u""_ustr, TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_Prefix_SmXMLOperatorContext_Impl(std::u16string_view rText,
                                                                    sal_Int32 nIndex)
{
    auto cChar = o3tl::iterateCodePoints(rText, &nIndex);
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TLLINE, MS_VERTLINE, u"lline"_ustr, TG::LBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TLDLINE, MS_DVERTLINE, u"ldline"_ustr, TG::LBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace"_ustr, TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil"_ustr, TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor"_ustr, TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket"_ustr, TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"["_ustr, TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"("_ustr, TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace"_ustr, TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil"_ustr, TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor"_ustr, TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket"_ustr, TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]"_ustr, TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")"_ustr, TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none"_ustr, TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u""_ustr, TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken starmathdatabase::Identify_Postfix_SmXMLOperatorContext_Impl(std::u16string_view rText,
                                                                     sal_Int32 nIndex)
{
    auto cChar = o3tl::iterateCodePoints(rText, &nIndex);
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TRLINE, MS_VERTLINE, u"rline"_ustr, TG::RBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TRDLINE, MS_DVERTLINE, u"rdline"_ustr, TG::RBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace"_ustr, TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil"_ustr, TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor"_ustr, TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket"_ustr, TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"["_ustr, TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"("_ustr, TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace"_ustr, TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil"_ustr, TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor"_ustr, TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket"_ustr, TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]"_ustr, TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")"_ustr, TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none"_ustr, TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u""_ustr, TG::NONE, SAL_MAX_UINT16);
    }
}

SmToken
starmathdatabase::Identify_PrefixPostfix_SmXMLOperatorContext_Impl(std::u16string_view rText,
                                                                   sal_Int32 nIndex)
{
    auto cChar = o3tl::iterateCodePoints(rText, &nIndex);
    switch (cChar)
    {
        case MS_VERTLINE:
            return SmToken(TLRLINE, MS_VERTLINE, u"lrline"_ustr, TG::LBrace | TG::RBrace, 5);
        case MS_DVERTLINE:
            return SmToken(TLRDLINE, MS_DVERTLINE, u"lrdline"_ustr, TG::LBrace | TG::RBrace, 5);
        case MS_LANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LMATHANGLE:
            return SmToken(TLANGLE, MS_LMATHANGLE, u"langle"_ustr, TG::LBrace, 5);
        case MS_LBRACE:
            return SmToken(TLBRACE, MS_LBRACE, u"lbrace"_ustr, TG::LBrace, 5);
        case MS_LCEIL:
            return SmToken(TLCEIL, MS_LCEIL, u"lceil"_ustr, TG::LBrace, 5);
        case MS_LFLOOR:
            return SmToken(TLFLOOR, MS_LFLOOR, u"lfloor"_ustr, TG::LBrace, 5);
        case MS_LDBRACKET:
            return SmToken(TLDBRACKET, MS_LDBRACKET, u"ldbracket"_ustr, TG::LBrace, 5);
        case MS_LBRACKET:
            return SmToken(TLBRACKET, MS_LBRACKET, u"["_ustr, TG::LBrace, 5);
        case MS_LPARENT:
            return SmToken(TLPARENT, MS_LPARENT, u"("_ustr, TG::LBrace, 5);
        case MS_RANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RMATHANGLE:
            return SmToken(TRANGLE, MS_RMATHANGLE, u"rangle"_ustr, TG::RBrace, 5);
        case MS_RBRACE:
            return SmToken(TRBRACE, MS_RBRACE, u"rbrace"_ustr, TG::RBrace, 5);
        case MS_RCEIL:
            return SmToken(TRCEIL, MS_RCEIL, u"rceil"_ustr, TG::RBrace, 5);
        case MS_RFLOOR:
            return SmToken(TRFLOOR, MS_RFLOOR, u"rfloor"_ustr, TG::RBrace, 5);
        case MS_RDBRACKET:
            return SmToken(TRDBRACKET, MS_RDBRACKET, u"rdbracket"_ustr, TG::RBrace, 5);
        case MS_RBRACKET:
            return SmToken(TRBRACKET, MS_RBRACKET, u"]"_ustr, TG::RBrace, 5);
        case MS_RPARENT:
            return SmToken(TRPARENT, MS_RPARENT, u")"_ustr, TG::RBrace, 5);
        case MS_NONE:
            return SmToken(TNONE, MS_NONE, u"none"_ustr, TG::LBrace | TG::RBrace, 5);
        default:
            return SmToken(TERROR, MS_NONE, u""_ustr, TG::NONE, SAL_MAX_UINT16);
    }
}

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableParse[]
    = { { u"aliceblue"_ustr, THTMLCOL, COL_SM_ALICEBLUE },
        { u"antiquewhite"_ustr, THTMLCOL, COL_SM_ANTIQUEWHITE },
        { u"aqua"_ustr, TMATHMLCOL, COL_SM_AQUA },
        { u"aquamarine"_ustr, THTMLCOL, COL_SM_AQUAMARINE },
        { u"azure"_ustr, THTMLCOL, COL_SM_AZURE },
        { u"beige"_ustr, THTMLCOL, COL_SM_BEIGE },
        { u"bisque"_ustr, THTMLCOL, COL_SM_BISQUE },
        { u"black"_ustr, TMATHMLCOL, COL_SM_BLACK },
        { u"blanchedalmond"_ustr, THTMLCOL, COL_SM_BLANCHEDALMOND },
        { u"blue"_ustr, TMATHMLCOL, COL_SM_BLUE },
        { u"blueviolet"_ustr, THTMLCOL, COL_SM_BLUEVIOLET },
        { u"brown"_ustr, THTMLCOL, COL_SM_BROWN },
        { u"burlywood"_ustr, THTMLCOL, COL_SM_BURLYWOOD },
        { u"cadetblue"_ustr, THTMLCOL, COL_SM_CADETBLUE },
        { u"chartreuse"_ustr, THTMLCOL, COL_SM_CHARTREUSE },
        { u"chocolate"_ustr, THTMLCOL, COL_SM_CHOCOLATE },
        { u"coral"_ustr, THTMLCOL, COL_SM_CORAL },
        { u"cornflowerblue"_ustr, THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { u"cornsilk"_ustr, THTMLCOL, COL_SM_CORNSILK },
        { u"crimson"_ustr, THTMLCOL, COL_SM_CRIMSON },
        { u"cyan"_ustr, TMATHMLCOL, COL_SM_CYAN },
        { u"darkblue"_ustr, THTMLCOL, COL_SM_DARKBLUE },
        { u"darkcyan"_ustr, THTMLCOL, COL_SM_DARKCYAN },
        { u"darkgoldenrod"_ustr, THTMLCOL, COL_SM_DARKGOLDENROD },
        { u"darkgray"_ustr, THTMLCOL, COL_SM_DARKGRAY },
        { u"darkgreen"_ustr, THTMLCOL, COL_SM_DARKGREEN },
        { u"darkgrey"_ustr, THTMLCOL, COL_SM_DARKGREY },
        { u"darkkhaki"_ustr, THTMLCOL, COL_SM_DARKKHAKI },
        { u"darkmagenta"_ustr, THTMLCOL, COL_SM_DARKMAGENTA },
        { u"darkolivegreen"_ustr, THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { u"darkorange"_ustr, THTMLCOL, COL_SM_DARKORANGE },
        { u"darkorchid"_ustr, THTMLCOL, COL_SM_DARKORCHID },
        { u"darkred"_ustr, THTMLCOL, COL_SM_DARKRED },
        { u"darksalmon"_ustr, THTMLCOL, COL_SM_DARKSALMON },
        { u"darkseagreen"_ustr, THTMLCOL, COL_SM_DARKSEAGREEN },
        { u"darkslateblue"_ustr, THTMLCOL, COL_SM_DARKSLATEBLUE },
        { u"darkslategray"_ustr, THTMLCOL, COL_SM_DARKSLATEGRAY },
        { u"darkslategrey"_ustr, THTMLCOL, COL_SM_DARKSLATEGREY },
        { u"darkturquoise"_ustr, THTMLCOL, COL_SM_DARKTURQUOISE },
        { u"darkviolet"_ustr, THTMLCOL, COL_SM_DARKVIOLET },
        { u"debian"_ustr, TICONICCOL, COL_SM_DEBIAN_MAGENTA },
        { u"deeppink"_ustr, THTMLCOL, COL_SM_DEEPPINK },
        { u"deepskyblue"_ustr, THTMLCOL, COL_SM_DEEPSKYBLUE },
        { u"dimgray"_ustr, THTMLCOL, COL_SM_DIMGRAY },
        { u"dimgrey"_ustr, THTMLCOL, COL_SM_DIMGREY },
        { u"dodgerblue"_ustr, THTMLCOL, COL_SM_DODGERBLUE },
        { u"dvip"_ustr, TDVIPSNAMESCOL, COL_SM_BLACK },
        { u"firebrick"_ustr, THTMLCOL, COL_SM_FIREBRICK },
        { u"floralwhite"_ustr, THTMLCOL, COL_SM_FLORALWHITE },
        { u"forestgreen"_ustr, THTMLCOL, COL_SM_FORESTGREEN },
        { u"fuchsia"_ustr, TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gainsboro"_ustr, THTMLCOL, COL_SM_GAINSBORO },
        { u"ghostwhite"_ustr, THTMLCOL, COL_SM_GHOSTWHITE },
        { u"gold"_ustr, THTMLCOL, COL_SM_GOLD },
        { u"goldenrod"_ustr, THTMLCOL, COL_SM_GOLDENROD },
        { u"gray"_ustr, TMATHMLCOL, COL_SM_GRAY },
        { u"green"_ustr, TMATHMLCOL, COL_SM_GREEN },
        { u"greenyellow"_ustr, THTMLCOL, COL_SM_GREENYELLOW },
        { u"grey"_ustr, THTMLCOL, COL_SM_GREY },
        { u"hex"_ustr, THEX, COL_SM_BLACK },
        { u"honeydew"_ustr, THTMLCOL, COL_SM_HONEYDEW },
        { u"hotpink"_ustr, THTMLCOL, COL_SM_HOTPINK },
        { u"indianred"_ustr, THTMLCOL, COL_SM_INDIANRED },
        { u"indigo"_ustr, THTMLCOL, COL_SM_INDIGO },
        { u"ivory"_ustr, THTMLCOL, COL_SM_IVORY },
        { u"khaki"_ustr, THTMLCOL, COL_SM_KHAKI },
        { u"lavender"_ustr, THTMLCOL, COL_SM_LAVENDER },
        { u"lavenderblush"_ustr, THTMLCOL, COL_SM_LAVENDERBLUSH },
        { u"lawngreen"_ustr, THTMLCOL, COL_SM_LAWNGREEN },
        { u"lemonchiffon"_ustr, THTMLCOL, COL_SM_LEMONCHIFFON },
        { u"lightblue"_ustr, THTMLCOL, COL_SM_LIGHTBLUE },
        { u"lightcoral"_ustr, THTMLCOL, COL_SM_LIGHTCORAL },
        { u"lightcyan"_ustr, THTMLCOL, COL_SM_LIGHTCYAN },
        { u"lightgoldenrodyellow"_ustr, THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { u"lightgray"_ustr, THTMLCOL, COL_SM_LIGHTGRAY },
        { u"lightgreen"_ustr, THTMLCOL, COL_SM_LIGHTGREEN },
        { u"lightgrey"_ustr, THTMLCOL, COL_SM_LIGHTGREY },
        { u"lightpink"_ustr, THTMLCOL, COL_SM_LIGHTPINK },
        { u"lightsalmon"_ustr, THTMLCOL, COL_SM_LIGHTSALMON },
        { u"lightseagreen"_ustr, THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { u"lightskyblue"_ustr, THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { u"lightslategray"_ustr, THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { u"lightslategrey"_ustr, THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { u"lightsteelblue"_ustr, THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { u"lightyellow"_ustr, THTMLCOL, COL_SM_LIGHTYELLOW },
        { u"lime"_ustr, TMATHMLCOL, COL_SM_LIME },
        { u"limegreen"_ustr, THTMLCOL, COL_SM_LIMEGREEN },
        { u"linen"_ustr, THTMLCOL, COL_SM_LINEN },
        { u"lo"_ustr, TICONICCOL, COL_SM_LO_GREEN },
        { u"magenta"_ustr, TMATHMLCOL, COL_SM_MAGENTA },
        { u"maroon"_ustr, TMATHMLCOL, COL_SM_MAROON },
        { u"mediumaquamarine"_ustr, THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { u"mediumblue"_ustr, THTMLCOL, COL_SM_MEDIUMBLUE },
        { u"mediumorchid"_ustr, THTMLCOL, COL_SM_MEDIUMORCHID },
        { u"mediumpurple"_ustr, THTMLCOL, COL_SM_MEDIUMPURPLE },
        { u"mediumseagreen"_ustr, THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { u"mediumslateblue"_ustr, THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { u"mediumspringgreen"_ustr, THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { u"mediumturquoise"_ustr, THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { u"mediumvioletred"_ustr, THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { u"midnightblue"_ustr, THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { u"mintcream"_ustr, THTMLCOL, COL_SM_MINTCREAM },
        { u"mistyrose"_ustr, THTMLCOL, COL_SM_MISTYROSE },
        { u"moccasin"_ustr, THTMLCOL, COL_SM_MOCCASIN },
        { u"navajowhite"_ustr, THTMLCOL, COL_SM_NAVAJOWHITE },
        { u"navy"_ustr, TMATHMLCOL, COL_SM_NAVY },
        { u"oldlace"_ustr, THTMLCOL, COL_SM_OLDLACE },
        { u"olive"_ustr, TMATHMLCOL, COL_SM_OLIVE },
        { u"olivedrab"_ustr, THTMLCOL, COL_SM_OLIVEDRAB },
        { u"orange"_ustr, THTMLCOL, COL_SM_ORANGE },
        { u"orangered"_ustr, THTMLCOL, COL_SM_ORANGERED },
        { u"orchid"_ustr, THTMLCOL, COL_SM_ORCHID },
        { u"palegoldenrod"_ustr, THTMLCOL, COL_SM_PALEGOLDENROD },
        { u"palegreen"_ustr, THTMLCOL, COL_SM_PALEGREEN },
        { u"paleturquoise"_ustr, THTMLCOL, COL_SM_PALETURQUOISE },
        { u"palevioletred"_ustr, THTMLCOL, COL_SM_PALEVIOLETRED },
        { u"papayawhip"_ustr, THTMLCOL, COL_SM_PAPAYAWHIP },
        { u"peachpuff"_ustr, THTMLCOL, COL_SM_PEACHPUFF },
        { u"peru"_ustr, THTMLCOL, COL_SM_PERU },
        { u"pink"_ustr, THTMLCOL, COL_SM_PINK },
        { u"plum"_ustr, THTMLCOL, COL_SM_PLUM },
        { u"powderblue"_ustr, THTMLCOL, COL_SM_POWDERBLUE },
        { u"purple"_ustr, TMATHMLCOL, COL_SM_PURPLE },
        { u"rebeccapurple"_ustr, THTMLCOL, COL_SM_REBECCAPURPLE },
        { u"red"_ustr, TMATHMLCOL, COL_SM_RED },
        { u"rgb"_ustr, TRGB, COL_AUTO },
        { u"rgba"_ustr, TRGBA, COL_AUTO },
        { u"rosybrown"_ustr, THTMLCOL, COL_SM_ROSYBROWN },
        { u"royalblue"_ustr, THTMLCOL, COL_SM_ROYALBLUE },
        { u"saddlebrown"_ustr, THTMLCOL, COL_SM_SADDLEBROWN },
        { u"salmon"_ustr, THTMLCOL, COL_SM_SALMON },
        { u"sandybrown"_ustr, THTMLCOL, COL_SM_SANDYBROWN },
        { u"seagreen"_ustr, THTMLCOL, COL_SM_SEAGREEN },
        { u"seashell"_ustr, THTMLCOL, COL_SM_SEASHELL },
        { u"sienna"_ustr, THTMLCOL, COL_SM_SIENNA },
        { u"silver"_ustr, TMATHMLCOL, COL_SM_SILVER },
        { u"skyblue"_ustr, THTMLCOL, COL_SM_SKYBLUE },
        { u"slateblue"_ustr, THTMLCOL, COL_SM_SLATEBLUE },
        { u"slategray"_ustr, THTMLCOL, COL_SM_SLATEGRAY },
        { u"slategrey"_ustr, THTMLCOL, COL_SM_SLATEGREY },
        { u"snow"_ustr, THTMLCOL, COL_SM_SNOW },
        { u"springgreen"_ustr, THTMLCOL, COL_SM_SPRINGGREEN },
        { u"steelblue"_ustr, THTMLCOL, COL_SM_STEELBLUE },
        { u"tan"_ustr, THTMLCOL, COL_SM_TAN },
        { u"teal"_ustr, TMATHMLCOL, COL_SM_TEAL },
        { u"thistle"_ustr, THTMLCOL, COL_SM_THISTLE },
        { u"tomato"_ustr, THTMLCOL, COL_SM_TOMATO },
        { u"turquoise"_ustr, THTMLCOL, COL_SM_TURQUOISE },
        { u"ubuntu"_ustr, TICONICCOL, COL_SM_UBUNTU_ORANGE },
        { u"violet"_ustr, THTMLCOL, COL_SM_VIOLET },
        { u"wheat"_ustr, THTMLCOL, COL_SM_WHEAT },
        { u"white"_ustr, TMATHMLCOL, COL_SM_WHITE },
        { u"whitesmoke"_ustr, THTMLCOL, COL_SM_WHITESMOKE },
        { u"yellow"_ustr, TMATHMLCOL, COL_SM_YELLOW },
        { u"yellowgreen"_ustr, THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableHTML[]
    = { { u"aliceblue"_ustr, THTMLCOL, COL_SM_ALICEBLUE },
        { u"antiquewhite"_ustr, THTMLCOL, COL_SM_ANTIQUEWHITE },
        { u"aqua"_ustr, TMATHMLCOL, COL_SM_AQUA },
        { u"aquamarine"_ustr, THTMLCOL, COL_SM_AQUAMARINE },
        { u"azure"_ustr, THTMLCOL, COL_SM_AZURE },
        { u"beige"_ustr, THTMLCOL, COL_SM_BEIGE },
        { u"bisque"_ustr, THTMLCOL, COL_SM_BISQUE },
        { u"black"_ustr, TMATHMLCOL, COL_SM_BLACK },
        { u"blanchedalmond"_ustr, THTMLCOL, COL_SM_BLANCHEDALMOND },
        { u"blue"_ustr, TMATHMLCOL, COL_SM_BLUE },
        { u"blueviolet"_ustr, THTMLCOL, COL_SM_BLUEVIOLET },
        { u"brown"_ustr, THTMLCOL, COL_SM_BROWN },
        { u"burlywood"_ustr, THTMLCOL, COL_SM_BURLYWOOD },
        { u"cadetblue"_ustr, THTMLCOL, COL_SM_CADETBLUE },
        { u"chartreuse"_ustr, THTMLCOL, COL_SM_CHARTREUSE },
        { u"chocolate"_ustr, THTMLCOL, COL_SM_CHOCOLATE },
        { u"coral"_ustr, THTMLCOL, COL_SM_CORAL },
        { u"cornflowerblue"_ustr, THTMLCOL, COL_SM_CORNFLOWERBLUE },
        { u"cornsilk"_ustr, THTMLCOL, COL_SM_CORNSILK },
        { u"crimson"_ustr, THTMLCOL, COL_SM_CRIMSON },
        { u"cyan"_ustr, TMATHMLCOL, COL_SM_CYAN },
        { u"darkblue"_ustr, THTMLCOL, COL_SM_DARKBLUE },
        { u"darkcyan"_ustr, THTMLCOL, COL_SM_DARKCYAN },
        { u"darkgoldenrod"_ustr, THTMLCOL, COL_SM_DARKGOLDENROD },
        { u"darkgray"_ustr, THTMLCOL, COL_SM_DARKGRAY },
        { u"darkgreen"_ustr, THTMLCOL, COL_SM_DARKGREEN },
        { u"darkgrey"_ustr, THTMLCOL, COL_SM_DARKGREY },
        { u"darkkhaki"_ustr, THTMLCOL, COL_SM_DARKKHAKI },
        { u"darkmagenta"_ustr, THTMLCOL, COL_SM_DARKMAGENTA },
        { u"darkolivegreen"_ustr, THTMLCOL, COL_SM_DARKOLIVEGREEN },
        { u"darkorange"_ustr, THTMLCOL, COL_SM_DARKORANGE },
        { u"darkorchid"_ustr, THTMLCOL, COL_SM_DARKORCHID },
        { u"darkred"_ustr, THTMLCOL, COL_SM_DARKRED },
        { u"darksalmon"_ustr, THTMLCOL, COL_SM_DARKSALMON },
        { u"darkseagreen"_ustr, THTMLCOL, COL_SM_DARKSEAGREEN },
        { u"darkslateblue"_ustr, THTMLCOL, COL_SM_DARKSLATEBLUE },
        { u"darkslategray"_ustr, THTMLCOL, COL_SM_DARKSLATEGRAY },
        { u"darkslategrey"_ustr, THTMLCOL, COL_SM_DARKSLATEGREY },
        { u"darkturquoise"_ustr, THTMLCOL, COL_SM_DARKTURQUOISE },
        { u"darkviolet"_ustr, THTMLCOL, COL_SM_DARKVIOLET },
        { u"deeppink"_ustr, THTMLCOL, COL_SM_DEEPPINK },
        { u"deepskyblue"_ustr, THTMLCOL, COL_SM_DEEPSKYBLUE },
        { u"dimgray"_ustr, THTMLCOL, COL_SM_DIMGRAY },
        { u"dimgrey"_ustr, THTMLCOL, COL_SM_DIMGREY },
        { u"dodgerblue"_ustr, THTMLCOL, COL_SM_DODGERBLUE },
        { u"firebrick"_ustr, THTMLCOL, COL_SM_FIREBRICK },
        { u"floralwhite"_ustr, THTMLCOL, COL_SM_FLORALWHITE },
        { u"forestgreen"_ustr, THTMLCOL, COL_SM_FORESTGREEN },
        { u"fuchsia"_ustr, TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gainsboro"_ustr, THTMLCOL, COL_SM_GAINSBORO },
        { u"ghostwhite"_ustr, THTMLCOL, COL_SM_GHOSTWHITE },
        { u"gold"_ustr, THTMLCOL, COL_SM_GOLD },
        { u"goldenrod"_ustr, THTMLCOL, COL_SM_GOLDENROD },
        { u"gray"_ustr, TMATHMLCOL, COL_SM_GRAY },
        { u"green"_ustr, TMATHMLCOL, COL_SM_GREEN },
        { u"greenyellow"_ustr, THTMLCOL, COL_SM_GREENYELLOW },
        { u"grey"_ustr, THTMLCOL, COL_SM_GREY },
        { u"honeydew"_ustr, THTMLCOL, COL_SM_HONEYDEW },
        { u"hotpink"_ustr, THTMLCOL, COL_SM_HOTPINK },
        { u"indianred"_ustr, THTMLCOL, COL_SM_INDIANRED },
        { u"indigo"_ustr, THTMLCOL, COL_SM_INDIGO },
        { u"ivory"_ustr, THTMLCOL, COL_SM_IVORY },
        { u"khaki"_ustr, THTMLCOL, COL_SM_KHAKI },
        { u"lavender"_ustr, THTMLCOL, COL_SM_LAVENDER },
        { u"lavenderblush"_ustr, THTMLCOL, COL_SM_LAVENDERBLUSH },
        { u"lawngreen"_ustr, THTMLCOL, COL_SM_LAWNGREEN },
        { u"lemonchiffon"_ustr, THTMLCOL, COL_SM_LEMONCHIFFON },
        { u"lightblue"_ustr, THTMLCOL, COL_SM_LIGHTBLUE },
        { u"lightcoral"_ustr, THTMLCOL, COL_SM_LIGHTCORAL },
        { u"lightcyan"_ustr, THTMLCOL, COL_SM_LIGHTCYAN },
        { u"lightgoldenrodyellow"_ustr, THTMLCOL, COL_SM_LIGHTGOLDENRODYELLOW },
        { u"lightgray"_ustr, THTMLCOL, COL_SM_LIGHTGRAY },
        { u"lightgreen"_ustr, THTMLCOL, COL_SM_LIGHTGREEN },
        { u"lightgrey"_ustr, THTMLCOL, COL_SM_LIGHTGREY },
        { u"lightpink"_ustr, THTMLCOL, COL_SM_LIGHTPINK },
        { u"lightsalmon"_ustr, THTMLCOL, COL_SM_LIGHTSALMON },
        { u"lightseagreen"_ustr, THTMLCOL, COL_SM_LIGHTSEAGREEN },
        { u"lightskyblue"_ustr, THTMLCOL, COL_SM_LIGHTSKYBLUE },
        { u"lightslategray"_ustr, THTMLCOL, COL_SM_LIGHTSLATEGRAY },
        { u"lightslategrey"_ustr, THTMLCOL, COL_SM_LIGHTSLATEGREY },
        { u"lightsteelblue"_ustr, THTMLCOL, COL_SM_LIGHTSTEELBLUE },
        { u"lightyellow"_ustr, THTMLCOL, COL_SM_LIGHTYELLOW },
        { u"lime"_ustr, TMATHMLCOL, COL_SM_LIME },
        { u"limegreen"_ustr, THTMLCOL, COL_SM_LIMEGREEN },
        { u"linen"_ustr, THTMLCOL, COL_SM_LINEN },
        { u"magenta"_ustr, TMATHMLCOL, COL_SM_MAGENTA },
        { u"maroon"_ustr, TMATHMLCOL, COL_SM_MAROON },
        { u"mediumaquamarine"_ustr, THTMLCOL, COL_SM_MEDIUMAQUAMARINE },
        { u"mediumblue"_ustr, THTMLCOL, COL_SM_MEDIUMBLUE },
        { u"mediumorchid"_ustr, THTMLCOL, COL_SM_MEDIUMORCHID },
        { u"mediumpurple"_ustr, THTMLCOL, COL_SM_MEDIUMPURPLE },
        { u"mediumseagreen"_ustr, THTMLCOL, COL_SM_MEDIUMSEAGREEN },
        { u"mediumslateblue"_ustr, THTMLCOL, COL_SM_MEDIUMSLATEBLUE },
        { u"mediumspringgreen"_ustr, THTMLCOL, COL_SM_MEDIUMSPRINGGREEN },
        { u"mediumturquoise"_ustr, THTMLCOL, COL_SM_MEDIUMTURQUOISE },
        { u"mediumvioletred"_ustr, THTMLCOL, COL_SM_MEDIUMVIOLETRED },
        { u"midnightblue"_ustr, THTMLCOL, COL_SM_MIDNIGHTBLUE },
        { u"mintcream"_ustr, THTMLCOL, COL_SM_MINTCREAM },
        { u"mistyrose"_ustr, THTMLCOL, COL_SM_MISTYROSE },
        { u"moccasin"_ustr, THTMLCOL, COL_SM_MOCCASIN },
        { u"navajowhite"_ustr, THTMLCOL, COL_SM_NAVAJOWHITE },
        { u"navy"_ustr, TMATHMLCOL, COL_SM_NAVY },
        { u"oldlace"_ustr, THTMLCOL, COL_SM_OLDLACE },
        { u"olive"_ustr, TMATHMLCOL, COL_SM_OLIVE },
        { u"olivedrab"_ustr, THTMLCOL, COL_SM_OLIVEDRAB },
        { u"orange"_ustr, THTMLCOL, COL_SM_ORANGE },
        { u"orangered"_ustr, THTMLCOL, COL_SM_ORANGERED },
        { u"orchid"_ustr, THTMLCOL, COL_SM_ORCHID },
        { u"palegoldenrod"_ustr, THTMLCOL, COL_SM_PALEGOLDENROD },
        { u"palegreen"_ustr, THTMLCOL, COL_SM_PALEGREEN },
        { u"paleturquoise"_ustr, THTMLCOL, COL_SM_PALETURQUOISE },
        { u"palevioletred"_ustr, THTMLCOL, COL_SM_PALEVIOLETRED },
        { u"papayawhip"_ustr, THTMLCOL, COL_SM_PAPAYAWHIP },
        { u"peachpuff"_ustr, THTMLCOL, COL_SM_PEACHPUFF },
        { u"peru"_ustr, THTMLCOL, COL_SM_PERU },
        { u"pink"_ustr, THTMLCOL, COL_SM_PINK },
        { u"plum"_ustr, THTMLCOL, COL_SM_PLUM },
        { u"powderblue"_ustr, THTMLCOL, COL_SM_POWDERBLUE },
        { u"purple"_ustr, TMATHMLCOL, COL_SM_PURPLE },
        { u"rebeccapurple"_ustr, THTMLCOL, COL_SM_REBECCAPURPLE },
        { u"red"_ustr, TMATHMLCOL, COL_SM_RED },
        { u"rosybrown"_ustr, THTMLCOL, COL_SM_ROSYBROWN },
        { u"royalblue"_ustr, THTMLCOL, COL_SM_ROYALBLUE },
        { u"saddlebrown"_ustr, THTMLCOL, COL_SM_SADDLEBROWN },
        { u"salmon"_ustr, THTMLCOL, COL_SM_SALMON },
        { u"sandybrown"_ustr, THTMLCOL, COL_SM_SANDYBROWN },
        { u"seagreen"_ustr, THTMLCOL, COL_SM_SEAGREEN },
        { u"seashell"_ustr, THTMLCOL, COL_SM_SEASHELL },
        { u"sienna"_ustr, THTMLCOL, COL_SM_SIENNA },
        { u"silver"_ustr, TMATHMLCOL, COL_SM_SILVER },
        { u"skyblue"_ustr, THTMLCOL, COL_SM_SKYBLUE },
        { u"slateblue"_ustr, THTMLCOL, COL_SM_SLATEBLUE },
        { u"slategray"_ustr, THTMLCOL, COL_SM_SLATEGRAY },
        { u"slategrey"_ustr, THTMLCOL, COL_SM_SLATEGREY },
        { u"snow"_ustr, THTMLCOL, COL_SM_SNOW },
        { u"springgreen"_ustr, THTMLCOL, COL_SM_SPRINGGREEN },
        { u"steelblue"_ustr, THTMLCOL, COL_SM_STEELBLUE },
        { u"tan"_ustr, THTMLCOL, COL_SM_TAN },
        { u"teal"_ustr, TMATHMLCOL, COL_SM_TEAL },
        { u"thistle"_ustr, THTMLCOL, COL_SM_THISTLE },
        { u"tomato"_ustr, THTMLCOL, COL_SM_TOMATO },
        { u"turquoise"_ustr, THTMLCOL, COL_SM_TURQUOISE },
        { u"violet"_ustr, THTMLCOL, COL_SM_VIOLET },
        { u"wheat"_ustr, THTMLCOL, COL_SM_WHEAT },
        { u"white"_ustr, TMATHMLCOL, COL_SM_WHITE },
        { u"whitesmoke"_ustr, THTMLCOL, COL_SM_WHITESMOKE },
        { u"yellow"_ustr, TMATHMLCOL, COL_SM_YELLOW },
        { u"yellowgreen"_ustr, THTMLCOL, COL_SM_YELLOWGREEN } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableDVIPS[]
    = { { u"apricot"_ustr, TDVIPSNAMESCOL, COL_SM_DIV_APRICOT },
        { u"aquamarine"_ustr, TDVIPSNAMESCOL, COL_SM_DIV_AQUAMARINE },
        { u"bittersweet"_ustr, TDVIPSNAMESCOL, COL_SM_DIV_BITTERSWEET },
        { u"black"_ustr, TDVIPSNAMESCOL, COL_SM_BLACK },
        { u"blue"_ustr, TDVIPSNAMESCOL, COL_SM_BLACK } };

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableMATHML[] = {
    // clang-format off
        { u"aqua"_ustr, TMATHMLCOL, COL_SM_AQUA },
        { u"black"_ustr, TMATHMLCOL, COL_SM_BLACK },
        { u"blue"_ustr, TMATHMLCOL, COL_SM_BLUE },
        { u"fuchsia"_ustr, TMATHMLCOL, COL_SM_FUCHSIA },
        { u"gray"_ustr, TMATHMLCOL, COL_SM_GRAY },
        { u"green"_ustr, TMATHMLCOL, COL_SM_GREEN },
        { u"lime"_ustr, TMATHMLCOL, COL_SM_LIME },
        { u"maroon"_ustr, TMATHMLCOL, COL_SM_MAROON },
        { u"navy"_ustr, TMATHMLCOL, COL_SM_NAVY },
        { u"olive"_ustr, TMATHMLCOL, COL_SM_OLIVE },
        { u"purple"_ustr, TMATHMLCOL, COL_SM_PURPLE },
        { u"red"_ustr, TMATHMLCOL, COL_SM_RED },
        { u"silver"_ustr, TMATHMLCOL, COL_SM_SILVER },
        { u"teal"_ustr, TMATHMLCOL, COL_SM_TEAL },
        { u"white"_ustr, TMATHMLCOL, COL_SM_WHITE },
        { u"yellow"_ustr, TMATHMLCOL, COL_SM_YELLOW }
    // clang-format on
};

const SmColorTokenTableEntry starmathdatabase::aColorTokenTableERROR[]
    = { { u""_ustr, TERROR, COL_SM_BLACK } };

SmColorTokenTableEntry starmathdatabase::Identify_Color_Parser(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
        if (i->equals(cColor))
            return i;
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
        if (i->equals(cColor))
            return i;
    if ((cColor & 0x00FFFFFF) == cColor)
        return SmColorTokenTableEntry(u""_ustr, TRGB, cColor);
    else
        return SmColorTokenTableEntry(u""_ustr, TRGBA, cColor);
}

SmColorTokenTableEntry starmathdatabase::Identify_Color_MATHML(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableMATHML); i < std::end(aColorTokenTableMATHML); ++i)
        if (i->equals(cColor))
            return i;
    if ((cColor & 0x00FFFFFF) == cColor)
        return SmColorTokenTableEntry(u""_ustr, TRGB, cColor);
    else
        return SmColorTokenTableEntry(u""_ustr, TRGBA, cColor);
}

SmColorTokenTableEntry starmathdatabase::Identify_Color_DVIPSNAMES(sal_uInt32 cColor)
{
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
        if (i->equals(cColor))
            return i;
    if ((cColor & 0x00FFFFFF) == cColor)
        return SmColorTokenTableEntry(u""_ustr, TRGB, cColor);
    else
        return SmColorTokenTableEntry(u""_ustr, TRGBA, cColor);
}

const SmColorTokenTableEntry*
starmathdatabase::Identify_ColorName_Parser(std::u16string_view colorname)
{
    if (colorname.empty())
        return &aColorTokenTableERROR[0];
    for (auto i = std::begin(aColorTokenTableParse); i < std::end(aColorTokenTableParse); ++i)
    {
        sal_Int32 matches = o3tl::compareToIgnoreAsciiCase(colorname, i->aIdent);
        if (matches == 0)
            return i;
        if (matches < 0)
            break;
    }
    return &aColorTokenTableERROR[0];
}
SmColorTokenTableEntry starmathdatabase::Identify_ColorName_HTML(std::u16string_view colorname)
{
    if (colorname.empty())
        return SmColorTokenTableEntry(u""_ustr, TERROR, COL_SM_BLACK);
    if (colorname[0] == '#')
    {
        Color col = Color::STRtoRGB(colorname);
        return SmColorTokenTableEntry(u""_ustr, TRGB, col);
    }
    for (auto i = std::begin(aColorTokenTableHTML); i < std::end(aColorTokenTableHTML); ++i)
    {
        sal_Int32 matches = o3tl::compareToIgnoreAsciiCase(colorname, i->aIdent);
        if (matches == 0)
            return i;
        if (matches < 0)
            break;
    }
    return SmColorTokenTableEntry(u""_ustr, TERROR, COL_SM_BLACK);
}
const SmColorTokenTableEntry*
starmathdatabase::Identify_ColorName_DVIPSNAMES(std::u16string_view colorname)
{
    if (colorname.empty())
        return &aColorTokenTableERROR[0];
    for (auto i = std::begin(aColorTokenTableDVIPS); i < std::end(aColorTokenTableDVIPS); ++i)
    {
        sal_Int32 matches = o3tl::compareToIgnoreAsciiCase(colorname, i->aIdent);
        if (matches == 0)
            return i;
        if (matches < 0)
            break;
    }
    return &aColorTokenTableERROR[0];
}

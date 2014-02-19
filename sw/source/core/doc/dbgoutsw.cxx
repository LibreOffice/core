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

#ifdef DBG_UTIL

#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemiter.hxx>
#include <string>
#include <map>
#include <node.hxx>
#include <ndtxt.hxx>
#include <ndhints.hxx>
#include <txatbase.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <undobj.hxx>
#include <numrule.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <swrect.hxx>
#include <ndarr.hxx>
#include <paratr.hxx>
#include <SwNodeNum.hxx>
#include <dbgoutsw.hxx>
#include <SwRewriter.hxx>
#include <iostream>
#include <cstdio>

using namespace std;

static OString aDbgOutResult;
bool bDbgOutStdErr = false;
bool bDbgOutPrintAttrSet = false;

char* db_pretty_print(const OUString* str, int flags, char* fmt)
{
    (void) fmt;
    (void) flags;
    return const_cast<char*>(dbg_out(*str));
}

template<class T>
static OUString lcl_dbg_out_SvPtrArr(const T & rArr)
{
    OUString aStr("[ ");

    for (typename T::const_iterator i(rArr.begin()); i != rArr.end(); ++i)
    {
        if (i != rArr.begin())
            aStr += ", ";

        if (*i)
            aStr += lcl_dbg_out(**i);
        else
            aStr += "(null)";
    }

    aStr += " ]";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const void * pVoid)
{
    char sBuffer[1024];

    sprintf(sBuffer, "%p", pVoid);

    OUString aTmpStr(sBuffer, strlen(sBuffer), RTL_TEXTENCODING_ASCII_US);

    return dbg_out(aTmpStr);
}

SW_DLLPUBLIC const char * dbg_out(const OUString & aStr)
{
    aDbgOutResult = OUStringToOString(aStr, RTL_TEXTENCODING_ASCII_US);

    if (bDbgOutStdErr)
        fprintf(stderr, "%s", aDbgOutResult.getStr());

    return aDbgOutResult.getStr();
}

struct CompareUShort
{
    bool operator()(sal_uInt16 a, sal_uInt16 b) const
    {
        return a < b;
    }
};

map<sal_uInt16,OUString,CompareUShort> & GetItemWhichMap()
{
    static map<sal_uInt16,OUString,CompareUShort> aItemWhichMap;
    static bool bInitialized = false;

    if (! bInitialized)
    {
        aItemWhichMap[RES_CHRATR_CASEMAP] = "CHRATR_CASEMAP";
        aItemWhichMap[RES_CHRATR_CHARSETCOLOR] = "CHRATR_CHARSETCOLOR";
        aItemWhichMap[RES_CHRATR_COLOR] = "CHRATR_COLOR";
        aItemWhichMap[RES_CHRATR_CONTOUR] = "CHRATR_CONTOUR";
        aItemWhichMap[RES_CHRATR_CROSSEDOUT] = "CHRATR_CROSSEDOUT";
        aItemWhichMap[RES_CHRATR_ESCAPEMENT] = "CHRATR_ESCAPEMENT";
        aItemWhichMap[RES_CHRATR_FONT] = "CHRATR_FONT";
        aItemWhichMap[RES_CHRATR_FONTSIZE] = "CHRATR_FONTSIZE";
        aItemWhichMap[RES_CHRATR_KERNING] = "CHRATR_KERNING";
        aItemWhichMap[RES_CHRATR_LANGUAGE] = "CHRATR_LANGUAGE";
        aItemWhichMap[RES_CHRATR_POSTURE] = "CHRATR_POSTURE";
        aItemWhichMap[RES_CHRATR_PROPORTIONALFONTSIZE] = "CHRATR_PROPORTIONALFONTSIZE";
        aItemWhichMap[RES_CHRATR_SHADOWED] = "CHRATR_SHADOWED";
        aItemWhichMap[RES_CHRATR_UNDERLINE] = "CHRATR_UNDERLINE";
        aItemWhichMap[RES_CHRATR_OVERLINE] = "CHRATR_OVERLINE";
        aItemWhichMap[RES_CHRATR_WEIGHT] = "CHRATR_WEIGHT";
        aItemWhichMap[RES_CHRATR_WORDLINEMODE] = "CHRATR_WORDLINEMODE";
        aItemWhichMap[RES_CHRATR_AUTOKERN] = "CHRATR_AUTOKERN";
        aItemWhichMap[RES_CHRATR_BLINK] = "CHRATR_BLINK";
        aItemWhichMap[RES_CHRATR_NOHYPHEN] = "CHRATR_NOHYPHEN";
        aItemWhichMap[RES_CHRATR_NOLINEBREAK] = "CHRATR_NOLINEBREAK";
        aItemWhichMap[RES_CHRATR_BACKGROUND] = "CHRATR_BACKGROUND";
        aItemWhichMap[RES_CHRATR_HIGHLIGHT] = "CHRATR_HIGHLIGHT";
        aItemWhichMap[RES_CHRATR_CJK_FONT] = "CHRATR_CJK_FONT";
        aItemWhichMap[RES_CHRATR_CJK_FONTSIZE] = "CHRATR_CJK_FONTSIZE";
        aItemWhichMap[RES_CHRATR_CJK_LANGUAGE] = "CHRATR_CJK_LANGUAGE";
        aItemWhichMap[RES_CHRATR_CJK_POSTURE] = "CHRATR_CJK_POSTURE";
        aItemWhichMap[RES_CHRATR_CJK_WEIGHT] = "CHRATR_CJK_WEIGHT";
        aItemWhichMap[RES_CHRATR_CTL_FONT] = "CHRATR_CTL_FONT";
        aItemWhichMap[RES_CHRATR_CTL_FONTSIZE] = "CHRATR_CTL_FONTSIZE";
        aItemWhichMap[RES_CHRATR_CTL_LANGUAGE] = "CHRATR_CTL_LANGUAGE";
        aItemWhichMap[RES_CHRATR_CTL_POSTURE] = "CHRATR_CTL_POSTURE";
        aItemWhichMap[RES_CHRATR_CTL_WEIGHT] = "CHRATR_CTL_WEIGHT";
        aItemWhichMap[RES_CHRATR_ROTATE] = "CHRATR_ROTATE";
        aItemWhichMap[RES_CHRATR_EMPHASIS_MARK] = "CHRATR_EMPHASIS_MARK";
        aItemWhichMap[RES_CHRATR_TWO_LINES] = "CHRATR_TWO_LINES";
        aItemWhichMap[RES_CHRATR_SCALEW] = "CHRATR_SCALEW";
        aItemWhichMap[RES_CHRATR_RELIEF] = "CHRATR_RELIEF";
        aItemWhichMap[RES_CHRATR_HIDDEN] = "CHRATR_HIDDEN";
        aItemWhichMap[RES_CHRATR_BOX] = "CHRATR_BOX";
        aItemWhichMap[RES_CHRATR_SHADOW] = "CHRATR_SHADOW";
        aItemWhichMap[RES_TXTATR_AUTOFMT] = "TXTATR_AUTOFMT";
        aItemWhichMap[RES_TXTATR_INETFMT] = "TXTATR_INETFMT";
        aItemWhichMap[RES_TXTATR_REFMARK] = "TXTATR_REFMARK";
        aItemWhichMap[RES_TXTATR_TOXMARK] = "TXTATR_TOXMARK";
        aItemWhichMap[RES_TXTATR_CHARFMT] = "TXTATR_CHARFMT";
        aItemWhichMap[RES_TXTATR_INPUTFIELD] = "RES_TXTATR_INPUTFIELD";
        aItemWhichMap[RES_TXTATR_CJK_RUBY] = "TXTATR_CJK_RUBY";
        aItemWhichMap[RES_TXTATR_UNKNOWN_CONTAINER] = "TXTATR_UNKNOWN_CONTAINER";
        aItemWhichMap[RES_TXTATR_META] = "TXTATR_META";
        aItemWhichMap[RES_TXTATR_METAFIELD] = "TXTATR_METAFIELD";
        aItemWhichMap[RES_TXTATR_FIELD] = "TXTATR_FIELD";
        aItemWhichMap[RES_TXTATR_FLYCNT] = "TXTATR_FLYCNT";
        aItemWhichMap[RES_TXTATR_FTN] = "TXTATR_FTN";
        aItemWhichMap[RES_TXTATR_ANNOTATION] = "TXTATR_ANNOTATION";
        aItemWhichMap[RES_TXTATR_DUMMY3] = "TXTATR_DUMMY3";
        aItemWhichMap[RES_TXTATR_DUMMY1] = "TXTATR_DUMMY1";
        aItemWhichMap[RES_TXTATR_DUMMY2] = "TXTATR_DUMMY2";
        aItemWhichMap[RES_PARATR_LINESPACING] = "PARATR_LINESPACING";
        aItemWhichMap[RES_PARATR_ADJUST] = "PARATR_ADJUST";
        aItemWhichMap[RES_PARATR_SPLIT] = "PARATR_SPLIT";
        aItemWhichMap[RES_PARATR_ORPHANS] = "PARATR_ORPHANS";
        aItemWhichMap[RES_PARATR_WIDOWS] = "PARATR_WIDOWS";
        aItemWhichMap[RES_PARATR_TABSTOP] = "PARATR_TABSTOP";
        aItemWhichMap[RES_PARATR_HYPHENZONE] = "PARATR_HYPHENZONE";
        aItemWhichMap[RES_PARATR_DROP] = "PARATR_DROP";
        aItemWhichMap[RES_PARATR_REGISTER] = "PARATR_REGISTER";
        aItemWhichMap[RES_PARATR_NUMRULE] = "PARATR_NUMRULE";
        aItemWhichMap[RES_PARATR_SCRIPTSPACE] = "PARATR_SCRIPTSPACE";
        aItemWhichMap[RES_PARATR_HANGINGPUNCTUATION] = "PARATR_HANGINGPUNCTUATION";
        aItemWhichMap[RES_PARATR_FORBIDDEN_RULES] = "PARATR_FORBIDDEN_RULES";
        aItemWhichMap[RES_PARATR_VERTALIGN] = "PARATR_VERTALIGN";
        aItemWhichMap[RES_PARATR_SNAPTOGRID] = "PARATR_SNAPTOGRID";
        aItemWhichMap[RES_PARATR_CONNECT_BORDER] = "PARATR_CONNECT_BORDER";
        aItemWhichMap[RES_FILL_ORDER] = "FILL_ORDER";
        aItemWhichMap[RES_FRM_SIZE] = "FRM_SIZE";
        aItemWhichMap[RES_PAPER_BIN] = "PAPER_BIN";
        aItemWhichMap[RES_LR_SPACE] = "LR_SPACE";
        aItemWhichMap[RES_UL_SPACE] = "UL_SPACE";
        aItemWhichMap[RES_PAGEDESC] = "PAGEDESC";
        aItemWhichMap[RES_BREAK] = "BREAK";
        aItemWhichMap[RES_CNTNT] = "CNTNT";
        aItemWhichMap[RES_HEADER] = "HEADER";
        aItemWhichMap[RES_FOOTER] = "FOOTER";
        aItemWhichMap[RES_PRINT] = "PRINT";
        aItemWhichMap[RES_OPAQUE] = "OPAQUE";
        aItemWhichMap[RES_PROTECT] = "PROTECT";
        aItemWhichMap[RES_SURROUND] = "SURROUND";
        aItemWhichMap[RES_VERT_ORIENT] = "VERT_ORIENT";
        aItemWhichMap[RES_HORI_ORIENT] = "HORI_ORIENT";
        aItemWhichMap[RES_ANCHOR] = "ANCHOR";
        aItemWhichMap[RES_BACKGROUND] = "BACKGROUND";
        aItemWhichMap[RES_BOX] = "BOX";
        aItemWhichMap[RES_SHADOW] = "SHADOW";
        aItemWhichMap[RES_FRMMACRO] = "FRMMACRO";
        aItemWhichMap[RES_COL] = "COL";
        aItemWhichMap[RES_KEEP] = "KEEP";
        aItemWhichMap[RES_URL] = "URL";
        aItemWhichMap[RES_EDIT_IN_READONLY] = "EDIT_IN_READONLY";
        aItemWhichMap[RES_LAYOUT_SPLIT] = "LAYOUT_SPLIT";
        aItemWhichMap[RES_CHAIN] = "CHAIN";
        aItemWhichMap[RES_TEXTGRID] = "TEXTGRID";
        aItemWhichMap[RES_LINENUMBER  ] = "LINENUMBER  ";
        aItemWhichMap[RES_FTN_AT_TXTEND] = "FTN_AT_TXTEND";
        aItemWhichMap[RES_END_AT_TXTEND] = "END_AT_TXTEND";
        aItemWhichMap[RES_COLUMNBALANCE] = "COLUMNBALANCE";
        aItemWhichMap[RES_FRAMEDIR] = "FRAMEDIR";
        aItemWhichMap[RES_HEADER_FOOTER_EAT_SPACING] = "HEADER_FOOTER_EAT_SPACING";
        aItemWhichMap[RES_ROW_SPLIT] = "ROW_SPLIT";
        aItemWhichMap[RES_GRFATR_MIRRORGRF] = "GRFATR_MIRRORGRF";
        aItemWhichMap[RES_GRFATR_CROPGRF] = "GRFATR_CROPGRF";
        aItemWhichMap[RES_GRFATR_ROTATION] = "GRFATR_ROTATION";
        aItemWhichMap[RES_GRFATR_LUMINANCE] = "GRFATR_LUMINANCE";
        aItemWhichMap[RES_GRFATR_CONTRAST] = "GRFATR_CONTRAST";
        aItemWhichMap[RES_GRFATR_CHANNELR] = "GRFATR_CHANNELR";
        aItemWhichMap[RES_GRFATR_CHANNELG] = "GRFATR_CHANNELG";
        aItemWhichMap[RES_GRFATR_CHANNELB] = "GRFATR_CHANNELB";
        aItemWhichMap[RES_GRFATR_GAMMA] = "GRFATR_GAMMA";
        aItemWhichMap[RES_GRFATR_INVERT] = "GRFATR_INVERT";
        aItemWhichMap[RES_GRFATR_TRANSPARENCY] = "GRFATR_TRANSPARENCY";
        aItemWhichMap[RES_GRFATR_DRAWMODE] = "GRFATR_DRAWMODE";
        aItemWhichMap[RES_BOXATR_FORMAT] = "BOXATR_FORMAT";
        aItemWhichMap[RES_BOXATR_FORMULA] = "BOXATR_FORMULA";
        aItemWhichMap[RES_BOXATR_VALUE] = "BOXATR_VALUE";

        bInitialized = true;
    }

    return aItemWhichMap;
}

static const OUString lcl_dbg_out(const SfxPoolItem & rItem)
{
    OUString aStr("[ ");

    if (GetItemWhichMap().find(rItem.Which()) != GetItemWhichMap().end())
        aStr += GetItemWhichMap()[rItem.Which()];
    else
        aStr += OUString::number(rItem.Which());

    aStr += " ]";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SfxPoolItem & rItem)
{
    return dbg_out(lcl_dbg_out(rItem));
}

SW_DLLPUBLIC const char * dbg_out(const SfxPoolItem * pItem)
{
    return dbg_out(pItem ? lcl_dbg_out(*pItem) : OUString("(nil)"));
}

static const OUString lcl_dbg_out(const SfxItemSet & rSet)
{
    SfxItemIter aIter(rSet);
    const SfxPoolItem * pItem;
    bool bFirst = true;
    OUString aStr = "[ ";

    pItem = aIter.FirstItem();

    while (pItem )
    {
        if (!bFirst)
            aStr += ", ";

        if ((sal_uIntPtr)pItem != SAL_MAX_SIZE)
            aStr += lcl_dbg_out(*pItem);
        else
            aStr += "invalid";

        bFirst = false;

        pItem = aIter.NextItem();
    }

    aStr += " ]";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SfxItemSet & rSet)
{
    return dbg_out(lcl_dbg_out(rSet));
}

static const OUString lcl_dbg_out(const SwTxtAttr & rAttr)
{
    OUString aStr("[ ");

    aStr += OUString::number(*rAttr.GetStart());
    aStr += "->";
    aStr += OUString::number(*rAttr.End());
    aStr += " ";
    aStr += lcl_dbg_out(rAttr.GetAttr());

    aStr += " ]";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwTxtAttr & rAttr)
{
    return dbg_out(lcl_dbg_out(rAttr));
}

static const OUString lcl_dbg_out(const SwpHints & rHints)
{
    OUString aStr("[ SwpHints\n");

    for (sal_uInt16 i = 0; i < rHints.Count(); i++)
    {
        aStr += "  ";
        aStr += lcl_dbg_out(*rHints[i]);
        aStr += "\n";
    }

    aStr += "]\n";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwpHints &rHints)
{
    return dbg_out(lcl_dbg_out(rHints));
}

static OUString lcl_dbg_out(const SwPosition & rPos)
{
    OUString aStr("( ");

    aStr += OUString::number(rPos.nNode.GetIndex());
    aStr += ", ";
    aStr += OUString::number(rPos.nContent.GetIndex());
    aStr += ": ";
    aStr += OUString::number
        (reinterpret_cast<sal_IntPtr>(rPos.nContent.GetIdxReg()), 16);

    aStr += " )";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwPosition & rPos)
{
    return dbg_out(lcl_dbg_out(rPos));
}

static OUString lcl_dbg_out(const SwPaM & rPam)
{
   OUString aStr("[ Pt: ");

   aStr += lcl_dbg_out(*rPam.GetPoint());

   if (rPam.HasMark())
   {
       aStr += ", Mk: ";
       aStr += lcl_dbg_out(*rPam.GetMark());
   }

   aStr += " ]";

   return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwPaM & rPam)
{
    return dbg_out(lcl_dbg_out(rPam));
}

static OUString lcl_dbg_out(const SwNodeNum & )
{
    return OUString();/*rNum.ToString();*/
}

SW_DLLPUBLIC const char * dbg_out(const SwNodeNum & rNum)
{
    return dbg_out(lcl_dbg_out(rNum));
}

static OUString lcl_dbg_out(const SwRect & rRect)
{
    OUString aResult("[ [");

    aResult += OUString::number(rRect.Left());
    aResult += ", ";
    aResult += OUString::number(rRect.Top());
    aResult += "], [";
    aResult += OUString::number(rRect.Right());
    aResult += ", ";
    aResult += OUString::number(rRect.Bottom());

    aResult += "] ]";

    return aResult;
}

SW_DLLPUBLIC const char * dbg_out(const SwRect & rRect)
{
    return dbg_out(lcl_dbg_out(rRect));
}

static OUString lcl_dbg_out(const SwFrmFmt & rFrmFmt)
{
    OUString aResult("[ ");

    char sBuffer[256];
    sprintf(sBuffer, "%p", &rFrmFmt);

    aResult += OUString(sBuffer, strlen(sBuffer), RTL_TEXTENCODING_ASCII_US);
    aResult += "(";
    aResult += rFrmFmt.GetName();
    aResult += ")";

    if (rFrmFmt.IsAuto())
        aResult += "*";

    aResult += " ,";
    aResult += lcl_dbg_out(rFrmFmt.FindLayoutRect());
    aResult += " ]";

    return aResult;
}

SW_DLLPUBLIC const char * dbg_out(const SwFrmFmt & rFrmFmt)
{
    return dbg_out(lcl_dbg_out(rFrmFmt));
}

static const OUString lcl_AnchoredFrames(const SwNode & rNode)
{
    OUString aResult("[");

    const SwDoc * pDoc = rNode.GetDoc();
    if (pDoc)
    {
        const SwFrmFmts * pFrmFmts = pDoc->GetSpzFrmFmts();

        if (pFrmFmts)
        {
            bool bFirst = true;
            for (SwFrmFmts::const_iterator i(pFrmFmts->begin());
                 i != pFrmFmts->end(); ++i)
            {
                const SwFmtAnchor & rAnchor = (*i)->GetAnchor();
                const SwPosition * pPos = rAnchor.GetCntntAnchor();

                if (pPos && &pPos->nNode.GetNode() == &rNode)
                {
                    if (! bFirst)
                        aResult += ", ";

                    if (*i)
                        aResult += lcl_dbg_out(**i);
                    bFirst = false;
                }
            }
        }
    }

    aResult += "]";

    return aResult;
}

static OUString lcl_dbg_out_NumType(sal_Int16 nType)
{
    OUString aTmpStr;

    switch (nType)
    {
    case SVX_NUM_NUMBER_NONE:
        aTmpStr += " NONE";

        break;
    case SVX_NUM_CHARS_UPPER_LETTER:
        aTmpStr += " CHARS_UPPER_LETTER";

        break;
    case SVX_NUM_CHARS_LOWER_LETTER:
        aTmpStr += " CHARS_LOWER_LETTER";

        break;
    case SVX_NUM_ROMAN_UPPER:
        aTmpStr += " ROMAN_UPPER";

        break;
    case SVX_NUM_ROMAN_LOWER:
        aTmpStr += " ROMAN_LOWER";

        break;
    case SVX_NUM_ARABIC:
        aTmpStr += " ARABIC";

        break;
    default:
        aTmpStr += " ??";

        break;
    }

    return aTmpStr;
}

static OUString lcl_dbg_out(const SwNode & rNode)
{
    OUString aTmpStr;

    aTmpStr += "<node ";
    aTmpStr += "index=\"";
    aTmpStr += OUString::number(rNode.GetIndex());
    aTmpStr += "\"";

#ifdef DBG_UTIL
    aTmpStr += " serial=\"";
    aTmpStr += OUString::number(rNode.GetSerial());
    aTmpStr += "\"";
#endif

    aTmpStr += " type=\"";
    aTmpStr += OUString::number(sal_Int32( rNode.GetNodeType() ) );
    aTmpStr += "\"";

    aTmpStr += " pointer=\"";

    char aBuffer[128];
    sprintf(aBuffer, "%p", &rNode);
    aTmpStr += OUString(aBuffer, strlen(aBuffer), RTL_TEXTENCODING_ASCII_US);

    aTmpStr += "\">";

    const SwTxtNode * pTxtNode = rNode.GetTxtNode();

    if (rNode.IsTxtNode())
    {
        const SfxItemSet * pAttrSet = pTxtNode->GetpSwAttrSet();

        aTmpStr += "<txt>";
        aTmpStr += pTxtNode->GetTxt().getLength() > 10 ? pTxtNode->GetTxt().copy(0, 10) : pTxtNode->GetTxt();
        aTmpStr += "</txt>";

        if (rNode.IsTableNode())
            aTmpStr += "<tbl/>";

        aTmpStr += "<outlinelevel>";
        aTmpStr += OUString::number(pTxtNode->GetAttrOutlineLevel()-1);
        aTmpStr += "</outlinelevel>";

        const SwNumRule * pNumRule = pTxtNode->GetNumRule();

        if (pNumRule != NULL)
        {
            aTmpStr += "<number>";
            if ( pTxtNode->GetNum() )
            {
                aTmpStr += lcl_dbg_out(*(pTxtNode->GetNum()));
            }
            aTmpStr += "</number>";

            aTmpStr += "<rule>";
            aTmpStr += pNumRule->GetName();

            const SfxPoolItem * pItem = NULL;

            if (pAttrSet && SFX_ITEM_SET ==
                pAttrSet->GetItemState(RES_PARATR_NUMRULE, false, &pItem))
            {
                aTmpStr += "(";
                aTmpStr +=
                    static_cast<const SwNumRuleItem *>(pItem)->GetValue();
                aTmpStr += ")";
                aTmpStr += "*";
            }

            const SwNumFmt * pNumFmt = NULL;
            aTmpStr += "</rule>";

            if (pTxtNode->GetActualListLevel() > 0)
                pNumFmt = pNumRule->GetNumFmt( static_cast< sal_uInt16 >(pTxtNode->GetActualListLevel()) );

            if (pNumFmt)
            {
                aTmpStr += "<numformat>";
                aTmpStr +=
                    lcl_dbg_out_NumType(pNumFmt->GetNumberingType());
                aTmpStr += "</numformat>";
            }
        }

        if (pTxtNode->IsCountedInList())
            aTmpStr += "<counted/>";

        SwFmtColl * pColl = pTxtNode->GetFmtColl();

        if (pColl)
        {
            aTmpStr += "<coll>";
            aTmpStr += pColl->GetName();

            aTmpStr += "(";
            aTmpStr += OUString::number
                (static_cast<SwTxtFmtColl *>(pColl)->GetAssignedOutlineStyleLevel());

            const SwNumRuleItem & rItem =
                static_cast<const SwNumRuleItem &>
                (pColl->GetFmtAttr(RES_PARATR_NUMRULE));
            const OUString sNumruleName = rItem.GetValue();

            if (!sNumruleName.isEmpty())
            {
                aTmpStr += ", ";
                aTmpStr += sNumruleName;
            }
            aTmpStr += ")";
            aTmpStr += "</coll>";
        }

        SwFmtColl * pCColl = pTxtNode->GetCondFmtColl();

        if (pCColl)
        {
            aTmpStr += "<ccoll>";
            aTmpStr += pCColl->GetName();
            aTmpStr += "</ccoll>";
        }

        aTmpStr += "<frms>";
        aTmpStr += lcl_AnchoredFrames(rNode);
        aTmpStr += "</frms>";

        if (bDbgOutPrintAttrSet)
        {
            aTmpStr += "<attrs>";
            aTmpStr += lcl_dbg_out(pTxtNode->GetSwAttrSet());
            aTmpStr += "</attrs>";
        }
    }
    else if (rNode.IsStartNode())
    {
        aTmpStr += "<start end=\"";

        const SwStartNode * pStartNode = dynamic_cast<const SwStartNode *> (&rNode);
        if (pStartNode != NULL)
            aTmpStr += OUString::number(pStartNode->EndOfSectionNode()->GetIndex());

        aTmpStr += "\"/>";
    }
    else if (rNode.IsEndNode())
        aTmpStr += "<end/>";

    aTmpStr += "</node>";

    return aTmpStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwNode & rNode)
{
    return dbg_out(lcl_dbg_out(rNode));
}

SW_DLLPUBLIC const char * dbg_out(const SwNode * pNode)
{
    if (NULL != pNode)
        return dbg_out(*pNode);
    else
        return NULL;
}

SW_DLLPUBLIC const char * dbg_out(const SwCntntNode * pNode)
{
    if (NULL != pNode)
        return dbg_out(*pNode);
    else
        return NULL;
}

SW_DLLPUBLIC const char * dbg_out(const SwTxtNode * pNode)
{
    if (NULL != pNode)
        return dbg_out(*pNode);
    else
        return NULL;
}

static void lcl_dbg_nodes_inner(OUString & aStr, SwNodes & rNodes, sal_uLong & nIndex)
{
    SwNode * pNode = rNodes[nIndex];
    SwStartNode * pStartNode = dynamic_cast<SwStartNode *> (pNode);

    SwNode * pEndNode = NULL;
    if (pStartNode != NULL)
        pEndNode = pStartNode->EndOfSectionNode();

    sal_uLong nCount = rNodes.Count();
    sal_uLong nStartIndex = nIndex;

    bool bDone = false;

    OUString aTag;
    if (pNode->IsTableNode())
        aTag += "table";
    else if (pNode->IsSectionNode())
        aTag += "section";
    else
        aTag += "nodes";

    aStr += "<";
    aStr += aTag;
    aStr += ">";

    while (! bDone)
    {
        if (pNode->IsStartNode() && nIndex != nStartIndex)
            lcl_dbg_nodes_inner(aStr, rNodes, nIndex);
        else
        {
            aStr += lcl_dbg_out(*pNode);
            aStr += "\n";

            nIndex++;
        }

        if (pNode == pEndNode || nIndex >= nCount)
            bDone = true;
        else
            pNode = rNodes[nIndex];
    }

    aStr += "</";
    aStr += aTag;
    aStr += ">\n";
}

static OUString lcl_dbg_out(SwNodes & rNodes)
{
    OUString aStr("<nodes-array>");

    sal_uLong nIndex = 0;
    sal_uLong nCount = rNodes.Count();

    while (nIndex < nCount)
    {
        lcl_dbg_nodes_inner(aStr, rNodes, nIndex);
    }

    aStr += "</nodes-array>\n";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(SwNodes & rNodes)
{
    return dbg_out(lcl_dbg_out(rNodes));
}

static OUString lcl_dbg_out(const SwUndo & rUndo)
{
    OUString aStr("[ ");

    aStr += OUString::number(
                static_cast<SfxUndoAction const&>(rUndo).GetId());
    aStr += ": ";

    aStr += rUndo.GetComment();
    aStr += " ]";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwUndo & rUndo)
{
    return dbg_out(lcl_dbg_out(rUndo));
}

static OUString lcl_dbg_out(SwOutlineNodes & rNodes)
{
    OUString aStr("[\n");

    for (sal_uInt16 i = 0; i < rNodes.size(); i++)
    {
        aStr += lcl_dbg_out(*rNodes[i]);
        aStr += "\n";
    }

    aStr += "]\n";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(SwOutlineNodes & rNodes)
{
    return dbg_out(lcl_dbg_out(rNodes));
}

//FIXME: this method seems to do nothing at all
static OUString lcl_dbg_out(const SwRewriter & rRewriter)
{
    (void) rRewriter;
    OUString aResult;

    //aResult = rRewriter.ToString();

    return aResult;
}

SW_DLLPUBLIC const char * dbg_out(const SwRewriter & rRewriter)
{
    return dbg_out(lcl_dbg_out(rRewriter));
}

static OUString lcl_dbg_out(const SvxNumberFormat & rFmt)
{
    OUString aResult;

    aResult = lcl_dbg_out_NumType(rFmt.GetNumberingType());

    return aResult;
}

static OUString lcl_dbg_out(const SwNumRule & rRule)
{
    OUString aResult("[ ");

    aResult += rRule.GetName();
    aResult += " [";

    for (sal_uInt8 n = 0; n < MAXLEVEL; n++)
    {
        if (n > 0)
            aResult += ", ";

        aResult += lcl_dbg_out(rRule.Get(n));
    }

    aResult += "]";

    aResult += "]";

    return aResult;
}

SW_DLLPUBLIC const char * dbg_out(const SwNumRule & rRule)
{
    return dbg_out(lcl_dbg_out(rRule));
}

static OUString lcl_dbg_out(const SwTxtFmtColl & rFmt)
{
    OUString aResult(rFmt.GetName());

    aResult += "(";
    aResult += OUString::number(rFmt.GetAttrOutlineLevel());
    aResult += ")";

    return aResult;
}

SW_DLLPUBLIC const char * dbg_out(const SwTxtFmtColl & rFmt)
{
    return dbg_out(lcl_dbg_out(rFmt));
}

static OUString lcl_dbg_out(const SwFrmFmts & rFrmFmts)
{
    return lcl_dbg_out_SvPtrArr<SwFrmFmts>(rFrmFmts);
}

SW_DLLPUBLIC const char * dbg_out(const SwFrmFmts & rFrmFmts)
{
    return dbg_out(lcl_dbg_out(rFrmFmts));
}

static OUString lcl_dbg_out(const SwNumRuleTbl & rTbl)
{
    OUString aResult("[");

    for (size_t n = 0; n < rTbl.size(); n++)
    {
        if (n > 0)
            aResult += ", ";

        aResult += rTbl[n]->GetName();

        char sBuffer[256];
        sprintf(sBuffer, "(%p)", rTbl[n]);
        aResult += OUString(sBuffer, strlen(sBuffer), RTL_TEXTENCODING_ASCII_US);
    }

    aResult += "]";

    return aResult;
}

SW_DLLPUBLIC const char * dbg_out(const SwNumRuleTbl & rTbl)
{
    return dbg_out(lcl_dbg_out(rTbl));
}

static OUString lcl_TokenType2Str(FormTokenType nType)
{
    switch(nType)
    {
    case TOKEN_ENTRY_NO:
        return OUString("NO");
    case TOKEN_ENTRY_TEXT:
        return OUString("ENTRY_TEXT");
    case TOKEN_ENTRY:
        return OUString("ENTRY");
    case TOKEN_TAB_STOP:
        return OUString("TAB_STOP");
    case TOKEN_TEXT:
        return OUString("TOKEN_TEXT");
    case TOKEN_PAGE_NUMS:
        return OUString("NUMS");
    case TOKEN_CHAPTER_INFO:
        return OUString("CHAPTER_INFO");
    case TOKEN_LINK_START:
        return OUString("LINK_START");
    case TOKEN_LINK_END:
        return OUString("LINK_END");
    case TOKEN_AUTHORITY:
        return OUString("AUTHORITY");
    case TOKEN_END:
        return OUString("END");
    default:
        OSL_FAIL("should not be reached");
        return OUString("??");
    }
}

static OUString lcl_dbg_out(const SwFormToken & rToken)
{
    return rToken.GetString();
}

SW_DLLPUBLIC const char * dbg_out(const SwFormToken & rToken)
{
    return dbg_out(lcl_dbg_out(rToken));
}

static OUString lcl_dbg_out(const SwFormTokens & rTokens)
{
    OUString aStr("[");

    SwFormTokens::const_iterator aIt;

    for (aIt = rTokens.begin(); aIt != rTokens.end(); ++aIt)
    {
        if (aIt != rTokens.begin())
            aStr += ", ";

        aStr += lcl_TokenType2Str(aIt->eTokenType);
        aStr += ": ";
        aStr += lcl_dbg_out(*aIt);
    }

    aStr += "]";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwFormTokens & rTokens)
{
    return dbg_out(lcl_dbg_out(rTokens));
}

static OUString lcl_dbg_out(const SwNodeRange & rRange)
{
    OUString aStr("[");

    aStr += lcl_dbg_out(SwPosition(rRange.aStart));
    aStr += ", ";
    aStr += lcl_dbg_out(SwPosition(rRange.aEnd));

    aStr += "]";

    return aStr;
}

SW_DLLPUBLIC const char * dbg_out(const SwNodeRange & rRange)
{
    return dbg_out(lcl_dbg_out(rRange));
}

#endif // DEBUG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

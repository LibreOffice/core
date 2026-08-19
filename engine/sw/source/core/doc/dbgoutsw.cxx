/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemiter.hxx>
#include <map>
#include <node.hxx>
#include <ndtxt.hxx>
#include <ndhints.hxx>
#include <txatbase.hxx>
#include <pam.hxx>
#include <docary.hxx>
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
#include <frameformats.hxx>
#include <cstdio>

static OString aDbgOutResult;
bool bDbgOutStdErr = false;
bool bDbgOutPrintAttrSet = false;

template<class T>
static OUString lcl_dbg_out_SvPtrArr(const T & rArr)
{
    OUStringBuffer aStr("[ ");

    for (typename T::const_iterator i(rArr.begin()); i != rArr.end(); ++i)
    {
        if (i != rArr.begin())
            aStr.append(", ");

        if (*i)
            aStr.append(lcl_dbg_out(**i));
        else
            aStr.append("(null)");
    }

    aStr.append(" ]");

    return aStr.makeStringAndClear();
}

const char * dbg_out(const void * pVoid)
{
    return dbg_out(OUString::number(reinterpret_cast<sal_uIntPtr>(pVoid), 16));
}

const char * dbg_out(std::u16string_view aStr)
{
    aDbgOutResult = OUStringToOString(aStr, RTL_TEXTENCODING_ASCII_US);

    if (bDbgOutStdErr)
        fprintf(stderr, "%s", aDbgOutResult.getStr());

    return aDbgOutResult.getStr();
}

static std::map<sal_uInt16,OUString> & GetItemWhichMap()
{
    static std::map<sal_uInt16,OUString> aItemWhichMap
    {
        { RES_CHRATR_CASEMAP , u"CHRATR_CASEMAP"_ustr },
        { RES_CHRATR_CHARSETCOLOR , u"CHRATR_CHARSETCOLOR"_ustr },
        { RES_CHRATR_COLOR , u"CHRATR_COLOR"_ustr },
        { RES_CHRATR_CONTOUR , u"CHRATR_CONTOUR"_ustr },
        { RES_CHRATR_CROSSEDOUT , u"CHRATR_CROSSEDOUT"_ustr },
        { RES_CHRATR_ESCAPEMENT , u"CHRATR_ESCAPEMENT"_ustr },
        { RES_CHRATR_FONT , u"CHRATR_FONT"_ustr },
        { RES_CHRATR_FONTSIZE , u"CHRATR_FONTSIZE"_ustr },
        { RES_CHRATR_KERNING , u"CHRATR_KERNING"_ustr },
        { RES_CHRATR_LANGUAGE , u"CHRATR_LANGUAGE"_ustr },
        { RES_CHRATR_POSTURE , u"CHRATR_POSTURE"_ustr },
        { RES_CHRATR_SHADOWED , u"CHRATR_SHADOWED"_ustr },
        { RES_CHRATR_UNDERLINE , u"CHRATR_UNDERLINE"_ustr },
        { RES_CHRATR_OVERLINE , u"CHRATR_OVERLINE"_ustr },
        { RES_CHRATR_WEIGHT , u"CHRATR_WEIGHT"_ustr },
        { RES_CHRATR_WORDLINEMODE , u"CHRATR_WORDLINEMODE"_ustr },
        { RES_CHRATR_AUTOKERN , u"CHRATR_AUTOKERN"_ustr },
        { RES_CHRATR_BLINK , u"CHRATR_BLINK"_ustr },
        { RES_CHRATR_NOHYPHEN , u"CHRATR_NOHYPHEN"_ustr },
        { RES_CHRATR_BACKGROUND , u"CHRATR_BACKGROUND"_ustr },
        { RES_CHRATR_HIGHLIGHT , u"CHRATR_HIGHLIGHT"_ustr },
        { RES_CHRATR_CJK_FONT , u"CHRATR_CJK_FONT"_ustr },
        { RES_CHRATR_CJK_FONTSIZE , u"CHRATR_CJK_FONTSIZE"_ustr },
        { RES_CHRATR_CJK_LANGUAGE , u"CHRATR_CJK_LANGUAGE"_ustr },
        { RES_CHRATR_CJK_POSTURE , u"CHRATR_CJK_POSTURE"_ustr },
        { RES_CHRATR_CJK_WEIGHT , u"CHRATR_CJK_WEIGHT"_ustr },
        { RES_CHRATR_CTL_FONT , u"CHRATR_CTL_FONT"_ustr },
        { RES_CHRATR_CTL_FONTSIZE , u"CHRATR_CTL_FONTSIZE"_ustr },
        { RES_CHRATR_CTL_LANGUAGE , u"CHRATR_CTL_LANGUAGE"_ustr },
        { RES_CHRATR_CTL_POSTURE , u"CHRATR_CTL_POSTURE"_ustr },
        { RES_CHRATR_CTL_WEIGHT , u"CHRATR_CTL_WEIGHT"_ustr },
        { RES_CHRATR_ROTATE , u"CHRATR_ROTATE"_ustr },
        { RES_CHRATR_EMPHASIS_MARK , u"CHRATR_EMPHASIS_MARK"_ustr },
        { RES_CHRATR_TWO_LINES , u"CHRATR_TWO_LINES"_ustr },
        { RES_CHRATR_SCALEW , u"CHRATR_SCALEW"_ustr },
        { RES_CHRATR_RELIEF , u"CHRATR_RELIEF"_ustr },
        { RES_CHRATR_HIDDEN , u"CHRATR_HIDDEN"_ustr },
        { RES_CHRATR_BOX , u"CHRATR_BOX"_ustr },
        { RES_CHRATR_SHADOW , u"CHRATR_SHADOW"_ustr },
        { RES_CHRATR_OPTICAL_SIZING , u"CHRATR_OPTICAL_SIZING"_ustr },
        { RES_TXTATR_AUTOFMT , u"TXTATR_AUTOFMT"_ustr },
        { RES_TXTATR_INETFMT , u"TXTATR_INETFMT"_ustr },
        { RES_TXTATR_REFMARK , u"TXTATR_REFMARK"_ustr },
        { RES_TXTATR_TOXMARK , u"TXTATR_TOXMARK"_ustr },
        { RES_TXTATR_CHARFMT , u"TXTATR_CHARFMT"_ustr },
        { RES_TXTATR_INPUTFIELD , u"RES_TXTATR_INPUTFIELD"_ustr },
        { RES_TXTATR_CONTENTCONTROL , u"RES_TXTATR_CONTENTCONTROL"_ustr },
        { RES_TXTATR_CJK_RUBY , u"TXTATR_CJK_RUBY"_ustr },
        { RES_TXTATR_UNKNOWN_CONTAINER , u"TXTATR_UNKNOWN_CONTAINER"_ustr },
        { RES_TXTATR_META , u"TXTATR_META"_ustr },
        { RES_TXTATR_METAFIELD , u"TXTATR_METAFIELD"_ustr },
        { RES_TXTATR_FIELD , u"TXTATR_FIELD"_ustr },
        { RES_TXTATR_FLYCNT , u"TXTATR_FLYCNT"_ustr },
        { RES_TXTATR_FTN , u"TXTATR_FTN"_ustr },
        { RES_TXTATR_ANNOTATION , u"TXTATR_ANNOTATION"_ustr },
        { RES_TXTATR_LINEBREAK , u"RES_TXTATR_LINEBREAK"_ustr },
        { RES_TXTATR_DUMMY1 , u"TXTATR_DUMMY1"_ustr },
        { RES_PARATR_LINESPACING , u"PARATR_LINESPACING"_ustr },
        { RES_PARATR_ADJUST , u"PARATR_ADJUST"_ustr },
        { RES_PARATR_SPLIT , u"PARATR_SPLIT"_ustr },
        { RES_PARATR_ORPHANS , u"PARATR_ORPHANS"_ustr },
        { RES_PARATR_WIDOWS , u"PARATR_WIDOWS"_ustr },
        { RES_PARATR_TABSTOP , u"PARATR_TABSTOP"_ustr },
        { RES_PARATR_HYPHENZONE , u"PARATR_HYPHENZONE"_ustr },
        { RES_PARATR_DROP , u"PARATR_DROP"_ustr },
        { RES_PARATR_REGISTER , u"PARATR_REGISTER"_ustr },
        { RES_PARATR_NUMRULE , u"PARATR_NUMRULE"_ustr },
        { RES_PARATR_SCRIPTSPACE , u"PARATR_SCRIPTSPACE"_ustr },
        { RES_PARATR_HANGINGPUNCTUATION , u"PARATR_HANGINGPUNCTUATION"_ustr },
        { RES_PARATR_FORBIDDEN_RULES , u"PARATR_FORBIDDEN_RULES"_ustr },
        { RES_PARATR_VERTALIGN , u"PARATR_VERTALIGN"_ustr },
        { RES_PARATR_SNAPTOGRID , u"PARATR_SNAPTOGRID"_ustr },
        { RES_PARATR_CONNECT_BORDER , u"PARATR_CONNECT_BORDER"_ustr },
        { RES_PARATR_AUTOFRAMEDIR, u"PARATR_AUTOFRAMEDIR"_ustr },
        { RES_FILL_ORDER , u"FILL_ORDER"_ustr },
        { RES_FRM_SIZE , u"FRM_SIZE"_ustr },
        { RES_PAPER_BIN , u"PAPER_BIN"_ustr },
        { RES_MARGIN_FIRSTLINE, u"FIRSTLINE"_ustr },
        { RES_MARGIN_TEXTLEFT, u"TEXTLEFT"_ustr },
        { RES_MARGIN_RIGHT, u"RIGHT"_ustr },
        { RES_MARGIN_LEFT, u"LEFT"_ustr },
        { RES_MARGIN_GUTTER, u"GUTTER"_ustr },
        { RES_MARGIN_GUTTER_RIGHT, u"GUTTER_RIGHT"_ustr },
        { RES_LR_SPACE , u"LR_SPACE"_ustr },
        { RES_UL_SPACE , u"UL_SPACE"_ustr },
        { RES_PAGEDESC , u"PAGEDESC"_ustr },
        { RES_BREAK , u"BREAK"_ustr },
        { RES_CNTNT , u"CNTNT"_ustr },
        { RES_HEADER , u"HEADER"_ustr },
        { RES_FOOTER , u"FOOTER"_ustr },
        { RES_PRINT , u"PRINT"_ustr },
        { RES_OPAQUE , u"OPAQUE"_ustr },
        { RES_PROTECT , u"PROTECT"_ustr },
        { RES_SURROUND , u"SURROUND"_ustr },
        { RES_VERT_ORIENT , u"VERT_ORIENT"_ustr },
        { RES_HORI_ORIENT , u"HORI_ORIENT"_ustr },
        { RES_ANCHOR , u"ANCHOR"_ustr },
        { RES_BACKGROUND , u"BACKGROUND"_ustr },
        { RES_BOX , u"BOX"_ustr },
        { RES_SHADOW , u"SHADOW"_ustr },
        { RES_FRMMACRO , u"FRMMACRO"_ustr },
        { RES_COL , u"COL"_ustr },
        { RES_KEEP , u"KEEP"_ustr },
        { RES_URL , u"URL"_ustr },
        { RES_EDIT_IN_READONLY , u"EDIT_IN_READONLY"_ustr },
        { RES_LAYOUT_SPLIT , u"LAYOUT_SPLIT"_ustr },
        { RES_CHAIN , u"CHAIN"_ustr },
        { RES_TEXTGRID , u"TEXTGRID"_ustr },
        { RES_LINENUMBER   , u"LINENUMBER"_ustr },
        { RES_FTN_AT_TXTEND , u"FTN_AT_TXTEND"_ustr },
        { RES_END_AT_TXTEND , u"END_AT_TXTEND"_ustr },
        { RES_COLUMNBALANCE , u"COLUMNBALANCE"_ustr },
        { RES_FRAMEDIR , u"FRAMEDIR"_ustr },
        { RES_HEADER_FOOTER_EAT_SPACING , u"HEADER_FOOTER_EAT_SPACING"_ustr },
        { RES_ROW_SPLIT , u"ROW_SPLIT"_ustr },
        { RES_GRFATR_MIRRORGRF , u"GRFATR_MIRRORGRF"_ustr },
        { RES_GRFATR_CROPGRF , u"GRFATR_CROPGRF"_ustr },
        { RES_GRFATR_ROTATION , u"GRFATR_ROTATION"_ustr },
        { RES_GRFATR_LUMINANCE , u"GRFATR_LUMINANCE"_ustr },
        { RES_GRFATR_CONTRAST , u"GRFATR_CONTRAST"_ustr },
        { RES_GRFATR_CHANNELR , u"GRFATR_CHANNELR"_ustr },
        { RES_GRFATR_CHANNELG , u"GRFATR_CHANNELG"_ustr },
        { RES_GRFATR_CHANNELB , u"GRFATR_CHANNELB"_ustr },
        { RES_GRFATR_GAMMA , u"GRFATR_GAMMA"_ustr },
        { RES_GRFATR_INVERT , u"GRFATR_INVERT"_ustr },
        { RES_GRFATR_TRANSPARENCY , u"GRFATR_TRANSPARENCY"_ustr },
        { RES_GRFATR_DRAWMODE , u"GRFATR_DRAWMODE"_ustr },
        { RES_BOXATR_FORMAT , u"BOXATR_FORMAT"_ustr },
        { RES_BOXATR_FORMULA , u"BOXATR_FORMULA"_ustr },
        { RES_BOXATR_VALUE , u"BOXATR_VALUE"_ustr },
    };

    return aItemWhichMap;
}

static OUString lcl_dbg_out(const SfxPoolItem & rItem)
{
    OUString aStr(u"[ "_ustr);

    auto & rWhichMap = GetItemWhichMap();
    auto it = rWhichMap.find(rItem.Which());
    if ( it != rWhichMap.end())
        aStr += it->second;
    else
        aStr += OUString::number(rItem.Which());

    aStr += " ]";

    return aStr;
}

const char * dbg_out(const SfxPoolItem & rItem)
{
    return dbg_out(lcl_dbg_out(rItem));
}

const char * dbg_out(const SfxPoolItem * pItem)
{
    return dbg_out(pItem ? lcl_dbg_out(*pItem) : u"(nil)"_ustr);
}

static OUString lcl_dbg_out(const SfxItemSet & rSet)
{
    bool bFirst = true;
    OUStringBuffer aStr = "[ ";

    for (SfxItemIter aIter( rSet ); !aIter.IsAtEnd(); aIter.Next())
    {
        const SfxPoolItem* pItem = aIter.GetCurItem();
        if (!bFirst)
            aStr.append(", ");

        if (reinterpret_cast<sal_uIntPtr>(pItem) != SAL_MAX_SIZE)
            aStr.append(lcl_dbg_out(*pItem));
        else
            aStr.append("invalid");

        bFirst = false;
    }

    aStr.append(" ]");

    return aStr.makeStringAndClear();
}

const char * dbg_out(const SfxItemSet & rSet)
{
    return dbg_out(lcl_dbg_out(rSet));
}

static OUString lcl_dbg_out(const SwTextAttr & rAttr)
{
    OUString aStr =
        "[ " +
        OUString::number(rAttr.GetStart()) +
        "->" +
        OUString::number(*rAttr.End()) +
        " " +
        lcl_dbg_out(rAttr.GetAttr()) +
        " ]";

    return aStr;
}

const char * dbg_out(const SwTextAttr & rAttr)
{
    return dbg_out(lcl_dbg_out(rAttr));
}

static OUString lcl_dbg_out(const SwpHints & rHints)
{
    OUStringBuffer aStr("[ SwpHints\n");

    for (size_t i = 0; i < rHints.Count(); ++i)
    {
        aStr.append("  " + lcl_dbg_out(*rHints.Get(i)) + "\n");
    }

    aStr.append("]\n");

    return aStr.makeStringAndClear();
}

const char * dbg_out(const SwpHints &rHints)
{
    return dbg_out(lcl_dbg_out(rHints));
}

static OUString lcl_dbg_out(const SwPosition & rPos)
{
    OUString aStr =
        "( " +
        OUString::number(sal_Int32(rPos.GetNodeIndex())) +
        ", " +
        OUString::number(rPos.GetContentIndex()) +
        ": " +
        OUString::number(reinterpret_cast<sal_IntPtr>(rPos.GetContentNode()), 16) +
        " )";

    return aStr;
}

const char * dbg_out(const SwPosition & rPos)
{
    return dbg_out(lcl_dbg_out(rPos));
}

static OUString lcl_dbg_out(const SwPaM & rPam)
{
    OUString aStr =
        "[ Pt: " +
        lcl_dbg_out(*rPam.GetPoint());

    if (rPam.HasMark())
    {
        aStr += ", Mk: " + lcl_dbg_out(*rPam.GetMark());
    }

    aStr += " ]";

    return aStr;
}

const char * dbg_out(const SwPaM & rPam)
{
    return dbg_out(lcl_dbg_out(rPam));
}

static const OUString & lcl_dbg_out(const SwNodeNum & )
{
    return EMPTY_OUSTRING;/*rNum.ToString();*/
}

const char * dbg_out(const SwNodeNum & rNum)
{
    return dbg_out(lcl_dbg_out(rNum));
}

static OUString lcl_dbg_out(const SwRect & rRect)
{
    OUString aResult =
        "[ [" +
        OUString::number(rRect.Left()) +
        ", " +
        OUString::number(rRect.Top()) +
        "], [" +
        OUString::number(rRect.Right()) +
        ", " +
        OUString::number(rRect.Bottom()) +
        "] ]";

    return aResult;
}

const char * dbg_out(const SwRect & rRect)
{
    return dbg_out(lcl_dbg_out(rRect));
}

static OUString lcl_dbg_out(const SwFrameFormat & rFrameFormat)
{
    OUString aResult = "[ " +
        OUString::number(reinterpret_cast<sal_uIntPtr>(&rFrameFormat), 16) +
        "(" +
        rFrameFormat.GetName().toString() + ")";

    if (rFrameFormat.IsAuto())
        aResult += "*";

    aResult += " ," + lcl_dbg_out(rFrameFormat.FindLayoutRect()) + " ]";

    return aResult;
}

const char * dbg_out(const SwFrameFormat & rFrameFormat)
{
    return dbg_out(lcl_dbg_out(rFrameFormat));
}

static OUString lcl_AnchoredFrames(const SwNode & rNode)
{
    OUStringBuffer aResult("[");

    const SwDoc& rDoc = rNode.GetDoc();
    const sw::SpzFrameFormats* pSpzs = rDoc.GetSpzFrameFormats();

    if (pSpzs)
    {
        bool bFirst = true;
        for(const sw::SpzFrameFormat* pSpz: *pSpzs)
        {
            const SwFormatAnchor& rAnchor = pSpz->GetAnchor();
            const SwNode * pPos = rAnchor.GetAnchorNode();

            if (pPos && *pPos == rNode)
            {
                if (! bFirst)
                    aResult.append(", ");

                if (pSpz)
                    aResult.append(lcl_dbg_out(*pSpz));
                bFirst = false;
            }
        }
    }

    aResult.append("]");

    return aResult.makeStringAndClear();
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
    OUString aTmpStr = "<node "
        "index=\"" +
        OUString::number(sal_Int32(rNode.GetIndex())) +
        "\""
        " serial=\"" +
        OUString::number(rNode.GetSerial()) +
        "\""
        " type=\"" +
        OUString::number(sal_Int32( rNode.GetNodeType() ) ) +
        "\""
        " pointer=\"" +
        OUString::number(reinterpret_cast<sal_uIntPtr>(&rNode), 16) +
        "\">";

    const SwTextNode * pTextNode = rNode.GetTextNode();

    if (rNode.IsTextNode())
    {
        const SfxItemSet * pAttrSet = pTextNode->GetpSwAttrSet();

        aTmpStr += "<txt>" + (pTextNode->GetText().getLength() > 10 ? pTextNode->GetText().copy(0, 10) : pTextNode->GetText()) + "</txt>";

        if (rNode.IsTableNode())
            aTmpStr += "<tbl/>";

        aTmpStr += "<outlinelevel>" + OUString::number(pTextNode->GetAttrOutlineLevel()-1) + "</outlinelevel>";

        const SwNumRule * pNumRule = pTextNode->GetNumRule();

        if (pNumRule != nullptr)
        {
            aTmpStr += "<number>";
            if ( pTextNode->GetNum() )
            {
                aTmpStr += lcl_dbg_out(*(pTextNode->GetNum()));
            }
            aTmpStr += "</number><rule>" +
                pNumRule->GetName().toString();

            const SwNumRuleItem * pItem = nullptr;

            if (pAttrSet &&
                (pItem = pAttrSet->GetItemIfSet(RES_PARATR_NUMRULE, false)))
            {
                aTmpStr += "(" + pItem->GetValue().toString() + ")*";
            }

            const SwNumFormat * pNumFormat = nullptr;
            aTmpStr += "</rule>";

            if (pTextNode->GetActualListLevel() > 0)
                pNumFormat = pNumRule->GetNumFormat( static_cast< sal_uInt16 >(pTextNode->GetActualListLevel()) );

            if (pNumFormat)
            {
                aTmpStr += "<numformat>" +
                    lcl_dbg_out_NumType(pNumFormat->GetNumberingType()) + "</numformat>";
            }
        }

        if (pTextNode->IsCountedInList())
            aTmpStr += "<counted/>";

        SwFormatColl * pColl = pTextNode->GetFormatColl();

        if (pColl)
        {
            aTmpStr += "<coll>" + pColl->GetName().toString() + "(";

            SwTextFormatColl *pTextColl = static_cast<SwTextFormatColl*>(pColl);
            if (pTextColl->IsAssignedToListLevelOfOutlineStyle())
            {
                aTmpStr += OUString::number(pTextColl->GetAssignedOutlineStyleLevel());
            }
            else
            {
                aTmpStr += OUString::number(-1);
            }

            const SwNumRuleItem & rItem =
                pColl->GetFormatAttr(RES_PARATR_NUMRULE);
            const UIName sNumruleName = rItem.GetValue();

            if (!sNumruleName.isEmpty())
            {
                aTmpStr += ", " + sNumruleName.toString();
            }
            aTmpStr += ")"
                "</coll>";
        }

        SwFormatColl * pCColl = pTextNode->GetCondFormatColl();

        if (pCColl)
        {
            aTmpStr += "<ccoll>" + pCColl->GetName().toString() + "</ccoll>";
        }

        aTmpStr += "<frms>" + lcl_AnchoredFrames(rNode) + "</frms>";

        if (bDbgOutPrintAttrSet)
        {
            aTmpStr += "<attrs>" + lcl_dbg_out(pTextNode->GetSwAttrSet()) + "</attrs>";
        }
    }
    else if (rNode.IsStartNode())
    {
        aTmpStr += "<start end=\"";

        const SwStartNode * pStartNode = rNode.GetStartNode();
        if (pStartNode != nullptr)
            aTmpStr += OUString::number(sal_Int32(pStartNode->EndOfSectionNode()->GetIndex()));

        aTmpStr += "\"/>";
    }
    else if (rNode.IsEndNode())
        aTmpStr += "<end/>";

    aTmpStr += "</node>";

    return aTmpStr;
}

const char * dbg_out(const SwNode & rNode)
{
    return dbg_out(lcl_dbg_out(rNode));
}

const char * dbg_out(const SwNode * pNode)
{
    if (nullptr != pNode)
        return dbg_out(*pNode);
    else
        return nullptr;
}

const char * dbg_out(const SwContentNode * pNode)
{
    if (nullptr != pNode)
        return dbg_out(*pNode);
    else
        return nullptr;
}

const char * dbg_out(const SwTextNode * pNode)
{
    if (nullptr != pNode)
        return dbg_out(*pNode);
    else
        return nullptr;
}

static OUString lcl_dbg_out(const SwUndo & rUndo)
{
    return "[ " + OUString::number(static_cast<int>(rUndo.GetId()))
                + ": " + rUndo.GetComment() + " ]";
}

const char * dbg_out(const SwUndo & rUndo)
{
    return dbg_out(lcl_dbg_out(rUndo));
}

static OUString lcl_dbg_out(SwOutlineNodes const & rNodes)
{
    OUStringBuffer aStr("[\n");

    for (size_t i = 0; i < rNodes.size(); i++)
    {
        aStr.append(lcl_dbg_out(*rNodes[i]) + "\n");
    }

    aStr.append("]\n");

    return aStr.makeStringAndClear();
}

const char * dbg_out( SwOutlineNodes const & rNodes)
{
    return dbg_out(lcl_dbg_out(rNodes));
}

static OUString lcl_dbg_out(const SvxNumberFormat & rFormat)
{
    OUString aResult = lcl_dbg_out_NumType(rFormat.GetNumberingType());
    return aResult;
}

static OUString lcl_dbg_out(const SwNumRule & rRule)
{
    OUStringBuffer aResult("[ " + rRule.GetName().toString() + " [");

    for (sal_uInt8 n = 0; n < MAXLEVEL; n++)
    {
        if (n > 0)
            aResult.append(", ");

        aResult.append(lcl_dbg_out(rRule.Get(n)));
    }

    aResult.append("]]");

    return aResult.makeStringAndClear();
}

const char * dbg_out(const SwNumRule & rRule)
{
    return dbg_out(lcl_dbg_out(rRule));
}

static OUString lcl_dbg_out(const SwTextFormatColl & rFormat)
{
    return rFormat.GetName().toString() + "(" +
        OUString::number(rFormat.GetAttrOutlineLevel()) + ")";
}

const char * dbg_out(const SwTextFormatColl & rFormat)
{
    return dbg_out(lcl_dbg_out(rFormat));
}

static OUString lcl_dbg_out(const sw::FrameFormats<sw::SpzFrameFormat*>& rFrameFormats)
{
    return lcl_dbg_out_SvPtrArr<sw::FrameFormats<sw::SpzFrameFormat*>>(rFrameFormats);
}

const char * dbg_out(const sw::FrameFormats<sw::SpzFrameFormat*>& rFrameFormats)
{
    return dbg_out(lcl_dbg_out(rFrameFormats));
}

static OUString lcl_dbg_out(const SwNumRuleTable & rTable)
{
    OUStringBuffer aResult("[");

    for (size_t n = 0; n < rTable.size(); n++)
    {
        if (n > 0)
            aResult.append(", ");

        aResult.append(rTable[n]->GetName().toString());

        aResult.append("(" + OUString::number(reinterpret_cast<sal_uIntPtr>(rTable[n]), 16) + ")");
    }

    aResult.append("]");

    return aResult.makeStringAndClear();
}

const char * dbg_out(const SwNumRuleTable & rTable)
{
    return dbg_out(lcl_dbg_out(rTable));
}

static OUString lcl_TokenType2Str(FormTokenType nType)
{
    switch(nType)
    {
    case TOKEN_ENTRY_NO:
        return u"NO"_ustr;
    case TOKEN_ENTRY_TEXT:
        return u"ENTRY_TEXT"_ustr;
    case TOKEN_ENTRY:
        return u"ENTRY"_ustr;
    case TOKEN_TAB_STOP:
        return u"TAB_STOP"_ustr;
    case TOKEN_TEXT:
        return u"TOKEN_TEXT"_ustr;
    case TOKEN_PAGE_NUMS:
        return u"NUMS"_ustr;
    case TOKEN_CHAPTER_INFO:
        return u"CHAPTER_INFO"_ustr;
    case TOKEN_LINK_START:
        return u"LINK_START"_ustr;
    case TOKEN_LINK_END:
        return u"LINK_END"_ustr;
    case TOKEN_AUTHORITY:
        return u"AUTHORITY"_ustr;
    case TOKEN_END:
        return u"END"_ustr;
    default:
        OSL_FAIL("should not be reached");
        return u"??"_ustr;
    }
}

static OUString lcl_dbg_out(const SwFormToken & rToken)
{
    return rToken.GetString();
}

const char * dbg_out(const SwFormToken & rToken)
{
    return dbg_out(lcl_dbg_out(rToken));
}

static OUString lcl_dbg_out(const SwFormTokens & rTokens)
{
    OUStringBuffer aStr("[");

    SwFormTokens::const_iterator aIt;

    for (aIt = rTokens.begin(); aIt != rTokens.end(); ++aIt)
    {
        if (aIt != rTokens.begin())
            aStr.append(", ");

        aStr.append(lcl_TokenType2Str(aIt->eTokenType) + ": " + lcl_dbg_out(*aIt));
    }

    aStr.append("]");

    return aStr.makeStringAndClear();
}

const char * dbg_out(const SwFormTokens & rTokens)
{
    return dbg_out(lcl_dbg_out(rTokens));
}

static OUString lcl_dbg_out(const SwNodeRange & rRange)
{
    OUString aStr =
        "[" +
        lcl_dbg_out(SwPosition(rRange.aStart)) +
        ", " +
        lcl_dbg_out(SwPosition(rRange.aEnd)) +
        "]";

    return aStr;
}

const char * dbg_out(const SwNodeRange & rRange)
{
    return dbg_out(lcl_dbg_out(rRange));
}

#endif // DEBUG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

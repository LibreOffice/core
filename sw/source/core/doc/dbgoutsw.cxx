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
        { RES_CHRATR_CASEMAP , "CHRATR_CASEMAP" },
        { RES_CHRATR_CHARSETCOLOR , "CHRATR_CHARSETCOLOR" },
        { RES_CHRATR_COLOR , "CHRATR_COLOR" },
        { RES_CHRATR_CONTOUR , "CHRATR_CONTOUR" },
        { RES_CHRATR_CROSSEDOUT , "CHRATR_CROSSEDOUT" },
        { RES_CHRATR_ESCAPEMENT , "CHRATR_ESCAPEMENT" },
        { RES_CHRATR_FONT , "CHRATR_FONT" },
        { RES_CHRATR_FONTSIZE , "CHRATR_FONTSIZE" },
        { RES_CHRATR_KERNING , "CHRATR_KERNING" },
        { RES_CHRATR_LANGUAGE , "CHRATR_LANGUAGE" },
        { RES_CHRATR_POSTURE , "CHRATR_POSTURE" },
        { RES_CHRATR_SHADOWED , "CHRATR_SHADOWED" },
        { RES_CHRATR_UNDERLINE , "CHRATR_UNDERLINE" },
        { RES_CHRATR_OVERLINE , "CHRATR_OVERLINE" },
        { RES_CHRATR_WEIGHT , "CHRATR_WEIGHT" },
        { RES_CHRATR_WORDLINEMODE , "CHRATR_WORDLINEMODE" },
        { RES_CHRATR_AUTOKERN , "CHRATR_AUTOKERN" },
        { RES_CHRATR_BLINK , "CHRATR_BLINK" },
        { RES_CHRATR_NOHYPHEN , "CHRATR_NOHYPHEN" },
        { RES_CHRATR_BACKGROUND , "CHRATR_BACKGROUND" },
        { RES_CHRATR_HIGHLIGHT , "CHRATR_HIGHLIGHT" },
        { RES_CHRATR_CJK_FONT , "CHRATR_CJK_FONT" },
        { RES_CHRATR_CJK_FONTSIZE , "CHRATR_CJK_FONTSIZE" },
        { RES_CHRATR_CJK_LANGUAGE , "CHRATR_CJK_LANGUAGE" },
        { RES_CHRATR_CJK_POSTURE , "CHRATR_CJK_POSTURE" },
        { RES_CHRATR_CJK_WEIGHT , "CHRATR_CJK_WEIGHT" },
        { RES_CHRATR_CTL_FONT , "CHRATR_CTL_FONT" },
        { RES_CHRATR_CTL_FONTSIZE , "CHRATR_CTL_FONTSIZE" },
        { RES_CHRATR_CTL_LANGUAGE , "CHRATR_CTL_LANGUAGE" },
        { RES_CHRATR_CTL_POSTURE , "CHRATR_CTL_POSTURE" },
        { RES_CHRATR_CTL_WEIGHT , "CHRATR_CTL_WEIGHT" },
        { RES_CHRATR_ROTATE , "CHRATR_ROTATE" },
        { RES_CHRATR_EMPHASIS_MARK , "CHRATR_EMPHASIS_MARK" },
        { RES_CHRATR_TWO_LINES , "CHRATR_TWO_LINES" },
        { RES_CHRATR_SCALEW , "CHRATR_SCALEW" },
        { RES_CHRATR_RELIEF , "CHRATR_RELIEF" },
        { RES_CHRATR_HIDDEN , "CHRATR_HIDDEN" },
        { RES_CHRATR_BOX , "CHRATR_BOX" },
        { RES_CHRATR_SHADOW , "CHRATR_SHADOW" },
        { RES_TXTATR_AUTOFMT , "TXTATR_AUTOFMT" },
        { RES_TXTATR_INETFMT , "TXTATR_INETFMT" },
        { RES_TXTATR_REFMARK , "TXTATR_REFMARK" },
        { RES_TXTATR_TOXMARK , "TXTATR_TOXMARK" },
        { RES_TXTATR_CHARFMT , "TXTATR_CHARFMT" },
        { RES_TXTATR_INPUTFIELD , "RES_TXTATR_INPUTFIELD" },
        { RES_TXTATR_CONTENTCONTROL , "RES_TXTATR_CONTENTCONTROL" },
        { RES_TXTATR_CJK_RUBY , "TXTATR_CJK_RUBY" },
        { RES_TXTATR_UNKNOWN_CONTAINER , "TXTATR_UNKNOWN_CONTAINER" },
        { RES_TXTATR_META , "TXTATR_META" },
        { RES_TXTATR_METAFIELD , "TXTATR_METAFIELD" },
        { RES_TXTATR_FIELD , "TXTATR_FIELD" },
        { RES_TXTATR_FLYCNT , "TXTATR_FLYCNT" },
        { RES_TXTATR_FTN , "TXTATR_FTN" },
        { RES_TXTATR_ANNOTATION , "TXTATR_ANNOTATION" },
        { RES_TXTATR_LINEBREAK , "RES_TXTATR_LINEBREAK" },
        { RES_TXTATR_DUMMY1 , "TXTATR_DUMMY1" },
        { RES_PARATR_LINESPACING , "PARATR_LINESPACING" },
        { RES_PARATR_ADJUST , "PARATR_ADJUST" },
        { RES_PARATR_SPLIT , "PARATR_SPLIT" },
        { RES_PARATR_ORPHANS , "PARATR_ORPHANS" },
        { RES_PARATR_WIDOWS , "PARATR_WIDOWS" },
        { RES_PARATR_TABSTOP , "PARATR_TABSTOP" },
        { RES_PARATR_HYPHENZONE , "PARATR_HYPHENZONE" },
        { RES_PARATR_DROP , "PARATR_DROP" },
        { RES_PARATR_REGISTER , "PARATR_REGISTER" },
        { RES_PARATR_NUMRULE , "PARATR_NUMRULE" },
        { RES_PARATR_SCRIPTSPACE , "PARATR_SCRIPTSPACE" },
        { RES_PARATR_HANGINGPUNCTUATION , "PARATR_HANGINGPUNCTUATION" },
        { RES_PARATR_FORBIDDEN_RULES , "PARATR_FORBIDDEN_RULES" },
        { RES_PARATR_VERTALIGN , "PARATR_VERTALIGN" },
        { RES_PARATR_SNAPTOGRID , "PARATR_SNAPTOGRID" },
        { RES_PARATR_CONNECT_BORDER , "PARATR_CONNECT_BORDER" },
        { RES_FILL_ORDER , "FILL_ORDER" },
        { RES_FRM_SIZE , "FRM_SIZE" },
        { RES_PAPER_BIN , "PAPER_BIN" },
        { RES_MARGIN_FIRSTLINE, "FIRSTLINE" },
        { RES_MARGIN_TEXTLEFT, "TEXTLEFT" },
        { RES_MARGIN_RIGHT, "RIGHT" },
        { RES_MARGIN_LEFT, "LEFT" },
        { RES_MARGIN_GUTTER, "GUTTER" },
        { RES_MARGIN_GUTTER_RIGHT, "GUTTER_RIGHT" },
        { RES_LR_SPACE , "LR_SPACE" },
        { RES_UL_SPACE , "UL_SPACE" },
        { RES_PAGEDESC , "PAGEDESC" },
        { RES_BREAK , "BREAK" },
        { RES_CNTNT , "CNTNT" },
        { RES_HEADER , "HEADER" },
        { RES_FOOTER , "FOOTER" },
        { RES_PRINT , "PRINT" },
        { RES_OPAQUE , "OPAQUE" },
        { RES_PROTECT , "PROTECT" },
        { RES_SURROUND , "SURROUND" },
        { RES_VERT_ORIENT , "VERT_ORIENT" },
        { RES_HORI_ORIENT , "HORI_ORIENT" },
        { RES_ANCHOR , "ANCHOR" },
        { RES_BACKGROUND , "BACKGROUND" },
        { RES_BOX , "BOX" },
        { RES_SHADOW , "SHADOW" },
        { RES_FRMMACRO , "FRMMACRO" },
        { RES_COL , "COL" },
        { RES_KEEP , "KEEP" },
        { RES_URL , "URL" },
        { RES_EDIT_IN_READONLY , "EDIT_IN_READONLY" },
        { RES_LAYOUT_SPLIT , "LAYOUT_SPLIT" },
        { RES_CHAIN , "CHAIN" },
        { RES_TEXTGRID , "TEXTGRID" },
        { RES_LINENUMBER   , "LINENUMBER" },
        { RES_FTN_AT_TXTEND , "FTN_AT_TXTEND" },
        { RES_END_AT_TXTEND , "END_AT_TXTEND" },
        { RES_COLUMNBALANCE , "COLUMNBALANCE" },
        { RES_FRAMEDIR , "FRAMEDIR" },
        { RES_HEADER_FOOTER_EAT_SPACING , "HEADER_FOOTER_EAT_SPACING" },
        { RES_ROW_SPLIT , "ROW_SPLIT" },
        { RES_GRFATR_MIRRORGRF , "GRFATR_MIRRORGRF" },
        { RES_GRFATR_CROPGRF , "GRFATR_CROPGRF" },
        { RES_GRFATR_ROTATION , "GRFATR_ROTATION" },
        { RES_GRFATR_LUMINANCE , "GRFATR_LUMINANCE" },
        { RES_GRFATR_CONTRAST , "GRFATR_CONTRAST" },
        { RES_GRFATR_CHANNELR , "GRFATR_CHANNELR" },
        { RES_GRFATR_CHANNELG , "GRFATR_CHANNELG" },
        { RES_GRFATR_CHANNELB , "GRFATR_CHANNELB" },
        { RES_GRFATR_GAMMA , "GRFATR_GAMMA" },
        { RES_GRFATR_INVERT , "GRFATR_INVERT" },
        { RES_GRFATR_TRANSPARENCY , "GRFATR_TRANSPARENCY" },
        { RES_GRFATR_DRAWMODE , "GRFATR_DRAWMODE" },
        { RES_BOXATR_FORMAT , "BOXATR_FORMAT" },
        { RES_BOXATR_FORMULA , "BOXATR_FORMULA" },
        { RES_BOXATR_VALUE , "BOXATR_VALUE" },
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
    SfxItemIter aIter(rSet);
    bool bFirst = true;
    OUStringBuffer aStr = "[ ";

    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
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

static OUString lcl_dbg_out(const SwNodeNum & )
{
    return OUString();/*rNum.ToString();*/
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
        rFrameFormat.GetName() + ")";

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
                pNumRule->GetName();

            const SwNumRuleItem * pItem = nullptr;

            if (pAttrSet &&
                (pItem = pAttrSet->GetItemIfSet(RES_PARATR_NUMRULE, false)))
            {
                aTmpStr += "(" + pItem->GetValue() + ")*";
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
            aTmpStr += "<coll>" + pColl->GetName() + "(";

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
            const OUString& sNumruleName = rItem.GetValue();

            if (!sNumruleName.isEmpty())
            {
                aTmpStr += ", " + sNumruleName;
            }
            aTmpStr += ")"
                "</coll>";
        }

        SwFormatColl * pCColl = pTextNode->GetCondFormatColl();

        if (pCColl)
        {
            aTmpStr += "<ccoll>" + pCColl->GetName() + "</ccoll>";
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
    OUStringBuffer aResult("[ " + rRule.GetName() + " [");

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
    return rFormat.GetName() + "(" +
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

        aResult.append(rTable[n]->GetName());

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

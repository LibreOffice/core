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

#include <tools/multisel.hxx>
#include <doc.hxx>
#include <IMark.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <fmtftn.hxx>
#include <modeltoviewhelper.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <txatbase.hxx>
#include <txtfld.hxx>
#include <txtftn.hxx>
#include <scriptinfo.hxx>
#include <IDocumentMarkAccess.hxx>
#include <set>
#include <vector>

namespace {

struct FieldResult
{
    sal_Int32 m_nFieldPos;
    OUString m_sExpand;
    enum { NONE, FIELD, FOOTNOTE } m_eType;
    explicit FieldResult(sal_Int32 const nPos)
        : m_nFieldPos(nPos), m_eType(NONE)
    { }
};

class sortfieldresults
{
public:
    bool operator()(const FieldResult &rOne, const FieldResult &rTwo) const
    {
        return rOne.m_nFieldPos < rTwo.m_nFieldPos;
    }
};

}

namespace {

struct block
{
    sal_Int32 m_nStart;
    sal_Int32 m_nLen;
    bool m_bVisible;
    std::set<FieldResult, sortfieldresults> m_aAttrs;
    block(sal_Int32 nStart, sal_Int32 nLen, bool bVisible)
        : m_nStart(nStart), m_nLen(nLen), m_bVisible(bVisible)
    {
    }
};

struct containsPos
{
    const sal_Int32 m_nPos;
    explicit containsPos(const sal_Int32 nPos)
        : m_nPos(nPos)
    {
    }
    bool operator() (const block& rIn) const
    {
        return m_nPos >= rIn.m_nStart && m_nPos < rIn.m_nStart + rIn.m_nLen;
    }
};

}

ModelToViewHelper::ModelToViewHelper(const SwTextNode &rNode,
        SwRootFrame const*const pLayout, ExpandMode eMode)
{
    const OUString& rNodeText = rNode.GetText();
    m_aRetText = rNodeText;

    if (eMode == ExpandMode::PassThrough)
        return;

    Range aRange( 0, rNodeText.isEmpty() ? 0 : rNodeText.getLength() - 1);
    MultiSelection aHiddenMulti(aRange);

    if (eMode & ExpandMode::HideInvisible)
        SwScriptInfo::selectHiddenTextProperty(rNode, aHiddenMulti, nullptr);

    if (eMode & ExpandMode::HideDeletions)
        SwScriptInfo::selectRedLineDeleted(rNode, aHiddenMulti);

    std::vector<block> aBlocks;

    sal_Int32 nShownStart = 0;
    for (sal_Int32 i = 0; i < aHiddenMulti.GetRangeCount(); ++i)
    {
        const Range& rRange = aHiddenMulti.GetRange(i);
        const sal_Int32 nHiddenStart = rRange.Min();
        const sal_Int32 nHiddenEnd = rRange.Max() + 1;
        const sal_Int32 nHiddenLen = nHiddenEnd - nHiddenStart;

        const sal_Int32 nShownEnd = nHiddenStart;
        const sal_Int32 nShownLen = nShownEnd - nShownStart;

        if (nShownLen)
            aBlocks.emplace_back(nShownStart, nShownLen, true);

        if (nHiddenLen)
            aBlocks.emplace_back(nHiddenStart, nHiddenLen, false);

        nShownStart = nHiddenEnd;
    }

    sal_Int32 nTrailingShownLen = rNodeText.getLength() - nShownStart;
    if (nTrailingShownLen)
        aBlocks.emplace_back(nShownStart, nTrailingShownLen, true);

    if (eMode & ExpandMode::ExpandFields || eMode & ExpandMode::ExpandFootnote)
    {
        //first the normal fields, get their position in the node and what the text they expand
        //to is
        const SwpHints* pSwpHints2 = rNode.GetpSwpHints();
        for ( size_t i = 0; pSwpHints2 && i < pSwpHints2->Count(); ++i )
        {
            const SwTextAttr* pAttr = pSwpHints2->Get(i);
            if (pAttr->HasDummyChar())
            {
                const sal_Int32 nDummyCharPos = pAttr->GetStart();
                if (aHiddenMulti.IsSelected(nDummyCharPos))
                    continue;
                std::vector<block>::iterator aFind = std::find_if(aBlocks.begin(),
                    aBlocks.end(), containsPos(nDummyCharPos));
                if (aFind != aBlocks.end())
                {
                    FieldResult aFieldResult(nDummyCharPos);
                    switch (pAttr->Which())
                    {
                        case RES_TXTATR_FIELD:
                        case RES_TXTATR_ANNOTATION:
                            if (eMode & ExpandMode::ExpandFields)
                            {
                                // add a ZWSP before the expanded field in replace mode
                                aFieldResult.m_sExpand = ((eMode & ExpandMode::ReplaceMode)
                                    ? OUString(CHAR_ZWSP) : OUString("")) +
                                      static_txtattr_cast<SwTextField const*>(pAttr)->
                                      GetFormatField().GetField()->ExpandField(true, pLayout);
                                aFieldResult.m_eType = FieldResult::FIELD;
                            }
                            break;
                        case RES_TXTATR_FTN:
                            if (eMode & ExpandMode::ExpandFootnote)
                            {
                                const SwFormatFootnote& rFootnote = static_cast<SwTextFootnote const*>(pAttr)->GetFootnote();
                                const SwDoc *pDoc = rNode.GetDoc();
                                aFieldResult.m_sExpand = (eMode & ExpandMode::ReplaceMode)
                                    ? OUString(CHAR_ZWSP)
                                    : rFootnote.GetViewNumStr(*pDoc, pLayout);
                                aFieldResult.m_eType = FieldResult::FOOTNOTE;
                            }
                            break;
                        default:
                            break;
                    }
                    aFind->m_aAttrs.insert(aFieldResult);
                }
            }
        }

        if (eMode & ExpandMode::ExpandFields)
        {
            //now get the dropdown formfields, get their position in the node and what the text they expand
            //to is
            SwPaM aPaM(rNode, 0, rNode, rNode.Len());
            std::vector<sw::mark::IFieldmark*> aDropDowns =
                rNode.GetDoc()->getIDocumentMarkAccess()->getDropDownsFor(aPaM);

            for (sw::mark::IFieldmark *pMark : aDropDowns)
            {
                const sal_Int32 nDummyCharPos = pMark->GetMarkPos().nContent.GetIndex()-1;
                if (aHiddenMulti.IsSelected(nDummyCharPos))
                    continue;
                std::vector<block>::iterator aFind = std::find_if(aBlocks.begin(), aBlocks.end(),
                    containsPos(nDummyCharPos));
                if (aFind != aBlocks.end())
                {
                    FieldResult aFieldResult(nDummyCharPos);
                    aFieldResult.m_sExpand = (eMode & ExpandMode::ReplaceMode)
                        ? OUString(CHAR_ZWSP)
                        : sw::mark::ExpandFieldmark(pMark);
                    aFieldResult.m_eType = FieldResult::FIELD;
                    aFind->m_aAttrs.insert(aFieldResult);
                }
            }
        }
    }

    //store the end of each range in the model and where that end of range
    //maps to in the view
    sal_Int32 nOffset = 0;
    for (const auto& rBlock : aBlocks)
    {
        const sal_Int32 nBlockLen = rBlock.m_nLen;
        if (!nBlockLen)
            continue;
        const sal_Int32 nBlockStart = rBlock.m_nStart;
        const sal_Int32 nBlockEnd = nBlockStart + nBlockLen;

        if (!rBlock.m_bVisible)
        {
            sal_Int32 const modelBlockPos(nBlockEnd);
            sal_Int32 const viewBlockPos(nBlockStart + nOffset);
            m_aMap.emplace_back(modelBlockPos, viewBlockPos, false);

            m_aRetText = m_aRetText.replaceAt(nOffset + nBlockStart, nBlockLen, OUString());
            nOffset -= nBlockLen;
        }
        else
        {
            for (const auto& rAttr : rBlock.m_aAttrs)
            {
                sal_Int32 const modelFieldPos(rAttr.m_nFieldPos);
                sal_Int32 const viewFieldPos(rAttr.m_nFieldPos + nOffset);
                m_aMap.emplace_back(modelFieldPos, viewFieldPos, true );

                m_aRetText = m_aRetText.replaceAt(viewFieldPos, 1, rAttr.m_sExpand);
                nOffset += rAttr.m_sExpand.getLength() - 1;

                switch (rAttr.m_eType)
                {
                    case FieldResult::FIELD:
                        m_FieldPositions.push_back(viewFieldPos);
                    break;
                    case FieldResult::FOOTNOTE:
                        m_FootnotePositions.push_back(viewFieldPos);
                    break;
                    case FieldResult::NONE: /*ignore*/
                    break;
                }
            }

            sal_Int32 const modelEndBlock(nBlockEnd);
            sal_Int32 const viewFieldPos(nBlockEnd + nOffset);
            m_aMap.emplace_back(modelEndBlock, viewFieldPos, true);
        }
    }
}

/** Converts a model position into a view position
*/
sal_Int32 ModelToViewHelper::ConvertToViewPosition( sal_Int32 nModelPos ) const
{
    // Search for entry after nPos:
    auto aIter = std::find_if(m_aMap.begin(), m_aMap.end(),
        [nModelPos](const ConversionMapEntry& rEntry) { return rEntry.m_nModelPos >= nModelPos; });
    if (aIter != m_aMap.end())
    {
        //if it's an invisible portion, map all contained positions
        //to the anchor viewpos
        if (!aIter->m_bVisible)
            return aIter->m_nViewPos;

        //if it's a visible portion, then the view position is the anchor
        //viewpos - the offset of the input modelpos from the anchor
        //modelpos
        const sal_Int32 nOffsetFromEnd = aIter->m_nModelPos - nModelPos;
        return aIter->m_nViewPos - nOffsetFromEnd;
    }

    return nModelPos;
}

/** Converts a view position into a model position
*/
ModelToViewHelper::ModelPosition ModelToViewHelper::ConvertToModelPosition( sal_Int32 nViewPos ) const
{
    ModelPosition aRet;
    aRet.mnPos = nViewPos;

    // Search for entry after nPos:
    auto aIter = std::find_if(m_aMap.begin(), m_aMap.end(),
        [nViewPos](const ConversionMapEntry& rEntry) { return rEntry.m_nViewPos > nViewPos; });

    // If nViewPos is in front of first field, we are finished.
    if (aIter != m_aMap.end() && aIter != m_aMap.begin())
    {
        const sal_Int32 nPosModel  = aIter->m_nModelPos;
        const sal_Int32 nPosExpand = aIter->m_nViewPos;

        --aIter;

        // nPrevPosModel is the field position
        const sal_Int32 nPrevPosModel  = aIter->m_nModelPos;
        const sal_Int32 nPrevPosExpand = aIter->m_nViewPos;

        const sal_Int32 nLengthModel  = nPosModel - nPrevPosModel;
        const sal_Int32 nLengthExpand = nPosExpand - nPrevPosExpand;

        const sal_Int32 nFieldLengthExpand = nLengthExpand - nLengthModel + 1;
        const sal_Int32 nFieldEndExpand = nPrevPosExpand + nFieldLengthExpand;

        // Check if nPos is outside of field:
        if ( nFieldEndExpand <= nViewPos )
        {
            // nPos is outside of field:
            const sal_Int32 nDistToField = nViewPos - nFieldEndExpand + 1;
            aRet.mnPos = nPrevPosModel + nDistToField;
        }
        else
        {
            // nViewPos is inside a field:
            aRet.mnPos = nPrevPosModel;
            aRet.mnSubPos = nViewPos - nPrevPosExpand;
            aRet.mbIsField = true;
        }
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <modeltoviewhelper.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <fmtftn.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include <txtfld.hxx>
#include <txtftn.hxx>

#include <tools/multisel.hxx>

#include <scriptinfo.hxx>

struct block
{
    sal_Int32 m_nStart;
    sal_Int32 m_nLen;
    bool m_bVisible;
    std::vector<const SwTxtAttr*> m_aAttrs;
    block(sal_Int32 nStart, sal_Int32 nLen, bool bVisible)
        : m_nStart(nStart), m_nLen(nLen), m_bVisible(bVisible)
    {
    }
};

struct containsPos
{
    const sal_Int32 m_nPos;
    containsPos(const sal_Int32 nPos)
        : m_nPos(nPos)
    {
    }
    bool operator() (const block& rIn) const
    {
        return m_nPos >= rIn.m_nStart && m_nPos < rIn.m_nStart + rIn.m_nLen;
    }
};

ModelToViewHelper::ModelToViewHelper(const SwTxtNode &rNode, sal_uInt16 eMode)
{
    const OUString& rNodeText = rNode.GetTxt();
    m_aRetText = rNodeText;

    if (eMode == PASSTHROUGH)
        return;

    Range aRange( 0, rNodeText.isEmpty() ? 0 : rNodeText.getLength() - 1);
    MultiSelection aHiddenMulti(aRange);

    if (eMode & HIDEINVISIBLE)
        SwScriptInfo::selectHiddenTextProperty(rNode, aHiddenMulti);

    if (eMode & HIDEREDLINED)
        SwScriptInfo::selectRedLineDeleted(rNode, aHiddenMulti);

    std::vector<block> aBlocks;

    sal_Int32 nShownStart = 0;
    for (size_t i = 0; i < aHiddenMulti.GetRangeCount(); ++i)
    {
        const Range& rRange = aHiddenMulti.GetRange(i);
        const sal_Int32 nHiddenStart = rRange.Min();
        const sal_Int32 nHiddenEnd = rRange.Max() + 1;
        const sal_Int32 nHiddenLen = nHiddenEnd - nHiddenStart;

        const sal_Int32 nShownEnd = nHiddenStart;
        const sal_Int32 nShownLen = nShownEnd - nShownStart;

        if (nShownLen)
            aBlocks.push_back(block(nShownStart, nShownLen, true));

        if (nHiddenLen)
            aBlocks.push_back(block(nHiddenStart, nHiddenLen, false));

        nShownStart = nHiddenEnd;
    }

    sal_Int32 nTrailingShownLen = rNodeText.getLength() - nShownStart;
    if (nTrailingShownLen)
        aBlocks.push_back(block(nShownStart, nTrailingShownLen, true));

    if (eMode & EXPANDFIELDS)
    {
        const SwpHints* pSwpHints2 = rNode.GetpSwpHints();
        for ( sal_uInt16 i = 0; pSwpHints2 && i < pSwpHints2->Count(); ++i )
        {
            const SwTxtAttr* pAttr = (*pSwpHints2)[i];
            if (pAttr->HasDummyChar())
            {
                const sal_Int32 nDummyCharPos = *pAttr->GetStart();
                if (aHiddenMulti.IsSelected(nDummyCharPos))
                    continue;
                std::vector<block>::iterator aFind = std::find_if(aBlocks.begin(), aBlocks.end(), containsPos(nDummyCharPos));
                if (aFind != aBlocks.end())
                {
                    aFind->m_aAttrs.push_back(pAttr);
                }
            }
        }
    }

    sal_Int32 nOffset = 0;
    for (std::vector<block>::iterator i = aBlocks.begin(); i != aBlocks.end(); ++i)
    {
        if (!i->m_bVisible)
        {
            const sal_Int32 nHiddenStart = i->m_nStart;
            const sal_Int32 nHiddenLen = i->m_nLen;

            m_aRetText = m_aRetText.replaceAt( nOffset + nHiddenStart, nHiddenLen, OUString() );
            m_aMap.push_back( ConversionMapEntry( nHiddenStart, nOffset + nHiddenStart ) );
            nOffset -= nHiddenLen;
        }
        else
        {
            for (std::vector<const SwTxtAttr*>::iterator j = i->m_aAttrs.begin(); j != i->m_aAttrs.end(); ++j)
            {
                const SwTxtAttr* pAttr = *j;
                const sal_Int32 nFieldPos = *pAttr->GetStart();
                OUString aExpand;
                switch (pAttr->Which())
                {
                    case RES_TXTATR_FIELD:
                    case RES_TXTATR_ANNOTATION:
                        aExpand =
                            static_cast<SwTxtFld const*>(pAttr)->GetFmtFld().GetField()
                                ->ExpandField(true);
                        break;
                    case RES_TXTATR_FTN:
                        {
                            const SwFmtFtn& rFtn = static_cast<SwTxtFtn const*>(pAttr)->GetFtn();
                            const SwDoc *pDoc = rNode.GetDoc();
                            aExpand = rFtn.GetViewNumStr(*pDoc);
                        }
                        break;
                    default:
                        break;
                }
                m_aRetText = m_aRetText.replaceAt( nOffset + nFieldPos, 1, aExpand );
                m_aMap.push_back( ConversionMapEntry( nFieldPos, nOffset + nFieldPos ) );
                nOffset += aExpand.getLength() - 1;
            }
        }
    }

    if ( !m_aMap.empty() )
        m_aMap.push_back( ConversionMapEntry( rNodeText.getLength()+1, m_aRetText.getLength()+1 ) );
}

/** Converts a model position into a view position
*/
sal_Int32 ModelToViewHelper::ConvertToViewPosition( sal_Int32 nModelPos ) const
{
    // Search for entry after nPos:
    ConversionMap::const_iterator aIter;
    for ( aIter = m_aMap.begin(); aIter != m_aMap.end(); ++aIter )
    {
        if ( (*aIter).first >= nModelPos )
        {
            const sal_Int32 nPosModel  = (*aIter).first;
            const sal_Int32 nPosExpand = (*aIter).second;

            const sal_Int32 nDistToNextModel  = nPosModel - nModelPos;
            return nPosExpand - nDistToNextModel;
        }
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
    ConversionMap::const_iterator aIter;
    for ( aIter = m_aMap.begin(); aIter != m_aMap.end(); ++aIter )
    {
        if ( (*aIter).second > nViewPos )
        {
            const sal_Int32 nPosModel  = (*aIter).first;
            const sal_Int32 nPosExpand = (*aIter).second;

            // If nViewPos is in front of first field, we are finished.
            if ( aIter == m_aMap.begin() )
                break;

            --aIter;

            // nPrevPosModel is the field position
            const sal_Int32 nPrevPosModel  = (*aIter).first;
            const sal_Int32 nPrevPosExpand = (*aIter).second;

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

            break;
        }
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

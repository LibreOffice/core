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

#include <memory>
#include <editeng/unoedhlp.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <svl/itemset.hxx>

#include <osl/diagnose.h>


SvxEditSourceHint::SvxEditSourceHint( SfxHintId _nId ) :
    TextHint( _nId ),
    mnStart( 0 ),
    mnEnd( 0 )
{
}

SvxEditSourceHint::SvxEditSourceHint( SfxHintId _nId, sal_uLong nValue, sal_Int32 nStart, sal_Int32 nEnd ) :
    TextHint( _nId, nValue ),
    mnStart( nStart),
    mnEnd( nEnd )
{
}


std::unique_ptr<SfxHint> SvxEditSourceHelper::EENotification2Hint( EENotify const * aNotify )
{
    if( aNotify )
    {
        switch( aNotify->eNotificationType )
        {
            case EE_NOTIFY_TEXTMODIFIED:
                return std::unique_ptr<SfxHint>( new TextHint( SfxHintId::TextModified, aNotify->nParagraph ) );

            case EE_NOTIFY_PARAGRAPHINSERTED:
                return std::unique_ptr<SfxHint>( new TextHint( SfxHintId::TextParaInserted, aNotify->nParagraph ) );

            case EE_NOTIFY_PARAGRAPHREMOVED:
                return std::unique_ptr<SfxHint>( new TextHint( SfxHintId::TextParaRemoved, aNotify->nParagraph ) );

            case EE_NOTIFY_PARAGRAPHSMOVED:
                return std::unique_ptr<SfxHint>( new SvxEditSourceHint( SfxHintId::EditSourceParasMoved, aNotify->nParagraph, aNotify->nParam1, aNotify->nParam2 ) );

            case EE_NOTIFY_TextHeightChanged:
                return std::unique_ptr<SfxHint>( new TextHint( SfxHintId::TextHeightChanged, aNotify->nParagraph ) );

            case EE_NOTIFY_TEXTVIEWSCROLLED:
                return std::unique_ptr<SfxHint>( new TextHint( SfxHintId::TextViewScrolled ) );

            case EE_NOTIFY_TEXTVIEWSELECTIONCHANGED:
                return std::unique_ptr<SfxHint>( new SvxEditSourceHint( SfxHintId::EditSourceSelectionChanged ) );

            case EE_NOTIFY_PROCESSNOTIFICATIONS:
                return std::unique_ptr<SfxHint>( new TextHint( SfxHintId::TextProcessNotifications ));

            case EE_NOTIFY_TEXTVIEWSELECTIONCHANGED_ENDD_PARA:
                return std::unique_ptr<SfxHint>( new SvxEditSourceHintEndPara );
            default:
                OSL_FAIL( "SvxEditSourceHelper::EENotification2Hint unknown notification" );
                break;
        }
    }

    return std::make_unique<SfxHint>( );
}

void SvxEditSourceHelper::GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, const EditEngine& rEE, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell )
{
    // IA2 CWS introduced bInCell, but also did many other changes here.
    // Need to verify implementation with AT (IA2 and ATK)
    // Old implementation at the end of the method for reference...

    //added dummy attributes for the default text
    std::vector<EECharAttrib> aCharAttribs, aTempCharAttribs;
    rEE.GetCharAttribs( nPara, aTempCharAttribs );

    if (!aTempCharAttribs.empty())
    {
        sal_Int32 nIndex2 = 0;
        sal_Int32 nParaLen = rEE.GetTextLen(nPara);
        for (size_t nAttr = 0; nAttr < aTempCharAttribs.size(); ++nAttr)
        {
            if (nIndex2 < aTempCharAttribs[nAttr].nStart)
            {
                EECharAttrib aEEAttr;
                aEEAttr.nStart = nIndex2;
                aEEAttr.nEnd = aTempCharAttribs[nAttr].nStart;
                aCharAttribs.insert(aCharAttribs.begin() + nAttr, aEEAttr);
            }
            nIndex2 = aTempCharAttribs[nAttr].nEnd;
            aCharAttribs.push_back(aTempCharAttribs[nAttr]);
        }
        if ( nIndex2 != nParaLen )
        {
            EECharAttrib aEEAttr;
            aEEAttr.nStart = nIndex2;
            aEEAttr.nEnd = nParaLen;
            aCharAttribs.push_back(aEEAttr);
        }
    }
    // find closest index in front of nIndex
    sal_Int32 nCurrIndex;
    sal_Int32 nClosestStartIndex_s = 0, nClosestStartIndex_e = 0;
    for (auto const& charAttrib : aCharAttribs)
    {
        nCurrIndex = charAttrib.nStart;

        if( nCurrIndex > nClosestStartIndex_s &&
            nCurrIndex <= nIndex)
        {
            nClosestStartIndex_s = nCurrIndex;
        }
        nCurrIndex = charAttrib.nEnd;
        if ( nCurrIndex > nClosestStartIndex_e &&
            nCurrIndex < nIndex )
        {
            nClosestStartIndex_e = nCurrIndex;
        }
    }
    sal_Int32 nClosestStartIndex = std::max(nClosestStartIndex_s, nClosestStartIndex_e);

    // find closest index behind of nIndex
    sal_Int32 nClosestEndIndex_s, nClosestEndIndex_e;
    nClosestEndIndex_s = nClosestEndIndex_e = rEE.GetTextLen(nPara);
    for (auto const& charAttrib : aCharAttribs)
    {
        nCurrIndex = charAttrib.nEnd;

        if( nCurrIndex > nIndex &&
            nCurrIndex < nClosestEndIndex_e )
        {
            nClosestEndIndex_e = nCurrIndex;
        }
        nCurrIndex = charAttrib.nStart;
        if ( nCurrIndex > nIndex &&
            nCurrIndex < nClosestEndIndex_s)
        {
            nClosestEndIndex_s = nCurrIndex;
        }
    }
    sal_Int32 nClosestEndIndex = std::min(nClosestEndIndex_s, nClosestEndIndex_e);

    nStartIndex = nClosestStartIndex;
    nEndIndex = nClosestEndIndex;

    if ( bInCell )
    {
        EPosition aStartPos( nPara, nStartIndex ), aEndPos( nPara, nEndIndex );
        sal_Int32 nParaCount = rEE.GetParagraphCount();
        sal_Int32 nCrrntParaLen = rEE.GetTextLen(nPara);
        //need to find closest index in front of nIndex in the previous paragraphs
        if ( aStartPos.nIndex == 0 )
        {
            SfxItemSet aCrrntSet = rEE.GetAttribs( nPara, 0, 1, GetAttribsFlags::CHARATTRIBS );
            for ( sal_Int32 nParaIdx = nPara-1; nParaIdx >= 0; nParaIdx-- )
            {
                sal_uInt32 nLen = rEE.GetTextLen(nParaIdx);
                if ( nLen )
                {
                    sal_Int32 nStartIdx, nEndIdx;
                    GetAttributeRun( nStartIdx, nEndIdx, rEE, nParaIdx, nLen );
                    SfxItemSet aSet = rEE.GetAttribs( nParaIdx, nLen-1, nLen, GetAttribsFlags::CHARATTRIBS );
                    if ( aSet == aCrrntSet )
                    {
                        aStartPos.nPara = nParaIdx;
                        aStartPos.nIndex = nStartIdx;
                        if ( aStartPos.nIndex != 0 )
                        {
                            break;
                        }
                    }
                }
            }
        }
        //need find closest index behind nIndex in the following paragraphs
        if ( aEndPos.nIndex == nCrrntParaLen )
        {
            SfxItemSet aCrrntSet = rEE.GetAttribs( nPara, nCrrntParaLen-1, nCrrntParaLen, GetAttribsFlags::CHARATTRIBS );
            for ( sal_Int32 nParaIdx = nPara+1; nParaIdx < nParaCount; nParaIdx++ )
            {
                sal_Int32 nLen = rEE.GetTextLen( nParaIdx );
                if ( nLen )
                {
                    sal_Int32 nStartIdx, nEndIdx;
                    GetAttributeRun( nStartIdx, nEndIdx, rEE, nParaIdx, 0 );
                    SfxItemSet aSet = rEE.GetAttribs( nParaIdx, 0, 1, GetAttribsFlags::CHARATTRIBS );
                    if ( aSet == aCrrntSet )
                    {
                        aEndPos.nPara = nParaIdx;
                        aEndPos.nIndex = nEndIdx;
                        if ( aEndPos.nIndex != nLen )
                        {
                            break;
                        }
                    }
                }
            }
        }
        nStartIndex = 0;
        if ( aStartPos.nPara > 0 )
        {
            for ( sal_Int32 i = 0; i < aStartPos.nPara; i++ )
            {
                nStartIndex += rEE.GetTextLen(i)+1;
            }
        }
        nStartIndex += aStartPos.nIndex;
        nEndIndex = 0;
        if ( aEndPos.nPara > 0 )
        {
           for ( sal_Int32 i = 0; i < aEndPos.nPara; i++ )
           {
               nEndIndex += rEE.GetTextLen(i)+1;
           }
        }
        nEndIndex += aEndPos.nIndex;
    }
}

Point SvxEditSourceHelper::EEToUserSpace( const Point& rPoint, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Point( -rPoint.Y() + rEESize.Height(), rPoint.X() ) : rPoint;
}

Point SvxEditSourceHelper::UserSpaceToEE( const Point& rPoint, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Point( rPoint.Y(), -rPoint.X() + rEESize.Height() ) : rPoint;
}

tools::Rectangle SvxEditSourceHelper::EEToUserSpace( const tools::Rectangle& rRect, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? tools::Rectangle( EEToUserSpace(rRect.BottomLeft(), rEESize, bIsVertical),
                                    EEToUserSpace(rRect.TopRight(), rEESize, bIsVertical) ) : rRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

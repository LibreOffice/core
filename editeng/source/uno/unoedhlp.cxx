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

#include <editeng/unoedhlp.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <svl/itemset.hxx>



TYPEINIT1( SvxEditSourceHint, TextHint );

SvxEditSourceHint::SvxEditSourceHint( sal_uLong _nId ) :
    TextHint( _nId ),
    mnStart( 0 ),
    mnEnd( 0 )
{
}

SvxEditSourceHint::SvxEditSourceHint( sal_uLong _nId, sal_uLong nValue, sal_Int32 nStart, sal_Int32 nEnd ) :
    TextHint( _nId, nValue ),
    mnStart( nStart),
    mnEnd( nEnd )
{
}

sal_uLong SvxEditSourceHint::GetValue() const
{
    return TextHint::GetValue();
}

sal_Int32 SvxEditSourceHint::GetStartValue() const
{
    return mnStart;
}

sal_Int32 SvxEditSourceHint::GetEndValue() const
{
    return mnEnd;
}
TYPEINIT1( SvxEditSourceHintEndPara , SvxEditSourceHint );


SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr<SfxHint> SvxEditSourceHelper::EENotification2Hint( EENotify* aNotify )
{
    if( aNotify )
    {
        switch( aNotify->eNotificationType )
        {
            case EE_NOTIFY_TEXTMODIFIED:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_MODIFIED, aNotify->nParagraph ) );

            case EE_NOTIFY_PARAGRAPHINSERTED:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_PARAINSERTED, aNotify->nParagraph ) );

            case EE_NOTIFY_PARAGRAPHREMOVED:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_PARAREMOVED, aNotify->nParagraph ) );

            case EE_NOTIFY_PARAGRAPHSMOVED:
                return ::std::auto_ptr<SfxHint>( new SvxEditSourceHint( EDITSOURCE_HINT_PARASMOVED, aNotify->nParagraph, aNotify->nParam1, aNotify->nParam2 ) );

            case EE_NOTIFY_TEXTHEIGHTCHANGED:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_TEXTHEIGHTCHANGED, aNotify->nParagraph ) );

            case EE_NOTIFY_TEXTVIEWSCROLLED:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_VIEWSCROLLED ) );

            case EE_NOTIFY_TEXTVIEWSELECTIONCHANGED:
                return ::std::auto_ptr<SfxHint>( new SvxEditSourceHint( EDITSOURCE_HINT_SELECTIONCHANGED ) );

            case EE_NOTIFY_BLOCKNOTIFICATION_START:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_BLOCKNOTIFICATION_START, 0 ) );

            case EE_NOTIFY_BLOCKNOTIFICATION_END:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_BLOCKNOTIFICATION_END, 0 ) );

            case EE_NOTIFY_INPUT_START:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_INPUT_START, 0 ) );

            case EE_NOTIFY_INPUT_END:
                return ::std::auto_ptr<SfxHint>( new TextHint( TEXT_HINT_INPUT_END, 0 ) );
            case EE_NOTIFY_TEXTVIEWSELECTIONCHANGED_ENDD_PARA:
                return ::std::auto_ptr<SfxHint>( new SvxEditSourceHintEndPara( EDITSOURCE_HINT_SELECTIONCHANGED ) );
            default:
                OSL_FAIL( "SvxEditSourceHelper::EENotification2Hint unknown notification" );
                break;
        }
    }

    return ::std::auto_ptr<SfxHint>( new SfxHint() );
}
SAL_WNODEPRECATED_DECLARATIONS_POP

sal_Bool SvxEditSourceHelper::GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, const EditEngine& rEE, sal_Int32 nPara, sal_Int32 nIndex, sal_Bool bInCell )
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
    for(std::vector<EECharAttrib>::iterator i = aCharAttribs.begin(); i < aCharAttribs.end(); ++i)
    {
        nCurrIndex = i->nStart;

        if( nCurrIndex > nClosestStartIndex_s &&
            nCurrIndex <= nIndex)
        {
            nClosestStartIndex_s = nCurrIndex;
        }
        nCurrIndex = i->nEnd;
        if ( nCurrIndex > nClosestStartIndex_e &&
            nCurrIndex < nIndex )
        {
            nClosestStartIndex_e = nCurrIndex;
        }
    }
    sal_Int32 nClosestStartIndex = nClosestStartIndex_s > nClosestStartIndex_e ? nClosestStartIndex_s : nClosestStartIndex_e;

    // find closest index behind of nIndex
    sal_Int32 nClosestEndIndex_s, nClosestEndIndex_e;
    nClosestEndIndex_s = nClosestEndIndex_e = rEE.GetTextLen(nPara);
    for(std::vector<EECharAttrib>::iterator i = aCharAttribs.begin(); i < aCharAttribs.end(); ++i)
    {
        nCurrIndex = i->nEnd;

        if( nCurrIndex > nIndex &&
            nCurrIndex < nClosestEndIndex_e )
        {
            nClosestEndIndex_e = nCurrIndex;
        }
        nCurrIndex = i->nStart;
        if ( nCurrIndex > nIndex &&
            nCurrIndex < nClosestEndIndex_s)
        {
            nClosestEndIndex_s = nCurrIndex;
        }
    }
    sal_Int32 nClosestEndIndex = nClosestEndIndex_s < nClosestEndIndex_e ? nClosestEndIndex_s : nClosestEndIndex_e;

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
            SfxItemSet aCrrntSet = rEE.GetAttribs( nPara, 0, 1, GETATTRIBS_CHARATTRIBS );
            for ( sal_Int32 nParaIdx = nPara-1; nParaIdx >= 0; nParaIdx-- )
            {
                sal_uInt32 nLen = rEE.GetTextLen(nParaIdx);
                if ( nLen )
                {
                    sal_Int32 nStartIdx, nEndIdx;
                    GetAttributeRun( nStartIdx, nEndIdx, rEE, nParaIdx, nLen, sal_False );
                    SfxItemSet aSet = rEE.GetAttribs( nParaIdx, nLen-1, nLen, GETATTRIBS_CHARATTRIBS );
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
        //need find closest index behind nIndex in the following paragrphs
        if ( aEndPos.nIndex == nCrrntParaLen )
        {
            SfxItemSet aCrrntSet = rEE.GetAttribs( nPara, nCrrntParaLen-1, nCrrntParaLen, GETATTRIBS_CHARATTRIBS );
            for ( sal_Int32 nParaIdx = nPara+1; nParaIdx < nParaCount; nParaIdx++ )
            {
                sal_Int32 nLen = rEE.GetTextLen( nParaIdx );
                if ( nLen )
                {
                    sal_Int32 nStartIdx, nEndIdx;
                    GetAttributeRun( nStartIdx, nEndIdx, rEE, nParaIdx, 0, sal_False );
                    SfxItemSet aSet = rEE.GetAttribs( nParaIdx, 0, 1, GETATTRIBS_CHARATTRIBS );
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

    return sal_True;
}

Point SvxEditSourceHelper::EEToUserSpace( const Point& rPoint, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Point( -rPoint.Y() + rEESize.Height(), rPoint.X() ) : rPoint;
}

Point SvxEditSourceHelper::UserSpaceToEE( const Point& rPoint, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Point( rPoint.Y(), -rPoint.X() + rEESize.Height() ) : rPoint;
}

Rectangle SvxEditSourceHelper::EEToUserSpace( const Rectangle& rRect, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Rectangle( EEToUserSpace(rRect.BottomLeft(), rEESize, bIsVertical),
                                    EEToUserSpace(rRect.TopRight(), rEESize, bIsVertical) ) : rRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

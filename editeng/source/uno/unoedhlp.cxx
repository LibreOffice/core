/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"
#include <tools/debug.hxx>

#include <editeng/unoedhlp.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <svl/itemset.hxx>

//------------------------------------------------------------------------

TYPEINIT1( SvxEditSourceHint, TextHint );

SvxEditSourceHint::SvxEditSourceHint( sal_uLong _nId ) :
    TextHint( _nId ),
    mnStart( 0 ),
    mnEnd( 0 )
{
}

SvxEditSourceHint::SvxEditSourceHint( sal_uLong _nId, sal_uLong nValue, sal_uLong nStart, sal_uLong nEnd ) :
    TextHint( _nId, nValue ),
    mnStart( nStart),
    mnEnd( nEnd )
{
}

sal_uLong SvxEditSourceHint::GetValue() const
{
    return TextHint::GetValue();
}

sal_uLong SvxEditSourceHint::GetStartValue() const
{
    return mnStart;
}

sal_uLong SvxEditSourceHint::GetEndValue() const
{
    return mnEnd;
}

void SvxEditSourceHint::SetValue( sal_uLong n )
{
    TextHint::SetValue( n );
}

void SvxEditSourceHint::SetStartValue( sal_uLong n )
{
    mnStart = n;
}

void SvxEditSourceHint::SetEndValue( sal_uLong n )
{
    mnEnd = n;
}

//------------------------------------------------------------------------

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

            default:
                OSL_FAIL( "SvxEditSourceHelper::EENotification2Hint unknown notification" );
                break;
        }
    }

    return ::std::auto_ptr<SfxHint>( new SfxHint() );
}

sal_Bool SvxEditSourceHelper::GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, const EditEngine& rEE,
                                               sal_uInt16 nPara, sal_uInt16 nIndex, bool bInCell )
{

    //added dummy attributes for the default text
    EECharAttribArray aCharAttribs, aTempCharAttribs;
    rEE.GetCharAttribs( nPara, aTempCharAttribs );
    if ( aTempCharAttribs.Count() )
    {
        sal_uInt32 nIndex2 = 0;
        sal_uInt32 nParaLen = rEE.GetTextLen(nPara);
        for ( sal_uInt16 nAttr = 0; nAttr < aTempCharAttribs.Count(); nAttr++ )
        {
            if ( nIndex2 < aTempCharAttribs[nAttr].nStart )
            {
                EECharAttrib aEEAttr;
                aEEAttr.nStart = sal_uInt16(nIndex2);
                aEEAttr.nEnd = aTempCharAttribs[nAttr].nStart;
                aCharAttribs.Insert( aEEAttr, nAttr );
            }
            nIndex2 = aTempCharAttribs[nAttr].nEnd;
            aCharAttribs.Insert( aTempCharAttribs[nAttr], aCharAttribs.Count() );
        }
        if ( nIndex2 != nParaLen )
        {
            EECharAttrib aEEAttr;
            aEEAttr.nStart = sal_uInt16(nIndex2);
            aEEAttr.nEnd = sal_uInt16(nParaLen);
            aCharAttribs.Insert( aEEAttr, aCharAttribs.Count() );
        }
    }
    // find closest index in front of nIndex
    sal_uInt16 nAttr, nCurrIndex;
    sal_Int32 nClosestStartIndex;
    sal_Int32 nClosestStartIndex_s, nClosestStartIndex_e;
    for( nAttr=0, nClosestStartIndex_s=0, nClosestStartIndex_e=0; nAttr<aCharAttribs.Count(); ++nAttr )
    {
        nCurrIndex = aCharAttribs[nAttr].nStart;

        //if( nCurrIndex > nIndex )
        //    break; // aCharAttribs array is sorted in increasing order for nStart values

        if( nCurrIndex > nClosestStartIndex_s &&
            nCurrIndex <= nIndex)
        {
            nClosestStartIndex_s = nCurrIndex;
        }
        nCurrIndex = aCharAttribs[nAttr].nEnd;
        if ( nCurrIndex > nClosestStartIndex_e &&
             nCurrIndex < nIndex )
        {
            nClosestStartIndex_e = nCurrIndex;
        }
    }
    nClosestStartIndex = nClosestStartIndex_s > nClosestStartIndex_e ? nClosestStartIndex_s : nClosestStartIndex_e;

    // find closest index behind of nIndex
    sal_Int32 nClosestEndIndex;
    sal_Int32 nClosestEndIndex_s, nClosestEndIndex_e;
    for( nAttr=0, nClosestEndIndex_s=nClosestEndIndex_e=rEE.GetTextLen(nPara); nAttr<aCharAttribs.Count(); ++nAttr )
    {
        nCurrIndex = aCharAttribs[nAttr].nEnd;

        if( nCurrIndex > nIndex &&
            nCurrIndex < nClosestEndIndex_e )
        {
            nClosestEndIndex_e = nCurrIndex;
        }
        nCurrIndex = aCharAttribs[nAttr].nStart;
        if ( nCurrIndex > nIndex &&
             nCurrIndex < nClosestEndIndex_s)
        {
            nClosestEndIndex_s = nCurrIndex;
        }
    }
    nClosestEndIndex = nClosestEndIndex_s < nClosestEndIndex_e ? nClosestEndIndex_s : nClosestEndIndex_e;

    nStartIndex = static_cast<sal_uInt16>( nClosestStartIndex );
    nEndIndex = static_cast<sal_uInt16>( nClosestEndIndex );
    if ( bInCell )
    {
        EPosition aStartPos( nPara, nStartIndex ), aEndPos( nPara, nEndIndex );
        sal_uInt32 nParaCount = rEE.GetParagraphCount();
        sal_uInt32 nCrrntParaLen = rEE.GetTextLen(nPara);
        //need to find closest index in front of nIndex in the previous paragraphs
        if ( aStartPos.nIndex == 0 )
        {
            SfxItemSet aCrrntSet = rEE.GetAttribs( nPara, 0, 1, GETATTRIBS_CHARATTRIBS );
            for ( sal_Int32 nParaIdx = nPara-1; nParaIdx >= 0; nParaIdx-- )
            {
                sal_uInt32 nLen = rEE.GetTextLen( sal_uInt16(nParaIdx) );
                if ( nLen )
                {
                    sal_uInt16 nStartIdx, nEndIdx;
                    GetAttributeRun( nStartIdx, nEndIdx, rEE, sal_uInt16(nParaIdx), sal_uInt16(nLen), sal_False );
                    SfxItemSet aSet = rEE.GetAttribs( sal_uInt16(nParaIdx), sal_uInt16(nLen-1), sal_uInt16(nLen), GETATTRIBS_CHARATTRIBS );
                    if ( aSet == aCrrntSet )
                    {
                        aStartPos.nPara = sal_uInt16(nParaIdx);
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
            SfxItemSet aCrrntSet = rEE.GetAttribs( nPara, sal_uInt16(nCrrntParaLen-1), sal_uInt16(nCrrntParaLen), GETATTRIBS_CHARATTRIBS );
            for ( sal_uInt32 nParaIdx = nPara+1; nParaIdx < nParaCount; nParaIdx++ )
            {
                sal_uInt32 nLen = rEE.GetTextLen( sal_uInt16(nParaIdx) );
                if ( nLen )
                {
                    sal_uInt16 nStartIdx, nEndIdx;
                    GetAttributeRun( nStartIdx, nEndIdx, rEE, sal_uInt16(nParaIdx), 0, sal_False );
                    SfxItemSet aSet = rEE.GetAttribs( sal_uInt16(nParaIdx), 0, 1, GETATTRIBS_CHARATTRIBS );
                    if ( aSet == aCrrntSet )
                    {
                        aEndPos.nPara = sal_uInt16(nParaIdx);
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
            for ( sal_uInt16 i = 0; i < aStartPos.nPara; i++ )
            {
                nStartIndex += rEE.GetTextLen(i)+1;
            }
        }
        nStartIndex += aStartPos.nIndex;
        nEndIndex = 0;
        if ( aEndPos.nPara > 0 )
        {
            for ( sal_uInt16 i = 0; i < aEndPos.nPara; i++ )
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

Rectangle SvxEditSourceHelper::UserSpaceToEE( const Rectangle& rRect, const Size& rEESize, bool bIsVertical )
{
    return bIsVertical ? Rectangle( UserSpaceToEE(rRect.TopRight(), rEESize, bIsVertical),
                                    UserSpaceToEE(rRect.BottomLeft(), rEESize, bIsVertical) ) : rRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

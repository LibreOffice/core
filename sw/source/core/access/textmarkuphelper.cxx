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

#include <textmarkuphelper.hxx>
#include <accportions.hxx>

#include <vector>
#include <algorithm>
#include <comphelper/stlunosequence.hxx>

#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <ndtxt.hxx>
#include <wrong.hxx>

using namespace com::sun::star;

// helper functions
namespace {
    const SwWrongList* getTextMarkupList( const SwTxtNode& rTxtNode,
                                          const sal_Int32 nTextMarkupType )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException)
    {
        const SwWrongList* pTextMarkupList( 0 );
        switch ( nTextMarkupType )
        {
            case text::TextMarkupType::SPELLCHECK:
            {
                pTextMarkupList = rTxtNode.GetWrong();
            }
            break;
            case text::TextMarkupType::PROOFREADING:
            {
                // support not implemented yet
                pTextMarkupList = 0;
            }
            break;
            case text::TextMarkupType::SMARTTAG:
            {
                // support not implemented yet
                pTextMarkupList = 0;
            }
            break;
            default:
            {
                throw lang::IllegalArgumentException();
            }
        }

        return pTextMarkupList;
    }
}

// implementation of class <SwTextMarkupoHelper>
SwTextMarkupHelper::SwTextMarkupHelper( const SwAccessiblePortionData& rPortionData,
                                        const SwTxtNode& rTxtNode )
    : mrPortionData( rPortionData )
    // #i108125#
    , mpTxtNode( &rTxtNode )
    , mpTextMarkupList( 0 )
{
}

// #i108125#
SwTextMarkupHelper::SwTextMarkupHelper( const SwAccessiblePortionData& rPortionData,
                                        const SwWrongList& rTextMarkupList )
    : mrPortionData( rPortionData )
    , mpTxtNode( 0 )
    , mpTextMarkupList( &rTextMarkupList )
{
}

sal_Int32 SwTextMarkupHelper::getTextMarkupCount( const sal_Int32 nTextMarkupType )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nTextMarkupCount( 0 );

    // #i108125#
    const SwWrongList* pTextMarkupList =
                            mpTextMarkupList
                            ? mpTextMarkupList
                            : getTextMarkupList( *mpTxtNode, nTextMarkupType );
    if ( pTextMarkupList )
    {
        nTextMarkupCount = pTextMarkupList->Count();
    }

    return nTextMarkupCount;
}

::com::sun::star::accessibility::TextSegment
        SwTextMarkupHelper::getTextMarkup( const sal_Int32 nTextMarkupIndex,
                                           const sal_Int32 nTextMarkupType )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException)
{
    if ( nTextMarkupIndex >= getTextMarkupCount( nTextMarkupType ) ||
         nTextMarkupIndex < 0 )
    {
        throw lang::IndexOutOfBoundsException();
    }

    ::com::sun::star::accessibility::TextSegment aTextMarkupSegment;
    aTextMarkupSegment.SegmentStart = -1;
    aTextMarkupSegment.SegmentEnd = -1;

    // #i108125#
    const SwWrongList* pTextMarkupList =
                            mpTextMarkupList
                            ? mpTextMarkupList
                            : getTextMarkupList( *mpTxtNode, nTextMarkupType );
    if ( pTextMarkupList )
    {
        const SwWrongArea* pTextMarkup =
                pTextMarkupList->GetElement( static_cast<sal_uInt16>(nTextMarkupIndex) );
        if ( pTextMarkup )
        {
            const OUString rText = mrPortionData.GetAccessibleString();
            const sal_Int32 nStartPos =
                            mrPortionData.GetAccessiblePosition( pTextMarkup->mnPos );
            const sal_Int32 nEndPos =
                            mrPortionData.GetAccessiblePosition( pTextMarkup->mnPos + pTextMarkup->mnLen );
            aTextMarkupSegment.SegmentText = rText.copy( nStartPos, nEndPos - nStartPos );
            aTextMarkupSegment.SegmentStart = nStartPos;
            aTextMarkupSegment.SegmentEnd = nEndPos;
        }
        else
        {
            OSL_FAIL( "<SwTextMarkupHelper::getTextMarkup(..)> - missing <SwWrongArea> instance" );
        }
    }

    return aTextMarkupSegment;
}

::com::sun::star::uno::Sequence< ::com::sun::star::accessibility::TextSegment >
        SwTextMarkupHelper::getTextMarkupAtIndex( const sal_Int32 nCharIndex,
                                                  const sal_Int32 nTextMarkupType )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException)
{
    // assumption:
    // value of <nCharIndex> is in range [0..length of accessible text)

    const sal_uInt16 nCoreCharIndex = mrPortionData.GetModelPosition( nCharIndex );
    // Handling of portions with core length == 0 at the beginning of the
    // paragraph - e.g. numbering portion.
    if ( mrPortionData.GetAccessiblePosition( nCoreCharIndex ) > nCharIndex )
    {
        return uno::Sequence< ::com::sun::star::accessibility::TextSegment >();
    }

    // #i108125#
    const SwWrongList* pTextMarkupList =
                            mpTextMarkupList
                            ? mpTextMarkupList
                            : getTextMarkupList( *mpTxtNode, nTextMarkupType );
    ::std::vector< ::com::sun::star::accessibility::TextSegment > aTmpTextMarkups;
    if ( pTextMarkupList )
    {
        const OUString rText = mrPortionData.GetAccessibleString();

        const sal_uInt16 nTextMarkupCount = pTextMarkupList->Count();
        for ( sal_uInt16 nTextMarkupIdx = 0; nTextMarkupIdx < nTextMarkupCount; ++nTextMarkupIdx )
        {
            const SwWrongArea* pTextMarkup =
                    pTextMarkupList->GetElement( static_cast<sal_uInt16>(nTextMarkupIdx) );
            OSL_ENSURE( pTextMarkup,
                    "<SwTextMarkupHelper::getTextMarkup(..)> - missing <SwWrongArea> instance" );
            if ( pTextMarkup &&
                 pTextMarkup->mnPos <= nCoreCharIndex &&
                 nCoreCharIndex < ( pTextMarkup->mnPos + pTextMarkup->mnLen ) )
            {
                const sal_Int32 nStartPos =
                    mrPortionData.GetAccessiblePosition( pTextMarkup->mnPos );
                const sal_Int32 nEndPos =
                    mrPortionData.GetAccessiblePosition( pTextMarkup->mnPos + pTextMarkup->mnLen );
                ::com::sun::star::accessibility::TextSegment aTextMarkupSegment;
                aTextMarkupSegment.SegmentText = rText.copy( nStartPos, nEndPos - nStartPos );
                aTextMarkupSegment.SegmentStart = nStartPos;
                aTextMarkupSegment.SegmentEnd = nEndPos;
                aTmpTextMarkups.push_back( aTextMarkupSegment );
            }
        }
    }

    uno::Sequence< ::com::sun::star::accessibility::TextSegment > aTextMarkups(
                                                    aTmpTextMarkups.size() );
    ::std::copy( aTmpTextMarkups.begin(), aTmpTextMarkups.end(),
                 ::comphelper::stl_begin( aTextMarkups ) );

    return aTextMarkups;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

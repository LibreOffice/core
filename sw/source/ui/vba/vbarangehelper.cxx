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
#include "vbarangehelper.hxx"
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/**
 * get a range in a xText by creating
 * a cursor that iterates over the text. If the iterating cursor is
 * equal to the desired position, the range equivalent is returned.
 * Some special cases are tables that are inside of the text, because the
 * position has to be adjusted.
 * @param xText a text where a range position is searched
 * @param position a position inside o the text
 * @return a range for the postion; null is returned if no range can be
 * constructed.
 */
uno::Reference< text::XTextRange > SwVbaRangeHelper::getRangeByPosition( const uno::Reference< text::XText >& rText, sal_Int32 _position ) throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xRange;
    if( rText.is() )
    {
        sal_Int32 nPos = 0;
        uno::Reference< text::XTextCursor > xCursor = rText->createTextCursor();
        xCursor->collapseToStart();
        sal_Bool bCanGo = sal_True;
        while( !xRange.is() && bCanGo )
        {
            if( _position == nPos )
            {
                xRange = xCursor->getStart();
            }
            else
            {
                bCanGo = xCursor->goRight( 1, sal_False );
                nPos++;
            }
        }
    }
    return xRange;
}


void SwVbaRangeHelper::insertString( uno::Reference< text::XTextRange >& rTextRange, uno::Reference< text::XText >& rText, const rtl::OUString& rStr, sal_Bool _bAbsorb ) throw ( uno::RuntimeException )
{
    sal_Int32 nlastIndex = 0;
    sal_Int32 nIndex = 0;
    uno::Reference< text::XTextRange > xRange = rTextRange;

    while(( nIndex = rStr.indexOf('\n', nlastIndex)) >= 0  )
    {
        xRange = xRange->getEnd();
        if( nlastIndex < ( nIndex - 1 ) )
        {
            rText->insertString( xRange, rStr.copy( nlastIndex, ( nIndex - 1 - nlastIndex ) ), _bAbsorb );
            xRange = xRange->getEnd();
        }

        rText->insertControlCharacter( xRange, text::ControlCharacter::PARAGRAPH_BREAK, _bAbsorb );
        nlastIndex = nIndex + 1;
    }

    if( nlastIndex < rStr.getLength() )
    {
        xRange = xRange->getEnd();

        rtl::OUString aWatt = rStr.copy( nlastIndex );
        rText->insertString( xRange, aWatt, _bAbsorb );
    }
}

uno::Reference< text::XTextCursor > SwVbaRangeHelper::initCursor( const uno::Reference< text::XTextRange >& rTextRange, const uno::Reference< text::XText >& rText ) throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextCursor > xTextCursor;
    sal_Bool bGotTextCursor = sal_False;

    try
    {
        xTextCursor = rText->createTextCursorByRange( rTextRange );
        bGotTextCursor = sal_True;
    }
    catch (uno::Exception& e)
    {
        DebugHelper::exception(e);
    }

    if( !bGotTextCursor || !xTextCursor.is() )
    {
        try
        {
            uno::Reference< text::XText > xText = rTextRange->getText();
            xTextCursor = xText->createTextCursor();
            bGotTextCursor = sal_True;
        }
        catch( uno::Exception& e )
        {
            DebugHelper::exception(e);
        }
    }

    if( !bGotTextCursor || !xTextCursor.is() )
    {
        try
        {
            xTextCursor = rText->createTextCursor();
            bGotTextCursor = sal_True;
        }
        catch( uno::Exception& e )
        {
            DebugHelper::exception(e);
        }
    }
    return xTextCursor;
}

sal_Int32 SwVbaRangeHelper::getPosition( const uno::Reference< text::XText >& rText, const uno::Reference< text::XTextRange >& rTextRange ) throw ( uno::RuntimeException )
{
    sal_Int32 nPosition = -1;
    if( rText.is() && rTextRange.is() )
    {
        nPosition = 0;
        uno::Reference< text::XTextCursor > xCursor = rText->createTextCursor();
        xCursor->collapseToStart();
        uno::Reference< text::XTextRangeCompare > xCompare( rText, uno::UNO_QUERY_THROW );
        // compareValue is 0 if the ranges are equal
        sal_Int32 nCompareValue = xCompare->compareRegionStarts( xCursor->getStart(), rTextRange );
        sal_Bool canGo = sal_True;

        while( nCompareValue !=0 && canGo )
        {
            canGo = xCursor->goRight( 1, sal_False );
            nCompareValue = xCompare->compareRegionStarts( xCursor->getStart(), rTextRange );
            nPosition++;
        }

        // check fails: no correct position found
        if( !canGo && nCompareValue != 0 )
        {
            nPosition = -1;
        }
    }

    return nPosition;
}

uno::Reference< text::XTextContent > SwVbaRangeHelper::findBookmarkByPosition( const uno::Reference< text::XTextDocument >& xTextDoc, const uno::Reference< text::XTextRange >& xTextRange ) throw ( css::uno::RuntimeException )
{
    uno::Reference< text::XBookmarksSupplier > xBookmarksSupplier( xTextDoc, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY_THROW );
    for( sal_Int32 index = 0; index < xIndexAccess->getCount(); index++ )
    {
        uno::Reference< text::XTextContent > xBookmark( xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextRange > xBkAnchor = xBookmark->getAnchor();
        uno::Reference< text::XTextRangeCompare > xCompare( xBkAnchor->getText(), uno::UNO_QUERY_THROW );
        if( xCompare->compareRegionStarts( xBkAnchor->getStart(), xBkAnchor->getEnd() ) == 0 )
        {
            try
            {
                if( xCompare->compareRegionStarts( xTextRange, xBkAnchor->getStart() ) == 0 )
                    return xBookmark;
            }
            catch( uno::Exception& )
            {
                continue;
            }
        }
    }
    return uno::Reference< text::XTextContent >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

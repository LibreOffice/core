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
#include "vbacomment.hxx"

#include <ooo/vba/excel/XlCreator.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>

#include <vbahelper/vbashape.hxx>
#include "vbaglobals.hxx"
#include "vbacomments.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaComment::ScVbaComment(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< table::XCellRange >& xRange ) throw( lang::IllegalArgumentException ) :
    ScVbaComment_BASE( xParent, xContext ),
    mxModel( xModel, uno::UNO_SET_THROW ),
    mxRange( xRange )
{
    if  ( !xRange.is() )
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "range is not set " ) ), uno::Reference< uno::XInterface >() , 1 );
    uno::Reference< text::XSimpleText > xAnnoText( getAnnotation(), uno::UNO_QUERY );
}

// private helper functions

uno::Reference< sheet::XSheetAnnotation > SAL_CALL
ScVbaComment::getAnnotation() throw (uno::RuntimeException)
{
    uno::Reference< table::XCell > xCell( mxRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSheetAnnotationAnchor > xAnnoAnchor( xCell, uno::UNO_QUERY_THROW );
    return uno::Reference< sheet::XSheetAnnotation > ( xAnnoAnchor->getAnnotation(), uno::UNO_QUERY_THROW );
}

uno::Reference< sheet::XSheetAnnotations > SAL_CALL
ScVbaComment::getAnnotations() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSheetCellRange > xSheetCellRange(mxRange, ::uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet = xSheetCellRange->getSpreadsheet();
    uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnosSupp( xSheet, uno::UNO_QUERY_THROW );

    return uno::Reference< sheet::XSheetAnnotations > ( xAnnosSupp->getAnnotations(), uno::UNO_QUERY_THROW );
}

sal_Int32 SAL_CALL
ScVbaComment::getAnnotationIndex() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSheetAnnotations > xAnnos = getAnnotations();
    table::CellAddress aAddress = getAnnotation()->getPosition();

    sal_Int32 aIndex = 0;
    sal_Int32 aCount = xAnnos->getCount();

    for ( ; aIndex < aCount ; aIndex++ )
    {
        uno::Reference< sheet::XSheetAnnotation > xAnno( xAnnos->getByIndex( aIndex ), uno::UNO_QUERY_THROW );
        table::CellAddress xAddress = xAnno->getPosition();

        if ( xAddress.Column == aAddress.Column && xAddress.Row == aAddress.Row && xAddress.Sheet == aAddress.Sheet )
        {
            OSL_TRACE("** terminating search, index is %d", aIndex );
            break;
        }
    }
    OSL_TRACE("** returning index is %d", aIndex );

       return aIndex;
}

uno::Reference< excel::XComment > SAL_CALL
ScVbaComment::getCommentByIndex( sal_Int32 Index ) throw (uno::RuntimeException)
{
    uno::Reference< container::XIndexAccess > xIndexAccess( getAnnotations(), uno::UNO_QUERY_THROW );
    // parent is sheet ( parent of the range which is the parent of the comment )
    uno::Reference< XCollection > xColl(  new ScVbaComments( getParent()->getParent(), mxContext, mxModel, xIndexAccess ) );

    return uno::Reference< excel::XComment > ( xColl->Item( uno::makeAny( Index ), uno::Any() ), uno::UNO_QUERY_THROW );
 }

// public vba functions

rtl::OUString SAL_CALL
ScVbaComment::getAuthor() throw (uno::RuntimeException)
{
    return getAnnotation()->getAuthor();
}

void SAL_CALL
ScVbaComment::setAuthor( const rtl::OUString& /*_author*/ ) throw (uno::RuntimeException)
{
    // #TODO #FIXME  implementation needed
}

uno::Reference< msforms::XShape > SAL_CALL
ScVbaComment::getShape() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSheetAnnotationShapeSupplier > xAnnoShapeSupp( getAnnotation(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xAnnoShape( xAnnoShapeSupp->getAnnotationShape(), uno::UNO_SET_THROW );
    uno::Reference< sheet::XSheetCellRange > xCellRange( mxRange, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupp( xCellRange->getSpreadsheet(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShapes > xShapes( xDrawPageSupp->getDrawPage(), uno::UNO_QUERY_THROW );
    return new ScVbaShape( this, mxContext, xAnnoShape, xShapes, mxModel, office::MsoShapeType::msoComment );
}

sal_Bool SAL_CALL
ScVbaComment::getVisible() throw (uno::RuntimeException)
{
    return getAnnotation()->getIsVisible();
}

void SAL_CALL
ScVbaComment::setVisible( sal_Bool _visible ) throw (uno::RuntimeException)
{
    getAnnotation()->setIsVisible( _visible );
}

void SAL_CALL
ScVbaComment::Delete() throw (uno::RuntimeException)
{
    getAnnotations()->removeByIndex( getAnnotationIndex() );
}

uno::Reference< excel::XComment > SAL_CALL
ScVbaComment::Next() throw (uno::RuntimeException)
{
    // index: uno = 0, vba = 1
    return getCommentByIndex( getAnnotationIndex() + 2 );
}

uno::Reference< excel::XComment > SAL_CALL
ScVbaComment::Previous() throw (uno::RuntimeException)
{
    // index: uno = 0, vba = 1
    return getCommentByIndex( getAnnotationIndex() );
}

rtl::OUString SAL_CALL
ScVbaComment::Text( const uno::Any& aText, const uno::Any& aStart, const uno::Any& Overwrite ) throw (uno::RuntimeException)
{
    rtl::OUString sText;
    aText >>= sText;

    uno::Reference< text::XSimpleText > xAnnoText( getAnnotation(), uno::UNO_QUERY_THROW );
    rtl::OUString sAnnoText = xAnnoText->getString();

    if ( aStart.hasValue() )
    {
        sal_Int16 nStart = 0;
        sal_Bool bOverwrite = sal_True;
        Overwrite >>= bOverwrite;

        if ( aStart >>= nStart )
        {
            uno::Reference< text::XTextCursor > xTextCursor( xAnnoText->createTextCursor(), uno::UNO_QUERY_THROW );

            if ( bOverwrite )
            {
                xTextCursor->collapseToStart();
                xTextCursor->gotoStart( false );
                xTextCursor->goRight( nStart - 1, false );
                xTextCursor->gotoEnd( sal_True );
            }
            else
            {
                xTextCursor->collapseToStart();
                xTextCursor->gotoStart( false );
                xTextCursor->goRight( nStart - 1 , sal_True );
            }

            uno::Reference< text::XTextRange > xRange( xTextCursor, uno::UNO_QUERY_THROW );
            xAnnoText->insertString( xRange, sText, bOverwrite );
            return xAnnoText->getString();
        }
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScVbaComment::Text - bad Start value " ) ), uno::Reference< uno::XInterface >() );
    }
    else if ( aText.hasValue() )
    {
     uno::Reference< sheet::XCellAddressable > xCellAddr(mxRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
     table::CellAddress aAddress = xCellAddr->getCellAddress();
     getAnnotations()->insertNew( aAddress, sText );
    }

    return sAnnoText;
}

rtl::OUString
ScVbaComment::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaComment"));
}

uno::Sequence< rtl::OUString >
ScVbaComment::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.ScVbaComment" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

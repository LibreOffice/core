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
        throw lang::IllegalArgumentException( rtl::OUString( "range is not set " ), uno::Reference< uno::XInterface >() , 1 );
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
        throw uno::RuntimeException( rtl::OUString( "ScVbaComment::Text - bad Start value " ), uno::Reference< uno::XInterface >() );
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
    return rtl::OUString("ScVbaComment");
}

uno::Sequence< rtl::OUString >
ScVbaComment::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.ScVbaComment" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

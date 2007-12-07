/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbacomment.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:48:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "vbacomment.hxx"

#include <org/openoffice/excel/XlCreator.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>

#include "vbaglobals.hxx"
#include "vbacomments.hxx"


using namespace ::org::openoffice;
using namespace ::com::sun::star;

ScVbaComment::ScVbaComment( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ) throw( lang::IllegalArgumentException )
: ScVbaComment_BASE( xParent, xContext ), mxRange( xRange )
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
    uno::Reference< vba::XCollection > xColl(  new ScVbaComments( getParent()->getParent(), mxContext, xIndexAccess ) );

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
                xTextCursor->gotoStart( sal_False );
                xTextCursor->goRight( nStart - 1, sal_False );
                xTextCursor->gotoEnd( sal_True );
            }
            else
            {
                xTextCursor->collapseToStart();
                xTextCursor->gotoStart( sal_False );
                xTextCursor->goRight( nStart - 1 , sal_True );
            }

            uno::Reference< text::XTextRange > xRange( xTextCursor, uno::UNO_QUERY_THROW );
            xAnnoText->insertString( xRange, sText, bOverwrite );
        }
        else
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScVbaComment::Text - bad Start value " ) ), uno::Reference< uno::XInterface >() );
    }
    else if ( aText.hasValue() )
        xAnnoText->setString( sText );

    return sAnnoText;
}

rtl::OUString&
ScVbaComment::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaComment") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaComment::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.ScVbaComment" ) );
    }
    return aServiceNames;
}

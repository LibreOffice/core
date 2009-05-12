/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
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
#include "vbapagesetup.hxx"
#include "cellsuno.hxx"
#include "convuno.hxx"
#include "rangelst.hxx"
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <ooo/vba/excel/XlPageOrientation.hpp>
#include <ooo/vba/excel/XlOrder.hpp>
#include <ooo/vba/excel/Constants.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define ZOOM_IN 10
#define ZOOM_MAX 400

bool getScRangeListForAddress( const rtl::OUString& sName, ScDocShell* pDocSh, ScRange& refRange, ScRangeList& aCellRanges, formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1 ) throw ( uno::RuntimeException );

ScVbaPageSetup::ScVbaPageSetup(const uno::Reference< XHelperInterface >& xParent,
                const uno::Reference< uno::XComponentContext >& xContext,
                const uno::Reference< sheet::XSpreadsheet >& xSheet,
                const uno::Reference< frame::XModel >& xModel) throw (uno::RuntimeException):
           ScVbaPageSetup_BASE( xParent, xContext ),
        mxSheet( xSheet ), mxModel( xModel )
{
    // query for current page style
    uno::Reference< beans::XPropertySet > xSheetProps( mxSheet, uno::UNO_QUERY_THROW );
    uno::Any aValue = xSheetProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageStyle" )));
    rtl::OUString aStyleName;
    aValue >>= aStyleName;

    uno::Reference< style::XStyleFamiliesSupplier > xStyleFamiliesSup( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStyleFamiliesSup->getStyleFamilies();
    uno::Reference< container::XNameAccess > xPageStyle( xStyleFamilies->getByName(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyles"))), uno::UNO_QUERY_THROW );
    mxPageProps.set( xPageStyle->getByName(aStyleName), uno::UNO_QUERY_THROW );
}

rtl::OUString SAL_CALL ScVbaPageSetup::getPrintArea() throw (css::uno::RuntimeException)
{
    String aPrintArea;
    uno::Reference< sheet::XPrintAreas > xPrintAreas( mxSheet, uno::UNO_QUERY_THROW );
    uno::Sequence< table::CellRangeAddress > aSeq = xPrintAreas->getPrintAreas();
    sal_Int32 nCount = aSeq.getLength();
    if( nCount )
    {
        ScAddress::Details aDetails( formula::FormulaGrammar::CONV_XL_A1, 0, 0 );
        USHORT nFlags = SCA_VALID;
        nFlags |= ( SCA_TAB_ABSOLUTE | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB2_ABSOLUTE | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE );
        ScRangeList aRangeList;
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            ScRange aRange;
            ScUnoConversion::FillScRange( aRange, aSeq[i] );
            aRangeList.Append( aRange );
        }
        ScDocument* pDoc = getDocShell( mxModel )->GetDocument();
        aRangeList.Format( aPrintArea, nFlags, pDoc, formula::FormulaGrammar::CONV_XL_A1, ','  );
    }

    return aPrintArea;
}

void SAL_CALL ScVbaPageSetup::setPrintArea( const rtl::OUString& rAreas ) throw (css::uno::RuntimeException)
{
    uno::Reference< sheet::XPrintAreas > xPrintAreas( mxSheet, uno::UNO_QUERY_THROW );
    if( rAreas.getLength() == 0 ||
        rAreas.equalsIgnoreAsciiCase ( rtl::OUString::createFromAscii("FALSE") ) )
    {
        // print the whole sheet
        uno::Sequence< table::CellRangeAddress > aSeq;
        xPrintAreas->setPrintAreas( aSeq );
    }
    else
    {
        ScRangeList aCellRanges;
        ScRange aRange;
        if( getScRangeListForAddress( rAreas, getDocShell( mxModel ) , aRange, aCellRanges ) )
        {
            uno::Sequence< table::CellRangeAddress > aSeq( aCellRanges.Count() );
            USHORT i=0;
            for( ScRange* pRange = aCellRanges.First(); pRange; pRange = aCellRanges.Next() )
            {
                table::CellRangeAddress aRangeAddress;
                ScUnoConversion::FillApiRange( aRangeAddress, *pRange );
                aSeq[ i++ ] = aRangeAddress;
            }
            xPrintAreas->setPrintAreas( aSeq );
        }
    }
}

double SAL_CALL ScVbaPageSetup::getTopMargin() throw (css::uno::RuntimeException)
{
    sal_Bool headerOn = sal_False;
    sal_Int32 topMargin = 0;
    sal_Int32 headerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")));
        aValue >>= headerOn;

        aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")));
        aValue >>= topMargin;

        if( headerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderHeight")));
            aValue >>= headerHeight;
            topMargin += headerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( topMargin );
}

void SAL_CALL ScVbaPageSetup::setTopMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 topMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    sal_Bool headerOn = sal_False;
    sal_Int32 headerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")));
        aValue >>= headerOn;

        if( headerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderHeight")));
            aValue >>= headerHeight;
            topMargin -= headerHeight;
        }

        aValue <<= topMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL ScVbaPageSetup::getBottomMargin() throw (css::uno::RuntimeException)
{
    sal_Bool footerOn = sal_False;
    sal_Int32 bottomMargin = 0;
    sal_Int32 footerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")));
        aValue >>= footerOn;

        aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")));
        aValue >>= bottomMargin;

        if( footerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterHeight")));
            aValue >>= footerHeight;
            bottomMargin += footerHeight;
        }
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( bottomMargin );
}

void SAL_CALL ScVbaPageSetup::setBottomMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 bottomMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    sal_Bool footerOn = sal_False;
    sal_Int32 footerHeight = 0;

    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")));
        aValue >>= footerOn;

        if( footerOn )
        {
            aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterHeight")));
            aValue >>= footerHeight;
            bottomMargin -= footerHeight;
        }

        aValue <<= bottomMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL ScVbaPageSetup::getRightMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 rightMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin")));
        aValue >>= rightMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( rightMargin );;
}

void SAL_CALL ScVbaPageSetup::setRightMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 rightMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= rightMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }

}

double SAL_CALL ScVbaPageSetup::getLeftMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 leftMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin")));
        aValue >>= leftMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( leftMargin );;
}

void SAL_CALL ScVbaPageSetup::setLeftMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 leftMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= leftMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LeftMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL ScVbaPageSetup::getHeaderMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 headerMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")));
        aValue >>= headerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( headerMargin );;
}

void SAL_CALL ScVbaPageSetup::setHeaderMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 headerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= headerMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TopMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

double SAL_CALL ScVbaPageSetup::getFooterMargin() throw (css::uno::RuntimeException)
{
    sal_Int32 footerMargin = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")));
        aValue >>= footerMargin;
    }
    catch( uno::Exception& )
    {
    }

    return Millimeter::getInPoints( footerMargin );;
}

void SAL_CALL ScVbaPageSetup::setFooterMargin( double margin ) throw (css::uno::RuntimeException)
{
    sal_Int32 footerMargin = Millimeter::getInHundredthsOfOneMillimeter( margin );
    try
    {
        uno::Any aValue;
        aValue <<= footerMargin;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BottomMargin")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

sal_Int32 SAL_CALL ScVbaPageSetup::getOrientation() throw (css::uno::RuntimeException)
{
    sal_Int32 orientation = excel::XlPageOrientation::xlPortrait;
    try
    {
        sal_Bool isLandscape = sal_False;
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsLandscape")));
        aValue >>= isLandscape;

        if( isLandscape )
        {
            orientation = excel::XlPageOrientation::xlLandscape;
        }
    }
    catch( uno::Exception& )
    {
    }
    return orientation;
}

void SAL_CALL ScVbaPageSetup::setOrientation( sal_Int32 orientation ) throw (css::uno::RuntimeException)
{
    if( ( orientation != excel::XlPageOrientation::xlPortrait ) &&
        ( orientation != excel::XlPageOrientation::xlLandscape ) )
    {
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    }

    try
    {
        sal_Bool isLandscape = sal_False;
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsLandscape")));
        aValue >>= isLandscape;

        sal_Bool switchOrientation = sal_False;
        if(( isLandscape && orientation != excel::XlPageOrientation::xlLandscape ) ||
            ( !isLandscape && orientation != excel::XlPageOrientation::xlPortrait ))
        {
            switchOrientation = sal_True;
        }

        if( switchOrientation )
        {
            aValue <<= !isLandscape;
            uno::Any aHeight = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Height")));
            uno::Any aWidth = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Width")));
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsLandscape")), aValue );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Width")),  aHeight );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Height")), aWidth );
        }

        if( isLandscape )
        {
            orientation = excel::XlPageOrientation::xlLandscape;
        }
    }
    catch( uno::Exception& )
    {
    }
}

uno::Any SAL_CALL ScVbaPageSetup::getFitToPagesTall() throw (css::uno::RuntimeException)
{
    return mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleToPagesY")));
}

void SAL_CALL ScVbaPageSetup::setFitToPagesTall( const uno::Any& fitToPagesTall) throw (css::uno::RuntimeException)
{
    USHORT scaleToPageY = 0;
    try
    {
        sal_Bool aValue;
        if( fitToPagesTall.getValueTypeClass() != uno::TypeClass_BOOLEAN || (fitToPagesTall >>= aValue))
        {
            fitToPagesTall >>= scaleToPageY;
        }

        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleToPagesY")), uno::makeAny( scaleToPageY ));
    }
    catch( uno::Exception& )
    {
    }
}

uno::Any SAL_CALL ScVbaPageSetup::getFitToPagesWide() throw (css::uno::RuntimeException)
{
    return mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleToPagesX")));
}

void SAL_CALL ScVbaPageSetup::setFitToPagesWide( const uno::Any& fitToPagesWide) throw (css::uno::RuntimeException)
{
    USHORT scaleToPageX = 0;
    try
    {
        sal_Bool aValue = sal_False;
        if( fitToPagesWide.getValueTypeClass() != uno::TypeClass_BOOLEAN || (fitToPagesWide >>= aValue))
        {
            fitToPagesWide >>= scaleToPageX;
        }

        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleToPagesX")), uno::makeAny( scaleToPageX ));
    }
    catch( uno::Exception& )
    {
    }
}

uno::Any SAL_CALL ScVbaPageSetup::getZoom() throw (css::uno::RuntimeException)
{
    return mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageScale")));
}

void SAL_CALL ScVbaPageSetup::setZoom( const uno::Any& zoom) throw (css::uno::RuntimeException)
{
    USHORT pageScale = 0;
    try
    {
        if( zoom.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        {
            sal_Bool aValue = sal_False;
            zoom >>= aValue;
            if( aValue )
            {
                DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
            }
        }
        else
        {
            zoom >>= pageScale;
            if(( pageScale < ZOOM_IN )||( pageScale > ZOOM_MAX ))
            {
                DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
            }
        }

        // these only exist in S08
        USHORT nScale = 0;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleToPages")), uno::makeAny( nScale ));
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleToPagesX")), uno::makeAny( nScale ));
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleToPagesY")), uno::makeAny( nScale ));
    }
    catch( beans::UnknownPropertyException& )
    {
        if( pageScale == 0 )
        {
            DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
        }
    }
    catch( uno::Exception& )
    {
    }

    mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageScale")), uno::makeAny( pageScale ));
}

rtl::OUString SAL_CALL ScVbaPageSetup::getLeftHeader() throw (css::uno::RuntimeException)
{
    rtl::OUString leftHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent"))), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getLeftText();
            leftHeader = xText->getString();
        }
    }
    catch( uno::Exception& )
    {
    }

    return leftHeader;
}

void SAL_CALL ScVbaPageSetup::setLeftHeader( const rtl::OUString& leftHeader) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent"))), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getLeftText();
            xText->setString( leftHeader );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent")), uno::makeAny(xHeaderContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

rtl::OUString SAL_CALL ScVbaPageSetup::getCenterHeader() throw (css::uno::RuntimeException)
{
    rtl::OUString centerHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent"))), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getCenterText();
            centerHeader = xText->getString();
        }
    }
    catch( uno::Exception& )
    {
    }

    return centerHeader;
}

void SAL_CALL ScVbaPageSetup::setCenterHeader( const rtl::OUString& centerHeader) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent"))), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getCenterText();
            xText->setString( centerHeader );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent")), uno::makeAny(xHeaderContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

rtl::OUString SAL_CALL ScVbaPageSetup::getRightHeader() throw (css::uno::RuntimeException)
{
    rtl::OUString rightHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent"))), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getRightText();
            rightHeader = xText->getString();
        }
    }
    catch( uno::Exception& )
    {
    }

    return rightHeader;
}

void SAL_CALL ScVbaPageSetup::setRightHeader( const rtl::OUString& rightHeader) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent"))), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getRightText();
            xText->setString( rightHeader );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageHeaderContent")), uno::makeAny(xHeaderContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

rtl::OUString SAL_CALL ScVbaPageSetup::getLeftFooter() throw (css::uno::RuntimeException)
{
    rtl::OUString leftFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent"))), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getLeftText();
            leftFooter = xText->getString();
        }
    }
    catch( uno::Exception& )
    {
    }

    return leftFooter;
}

void SAL_CALL ScVbaPageSetup::setLeftFooter( const rtl::OUString& leftFooter) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent"))), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getLeftText();
            xText->setString( leftFooter );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent")), uno::makeAny(xFooterContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

rtl::OUString SAL_CALL ScVbaPageSetup::getCenterFooter() throw (css::uno::RuntimeException)
{
    rtl::OUString centerFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent"))), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getCenterText();
            centerFooter = xText->getString();
        }
    }
    catch( uno::Exception& )
    {
    }

    return centerFooter;
}

void SAL_CALL ScVbaPageSetup::setCenterFooter( const rtl::OUString& centerFooter) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent"))), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getCenterText();
            xText->setString( centerFooter );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent")), uno::makeAny(xFooterContent) );
        }
    }
    catch( uno::Exception& )
    {
    }

}

rtl::OUString SAL_CALL ScVbaPageSetup::getRightFooter() throw (css::uno::RuntimeException)
{
    rtl::OUString rightFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent"))), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getRightText();
            rightFooter = xText->getString();
        }
    }
    catch( uno::Exception& )
    {
    }

    return rightFooter;
}

void SAL_CALL ScVbaPageSetup::setRightFooter( const rtl::OUString& rightFooter) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent"))), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getRightText();
            xText->setString( rightFooter );
            mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RightPageFooterContent")), uno::makeAny(xFooterContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

sal_Int32 SAL_CALL ScVbaPageSetup::getOrder() throw (css::uno::RuntimeException)
{
    sal_Int32 order = excel::XlOrder::xlDownThenOver;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PrintDownFirst")));
        sal_Bool bPrintDownFirst = sal_False;
        aValue >>= bPrintDownFirst;
        if( !bPrintDownFirst )
            order = excel::XlOrder::xlOverThenDown;
    }
    catch( uno::Exception& )
    {
    }

    return order;
}

void SAL_CALL ScVbaPageSetup::setOrder( sal_Int32 order) throw (css::uno::RuntimeException)
{
    sal_Bool bOrder = sal_True;
    switch( order )
    {
        case excel::XlOrder::xlDownThenOver:
            break;
        case excel::XlOrder::xlOverThenDown:
            bOrder = sal_False;
            break;
        default:
            DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    }

    try
    {
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PrintDownFirst")), uno::makeAny( bOrder ));
    }
    catch( uno::Exception& )
    {
    }
}

sal_Int32 SAL_CALL ScVbaPageSetup::getFirstPageNumber() throw (css::uno::RuntimeException)
{
    sal_Int16 number = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstPageNumber")));
        aValue >>= number;
    }
    catch( uno::Exception& )
    {
    }

    if( number ==0 )
    {
        number = excel::Constants::xlAutomatic;
    }

    return number;
}

void SAL_CALL ScVbaPageSetup::setFirstPageNumber( sal_Int32 firstPageNumber) throw (css::uno::RuntimeException)
{
    if( firstPageNumber < 0 )
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    if( firstPageNumber == excel::Constants::xlAutomatic )
        firstPageNumber = 0;

    try
    {
        uno::Any aValue;
        aValue <<= (sal_Int16)firstPageNumber;
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstPageNumber")), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getCenterVertically() throw (css::uno::RuntimeException)
{
    sal_Bool centerVertically = sal_False;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CenterVertically")));
        aValue >>= centerVertically;
    }
    catch( uno::Exception& )
    {
    }
    return centerVertically;
}

void SAL_CALL ScVbaPageSetup::setCenterVertically( sal_Bool centerVertically) throw (css::uno::RuntimeException)
{
    try
    {
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CenterVertically")), uno::makeAny( centerVertically ));
    }
    catch( uno::Exception& )
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getCenterHorizontally() throw (css::uno::RuntimeException)
{
    sal_Bool centerHorizontally = sal_False;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CenterHorizontally")));
        aValue >>= centerHorizontally;
    }
    catch( uno::Exception& )
    {
    }
    return centerHorizontally;
}

void SAL_CALL ScVbaPageSetup::setCenterHorizontally( sal_Bool centerHorizontally) throw (css::uno::RuntimeException)
{
    try
    {
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CenterHorizontally")), uno::makeAny( centerHorizontally ));
    }
    catch( uno::Exception& )
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getPrintHeadings() throw (css::uno::RuntimeException)
{
    sal_Bool printHeadings = sal_False;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PrintHeaders")));
        aValue >>= printHeadings;
    }
    catch( uno::Exception& )
    {
    }
    return printHeadings;
}

void SAL_CALL ScVbaPageSetup::setPrintHeadings( sal_Bool printHeadings) throw (css::uno::RuntimeException)
{
    try
    {
        mxPageProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PrintHeaders")), uno::makeAny( printHeadings ));
    }
    catch( uno::Exception& )
    {
    }
}

rtl::OUString&
ScVbaPageSetup::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPageSetup") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaPageSetup::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.PageSetup" ) );
    }
    return aServiceNames;
}

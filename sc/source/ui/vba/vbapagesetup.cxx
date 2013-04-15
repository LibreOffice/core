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
#include "vbapagesetup.hxx"
#include "cellsuno.hxx"
#include "convuno.hxx"
#include "rangelst.hxx"
#include "excelvbahelper.hxx"
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <ooo/vba/excel/XlPageOrientation.hpp>
#include <ooo/vba/excel/XlOrder.hpp>
#include <ooo/vba/excel/Constants.hpp>
#include <i18nutil/paper.hxx>
#include <sal/macros.h>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define ZOOM_IN 10
#define ZOOM_MAX 400

bool getScRangeListForAddress( const OUString& sName, ScDocShell* pDocSh, ScRange& refRange, ScRangeList& aCellRanges, formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1 ) throw ( uno::RuntimeException );

ScVbaPageSetup::ScVbaPageSetup(const uno::Reference< XHelperInterface >& xParent,
                const uno::Reference< uno::XComponentContext >& xContext,
                const uno::Reference< sheet::XSpreadsheet >& xSheet,
                const uno::Reference< frame::XModel >& xModel) throw (uno::RuntimeException):
           ScVbaPageSetup_BASE( xParent, xContext ), mxSheet( xSheet )
{
    // query for current page style
    mxModel.set( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xSheetProps( mxSheet, uno::UNO_QUERY_THROW );
    uno::Any aValue = xSheetProps->getPropertyValue( OUString(  "PageStyle" ));
    OUString aStyleName;
    aValue >>= aStyleName;

    uno::Reference< style::XStyleFamiliesSupplier > xStyleFamiliesSup( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStyleFamiliesSup->getStyleFamilies();
    uno::Reference< container::XNameAccess > xPageStyle( xStyleFamilies->getByName(OUString( "PageStyles")), uno::UNO_QUERY_THROW );
    mxPageProps.set( xPageStyle->getByName(aStyleName), uno::UNO_QUERY_THROW );
    mnOrientLandscape = excel::XlPageOrientation::xlLandscape;
    mnOrientPortrait = excel::XlPageOrientation::xlPortrait;
}

OUString SAL_CALL ScVbaPageSetup::getPrintArea() throw (css::uno::RuntimeException)
{
    String aPrintArea;
    uno::Reference< sheet::XPrintAreas > xPrintAreas( mxSheet, uno::UNO_QUERY_THROW );
    uno::Sequence< table::CellRangeAddress > aSeq = xPrintAreas->getPrintAreas();
    sal_Int32 nCount = aSeq.getLength();
    if( nCount )
    {
        ScAddress::Details aDetails( formula::FormulaGrammar::CONV_XL_A1, 0, 0 );
        sal_uInt16 nFlags = SCA_VALID;
        nFlags |= ( SCA_TAB_ABSOLUTE | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB2_ABSOLUTE | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE );
        ScRangeList aRangeList;
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            ScRange aRange;
            ScUnoConversion::FillScRange( aRange, aSeq[i] );
            aRangeList.Append( aRange );
        }
        ScDocument* pDoc = excel::getDocShell( mxModel )->GetDocument();
        aRangeList.Format( aPrintArea, nFlags, pDoc, formula::FormulaGrammar::CONV_XL_A1, ','  );
    }

    return aPrintArea;
}

void SAL_CALL ScVbaPageSetup::setPrintArea( const OUString& rAreas ) throw (css::uno::RuntimeException)
{
    uno::Reference< sheet::XPrintAreas > xPrintAreas( mxSheet, uno::UNO_QUERY_THROW );
    if( rAreas.isEmpty() ||
        rAreas.equalsIgnoreAsciiCase ( OUString("FALSE") ) )
    {
        // print the whole sheet
        uno::Sequence< table::CellRangeAddress > aSeq;
        xPrintAreas->setPrintAreas( aSeq );
    }
    else
    {
        ScRangeList aCellRanges;
        ScRange aRange;
        if( getScRangeListForAddress( rAreas, excel::getDocShell( mxModel ) , aRange, aCellRanges ) )
        {
            uno::Sequence< table::CellRangeAddress > aSeq( aCellRanges.size() );
            for ( size_t i = 0, nRanges = aCellRanges.size(); i < nRanges; ++i )
            {
                ScRange* pRange = aCellRanges[ i ];
                table::CellRangeAddress aRangeAddress;
                ScUnoConversion::FillApiRange( aRangeAddress, *pRange );
                aSeq[ i++ ] = aRangeAddress;
            }
            xPrintAreas->setPrintAreas( aSeq );
        }
    }
}

double SAL_CALL ScVbaPageSetup::getHeaderMargin() throw (css::uno::RuntimeException)
{
    return VbaPageSetupBase::getHeaderMargin();
}

void SAL_CALL ScVbaPageSetup::setHeaderMargin( double margin ) throw (css::uno::RuntimeException)
{
    VbaPageSetupBase::setHeaderMargin( margin );
}

double SAL_CALL ScVbaPageSetup::getFooterMargin() throw (css::uno::RuntimeException)
{
    return VbaPageSetupBase::getFooterMargin();
}

void SAL_CALL ScVbaPageSetup::setFooterMargin( double margin ) throw (css::uno::RuntimeException)
{
    VbaPageSetupBase::setFooterMargin( margin );
}

uno::Any SAL_CALL ScVbaPageSetup::getFitToPagesTall() throw (css::uno::RuntimeException)
{
    return mxPageProps->getPropertyValue( OUString( "ScaleToPagesY"));
}

void SAL_CALL ScVbaPageSetup::setFitToPagesTall( const uno::Any& fitToPagesTall) throw (css::uno::RuntimeException)
{
    sal_uInt16 scaleToPageY = 0;
    try
    {
        sal_Bool aValue;
        if( fitToPagesTall.getValueTypeClass() != uno::TypeClass_BOOLEAN || (fitToPagesTall >>= aValue))
        {
            fitToPagesTall >>= scaleToPageY;
        }

        mxPageProps->setPropertyValue( OUString( "ScaleToPagesY"), uno::makeAny( scaleToPageY ));
    }
    catch( uno::Exception& )
    {
    }
}

uno::Any SAL_CALL ScVbaPageSetup::getFitToPagesWide() throw (css::uno::RuntimeException)
{
    return mxPageProps->getPropertyValue( OUString( "ScaleToPagesX"));
}

void SAL_CALL ScVbaPageSetup::setFitToPagesWide( const uno::Any& fitToPagesWide) throw (css::uno::RuntimeException)
{
    sal_uInt16 scaleToPageX = 0;
    try
    {
        sal_Bool aValue = false;
        if( fitToPagesWide.getValueTypeClass() != uno::TypeClass_BOOLEAN || (fitToPagesWide >>= aValue))
        {
            fitToPagesWide >>= scaleToPageX;
        }

        mxPageProps->setPropertyValue( OUString( "ScaleToPagesX"), uno::makeAny( scaleToPageX ));
    }
    catch( uno::Exception& )
    {
    }
}

uno::Any SAL_CALL ScVbaPageSetup::getZoom() throw (css::uno::RuntimeException)
{
    return mxPageProps->getPropertyValue( OUString( "PageScale"));
}

void SAL_CALL ScVbaPageSetup::setZoom( const uno::Any& zoom) throw (css::uno::RuntimeException)
{
    sal_uInt16 pageScale = 0;
    try
    {
        if( zoom.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        {
            sal_Bool aValue = false;
            zoom >>= aValue;
            if( aValue )
            {
                DebugHelper::exception(SbERR_BAD_PARAMETER, OUString() );
            }
        }
        else
        {
            zoom >>= pageScale;
            if(( pageScale < ZOOM_IN )||( pageScale > ZOOM_MAX ))
            {
                DebugHelper::exception(SbERR_BAD_PARAMETER, OUString() );
            }
        }

        // these only exist in S08
        sal_uInt16 nScale = 0;
        mxPageProps->setPropertyValue( OUString( "ScaleToPages"), uno::makeAny( nScale ));
        mxPageProps->setPropertyValue( OUString( "ScaleToPagesX"), uno::makeAny( nScale ));
        mxPageProps->setPropertyValue( OUString( "ScaleToPagesY"), uno::makeAny( nScale ));
    }
    catch( beans::UnknownPropertyException& )
    {
        if( pageScale == 0 )
        {
            DebugHelper::exception(SbERR_BAD_PARAMETER, OUString() );
        }
    }
    catch( uno::Exception& )
    {
    }

    mxPageProps->setPropertyValue( OUString( "PageScale"), uno::makeAny( pageScale ));
}

OUString SAL_CALL ScVbaPageSetup::getLeftHeader() throw (css::uno::RuntimeException)
{
    OUString leftHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( OUString( "RightPageHeaderContent")), uno::UNO_QUERY_THROW);
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

void SAL_CALL ScVbaPageSetup::setLeftHeader( const OUString& leftHeader) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( OUString( "RightPageHeaderContent")), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getLeftText();
            xText->setString( leftHeader );
            mxPageProps->setPropertyValue( OUString( "RightPageHeaderContent"), uno::makeAny(xHeaderContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getCenterHeader() throw (css::uno::RuntimeException)
{
    OUString centerHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( OUString( "RightPageHeaderContent")), uno::UNO_QUERY_THROW);
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

void SAL_CALL ScVbaPageSetup::setCenterHeader( const OUString& centerHeader) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( OUString( "RightPageHeaderContent")), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getCenterText();
            xText->setString( centerHeader );
            mxPageProps->setPropertyValue( OUString( "RightPageHeaderContent"), uno::makeAny(xHeaderContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getRightHeader() throw (css::uno::RuntimeException)
{
    OUString rightHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( OUString( "RightPageHeaderContent")), uno::UNO_QUERY_THROW);
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

void SAL_CALL ScVbaPageSetup::setRightHeader( const OUString& rightHeader) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue( OUString( "RightPageHeaderContent")), uno::UNO_QUERY_THROW);
        if( xHeaderContent.is() )
        {
            uno::Reference< text::XText > xText = xHeaderContent->getRightText();
            xText->setString( rightHeader );
            mxPageProps->setPropertyValue( OUString( "RightPageHeaderContent"), uno::makeAny(xHeaderContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getLeftFooter() throw (css::uno::RuntimeException)
{
    OUString leftFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( OUString( "RightPageFooterContent")), uno::UNO_QUERY_THROW);
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

void SAL_CALL ScVbaPageSetup::setLeftFooter( const OUString& leftFooter) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( OUString( "RightPageFooterContent")), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getLeftText();
            xText->setString( leftFooter );
            mxPageProps->setPropertyValue( OUString( "RightPageFooterContent"), uno::makeAny(xFooterContent) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getCenterFooter() throw (css::uno::RuntimeException)
{
    OUString centerFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( OUString( "RightPageFooterContent")), uno::UNO_QUERY_THROW);
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

void SAL_CALL ScVbaPageSetup::setCenterFooter( const OUString& centerFooter) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( OUString( "RightPageFooterContent")), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getCenterText();
            xText->setString( centerFooter );
            mxPageProps->setPropertyValue( OUString( "RightPageFooterContent"), uno::makeAny(xFooterContent) );
        }
    }
    catch( uno::Exception& )
    {
    }

}

OUString SAL_CALL ScVbaPageSetup::getRightFooter() throw (css::uno::RuntimeException)
{
    OUString rightFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( OUString( "RightPageFooterContent")), uno::UNO_QUERY_THROW);
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

void SAL_CALL ScVbaPageSetup::setRightFooter( const OUString& rightFooter) throw (css::uno::RuntimeException)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue( OUString( "RightPageFooterContent")), uno::UNO_QUERY_THROW);
        if( xFooterContent.is() )
        {
            uno::Reference< text::XText > xText = xFooterContent->getRightText();
            xText->setString( rightFooter );
            mxPageProps->setPropertyValue( OUString( "RightPageFooterContent"), uno::makeAny(xFooterContent) );
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
        uno::Any aValue = mxPageProps->getPropertyValue( OUString( "PrintDownFirst"));
        sal_Bool bPrintDownFirst = false;
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
            bOrder = false;
            break;
        default:
            DebugHelper::exception(SbERR_BAD_PARAMETER, OUString() );
    }

    try
    {
        mxPageProps->setPropertyValue( OUString( "PrintDownFirst"), uno::makeAny( bOrder ));
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
        uno::Any aValue = mxPageProps->getPropertyValue( OUString( "FirstPageNumber"));
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
        DebugHelper::exception(SbERR_BAD_PARAMETER, OUString() );
    if( firstPageNumber == excel::Constants::xlAutomatic )
        firstPageNumber = 0;

    try
    {
        uno::Any aValue;
        aValue <<= (sal_Int16)firstPageNumber;
        mxPageProps->setPropertyValue( OUString( "FirstPageNumber"), aValue );
    }
    catch( uno::Exception& )
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getCenterVertically() throw (css::uno::RuntimeException)
{
    sal_Bool centerVertically = false;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( OUString( "CenterVertically"));
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
        mxPageProps->setPropertyValue( OUString( "CenterVertically"), uno::makeAny( centerVertically ));
    }
    catch( uno::Exception& )
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getCenterHorizontally() throw (css::uno::RuntimeException)
{
    sal_Bool centerHorizontally = false;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( OUString( "CenterHorizontally"));
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
        mxPageProps->setPropertyValue( OUString( "CenterHorizontally"), uno::makeAny( centerHorizontally ));
    }
    catch( uno::Exception& )
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getPrintHeadings() throw (css::uno::RuntimeException)
{
    sal_Bool printHeadings = false;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue( OUString( "PrintHeaders"));
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
        mxPageProps->setPropertyValue( OUString( "PrintHeaders"), uno::makeAny( printHeadings ));
    }
    catch( uno::Exception& )
    {
    }
}

::sal_Bool SAL_CALL ScVbaPageSetup::getPrintGridlines() throw (uno::RuntimeException)
{
    return false;
}

void SAL_CALL ScVbaPageSetup::setPrintGridlines( ::sal_Bool /*_printgridlines*/ ) throw (uno::RuntimeException)
{
}

OUString SAL_CALL ScVbaPageSetup::getPrintTitleRows() throw (uno::RuntimeException)
{
    return OUString();
}
void SAL_CALL ScVbaPageSetup::setPrintTitleRows( const OUString& /*_printtitlerows*/ ) throw (css::uno::RuntimeException)
{
}
OUString SAL_CALL ScVbaPageSetup::getPrintTitleColumns() throw (uno::RuntimeException)
{
    return OUString();
}

void SAL_CALL ScVbaPageSetup::setPrintTitleColumns( const OUString& /*_printtitlecolumns*/ ) throw (uno::RuntimeException)
{
}

OUString
ScVbaPageSetup::getServiceImplName()
{
    return OUString("ScVbaPageSetup");
}

uno::Sequence< OUString >
ScVbaPageSetup::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString( "ooo.vba.excel.PageSetup"  );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

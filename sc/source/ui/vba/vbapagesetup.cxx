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
#include <convuno.hxx>
#include <rangelst.hxx>
#include <docsh.hxx>
#include "excelvbahelper.hxx"
#include "vbarange.hxx"
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <ooo/vba/excel/XlPageOrientation.hpp>
#include <ooo/vba/excel/XlOrder.hpp>
#include <ooo/vba/excel/Constants.hpp>
#include <ooo/vba/excel/XlPaperSize.hpp>
#include <basic/sberrors.hxx>
#include <filter/msfilter/util.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define ZOOM_IN 10
#define ZOOM_MAX 400

ScVbaPageSetup::ScVbaPageSetup(const uno::Reference< XHelperInterface >& xParent,
                const uno::Reference< uno::XComponentContext >& xContext,
                const uno::Reference< sheet::XSpreadsheet >& xSheet,
                const uno::Reference< frame::XModel >& xModel):
           ScVbaPageSetup_BASE( xParent, xContext ), mxSheet( xSheet ), mbIsLandscape( false )
{
    // query for current page style
    mxModel.set( xModel, uno::UNO_SET_THROW );
    uno::Reference< beans::XPropertySet > xSheetProps( mxSheet, uno::UNO_QUERY_THROW );
    uno::Any aValue = xSheetProps->getPropertyValue("PageStyle");
    OUString aStyleName;
    aValue >>= aStyleName;

    uno::Reference< style::XStyleFamiliesSupplier > xStyleFamiliesSup( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStyleFamiliesSup->getStyleFamilies();
    uno::Reference< container::XNameAccess > xPageStyle( xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY_THROW );
    mxPageProps.set( xPageStyle->getByName(aStyleName), uno::UNO_QUERY_THROW );
    mnOrientLandscape = excel::XlPageOrientation::xlLandscape;
    mnOrientPortrait = excel::XlPageOrientation::xlPortrait;
    mxPageProps->getPropertyValue("IsLandscape") >>= mbIsLandscape;
}

OUString SAL_CALL ScVbaPageSetup::getPrintArea()
{
    OUString aPrintArea;
    uno::Reference< sheet::XPrintAreas > xPrintAreas( mxSheet, uno::UNO_QUERY_THROW );
    const uno::Sequence< table::CellRangeAddress > aSeq = xPrintAreas->getPrintAreas();
    if( aSeq.hasElements() )
    {
        ScRangeList aRangeList;
        for( const auto& rRange : aSeq )
        {
            ScRange aRange;
            ScUnoConversion::FillScRange( aRange, rRange );
            aRangeList.push_back( aRange );
        }
        ScDocument& rDoc = excel::getDocShell( mxModel )->GetDocument();
        aRangeList.Format( aPrintArea, ScRefFlags::RANGE_ABS, rDoc, formula::FormulaGrammar::CONV_XL_A1, ','  );
    }

    return aPrintArea;
}

void SAL_CALL ScVbaPageSetup::setPrintArea( const OUString& rAreas )
{
    uno::Reference< sheet::XPrintAreas > xPrintAreas( mxSheet, uno::UNO_QUERY_THROW );
    if( rAreas.isEmpty() ||
        rAreas.equalsIgnoreAsciiCase( "FALSE" ) )
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
                ScRange & rRange = aCellRanges[ i ];
                table::CellRangeAddress aRangeAddress;
                ScUnoConversion::FillApiRange( aRangeAddress, rRange );
                aSeq[ i++ ] = aRangeAddress;
            }
            xPrintAreas->setPrintAreas( aSeq );
        }
    }
}

double SAL_CALL ScVbaPageSetup::getHeaderMargin()
{
    return VbaPageSetupBase::getHeaderMargin();
}

void SAL_CALL ScVbaPageSetup::setHeaderMargin( double margin )
{
    VbaPageSetupBase::setHeaderMargin( margin );
}

double SAL_CALL ScVbaPageSetup::getFooterMargin()
{
    return VbaPageSetupBase::getFooterMargin();
}

void SAL_CALL ScVbaPageSetup::setFooterMargin( double margin )
{
    VbaPageSetupBase::setFooterMargin( margin );
}

uno::Any SAL_CALL ScVbaPageSetup::getFitToPagesTall()
{
    return mxPageProps->getPropertyValue("ScaleToPagesY");
}

void SAL_CALL ScVbaPageSetup::setFitToPagesTall( const uno::Any& fitToPagesTall)
{
    try
    {
        sal_uInt16 scaleToPageY = 0;
        bool aValue;
        if( fitToPagesTall.getValueTypeClass() != uno::TypeClass_BOOLEAN || (fitToPagesTall >>= aValue))
        {
            fitToPagesTall >>= scaleToPageY;
        }

        mxPageProps->setPropertyValue("ScaleToPagesY", uno::makeAny( scaleToPageY ));
    }
    catch( uno::Exception& )
    {
    }
}

uno::Any SAL_CALL ScVbaPageSetup::getFitToPagesWide()
{
    return mxPageProps->getPropertyValue("ScaleToPagesX");
}

void SAL_CALL ScVbaPageSetup::setFitToPagesWide( const uno::Any& fitToPagesWide)
{
    try
    {
        sal_uInt16 scaleToPageX = 0;
        bool aValue = false;
        if( fitToPagesWide.getValueTypeClass() != uno::TypeClass_BOOLEAN || (fitToPagesWide >>= aValue))
        {
            fitToPagesWide >>= scaleToPageX;
        }

        mxPageProps->setPropertyValue("ScaleToPagesX", uno::makeAny( scaleToPageX ));
    }
    catch( uno::Exception& )
    {
    }
}

uno::Any SAL_CALL ScVbaPageSetup::getZoom()
{
    return mxPageProps->getPropertyValue("PageScale");
}

void SAL_CALL ScVbaPageSetup::setZoom( const uno::Any& zoom)
{
    sal_uInt16 pageScale = 0;
    try
    {
        if( zoom.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        {
            bool aValue = false;
            zoom >>= aValue;
            if( aValue )
            {
                DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER);
            }
        }
        else
        {
            zoom >>= pageScale;
            if(( pageScale < ZOOM_IN )||( pageScale > ZOOM_MAX ))
            {
                DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER);
            }
        }

        // these only exist in S08
        sal_uInt16 nScale = 0;
        mxPageProps->setPropertyValue("ScaleToPages", uno::makeAny( nScale ));
        mxPageProps->setPropertyValue("ScaleToPagesX", uno::makeAny( nScale ));
        mxPageProps->setPropertyValue("ScaleToPagesY", uno::makeAny( nScale ));
    }
    catch (const beans::UnknownPropertyException&)
    {
        if( pageScale == 0 )
        {
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER);
        }
    }
    catch (const uno::Exception&)
    {
    }

    mxPageProps->setPropertyValue("PageScale", uno::makeAny( pageScale ));
}

OUString SAL_CALL ScVbaPageSetup::getLeftHeader()
{
    OUString leftHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xHeaderContent->getLeftText();
        leftHeader = xText->getString();
    }
    catch( uno::Exception& )
    {
    }

    return leftHeader;
}

void SAL_CALL ScVbaPageSetup::setLeftHeader( const OUString& leftHeader)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xHeaderContent->getLeftText();
        xText->setString( leftHeader );
        mxPageProps->setPropertyValue("RightPageHeaderContent", uno::makeAny(xHeaderContent) );
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getCenterHeader()
{
    OUString centerHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xHeaderContent->getCenterText();
        centerHeader = xText->getString();
    }
    catch( uno::Exception& )
    {
    }

    return centerHeader;
}

void SAL_CALL ScVbaPageSetup::setCenterHeader( const OUString& centerHeader)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xHeaderContent->getCenterText();
        xText->setString( centerHeader );
        mxPageProps->setPropertyValue("RightPageHeaderContent", uno::makeAny(xHeaderContent) );
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getRightHeader()
{
    OUString rightHeader;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xHeaderContent->getRightText();
        rightHeader = xText->getString();
    }
    catch( uno::Exception& )
    {
    }

    return rightHeader;
}

void SAL_CALL ScVbaPageSetup::setRightHeader( const OUString& rightHeader)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent( mxPageProps->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xHeaderContent->getRightText();
        xText->setString( rightHeader );
        mxPageProps->setPropertyValue("RightPageHeaderContent", uno::makeAny(xHeaderContent) );
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getLeftFooter()
{
    OUString leftFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue("RightPageFooterContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xFooterContent->getLeftText();
        leftFooter = xText->getString();
    }
    catch( uno::Exception& )
    {
    }

    return leftFooter;
}

void SAL_CALL ScVbaPageSetup::setLeftFooter( const OUString& leftFooter)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue("RightPageFooterContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xFooterContent->getLeftText();
        xText->setString( leftFooter );
        mxPageProps->setPropertyValue("RightPageFooterContent", uno::makeAny(xFooterContent) );
    }
    catch( uno::Exception& )
    {
    }
}

OUString SAL_CALL ScVbaPageSetup::getCenterFooter()
{
    OUString centerFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue("RightPageFooterContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xFooterContent->getCenterText();
        centerFooter = xText->getString();
    }
    catch( uno::Exception& )
    {
    }

    return centerFooter;
}

void SAL_CALL ScVbaPageSetup::setCenterFooter( const OUString& centerFooter)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue("RightPageFooterContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xFooterContent->getCenterText();
        xText->setString( centerFooter );
        mxPageProps->setPropertyValue("RightPageFooterContent", uno::makeAny(xFooterContent) );
    }
    catch( uno::Exception& )
    {
    }

}

OUString SAL_CALL ScVbaPageSetup::getRightFooter()
{
    OUString rightFooter;
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue("RightPageFooterContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xFooterContent->getRightText();
        rightFooter = xText->getString();
    }
    catch( uno::Exception& )
    {
    }

    return rightFooter;
}

void SAL_CALL ScVbaPageSetup::setRightFooter( const OUString& rightFooter)
{
    try
    {
        uno::Reference<sheet::XHeaderFooterContent> xFooterContent( mxPageProps->getPropertyValue("RightPageFooterContent"), uno::UNO_QUERY_THROW);
        uno::Reference< text::XText > xText = xFooterContent->getRightText();
        xText->setString( rightFooter );
        mxPageProps->setPropertyValue("RightPageFooterContent", uno::makeAny(xFooterContent) );
    }
    catch( uno::Exception& )
    {
    }
}

sal_Int32 SAL_CALL ScVbaPageSetup::getOrder()
{
    sal_Int32 order = excel::XlOrder::xlDownThenOver;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue("PrintDownFirst");
        bool bPrintDownFirst = false;
        aValue >>= bPrintDownFirst;
        if( !bPrintDownFirst )
            order = excel::XlOrder::xlOverThenDown;
    }
    catch( uno::Exception& )
    {
    }

    return order;
}

void SAL_CALL ScVbaPageSetup::setOrder(sal_Int32 order)
{
    bool bOrder = true;
    switch( order )
    {
        case excel::XlOrder::xlDownThenOver:
            break;
        case excel::XlOrder::xlOverThenDown:
            bOrder = false;
            break;
        default:
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER);
    }

    try
    {
        mxPageProps->setPropertyValue("PrintDownFirst", uno::makeAny( bOrder ));
    }
    catch (const uno::Exception&)
    {
    }
}

sal_Int32 SAL_CALL ScVbaPageSetup::getFirstPageNumber()
{
    sal_Int16 number = 0;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue("FirstPageNumber");
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

void SAL_CALL ScVbaPageSetup::setFirstPageNumber( sal_Int32 firstPageNumber)
{
    if( firstPageNumber == excel::Constants::xlAutomatic )
        firstPageNumber = 0;

    try
    {
        uno::Any aValue;
        aValue <<= static_cast<sal_Int16>(firstPageNumber);
        mxPageProps->setPropertyValue("FirstPageNumber", aValue );
    }
    catch (const uno::Exception&)
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getCenterVertically()
{
    bool centerVertically = false;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue("CenterVertically");
        aValue >>= centerVertically;
    }
    catch (const uno::Exception&)
    {
    }
    return centerVertically;
}

void SAL_CALL ScVbaPageSetup::setCenterVertically( sal_Bool centerVertically)
{
    try
    {
        mxPageProps->setPropertyValue("CenterVertically", uno::makeAny( centerVertically ));
    }
    catch (const uno::Exception&)
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getCenterHorizontally()
{
    bool centerHorizontally = false;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue("CenterHorizontally");
        aValue >>= centerHorizontally;
    }
    catch (const uno::Exception&)
    {
    }
    return centerHorizontally;
}

void SAL_CALL ScVbaPageSetup::setCenterHorizontally( sal_Bool centerHorizontally)
{
    try
    {
        mxPageProps->setPropertyValue("CenterHorizontally", uno::makeAny( centerHorizontally ));
    }
    catch (const uno::Exception&)
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getPrintHeadings()
{
    bool printHeadings = false;
    try
    {
        uno::Any aValue = mxPageProps->getPropertyValue("PrintHeaders");
        aValue >>= printHeadings;
    }
    catch (const uno::Exception&)
    {
    }
    return printHeadings;
}

void SAL_CALL ScVbaPageSetup::setPrintHeadings( sal_Bool printHeadings)
{
    try
    {
        mxPageProps->setPropertyValue("PrintHeaders", uno::makeAny( printHeadings ));
    }
    catch( uno::Exception& )
    {
    }
}

sal_Bool SAL_CALL ScVbaPageSetup::getPrintGridlines()
{
    return false;
}

void SAL_CALL ScVbaPageSetup::setPrintGridlines( sal_Bool /*_printgridlines*/ )
{
}

OUString SAL_CALL ScVbaPageSetup::getPrintTitleRows()
{
    return OUString();
}
void SAL_CALL ScVbaPageSetup::setPrintTitleRows( const OUString& /*_printtitlerows*/ )
{
}
OUString SAL_CALL ScVbaPageSetup::getPrintTitleColumns()
{
    return OUString();
}

void SAL_CALL ScVbaPageSetup::setPrintTitleColumns( const OUString& /*_printtitlecolumns*/ )
{
}

sal_Int32 SAL_CALL ScVbaPageSetup::getPaperSize()
{
    awt::Size aSize; // current papersize
    mxPageProps->getPropertyValue( "Size" ) >>= aSize;
    if ( mbIsLandscape )
        ::std::swap( aSize.Width, aSize.Height );

    sal_Int32 nPaperSizeIndex = msfilter::util::PaperSizeConv::getMSPaperSizeIndex( aSize );
    if ( nPaperSizeIndex == 0 )
        nPaperSizeIndex = excel::XlPaperSize::xlPaperUser;
    return nPaperSizeIndex;
}

void SAL_CALL ScVbaPageSetup::setPaperSize( sal_Int32 papersize )
{
    if ( papersize != excel::XlPaperSize::xlPaperUser )
    {
        awt::Size aPaperSize;
        const msfilter::util::ApiPaperSize& rConvertedSize = msfilter::util::PaperSizeConv::getApiSizeForMSPaperSizeIndex( papersize );
        aPaperSize.Height = rConvertedSize.mnHeight;
        aPaperSize.Width = rConvertedSize.mnWidth;
        if ( mbIsLandscape )
            ::std::swap( aPaperSize.Width, aPaperSize.Height );
        mxPageProps->setPropertyValue( "Size", uno::makeAny( aPaperSize ) );
    }
}

OUString
ScVbaPageSetup::getServiceImplName()
{
    return "ScVbaPageSetup";
}

uno::Sequence< OUString >
ScVbaPageSetup::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.PageSetup"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

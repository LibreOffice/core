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

#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <ooo/vba/excel/XlChartType.hpp>

#include "vbachartobjects.hxx"
#include "vbachartobject.hxx"
#include <docsh.hxx>
#include <cellsuno.hxx>

#include <string_view>
#include <utility>
#include <vector>
#include <basic/sberrors.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

namespace {

class ChartObjectEnumerationImpl : public EnumerationHelperImpl
{
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier;

public:
    /// @throws uno::RuntimeException
    ChartObjectEnumerationImpl( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration, uno::Reference< drawing::XDrawPageSupplier >  _xDrawPageSupplier, const uno::Reference< XHelperInterface >& _xParent ) : EnumerationHelperImpl( _xParent, xContext, xEnumeration ), xDrawPageSupplier(std::move( _xDrawPageSupplier )) {}
    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        uno::Any ret;

        try
        {
            uno::Reference< table::XTableChart > xTableChart( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
            // parent Object is sheet
            ret <<= uno::Reference< excel::XChartObject > ( new ScVbaChartObject(  m_xParent, m_xContext, xTableChart, xDrawPageSupplier ) );
        }
        catch (const lang::WrappedTargetException&)
        {
            throw;
        }
        catch (const container::NoSuchElementException&)
        {
            throw;
        }
        catch (const uno::RuntimeException&)
        {
            throw;
        }
        catch (const uno::Exception&)
        {
            css::uno::Any anyEx(cppu::getCaughtException());
            throw lang::WrappedTargetException(
                    u"Error creating ScVbaChartObject!"_ustr,
                    getXWeak(),
                    anyEx );
        }
        return ret;
    }
};

}

ScVbaChartObjects::ScVbaChartObjects( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableCharts >& _xTableCharts, uno::Reference< drawing::XDrawPageSupplier >  _xDrawPageSupplier ) : ChartObjects_BASE(_xParent, _xContext, css::uno::Reference< css::container::XIndexAccess >( _xTableCharts, css::uno::UNO_QUERY ) ), xTableCharts( _xTableCharts ) , xDrawPageSupplier(std::move( _xDrawPageSupplier ))
{

}

void
ScVbaChartObjects::removeByName(const OUString& _sChartName)
{
    xTableCharts->removeByName( _sChartName );
}

uno::Sequence< OUString >
ScVbaChartObjects::getChartObjectNames() const
{
    uno::Sequence< OUString > sChartNames;
    try
    {
        // c++ hackery
        uno::Reference< uno::XInterface > xIf( xDrawPageSupplier, uno::UNO_QUERY_THROW );
        ScCellRangesBase* pUno = dynamic_cast< ScCellRangesBase* >( xIf.get() );
        ScDocShell* pDocShell = nullptr;
        if ( !pUno )
            throw uno::RuntimeException(u"Failed to obtain the impl class from the drawpage"_ustr );
        pDocShell = pUno->GetDocShell();
        if ( !pDocShell )
            throw uno::RuntimeException(u"Failed to obtain the docshell implclass"_ustr );

        uno::Reference< sheet::XSpreadsheets > xSpreadsheets = pDocShell->GetModel()->getSheets();
        std::vector< OUString > aChartNamesVector;

        const uno::Sequence< OUString > sSheetNames = xSpreadsheets->getElementNames();
        for (const auto& rSheetName : sSheetNames)
        {
            uno::Reference< table::XTableChartsSupplier > xLocTableChartsSupplier( xSpreadsheets->getByName(rSheetName), uno::UNO_QUERY_THROW );
            const uno::Sequence< OUString > scurchartnames = xLocTableChartsSupplier->getCharts()->getElementNames();
            aChartNamesVector.insert( aChartNamesVector.end(), scurchartnames.begin(), scurchartnames.end() );
        }
        sChartNames = comphelper::containerToSequence( aChartNamesVector );
    }
    catch (uno::Exception& )
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_BASIC_METHOD_FAILED), OUString() );
    }
    return sChartNames;
}

// XChartObjects
uno::Any SAL_CALL
ScVbaChartObjects::Add( double _nX, double _nY, double _nWidth, double _nHeight )
{
    try
    {
        uno::Sequence< table::CellRangeAddress > aCellRangeAddress( 1 );
        awt::Rectangle aRectangle;
        aRectangle.X =  Millimeter::getInHundredthsOfOneMillimeter(_nX);
        aRectangle.Y = Millimeter::getInHundredthsOfOneMillimeter(_nY);
        aRectangle.Width = Millimeter::getInHundredthsOfOneMillimeter(_nWidth);
        aRectangle.Height = Millimeter::getInHundredthsOfOneMillimeter(_nHeight);
        // Note the space at the end of the stem ("Chart "). In ChartSheets only "Chart" is the stem
        OUString sPersistChartName = ContainerUtilities::getUniqueName( getChartObjectNames(), u"Chart "_ustr , std::u16string_view(), 1);
        xTableCharts->addNewByName(sPersistChartName, aRectangle, aCellRangeAddress, true, false );
        uno::Reference< excel::XChartObject > xChartObject( getItemByStringIndex( sPersistChartName ), uno::UNO_QUERY_THROW );
        xChartObject->getChart()->setChartType(excel::XlChartType::xlColumnClustered);
        return uno::Any( xChartObject );
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("sc");
    }
    return aNULL();
}
void SAL_CALL ScVbaChartObjects::Delete(  )
{
    const uno::Sequence< OUString > sChartNames = xTableCharts->getElementNames();
    for (const auto& rChartName : sChartNames)
        removeByName(rChartName);
}

// XEnumerationAccess

uno::Reference< container::XEnumeration >
ScVbaChartObjects::createEnumeration()
{
    css::uno::Reference< container::XEnumerationAccess > xEnumAccess( xTableCharts, uno::UNO_QUERY_THROW );
    return new ChartObjectEnumerationImpl( mxContext, xEnumAccess->createEnumeration(), xDrawPageSupplier, getParent() /* sheet */);
}

// XElementAccess

uno::Type
ScVbaChartObjects::getElementType()
{
    return cppu::UnoType<excel::XChartObject>::get();
}

// ScVbaCollectionBaseImpl
uno::Any
ScVbaChartObjects::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< table::XTableChart > xTableChart( aSource, uno::UNO_QUERY_THROW );
    // correct parent object is sheet
    return uno::Any( uno::Reference< excel::XChartObject > ( new ScVbaChartObject( getParent(), mxContext, xTableChart, xDrawPageSupplier ) ) );
}

OUString
ScVbaChartObjects::getServiceImplName()
{
    return u"ScVbaChartObjects"_ustr;
}

css::uno::Sequence<OUString>
ScVbaChartObjects::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.excel.ChartObjects"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

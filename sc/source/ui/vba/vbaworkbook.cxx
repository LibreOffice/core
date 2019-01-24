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

#include <vbahelper/helperdecl.hxx>
#include <tools/urlobj.hxx>
#include <boost/optional.hpp>

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XlFileFormat.hpp>
#include <ooo/vba/excel/XApplication.hpp>

#include <scextopt.hxx>
#include "service.hxx"
#include "vbaworksheet.hxx"
#include "vbaworksheets.hxx"
#include "vbaworkbook.hxx"
#include "vbawindows.hxx"
#include "vbastyles.hxx"
#include "excelvbahelper.hxx"
#include "vbapalette.hxx"
#include <osl/file.hxx>
#include "vbanames.hxx"
#include <nameuno.hxx>
#include <docoptio.hxx>
#include <unonames.hxx>
#include <docsh.hxx>

// Much of the impl. for the equivalent UNO module is
// sc/source/ui/unoobj/docuno.cxx, viewuno.cxx

using namespace ::ooo::vba;
using namespace ::com::sun::star;

uno::Sequence< sal_Int32 > ScVbaWorkbook::ColorData;

void ScVbaWorkbook::initColorData( const uno::Sequence< sal_Int32 >& sColors )
{
        const sal_Int32* pSource = sColors.getConstArray();
        sal_Int32* pDest = ColorData.getArray();
        const sal_Int32* pEnd = pSource + sColors.getLength();
        for ( ; pSource != pEnd; ++pSource, ++pDest )
            *pDest = *pSource;
}

void SAL_CALL
ScVbaWorkbook::ResetColors(  )
{
        uno::Reference< container::XIndexAccess > xIndexAccess( ScVbaPalette::getDefaultPalette(), uno::UNO_QUERY_THROW );
        sal_Int32 nLen = xIndexAccess->getCount();
        ColorData.realloc( nLen );

        uno::Sequence< sal_Int32 > dDefaultColors( nLen );
        sal_Int32* pDest = dDefaultColors.getArray();
        for ( sal_Int32 index=0; index < nLen; ++pDest, ++index )
            xIndexAccess->getByIndex( index )  >>= *pDest;
        initColorData( dDefaultColors );
}

::uno::Any SAL_CALL
ScVbaWorkbook::Colors( const ::uno::Any& Index )
{
    uno::Any aRet;
    if ( Index.hasValue() )
    {
        sal_Int32 nIndex = 0;
        Index >>= nIndex;
        aRet <<= XLRGBToOORGB( ColorData[ --nIndex ] );
    }
    else
        aRet <<= ColorData;
    return aRet;
}

bool ScVbaWorkbook::setFilterPropsFromFormat( sal_Int32 nFormat, uno::Sequence< beans::PropertyValue >& rProps )
{
    bool bRes = false;
    for ( sal_Int32 index = 0; index < rProps.getLength(); ++index )
    {
        if ( rProps[ index ].Name == "FilterName" )
        {
            switch( nFormat )
            {
                case excel::XlFileFormat::xlCSV:
                    rProps[ index ].Value <<= OUString(SC_TEXT_CSV_FILTER_NAME);
                    break;
                case excel::XlFileFormat::xlDBF4:
                    rProps[ index ].Value <<= OUString("DBF");
                    break;
                case excel::XlFileFormat::xlDIF:
                    rProps[ index ].Value <<= OUString("DIF");
                    break;
                case excel::XlFileFormat::xlWK3:
                    rProps[ index ].Value <<= OUString("Lotus");
                    break;
                case excel::XlFileFormat::xlExcel4Workbook:
                    rProps[ index ].Value <<= OUString("MS Excel 4.0");
                    break;
                case excel::XlFileFormat::xlExcel5:
                    rProps[ index ].Value <<= OUString("MS Excel 5.0/95");
                    break;
                case excel::XlFileFormat::xlHtml:
                    rProps[ index ].Value <<= OUString("HTML (StarCalc)");
                    break;
                case excel::XlFileFormat::xlExcel9795:
                default:
                    rProps[ index ].Value <<= OUString("MS Excel 97");
                    break;
            }
            bRes = true;
            break;
        }
    }
    return bRes;
}

::sal_Int32 SAL_CALL
ScVbaWorkbook::getFileFormat(  )
{
        sal_Int32 aFileFormat = 0;
        OUString aFilterName;
        uno::Sequence< beans::PropertyValue > aArgs = getModel()->getArgs();

        // #FIXME - seems suspect should we not walk through the properties
        // to find the FilterName
        if ( aArgs[0].Name == "FilterName" ) {
            aArgs[0].Value >>= aFilterName;
        } else {
           aArgs[1].Value >>= aFilterName;
        }

        if (aFilterName == SC_TEXT_CSV_FILTER_NAME) {
            aFileFormat = excel::XlFileFormat::xlCSV; //xlFileFormat.
        }

        if ( aFilterName == "DBF" ) {
            aFileFormat = excel::XlFileFormat::xlDBF4;
        }

        if ( aFilterName == "DIF" ) {
            aFileFormat = excel::XlFileFormat::xlDIF;
        }

        if ( aFilterName == "Lotus" ) {
            aFileFormat = excel::XlFileFormat::xlWK3;
        }

        if ( aFilterName == "MS Excel 4.0" ) {
            aFileFormat = excel::XlFileFormat::xlExcel4Workbook;
        }

        if ( aFilterName == "MS Excel 5.0/95" ) {
            aFileFormat = excel::XlFileFormat::xlExcel5;
        }

        if ( aFilterName == "MS Excel 97" ) {
            aFileFormat = excel::XlFileFormat::xlExcel9795;
        }

        if (aFilterName == "HTML (StarCalc)") {
            aFileFormat = excel::XlFileFormat::xlHtml;
        }

        if ( aFilterName == "calc_StarOffice_XML_Calc_Template" ) {
            aFileFormat = excel::XlFileFormat::xlTemplate;
        }

        if (aFilterName == "StarOffice XML (Calc)") {
            aFileFormat = excel::XlFileFormat::xlWorkbookNormal;
        }
        if ( aFilterName == "calc8" ) {
            aFileFormat = excel::XlFileFormat::xlWorkbookNormal;
        }

        return aFileFormat;
}

void
ScVbaWorkbook::init()
{
    if ( !ColorData.getLength() )
        ResetColors();
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY );
    if ( xModel.is() )
        excel::getDocShell( xModel )->RegisterAutomationWorkbookObject( this );
}

ScVbaWorkbook::ScVbaWorkbook(   const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::frame::XModel > const & xModel ) : ScVbaWorkbook_BASE( xParent, xContext, xModel )
{
    init();
}

ScVbaWorkbook::ScVbaWorkbook( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext ) : ScVbaWorkbook_BASE( args, xContext )
{
    init();
}

uno::Reference< excel::XWorksheet >
ScVbaWorkbook::getActiveSheet()
{
    uno::Reference< frame::XModel > xModel( getCurrentExcelDoc( mxContext ), uno::UNO_SET_THROW );
    uno::Reference< sheet::XSpreadsheetView > xView( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xView->getActiveSheet(), uno::UNO_SET_THROW );
    // #162503# return the original sheet module wrapper object, instead of a new instance
    uno::Reference< excel::XWorksheet > xWorksheet( excel::getUnoSheetModuleObj( xSheet ), uno::UNO_QUERY );
    if( xWorksheet.is() ) return xWorksheet;
    // #i116936# excel::getUnoSheetModuleObj() may return null in documents without global VBA mode enabled
    return new ScVbaWorksheet( this, mxContext, xSheet, xModel );
}

uno::Any SAL_CALL
ScVbaWorkbook::Sheets( const uno::Any& aIndex )
{
    return Worksheets( aIndex );
}

uno::Any SAL_CALL
ScVbaWorkbook::Worksheets( const uno::Any& aIndex )
{
    uno::Reference< frame::XModel > xModel( getModel() );
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY_THROW );
    uno::Reference<container::XIndexAccess > xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xWorkSheets(  new ScVbaWorksheets( this, mxContext, xSheets, xModel ) );
    if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
    {
        return uno::Any( xWorkSheets );
    }
    // pass on to collection
    return xWorkSheets->Item( aIndex, uno::Any() );
}
uno::Any SAL_CALL
ScVbaWorkbook::Windows( const uno::Any& aIndex )
{

    uno::Reference< excel::XWindows >  xWindows( new ScVbaWindows( getParent(), mxContext ) );
    if ( aIndex.getValueTypeClass() == uno::TypeClass_VOID )
        return uno::Any( xWindows );
    return xWindows->Item( aIndex, uno::Any() );
}

void SAL_CALL
ScVbaWorkbook::Activate()
{
    VbaDocumentBase::Activate();
}

void
ScVbaWorkbook::Protect( const uno::Any &aPassword )
{
    VbaDocumentBase::Protect( aPassword );
}

sal_Bool
ScVbaWorkbook::getProtectStructure()
{
    uno::Reference< util::XProtectable > xProt( getModel(), uno::UNO_QUERY_THROW );
    return xProt->isProtected();
}

sal_Bool SAL_CALL ScVbaWorkbook::getPrecisionAsDisplayed()
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument& rDoc = excel::getDocShell( xModel )->GetDocument();
    return rDoc.GetDocOptions().IsCalcAsShown();
}

void SAL_CALL ScVbaWorkbook::setPrecisionAsDisplayed( sal_Bool _precisionAsDisplayed )
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument& rDoc = excel::getDocShell( xModel )->GetDocument();
    ScDocOptions aOpt = rDoc.GetDocOptions();
    aOpt.SetCalcAsShown( _precisionAsDisplayed );
    rDoc.SetDocOptions( aOpt );
}

OUString SAL_CALL ScVbaWorkbook::getAuthor()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS( getModel(), uno::UNO_QUERY );
    if (!xDPS.is())
        return OUString("?");
    uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();
    return xDocProps->getAuthor();
}

void SAL_CALL ScVbaWorkbook::setAuthor( const OUString& _author )
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS( getModel(), uno::UNO_QUERY );
    if (!xDPS.is())
        return;
    uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();
    xDocProps->setAuthor( _author );
}

void
ScVbaWorkbook::SaveCopyAs( const OUString& sFileName )
{
    OUString aURL;
    osl::FileBase::getFileURLFromSystemPath( sFileName, aURL );
    uno::Reference< frame::XStorable > xStor( getModel(), uno::UNO_QUERY_THROW );
    uno::Sequence<  beans::PropertyValue > storeProps(1);
    storeProps[0].Name = "FilterName";
    storeProps[0].Value <<= OUString( "MS Excel 97" );
    xStor->storeToURL( aURL, storeProps );
}

void SAL_CALL
ScVbaWorkbook::SaveAs( const uno::Any& FileName, const uno::Any& FileFormat, const uno::Any& /*Password*/, const uno::Any& /*WriteResPassword*/, const uno::Any& /*ReadOnlyRecommended*/, const uno::Any& /*CreateBackup*/, const uno::Any& /*AccessMode*/, const uno::Any& /*ConflictResolution*/, const uno::Any& /*AddToMru*/, const uno::Any& /*TextCodepage*/, const uno::Any& /*TextVisualLayout*/, const uno::Any& /*Local*/ )
{
    OUString sFileName;
    FileName >>= sFileName;
    OUString sURL;
    osl::FileBase::getFileURLFromSystemPath( sFileName, sURL );
    // detect if there is no path then we need
    // to use the current folder
    INetURLObject aURL( sURL );
    sURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
    if( sURL.isEmpty() )
    {
        // need to add cur dir ( of this workbook ) or else the 'Work' dir
        sURL = getModel()->getURL();

        if ( sURL.isEmpty() )
        {
            // not path available from 'this' document
            // need to add the 'document'/work directory then
            uno::Reference< excel::XApplication > xApplication ( Application(),uno::UNO_QUERY_THROW );
            OUString sWorkPath = xApplication->getDefaultFilePath();
            OUString sWorkURL;
            osl::FileBase::getFileURLFromSystemPath( sWorkPath, sWorkURL );
            aURL.SetURL( sWorkURL );
        }
        else
        {
            aURL.SetURL( sURL );
            aURL.Append( sFileName );
        }
        sURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );

    }

    sal_Int32 nFileFormat = excel::XlFileFormat::xlExcel9795;
    FileFormat >>= nFileFormat;

    uno::Sequence<  beans::PropertyValue > storeProps(1);
    storeProps[0].Name = "FilterName" ;

    setFilterPropsFromFormat( nFileFormat, storeProps );

    uno::Reference< frame::XStorable > xStor( getModel(), uno::UNO_QUERY_THROW );
    xStor->storeAsURL( sURL, storeProps );
}

css::uno::Any SAL_CALL
ScVbaWorkbook::Styles( const uno::Any& Item )
{
    // quick look and Styles object doesn't seem to have a valid parent
    // or a least the object browser just shows an object that has no
    // variables ( therefore... leave as NULL for now )
    uno::Reference< XCollection > dStyles = new ScVbaStyles( uno::Reference< XHelperInterface >(), mxContext, getModel() );
    if ( Item.hasValue() )
        return dStyles->Item( Item, uno::Any() );
    return uno::makeAny( dStyles );
}

uno::Any SAL_CALL
ScVbaWorkbook::Names( const uno::Any& aIndex )
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_SET_THROW );
    uno::Reference< beans::XPropertySet > xProps( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XNamedRanges > xNamedRanges(  xProps->getPropertyValue("NamedRanges"), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xNames( new ScVbaNames( this, mxContext, xNamedRanges, xModel ) );
    if ( aIndex.hasValue() )
        return xNames->Item( aIndex, uno::Any() );
    return uno::Any( xNames );
}

OUString
ScVbaWorkbook::getServiceImplName()
{
    return OUString("ScVbaWorkbook");
}

uno::Sequence< OUString >
ScVbaWorkbook::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.Workbook"
    };
    return aServiceNames;
}

OUString SAL_CALL
ScVbaWorkbook::getCodeName()
{
    uno::Reference< beans::XPropertySet > xModelProp( getModel(), uno::UNO_QUERY_THROW );
    return xModelProp->getPropertyValue("CodeName").get< OUString >();
}

sal_Int64
ScVbaWorkbook::getSomething(const uno::Sequence<sal_Int8 >& rId )
{
    if (rId.getLength() == 16 &&
        0 == memcmp( ScVbaWorksheet::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ))
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace workbook
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaWorkbook, sdecl::with_args<true> > const serviceImpl;
sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaWorkbook",
    "ooo.vba.excel.Workbook" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

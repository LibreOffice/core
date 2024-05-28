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

#include <memory>
#include <config_features.h>

#include <sal/macros.h>
#include <svtools/unoimap.hxx>
#include <svx/unofill.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

#include <editsrc.hxx>
#include <servuno.hxx>
#include <unonames.hxx>
#include <appluno.hxx>
#include <cellsuno.hxx>
#include <fielduno.hxx>
#include <styleuno.hxx>
#include <afmtuno.hxx>
#include <defltuno.hxx>
#include <drdefuno.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <confuno.hxx>
#include <shapeuno.hxx>
#include "cellvaluebinding.hxx"
#include "celllistsource.hxx"
#include <addruno.hxx>
#include <chart2uno.hxx>
#include <tokenuno.hxx>
#include <PivotTableDataProvider.hxx>

// Support creation of GraphicStorageHandler and EmbeddedObjectResolver
#include <svx/xmleohlp.hxx>
#include <svx/xmlgrhlp.hxx>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/document/XCodeNameQuery.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <svx/unomod.hxx>
#include <vbahelper/vbaaccesshelper.hxx>

#include <comphelper/processfactory.hxx>
#include <basic/basmgr.hxx>
#include <sfx2/app.hxx>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>

using namespace ::com::sun::star;

#if HAVE_FEATURE_SCRIPTING

static bool isInVBAMode( ScDocShell& rDocSh )
{
    uno::Reference<script::XLibraryContainer> xLibContainer = rDocSh.GetBasicContainer();
    uno::Reference<script::vba::XVBACompatibility> xVBACompat( xLibContainer, uno::UNO_QUERY );
    if ( xVBACompat.is() )
        return xVBACompat->getVBACompatibilityMode();
    return false;
}

#endif

namespace {

#if HAVE_FEATURE_SCRIPTING
class ScVbaObjectForCodeNameProvider : public ::cppu::WeakImplHelper< container::XNameAccess >
{
    uno::Any maWorkbook;
    uno::Any maCachedObject;
    ScDocShell* mpDocShell;
public:
    explicit ScVbaObjectForCodeNameProvider( ScDocShell* pDocShell ) : mpDocShell( pDocShell )
    {
        uno::Sequence< uno::Any > aArgs{
            // access the application object ( parent for workbook )
            uno::Any(ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.Application", {} )),
            uno::Any(uno::Reference(static_cast<css::sheet::XSpreadsheetDocument*>(mpDocShell->GetModel())))
        };
        maWorkbook <<= ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.excel.Workbook", aArgs );
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        SolarMutexGuard aGuard;
        maCachedObject = uno::Any(); // clear cached object

        ScDocument& rDoc = mpDocShell->GetDocument();
        // aName is generated from the stream name which can be different ( case-wise )
        // from the code name
        if( aName.equalsIgnoreAsciiCase( rDoc.GetCodeName() ) )
            maCachedObject = maWorkbook;
        else
        {
            OUString sCodeName;
            SCTAB nCount = rDoc.GetTableCount();
            for( SCTAB i = 0; i < nCount; i++ )
            {
                rDoc.GetCodeName( i, sCodeName );
                // aName is generated from the stream name which can be different ( case-wise )
                // from the code name
                if( sCodeName.equalsIgnoreAsciiCase( aName ) )
                {
                    OUString sSheetName;
                    if( rDoc.GetName( i, sSheetName ) )
                    {
                        rtl::Reference< ScModelObj > xSpreadDoc( mpDocShell->GetModel() );
                        uno::Reference<sheet::XSpreadsheets > xSheets( xSpreadDoc->getSheets(), uno::UNO_SET_THROW );
                        uno::Reference< container::XIndexAccess > xIndexAccess( xSheets, uno::UNO_QUERY_THROW );
                        uno::Reference< sheet::XSpreadsheet > xSheet( xIndexAccess->getByIndex( i ), uno::UNO_QUERY_THROW );
                        uno::Sequence< uno::Any > aArgs{ maWorkbook, uno::Any(uno::Reference< frame::XModel >(xSpreadDoc)), uno::Any(sSheetName) };
                        // use the convenience function
                        maCachedObject <<= ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.excel.Worksheet", aArgs );
                        break;
                    }
                }
            }
        }
        return maCachedObject.hasValue();

    }
    css::uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        SolarMutexGuard aGuard;
        if ( !hasByName( aName ) )
            throw css::container::NoSuchElementException();
        return maCachedObject;
    }
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        SolarMutexGuard aGuard;
        ScDocument& rDoc = mpDocShell->GetDocument();
        SCTAB nCount = rDoc.GetTableCount();
        uno::Sequence< OUString > aNames( nCount + 1 );
        auto pNames = aNames.getArray();
        SCTAB index = 0;
        OUString sCodeName;
        for( ; index < nCount; ++index )
        {
            rDoc.GetCodeName( index, sCodeName );
            pNames[ index ] = sCodeName;
        }
        pNames[ index ] = rDoc.GetCodeName();
        return aNames;
    }
    // XElemenAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override { return uno::Type(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return true; }

};

class ScVbaCodeNameProvider : public ::cppu::WeakImplHelper< document::XCodeNameQuery >
{
    ScDocShell& mrDocShell;
public:
    explicit ScVbaCodeNameProvider( ScDocShell& rDocShell ) : mrDocShell(rDocShell) {}
    // XCodeNameQuery
    OUString SAL_CALL getCodeNameForObject( const uno::Reference< uno::XInterface >& xIf ) override
    {
        SolarMutexGuard aGuard;
        OUString sCodeName;

        // need to find the page ( and index )  for this control
        uno::Reference< container::XIndexAccess > xIndex(  mrDocShell.GetModel()->getDrawPages(), uno::UNO_QUERY_THROW );
        sal_Int32 nLen = xIndex->getCount();
        bool bMatched = false;
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            try
            {
                uno::Reference< form::XFormsSupplier >  xFormSupplier( xIndex->getByIndex( index ), uno::UNO_QUERY_THROW );
                uno::Reference< container::XIndexAccess > xFormIndex( xFormSupplier->getForms(), uno::UNO_QUERY_THROW );
                // get the www-standard container
                uno::Reference< container::XIndexAccess > xFormControls( xFormIndex->getByIndex(0), uno::UNO_QUERY_THROW );
                sal_Int32 nCntrls = xFormControls->getCount();
                for( sal_Int32 cIndex = 0; cIndex < nCntrls; ++cIndex )
                {
                    uno::Reference< uno::XInterface > xControl( xFormControls->getByIndex( cIndex ), uno::UNO_QUERY_THROW );
                    bMatched = ( xControl == xIf );
                    if ( bMatched )
                    {
                        OUString sName;
                        mrDocShell.GetDocument().GetCodeName( static_cast<SCTAB>( index ), sName );
                        sCodeName = sName;
                    }
                }
            }
            catch( uno::Exception& ) {}
            if ( bMatched )
                break;
        }
        // Probably should throw here ( if !bMatched )
        return sCodeName;
    }

    OUString SAL_CALL getCodeNameForContainer( const uno::Reference<uno::XInterface>& xContainer ) override
    {
        SolarMutexGuard aGuard;
        uno::Reference<container::XIndexAccess> xIndex(mrDocShell.GetModel()->getDrawPages(), uno::UNO_QUERY_THROW);

        for (sal_Int32 i = 0, n = xIndex->getCount(); i < n; ++i)
        {
            try
            {
                uno::Reference<form::XFormsSupplier>  xFormSupplier(xIndex->getByIndex(i), uno::UNO_QUERY_THROW);
                uno::Reference<container::XIndexAccess> xFormIndex(xFormSupplier->getForms(), uno::UNO_QUERY_THROW);
                // get the www-standard container
                uno::Reference<container::XIndexAccess> xFormControls(xFormIndex->getByIndex(0), uno::UNO_QUERY_THROW);
                if (xFormControls == xContainer)
                {
                    OUString aName;
                    if (mrDocShell.GetDocument().GetCodeName(static_cast<SCTAB>(i), aName))
                        return aName;
                }
            }
            catch( uno::Exception& ) {}
        }
        return OUString();
    }
};

#endif

using Type = ScServiceProvider::Type;

struct ProvNamesId_Type
{
    OUString                pName;
    ScServiceProvider::Type nType;
};

const ProvNamesId_Type aProvNamesId[] =
{
    { u"com.sun.star.sheet.Spreadsheet"_ustr,                 Type::SHEET },
    { u"com.sun.star.text.TextField.URL"_ustr,                Type::URLFIELD },
    { u"com.sun.star.text.TextField.PageNumber"_ustr,         Type::PAGEFIELD },
    { u"com.sun.star.text.TextField.PageCount"_ustr,          Type::PAGESFIELD },
    { u"com.sun.star.text.TextField.Date"_ustr,               Type::DATEFIELD },
    { u"com.sun.star.text.TextField.Time"_ustr,               Type::TIMEFIELD },
    { u"com.sun.star.text.TextField.DateTime"_ustr,           Type::EXT_TIMEFIELD },
    { u"com.sun.star.text.TextField.DocInfo.Title"_ustr,      Type::TITLEFIELD },
    { u"com.sun.star.text.TextField.FileName"_ustr,           Type::FILEFIELD },
    { u"com.sun.star.text.TextField.SheetName"_ustr,          Type::SHEETFIELD },
    { u"com.sun.star.style.CellStyle"_ustr,                   Type::CELLSTYLE },
    { u"com.sun.star.style.PageStyle"_ustr,                   Type::PAGESTYLE },
    { u"com.sun.star.style.GraphicStyle"_ustr,                Type::GRAPHICSTYLE },
    { u"com.sun.star.sheet.TableAutoFormat"_ustr,             Type::AUTOFORMAT },
    { u"com.sun.star.sheet.TableAutoFormats"_ustr,            Type::AUTOFORMATS },
    { u"com.sun.star.sheet.SheetCellRanges"_ustr,             Type::CELLRANGES },
    { u"com.sun.star.sheet.FunctionDescriptions"_ustr,        Type::FUNCTIONDESCRIPTIONS },
    { u"com.sun.star.sheet.GlobalSheetSettings"_ustr,         Type::GLOBALSHEETSETTINGS },
    { u"com.sun.star.sheet.RecentFunctions"_ustr,             Type::RECENTFUNCTIONS },
    { u"com.sun.star.drawing.GradientTable"_ustr,             Type::GRADTAB },
    { u"com.sun.star.drawing.HatchTable"_ustr,                Type::HATCHTAB },
    { u"com.sun.star.drawing.BitmapTable"_ustr,               Type::BITMAPTAB },
    { u"com.sun.star.drawing.TransparencyGradientTable"_ustr, Type::TRGRADTAB },
    { u"com.sun.star.drawing.MarkerTable"_ustr,               Type::MARKERTAB },
    { u"com.sun.star.drawing.DashTable"_ustr,                 Type::DASHTAB },
    { u"com.sun.star.text.NumberingRules"_ustr,               Type::NUMRULES },
    { u"com.sun.star.sheet.Defaults"_ustr,                    Type::DOCDEFLTS },
    { u"com.sun.star.drawing.Defaults"_ustr,                  Type::DRAWDEFLTS },
    { u"com.sun.star.comp.SpreadsheetSettings"_ustr,          Type::DOCSPRSETT },
    { u"com.sun.star.document.Settings"_ustr,                 Type::DOCCONF },
    { u"com.sun.star.image.ImageMapRectangleObject"_ustr,     Type::IMAP_RECT },
    { u"com.sun.star.image.ImageMapCircleObject"_ustr,        Type::IMAP_CIRC },
    { u"com.sun.star.image.ImageMapPolygonObject"_ustr,       Type::IMAP_POLY },

    // Support creation of GraphicStorageHandler and EmbeddedObjectResolver
    { u"com.sun.star.document.ExportGraphicStorageHandler"_ustr,  Type::EXPORT_GRAPHIC_STORAGE_HANDLER },
    { u"com.sun.star.document.ImportGraphicStorageHandler"_ustr,  Type::IMPORT_GRAPHIC_STORAGE_HANDLER },
    { u"com.sun.star.document.ExportEmbeddedObjectResolver"_ustr, Type::EXPORT_EOR },
    { u"com.sun.star.document.ImportEmbeddedObjectResolver"_ustr, Type::IMPORT_EOR },

    { SC_SERVICENAME_VALBIND,               Type::VALBIND },
    { SC_SERVICENAME_LISTCELLBIND,          Type::LISTCELLBIND },
    { SC_SERVICENAME_LISTSOURCE,            Type::LISTSOURCE },
    { SC_SERVICENAME_CELLADDRESS,           Type::CELLADDRESS },
    { SC_SERVICENAME_RANGEADDRESS,          Type::RANGEADDRESS },

    { u"com.sun.star.sheet.DocumentSettings"_ustr,Type::SHEETDOCSET },

    { SC_SERVICENAME_CHDATAPROV,            Type::CHDATAPROV },
    { SC_SERVICENAME_CHART_PIVOTTABLE_DATAPROVIDER, Type::CHART_PIVOTTABLE_DATAPROVIDER },
    { SC_SERVICENAME_FORMULAPARS,           Type::FORMULAPARS },
    { SC_SERVICENAME_OPCODEMAPPER,          Type::OPCODEMAPPER },
    { u"ooo.vba.VBAObjectModuleObjectProvider"_ustr, Type::VBAOBJECTPROVIDER },
    { u"ooo.vba.VBACodeNameProvider"_ustr,        Type::VBACODENAMEPROVIDER },
    { u"ooo.vba.VBAGlobals"_ustr,                 Type::VBAGLOBALS },

    // case-correct versions of the service names (#i102468#)
    { u"com.sun.star.text.textfield.URL"_ustr,                Type::URLFIELD },
    { u"com.sun.star.text.textfield.PageNumber"_ustr,         Type::PAGEFIELD },
    { u"com.sun.star.text.textfield.PageCount"_ustr,          Type::PAGESFIELD },
    { u"com.sun.star.text.textfield.Date"_ustr,               Type::DATEFIELD },
    { u"com.sun.star.text.textfield.Time"_ustr,               Type::TIMEFIELD },
    { u"com.sun.star.text.textfield.DateTime"_ustr,           Type::EXT_TIMEFIELD },
    { u"com.sun.star.text.textfield.docinfo.Title"_ustr,      Type::TITLEFIELD },
    { u"com.sun.star.text.textfield.FileName"_ustr,           Type::FILEFIELD },
    { u"com.sun.star.text.textfield.SheetName"_ustr,          Type::SHEETFIELD },
    { u"ooo.vba.VBAGlobals"_ustr,                             Type::VBAGLOBALS },
};

//  old service names that were in 567 still work in createInstance,
//  in case some macro is still using them
const ProvNamesId_Type aOldNames[] =
{
    { u"stardiv.one.text.TextField.URL"_ustr,           Type::URLFIELD },
    { u"stardiv.one.text.TextField.PageNumber"_ustr,    Type::PAGEFIELD },
    { u"stardiv.one.text.TextField.PageCount"_ustr,     Type::PAGESFIELD },
    { u"stardiv.one.text.TextField.Date"_ustr,          Type::DATEFIELD },
    { u"stardiv.one.text.TextField.Time"_ustr,          Type::TIMEFIELD },
    { u"stardiv.one.text.TextField.DocumentTitle"_ustr, Type::TITLEFIELD },
    { u"stardiv.one.text.TextField.FileName"_ustr,      Type::FILEFIELD },
    { u"stardiv.one.text.TextField.SheetName"_ustr,     Type::SHEETFIELD },
    { u"stardiv.one.style.CellStyle"_ustr,              Type::CELLSTYLE },
    { u"stardiv.one.style.PageStyle"_ustr,              Type::PAGESTYLE },
};

sal_Int32 getFieldType(ScServiceProvider::Type nOldType)
{
    switch (nOldType)
    {
        case Type::URLFIELD:
            return text::textfield::Type::URL;
        case Type::PAGEFIELD:
            return text::textfield::Type::PAGE;
        case Type::PAGESFIELD:
            return text::textfield::Type::PAGES;
        case Type::DATEFIELD:
            return text::textfield::Type::DATE;
        case Type::TIMEFIELD:
            return text::textfield::Type::TIME;
        case Type::EXT_TIMEFIELD:
            return text::textfield::Type::EXTENDED_TIME;
        case Type::TITLEFIELD:
            return text::textfield::Type::DOCINFO_TITLE;
        case Type::FILEFIELD:
            return text::textfield::Type::EXTENDED_FILE;
        case Type::SHEETFIELD:
            return text::textfield::Type::TABLE;
        default:
            ;
    }

    return text::textfield::Type::URL; // default to URL for no reason whatsoever.
}

} // namespace


ScServiceProvider::Type ScServiceProvider::GetProviderType(std::u16string_view rServiceName)
{
    if (!rServiceName.empty())
    {
        for (const ProvNamesId_Type & i : aProvNamesId)
        {
            if (rServiceName == i.pName)
            {
                return i.nType;
            }
        }

        for (const ProvNamesId_Type & rOldName : aOldNames)
        {
            if (rServiceName == rOldName.pName)
            {
                OSL_FAIL("old service name used");
                return rOldName.nType;
            }
        }
    }
    return Type::INVALID;
}

uno::Reference<uno::XInterface> ScServiceProvider::MakeInstance(
                                    Type nType, ScDocShell* pDocShell )
{
    uno::Reference<uno::XInterface> xRet;

    switch (nType)
    {
        case Type::SHEET:
            //  not inserted yet - DocShell=Null
            xRet.set(static_cast<sheet::XSpreadsheet*>(new ScTableSheetObj(nullptr,0)));
            break;
        case Type::URLFIELD:
        case Type::PAGEFIELD:
        case Type::PAGESFIELD:
        case Type::DATEFIELD:
        case Type::TIMEFIELD:
        case Type::EXT_TIMEFIELD:
        case Type::TITLEFIELD:
        case Type::FILEFIELD:
        case Type::SHEETFIELD:
        {
            uno::Reference<text::XTextRange> xNullContent;
            xRet.set(static_cast<text::XTextField*>(
                new ScEditFieldObj(xNullContent, nullptr, getFieldType(nType), ESelection())));
        }   break;
        case Type::CELLSTYLE:
            xRet.set(static_cast<style::XStyle*>(new ScStyleObj( nullptr, SfxStyleFamily::Para, OUString() )));
            break;
        case Type::PAGESTYLE:
            xRet.set(static_cast<style::XStyle*>(new ScStyleObj( nullptr, SfxStyleFamily::Page, OUString() )));
            break;
        case Type::GRAPHICSTYLE:
            if (pDocShell)
            {
                pDocShell->MakeDrawLayer();
                xRet.set(static_cast<style::XStyle*>(new ScStyleObj( nullptr, SfxStyleFamily::Frame, OUString() )));
            }
            break;
        case Type::AUTOFORMAT:
            xRet.set(static_cast<container::XIndexAccess*>(new ScAutoFormatObj( SC_AFMTOBJ_INVALID )));
            break;
        case Type::AUTOFORMATS:
            xRet.set(static_cast<container::XIndexAccess*>(new ScAutoFormatsObj()));
            break;
        case Type::CELLRANGES:
            //  isn't inserted, rather filled
            //  -> DocShell must be set, but empty ranges
            if (pDocShell)
                xRet.set(static_cast<sheet::XSheetCellRanges*>(new ScCellRangesObj( pDocShell, ScRangeList() )));
            break;
        case Type::FUNCTIONDESCRIPTIONS:
            xRet.set(static_cast<sheet::XFunctionDescriptions*>(new ScFunctionListObj()));
            break;
        case Type::GLOBALSHEETSETTINGS:
            xRet.set(static_cast<sheet::XGlobalSheetSettings*>(new ScSpreadsheetSettings()));
            break;
        case Type::RECENTFUNCTIONS:
            xRet.set(static_cast<sheet::XRecentFunctions*>(new ScRecentFunctionsObj()));
            break;
        case Type::DOCDEFLTS:
            if (pDocShell)
                xRet.set(static_cast<beans::XPropertySet*>(new ScDocDefaultsObj( pDocShell )));
            break;
        case Type::DRAWDEFLTS:
            if (pDocShell)
                xRet.set(static_cast<beans::XPropertySet*>(new ScDrawDefaultsObj( pDocShell )));
            break;

        //  Drawing layer tables are not in SvxUnoDrawMSFactory,
        //  because SvxUnoDrawMSFactory doesn't have a SdrModel pointer.
        //  Drawing layer is always allocated if not there (MakeDrawLayer).

        case Type::GRADTAB:
            if (pDocShell)
                xRet.set(SvxUnoGradientTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case Type::HATCHTAB:
            if (pDocShell)
                xRet.set(SvxUnoHatchTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case Type::BITMAPTAB:
            if (pDocShell)
                xRet.set(SvxUnoBitmapTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case Type::TRGRADTAB:
            if (pDocShell)
                xRet.set(SvxUnoTransGradientTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case Type::MARKERTAB:
            if (pDocShell)
                xRet.set(SvxUnoMarkerTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case Type::DASHTAB:
            if (pDocShell)
                xRet.set(SvxUnoDashTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case Type::NUMRULES:
            if (pDocShell)
                xRet.set(SvxCreateNumRule( pDocShell->MakeDrawLayer() ));
            break;
        case Type::DOCSPRSETT:
        case Type::SHEETDOCSET:
        case Type::DOCCONF:
            if (pDocShell)
                xRet.set(static_cast<beans::XPropertySet*>(new ScDocumentConfiguration(pDocShell)));
            break;
        case Type::IMAP_RECT:
            xRet.set(SvUnoImageMapRectangleObject_createInstance( ScShapeObj::GetSupportedMacroItems() ));
            break;
        case Type::IMAP_CIRC:
            xRet.set(SvUnoImageMapCircleObject_createInstance( ScShapeObj::GetSupportedMacroItems() ));
            break;
        case Type::IMAP_POLY:
            xRet.set(SvUnoImageMapPolygonObject_createInstance( ScShapeObj::GetSupportedMacroItems() ));
            break;

        // Support creation of GraphicStorageHandler and EmbeddedObjectResolver
        case Type::EXPORT_GRAPHIC_STORAGE_HANDLER:
            xRet.set(getXWeak(new SvXMLGraphicHelper( SvXMLGraphicHelperMode::Write )));
            break;
        case Type::IMPORT_GRAPHIC_STORAGE_HANDLER:
            xRet.set(getXWeak(new SvXMLGraphicHelper( SvXMLGraphicHelperMode::Read )));
            break;
        case Type::EXPORT_EOR:
            if (pDocShell)
                xRet.set(getXWeak(new SvXMLEmbeddedObjectHelper( *pDocShell, SvXMLEmbeddedObjectHelperMode::Write )));
            break;
        case Type::IMPORT_EOR:
            if (pDocShell)
                xRet.set(getXWeak(new SvXMLEmbeddedObjectHelper( *pDocShell, SvXMLEmbeddedObjectHelperMode::Read )));
            break;
        case Type::VALBIND:
        case Type::LISTCELLBIND:
            if (pDocShell)
            {
                bool bListPos = ( nType == Type::LISTCELLBIND );
                uno::Reference<sheet::XSpreadsheetDocument> xDoc( pDocShell->GetBaseModel(), uno::UNO_QUERY );
                xRet.set(*new calc::OCellValueBinding( xDoc, bListPos ));
            }
            break;
        case Type::LISTSOURCE:
            if (pDocShell)
            {
                uno::Reference<sheet::XSpreadsheetDocument> xDoc( pDocShell->GetBaseModel(), uno::UNO_QUERY );
                xRet.set(*new calc::OCellListSource( xDoc ));
            }
            break;
        case Type::CELLADDRESS:
        case Type::RANGEADDRESS:
            if (pDocShell)
            {
                bool bIsRange = ( nType == Type::RANGEADDRESS );
                xRet.set(*new ScAddressConversionObj( pDocShell, bIsRange ));
            }
            break;
        case Type::CHDATAPROV:
            if (pDocShell)
                xRet = *new ScChart2DataProvider( &pDocShell->GetDocument() );
            break;
        case Type::CHART_PIVOTTABLE_DATAPROVIDER:
            if (pDocShell)
                xRet = *new sc::PivotTableDataProvider(pDocShell->GetDocument());
            break;
        case Type::FORMULAPARS:
            if (pDocShell)
                xRet.set(static_cast<sheet::XFormulaParser*>(new ScFormulaParserObj( pDocShell )));
            break;
        case Type::OPCODEMAPPER:
            if (pDocShell)
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                ScAddress aAddress;
                ScCompiler* pComp = new ScCompiler(rDoc, aAddress, rDoc.GetGrammar());
                xRet.set(static_cast<sheet::XFormulaOpCodeMapper*>(new ScFormulaOpCodeMapperObj(::std::unique_ptr<formula::FormulaCompiler> (pComp))));
                break;
            }
            break;
#if HAVE_FEATURE_SCRIPTING
        case Type::VBAOBJECTPROVIDER:
            if (pDocShell && pDocShell->GetDocument().IsInVBAMode())
            {
                xRet.set(static_cast<container::XNameAccess*>(new ScVbaObjectForCodeNameProvider( pDocShell )));
            }
            break;
        case Type::VBACODENAMEPROVIDER:
            if ( pDocShell && isInVBAMode( *pDocShell ) )
            {
                xRet.set(static_cast<document::XCodeNameQuery*>(new ScVbaCodeNameProvider(*pDocShell)));
            }
            break;
        case Type::VBAGLOBALS:
            if (pDocShell)
            {
                uno::Any aGlobs;
                if ( !pDocShell->GetBasicManager()->GetGlobalUNOConstant( u"VBAGlobals"_ustr, aGlobs ) )
                {
                    uno::Sequence< uno::Any > aArgs{ uno::Any(uno::Reference(static_cast<css::sheet::XSpreadsheetDocument*>(pDocShell->GetModel()))) };
                    xRet = ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( u"ooo.vba.excel.Globals"_ustr, aArgs );
                    pDocShell->GetBasicManager()->SetGlobalUNOConstant( u"VBAGlobals"_ustr, uno::Any( xRet ) );
                    BasicManager* pAppMgr = SfxApplication::GetBasicManager();
                    if ( pAppMgr )
                        pAppMgr->SetGlobalUNOConstant( u"ThisExcelDoc"_ustr, aArgs[ 0 ] );

                    // create the VBA document event processor
                    uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents(
                        ::ooo::vba::createVBAUnoAPIServiceWithArgs( pDocShell, "com.sun.star.script.vba.VBASpreadsheetEventProcessor", aArgs ), uno::UNO_QUERY );
                    pDocShell->GetDocument().SetVbaEventProcessor( xVbaEvents );
                }
            }
            break;
#endif
        default:
            break;
    }

    return xRet;
}

uno::Sequence<OUString> ScServiceProvider::GetAllServiceNames()
{
    const sal_uInt16 nEntries = SAL_N_ELEMENTS(aProvNamesId);
    uno::Sequence<OUString> aRet(nEntries);
    OUString* pArray = aRet.getArray();
    for (sal_uInt16 i = 0; i < nEntries; i++)
    {
        pArray[i] = aProvNamesId[i].pName;
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

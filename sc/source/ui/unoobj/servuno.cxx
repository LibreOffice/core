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

#include <config_features.h>

#include <sal/macros.h>
#include <svtools/unoimap.hxx>
#include <svx/unofill.hxx>
#include <editeng/unonrule.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

#include "servuno.hxx"
#include "unonames.hxx"
#include "cellsuno.hxx"
#include "fielduno.hxx"
#include "styleuno.hxx"
#include "afmtuno.hxx"
#include "defltuno.hxx"
#include "drdefuno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "confuno.hxx"
#include "shapeuno.hxx"
#include "cellvaluebinding.hxx"
#include "celllistsource.hxx"
#include "addruno.hxx"
#include "chart2uno.hxx"
#include "tokenuno.hxx"

// Support creation of GraphicObjectResolver and EmbeddedObjectResolver
#include <svx/xmleohlp.hxx>
#include <svx/xmlgrhlp.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/document/XCodeNameQuery.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <svx/unomod.hxx>
#include <vbahelper/vbaaccesshelper.hxx>

#include <comphelper/processfactory.hxx>
#include <basic/basmgr.hxx>
#include <sfx2/app.hxx>

#include <cppuhelper/component_context.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>

using namespace ::com::sun::star;

bool isInVBAMode( ScDocShell& rDocSh )
{
    uno::Reference<script::XLibraryContainer> xLibContainer = rDocSh.GetBasicContainer();
    uno::Reference<script::vba::XVBACompatibility> xVBACompat( xLibContainer, uno::UNO_QUERY );
    if ( xVBACompat.is() )
        return xVBACompat->getVBACompatibilityMode();
    return false;
}

class ScVbaObjectForCodeNameProvider : public ::cppu::WeakImplHelper< container::XNameAccess >
{
    uno::Any maWorkbook;
    uno::Any maCachedObject;
    ScDocShell* mpDocShell;
public:
    explicit ScVbaObjectForCodeNameProvider( ScDocShell* pDocShell ) : mpDocShell( pDocShell )
    {
        uno::Sequence< uno::Any > aArgs(2);
        // access the application object ( parent for workbook )
        aArgs[0] = uno::Any( ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.Application", uno::Sequence< uno::Any >() ) );
        aArgs[1] = uno::Any( mpDocShell->GetModel() );
        maWorkbook <<= ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.excel.Workbook", aArgs );
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception ) override
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
                        uno::Reference< frame::XModel > xModel( mpDocShell->GetModel() );
                        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY_THROW );
                        uno::Reference<sheet::XSpreadsheets > xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
                        uno::Reference< container::XIndexAccess > xIndexAccess( xSheets, uno::UNO_QUERY_THROW );
                        uno::Reference< sheet::XSpreadsheet > xSheet( xIndexAccess->getByIndex( i ), uno::UNO_QUERY_THROW );
                        uno::Sequence< uno::Any > aArgs(3);
                        aArgs[0] = maWorkbook;
                        aArgs[1] = uno::Any( xModel );
                        aArgs[2] = uno::Any( OUString( sSheetName ) );
                        // use the convience function
                        maCachedObject <<= ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.excel.Worksheet", aArgs );
                        break;
                    }
                }
            }
        }
        return maCachedObject.hasValue();

    }
    css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {
        SolarMutexGuard aGuard;
        OSL_TRACE("ScVbaObjectForCodeNameProvider::getByName( %s )",
            OUStringToOString( aName, RTL_TEXTENCODING_UTF8 ).getStr() );
        if ( !hasByName( aName ) )
            throw css::container::NoSuchElementException();
        return maCachedObject;
    }
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException, std::exception) override
    {
        SolarMutexGuard aGuard;
        ScDocument& rDoc = mpDocShell->GetDocument();
        SCTAB nCount = rDoc.GetTableCount();
        uno::Sequence< OUString > aNames( nCount + 1 );
        SCTAB index = 0;
        OUString sCodeName;
        for( ; index < nCount; ++index )
        {
            rDoc.GetCodeName( index, sCodeName );
            aNames[ index ] = sCodeName;
        }
        aNames[ index ] = rDoc.GetCodeName();
        return aNames;
    }
    // XElemenAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override { return uno::Type(); }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception ) override { return sal_True; }

};

class ScVbaCodeNameProvider : public ::cppu::WeakImplHelper< document::XCodeNameQuery >
{
    ScDocShell& mrDocShell;
public:
    explicit ScVbaCodeNameProvider( ScDocShell& rDocShell ) : mrDocShell(rDocShell) {}
    // XCodeNameQuery
    OUString SAL_CALL getCodeNameForObject( const uno::Reference< uno::XInterface >& xIf ) throw( uno::RuntimeException, std::exception ) override
    {
        SolarMutexGuard aGuard;
        OUString sCodeName;

        // need to find the page ( and index )  for this control
        uno::Reference< drawing::XDrawPagesSupplier > xSupplier( mrDocShell.GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndex( xSupplier->getDrawPages(), uno::UNO_QUERY_THROW );
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

    OUString SAL_CALL getCodeNameForContainer( const uno::Reference<uno::XInterface>& xContainer )
            throw( uno::RuntimeException, std::exception ) override
    {
        SolarMutexGuard aGuard;
        uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mrDocShell.GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndex(xSupplier->getDrawPages(), uno::UNO_QUERY_THROW);

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

namespace {

using Type = ScServiceProvider::Type;

struct ProvNamesId_Type
{
    const char *            pName;
    ScServiceProvider::Type nType;
};

const ProvNamesId_Type aProvNamesId[] =
{
    { "com.sun.star.sheet.Spreadsheet",                 Type::SHEET },
    { "com.sun.star.text.TextField.URL",                Type::URLFIELD },
    { "com.sun.star.text.TextField.PageNumber",         Type::PAGEFIELD },
    { "com.sun.star.text.TextField.PageCount",          Type::PAGESFIELD },
    { "com.sun.star.text.TextField.Date",               Type::DATEFIELD },
    { "com.sun.star.text.TextField.Time",               Type::TIMEFIELD },
    { "com.sun.star.text.TextField.DateTime",           Type::EXT_TIMEFIELD },
    { "com.sun.star.text.TextField.DocInfo.Title",      Type::TITLEFIELD },
    { "com.sun.star.text.TextField.FileName",           Type::FILEFIELD },
    { "com.sun.star.text.TextField.SheetName",          Type::SHEETFIELD },
    { "com.sun.star.style.CellStyle",                   Type::CELLSTYLE },
    { "com.sun.star.style.PageStyle",                   Type::PAGESTYLE },
    { "com.sun.star.sheet.TableAutoFormat",             Type::AUTOFORMAT },
    { "com.sun.star.sheet.SheetCellRanges",             Type::CELLRANGES },
    { "com.sun.star.drawing.GradientTable",             Type::GRADTAB },
    { "com.sun.star.drawing.HatchTable",                Type::HATCHTAB },
    { "com.sun.star.drawing.BitmapTable",               Type::BITMAPTAB },
    { "com.sun.star.drawing.TransparencyGradientTable", Type::TRGRADTAB },
    { "com.sun.star.drawing.MarkerTable",               Type::MARKERTAB },
    { "com.sun.star.drawing.DashTable",                 Type::DASHTAB },
    { "com.sun.star.text.NumberingRules",               Type::NUMRULES },
    { "com.sun.star.sheet.Defaults",                    Type::DOCDEFLTS },
    { "com.sun.star.drawing.Defaults",                  Type::DRAWDEFLTS },
    { "com.sun.star.comp.SpreadsheetSettings",          Type::DOCSPRSETT },
    { "com.sun.star.document.Settings",                 Type::DOCCONF },
    { "com.sun.star.image.ImageMapRectangleObject",     Type::IMAP_RECT },
    { "com.sun.star.image.ImageMapCircleObject",        Type::IMAP_CIRC },
    { "com.sun.star.image.ImageMapPolygonObject",       Type::IMAP_POLY },

    // Support creation of GraphicObjectResolver and EmbeddedObjectResolver
    { "com.sun.star.document.ExportGraphicObjectResolver",  Type::EXPORT_GOR },
    { "com.sun.star.document.ImportGraphicObjectResolver",  Type::IMPORT_GOR },
    { "com.sun.star.document.ExportEmbeddedObjectResolver", Type::EXPORT_EOR },
    { "com.sun.star.document.ImportEmbeddedObjectResolver", Type::IMPORT_EOR },

    { SC_SERVICENAME_VALBIND,               Type::VALBIND },
    { SC_SERVICENAME_LISTCELLBIND,          Type::LISTCELLBIND },
    { SC_SERVICENAME_LISTSOURCE,            Type::LISTSOURCE },
    { SC_SERVICENAME_CELLADDRESS,           Type::CELLADDRESS },
    { SC_SERVICENAME_RANGEADDRESS,          Type::RANGEADDRESS },

    { "com.sun.star.sheet.DocumentSettings",Type::SHEETDOCSET },

    { SC_SERVICENAME_CHDATAPROV,            Type::CHDATAPROV },
    { SC_SERVICENAME_FORMULAPARS,           Type::FORMULAPARS },
    { SC_SERVICENAME_OPCODEMAPPER,          Type::OPCODEMAPPER },
    { "ooo.vba.VBAObjectModuleObjectProvider", Type::VBAOBJECTPROVIDER },
    { "ooo.vba.VBACodeNameProvider",        Type::VBACODENAMEPROVIDER },
    { "ooo.vba.VBAGlobals",                 Type::VBAGLOBALS },

    // case-correct versions of the service names (#i102468#)
    { "com.sun.star.text.textfield.URL",                Type::URLFIELD },
    { "com.sun.star.text.textfield.PageNumber",         Type::PAGEFIELD },
    { "com.sun.star.text.textfield.PageCount",          Type::PAGESFIELD },
    { "com.sun.star.text.textfield.Date",               Type::DATEFIELD },
    { "com.sun.star.text.textfield.Time",               Type::TIMEFIELD },
    { "com.sun.star.text.textfield.DateTime",           Type::EXT_TIMEFIELD },
    { "com.sun.star.text.textfield.docinfo.Title",      Type::TITLEFIELD },
    { "com.sun.star.text.textfield.FileName",           Type::FILEFIELD },
    { "com.sun.star.text.textfield.SheetName",          Type::SHEETFIELD },
    { "ooo.vba.VBAGlobals",                             Type::VBAGLOBALS },
};

//  old service names that were in 567 still work in createInstance,
//  in case some macro is still using them
const ProvNamesId_Type aOldNames[] =
{
    { "stardiv.one.text.TextField.URL",           Type::URLFIELD },
    { "stardiv.one.text.TextField.PageNumber",    Type::PAGEFIELD },
    { "stardiv.one.text.TextField.PageCount",     Type::PAGESFIELD },
    { "stardiv.one.text.TextField.Date",          Type::DATEFIELD },
    { "stardiv.one.text.TextField.Time",          Type::TIMEFIELD },
    { "stardiv.one.text.TextField.DocumentTitle", Type::TITLEFIELD },
    { "stardiv.one.text.TextField.FileName",      Type::FILEFIELD },
    { "stardiv.one.text.TextField.SheetName",     Type::SHEETFIELD },
    { "stardiv.one.style.CellStyle",              Type::CELLSTYLE },
    { "stardiv.one.style.PageStyle",              Type::PAGESTYLE },
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


ScServiceProvider::Type ScServiceProvider::GetProviderType(const OUString& rServiceName)
{
    if (!rServiceName.isEmpty())
    {
        for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aProvNamesId); i++)
        {
            if (rServiceName.equalsAscii( aProvNamesId[i].pName ))
            {
                return aProvNamesId[i].nType;
            }
        }

        for (sal_uInt16 i=0; i < SAL_N_ELEMENTS(aOldNames); i++)
        {
            OSL_ENSURE( aOldNames[i].pName, "ScServiceProvider::GetProviderType: no oldname => crash");
            if (rServiceName.equalsAscii( aOldNames[i].pName ))
            {
                OSL_FAIL("old service name used");
                return aOldNames[i].nType;
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
            //  noch nicht eingefuegt - DocShell=Null
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
            xRet.set(static_cast<style::XStyle*>(new ScStyleObj( nullptr, SFX_STYLE_FAMILY_PARA, OUString() )));
            break;
        case Type::PAGESTYLE:
            xRet.set(static_cast<style::XStyle*>(new ScStyleObj( nullptr, SFX_STYLE_FAMILY_PAGE, OUString() )));
            break;
        case Type::AUTOFORMAT:
            xRet.set(static_cast<container::XIndexAccess*>(new ScAutoFormatObj( SC_AFMTOBJ_INVALID )));
            break;
        case Type::CELLRANGES:
            //  wird nicht eingefuegt, sondern gefuellt
            //  -> DocShell muss gesetzt sein, aber leere Ranges
            if (pDocShell)
                xRet.set(static_cast<sheet::XSheetCellRanges*>(new ScCellRangesObj( pDocShell, ScRangeList() )));
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

        // Support creation of GraphicObjectResolver and EmbeddedObjectResolver
        case Type::EXPORT_GOR:
            xRet.set(static_cast<cppu::OWeakObject *>(new SvXMLGraphicHelper( GRAPHICHELPER_MODE_WRITE )));
            break;
        case Type::IMPORT_GOR:
            xRet.set(static_cast<cppu::OWeakObject *>(new SvXMLGraphicHelper( GRAPHICHELPER_MODE_READ )));
            break;
        case Type::EXPORT_EOR:
            if (pDocShell)
                xRet.set(static_cast<cppu::OWeakObject *>(new SvXMLEmbeddedObjectHelper( *pDocShell, EMBEDDEDOBJECTHELPER_MODE_WRITE )));
            break;
        case Type::IMPORT_EOR:
            if (pDocShell)
                xRet.set(static_cast<cppu::OWeakObject *>(new SvXMLEmbeddedObjectHelper( *pDocShell, EMBEDDEDOBJECTHELPER_MODE_READ )));
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
        case Type::FORMULAPARS:
            if (pDocShell)
                xRet.set(static_cast<sheet::XFormulaParser*>(new ScFormulaParserObj( pDocShell )));
            break;
        case Type::OPCODEMAPPER:
            if (pDocShell)
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                ScAddress aAddress;
                ScCompiler* pComp = new ScCompiler(&rDoc,aAddress);
                pComp->SetGrammar( rDoc.GetGrammar() );
                xRet.set(static_cast<sheet::XFormulaOpCodeMapper*>(new ScFormulaOpCodeMapperObj(::std::unique_ptr<formula::FormulaCompiler> (pComp))));
                break;
            }
#if HAVE_FEATURE_SCRIPTING
        case Type::VBAOBJECTPROVIDER:
            if (pDocShell && pDocShell->GetDocument().IsInVBAMode())
            {
                OSL_TRACE("**** creating VBA Object mapper");
                xRet.set(static_cast<container::XNameAccess*>(new ScVbaObjectForCodeNameProvider( pDocShell )));
            }
            break;
        case Type::VBACODENAMEPROVIDER:
            if ( pDocShell && isInVBAMode( *pDocShell ) )
            {
                OSL_TRACE("**** creating VBA Object provider");
                xRet.set(static_cast<document::XCodeNameQuery*>(new ScVbaCodeNameProvider(*pDocShell)));
            }
            break;
        case Type::VBAGLOBALS:
            if (pDocShell)
            {
                uno::Any aGlobs;
                if ( !pDocShell->GetBasicManager()->GetGlobalUNOConstant( "VBAGlobals", aGlobs ) )
                {
                    uno::Sequence< uno::Any > aArgs(1);
                    aArgs[ 0 ] <<= pDocShell->GetModel();
                    xRet = ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( "ooo.vba.excel.Globals", aArgs );
                    pDocShell->GetBasicManager()->SetGlobalUNOConstant( "VBAGlobals", uno::Any( xRet ) );
                    BasicManager* pAppMgr = SfxApplication::GetBasicManager();
                    if ( pAppMgr )
                        pAppMgr->SetGlobalUNOConstant( "ThisExcelDoc", aArgs[ 0 ] );

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
    const sal_uInt16 nEntries = sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
    uno::Sequence<OUString> aRet(nEntries);
    OUString* pArray = aRet.getArray();
    for (sal_uInt16 i = 0; i < nEntries; i++)
    {
        pArray[i] = OUString::createFromAscii( aProvNamesId[i].pName );
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

class ScVbaObjectForCodeNameProvider : public ::cppu::WeakImplHelper1< container::XNameAccess >
{
    uno::Any maWorkbook;
    uno::Any maCachedObject;
    ScDocShell* mpDocShell;
public:
    ScVbaObjectForCodeNameProvider( ScDocShell* pDocShell ) : mpDocShell( pDocShell )
    {
        ScDocument* pDoc = mpDocShell->GetDocument();
        if ( !pDoc )
            throw uno::RuntimeException("", uno::Reference< uno::XInterface >() );

        uno::Sequence< uno::Any > aArgs(2);
        // access the application object ( parent for workbook )
        aArgs[0] = uno::Any( ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.Application", uno::Sequence< uno::Any >() ) );
        aArgs[1] = uno::Any( mpDocShell->GetModel() );
        maWorkbook <<= ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "ooo.vba.excel.Workbook", aArgs );
    }

    virtual ::sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException )
    {
        SolarMutexGuard aGuard;
        maCachedObject = uno::Any(); // clear cached object
        String sName = aName;

        ScDocument* pDoc = mpDocShell->GetDocument();
        if ( !pDoc )
            throw uno::RuntimeException();
        // aName ( sName ) is generated from the stream name which can be different ( case-wise )
        // from the code name
        if( sName.EqualsIgnoreCaseAscii( pDoc->GetCodeName() ) )
            maCachedObject = maWorkbook;
        else
        {
            OUString sCodeName;
            SCTAB nCount = pDoc->GetTableCount();
            for( SCTAB i = 0; i < nCount; i++ )
            {
                pDoc->GetCodeName( i, sCodeName );
                // aName ( sName ) is generated from the stream name which can be different ( case-wise )
                // from the code name
                if( String(sCodeName).EqualsIgnoreCaseAscii( sName ) )
                {
                    OUString sSheetName;
                    if( pDoc->GetName( i, sSheetName ) )
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
    ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;
        OSL_TRACE("ScVbaObjectForCodeNameProvider::getByName( %s )",
            OUStringToOString( aName, RTL_TEXTENCODING_UTF8 ).getStr() );
        if ( !hasByName( aName ) )
            throw ::com::sun::star::container::NoSuchElementException();
        return maCachedObject;
    }
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;
        ScDocument* pDoc = mpDocShell->GetDocument();
        if ( !pDoc )
            throw uno::RuntimeException();
        SCTAB nCount = pDoc->GetTableCount();
        uno::Sequence< OUString > aNames( nCount + 1 );
        SCTAB index = 0;
        OUString sCodeName;
        for( ; index < nCount; ++index )
        {
            pDoc->GetCodeName( index, sCodeName );
            aNames[ index ] = sCodeName;
        }
        aNames[ index ] = pDoc->GetCodeName();
        return aNames;
    }
    // XElemenAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException){ return uno::Type(); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException ) { return sal_True; }

};

class ScVbaCodeNameProvider : public ::cppu::WeakImplHelper1< document::XCodeNameQuery >
{
    ScDocShell& mrDocShell;
public:
    ScVbaCodeNameProvider( ScDocShell& rDocShell ) : mrDocShell(rDocShell) {}
    // XCodeNameQuery
    OUString SAL_CALL getCodeNameForObject( const uno::Reference< uno::XInterface >& xIf ) throw( uno::RuntimeException )
    {
        SolarMutexGuard aGuard;
        OUString sCodeName;

        // need to find the page ( and index )  for this control
        uno::Reference< drawing::XDrawPagesSupplier > xSupplier( mrDocShell.GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndex( xSupplier->getDrawPages(), uno::UNO_QUERY_THROW );
        sal_Int32 nLen = xIndex->getCount();
        bool bMatched = false;
        uno::Sequence< script::ScriptEventDescriptor > aFakeEvents;
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
                        mrDocShell.GetDocument()->GetCodeName( static_cast<SCTAB>( index ), sName );
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
            throw( uno::RuntimeException )
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
                    if (mrDocShell.GetDocument()->GetCodeName(static_cast<SCTAB>(i), aName))
                        return aName;
                }
            }
            catch( uno::Exception& ) {}
        }
        return OUString();
    }
};

//------------------------------------------------------------------------
//
struct ProvNamesId_Type
{
    const char *    pName;
    sal_uInt16      nType;
};

static const ProvNamesId_Type aProvNamesId[] =
{
    { "com.sun.star.sheet.Spreadsheet",                 SC_SERVICE_SHEET },
    { "com.sun.star.text.TextField.URL",                SC_SERVICE_URLFIELD },
    { "com.sun.star.text.TextField.PageNumber",         SC_SERVICE_PAGEFIELD },
    { "com.sun.star.text.TextField.PageCount",          SC_SERVICE_PAGESFIELD },
    { "com.sun.star.text.TextField.Date",               SC_SERVICE_DATEFIELD },
    { "com.sun.star.text.TextField.Time",               SC_SERVICE_TIMEFIELD },
    { "com.sun.star.text.TextField.DateTime",           SC_SERVICE_EXT_TIMEFIELD },
    { "com.sun.star.text.TextField.DocInfo.Title",      SC_SERVICE_TITLEFIELD },
    { "com.sun.star.text.TextField.FileName",           SC_SERVICE_FILEFIELD },
    { "com.sun.star.text.TextField.SheetName",          SC_SERVICE_SHEETFIELD },
    { "com.sun.star.style.CellStyle",                   SC_SERVICE_CELLSTYLE },
    { "com.sun.star.style.PageStyle",                   SC_SERVICE_PAGESTYLE },
    { "com.sun.star.sheet.TableAutoFormat",             SC_SERVICE_AUTOFORMAT },
    { "com.sun.star.sheet.SheetCellRanges",             SC_SERVICE_CELLRANGES },
    { "com.sun.star.drawing.GradientTable",             SC_SERVICE_GRADTAB },
    { "com.sun.star.drawing.HatchTable",                SC_SERVICE_HATCHTAB },
    { "com.sun.star.drawing.BitmapTable",               SC_SERVICE_BITMAPTAB },
    { "com.sun.star.drawing.TransparencyGradientTable", SC_SERVICE_TRGRADTAB },
    { "com.sun.star.drawing.MarkerTable",               SC_SERVICE_MARKERTAB },
    { "com.sun.star.drawing.DashTable",                 SC_SERVICE_DASHTAB },
    { "com.sun.star.text.NumberingRules",               SC_SERVICE_NUMRULES },
    { "com.sun.star.sheet.Defaults",                    SC_SERVICE_DOCDEFLTS },
    { "com.sun.star.drawing.Defaults",                  SC_SERVICE_DRAWDEFLTS },
    { "com.sun.star.comp.SpreadsheetSettings",          SC_SERVICE_DOCSPRSETT },
    { "com.sun.star.document.Settings",                 SC_SERVICE_DOCCONF },
    { "com.sun.star.image.ImageMapRectangleObject",     SC_SERVICE_IMAP_RECT },
    { "com.sun.star.image.ImageMapCircleObject",        SC_SERVICE_IMAP_CIRC },
    { "com.sun.star.image.ImageMapPolygonObject",       SC_SERVICE_IMAP_POLY },

        // Support creation of GraphicObjectResolver and EmbeddedObjectResolver
    { "com.sun.star.document.ExportGraphicObjectResolver",  SC_SERVICE_EXPORT_GOR },
    { "com.sun.star.document.ImportGraphicObjectResolver",  SC_SERVICE_IMPORT_GOR },
    { "com.sun.star.document.ExportEmbeddedObjectResolver", SC_SERVICE_EXPORT_EOR },
    { "com.sun.star.document.ImportEmbeddedObjectResolver", SC_SERVICE_IMPORT_EOR },

    { SC_SERVICENAME_VALBIND,               SC_SERVICE_VALBIND },
    { SC_SERVICENAME_LISTCELLBIND,          SC_SERVICE_LISTCELLBIND },
    { SC_SERVICENAME_LISTSOURCE,            SC_SERVICE_LISTSOURCE },
    { SC_SERVICENAME_CELLADDRESS,           SC_SERVICE_CELLADDRESS },
    { SC_SERVICENAME_RANGEADDRESS,          SC_SERVICE_RANGEADDRESS },

    { "com.sun.star.sheet.DocumentSettings",SC_SERVICE_SHEETDOCSET },

    { SC_SERVICENAME_CHDATAPROV,            SC_SERVICE_CHDATAPROV },
    { SC_SERVICENAME_FORMULAPARS,           SC_SERVICE_FORMULAPARS },
    { SC_SERVICENAME_OPCODEMAPPER,          SC_SERVICE_OPCODEMAPPER },
    { "ooo.vba.VBAObjectModuleObjectProvider", SC_SERVICE_VBAOBJECTPROVIDER },
    { "ooo.vba.VBACodeNameProvider",        SC_SERVICE_VBACODENAMEPROVIDER },
    { "ooo.vba.VBAGlobals",                 SC_SERVICE_VBAGLOBALS },

    // case-correct versions of the service names (#i102468#)
    { "com.sun.star.text.textfield.URL",                SC_SERVICE_URLFIELD },
    { "com.sun.star.text.textfield.PageNumber",         SC_SERVICE_PAGEFIELD },
    { "com.sun.star.text.textfield.PageCount",          SC_SERVICE_PAGESFIELD },
    { "com.sun.star.text.textfield.Date",               SC_SERVICE_DATEFIELD },
    { "com.sun.star.text.textfield.Time",               SC_SERVICE_TIMEFIELD },
    { "com.sun.star.text.textfield.DateTime",           SC_SERVICE_EXT_TIMEFIELD },
    { "com.sun.star.text.textfield.docinfo.Title",      SC_SERVICE_TITLEFIELD },
    { "com.sun.star.text.textfield.FileName",           SC_SERVICE_FILEFIELD },
    { "com.sun.star.text.textfield.SheetName",          SC_SERVICE_SHEETFIELD },
    { "ooo.vba.VBAGlobals",          SC_SERVICE_VBAGLOBALS },
};

//
//  old service names that were in 567 still work in createInstance,
//  in case some macro is still using them
//

static const sal_Char* aOldNames[SC_SERVICE_COUNT] =
    {
        "",                                         // SC_SERVICE_SHEET
        "stardiv.one.text.TextField.URL",           // SC_SERVICE_URLFIELD
        "stardiv.one.text.TextField.PageNumber",    // SC_SERVICE_PAGEFIELD
        "stardiv.one.text.TextField.PageCount",     // SC_SERVICE_PAGESFIELD
        "stardiv.one.text.TextField.Date",          // SC_SERVICE_DATEFIELD
        "stardiv.one.text.TextField.Time",          // SC_SERVICE_TIMEFIELD
        "stardiv.one.text.TextField.DocumentTitle", // SC_SERVICE_TITLEFIELD
        "stardiv.one.text.TextField.FileName",      // SC_SERVICE_FILEFIELD
        "stardiv.one.text.TextField.SheetName",     // SC_SERVICE_SHEETFIELD
        "stardiv.one.style.CellStyle",              // SC_SERVICE_CELLSTYLE
        "stardiv.one.style.PageStyle",              // SC_SERVICE_PAGESTYLE
        "",                                         // SC_SERVICE_AUTOFORMAT
        "",                                         // SC_SERVICE_CELLRANGES
        "",                                         // SC_SERVICE_GRADTAB
        "",                                         // SC_SERVICE_HATCHTAB
        "",                                         // SC_SERVICE_BITMAPTAB
        "",                                         // SC_SERVICE_TRGRADTAB
        "",                                         // SC_SERVICE_MARKERTAB
        "",                                         // SC_SERVICE_DASHTAB
        "",                                         // SC_SERVICE_NUMRULES
        "",                                         // SC_SERVICE_DOCDEFLTS
        "",                                         // SC_SERVICE_DRAWDEFLTS
        "",                                         // SC_SERVICE_DOCSPRSETT
        "",                                         // SC_SERVICE_DOCCONF
        "",                                         // SC_SERVICE_IMAP_RECT
        "",                                         // SC_SERVICE_IMAP_CIRC
        "",                                         // SC_SERVICE_IMAP_POLY

        // Support creation of GraphicObjectResolver and EmbeddedObjectResolver
        "",                                         // SC_SERVICE_EXPORT_GOR
        "",                                         // SC_SERVICE_IMPORT_GOR
        "",                                         // SC_SERVICE_EXPORT_EOR
        "",                                         // SC_SERVICE_IMPORT_EOR

        "",                                         // SC_SERVICE_VALBIND
        "",                                         // SC_SERVICE_LISTCELLBIND
        "",                                         // SC_SERVICE_LISTSOURCE
        "",                                         // SC_SERVICE_CELLADDRESS
        "",                                         // SC_SERVICE_RANGEADDRESS
        "",                                         // SC_SERVICE_SHEETDOCSET
        "",                                         // SC_SERVICE_CHDATAPROV
        "",                                         // SC_SERVICE_FORMULAPARS
        "",                                         // SC_SERVICE_OPCODEMAPPER
        "",                                         // SC_SERVICE_VBAOBJECTPROVIDER
        "",                                         // SC_SERVICE_VBACODENAMEPROVIDER
        "",                                         // SC_SERVICE_VBAGLOBALS
        "",                                         // SC_SERVICE_EXT_TIMEFIELD
    };




//------------------------------------------------------------------------

//  alles static


sal_uInt16 ScServiceProvider::GetProviderType(const OUString& rServiceName)
{
    if (!rServiceName.isEmpty())
    {
        const sal_uInt16 nEntries =
            sizeof(aProvNamesId) / sizeof(aProvNamesId[0]);
        for (sal_uInt16 i = 0; i < nEntries; i++)
        {
            if (rServiceName.equalsAscii( aProvNamesId[i].pName ))
            {
                return aProvNamesId[i].nType;
            }
        }

        sal_uInt16 i;
        for (i=0; i<SC_SERVICE_COUNT; i++)
        {
            OSL_ENSURE( aOldNames[i], "ScServiceProvider::GetProviderType: no oldname => crash");
            if (rServiceName.equalsAscii( aOldNames[i] ))
            {
                OSL_FAIL("old service name used");
                return i;
            }
        }
    }
    return SC_SERVICE_INVALID;
}

namespace {

sal_Int32 getFieldType(sal_uInt16 nOldType)
{
    switch (nOldType)
    {
        case SC_SERVICE_URLFIELD:
            return text::textfield::Type::URL;
        case SC_SERVICE_PAGEFIELD:
            return text::textfield::Type::PAGE;
        case SC_SERVICE_PAGESFIELD:
            return text::textfield::Type::PAGES;
        case SC_SERVICE_DATEFIELD:
            return text::textfield::Type::DATE;
        case SC_SERVICE_TIMEFIELD:
            return text::textfield::Type::TIME;
        case SC_SERVICE_EXT_TIMEFIELD:
            return text::textfield::Type::EXTENDED_TIME;
        case SC_SERVICE_TITLEFIELD:
            return text::textfield::Type::DOCINFO_TITLE;
        case SC_SERVICE_FILEFIELD:
            return text::textfield::Type::EXTENDED_FILE;
        case SC_SERVICE_SHEETFIELD:
            return text::textfield::Type::TABLE;
        default:
            ;
    }

    return text::textfield::Type::URL; // default to URL for no reason whatsoever.
}

}

uno::Reference<uno::XInterface> ScServiceProvider::MakeInstance(
                                    sal_uInt16 nType, ScDocShell* pDocShell )
{
    uno::Reference<uno::XInterface> xRet;
    switch (nType)
    {
        case SC_SERVICE_SHEET:
            //  noch nicht eingefuegt - DocShell=Null
            xRet.set((sheet::XSpreadsheet*)new ScTableSheetObj(NULL,0));
            break;
        case SC_SERVICE_URLFIELD:
        case SC_SERVICE_PAGEFIELD:
        case SC_SERVICE_PAGESFIELD:
        case SC_SERVICE_DATEFIELD:
        case SC_SERVICE_TIMEFIELD:
        case SC_SERVICE_EXT_TIMEFIELD:
        case SC_SERVICE_TITLEFIELD:
        case SC_SERVICE_FILEFIELD:
        case SC_SERVICE_SHEETFIELD:
        {
            uno::Reference<text::XTextRange> xNullContent;
            xRet.set(static_cast<text::XTextField*>(
                new ScEditFieldObj(xNullContent, NULL, getFieldType(nType), ESelection())));
        }
        break;
        case SC_SERVICE_CELLSTYLE:
            xRet.set((style::XStyle*)new ScStyleObj( NULL, SFX_STYLE_FAMILY_PARA, String() ));
            break;
        case SC_SERVICE_PAGESTYLE:
            xRet.set((style::XStyle*)new ScStyleObj( NULL, SFX_STYLE_FAMILY_PAGE, String() ));
            break;
        case SC_SERVICE_AUTOFORMAT:
            xRet.set((container::XIndexAccess*)new ScAutoFormatObj( SC_AFMTOBJ_INVALID ));
            break;
        case SC_SERVICE_CELLRANGES:
            //  wird nicht eingefuegt, sondern gefuellt
            //  -> DocShell muss gesetzt sein, aber leere Ranges
            if (pDocShell)
                xRet.set((sheet::XSheetCellRanges*)new ScCellRangesObj( pDocShell, ScRangeList() ));
            break;

        case SC_SERVICE_DOCDEFLTS:
            if (pDocShell)
                xRet.set((beans::XPropertySet*)new ScDocDefaultsObj( pDocShell ));
            break;
        case SC_SERVICE_DRAWDEFLTS:
            if (pDocShell)
                xRet.set((beans::XPropertySet*)new ScDrawDefaultsObj( pDocShell ));
            break;

        //  Drawing layer tables are not in SvxUnoDrawMSFactory,
        //  because SvxUnoDrawMSFactory doesn't have a SdrModel pointer.
        //  Drawing layer is always allocated if not there (MakeDrawLayer).

        case SC_SERVICE_GRADTAB:
            if (pDocShell)
                xRet.set(SvxUnoGradientTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case SC_SERVICE_HATCHTAB:
            if (pDocShell)
                xRet.set(SvxUnoHatchTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case SC_SERVICE_BITMAPTAB:
            if (pDocShell)
                xRet.set(SvxUnoBitmapTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case SC_SERVICE_TRGRADTAB:
            if (pDocShell)
                xRet.set(SvxUnoTransGradientTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case SC_SERVICE_MARKERTAB:
            if (pDocShell)
                xRet.set(SvxUnoMarkerTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case SC_SERVICE_DASHTAB:
            if (pDocShell)
                xRet.set(SvxUnoDashTable_createInstance( pDocShell->MakeDrawLayer() ));
            break;
        case SC_SERVICE_NUMRULES:
            if (pDocShell)
                xRet.set(SvxCreateNumRule( pDocShell->MakeDrawLayer() ));
            break;
        case SC_SERVICE_DOCSPRSETT:
        case SC_SERVICE_SHEETDOCSET:
        case SC_SERVICE_DOCCONF:
            if (pDocShell)
                xRet.set((beans::XPropertySet*)new ScDocumentConfiguration(pDocShell));
            break;

        case SC_SERVICE_IMAP_RECT:
            xRet.set(SvUnoImageMapRectangleObject_createInstance( ScShapeObj::GetSupportedMacroItems() ));
            break;
        case SC_SERVICE_IMAP_CIRC:
            xRet.set(SvUnoImageMapCircleObject_createInstance( ScShapeObj::GetSupportedMacroItems() ));
            break;
        case SC_SERVICE_IMAP_POLY:
            xRet.set(SvUnoImageMapPolygonObject_createInstance( ScShapeObj::GetSupportedMacroItems() ));
            break;

        // Support creation of GraphicObjectResolver and EmbeddedObjectResolver
        case SC_SERVICE_EXPORT_GOR:
            xRet.set((::cppu::OWeakObject * )new SvXMLGraphicHelper( GRAPHICHELPER_MODE_WRITE ));
            break;

        case SC_SERVICE_IMPORT_GOR:
            xRet.set((::cppu::OWeakObject * )new SvXMLGraphicHelper( GRAPHICHELPER_MODE_READ ));
            break;

        case SC_SERVICE_EXPORT_EOR:
            if (pDocShell)
                xRet.set((::cppu::OWeakObject * )new SvXMLEmbeddedObjectHelper( *pDocShell, EMBEDDEDOBJECTHELPER_MODE_WRITE ));
            break;

        case SC_SERVICE_IMPORT_EOR:
            if (pDocShell)
                xRet.set((::cppu::OWeakObject * )new SvXMLEmbeddedObjectHelper( *pDocShell, EMBEDDEDOBJECTHELPER_MODE_READ ));
            break;

        case SC_SERVICE_VALBIND:
        case SC_SERVICE_LISTCELLBIND:
            if (pDocShell)
            {
                sal_Bool bListPos = ( nType == SC_SERVICE_LISTCELLBIND );
                uno::Reference<sheet::XSpreadsheetDocument> xDoc( pDocShell->GetBaseModel(), uno::UNO_QUERY );
                xRet.set(*new calc::OCellValueBinding( xDoc, bListPos ));
            }
            break;
        case SC_SERVICE_LISTSOURCE:
            if (pDocShell)
            {
                uno::Reference<sheet::XSpreadsheetDocument> xDoc( pDocShell->GetBaseModel(), uno::UNO_QUERY );
                xRet.set(*new calc::OCellListSource( xDoc ));
            }
            break;
        case SC_SERVICE_CELLADDRESS:
        case SC_SERVICE_RANGEADDRESS:
            if (pDocShell)
            {
                sal_Bool bRange = ( nType == SC_SERVICE_RANGEADDRESS );
                xRet.set(*new ScAddressConversionObj( pDocShell, bRange ));
            }
            break;

        case SC_SERVICE_CHDATAPROV:
            if (pDocShell && pDocShell->GetDocument())
                xRet = *new ScChart2DataProvider( pDocShell->GetDocument() );
            break;

        case SC_SERVICE_FORMULAPARS:
            if (pDocShell)
                xRet.set(static_cast<sheet::XFormulaParser*>(new ScFormulaParserObj( pDocShell )));
            break;

        case SC_SERVICE_OPCODEMAPPER:
            if (pDocShell)
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                ScAddress aAddress;
                ScCompiler* pComp = new ScCompiler(pDoc,aAddress);
                pComp->SetGrammar( pDoc->GetGrammar() );
                xRet.set(static_cast<sheet::XFormulaOpCodeMapper*>(new ScFormulaOpCodeMapperObj(::std::auto_ptr<formula::FormulaCompiler> (pComp))));
                break;
            }
#ifndef DISABLE_SCRIPTING
        case SC_SERVICE_VBAOBJECTPROVIDER:
            if (pDocShell && pDocShell->GetDocument()->IsInVBAMode())
            {
                OSL_TRACE("**** creating VBA Object mapper");
                xRet.set(static_cast<container::XNameAccess*>(new ScVbaObjectForCodeNameProvider( pDocShell )));
            }
            break;
        case SC_SERVICE_VBACODENAMEPROVIDER:
            if ( pDocShell && isInVBAMode( *pDocShell ) )
            {
                OSL_TRACE("**** creating VBA Object provider");
                xRet.set(static_cast<document::XCodeNameQuery*>(new ScVbaCodeNameProvider(*pDocShell)));
            }
            break;
        case SC_SERVICE_VBAGLOBALS:
            if (pDocShell)
            {
                uno::Any aGlobs;
                if ( !pDocShell->GetBasicManager()->GetGlobalUNOConstant( "VBAGlobals", aGlobs ) )
                {
                    uno::Sequence< uno::Any > aArgs(1);
                    aArgs[ 0 ] <<= pDocShell->GetModel();
                    xRet = ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( OUString( "ooo.vba.excel.Globals" ), aArgs );
                    pDocShell->GetBasicManager()->SetGlobalUNOConstant( "VBAGlobals", uno::Any( xRet ) );
                    BasicManager* pAppMgr = SFX_APP()->GetBasicManager();
                    if ( pAppMgr )
                        pAppMgr->SetGlobalUNOConstant( "ThisExcelDoc", aArgs[ 0 ] );

                    // create the VBA document event processor
                    uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents(
                        ::ooo::vba::createVBAUnoAPIServiceWithArgs( pDocShell, "com.sun.star.script.vba.VBASpreadsheetEventProcessor", aArgs ), uno::UNO_QUERY );
                    pDocShell->GetDocument()->SetVbaEventProcessor( xVbaEvents );
                }
            }
        break;
#endif
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

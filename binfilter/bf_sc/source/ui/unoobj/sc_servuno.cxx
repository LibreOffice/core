/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svtools/unoimap.hxx>
#include <bf_svx/unofill.hxx>

#include <bf_svx/unonrule.hxx>

#include "servuno.hxx"
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

// #100263# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
#include <bf_svx/xmleohlp.hxx>

#include <bf_svx/xmlgrhlp.hxx>
namespace binfilter {

using namespace ::com::sun::star;


//------------------------------------------------------------------------

static const sal_Char* __FAR_DATA aProvNames[SC_SERVICE_COUNT] =
    {
        "com.sun.star.sheet.Spreadsheet",			// SC_SERVICE_SHEET
        "com.sun.star.text.TextField.URL",			// SC_SERVICE_URLFIELD
        "com.sun.star.text.TextField.PageNumber",	// SC_SERVICE_PAGEFIELD
        "com.sun.star.text.TextField.PageCount",	// SC_SERVICE_PAGESFIELD
        "com.sun.star.text.TextField.Date",			// SC_SERVICE_DATEFIELD
        "com.sun.star.text.TextField.Time",			// SC_SERVICE_TIMEFIELD
        "com.sun.star.text.TextField.DocumentTitle",// SC_SERVICE_TITLEFIELD
        "com.sun.star.text.TextField.FileName",		// SC_SERVICE_FILEFIELD
        "com.sun.star.text.TextField.SheetName",	// SC_SERVICE_SHEETFIELD
        "com.sun.star.style.CellStyle",				// SC_SERVICE_CELLSTYLE
        "com.sun.star.style.PageStyle",				// SC_SERVICE_PAGESTYLE
        "com.sun.star.sheet.TableAutoFormat",		// SC_SERVICE_AUTOFORMAT
        "com.sun.star.sheet.SheetCellRanges",		// SC_SERVICE_CELLRANGES
        "com.sun.star.drawing.GradientTable",		// SC_SERVICE_GRADTAB
        "com.sun.star.drawing.HatchTable",			// SC_SERVICE_HATCHTAB
        "com.sun.star.drawing.BitmapTable",			// SC_SERVICE_BITMAPTAB
        "com.sun.star.drawing.TransparencyGradientTable",	// SC_SERVICE_TRGRADTAB
        "com.sun.star.drawing.MarkerTable",			// SC_SERVICE_MARKERTAB
        "com.sun.star.drawing.DashTable",			// SC_SERVICE_DASHTAB
        "com.sun.star.text.NumberingRules",			// SC_SERVICE_NUMRULES
        "com.sun.star.sheet.Defaults",				// SC_SERVICE_DOCDEFLTS
        "com.sun.star.drawing.Defaults",			// SC_SERVICE_DRAWDEFLTS
        "com.sun.star.comp.SpreadsheetSettings",	// SC_SERVICE_DOCSPRSETT
        "com.sun.star.document.Settings",			// SC_SERVICE_DOCCONF
        "com.sun.star.image.ImageMapRectangleObject",// SC_SERVICE_IMAP_RECT
        "com.sun.star.image.ImageMapCircleObject",	// SC_SERVICE_IMAP_CIRC
        "com.sun.star.image.ImageMapPolygonObject",	// SC_SERVICE_IMAP_POLY

        // #100263# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
        "com.sun.star.document.ExportGraphicObjectResolver",	// SC_SERVICE_EXPORT_GOR
        "com.sun.star.document.ImportGraphicObjectResolver",	// SC_SERVICE_IMPORT_GOR
        "com.sun.star.document.ExportEmbeddedObjectResolver",	// SC_SERVICE_EXPORT_EOR
        "com.sun.star.document.ImportEmbeddedObjectResolver"	// SC_SERVICE_IMPORT_EOR
    };

//
//	old service names that were in 567 still work in createInstance,
//	in case some macro is still using them
//

static const sal_Char* __FAR_DATA aOldNames[SC_SERVICE_COUNT] =
    {
        "",											// SC_SERVICE_SHEET
        "stardiv.one.text.TextField.URL",			// SC_SERVICE_URLFIELD
        "stardiv.one.text.TextField.PageNumber",	// SC_SERVICE_PAGEFIELD
        "stardiv.one.text.TextField.PageCount",		// SC_SERVICE_PAGESFIELD
        "stardiv.one.text.TextField.Date",			// SC_SERVICE_DATEFIELD
        "stardiv.one.text.TextField.Time",			// SC_SERVICE_TIMEFIELD
        "stardiv.one.text.TextField.DocumentTitle",	// SC_SERVICE_TITLEFIELD
        "stardiv.one.text.TextField.FileName",		// SC_SERVICE_FILEFIELD
        "stardiv.one.text.TextField.SheetName",		// SC_SERVICE_SHEETFIELD
        "stardiv.one.style.CellStyle",				// SC_SERVICE_CELLSTYLE
        "stardiv.one.style.PageStyle",				// SC_SERVICE_PAGESTYLE
        "",											// SC_SERVICE_AUTOFORMAT
        "",											// SC_SERVICE_CELLRANGES
        "",											// SC_SERVICE_GRADTAB
        "",											// SC_SERVICE_HATCHTAB
        "",											// SC_SERVICE_BITMAPTAB
        "",											// SC_SERVICE_TRGRADTAB
        "",											// SC_SERVICE_MARKERTAB
        "",											// SC_SERVICE_DASHTAB
        "",											// SC_SERVICE_NUMRULES
        "",											// SC_SERVICE_DOCDEFLTS
        "",											// SC_SERVICE_DRAWDEFLTS
        "",											// SC_SERVICE_DOCSPRSETT
        "",											// SC_SERVICE_DOCCONF
        "",											// SC_SERVICE_IMAP_RECT
        "",											// SC_SERVICE_IMAP_CIRC
        "",											// SC_SERVICE_IMAP_POLY

        // #100263# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
        "",											// SC_SERVICE_EXPORT_GOR
        "",											// SC_SERVICE_IMPORT_GOR
        "",											// SC_SERVICE_EXPORT_EOR
        ""											// SC_SERVICE_IMPORT_EOR
    };




//------------------------------------------------------------------------

//	alles static

sal_uInt16 ScServiceProvider::GetProviderType(const String& rServiceName)
{
    if (rServiceName.Len())
    {
        USHORT i;
        for (i=0; i<SC_SERVICE_COUNT; i++)
            if (rServiceName.EqualsAscii( aProvNames[i] ))
                return i;

        for (i=0; i<SC_SERVICE_COUNT; i++)
            if (rServiceName.EqualsAscii( aOldNames[i] ))
            {
                DBG_ERROR("old service name used");
                return i;
            }
    }
    return SC_SERVICE_INVALID;
}

uno::Reference<uno::XInterface> ScServiceProvider::MakeInstance(
                                    sal_uInt16 nType, ScDocShell* pDocShell )
{
    uno::Reference<uno::XInterface> xRet;
    switch (nType)
    {
        case SC_SERVICE_SHEET:
            //	noch nicht eingefuegt - DocShell=Null
            xRet = (sheet::XSpreadsheet*)new ScTableSheetObj(NULL,0);
            break;
        case SC_SERVICE_URLFIELD:
            xRet = (text::XTextField*)new ScCellFieldObj( NULL, ScAddress(), ESelection() );
            break;
        case SC_SERVICE_PAGEFIELD:
        case SC_SERVICE_PAGESFIELD:
        case SC_SERVICE_DATEFIELD:
        case SC_SERVICE_TIMEFIELD:
        case SC_SERVICE_TITLEFIELD:
        case SC_SERVICE_FILEFIELD:
        case SC_SERVICE_SHEETFIELD:
            xRet = (text::XTextField*)new ScHeaderFieldObj( NULL, 0, nType, ESelection() );
            break;
        case SC_SERVICE_CELLSTYLE:
            xRet = (style::XStyle*)new ScStyleObj( NULL, SFX_STYLE_FAMILY_PARA, String() );
            break;
        case SC_SERVICE_PAGESTYLE:
            xRet = (style::XStyle*)new ScStyleObj( NULL, SFX_STYLE_FAMILY_PAGE, String() );
            break;
        case SC_SERVICE_AUTOFORMAT:
            xRet = (container::XIndexAccess*)new ScAutoFormatObj( SC_AFMTOBJ_INVALID );
            break;
        case SC_SERVICE_CELLRANGES:
            //	wird nicht eingefuegt, sondern gefuellt
            //	-> DocShell muss gesetzt sein, aber leere Ranges
            if (pDocShell)
                xRet = (sheet::XSheetCellRanges*)new ScCellRangesObj( pDocShell, ScRangeList() );
            break;

        case SC_SERVICE_DOCDEFLTS:
            if (pDocShell)
                xRet = (beans::XPropertySet*)new ScDocDefaultsObj( pDocShell );
            break;
        case SC_SERVICE_DRAWDEFLTS:
            if (pDocShell)
                xRet = (beans::XPropertySet*)new ScDrawDefaultsObj( pDocShell );
            break;

        //	Drawing layer tables are not in SvxUnoDrawMSFactory,
        //	because SvxUnoDrawMSFactory doesn't have a SdrModel pointer.
        //	Drawing layer is always allocated if not there (MakeDrawLayer).

        case SC_SERVICE_GRADTAB:
            if (pDocShell)
                xRet = SvxUnoGradientTable_createInstance( pDocShell->MakeDrawLayer() );
            break;
        case SC_SERVICE_HATCHTAB:
            if (pDocShell)
                xRet = SvxUnoHatchTable_createInstance( pDocShell->MakeDrawLayer() );
            break;
        case SC_SERVICE_BITMAPTAB:
            if (pDocShell)
                xRet = SvxUnoBitmapTable_createInstance( pDocShell->MakeDrawLayer() );
            break;
        case SC_SERVICE_TRGRADTAB:
            if (pDocShell)
                xRet = SvxUnoTransGradientTable_createInstance( pDocShell->MakeDrawLayer() );
            break;
        case SC_SERVICE_MARKERTAB:
            if (pDocShell)
                xRet = SvxUnoMarkerTable_createInstance( pDocShell->MakeDrawLayer() );
            break;
        case SC_SERVICE_DASHTAB:
            if (pDocShell)
                xRet = SvxUnoDashTable_createInstance( pDocShell->MakeDrawLayer() );
            break;
        case SC_SERVICE_NUMRULES:
            if (pDocShell)
                xRet = SvxCreateNumRule( pDocShell->MakeDrawLayer() );
            break;
        case SC_SERVICE_DOCSPRSETT:
        case SC_SERVICE_DOCCONF:
            if (pDocShell)
                xRet = (beans::XPropertySet*)new ScDocumentConfiguration(pDocShell);
            break;

        case SC_SERVICE_IMAP_RECT:
            xRet = SvUnoImageMapRectangleObject_createInstance( ScShapeObj::GetSupportedMacroItems() );
            break;
        case SC_SERVICE_IMAP_CIRC:
            xRet = SvUnoImageMapCircleObject_createInstance( ScShapeObj::GetSupportedMacroItems() );
            break;
        case SC_SERVICE_IMAP_POLY:
            xRet = SvUnoImageMapPolygonObject_createInstance( ScShapeObj::GetSupportedMacroItems() );
            break;

        // #100263# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
        case SC_SERVICE_EXPORT_GOR:
            xRet = (::cppu::OWeakObject * )new SvXMLGraphicHelper( GRAPHICHELPER_MODE_WRITE );
            break;

        case SC_SERVICE_IMPORT_GOR:
            xRet = (::cppu::OWeakObject * )new SvXMLGraphicHelper( GRAPHICHELPER_MODE_READ );
            break;
        
        case SC_SERVICE_EXPORT_EOR:
            if (pDocShell)
                xRet = (::cppu::OWeakObject * )new SvXMLEmbeddedObjectHelper( *pDocShell, EMBEDDEDOBJECTHELPER_MODE_WRITE );
            break;
        
        case SC_SERVICE_IMPORT_EOR:
            if (pDocShell)
                xRet = (::cppu::OWeakObject * )new SvXMLEmbeddedObjectHelper( *pDocShell, EMBEDDEDOBJECTHELPER_MODE_READ );
            break;
    }
    return xRet;
}

uno::Sequence< ::rtl::OUString> ScServiceProvider::GetAllServiceNames()
{
    uno::Sequence< ::rtl::OUString> aRet(SC_SERVICE_COUNT);
    ::rtl::OUString* pArray = aRet.getArray();
    for (sal_uInt16 i = 0; i < SC_SERVICE_COUNT; i++)
        pArray[i] = ::rtl::OUString::createFromAscii( aProvNames[i] );
    return aRet;
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

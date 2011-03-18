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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <swtypes.hxx>
#include <osl/diagnose.h>
#include <unomod.hxx>
#include <unomid.h>
#include <unoprnms.hxx>
#include <unomap.hxx>
#include <prtopt.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <doc.hxx>
#include <comphelper/TypeGeneration.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <comphelper/ChainablePropertySetInfo.hxx>
#include <edtwin.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/urlobj.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::comphelper;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

enum SwViewSettingsPropertyHandles
{
    HANDLE_VIEWSET_ANNOTATIONS,
    HANDLE_VIEWSET_BREAKS,
    HANDLE_VIEWSET_DRAWINGS,
    HANDLE_VIEWSET_FIELD_COMMANDS,
    HANDLE_VIEWSET_FOOTNOTE_BACKGROUND,
    HANDLE_VIEWSET_GRAPHICS,
    HANDLE_VIEWSET_HIDDEN_CHARACTERS,
    HANDLE_VIEWSET_HIDDEN_PARAGRAPHS,
    HANDLE_VIEWSET_HIDDEN_TEXT,
    HANDLE_VIEWSET_HRULER,
    HANDLE_VIEWSET_HSCROLL,
    HANDLE_VIEWSET_INDEX_MARK_BACKGROUND,
    HANDLE_VIEWSET_NONPRINTING_CHARACTERS,
    HANDLE_VIEWSET_ONLINE_LAYOUT,
    HANDLE_VIEWSET_PARA_BREAKS,
    HANDLE_VIEWSET_PROTECTED_SPACES,
    HANDLE_VIEWSET_SOFT_HYPHENS,
    HANDLE_VIEWSET_SPACES,
    HANDLE_VIEWSET_TABLE_BOUNDARIES,
    HANDLE_VIEWSET_TABLES,
    HANDLE_VIEWSET_TABSTOPS,
    HANDLE_VIEWSET_TEXT_BOUNDARIES,
    HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND,
    HANDLE_VIEWSET_VRULER,
    HANDLE_VIEWSET_VSCROLL,
    HANDLE_VIEWSET_SMOOTH_SCROLLING,
    HANDLE_VIEWSET_SOLID_MARK_HANDLES,
    HANDLE_VIEWSET_ZOOM_TYPE,
    HANDLE_VIEWSET_ZOOM,
    HANDLE_VIEWSET_PREVENT_TIPS,
    HANDLE_VIEWSET_HELP_URL,
    HANDLE_VIEWSET_VRULER_RIGHT,
    HANDLE_VIEWSET_SHOW_RULER,
    HANDLE_VIEWSET_IS_RASTER_VISIBLE,
    HANDLE_VIEWSET_IS_SNAP_TO_RASTER,
    HANDLE_VIEWSET_RASTER_RESOLUTION_X,
    HANDLE_VIEWSET_RASTER_RESOLUTION_Y,
    HANDLE_VIEWSET_RASTER_SUBDIVISION_X,
    HANDLE_VIEWSET_RASTER_SUBDIVISION_Y,
    HANDLE_VIEWSET_HORI_RULER_METRIC,
    HANDLE_VIEWSET_VERT_RULER_METRIC
};

enum SwPrintSettingsPropertyHandles
{
    HANDLE_PRINTSET_ANNOTATION_MODE,
    HANDLE_PRINTSET_BLACK_FONTS,
    HANDLE_PRINTSET_CONTROLS,
    HANDLE_PRINTSET_DRAWINGS,
    HANDLE_PRINTSET_GRAPHICS,
    HANDLE_PRINTSET_LEFT_PAGES,
    HANDLE_PRINTSET_PAGE_BACKGROUND,
    HANDLE_PRINTSET_PROSPECT,
    HANDLE_PRINTSET_REVERSED,
    HANDLE_PRINTSET_RIGHT_PAGES,
    HANDLE_PRINTSET_FAX_NAME,
    HANDLE_PRINTSET_PAPER_FROM_SETUP,
    HANDLE_PRINTSET_TABLES,
    HANDLE_PRINTSET_SINGLE_JOBS,
    HANDLE_PRINTSET_EMPTY_PAGES,
    HANDLE_PRINTSET_PROSPECT_RTL,
    HANDLE_PRINTSET_PLACEHOLDER,
    HANDLE_PRINTSET_HIDDEN_TEXT
};

static ChainablePropertySetInfo * lcl_createViewSettingsInfo()
{
    static PropertyInfo aViewSettingsMap_Impl[] =
    {
        { RTL_CONSTASCII_STRINGPARAM ( "HelpURL" ),             HANDLE_VIEWSET_HELP_URL             , CPPUTYPE_OUSTRING,    PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "HorizontalRulerMetric"),HANDLE_VIEWSET_HORI_RULER_METRIC   , CPPUTYPE_INT32, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "IsRasterVisible"),      HANDLE_VIEWSET_IS_RASTER_VISIBLE,       CPPUTYPE_BOOLEAN,   PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "IsSnapToRaster"),       HANDLE_VIEWSET_IS_SNAP_TO_RASTER,       CPPUTYPE_BOOLEAN,   PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "IsVertRulerRightAligned"),HANDLE_VIEWSET_VRULER_RIGHT         , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "PreventHelpTips" ),     HANDLE_VIEWSET_PREVENT_TIPS         , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "RasterResolutionX"),    HANDLE_VIEWSET_RASTER_RESOLUTION_X,     CPPUTYPE_INT32,     PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "RasterResolutionY"),    HANDLE_VIEWSET_RASTER_RESOLUTION_Y,     CPPUTYPE_INT32,     PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "RasterSubdivisionX"),   HANDLE_VIEWSET_RASTER_SUBDIVISION_X,    CPPUTYPE_INT32,     PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "RasterSubdivisionY"),   HANDLE_VIEWSET_RASTER_SUBDIVISION_Y,    CPPUTYPE_INT32,     PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowAnnotations" ),     HANDLE_VIEWSET_ANNOTATIONS          , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowBreaks"),           HANDLE_VIEWSET_BREAKS               , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowDrawings"),         HANDLE_VIEWSET_DRAWINGS             , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowFieldCommands"),    HANDLE_VIEWSET_FIELD_COMMANDS       , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowFootnoteBackground"),HANDLE_VIEWSET_FOOTNOTE_BACKGROUND , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowGraphics"),         HANDLE_VIEWSET_GRAPHICS             , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHiddenCharacters"), HANDLE_VIEWSET_HIDDEN_CHARACTERS    , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHiddenParagraphs"), HANDLE_VIEWSET_HIDDEN_PARAGRAPHS    , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHiddenText"),       HANDLE_VIEWSET_HIDDEN_TEXT          , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHoriRuler"),        HANDLE_VIEWSET_HRULER               , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHoriScrollBar"),    HANDLE_VIEWSET_HSCROLL              , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowIndexMarkBackground"),HANDLE_VIEWSET_INDEX_MARK_BACKGROUND, CPPUTYPE_BOOLEAN,PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowNonprintingCharacters"),HANDLE_VIEWSET_NONPRINTING_CHARACTERS, CPPUTYPE_BOOLEAN,PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowOnlineLayout"),     HANDLE_VIEWSET_ONLINE_LAYOUT        , CPPUTYPE_BOOLEAN, PropertyAttribute::MAYBEVOID,   0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowParaBreaks"),       HANDLE_VIEWSET_PARA_BREAKS          , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowProtectedSpaces"),  HANDLE_VIEWSET_PROTECTED_SPACES     , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowRulers"),           HANDLE_VIEWSET_SHOW_RULER           , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowSoftHyphens"),      HANDLE_VIEWSET_SOFT_HYPHENS         , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowSpaces"),           HANDLE_VIEWSET_SPACES               , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTableBoundaries"),  HANDLE_VIEWSET_TABLE_BOUNDARIES     , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTables"),           HANDLE_VIEWSET_TABLES               , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTabstops"),         HANDLE_VIEWSET_TABSTOPS             , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTextBoundaries"),   HANDLE_VIEWSET_TEXT_BOUNDARIES      , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTextFieldBackground"),HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND, CPPUTYPE_BOOLEAN,PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowVertRuler"),        HANDLE_VIEWSET_VRULER               , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowVertScrollBar"),    HANDLE_VIEWSET_VSCROLL              , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "SmoothScrolling"),      HANDLE_VIEWSET_SMOOTH_SCROLLING     , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "SolidMarkHandles"),     HANDLE_VIEWSET_SOLID_MARK_HANDLES   , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "VerticalRulerMetric"),  HANDLE_VIEWSET_VERT_RULER_METRIC   , CPPUTYPE_INT32, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ZoomType"),             HANDLE_VIEWSET_ZOOM_TYPE            , CPPUTYPE_INT16,   PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ZoomValue"),            HANDLE_VIEWSET_ZOOM                 , CPPUTYPE_INT16,   PROPERTY_NONE, 0},
        { 0, 0, 0, CPPUTYPE_UNKNOWN, 0, 0 }
    };
    return new ChainablePropertySetInfo ( aViewSettingsMap_Impl );
}

static ChainablePropertySetInfo * lcl_createPrintSettingsInfo()
{
    static PropertyInfo aPrintSettingsMap_Impl[] =
    {
        { RTL_CONSTASCII_STRINGPARAM ( "PrintAnnotationMode" ), HANDLE_PRINTSET_ANNOTATION_MODE , CPPUTYPE_INT16,   PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintBlackFonts" ),      HANDLE_PRINTSET_BLACK_FONTS        , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintControls" ),        HANDLE_PRINTSET_CONTROLS           , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintDrawings" ),        HANDLE_PRINTSET_DRAWINGS           , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintGraphics" ),        HANDLE_PRINTSET_GRAPHICS           , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintHiddenText"),       HANDLE_PRINTSET_HIDDEN_TEXT        , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintLeftPages" ),       HANDLE_PRINTSET_LEFT_PAGES         , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintPageBackground" ),  HANDLE_PRINTSET_PAGE_BACKGROUND    , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintProspect" ),        HANDLE_PRINTSET_PROSPECT           , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintProspectRTL" ),     HANDLE_PRINTSET_PROSPECT_RTL       , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintReversed" ),        HANDLE_PRINTSET_REVERSED           , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintRightPages" ),      HANDLE_PRINTSET_RIGHT_PAGES        , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintFaxName" ),         HANDLE_PRINTSET_FAX_NAME           , CPPUTYPE_OUSTRING, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintPaperFromSetup" ),  HANDLE_PRINTSET_PAPER_FROM_SETUP   , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintTables" ),          HANDLE_PRINTSET_TABLES             , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintTextPlaceholder"),  HANDLE_PRINTSET_PLACEHOLDER        , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintSingleJobs" ),      HANDLE_PRINTSET_SINGLE_JOBS        , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintEmptyPages" ),      HANDLE_PRINTSET_EMPTY_PAGES        , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { 0, 0, 0, CPPUTYPE_UNKNOWN, 0, 0 }
    };
    return new ChainablePropertySetInfo ( aPrintSettingsMap_Impl );
}

/******************************************************************
 * SwXModule
 ******************************************************************/
Reference< uno::XInterface > SAL_CALL SwXModule_createInstance(
    const Reference< XMultiServiceFactory > & /*rSMgr*/) throw( Exception )
{
    static Reference< uno::XInterface >  xModule = (cppu::OWeakObject*)new SwXModule();;
    return xModule;
}

Sequence< OUString > SAL_CALL SwXModule_getSupportedServiceNames() throw()
{
    OUString sService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.GlobalSettings"));
    const Sequence< OUString > aSeq( &sService, 1 );
    return aSeq;
}

OUString SAL_CALL SwXModule_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SwXModule" ) );
}

SwXModule::SwXModule() :
    pxViewSettings(0),
    pxPrintSettings(0)
{
}

SwXModule::~SwXModule()
{
    delete pxViewSettings;
    delete pxPrintSettings;
}

Reference< XPropertySet >  SwXModule::getViewSettings(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pxViewSettings)
    {
        ((SwXModule*)this)->pxViewSettings = new Reference< XPropertySet > ;
        OSL_FAIL("Web or Text?");
        *pxViewSettings = static_cast < HelperBaseNoState * > ( new SwXViewSettings( sal_False, 0 ) );
    }
    return *pxViewSettings;
}

Reference< XPropertySet >  SwXModule::getPrintSettings(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pxPrintSettings)
    {
        ((SwXModule*)this)->pxPrintSettings = new Reference< XPropertySet > ;
        OSL_FAIL("Web or Text?");
        *pxPrintSettings = static_cast < HelperBaseNoState * > ( new SwXPrintSettings ( PRINT_SETTINGS_MODULE ) );
    }
    return *pxPrintSettings;
}

OUString SwXModule::getImplementationName(void) throw( RuntimeException )
{
    return SwXModule_getImplementationName();
}

sal_Bool SwXModule::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    const Sequence< OUString > aNames = SwXModule_getSupportedServiceNames();
    for(sal_Int32 nService = 0; nService < aNames.getLength(); nService++)
    {
        if(aNames.getConstArray()[nService] == rServiceName)
            return sal_True;
    }
    return sal_False;
}

Sequence< OUString > SwXModule::getSupportedServiceNames(void) throw( RuntimeException )
{
    return SwXModule_getSupportedServiceNames();
}

/******************************************************************
 * SwXPrintSettings
 ******************************************************************/
SwXPrintSettings::SwXPrintSettings(SwXPrintSettingsType eType, SwDoc* pDoc)
: ChainableHelperNoState ( lcl_createPrintSettingsInfo (), &Application::GetSolarMutex() )
, meType(eType)
, mpPrtOpt ( NULL )
, mpDoc ( pDoc )
{
}

SwXPrintSettings::~SwXPrintSettings()
    throw()
{
}

void SwXPrintSettings::_preSetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    switch (meType)
    {
        case PRINT_SETTINGS_MODULE:
            mpPrtOpt = SW_MOD()->GetPrtOptions( sal_False );
        break;
        case PRINT_SETTINGS_WEB:
            mpPrtOpt = SW_MOD()->GetPrtOptions( sal_True );
        break;
        case PRINT_SETTINGS_DOCUMENT:
        {
            if (!mpDoc)
                throw IllegalArgumentException ();
            mpPrtOpt = const_cast< SwPrintData * >(&mpDoc->getPrintData());
        }
        break;
    }
}

void SwXPrintSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const uno::Any &rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    sal_Bool bVal;
    if ( rInfo.mnHandle != HANDLE_PRINTSET_ANNOTATION_MODE &&
         rInfo.mnHandle != HANDLE_PRINTSET_FAX_NAME )
        bVal = *(sal_Bool*)rValue.getValue();

    switch( rInfo.mnHandle )
    {
        case HANDLE_PRINTSET_LEFT_PAGES:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintLeftPage(bVal);
        }
        break;
        case HANDLE_PRINTSET_RIGHT_PAGES:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintRightPage(bVal);
        }
        break;
        case HANDLE_PRINTSET_REVERSED:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintReverse(bVal);
        }
        break;
        case HANDLE_PRINTSET_PROSPECT:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintProspect(bVal);
        }
        break;
        case HANDLE_PRINTSET_GRAPHICS:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintGraphic(bVal);
        }
        break;
        case HANDLE_PRINTSET_TABLES:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintTable(bVal);
        }
        break;
        case HANDLE_PRINTSET_DRAWINGS:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintDraw(bVal);
        }
        break;
        case HANDLE_PRINTSET_CONTROLS:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintControl(bVal);
        }
        break;
        case HANDLE_PRINTSET_PAGE_BACKGROUND:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintPageBackground(bVal);
        }
        break;
        case HANDLE_PRINTSET_BLACK_FONTS:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintBlackFont(bVal);
        }
        break;
        case HANDLE_PRINTSET_SINGLE_JOBS:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintSingleJobs(bVal);
        }
        break;
        case HANDLE_PRINTSET_PAPER_FROM_SETUP:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPaperFromSetup(bVal);
        }
        break;
        case HANDLE_PRINTSET_ANNOTATION_MODE:
        {
            sal_Int16 nVal = 0;
            rValue >>= nVal;
            if(nVal <= text::NotePrintMode_PAGE_END)
                mpPrtOpt->SetPrintPostIts(nVal);
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case HANDLE_PRINTSET_EMPTY_PAGES:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintEmptyPages(bVal);
        }
        break;
        case HANDLE_PRINTSET_FAX_NAME:
        {
            OUString sString;
            if ( rValue >>= sString)
                mpPrtOpt->SetFaxName(sString);
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case HANDLE_PRINTSET_PROSPECT_RTL:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintProspect_RTL(bVal);
        }
        break;
        case HANDLE_PRINTSET_PLACEHOLDER:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintTextPlaceholder(bVal);
        }
        break;
        case HANDLE_PRINTSET_HIDDEN_TEXT:
        {
            bVal = *(sal_Bool*)rValue.getValue();
            mpPrtOpt->SetPrintHiddenText(bVal);
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXPrintSettings::_postSetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    mpPrtOpt = NULL;
}

void SwXPrintSettings::_preGetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    switch (meType)
    {
        case PRINT_SETTINGS_MODULE:
            mpPrtOpt = SW_MOD()->GetPrtOptions( sal_False );
        break;
        case PRINT_SETTINGS_WEB:
            mpPrtOpt = SW_MOD()->GetPrtOptions( sal_True );
        break;
        case PRINT_SETTINGS_DOCUMENT:
        {
            if (!mpDoc)
                throw IllegalArgumentException ();
            mpPrtOpt = const_cast< SwPrintData * >(&mpDoc->getPrintData());
        }
        break;
    }
}

void SwXPrintSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, uno::Any & rValue )
    throw(UnknownPropertyException, WrappedTargetException )
{
    switch( rInfo.mnHandle )
    {
        case HANDLE_PRINTSET_LEFT_PAGES:
            rValue <<= mpPrtOpt->IsPrintLeftPage();
        break;
        case HANDLE_PRINTSET_RIGHT_PAGES:
            rValue <<= mpPrtOpt->IsPrintRightPage();
        break;
        case HANDLE_PRINTSET_REVERSED:
            rValue <<= mpPrtOpt->IsPrintReverse();
        break;
        case HANDLE_PRINTSET_PROSPECT:
            rValue <<= mpPrtOpt->IsPrintProspect();
        break;
        case HANDLE_PRINTSET_GRAPHICS:
            rValue <<= mpPrtOpt->IsPrintGraphic();
        break;
        case HANDLE_PRINTSET_TABLES:
            rValue <<= mpPrtOpt->IsPrintTable();
        break;
        case HANDLE_PRINTSET_DRAWINGS:
            rValue <<= mpPrtOpt->IsPrintDraw();
        break;
        case HANDLE_PRINTSET_CONTROLS:
            rValue <<= mpPrtOpt->IsPrintControl();
        break;
        case HANDLE_PRINTSET_PAGE_BACKGROUND:
            rValue <<= mpPrtOpt->IsPrintPageBackground();
        break;
        case HANDLE_PRINTSET_BLACK_FONTS:
            rValue <<= mpPrtOpt->IsPrintBlackFont();
        break;
        case HANDLE_PRINTSET_SINGLE_JOBS:
            rValue <<= mpPrtOpt->IsPrintSingleJobs();
        break;
        case HANDLE_PRINTSET_EMPTY_PAGES:
            rValue <<= mpPrtOpt->IsPrintEmptyPages();
        break;
        case HANDLE_PRINTSET_PAPER_FROM_SETUP:
            rValue <<= mpPrtOpt->IsPaperFromSetup();
        break;
        case HANDLE_PRINTSET_ANNOTATION_MODE:
        {
            rValue <<= static_cast < sal_Int16 > ( mpPrtOpt->GetPrintPostIts() );
        }
        break;
        case HANDLE_PRINTSET_FAX_NAME :
        {
            rValue <<= mpPrtOpt->GetFaxName();
        }
        break;
        case HANDLE_PRINTSET_PROSPECT_RTL:
        {
            rValue <<= mpPrtOpt->IsPrintProspectRTL();
        }
        break;
        case HANDLE_PRINTSET_PLACEHOLDER:
        {
            rValue <<= mpPrtOpt->IsPrintTextPlaceholder();
        }
        break;
        case HANDLE_PRINTSET_HIDDEN_TEXT:
        {
            rValue <<= mpPrtOpt->IsPrintHiddenText();
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXPrintSettings::_postGetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    mpPrtOpt = NULL;
}

OUString SwXPrintSettings::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXPrintSettings");
}

sal_Bool SwXPrintSettings::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.PrintSettings") == rServiceName;
}

Sequence< OUString > SwXPrintSettings::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.PrintSettings");
    return aRet;
}

SwXViewSettings::SwXViewSettings(sal_Bool bWebView, SwView* pVw)
: ChainableHelperNoState( lcl_createViewSettingsInfo (), &Application::GetSolarMutex() )
, pView(pVw)
, mpViewOption ( NULL )
, mpConstViewOption ( NULL )
, bObjectValid(sal_True)
, bWeb(bWebView)
, eHRulerUnit( FUNIT_CM )
, mbApplyHRulerMetric( sal_False )
, eVRulerUnit( FUNIT_CM )
, mbApplyVRulerMetric( sal_False )
{
    // This property only exists if we have a view (ie, not at the module )
    if ( !pView )
        mpInfo->remove ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "HelpURL" ) ) );

}

SwXViewSettings::~SwXViewSettings()
    throw()
{

}

void SwXViewSettings::_preSetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    const SwViewOption* pVOpt = 0;
    if(pView)
    {
        if(!IsValid())
            return;
        pVOpt = pView->GetWrtShell().GetViewOptions();
    }
    else
        pVOpt = SW_MOD()->GetViewOption(bWeb);

    mpViewOption = new SwViewOption (*pVOpt);
    mbApplyZoom = sal_False;
    if(pView)
        mpViewOption->SetStarOneSetting(sal_True);
}

void SwXViewSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const uno::Any &rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    sal_Bool bVal = HANDLE_VIEWSET_ZOOM != rInfo.mnHandle ?
        *(sal_Bool*)rValue.getValue() : sal_False;
    // the API flag should not be set to the application's view settings
    switch( rInfo.mnHandle )
    {
        case  HANDLE_VIEWSET_SHOW_RULER            :   mpViewOption->SetViewAnyRuler(bVal); break;
        case  HANDLE_VIEWSET_HRULER                :   mpViewOption->SetViewHRuler(bVal);   break;
        case  HANDLE_VIEWSET_VRULER                :   mpViewOption->SetViewVRuler(bVal);break;
        case  HANDLE_VIEWSET_VRULER_RIGHT          :   mpViewOption->SetVRulerRight(bVal);break;
        case  HANDLE_VIEWSET_HSCROLL               :   mpViewOption->SetViewHScrollBar(bVal);break;
        case  HANDLE_VIEWSET_VSCROLL               :   mpViewOption->SetViewVScrollBar(bVal);break;
        case  HANDLE_VIEWSET_GRAPHICS              :   mpViewOption->SetGraphic(bVal);break;
        case  HANDLE_VIEWSET_TABLES                :   mpViewOption->SetTable(bVal);    break;
        case  HANDLE_VIEWSET_DRAWINGS              :   mpViewOption->SetDraw(bVal); break;
        case  HANDLE_VIEWSET_FIELD_COMMANDS        :   mpViewOption->SetFldName(bVal);  break;
        case  HANDLE_VIEWSET_ANNOTATIONS           :   mpViewOption->SetPostIts(bVal);  break;
        case  HANDLE_VIEWSET_INDEX_MARK_BACKGROUND :   mpViewOption->SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, sal_True);  break;
        case  HANDLE_VIEWSET_NONPRINTING_CHARACTERS:   mpViewOption->SetViewMetaChars( bVal ); break;
        case  HANDLE_VIEWSET_FOOTNOTE_BACKGROUND   :   mpViewOption->SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, sal_True); break;
        case  HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND :   mpViewOption->SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, sal_True);    break;
        case  HANDLE_VIEWSET_PARA_BREAKS           :   mpViewOption->SetParagraph(bVal);    break;
        case  HANDLE_VIEWSET_SOFT_HYPHENS          :   mpViewOption->SetSoftHyph(bVal); break;
        case  HANDLE_VIEWSET_SPACES                :   mpViewOption->SetBlank(bVal);    break;
        case  HANDLE_VIEWSET_PROTECTED_SPACES      :   mpViewOption->SetHardBlank(bVal);    break;
        case  HANDLE_VIEWSET_TABSTOPS              :   mpViewOption->SetTab(bVal);  break;
        case  HANDLE_VIEWSET_BREAKS                :   mpViewOption->SetLineBreak(bVal); break;
        case  HANDLE_VIEWSET_HIDDEN_TEXT           :   mpViewOption->SetShowHiddenField(bVal);  break;
        case  HANDLE_VIEWSET_HIDDEN_CHARACTERS     :   mpViewOption->SetShowHiddenChar(bVal); break;
        case  HANDLE_VIEWSET_HIDDEN_PARAGRAPHS     :   mpViewOption->SetShowHiddenPara(bVal);   break;
        case  HANDLE_VIEWSET_TABLE_BOUNDARIES      :   mpViewOption->SetAppearanceFlag(VIEWOPT_TABLE_BOUNDARIES, bVal, sal_True);    break;
        case  HANDLE_VIEWSET_TEXT_BOUNDARIES       :   mpViewOption->SetDocBoundaries(bVal);    break;
        case  HANDLE_VIEWSET_SMOOTH_SCROLLING      :   mpViewOption->SetSmoothScroll(bVal); break;
        case  HANDLE_VIEWSET_SOLID_MARK_HANDLES    :   mpViewOption->SetSolidMarkHdl(bVal); break;
        case  HANDLE_VIEWSET_PREVENT_TIPS :            mpViewOption->SetPreventTips(bVal); break;
        case  HANDLE_VIEWSET_IS_RASTER_VISIBLE     : mpViewOption->SetGridVisible(bVal); break;
        case  HANDLE_VIEWSET_IS_SNAP_TO_RASTER     : mpViewOption->SetSnap(bVal); break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_X   :
        {
            sal_Int32 nTmp = 0;
            if(!(rValue >>= nTmp)  ||  nTmp < 10)
                throw IllegalArgumentException();
            Size aSize( mpViewOption->GetSnapSize() );
            aSize.Width() = MM100_TO_TWIP( nTmp );
            mpViewOption->SetSnapSize( aSize );
        }
        break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_Y   :
        {
            sal_Int32 nTmp = 0;
            if(!(rValue >>= nTmp)  ||  nTmp < 10)
                throw IllegalArgumentException();
            Size aSize( mpViewOption->GetSnapSize() );
            aSize.Height() = MM100_TO_TWIP( nTmp );
            mpViewOption->SetSnapSize( aSize );
        }
        break;
        case  HANDLE_VIEWSET_RASTER_SUBDIVISION_X  :
        {
            sal_Int32 nTmp = 0;
            if(!(rValue >>= nTmp)  ||  !(0 <= nTmp  &&  nTmp < 100))
                throw IllegalArgumentException();
            mpViewOption->SetDivisionX( (short) nTmp );
        }
        break;
        case  HANDLE_VIEWSET_RASTER_SUBDIVISION_Y  :
        {
            sal_Int32 nTmp = 0;
            if(!(rValue >>= nTmp)  ||  !(0 <= nTmp  &&  nTmp < 100))
                throw IllegalArgumentException();
            mpViewOption->SetDivisionY( (short) nTmp );
        }
        break;
        case  HANDLE_VIEWSET_ZOOM                   :
        {
            sal_Int16 nZoom = 0;
            if(!(rValue >>= nZoom) || nZoom > 1000 || nZoom < 5)
                throw lang::IllegalArgumentException();
            mpViewOption->SetZoom((sal_uInt16)nZoom);
            mbApplyZoom = sal_True;
        }
        break;
        case HANDLE_VIEWSET_ZOOM_TYPE:
        {
            sal_Int16 nZoom = 0;
            if(!(rValue >>= nZoom))
                throw IllegalArgumentException();
            SvxZoomType eZoom = (SvxZoomType)USHRT_MAX;
            switch (nZoom)
            {
                case view::DocumentZoomType::OPTIMAL:
                    eZoom = SVX_ZOOM_OPTIMAL;
                break;
                case view::DocumentZoomType::PAGE_WIDTH:
                    eZoom = SVX_ZOOM_PAGEWIDTH;
                break;
                case view::DocumentZoomType::ENTIRE_PAGE:
                    eZoom = SVX_ZOOM_WHOLEPAGE;
                break;
                case view::DocumentZoomType::BY_VALUE:
                    eZoom = SVX_ZOOM_PERCENT;
                break;
                case view::DocumentZoomType::PAGE_WIDTH_EXACT:
                    eZoom = SVX_ZOOM_PAGEWIDTH_NOBORDER;
                break;
                default:
                    throw IllegalArgumentException(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "SwXViewSettings: invalid zoom type")), 0, 0);
                break;
            }
            if(eZoom < USHRT_MAX)
            {
                mpViewOption->SetZoomType( eZoom );
                mbApplyZoom = sal_True;
            }
        }
        break;
        case HANDLE_VIEWSET_ONLINE_LAYOUT :
        {
            if( pView && !bVal != !pView->GetWrtShell().getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
                pView->GetDocShell()->ToggleBrowserMode(bVal, pView );
        }
        break;
        case HANDLE_VIEWSET_HELP_URL :
        {
            if ( pView )
            {
                OUString sHelpURL;
                if ( ! ( rValue >>= sHelpURL ) )
                    throw IllegalArgumentException();

                INetURLObject aHID( sHelpURL );
                if ( aHID.GetProtocol() == INET_PROT_HID )
                      pView->GetEditWin().SetHelpId( rtl::OUStringToOString( aHID.GetURLPath(), RTL_TEXTENCODING_UTF8 ) );
                else
                    throw IllegalArgumentException ();
            }
            else
                throw UnknownPropertyException();
        }
        break;
        case HANDLE_VIEWSET_HORI_RULER_METRIC:
        case HANDLE_VIEWSET_VERT_RULER_METRIC:
        {
            sal_Int32 nUnit = -1;
            if( rValue >>= nUnit )
            switch( nUnit )
            {
                case FUNIT_MM:
                case FUNIT_CM:
                case FUNIT_POINT:
                case FUNIT_PICA:
                case FUNIT_INCH:
                if( rInfo.mnHandle == HANDLE_VIEWSET_HORI_RULER_METRIC )
                {
                    eHRulerUnit = nUnit;
                    mbApplyHRulerMetric = sal_True;
                }
                else
                {
                    eVRulerUnit = nUnit;
                    mbApplyVRulerMetric = sal_True;
                }
                break;
                default:
                    throw IllegalArgumentException();
            }
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXViewSettings::_postSetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    if( pView )
    {
        if(mbApplyZoom )
            pView->SetZoom( (SvxZoomType)mpViewOption->GetZoomType(),
                            mpViewOption->GetZoom(), sal_True );
        if(mbApplyHRulerMetric)
            pView->ChangeTabMetric((FieldUnit)eHRulerUnit);
        if(mbApplyVRulerMetric)
            pView->ChangeVLinealMetric((FieldUnit)eVRulerUnit);

    }
    else
    {
        if(mbApplyHRulerMetric)
            SW_MOD()->ApplyRulerMetric( (FieldUnit)eHRulerUnit, sal_True, bWeb );
        if(mbApplyVRulerMetric)
            SW_MOD()->ApplyRulerMetric( (FieldUnit)eVRulerUnit, sal_False, bWeb );
    }


    SW_MOD()->ApplyUsrPref( *mpViewOption, pView, pView ? VIEWOPT_DEST_VIEW_ONLY
                                                  : bWeb ? VIEWOPT_DEST_WEB
                                                          : VIEWOPT_DEST_TEXT );


    delete mpViewOption;
    mpViewOption = NULL;
}

void SwXViewSettings::_preGetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    if(pView)
    {
        if(!IsValid())
            return;
        mpConstViewOption = pView->GetWrtShell().GetViewOptions();
    }
    else
        mpConstViewOption = SW_MOD()->GetViewOption(bWeb);
}

void SwXViewSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, uno::Any & rValue )
    throw(UnknownPropertyException, WrappedTargetException )
{
    sal_Bool bBool = sal_True;
    sal_Bool bBoolVal;
    switch( rInfo.mnHandle )
    {
        case  HANDLE_VIEWSET_SHOW_RULER:                bBoolVal = mpConstViewOption->IsViewAnyRuler();   break;
        case  HANDLE_VIEWSET_HRULER :                   bBoolVal = mpConstViewOption->IsViewHRuler(sal_True);   break;
        case  HANDLE_VIEWSET_VRULER :                   bBoolVal = mpConstViewOption->IsViewVRuler(sal_True);break;
        case  HANDLE_VIEWSET_VRULER_RIGHT          :   bBoolVal = mpConstViewOption->IsVRulerRight();break;
        case  HANDLE_VIEWSET_HSCROLL:                   bBoolVal = mpConstViewOption->IsViewHScrollBar();break;
        case  HANDLE_VIEWSET_VSCROLL:                   bBoolVal = mpConstViewOption->IsViewVScrollBar();break;
        case  HANDLE_VIEWSET_GRAPHICS              :    bBoolVal = mpConstViewOption->IsGraphic();break;
        case  HANDLE_VIEWSET_TABLES                :   bBoolVal = mpConstViewOption->IsTable(); break;
        case  HANDLE_VIEWSET_DRAWINGS              :   bBoolVal = mpConstViewOption->IsDraw();  break;
        case  HANDLE_VIEWSET_FIELD_COMMANDS        :   bBoolVal = mpConstViewOption->IsFldName();   break;
        case  HANDLE_VIEWSET_ANNOTATIONS           :   bBoolVal = mpConstViewOption->IsPostIts();   break;
        case  HANDLE_VIEWSET_INDEX_MARK_BACKGROUND :   bBoolVal = SwViewOption::IsFieldShadings();   break;
        case  HANDLE_VIEWSET_NONPRINTING_CHARACTERS:   bBoolVal = mpConstViewOption->IsViewMetaChars(); break;
        case  HANDLE_VIEWSET_FOOTNOTE_BACKGROUND   :   bBoolVal = SwViewOption::IsFieldShadings();  break;
        case  HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND :   bBoolVal = SwViewOption::IsFieldShadings(); break;
        case  HANDLE_VIEWSET_PARA_BREAKS           :   bBoolVal = mpConstViewOption->IsParagraph(sal_True); break;
        case  HANDLE_VIEWSET_SOFT_HYPHENS          :   bBoolVal = mpConstViewOption->IsSoftHyph();  break;
        case  HANDLE_VIEWSET_SPACES                :   bBoolVal = mpConstViewOption->IsBlank(sal_True); break;
        case  HANDLE_VIEWSET_PROTECTED_SPACES      :   bBoolVal = mpConstViewOption->IsHardBlank(); break;
        case  HANDLE_VIEWSET_TABSTOPS              :   bBoolVal = mpConstViewOption->IsTab(sal_True);   break;
        case  HANDLE_VIEWSET_BREAKS                :   bBoolVal = mpConstViewOption->IsLineBreak(sal_True); break;
        case  HANDLE_VIEWSET_HIDDEN_TEXT           :   bBoolVal = mpConstViewOption->IsShowHiddenField();   break;
        case  HANDLE_VIEWSET_HIDDEN_CHARACTERS     :   bBoolVal = mpConstViewOption->IsShowHiddenChar(sal_True); break;
        case  HANDLE_VIEWSET_HIDDEN_PARAGRAPHS     :   bBoolVal = mpConstViewOption->IsShowHiddenPara();    break;
        case  HANDLE_VIEWSET_TABLE_BOUNDARIES      :   bBoolVal = SwViewOption::IsTableBoundaries(); break;
        case  HANDLE_VIEWSET_TEXT_BOUNDARIES       :   bBoolVal = SwViewOption::IsDocBoundaries(); break;
        case  HANDLE_VIEWSET_SMOOTH_SCROLLING      :   bBoolVal = mpConstViewOption->IsSmoothScroll();  break;
        case  HANDLE_VIEWSET_SOLID_MARK_HANDLES    :   bBoolVal = mpConstViewOption->IsSolidMarkHdl();  break;
        case  HANDLE_VIEWSET_PREVENT_TIPS :            bBoolVal = mpConstViewOption->IsPreventTips(); break;
        case  HANDLE_VIEWSET_IS_RASTER_VISIBLE     : bBoolVal = mpConstViewOption->IsGridVisible(); break;
        case  HANDLE_VIEWSET_IS_SNAP_TO_RASTER     : bBoolVal = mpConstViewOption->IsSnap(); break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_X   :
            bBool = sal_False;
            rValue <<= (sal_Int32) TWIP_TO_MM100(mpConstViewOption->GetSnapSize().Width());
        break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_Y   :
            bBool = sal_False;
            rValue <<= (sal_Int32) TWIP_TO_MM100(mpConstViewOption->GetSnapSize().Height());
        break;
        case  HANDLE_VIEWSET_RASTER_SUBDIVISION_X  :
            bBool = sal_False;
            rValue <<= (sal_Int32) mpConstViewOption->GetDivisionX();
        break;
        case  HANDLE_VIEWSET_RASTER_SUBDIVISION_Y  :
            bBool = sal_False;
            rValue <<= (sal_Int32) mpConstViewOption->GetDivisionY();
        break;
        case  HANDLE_VIEWSET_ZOOM                   :
                bBool = sal_False;
                rValue <<= (sal_Int16)mpConstViewOption->GetZoom();
        break;
        case HANDLE_VIEWSET_ZOOM_TYPE:
        {
            bBool = sal_False;
            sal_Int16 nRet(0);
            switch (mpConstViewOption->GetZoomType())
            {
                case SVX_ZOOM_OPTIMAL:
                    nRet = view::DocumentZoomType::OPTIMAL;
                break;
                case SVX_ZOOM_PAGEWIDTH:
                    nRet = view::DocumentZoomType::PAGE_WIDTH;
                break;
                case SVX_ZOOM_WHOLEPAGE:
                    nRet = view::DocumentZoomType::ENTIRE_PAGE;
                break;
                case SVX_ZOOM_PERCENT:
                    nRet = view::DocumentZoomType::BY_VALUE;
                break;
                case SVX_ZOOM_PAGEWIDTH_NOBORDER:
                    nRet = view::DocumentZoomType::PAGE_WIDTH_EXACT;
                break;
                default:
                    OSL_FAIL("SwXViewSettings: invalid zoom type");
                break;
            }
            rValue <<= nRet;
        }
        break;
        case HANDLE_VIEWSET_ONLINE_LAYOUT:
            if(pView)
                bBoolVal = pView->GetWrtShell().getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE);
        break;
        case HANDLE_VIEWSET_HELP_URL :
        {
            if ( pView )
            {
                bBool = sal_False;
                OUStringBuffer sHelpURL;
                sHelpURL.appendAscii ( INET_HID_SCHEME );
                SwEditWin &rEditWin = pView->GetEditWin();
                sHelpURL.append( rtl::OStringToOUString( rEditWin.GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
                rValue <<= sHelpURL.makeStringAndClear();
            }
            else
                throw UnknownPropertyException();
        }
        break;
        case HANDLE_VIEWSET_HORI_RULER_METRIC:
        {
            if ( pView )
            {
                FieldUnit eUnit;
                pView->GetHLinealMetric( eUnit );
                rValue <<= (sal_Int32)eUnit;
            }
            else
            {
                const SwMasterUsrPref* pUsrPref = SW_MOD()->GetUsrPref( bWeb );
                rValue <<= (sal_Int32)pUsrPref->GetHScrollMetric();
            }
            bBool = sal_False;
        }
        break;
        case HANDLE_VIEWSET_VERT_RULER_METRIC:
        {
            if ( pView )
            {
                FieldUnit eUnit;
                pView->GetVLinealMetric( eUnit );
                rValue <<= (sal_Int32)eUnit;
            }
            else
            {
                const SwMasterUsrPref* pUsrPref = SW_MOD()->GetUsrPref( bWeb );
                rValue <<= (sal_Int32)pUsrPref->GetVScrollMetric();
            }
            bBool = sal_False;
        }
        break;
        default: OSL_FAIL("there is no such ID!");
    }
    if( bBool )
        rValue.setValue(&bBoolVal, ::getBooleanCppuType());
}

void SwXViewSettings::_postGetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    mpConstViewOption = NULL;
}

OUString SwXViewSettings::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXViewSettings");
}

sal_Bool SwXViewSettings::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.ViewSettings") == rServiceName;
}

Sequence< OUString > SwXViewSettings::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ViewSettings");
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

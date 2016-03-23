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

#include <swtypes.hxx>
#include <osl/diagnose.h>
#include <unomod.hxx>
#include <unomid.h>
#include <unoprnms.hxx>
#include <unomap.hxx>
#include <prtopt.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <comphelper/ChainablePropertySetInfo.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <edtwin.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/urlobj.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::comphelper;

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
    HANDLE_VIEWSET_ZOOM_TYPE,
    HANDLE_VIEWSET_ZOOM,
    HANDLE_VIEWSET_SHOW_CONTENT_TIPS,
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
    HANDLE_VIEWSET_VERT_RULER_METRIC,
    HANDLE_VIEWSET_SCROLLBAR_TIPS,
    HANDLE_VIEWSET_HIDE_WHITESPACE
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
    static PropertyInfo const aViewSettingsMap_Impl[] =
    {
        { OUString( "HelpURL" ),             HANDLE_VIEWSET_HELP_URL             , cppu::UnoType<OUString>::get(),    PROPERTY_NONE},
        { OUString( "HorizontalRulerMetric"),HANDLE_VIEWSET_HORI_RULER_METRIC   , cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE},
        { OUString( "IsRasterVisible"),      HANDLE_VIEWSET_IS_RASTER_VISIBLE,       cppu::UnoType<bool>::get(),   PROPERTY_NONE},
        { OUString( "IsSnapToRaster"),       HANDLE_VIEWSET_IS_SNAP_TO_RASTER,       cppu::UnoType<bool>::get(),   PROPERTY_NONE},
        { OUString( "IsVertRulerRightAligned"),HANDLE_VIEWSET_VRULER_RIGHT         , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowContentTips" ),     HANDLE_VIEWSET_SHOW_CONTENT_TIPS      , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "RasterResolutionX"),    HANDLE_VIEWSET_RASTER_RESOLUTION_X,     cppu::UnoType<sal_Int32>::get(),     PROPERTY_NONE},
        { OUString( "RasterResolutionY"),    HANDLE_VIEWSET_RASTER_RESOLUTION_Y,     cppu::UnoType<sal_Int32>::get(),     PROPERTY_NONE},
        { OUString( "RasterSubdivisionX"),   HANDLE_VIEWSET_RASTER_SUBDIVISION_X,    cppu::UnoType<sal_Int32>::get(),     PROPERTY_NONE},
        { OUString( "RasterSubdivisionY"),   HANDLE_VIEWSET_RASTER_SUBDIVISION_Y,    cppu::UnoType<sal_Int32>::get(),     PROPERTY_NONE},
        { OUString( "ShowAnnotations" ),     HANDLE_VIEWSET_ANNOTATIONS          , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowBreaks"),           HANDLE_VIEWSET_BREAKS               , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowDrawings"),         HANDLE_VIEWSET_DRAWINGS             , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowFieldCommands"),    HANDLE_VIEWSET_FIELD_COMMANDS       , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowFootnoteBackground"),HANDLE_VIEWSET_FOOTNOTE_BACKGROUND , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowGraphics"),         HANDLE_VIEWSET_GRAPHICS             , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowHiddenCharacters"), HANDLE_VIEWSET_HIDDEN_CHARACTERS    , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "HideWhitespace"),       HANDLE_VIEWSET_HIDE_WHITESPACE      , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowHiddenParagraphs"), HANDLE_VIEWSET_HIDDEN_PARAGRAPHS    , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowHiddenText"),       HANDLE_VIEWSET_HIDDEN_TEXT          , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowHoriRuler"),        HANDLE_VIEWSET_HRULER               , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowHoriScrollBar"),    HANDLE_VIEWSET_HSCROLL              , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowIndexMarkBackground"),HANDLE_VIEWSET_INDEX_MARK_BACKGROUND, cppu::UnoType<bool>::get(),PROPERTY_NONE},
        { OUString( "ShowNonprintingCharacters"),HANDLE_VIEWSET_NONPRINTING_CHARACTERS, cppu::UnoType<bool>::get(),PROPERTY_NONE},
        { OUString( "ShowOnlineLayout"),     HANDLE_VIEWSET_ONLINE_LAYOUT        , cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID},
        { OUString( "ShowParaBreaks"),       HANDLE_VIEWSET_PARA_BREAKS          , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowProtectedSpaces"),  HANDLE_VIEWSET_PROTECTED_SPACES     , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowRulers"),           HANDLE_VIEWSET_SHOW_RULER           , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowScrollBarTips"),    HANDLE_VIEWSET_SCROLLBAR_TIPS       , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowSoftHyphens"),      HANDLE_VIEWSET_SOFT_HYPHENS         , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowSpaces"),           HANDLE_VIEWSET_SPACES               , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowTableBoundaries"),  HANDLE_VIEWSET_TABLE_BOUNDARIES     , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowTables"),           HANDLE_VIEWSET_TABLES               , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowTabstops"),         HANDLE_VIEWSET_TABSTOPS             , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowTextBoundaries"),   HANDLE_VIEWSET_TEXT_BOUNDARIES      , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowTextFieldBackground"),HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND, cppu::UnoType<bool>::get(),PROPERTY_NONE},
        { OUString( "ShowVertRuler"),        HANDLE_VIEWSET_VRULER               , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "ShowVertScrollBar"),    HANDLE_VIEWSET_VSCROLL              , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "SmoothScrolling"),      HANDLE_VIEWSET_SMOOTH_SCROLLING     , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "VerticalRulerMetric"),  HANDLE_VIEWSET_VERT_RULER_METRIC   , cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE},
        { OUString( "ZoomType"),             HANDLE_VIEWSET_ZOOM_TYPE            , cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE},
        { OUString( "ZoomValue"),            HANDLE_VIEWSET_ZOOM                 , cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE},
        { OUString(), 0, css::uno::Type(), 0 }
    };
    return new ChainablePropertySetInfo ( aViewSettingsMap_Impl );
}

static ChainablePropertySetInfo * lcl_createPrintSettingsInfo()
{
    static PropertyInfo const aPrintSettingsMap_Impl[] =
    {
        { OUString( "PrintAnnotationMode" ), HANDLE_PRINTSET_ANNOTATION_MODE , cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE},
        { OUString( "PrintBlackFonts" ),      HANDLE_PRINTSET_BLACK_FONTS        , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintControls" ),        HANDLE_PRINTSET_CONTROLS           , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintDrawings" ),        HANDLE_PRINTSET_DRAWINGS           , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintGraphics" ),        HANDLE_PRINTSET_GRAPHICS           , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintHiddenText"),       HANDLE_PRINTSET_HIDDEN_TEXT        , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintLeftPages" ),       HANDLE_PRINTSET_LEFT_PAGES         , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintPageBackground" ),  HANDLE_PRINTSET_PAGE_BACKGROUND    , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintProspect" ),        HANDLE_PRINTSET_PROSPECT           , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintProspectRTL" ),     HANDLE_PRINTSET_PROSPECT_RTL       , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintReversed" ),        HANDLE_PRINTSET_REVERSED           , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintRightPages" ),      HANDLE_PRINTSET_RIGHT_PAGES        , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintFaxName" ),         HANDLE_PRINTSET_FAX_NAME           , cppu::UnoType<OUString>::get(), PROPERTY_NONE},
        { OUString( "PrintPaperFromSetup" ),  HANDLE_PRINTSET_PAPER_FROM_SETUP   , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintTables" ),          HANDLE_PRINTSET_TABLES             , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintTextPlaceholder"),  HANDLE_PRINTSET_PLACEHOLDER        , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintSingleJobs" ),      HANDLE_PRINTSET_SINGLE_JOBS        , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString( "PrintEmptyPages" ),      HANDLE_PRINTSET_EMPTY_PAGES        , cppu::UnoType<bool>::get(), PROPERTY_NONE},
        { OUString(), 0, css::uno::Type(), 0}
    };
    return new ChainablePropertySetInfo ( aPrintSettingsMap_Impl );
}

SwXModule::SwXModule() :
    mxViewSettings(),
    mxPrintSettings()
{
}

SwXModule::~SwXModule()
{
}

Reference< XPropertySet >  SwXModule::getViewSettings() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!mxViewSettings.is())
    {
        OSL_FAIL("Web or Text?");
        mxViewSettings = static_cast < HelperBaseNoState * > ( new SwXViewSettings( false, nullptr ) );
    }
    return mxViewSettings;
}

Reference< XPropertySet >  SwXModule::getPrintSettings() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!mxPrintSettings.is())
    {
        OSL_FAIL("Web or Text?");
        mxPrintSettings = static_cast < HelperBaseNoState * > ( new SwXPrintSettings ( PRINT_SETTINGS_MODULE ) );
    }
    return mxPrintSettings;
}

OUString SwXModule::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString( "SwXModule"  );
}

sal_Bool SwXModule::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXModule::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    OUString sService( "com.sun.star.text.GlobalSettings");
    const Sequence< OUString > aSeq( &sService, 1 );
    return aSeq;
}

SwXPrintSettings::SwXPrintSettings(SwXPrintSettingsType eType, SwDoc* pDoc)
: ChainableHelperNoState ( lcl_createPrintSettingsInfo (), &Application::GetSolarMutex() )
, meType(eType)
, mpPrtOpt ( nullptr )
, mpDoc ( pDoc )
{
}

SwXPrintSettings::~SwXPrintSettings()
    throw()
{
}

void SwXPrintSettings::_preSetValues ()
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException)
{
    switch (meType)
    {
        case PRINT_SETTINGS_MODULE:
            mpPrtOpt = SW_MOD()->GetPrtOptions( false );
        break;
        case PRINT_SETTINGS_WEB:
            mpPrtOpt = SW_MOD()->GetPrtOptions( true );
        break;
        case PRINT_SETTINGS_DOCUMENT:
        {
            if (!mpDoc)
                throw IllegalArgumentException ();
            mpPrtOpt = const_cast< SwPrintData * >(&mpDoc->getIDocumentDeviceAccess().getPrintData());
        }
        break;
    }
}

void SwXPrintSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const uno::Any &rValue )
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException)
{
    bool bVal;
    if ( rInfo.mnHandle != HANDLE_PRINTSET_ANNOTATION_MODE &&
         rInfo.mnHandle != HANDLE_PRINTSET_FAX_NAME )
        bVal = *static_cast<sal_Bool const *>(rValue.getValue());

    switch( rInfo.mnHandle )
    {
        case HANDLE_PRINTSET_LEFT_PAGES:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintLeftPage(bVal);
        }
        break;
        case HANDLE_PRINTSET_RIGHT_PAGES:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintRightPage(bVal);
        }
        break;
        case HANDLE_PRINTSET_REVERSED:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintReverse(bVal);
        }
        break;
        case HANDLE_PRINTSET_PROSPECT:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintProspect(bVal);
        }
        break;
        case HANDLE_PRINTSET_GRAPHICS:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintGraphic(bVal);
        }
        break;
        case HANDLE_PRINTSET_TABLES:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintTable(bVal);
        }
        break;
        case HANDLE_PRINTSET_DRAWINGS:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintDraw(bVal);
        }
        break;
        case HANDLE_PRINTSET_CONTROLS:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintControl(bVal);
        }
        break;
        case HANDLE_PRINTSET_PAGE_BACKGROUND:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintPageBackground(bVal);
        }
        break;
        case HANDLE_PRINTSET_BLACK_FONTS:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintBlackFont(bVal);
        }
        break;
        case HANDLE_PRINTSET_SINGLE_JOBS:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintSingleJobs(bVal);
        }
        break;
        case HANDLE_PRINTSET_PAPER_FROM_SETUP:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPaperFromSetup(bVal);
        }
        break;
        case HANDLE_PRINTSET_ANNOTATION_MODE:
        {
            sal_Int16 nTmp = 0;
            rValue >>= nTmp;
            SwPostItMode nVal = static_cast<SwPostItMode>(nTmp);
            if(nVal <= SwPostItMode::EndPage)
                mpPrtOpt->SetPrintPostIts(nVal);
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case HANDLE_PRINTSET_EMPTY_PAGES:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
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
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintProspect_RTL(bVal);
        }
        break;
        case HANDLE_PRINTSET_PLACEHOLDER:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintTextPlaceholder(bVal);
        }
        break;
        case HANDLE_PRINTSET_HIDDEN_TEXT:
        {
            bVal = *static_cast<sal_Bool const *>(rValue.getValue());
            mpPrtOpt->SetPrintHiddenText(bVal);
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXPrintSettings::_postSetValues()
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException)
{
    mpPrtOpt = nullptr;
}

void SwXPrintSettings::_preGetValues()
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException)
{
    switch (meType)
    {
        case PRINT_SETTINGS_MODULE:
            mpPrtOpt = SW_MOD()->GetPrtOptions( false );
        break;
        case PRINT_SETTINGS_WEB:
            mpPrtOpt = SW_MOD()->GetPrtOptions( true );
        break;
        case PRINT_SETTINGS_DOCUMENT:
        {
            if (!mpDoc)
                throw IllegalArgumentException ();
            mpPrtOpt = const_cast< SwPrintData * >(&mpDoc->getIDocumentDeviceAccess().getPrintData());
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
    mpPrtOpt = nullptr;
}

OUString SwXPrintSettings::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXPrintSettings");
}

sal_Bool SwXPrintSettings::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXPrintSettings::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence<OUString> aRet { "com.sun.star.text.PrintSettings" };
    return aRet;
}

SwXViewSettings::SwXViewSettings(bool bWebView, SwView* pVw)
    : ChainableHelperNoState( lcl_createViewSettingsInfo (), &Application::GetSolarMutex() )
    , pView(pVw)
    , mpViewOption(nullptr)
    , mpConstViewOption(nullptr)
    , bObjectValid(true)
    , bWeb(bWebView)
    , mbApplyZoom(false)
    , eHRulerUnit(FUNIT_CM)
    , mbApplyHRulerMetric(false)
    , eVRulerUnit(FUNIT_CM)
    , mbApplyVRulerMetric(false)
{
    // This property only exists if we have a view (ie, not at the module )
    if ( !pView )
        mpInfo->remove ( "HelpURL" );

}

SwXViewSettings::~SwXViewSettings()
    throw()
{

}

void SwXViewSettings::_preSetValues ()
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException)
{
    const SwViewOption* pVOpt = nullptr;
    if(pView)
    {
        if(!IsValid())
            return;
        pVOpt = pView->GetWrtShell().GetViewOptions();
    }
    else
        pVOpt = SW_MOD()->GetViewOption(bWeb);

    mpViewOption = new SwViewOption (*pVOpt);
    mbApplyZoom = false;
    if(pView)
        mpViewOption->SetStarOneSetting(true);
}

void SwXViewSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const uno::Any &rValue )
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException, std::exception)
{
    bool bVal = HANDLE_VIEWSET_ZOOM != rInfo.mnHandle && *static_cast<sal_Bool const *>(rValue.getValue());
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
        case  HANDLE_VIEWSET_FIELD_COMMANDS        :   mpViewOption->SetFieldName(bVal);  break;
        case  HANDLE_VIEWSET_ANNOTATIONS           :   mpViewOption->SetPostIts(bVal);  break;
        case  HANDLE_VIEWSET_INDEX_MARK_BACKGROUND :   SwViewOption::SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, true);  break;
        case  HANDLE_VIEWSET_NONPRINTING_CHARACTERS:   mpViewOption->SetViewMetaChars( bVal ); break;
        case  HANDLE_VIEWSET_FOOTNOTE_BACKGROUND   :   SwViewOption::SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, true); break;
        case  HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND :   SwViewOption::SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, true);    break;
        case  HANDLE_VIEWSET_PARA_BREAKS           :   mpViewOption->SetParagraph(bVal);    break;
        case  HANDLE_VIEWSET_SOFT_HYPHENS          :   mpViewOption->SetSoftHyph(bVal); break;
        case  HANDLE_VIEWSET_SPACES                :   mpViewOption->SetBlank(bVal);    break;
        case  HANDLE_VIEWSET_PROTECTED_SPACES      :   mpViewOption->SetHardBlank(bVal);    break;
        case  HANDLE_VIEWSET_TABSTOPS              :   mpViewOption->SetTab(bVal);  break;
        case  HANDLE_VIEWSET_BREAKS                :   mpViewOption->SetLineBreak(bVal); break;
        case  HANDLE_VIEWSET_HIDDEN_TEXT           :   mpViewOption->SetShowHiddenField(bVal);  break;
        case  HANDLE_VIEWSET_HIDDEN_CHARACTERS     :   mpViewOption->SetShowHiddenChar(bVal); break;
        case  HANDLE_VIEWSET_HIDDEN_PARAGRAPHS     :   mpViewOption->SetShowHiddenPara(bVal);   break;
        case  HANDLE_VIEWSET_TABLE_BOUNDARIES      :   SwViewOption::SetAppearanceFlag(VIEWOPT_TABLE_BOUNDARIES, bVal, true);    break;
        case  HANDLE_VIEWSET_TEXT_BOUNDARIES       :   SwViewOption::SetDocBoundaries(bVal);    break;
        case  HANDLE_VIEWSET_SMOOTH_SCROLLING      :   mpViewOption->SetSmoothScroll(bVal); break;
        case  HANDLE_VIEWSET_SHOW_CONTENT_TIPS     :   mpViewOption->SetShowContentTips(bVal); break;
        case  HANDLE_VIEWSET_IS_RASTER_VISIBLE     : mpViewOption->SetGridVisible(bVal); break;
        case  HANDLE_VIEWSET_IS_SNAP_TO_RASTER     : mpViewOption->SetSnap(bVal); break;
        case  HANDLE_VIEWSET_SCROLLBAR_TIPS        : mpViewOption->SetShowScrollBarTips(bVal); break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_X   :
        {
            sal_Int32 nTmp = 0;
            if(!(rValue >>= nTmp)  ||  nTmp < 10)
                throw IllegalArgumentException();
            Size aSize( mpViewOption->GetSnapSize() );
            aSize.Width() = convertMm100ToTwip( nTmp );
            mpViewOption->SetSnapSize( aSize );
        }
        break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_Y   :
        {
            sal_Int32 nTmp = 0;
            if(!(rValue >>= nTmp)  ||  nTmp < 10)
                throw IllegalArgumentException();
            Size aSize( mpViewOption->GetSnapSize() );
            aSize.Height() = convertMm100ToTwip( nTmp );
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
            mbApplyZoom = true;
        }
        break;
        case HANDLE_VIEWSET_ZOOM_TYPE:
        {
            sal_Int16 nZoom = 0;
            if(!(rValue >>= nZoom))
                throw IllegalArgumentException();
            SvxZoomType eZoom;
            switch (nZoom)
            {
                case view::DocumentZoomType::OPTIMAL:
                    eZoom = SvxZoomType::OPTIMAL;
                break;
                case view::DocumentZoomType::PAGE_WIDTH:
                    eZoom = SvxZoomType::PAGEWIDTH;
                break;
                case view::DocumentZoomType::ENTIRE_PAGE:
                    eZoom = SvxZoomType::WHOLEPAGE;
                break;
                case view::DocumentZoomType::BY_VALUE:
                    eZoom = SvxZoomType::PERCENT;
                break;
                case view::DocumentZoomType::PAGE_WIDTH_EXACT:
                    eZoom = SvxZoomType::PAGEWIDTH_NOBORDER;
                break;
                default:
                    throw IllegalArgumentException(
                        "SwXViewSettings: invalid zoom type", nullptr, 0);
            }
            mpViewOption->SetZoomType( eZoom );
            mbApplyZoom = true;
        }
        break;
        case HANDLE_VIEWSET_ONLINE_LAYOUT :
        {
            if ( pView )
            {
                SwViewOption aOpt(*pView->GetWrtShell().GetViewOptions());
                if (!bVal != !aOpt.getBrowseMode())
                {
                    aOpt.setBrowseMode( bVal );
                    pView->GetWrtShell().ApplyViewOptions( aOpt );

                    // must be set in mpViewOption as this will overwrite settings in _post!
                    if(mpViewOption)
                        mpViewOption->setBrowseMode(bVal);

                    pView->GetDocShell()->ToggleLayoutMode(pView);
                }
            }
        }
        break;
        case HANDLE_VIEWSET_HIDE_WHITESPACE:
        {
            if ( pView )
            {
                SwViewOption aOpt(*pView->GetWrtShell().GetViewOptions());
                if (!bVal != !aOpt.IsHideWhitespaceMode())
                {
                    aOpt.SetHideWhitespaceMode( bVal );
                    pView->GetWrtShell().ApplyViewOptions( aOpt );

                    // must be set in mpViewOption as this will overwrite settings in _post!
                    if(mpViewOption)
                        mpViewOption->SetHideWhitespaceMode(bVal);
                }
            }
        }
        break;
        case HANDLE_VIEWSET_HELP_URL:
        {
            if ( pView )
            {
                OUString sHelpURL;
                if ( ! ( rValue >>= sHelpURL ) )
                    throw IllegalArgumentException();

                INetURLObject aHID( sHelpURL );
                if ( aHID.GetProtocol() == INetProtocol::Hid )
                      pView->GetEditWin().SetHelpId( OUStringToOString( aHID.GetURLPath(), RTL_TEXTENCODING_UTF8 ) );
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
            sal_uInt16 nUnit;
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
                        eHRulerUnit = static_cast<FieldUnit>(nUnit);
                        mbApplyHRulerMetric = true;
                    }
                    else
                    {
                        eVRulerUnit = static_cast<FieldUnit>(nUnit);
                        mbApplyVRulerMetric = true;
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

void SwXViewSettings::_postSetValues()
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException )
{
    if( pView )
    {
        if(mbApplyZoom )
            pView->SetZoom( (SvxZoomType)mpViewOption->GetZoomType(),
                            mpViewOption->GetZoom(), true );
        if(mbApplyHRulerMetric)
            pView->ChangeTabMetric((FieldUnit)eHRulerUnit);
        if(mbApplyVRulerMetric)
            pView->ChangeVRulerMetric((FieldUnit)eVRulerUnit);

    }
    else
    {
        if(mbApplyHRulerMetric)
            SW_MOD()->ApplyRulerMetric( (FieldUnit)eHRulerUnit, true, bWeb );
        if(mbApplyVRulerMetric)
            SW_MOD()->ApplyRulerMetric( (FieldUnit)eVRulerUnit, false, bWeb );
    }

    SW_MOD()->ApplyUsrPref( *mpViewOption, pView, pView ? VIEWOPT_DEST_VIEW_ONLY
                                                  : bWeb ? VIEWOPT_DEST_WEB
                                                          : VIEWOPT_DEST_TEXT );

    delete mpViewOption;
    mpViewOption = nullptr;
}

void SwXViewSettings::_preGetValues ()
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException)
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
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    bool bBool = true;
    bool bBoolVal = false;
    switch( rInfo.mnHandle )
    {
        case  HANDLE_VIEWSET_SHOW_RULER:                bBoolVal = mpConstViewOption->IsViewAnyRuler();   break;
        case  HANDLE_VIEWSET_HRULER :                   bBoolVal = mpConstViewOption->IsViewHRuler(true);   break;
        case  HANDLE_VIEWSET_VRULER :                   bBoolVal = mpConstViewOption->IsViewVRuler(true);break;
        case  HANDLE_VIEWSET_VRULER_RIGHT          :   bBoolVal = mpConstViewOption->IsVRulerRight();break;
        case  HANDLE_VIEWSET_HSCROLL:                   bBoolVal = mpConstViewOption->IsViewHScrollBar();break;
        case  HANDLE_VIEWSET_VSCROLL:                   bBoolVal = mpConstViewOption->IsViewVScrollBar();break;
        case  HANDLE_VIEWSET_GRAPHICS              :    bBoolVal = mpConstViewOption->IsGraphic();break;
        case  HANDLE_VIEWSET_TABLES                :   bBoolVal = mpConstViewOption->IsTable(); break;
        case  HANDLE_VIEWSET_DRAWINGS              :   bBoolVal = mpConstViewOption->IsDraw();  break;
        case  HANDLE_VIEWSET_FIELD_COMMANDS        :   bBoolVal = mpConstViewOption->IsFieldName();   break;
        case  HANDLE_VIEWSET_ANNOTATIONS           :   bBoolVal = mpConstViewOption->IsPostIts();   break;
        case  HANDLE_VIEWSET_INDEX_MARK_BACKGROUND :   bBoolVal = SwViewOption::IsFieldShadings();   break;
        case  HANDLE_VIEWSET_NONPRINTING_CHARACTERS:   bBoolVal = mpConstViewOption->IsViewMetaChars(); break;
        case  HANDLE_VIEWSET_FOOTNOTE_BACKGROUND   :   bBoolVal = SwViewOption::IsFieldShadings();  break;
        case  HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND :   bBoolVal = SwViewOption::IsFieldShadings(); break;
        case  HANDLE_VIEWSET_PARA_BREAKS           :   bBoolVal = mpConstViewOption->IsParagraph(true); break;
        case  HANDLE_VIEWSET_SOFT_HYPHENS          :   bBoolVal = mpConstViewOption->IsSoftHyph();  break;
        case  HANDLE_VIEWSET_SPACES                :   bBoolVal = mpConstViewOption->IsBlank(true); break;
        case  HANDLE_VIEWSET_PROTECTED_SPACES      :   bBoolVal = mpConstViewOption->IsHardBlank(); break;
        case  HANDLE_VIEWSET_TABSTOPS              :   bBoolVal = mpConstViewOption->IsTab(true);   break;
        case  HANDLE_VIEWSET_BREAKS                :   bBoolVal = mpConstViewOption->IsLineBreak(true); break;
        case  HANDLE_VIEWSET_HIDDEN_TEXT           :   bBoolVal = mpConstViewOption->IsShowHiddenField();   break;
        case  HANDLE_VIEWSET_HIDDEN_CHARACTERS     :   bBoolVal = mpConstViewOption->IsShowHiddenChar(true); break;
        case  HANDLE_VIEWSET_HIDE_WHITESPACE       :   bBoolVal = mpConstViewOption->IsHideWhitespaceMode(); break;
        case  HANDLE_VIEWSET_HIDDEN_PARAGRAPHS     :   bBoolVal = mpConstViewOption->IsShowHiddenPara();    break;
        case  HANDLE_VIEWSET_TABLE_BOUNDARIES      :   bBoolVal = SwViewOption::IsTableBoundaries(); break;
        case  HANDLE_VIEWSET_TEXT_BOUNDARIES       :   bBoolVal = SwViewOption::IsDocBoundaries(); break;
        case  HANDLE_VIEWSET_SMOOTH_SCROLLING      :   bBoolVal = mpConstViewOption->IsSmoothScroll();  break;
        case  HANDLE_VIEWSET_SHOW_CONTENT_TIPS     :   bBoolVal = mpConstViewOption->IsShowContentTips(); break;
        case  HANDLE_VIEWSET_IS_RASTER_VISIBLE     : bBoolVal = mpConstViewOption->IsGridVisible(); break;
        case  HANDLE_VIEWSET_IS_SNAP_TO_RASTER     : bBoolVal = mpConstViewOption->IsSnap(); break;
        case  HANDLE_VIEWSET_SCROLLBAR_TIPS        : bBoolVal = mpConstViewOption->IsShowScrollBarTips(); break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_X   :
            bBool = false;
            rValue <<= (sal_Int32) convertTwipToMm100(mpConstViewOption->GetSnapSize().Width());
        break;
        case  HANDLE_VIEWSET_RASTER_RESOLUTION_Y   :
            bBool = false;
            rValue <<= (sal_Int32) convertTwipToMm100(mpConstViewOption->GetSnapSize().Height());
        break;
        case  HANDLE_VIEWSET_RASTER_SUBDIVISION_X  :
            bBool = false;
            rValue <<= (sal_Int32) mpConstViewOption->GetDivisionX();
        break;
        case  HANDLE_VIEWSET_RASTER_SUBDIVISION_Y  :
            bBool = false;
            rValue <<= (sal_Int32) mpConstViewOption->GetDivisionY();
        break;
        case  HANDLE_VIEWSET_ZOOM                   :
                bBool = false;
                rValue <<= (sal_Int16)mpConstViewOption->GetZoom();
        break;
        case HANDLE_VIEWSET_ZOOM_TYPE:
        {
            bBool = false;
            sal_Int16 nRet(0);
            switch (mpConstViewOption->GetZoomType())
            {
                case SvxZoomType::OPTIMAL:
                    nRet = view::DocumentZoomType::OPTIMAL;
                break;
                case SvxZoomType::PAGEWIDTH:
                    nRet = view::DocumentZoomType::PAGE_WIDTH;
                break;
                case SvxZoomType::WHOLEPAGE:
                    nRet = view::DocumentZoomType::ENTIRE_PAGE;
                break;
                case SvxZoomType::PERCENT:
                    nRet = view::DocumentZoomType::BY_VALUE;
                break;
                case SvxZoomType::PAGEWIDTH_NOBORDER:
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
                bBoolVal = pView->GetWrtShell().GetViewOptions()->getBrowseMode();
        break;
        case HANDLE_VIEWSET_HELP_URL :
        {
            if ( pView )
            {
                bBool = false;
                OUStringBuffer sHelpURL;
                sHelpURL.append ( INET_HID_SCHEME );
                SwEditWin &rEditWin = pView->GetEditWin();
                sHelpURL.append( OUString::fromUtf8( rEditWin.GetHelpId() ) );
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
                pView->GetHRulerMetric( eUnit );
                rValue <<= (sal_Int32)eUnit;
            }
            else
            {
                const SwMasterUsrPref* pUsrPref = SW_MOD()->GetUsrPref( bWeb );
                rValue <<= (sal_Int32)pUsrPref->GetHScrollMetric();
            }
            bBool = false;
        }
        break;
        case HANDLE_VIEWSET_VERT_RULER_METRIC:
        {
            if ( pView )
            {
                FieldUnit eUnit;
                pView->GetVRulerMetric( eUnit );
                rValue <<= (sal_Int32)eUnit;
            }
            else
            {
                const SwMasterUsrPref* pUsrPref = SW_MOD()->GetUsrPref( bWeb );
                rValue <<= (sal_Int32)pUsrPref->GetVScrollMetric();
            }
            bBool = false;
        }
        break;
        default: OSL_FAIL("there is no such ID!");
    }
    if( bBool )
        rValue <<= bBoolVal;
}

void SwXViewSettings::_postGetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    mpConstViewOption = nullptr;
}

OUString SwXViewSettings::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXViewSettings");
}

sal_Bool SwXViewSettings::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXViewSettings::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence<OUString> aRet { "com.sun.star.text.ViewSettings" };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
SwXModule_get_implementation(css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwXModule());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <swtypes.hxx>
#include <tools/debug.hxx>
#include <unomod.hxx>

#include <errhdl.hxx>

#include <bf_svx/zoomitem.hxx>

#include <unoobj.hxx>
#include <unoprnms.hxx>
#include <unomap.hxx>
#include <bf_svtools/itemprop.hxx>
#include <prtopt.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>

#include <horiornt.hxx>

#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hdl>
#include <doc.hxx>
#include <comphelper/TypeGeneration.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/ChainablePropertySetInfo.hxx>
#include <rtl/ustrbuf.hxx>

#include "viewopt.hxx"

namespace binfilter {

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
    HANDLE_VIEWSET_HIDDEN_PARAGRAPHS,
    HANDLE_VIEWSET_HIDDEN_TEXT,
    HANDLE_VIEWSET_HRULER,
    HANDLE_VIEWSET_HSCROLL,
    HANDLE_VIEWSET_INDEX_MARK_BACKGROUND,
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
    HANDLE_VIEWSET_SHOW_RULER
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
    HANDLE_PRINTSET_SINGLE_JOBS
};

static ChainablePropertySetInfo * lcl_createViewSettingsInfo()
{
    static PropertyInfo aViewSettingsMap_Impl[] =
    {
        { RTL_CONSTASCII_STRINGPARAM ( "HelpURL" ),				HANDLE_VIEWSET_HELP_URL          	, CPPUTYPE_OUSTRING,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "IsVertRulerRightAligned"),HANDLE_VIEWSET_VRULER_RIGHT         , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "PreventHelpTips" ),     HANDLE_VIEWSET_PREVENT_TIPS         , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowAnnotations" ),     HANDLE_VIEWSET_ANNOTATIONS          , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowBreaks"),			HANDLE_VIEWSET_BREAKS               , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowDrawings"),			HANDLE_VIEWSET_DRAWINGS             , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowFieldCommands"),	HANDLE_VIEWSET_FIELD_COMMANDS       , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowFootnoteBackground"),HANDLE_VIEWSET_FOOTNOTE_BACKGROUND , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowGraphics"),			HANDLE_VIEWSET_GRAPHICS             , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHiddenParagraphs"),	HANDLE_VIEWSET_HIDDEN_PARAGRAPHS    , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHiddenText"),		HANDLE_VIEWSET_HIDDEN_TEXT          , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHoriRuler"),		HANDLE_VIEWSET_HRULER 			  	, CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowHoriScrollBar"),	HANDLE_VIEWSET_HSCROLL			    , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowIndexMarkBackground"),HANDLE_VIEWSET_INDEX_MARK_BACKGROUND, CPPUTYPE_BOOLEAN,PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowOnlineLayout"),		HANDLE_VIEWSET_ONLINE_LAYOUT		, CPPUTYPE_BOOLEAN,	PropertyAttribute::MAYBEVOID,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowParaBreaks"),		HANDLE_VIEWSET_PARA_BREAKS          , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowProtectedSpaces"),	HANDLE_VIEWSET_PROTECTED_SPACES     , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowRulers"),           HANDLE_VIEWSET_SHOW_RULER           , CPPUTYPE_BOOLEAN, PROPERTY_NONE,  0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowSoftHyphens"),		HANDLE_VIEWSET_SOFT_HYPHENS         , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowSpaces"),			HANDLE_VIEWSET_SPACES               , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTableBoundaries"),	HANDLE_VIEWSET_TABLE_BOUNDARIES     , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTables"),			HANDLE_VIEWSET_TABLES               , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTabstops"),			HANDLE_VIEWSET_TABSTOPS             , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTextBoundaries"),	HANDLE_VIEWSET_TEXT_BOUNDARIES      , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowTextFieldBackground"),HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND, CPPUTYPE_BOOLEAN,PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowVertRuler"),		HANDLE_VIEWSET_VRULER 				, CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "ShowVertScrollBar"),    HANDLE_VIEWSET_VSCROLL              , CPPUTYPE_BOOLEAN, PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "SmoothScrolling"),		HANDLE_VIEWSET_SMOOTH_SCROLLING     , CPPUTYPE_BOOLEAN,	PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "SolidMarkHandles"), 	HANDLE_VIEWSET_SOLID_MARK_HANDLES	, CPPUTYPE_BOOLEAN,	PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ZoomType"), 			HANDLE_VIEWSET_ZOOM_TYPE			, CPPUTYPE_INT16,  	PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM ( "ZoomValue"), 			HANDLE_VIEWSET_ZOOM				 	, CPPUTYPE_INT16, 	PROPERTY_NONE, 0},
        { 0, 0, 0, CPPUTYPE_UNKNOWN, 0, 0 }
    };
    return new ChainablePropertySetInfo ( aViewSettingsMap_Impl );
}
static ChainablePropertySetInfo * lcl_createPrintSettingsInfo()
{
    static PropertyInfo aPrintSettingsMap_Impl[] =
    {
        { RTL_CONSTASCII_STRINGPARAM ( "PrintAnnotationMode" ), HANDLE_PRINTSET_ANNOTATION_MODE	, CPPUTYPE_INT16,   PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintBlackFonts" ), 	 HANDLE_PRINTSET_BLACK_FONTS		, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintControls" ), 		 HANDLE_PRINTSET_CONTROLS			, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintDrawings" ), 		 HANDLE_PRINTSET_DRAWINGS			, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintGraphics" ), 		 HANDLE_PRINTSET_GRAPHICS			, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintLeftPages" ), 		 HANDLE_PRINTSET_LEFT_PAGES			, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintPageBackground" ),  HANDLE_PRINTSET_PAGE_BACKGROUND	, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintProspect" ), 		 HANDLE_PRINTSET_PROSPECT			, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintReversed" ), 		 HANDLE_PRINTSET_REVERSED   		, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintRightPages" ), 	 HANDLE_PRINTSET_RIGHT_PAGES		, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintFaxName" ), 		 HANDLE_PRINTSET_FAX_NAME			, CPPUTYPE_OUSTRING, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintPaperFromSetup" ),  HANDLE_PRINTSET_PAPER_FROM_SETUP	, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintTables" ), 		 HANDLE_PRINTSET_TABLES				, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { RTL_CONSTASCII_STRINGPARAM ( "PrintSingleJobs" ), 	 HANDLE_PRINTSET_SINGLE_JOBS		, CPPUTYPE_BOOLEAN, PROPERTY_NONE,	0},
        { 0, 0, 0, CPPUTYPE_UNKNOWN, 0, 0 }
    };
    return new ChainablePropertySetInfo ( aPrintSettingsMap_Impl );
}

/******************************************************************
 * SwXModule
 ******************************************************************/
/* -----------------30.03.99 15:10-------------------
 *
 * --------------------------------------------------*/
Reference< uno::XInterface > SAL_CALL SwXModule_createInstance(
    const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    static Reference< uno::XInterface >  xModule = (cppu::OWeakObject*)new SwXModule();;
    return xModule;
}
/* -----------------------------17.04.01 13:11--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SAL_CALL SwXModule_getSupportedServiceNames() throw()
{
    OUString sService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.GlobalSettings"));
    const Sequence< OUString > aSeq( &sService, 1 );
    return aSeq;
}
/* -----------------------------17.04.01 13:11--------------------------------

 ---------------------------------------------------------------------------*/
OUString SAL_CALL SwXModule_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SwXModule" ) );
}
/*-- 17.12.98 12:19:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXModule::SwXModule() :
    pxViewSettings(0),
    pxPrintSettings(0)
{
}
/*-- 17.12.98 12:19:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXModule::~SwXModule()
{
    delete pxViewSettings;
    delete pxPrintSettings;
}
/*-- 17.12.98 12:19:03---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySet >  SwXModule::getViewSettings(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pxViewSettings)
    {
        ((SwXModule*)this)->pxViewSettings = new Reference< XPropertySet > ;
        DBG_ERROR("Web oder Text?");
        *pxViewSettings = static_cast < HelperBaseNoState * > ( new SwXViewSettings( sal_False, 0 ) );
    }
    return *pxViewSettings;
}
/*-- 17.12.98 12:19:03---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySet >  SwXModule::getPrintSettings(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pxPrintSettings)
    {
        ((SwXModule*)this)->pxPrintSettings = new Reference< XPropertySet > ;
        DBG_ERROR("Web oder Text?");
        *pxPrintSettings = static_cast < HelperBaseNoState * > ( new SwXPrintSettings ( PRINT_SETTINGS_MODULE ) );
    }
    return *pxPrintSettings;
}
/* -----------------------------06.04.00 10:59--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXModule::getImplementationName(void) throw( RuntimeException )
{
    return SwXModule_getImplementationName();
}
/* -----------------------------06.04.00 10:59--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXModule::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    const Sequence< OUString > aNames = SwXModule_getSupportedServiceNames();
    for(sal_Int32 nService = 0; nService < aNames.getLength(); nService++)
    {
        if(aNames.getConstArray()[nService] == rServiceName)
            return TRUE;
    }
    return FALSE;
}
/* -----------------------------06.04.00 10:59--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXModule::getSupportedServiceNames(void) throw( RuntimeException )
{
    return SwXModule_getSupportedServiceNames();
}

/******************************************************************
 * SwXPrintSettings
 ******************************************************************/
/*-- 17.12.98 12:54:04---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXPrintSettings::SwXPrintSettings(SwXPrintSettingsType eType, SwDoc* pDoc)
: ChainableHelperNoState ( lcl_createPrintSettingsInfo (), &Application::GetSolarMutex() )
, meType(eType)
, mpPrtOpt ( NULL )
, mpDoc ( pDoc )
{
}
/*-- 17.12.98 12:54:05---------------------------------------------------

  -----------------------------------------------------------------------*/
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
            if ( !mpDoc->GetPrintData() )
            {
                mpPrtOpt = new SwPrintData;
                mpDoc->SetPrintData ( *mpPrtOpt );
                delete mpPrtOpt;
            }
            mpPrtOpt = mpDoc->GetPrintData();
        }
        break;
    }
}

void SwXPrintSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue )
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
            sal_Int16 nVal;
            rValue >>= nVal;
            if(nVal <= text::NotePrintMode_PAGE_END)
                mpPrtOpt->SetPrintPostIts(nVal);
            else
                throw lang::IllegalArgumentException();
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
            if ( !mpDoc->GetPrintData() )
            {
                mpPrtOpt = new SwPrintData;
                mpDoc->SetPrintData ( *mpPrtOpt );
                delete mpPrtOpt;
            }
            mpPrtOpt = mpDoc->GetPrintData();
        }
        break;
    }
}
void SwXPrintSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
    throw(UnknownPropertyException, WrappedTargetException )
{
    sal_Bool bBool = TRUE;
    sal_Bool bBoolVal;
    switch( rInfo.mnHandle )
    {
        case HANDLE_PRINTSET_LEFT_PAGES:
            bBoolVal = mpPrtOpt->IsPrintLeftPage();
        break;
        case HANDLE_PRINTSET_RIGHT_PAGES:
            bBoolVal = mpPrtOpt->IsPrintRightPage();
        break;
        case HANDLE_PRINTSET_REVERSED:
            bBoolVal = mpPrtOpt->IsPrintReverse();
        break;
        case HANDLE_PRINTSET_PROSPECT:
            bBoolVal = mpPrtOpt->IsPrintProspect();
        break;
        case HANDLE_PRINTSET_GRAPHICS:
            bBoolVal = mpPrtOpt->IsPrintGraphic();
        break;
        case HANDLE_PRINTSET_TABLES:
            bBoolVal = mpPrtOpt->IsPrintTable();
        break;
        case HANDLE_PRINTSET_DRAWINGS:
            bBoolVal = mpPrtOpt->IsPrintDraw();
        break;
        case HANDLE_PRINTSET_CONTROLS:
            bBoolVal = mpPrtOpt->IsPrintControl();
        break;
        case HANDLE_PRINTSET_PAGE_BACKGROUND:
            bBoolVal = mpPrtOpt->IsPrintPageBackground();
        break;
        case HANDLE_PRINTSET_BLACK_FONTS:
            bBoolVal = mpPrtOpt->IsPrintBlackFont();
        break;
        case HANDLE_PRINTSET_SINGLE_JOBS:
            bBoolVal = mpPrtOpt->IsPrintSingleJobs();
        break;
        case HANDLE_PRINTSET_PAPER_FROM_SETUP:
            bBoolVal = mpPrtOpt->IsPaperFromSetup();
        break;
        case HANDLE_PRINTSET_ANNOTATION_MODE:
        {
            bBool = FALSE;
            rValue <<= static_cast < sal_Int16 > ( mpPrtOpt->GetPrintPostIts() );
        }
        break;
        case HANDLE_PRINTSET_FAX_NAME :
        {
            bBool = FALSE;
            rValue <<= mpPrtOpt->GetFaxName();
        }
        break;
        default:
            throw UnknownPropertyException();
    }
    if(bBool)
        rValue.setValue(&bBoolVal, ::getBooleanCppuType());
}
void SwXPrintSettings::_postGetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    mpPrtOpt = NULL;
}
/* -----------------------------06.04.00 11:02--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXPrintSettings::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXPrintSettings");
}
/* -----------------------------06.04.00 11:02--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXPrintSettings::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.PrintSettings") == rServiceName;
}
/* -----------------------------06.04.00 11:02--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXPrintSettings::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.PrintSettings");
    return aRet;
}


/******************************************************************
 *
 ******************************************************************/
/*-- 18.12.98 11:01:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXViewSettings::SwXViewSettings(sal_Bool bWebView, SwView*	pVw)
: ChainableHelperNoState( lcl_createViewSettingsInfo (), &Application::GetSolarMutex() )
, pView(pVw)
, bWeb(bWebView)
, bObjectValid(sal_True)
, mpViewOption ( NULL )
, mpConstViewOption ( NULL )
{
    // This property only exists if we have a view (ie, not at the module )
    if ( !pView )
        mpInfo->remove ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "HelpURL" ) ) );

}
/*-- 18.12.98 11:01:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXViewSettings::~SwXViewSettings()
    throw()
{

}
void SwXViewSettings::_preSetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    sal_Bool bApply = sal_True;
    sal_Bool bApplyZoom = sal_False;

    const SwViewOption* pVOpt = 0;
    pVOpt = SW_MOD()->GetViewOption(bWeb);

    mpViewOption = new SwViewOption (*pVOpt);
    mbApplyZoom = sal_False;
    if(pView)
        mpViewOption->SetStarOneSetting(sal_True);
}
void SwXViewSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue )
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
        case  HANDLE_VIEWSET_HSCROLL			   :   mpViewOption->SetViewHScrollBar(bVal);break;
        case  HANDLE_VIEWSET_VSCROLL			   :   mpViewOption->SetViewVScrollBar(bVal);break;
        case  HANDLE_VIEWSET_GRAPHICS              :   mpViewOption->SetGraphic(bVal);break;
        case  HANDLE_VIEWSET_TABLES                :   mpViewOption->SetTable(bVal);	break;
        case  HANDLE_VIEWSET_DRAWINGS              :   mpViewOption->SetDraw(bVal);	break;
        case  HANDLE_VIEWSET_FIELD_COMMANDS        :   mpViewOption->SetFldName(bVal);	break;
        case  HANDLE_VIEWSET_ANNOTATIONS           :   mpViewOption->SetPostIts(bVal);	break;
        case  HANDLE_VIEWSET_INDEX_MARK_BACKGROUND :   mpViewOption->SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, TRUE);  break;
        case  HANDLE_VIEWSET_FOOTNOTE_BACKGROUND   :   mpViewOption->SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, TRUE); break;
        case  HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND :   mpViewOption->SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bVal, TRUE);    break;
        case  HANDLE_VIEWSET_PARA_BREAKS           :   mpViewOption->SetParagraph(bVal);	break;
        case  HANDLE_VIEWSET_SOFT_HYPHENS          :   mpViewOption->SetSoftHyph(bVal);	break;
        case  HANDLE_VIEWSET_SPACES                :   mpViewOption->SetBlank(bVal);	break;
        case  HANDLE_VIEWSET_PROTECTED_SPACES      :   mpViewOption->SetHardBlank(bVal);	break;
        case  HANDLE_VIEWSET_TABSTOPS              :   mpViewOption->SetTab(bVal);	break;
        case  HANDLE_VIEWSET_BREAKS                :   mpViewOption->SetLineBreak(bVal); break;
        case  HANDLE_VIEWSET_HIDDEN_TEXT           :   mpViewOption->SetHidden(bVal);	break;
        case  HANDLE_VIEWSET_HIDDEN_PARAGRAPHS     :   mpViewOption->SetShowHiddenPara(bVal);	break;
        case  HANDLE_VIEWSET_TABLE_BOUNDARIES      :   mpViewOption->SetAppearanceFlag(VIEWOPT_TABLE_BOUNDARIES, bVal, TRUE);    break;
        case  HANDLE_VIEWSET_TEXT_BOUNDARIES       :   mpViewOption->SetDocBoundaries(bVal);    break;
        case  HANDLE_VIEWSET_SMOOTH_SCROLLING      :   mpViewOption->SetSmoothScroll(bVal);	break;
        case  HANDLE_VIEWSET_SOLID_MARK_HANDLES    :   mpViewOption->SetSolidMarkHdl(bVal);	break;
        case  HANDLE_VIEWSET_PREVENT_TIPS :            mpViewOption->SetPreventTips(bVal); break;
        break;
        case  HANDLE_VIEWSET_ZOOM                   :
        {
            sal_Int16 nZoom;
            if(!(rValue >>= nZoom) || nZoom > 1000 || nZoom < 5)
                throw lang::IllegalArgumentException();
            mpViewOption->SetZoom((sal_uInt16)nZoom);
            mbApplyZoom = sal_True;
        }
        break;
        case HANDLE_VIEWSET_ZOOM_TYPE:
        {
            sal_Int16 nZoom;
            if(!(rValue >>= nZoom))
                throw IllegalArgumentException();
            SvxZoomType eZoom = (SvxZoomType)USHRT_MAX;
            switch(nZoom)
            {
                case /*DocumentZoomType_OPTIMAL 	  */0:
                    eZoom = SVX_ZOOM_OPTIMAL;
                break;
                case /*DocumentZoomType_PAGE_WIDTH 	*/  1:
                    eZoom = SVX_ZOOM_PAGEWIDTH;
                break;
                case /*DocumentZoomType_ENTIRE_PAGE */  2:
                    eZoom = SVX_ZOOM_WHOLEPAGE;
                break;
                case /*DocumentZoomType_BY_VALUE	*/  3:
                    eZoom = SVX_ZOOM_PERCENT;
                break;
                case /*DocumentZoomType_PAGE_WIDTH_EXACT */ 4:
                    eZoom = SVX_ZOOM_PAGEWIDTH_NOBORDER;
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
/*?*/           DBG_BF_ASSERT(0, "STRIP");
        }
        break;
        case HANDLE_VIEWSET_HELP_URL :
        {
            throw UnknownPropertyException();
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXViewSettings::_postSetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    SW_MOD()->ApplyUsrPref( *mpViewOption, pView, pView ? VIEWOPT_DEST_VIEW_ONLY
                                                  : bWeb ? VIEWOPT_DEST_WEB
                                                          : VIEWOPT_DEST_TEXT );
    delete mpViewOption;
    mpViewOption = NULL;
}

void SwXViewSettings::_preGetValues ()
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    mpConstViewOption = SW_MOD()->GetViewOption(bWeb);
}

void SwXViewSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
    throw(UnknownPropertyException, WrappedTargetException )
{
    sal_Bool bBool = TRUE;
    sal_Bool bBoolVal;
    switch( rInfo.mnHandle )
    {
        case  HANDLE_VIEWSET_SHOW_RULER:                bBoolVal = mpConstViewOption->IsViewAnyRuler();   break;
        case  HANDLE_VIEWSET_HRULER :                   bBoolVal = mpConstViewOption->IsViewHRuler(TRUE);   break;
        case  HANDLE_VIEWSET_VRULER :                   bBoolVal = mpConstViewOption->IsViewVRuler(TRUE);break;
        case  HANDLE_VIEWSET_VRULER_RIGHT          :   bBoolVal = mpConstViewOption->IsVRulerRight();break;
        case  HANDLE_VIEWSET_HSCROLL:                   bBoolVal = mpConstViewOption->IsViewHScrollBar();break;
        case  HANDLE_VIEWSET_VSCROLL: 					bBoolVal = mpConstViewOption->IsViewVScrollBar();break;
        case  HANDLE_VIEWSET_GRAPHICS              :	bBoolVal = mpConstViewOption->IsGraphic();break;
        case  HANDLE_VIEWSET_TABLES                :   bBoolVal = mpConstViewOption->IsTable();	break;
        case  HANDLE_VIEWSET_DRAWINGS              :   bBoolVal = mpConstViewOption->IsDraw();	break;
        case  HANDLE_VIEWSET_FIELD_COMMANDS        :   bBoolVal = mpConstViewOption->IsFldName();	break;
        case  HANDLE_VIEWSET_ANNOTATIONS           :   bBoolVal = mpConstViewOption->IsPostIts();	break;
        case  HANDLE_VIEWSET_INDEX_MARK_BACKGROUND :   bBoolVal = SwViewOption::IsFieldShadings();   break;
        case  HANDLE_VIEWSET_FOOTNOTE_BACKGROUND   :   bBoolVal = SwViewOption::IsFieldShadings();  break;
        case  HANDLE_VIEWSET_TEXT_FIELD_BACKGROUND :   bBoolVal = SwViewOption::IsFieldShadings(); break;
        case  HANDLE_VIEWSET_PARA_BREAKS           :   bBoolVal = mpConstViewOption->IsParagraph(sal_True);	break;
        case  HANDLE_VIEWSET_SOFT_HYPHENS          :   bBoolVal = mpConstViewOption->IsSoftHyph();	break;
        case  HANDLE_VIEWSET_SPACES                :   bBoolVal = mpConstViewOption->IsBlank(sal_True);	break;
        case  HANDLE_VIEWSET_PROTECTED_SPACES      :   bBoolVal = mpConstViewOption->IsHardBlank();	break;
        case  HANDLE_VIEWSET_TABSTOPS              :   bBoolVal = mpConstViewOption->IsTab(sal_True);	break;
        case  HANDLE_VIEWSET_BREAKS                :   bBoolVal = mpConstViewOption->IsLineBreak(sal_True); break;
        case  HANDLE_VIEWSET_HIDDEN_TEXT           :   bBoolVal = mpConstViewOption->IsHidden();	break;
        case  HANDLE_VIEWSET_HIDDEN_PARAGRAPHS     :   bBoolVal = mpConstViewOption->IsShowHiddenPara();	break;
        case  HANDLE_VIEWSET_TABLE_BOUNDARIES      :   bBoolVal = SwViewOption::IsTableBoundaries(); break;
        case  HANDLE_VIEWSET_TEXT_BOUNDARIES       :   bBoolVal = SwViewOption::IsDocBoundaries(); break;
        case  HANDLE_VIEWSET_SMOOTH_SCROLLING      :   bBoolVal = mpConstViewOption->IsSmoothScroll();	break;
        case  HANDLE_VIEWSET_SOLID_MARK_HANDLES    :   bBoolVal = mpConstViewOption->IsSolidMarkHdl();	break;
        case  HANDLE_VIEWSET_PREVENT_TIPS :            bBoolVal = mpConstViewOption->IsPreventTips(); break;
        case  HANDLE_VIEWSET_ZOOM                   :
                bBool = FALSE;
                rValue <<= (sal_Int16)mpConstViewOption->GetZoom();
        break;
        case HANDLE_VIEWSET_ZOOM_TYPE:
        {
            bBool = FALSE;
            sal_Int16 nRet;
            switch(mpConstViewOption->GetZoomType())
            {
                case SVX_ZOOM_OPTIMAL:
                    nRet = /*DocumentZoomType_OPTIMAL*/ 0;
                break;
                case SVX_ZOOM_PAGEWIDTH:
                    nRet = /*DocumentZoomType_PAGE_WIDTH 	*/1;
                break;
                case SVX_ZOOM_WHOLEPAGE:
                    nRet = /*DocumentZoomType_ENTIRE_PAGE */  2;
                break;
                case SVX_ZOOM_PERCENT:
                    nRet = /*DocumentZoomType_BY_VALUE	*/  3;
                break;
            }
            rValue <<= nRet;
        }
        break;
        case HANDLE_VIEWSET_ONLINE_LAYOUT:
            DBG_BF_ASSERT(0, "STRIP");
        break;
        case HANDLE_VIEWSET_HELP_URL :
        {
            DBG_BF_ASSERT(0, "STRIP");
        }
        break;
        default: DBG_ERROR("Diese Id gibt's nicht!");
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

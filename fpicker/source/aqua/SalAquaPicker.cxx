/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SalAquaPicker.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-11 10:59:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef  _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _FPSERVICEINFO_HXX_
#include <FPServiceInfo.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SALAQUAPICKER_HXX_
#include "SalAquaPicker.hxx"
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif

#include <stdio.h>

#pragma mark DEFINES
#define CLASS_NAME "SalAquaPicker"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

void navigationEventHandler(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUserData)
{
    NavReplyRecord reply;
    NavUserAction userAction = 0;
    SalAquaPicker *pSalAquaPicker = (SalAquaPicker *) callBackUserData;

    switch (callBackSelector) {

    // We are ignoring several callbackSelectors here...
    case kNavCBEvent:
        pSalAquaPicker->implHandleNavDialogEvent(callBackParms);
        break;
    case kNavCBCustomize:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBCustomize");
        pSalAquaPicker->implHandleNavDialogCustomize(callBackParms);
        break;
    case kNavCBStart:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBStart");

//leave the following in in case we should consider X11 support in the future
#ifndef QUARTZ
        /*
         * We need to bring the dialog to the front. However, to make
         * the dialog usable, soffice.bin must be the application bundle's executable.
         */
        if (callBackParms) {
            WindowRef dialogWindow = callBackParms->window;
            if (dialogWindow) {
                ProcessSerialNumber psn;
                OSStatus error = GetCurrentProcess(&psn);
                if (error == noErr) {
                    (void)SetFrontProcess(&psn);
                }
                BringToFront(dialogWindow);
            }
        }
#endif
        pSalAquaPicker->implHandleNavDialogStart(callBackParms);
        break;
    case kNavCBAdjustRect:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBAdjustRect");
        pSalAquaPicker->getControlHelper()->handleAdjustRect(callBackParms);
        break;
    case kNavCBNewLocation:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBNewLocation");
        break;
    case kNavCBAccept:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBAccept");
        break;
    case kNavCBCancel:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBCancel");
        break;
    case kNavCBAdjustPreview:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBAdjustPreview");
        break;
    case kNavCBPopupMenuSelect:
        NavMenuItemSpec* menuItem = (NavMenuItemSpec*)callBackParms->eventData.eventDataParms.param;
        pSalAquaPicker->implHandlePopupMenuSelect(menuItem);
        break;
    case kNavCBUserAction:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBUserAction");

         if (NavDialogGetReply (callBackParms->context, &reply) == noErr )
         {
             userAction = NavDialogGetUserAction (callBackParms->context);

             switch (userAction) {
             case kNavUserActionSaveAs:
                 OSL_TRACE("NavigationServices userAction = knavUserActionSaveAs");
                 break;
             case kNavUserActionOpen:
                 OSL_TRACE("NavigationServices userAction = knavUserActionOpen");
                 break;
             case kNavUserActionCancel:
                 OSL_TRACE("NavigationServices userAction = knavUserActionCancel");
                 break;
             case kNavUserActionNewFolder:
                 OSL_TRACE("NavigationServices userAction = knavUserActionNewFolder");
                 break;
             default:
                 OSL_TRACE("NavigationServices userAction is UNKNOWN %d", userAction);
                 break;
             }

             NavDisposeReply (&reply);
         }
        break;
    case kNavCBTerminate:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBTerminate");
//leave the following in in case we should consider X11 support in the future
#ifndef QUARTZ
        {
            /*
             * This passage searches the process list to find the X11 application.
             * When it is found, it is being made the front process. Otherwise
             * OpenOffice.org app would stay on top and the X11 window might be hidden.
             */
            OSStatus status;
            ProcessSerialNumber psn = {0, kNoProcess};//to initialize the process list
            while((status = GetNextProcess(&psn)) == noErr && (psn.lowLongOfPSN != kNoProcess)) {
                CFStringRef processName;
                CopyProcessName(&psn, &processName);
                CFStringRef sX11 = CFSTR("X11");
                if (CFStringCompare(processName, sX11, NULL) == kCFCompareEqualTo) {
                    (void)SetFrontProcess(&psn);
                    CFRelease(processName);
                    CFRelease(sX11);
                    break;
                }
                CFRelease(processName);
                CFRelease(sX11);
            }
        }
#endif
        break;
    case kNavCBSelectEntry:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBSelectEntry");
        pSalAquaPicker->implHandleNavDialogSelectEntry(callBackParms);
        break;
    case kNavCBOpenSelection:
        OSL_TRACE("NavigationServices callBackSelector = kNavCBOpenSelection");
        //no need to do anythong here, sfx2 will call us
        break;
    default:
        OSL_TRACE("NavigationServices callBackSelector = %d", (int)callBackSelector);
        break;
    }

    pSalAquaPicker->setLatestEvent(callBackSelector);
}

MacOSBoolean filterEventHandler (AEDesc *theItem, void *info,
                      void *callBackUD,
                      NavFilterModes filterMode )
{
    SalAquaPicker *pPicker = (SalAquaPicker *) callBackUD;
    return pPicker->implFilterHandler(theItem, info, callBackUD, filterMode);
}

MacOSBoolean previewHandler (NavCBRecPtr callBackParms,
                       NavCallBackUserData callBackUD )
{
    SalAquaPicker *pSalAquaPicker = (SalAquaPicker *) callBackUD;

    if (NULL != pSalAquaPicker) {
        return pSalAquaPicker->implPreviewHandler(callBackParms);
    }

    return FALSE;
}

OSStatus SalAquaPicker::run()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (m_pDialog == NULL) {
        //this is the case e.g. for the folder picker at this stage
        implInitialize();
    }

    OSStatus status = NavDialogRun (m_pDialog);
    if (status != noErr) {
        OSL_TRACE("NavigationServices returned an error while running the dialog");
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, status);

    return status;
}

OSStatus SalAquaPicker::runandwaitforresult()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    OSStatus status = this->run();
    if (status != noErr) {
        DBG_PRINT_EXIT(CLASS_NAME, __func__, status);
        return status;
    }
    status = NavDialogGetReply (m_pDialog, &m_pReplyRecord);
    if (status != noErr) {
        OSL_TRACE("NavigationServices returned an error while getting the dialog reply");
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, status);
    return status;
}

// constructor
SalAquaPicker::SalAquaPicker()
: m_pDialog(NULL)
, m_pControlHelper(new ControlHelper())
, m_aLatestEvent(kNavCBEvent)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    // set the standard set of dialog options
    OSStatus status = NavGetDefaultDialogCreationOptions(&m_pDialogOptions);
    if (status != noErr) {
        OSL_TRACE("NavigationServices returned an error while creating dialog options");
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

SalAquaPicker::~SalAquaPicker()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL != m_pControlHelper)
        delete m_pControlHelper;
    if (NULL != m_pEventHandler)
        DisposeNavEventUPP(m_pEventHandler);
    if (NULL != m_pDialog)
        NavDialogDispose (m_pDialog);
    if (NULL != m_pFilterHandler)
        DisposeNavObjectFilterUPP(m_pFilterHandler);
    if (NULL != m_pPreviewHandler)
        DisposeNavPreviewUPP(m_pPreviewHandler);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaPicker::implsetDisplayDirectory( const rtl::OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "directory", aDirectory);

    m_sDisplayDirectory = aDirectory;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaPicker::implgetDisplayDirectory() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__, m_sDisplayDirectory);

    return m_sDisplayDirectory;
}

void SAL_CALL SalAquaPicker::implsetTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    m_pDialogOptions.windowTitle = CFStringCreateWithOUString ( aTitle );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaPicker::implInitialize()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pEventHandler = NewNavEventUPP( navigationEventHandler );

    //does not work currently - unfortunately
    //WindowRef parent = FrontWindow();
    WindowRef parent = NULL;
    if (NULL == parent) {
        m_pDialogOptions.modality = kWindowModalityAppModal;
    } else if (m_nDialogType == NAVIGATIONSERVICES_SAVE) {
        m_pDialogOptions.modality = kWindowModalityWindowModal;
        m_pDialogOptions.parentWindow = parent;
    }

    OSStatus status = noErr;
    // Create the corresponding dialog
    // pass the pointer to SalAquaPicker as an userData
    switch (m_nDialogType)
    {
        case NAVIGATIONSERVICES_OPEN:
            OSL_TRACE("NAVIGATIONSERVICES_OPEN");
            m_pFilterHandler = NewNavObjectFilterUPP(filterEventHandler);
            m_pPreviewHandler = NewNavPreviewUPP(previewHandler);
            status = NavCreateGetFileDialog (&m_pDialogOptions, NULL, m_pEventHandler, m_pPreviewHandler, m_pFilterHandler, (void *) this, &m_pDialog);
            break;

        case NAVIGATIONSERVICES_SAVE:
            OSL_TRACE("NAVIGATIONSERVICES_SAVE");
            status = NavCreatePutFileDialog (&m_pDialogOptions, kUnknownType, kUnknownType, m_pEventHandler, (void *) this, &m_pDialog);
            break;

        case NAVIGATIONSERVICES_DIRECTORY:
            OSL_TRACE("NAVIGATIONSERVICES_DIRECTORY");
            status = NavCreateChooseFolderDialog (&m_pDialogOptions, m_pEventHandler, NULL, (void *) this, &m_pDialog);
            break;

        default:
            OSL_TRACE("m_nDialogType is UNKNOWN: %d", m_nDialogType);
            break;
    }

    if (status != noErr) {
        OSL_TRACE("An error occurred while creating the dialog!");
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

/////
void SAL_CALL SalAquaPicker::implHandleNavDialogCustomize(NavCBRecPtr callBackParms) { }

void SAL_CALL SalAquaPicker::implHandleNavDialogStart(NavCBRecPtr callBackParms) { }

void SAL_CALL SalAquaPicker::implHandleNavDialogEvent(NavCBRecPtr callBackParms) { }

sal_Bool SAL_CALL SalAquaPicker::implFilterHandler(AEDesc *theItem, void *info,
                                                   void *callBackUD,
                                                   NavFilterModes filterMode)
{
    return sal_True;
}

sal_Bool SAL_CALL SalAquaPicker::implPreviewHandler(NavCBRecPtr callBackParms)
{
    return sal_False;
}

void SAL_CALL SalAquaPicker::implHandlePopupMenuSelect(NavMenuItemSpec* menuItem) { }

void SAL_CALL SalAquaPicker::implHandleNavDialogSelectEntry(NavCBRecPtr callBackParms) { }

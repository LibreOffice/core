/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SalAquaFilePicker.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-11 10:59:01 $
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
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_CONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
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
#ifndef _SALAQUAFILEPICKER_HXX_
#include "SalAquaFilePicker.hxx"
#endif
#ifndef _TOOLS_URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <iostream>
#include "resourceprovider.hxx"
#ifndef _SV_RC_H
#include <tools/rc.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif

#pragma mark DEFINES

#define LABEL_TOGGLE( elem ) \
case elem : \
aLabel = aResProvider.getResString( CHECKBOX_##elem ); \
    setLabel( CHECKBOX_##elem, aLabel ); \
    break

#define CLASS_NAME "SalAquaFilePicker"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controling event notifications
    const bool STARTUP_SUSPENDED = true;
    const bool STARTUP_ALIVE     = false;

    uno::Sequence<rtl::OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(3);
        aRet[0] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FilePicker" );
        aRet[1] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.SystemFilePicker" );
        aRet[2] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.AquaFilePicker" );
        return aRet;
    }
}

#pragma mark Constructor
//-----------------------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------------------

SalAquaFilePicker::SalAquaFilePicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr ) :
cppu::WeakComponentImplHelper9<XFilterManager, XFilterGroupManager, XFilePickerControlAccess, XFilePickerNotifier, XFilePreview, lang::XInitialization, util::XCancellable, lang::XEventListener, lang::XServiceInfo>( m_rbHelperMtx )
, m_xServiceMgr( xServiceMgr )
, m_pFilterHelper( NULL )
, bVersionWidthUnset( false )
, mbPreviewState( sal_False )
, m_PreviewImageWidth( 256 )
, m_PreviewImageHeight( 256 )
, m_bFilterUICorrectlySet( sal_False )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

SalAquaFilePicker::~SalAquaFilePicker()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL != m_pFilterHelper)
        delete m_pFilterHelper;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}


#pragma mark XFilePickerNotifier
//------------------------------------------------------------------------------------
// XFilePickerNotifier
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_xListener = xListener;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
    throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_xListener.clear();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XExecutableDialog
//-----------------------------------------------------------------------------------------
// XExecutableDialog functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::setTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    implsetTitle(aTitle);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

sal_Int16 SAL_CALL SalAquaFilePicker::execute() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_Int16 retVal = 0;

    implInitialize();

    if (m_pFilterHelper) {
        m_pFilterHelper->SetFilters();
    }

    if (m_nDialogType == NAVIGATIONSERVICES_SAVE) {
        if (m_sSaveFileName.getLength() == 0) {
            //if no filename is set, NavigationServices will set the name to "untitled". We don't want this!
            //So let's try to get the window title to get the real untitled name
            WindowRef frontWindow = FrontWindow();
            if (NULL != frontWindow) {
                CFStringRef sDefaultName = NULL;
                if (CopyWindowTitleAsCFString(frontWindow, &sDefaultName) == noErr) {
                    rtl::OUString ouName = CFStringToOUString(sDefaultName);
                    CFRelease(sDefaultName);
                    //a window title will typically be something like "Untitled1 - OpenOffice.org Writer"
                    //but we only want the "Untitled1" part of it
                    sal_Int32 indexOfDash = ouName.indexOf(rtl::OUString::createFromAscii(" - "));
                    if (indexOfDash > -1) {
                        m_sSaveFileName = ouName.copy(0,indexOfDash);
                    } else {
                        OSL_TRACE("no dash present in window title");
                    }
                } else {
                    OSL_TRACE("couldn't get window title");
                }
            } else {
                OSL_TRACE("no front window found");
            }
        }
        if (m_sSaveFileName.getLength() > 0) {
            setDefaultName(m_sSaveFileName, true);
        }
    }

    OSStatus nStatus = runandwaitforresult();

    switch( nStatus )
    {
        case noErr:
            OSL_TRACE("The dialog returned OK");
            retVal = ExecutableDialogResults::OK;
            break;

        case userCanceledErr:
            OSL_TRACE("The dialog was cancelled by the user!");
            retVal = ExecutableDialogResults::CANCEL;
            break;

        default:
            retVal = 0;
            break;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}


#pragma mark XFilePicker
//-----------------------------------------------------------------------------------------
// XFilePicker functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::setMultiSelectionMode( sal_Bool bMode ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "multiSelectable?", bMode);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if (bMode == sal_True)
        m_pDialogOptions.optionFlags |= kNavAllowMultipleFiles;
    else
        m_pDialogOptions.optionFlags &= ~kNavAllowMultipleFiles;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setDefaultName( const rtl::OUString& aName )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "name", aName);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    setDefaultName(aName, false);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setDisplayDirectory( const rtl::OUString& rDirectory )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "directory", rDirectory);

    implsetDisplayDirectory(rDirectory);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaFilePicker::getDisplayDirectory() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    rtl::OUString retVal = implgetDisplayDirectory();

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);
    return retVal;
}

uno::Sequence<rtl::OUString> SAL_CALL SalAquaFilePicker::getFiles() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    AEDescList aFileList;
    OSStatus status = noErr;

    FSRef fileAsFSRef;
    long nFiles = 0;
    short nIndex;

    if (m_pReplyRecord.validRecord == TRUE) {
        status = AECountItems(&m_pReplyRecord.selection, &nFiles);
    }
    if (status != noErr || nFiles == 0) {
        status = NavCustomControl(m_pDialog, kNavCtlGetSelection, &aFileList);
        if (status != noErr) {
            OSL_TRACE("NavigationServices couldn't get the current selection");
            return NULL;
        } else {
            status = AECountItems(&aFileList, &nFiles);
            if (status != noErr || nFiles == 0) {
                OSL_TRACE("The current selection is empty");
                return NULL;
            }
        }
    } else {
        aFileList = m_pReplyRecord.selection;
    }

    OSL_TRACE("starting work");
    /*
     * If more than one file is selected in an OpenDialog, then the first result
     * is the directory and the remaining results contain just the files' names
     * without the basedir path.
     */
    uno::Sequence< rtl::OUString > aSelectedFiles(nFiles > 1 ? nFiles + 1 : nFiles);

    for (nIndex = 1; nIndex <= nFiles; nIndex++) {
        // Get next file
        status = AEGetNthPtr(&aFileList, nIndex, typeFSRef, NULL, NULL, &fileAsFSRef, sizeof(FSRef), NULL);

        if (status != noErr) {
            continue;
        }

        /*
         * default setting: extract the full path
         * In a SaveDialog this is the directory where the file should be stored
         */
        InfoType info = FULLPATH;
        if (nFiles > 1) {
            //just get the file's name (only in OpenDialog)
            info = FILENAME;
        }
        OUString sFileOrDirURL = FSRefToOUString(fileAsFSRef, info);

        switch(m_nDialogType) {
            case NAVIGATIONSERVICES_OPEN:
            {
                OSL_TRACE("NAVIGATIONSERVICES_OPEN");

                //get the directory information, only on the first file processed
                if (nIndex == 1) {
                    OUString sDirectoryURL = FSRefToOUString(fileAsFSRef, PATHWITHOUTLASTCOMPONENT);

                    if (nFiles > 1) {
                        aSelectedFiles[0] = OUString(sDirectoryURL);
                    }
                    //if (m_pReplyRecord != NULL) {
                        implsetDisplayDirectory(sDirectoryURL);
                    //}

                    OSL_TRACE("dir url: %s", OUStringToOString(sDirectoryURL, RTL_TEXTENCODING_UTF8).getStr());
                }
                break;
            }
            case NAVIGATIONSERVICES_SAVE:
            {
                OSL_TRACE("NAVIGATIONSERVICES_SAVE");

                //we have the dir already, now get the file's name
                CFStringRef sFileName = NavDialogGetSaveFileName(m_pDialog);
                CFRetain(sFileName);

                sFileOrDirURL = sFileOrDirURL.concat(CFStringToOUString(sFileName));
                CFRelease(sFileName);

                break;
            }
            default:
            {
                OSL_TRACE("ooooooo");
            }
        }

        short nIndexToUse = nFiles > 1 ? nIndex : nIndex - 1;
        aSelectedFiles[nIndexToUse] = OUString(sFileOrDirURL);

        OSL_TRACE("Returned file in getFiles: \"%s\".", OUStringToOString(sFileOrDirURL, RTL_TEXTENCODING_UTF8).getStr());
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return aSelectedFiles;
}

#pragma mark XFilterManager
//-----------------------------------------------------------------------------------------
// XFilterManager functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::appendFilter( const rtl::OUString& aTitle, const rtl::OUString& aFilter )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ensureFilterHelper();
    m_pFilterHelper->appendFilter( aTitle, aFilter );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setCurrentFilter( const rtl::OUString& aTitle )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    OSL_TRACE( "Setting current filter to %s",
               OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr() );

    ensureFilterHelper();
    m_pFilterHelper->setCurrentFilter(aTitle);
    updateFilterUI();
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaFilePicker::getCurrentFilter() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

//  OSL_TRACE( "GetCURRENTfilter" );

    //UpdateFilterfromUI();

//  OSL_TRACE( "Returning current filter of ");
//    CFShow(m_aCurrentFilter);
    ensureFilterHelper();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return m_pFilterHelper->getCurrentFilter();
}

#pragma mark XFilterGroupManager
//-----------------------------------------------------------------------------------------
// XFilterGroupManager functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::appendFilterGroup( const rtl::OUString& sGroupTitle, const uno::Sequence<beans::StringPair>& aFilters )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    ensureFilterHelper();
    m_pFilterHelper->appendFilterGroup(sGroupTitle, aFilters);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XFilePickerControlAccess
//------------------------------------------------------------------------------------
// XFilePickerControlAccess functions
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pControlHelper->setValue(nControlId, nControlAction, rValue);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

uno::Any SAL_CALL SalAquaFilePicker::getValue( sal_Int16 nControlId, sal_Int16 nControlAction )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return m_pControlHelper->getValue(nControlId, nControlAction);
}

void SAL_CALL SalAquaFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pControlHelper->enableControl(nControlId, bEnable);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaFilePicker::setLabel( sal_Int16 nControlId, const ::rtl::OUString& aLabel )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    CFStringRef sLabel = CFStringCreateWithOUString( aLabel );
    m_pControlHelper->setLabel( nControlId, sLabel ) ;
    CFRelease( sLabel );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaFilePicker::getLabel( sal_Int16 nControlId )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return m_pControlHelper->getLabel(nControlId);
}

#pragma mark XFilePreview
//------------------------------------------------------------------------------------
// XFilePreview functions
//------------------------------------------------------------------------------------

uno::Sequence<sal_Int16> SAL_CALL SalAquaFilePicker::getSupportedImageFormats() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Sequence< sal_Int16 > aFormats( 1 );

    aFormats[0] = FilePreviewImageFormats::BITMAP;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return aFormats;
}

sal_Int32 SAL_CALL SalAquaFilePicker::getTargetColorDepth() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getTargetColorDepth();
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return 8;
}

sal_Int32 SAL_CALL SalAquaFilePicker::getAvailableWidth() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getAvailableWidth();
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return m_PreviewImageWidth;
}

sal_Int32 SAL_CALL SalAquaFilePicker::getAvailableHeight() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getAvailableHeight();
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return m_PreviewImageHeight;
}

void SAL_CALL SalAquaFilePicker::setImage( sal_Int16 /*aImageFormat*/, const uno::Any& /*aImage*/ )
throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO m_pImpl->setImage( aImageFormat, aImage );
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

sal_Bool SAL_CALL SalAquaFilePicker::setShowState( sal_Bool bShowState ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    mbPreviewState = bShowState;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return mbPreviewState;
}

sal_Bool SAL_CALL SalAquaFilePicker::getShowState() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    //  ::vos::OGuard aGuard( Application::GetSolarMutex() );

    //  // TODO return m_pImpl->getShowState();
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return mbPreviewState;
}

#pragma mark XInitialization
//------------------------------------------------------------------------------------
// XInitialization
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::initialize( const uno::Sequence<uno::Any>& aArguments )
throw( uno::Exception, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "arguments size", aArguments.getLength());

    // parameter checking
    uno::Any aAny;
    if( 0 == aArguments.getLength() )
        throw lang::IllegalArgumentException(rtl::OUString::createFromAscii( "no arguments" ),
                                             static_cast<XFilePicker*>( this ), 1 );

    aAny = aArguments[0];

    if( ( aAny.getValueType() != ::getCppuType( ( sal_Int16* )0 ) ) &&
        (aAny.getValueType() != ::getCppuType( ( sal_Int8* )0 ) ) )
        throw lang::IllegalArgumentException(rtl::OUString::createFromAscii( "invalid argument type" ),
                                             static_cast<XFilePicker*>( this ), 1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    //  //   TODO: extract full semantic from
    //  //   svtools/source/filepicker/filepicker.cxx (getWinBits)
    //templateId = FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS;
    switch( templateId )
    {
        case FILEOPEN_SIMPLE:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_SIMPLE" );
            break;
        case FILESAVE_SIMPLE:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_SIMPLE" );
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_PASSWORD" );
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS" );
            break;
        case FILESAVE_AUTOEXTENSION_SELECTION:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_SELECTION" );
            break;
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION_TEMPLATE" );
            break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE" );
            break;
        case FILEOPEN_PLAY:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_PLAY" );
            break;
        case FILEOPEN_READONLY_VERSION:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_READONLY_VERSION" );
            break;
        case FILEOPEN_LINK_PREVIEW:
            m_nDialogType = NAVIGATIONSERVICES_OPEN;
            OSL_TRACE( "Template: FILEOPEN_LINK_PREVIEW" );
            break;
        case FILESAVE_AUTOEXTENSION:
            m_nDialogType = NAVIGATIONSERVICES_SAVE;
            OSL_TRACE( "Template: FILESAVE_AUTOEXTENSION" );
            break;
        default:
            throw lang::IllegalArgumentException(rtl::OUString::createFromAscii( "Unknown template" ),
                                                 static_cast< XFilePicker* >( this ),
                                                 1 );
    }

    m_pControlHelper->initialize(templateId);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XCancellable
//------------------------------------------------------------------------------------
// XCancellable
//------------------------------------------------------------------------------------

void SAL_CALL SalAquaFilePicker::cancel() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OSStatus status = NavCustomControl(m_pDialog, kNavCtlCancel, NULL);
    if (status != noErr) {
        OSL_TRACE("NavigationServices wouldn't allow cancellation");
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XEventListener
//------------------------------------------------
// XEventListener
//------------------------------------------------

//using ::cppu::WeakComponentImplHelperBase::disposing;
void SAL_CALL SalAquaFilePicker::disposing( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    uno::Reference<XFilePickerListener> xFilePickerListener( aEvent.Source, ::com::sun::star::uno::UNO_QUERY );

    if( xFilePickerListener.is() )
        removeFilePickerListener( xFilePickerListener );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XServiceInfo
// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

rtl::OUString SAL_CALL SalAquaFilePicker::getImplementationName()
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    rtl::OUString retVal = rtl::OUString::createFromAscii( FILE_PICKER_IMPL_NAME );

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}

sal_Bool SAL_CALL SalAquaFilePicker::supportsService( const rtl::OUString& sServiceName )
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "service name", sServiceName);

    sal_Bool retVal = sal_False;

    uno::Sequence <rtl::OUString> supportedServicesNames = FilePicker_getSupportedServiceNames();

    for( sal_Int32 n = supportedServicesNames.getLength(); n--; ) {
        if( supportedServicesNames[n].compareTo( sServiceName ) == 0) {
            retVal = sal_True;
            break;
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return retVal;
}

uno::Sequence<rtl::OUString> SAL_CALL SalAquaFilePicker::getSupportedServiceNames()
throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return FilePicker_getSupportedServiceNames();
}

#pragma mark Misc/Private
//-----------------------------------------------------------------------------------------
// FilePicker Event functions
//-----------------------------------------------------------------------------------------

//void SAL_CALL SalAquaFilePicker::fileSelectionChanged( FilePickerEvent aEvent )
//{
//  OSL_TRACE( "file selection changed");
//    if (m_xListener.is()) m_xListener->fileSelectionChanged( aEvent );
//}
//
//void SAL_CALL SalAquaFilePicker::directoryChanged( FilePickerEvent aEvent )
//{
//  OSL_TRACE("directory changed");
//    if (m_xListener.is()) m_xListener->directoryChanged( aEvent );
//}
//
//void SAL_CALL SalAquaFilePicker::controlStateChanged( FilePickerEvent aEvent )
//{
//  OSL_TRACE("control state changed");
//    if (m_xListener.is()) m_xListener->controlStateChanged( aEvent );
//}
//
//void SAL_CALL SalAquaFilePicker::dialogSizeChanged()
//{
//    OSL_TRACE("dialog size changed");
//  /*
//    if (m_xListener.is()) m_xListener->dialogSizeChanged( aEvent );
//  */
//}

//-----------------------------------------------------------------------------------------
// If there are more then one listener the return value of the last one wins
//-----------------------------------------------------------------------------------------

//rtl::OUString SAL_CALL SalAquaFilePicker::helpRequested( FilePickerEvent aEvent ) const
//{
//  rtl::OUString aHelpText;
//
//  ::cppu::OInterfaceContainerHelper* pICHelper =
//      rBHelper.getContainer( getCppuType( ( uno::Reference<XFilePickerListener> * )0 ) );
//
//  if( pICHelper )
//  {
//      ::cppu::OInterfaceIteratorHelper iter( *pICHelper );
//
//      while( iter.hasMoreElements() )
//      {
//          try
//          {
//              /*
//                    if there are multiple listeners responding
//                        to this notification the next response
//                overwrittes  the one before if it is not empty
//                      */
//
//              rtl::OUString aTempString;
//
//              uno::Reference<XFilePickerListener> xFPListener( iter.next(), uno::UNO_QUERY );
//              if( xFPListener.is() )
//                      {
//                  aTempString = xFPListener->helpRequested( aEvent );
//                  if( aTempString.getLength() )
//                      aHelpText = aTempString;
//                      }
//
//          }
//          catch( uno::RuntimeException& )
//          {
//              OSL_ENSURE( false, "RuntimeException during event dispatching" );
//          }
//      }
//  }
//
//  return aHelpText;
//}

//=====================================================================

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

// void SalAquaFilePicker::updateCurrentFilterFromName(const gchar* filtername)
// {
//  OUString aFilterName(filtername, strlen(filtername), RTL_TEXTENCODING_UTF8);
//  FilterList::iterator aEnd = m_pFilterList->end();
//     for (FilterList::iterator aIter = m_pFilterList->begin(); aIter != aEnd; ++aIter)
//     {
//         if (aFilterName == shrinkFilterName( aIter->getTitle()))
//         {
//             m_aCurrentFilter = aIter->getTitle();
//             break;
//         }
//     }
// }

//------------------------------------------------------------------------------------

#define MAP_TOGGLE( elem ) \
case ExtendedFilePickerElementIds::CHECKBOX_##elem: \
    pWidget = m_pToggles[elem]; \
    break

#define MAP_BUTTON( elem ) \
case ExtendedFilePickerElementIds::PUSHBUTTON_##elem: \
    pWidget = m_pButtons[elem]; \
    break
#undef MAP_LIST
#define MAP_LIST( elem ) \
case ExtendedFilePickerElementIds::LISTBOX_##elem: \
    pWidget = m_pListControls[elem]; if (isAList != NULL) *isAList = sal_True; \
    break

#define MAP_LIST_LABEL( elem ) \
case ExtendedFilePickerElementIds::LISTBOX_##elem##_LABEL: \
    pWidget = m_pListControls[elem]; \
    break

//------------------------------------------------------------------------------------
// XFilePickerControlAccess functions
//------------------------------------------------------------------------------------

// void SalAquaFilePicker::preview_toggled_cb( GtkObject *cb, SalAquaFilePicker* pobjFP )
// {
//  if( pobjFP->mbToggleVisibility[PREVIEW] )
//      pobjFP->setShowState( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( cb ) ) );
// }

//--------------------------------------------------
// Misc
//-------------------------------------------------
void SalAquaFilePicker::ensureFilterHelper() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL == m_pFilterHelper) {
        m_pFilterHelper = new FilterHelper;
        if (m_nDialogType == NAVIGATIONSERVICES_SAVE) {
            m_pControlHelper->setPopupPresent(true);
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark NavigationServices callBack delegates
/**
 * Tries to get enough space for our controls. Therefore we have to know how large our controls want to be
 * If we don't get the desired dimensions, especially the desired width
 */
void SalAquaFilePicker::implHandleNavDialogCustomize(NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pControlHelper->handleCustomize(callBackParms);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::implHandleNavDialogStart(NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    m_pControlHelper->handleStart(callBackParms);

    updateFilterUI();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::implHandleNavDialogEvent(NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    //maybe the filter was still not correctly set
    if (sal_False == m_bFilterUICorrectlySet) {
        updateFilterUI();
    }

    //updateSaveFileNameFromUI();

    m_pControlHelper->handleEvent(callBackParms, m_aLatestEvent);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

sal_Bool SalAquaFilePicker::implFilterHandler(AEDesc *theItem, void *info,
                                   void *callBackUD,
                                   NavFilterModes filterMode)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    sal_Bool retVal = sal_False;

    if (m_pFilterHelper) {
        retVal = m_pFilterHelper->implFilterHandler(theItem, info, callBackUD, filterMode);
    } else {
        retVal = sal_True;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}

void SalAquaFilePicker::implInitialize()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    //first do something
    if (m_pFilterHelper) {
        CFStringList *filterNames = m_pFilterHelper->getFilterNames();
        if (filterNames) {
            CFMutableArrayRef namesArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
            CFStringList::iterator it;
            for ( it = filterNames->begin(); it != filterNames->end(); it++) {
                CFArrayAppendValue(namesArray,*it);
            }
            m_pDialogOptions.popupExtension = namesArray;
        }
    }

    //autoextension
    if (true == m_pControlHelper->getVisibility(ControlHelper::AUTOEXTENSION)) {
        m_pDialogOptions.optionFlags |= kNavPreserveSaveFileExtension;
    }

    //then call super
    SalAquaPicker::implInitialize();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::implHandlePopupMenuSelect(NavMenuItemSpec* menuItem) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (menuItem != NULL) {
        //the first call to this method is after we have set the filter programmatically
        //now we have to check if the filter was correctly set in the UI. Since sometimes this
        //does not work, we may have to repeatedly call updateFilterUI() until we succeed
        int nIndexFromUI = (int)(menuItem->menuType);
        OSL_TRACE("=== menu item selected: %d ===", nIndexFromUI);
        if (sal_False == m_bFilterUICorrectlySet) {
            if (nIndexFromUI == m_pFilterHelper->getCurrentFilterIndex()) {
                //the call was successful, erase the flag
                m_bFilterUICorrectlySet = sal_True;
            } else {
                updateFilterUI();
            }
        } else {
            //user initiated
            m_pFilterHelper->SetFilterAtIndex(menuItem->menuType);
            //update the file name with the new extension if we have a save dialog
            if (m_nDialogType == NAVIGATIONSERVICES_SAVE) {
                CFStringRef currentUIName = NavDialogGetSaveFileName(m_pDialog);
                setDefaultName(CFStringToOUString(currentUIName));
            }
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::updateFilterUI() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    OSStatus status = noErr;
    //set for gui
    NavMenuItemSpec menuItem;
    menuItem.version = kNavMenuItemSpecVersion;

    ensureFilterHelper();
    menuItem.menuType = m_pFilterHelper->getCurrentFilterIndex();
    if (m_pDialog) {
        OSL_TRACE("FH: SalAquaFilePicker::updateFilterUI -> dialog is created");
        status = NavCustomControl(m_pDialog,kNavCtlSelectCustomType,&menuItem);
        if (status != noErr) {
            OSL_TRACE("Could not update the UI!");
            //updateFilterUI();
        }
    } else {
        OSL_TRACE("FH: SalAquaFilePicker::updateFilterUI -> dialog is NOT created");
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::implHandleNavDialogSelectEntry(NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    FilePickerEvent aEvent;
    if (m_xListener.is())
        m_xListener->fileSelectionChanged( aEvent );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

sal_Bool SalAquaFilePicker::implPreviewHandler (NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__, sal_False);

    return sal_False;
#if 0
    OSErr       theErr;
    MacOSBoolean     previewShowing = false;
    MacOSBoolean     result = false;

    theErr = NavCustomControl(callBackParms->context,// 1
                              kNavCtlIsPreviewShowing,
                              &previewShowing );
    if (theErr == noErr && previewShowing)
    {
        AEDesc *theItem = (AEDesc*)callBackParms->eventData.eventDataParms.param;
        FSRef ref;
        AECoerceDesc (theItem, typeFSRef, theItem); // 2

        if ( AEGetDescData (theItem, &ref, sizeof (FSRef)) == noErr )
        {
            fprintf(stdout, " starting to work\n");
            char fileName[1024];
            theErr = FSRefMakePath(&ref, (UInt8*)fileName, 1023);
            fprintf(stdout, " file name: %s\n", fileName);
            if (theErr == noErr)
            {
                Rect previewInfoButtonRect ={
                    callBackParms->previewRect.top,
                    callBackParms->previewRect.left,
                    callBackParms->previewRect.bottom,
                    callBackParms->previewRect.right
                };

                m_PreviewImageWidth = previewInfoButtonRect.right - previewInfoButtonRect.left;
                m_PreviewImageHeight = previewInfoButtonRect.bottom - previewInfoButtonRect.top;

                Rect windowBounds;
                GetWindowBounds(callBackParms->window,kWindowContentRgn,&windowBounds);
                int windowTop = windowBounds.top;
                int windowHeight = windowBounds.bottom - windowTop;

                CGContextRef myContext;

                SetPortWindowPort (callBackParms->window);

                QDBeginCGContext (GetWindowPort (callBackParms->window), &myContext);

                // ********** Your drawing code here **********
                CGContextSetRGBFillColor (myContext, 1, 0, 0, 1);
                int previewWidth = previewInfoButtonRect.right - previewInfoButtonRect.left;
                int previewHeight = previewInfoButtonRect.bottom - previewInfoButtonRect.top;
                //CG coordinate system is flipped, so do a little calculus
                int previewTop = windowHeight - previewInfoButtonRect.bottom;
                CGRect cgRect = CGRectMake (/*x*/previewInfoButtonRect.left, /*y*/previewTop, previewWidth, previewHeight);
                CGContextFillRect (myContext, cgRect);
                //                        CGContextSetRGBFillColor (myContext, 0, 0, 1, .5);
                //                        CGContextFillRect (myContext, CGRectMake (0, 0, 100, 200));
                CGContextFlush(myContext);

                QDEndCGContext (GetWindowPort(callBackParms->window), &myContext);

                result = true;
            }
        }
    } else {
        m_PreviewImageWidth = 0;
        m_PreviewImageHeight = 0;
    }
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return result;
#endif
}

void SalAquaFilePicker::setDefaultName(const rtl::OUString& aName, bool appendExtension) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "name", aName, "appendExtension", appendExtension);

    m_sSaveFileName = aName;
    updateSaveFileNameExtension(appendExtension);

    if (m_pDialog && m_sSaveFileName.getLength() > 0) {
        OSStatus status = NavDialogSetSaveFileName (m_pDialog, CFStringCreateWithOUString(m_sSaveFileName));
        if (status == noErr) {
            OSL_TRACE("filename was set");
        } else {
            OSL_TRACE("filename was not set");
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SalAquaFilePicker::updateSaveFileNameExtension(bool appendExtension) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "appendExtension", appendExtension);

    ensureFilterHelper();

    OUStringList aStringList = m_pFilterHelper->getCurrentFilterSuffixList();
    if (0 < aStringList.size()) {
        sal_Int32 nLastDot = m_sSaveFileName.lastIndexOf('.');
        if (nLastDot > -1 || appendExtension == true) {
            //we either have an extension or we should set it.
            if (nLastDot > -1) {
                //get the name without suffix
                m_sSaveFileName = m_sSaveFileName.copy(0, nLastDot);
            }
            //and append the current suffix
            rtl::OUString suffix = (*(m_pFilterHelper->getCurrentFilterSuffixList().begin()));
            m_sSaveFileName += suffix;
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

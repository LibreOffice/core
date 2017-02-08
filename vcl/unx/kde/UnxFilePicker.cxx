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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>

#include <FPServiceInfo.hxx>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/resmgr.hxx>

#include <UnxFilePicker.hxx>
#include <UnxCommandThread.hxx>
#include <UnxNotifyThread.hxx>

#include <vcl/fpicker.hrc>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/syswin.hxx>
#include <vcl/window.hxx>

#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <iostream>

#include <config_vclplug.h>

using namespace ::com::sun::star;

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;

// helper functions

namespace
{
    uno::Sequence<OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<OUString> aRet(3);
        aRet[0] = "com.sun.star.ui.dialogs.FilePicker";
        aRet[1] = "com.sun.star.ui.dialogs.SystemFilePicker";
#if ENABLE_TDE
        aRet[2] = "com.sun.star.ui.dialogs.TDEFilePicker";
#else // ENABLE_TDE
        aRet[2] = "com.sun.star.ui.dialogs.KDEFilePicker";
#endif // ENABLE_TDE
        return aRet;
    }

void appendEscaped( OUStringBuffer &rBuffer, const OUString &rString )
{
    const sal_Unicode *pUnicode = rString.getStr();
    const sal_Unicode *pEnd     = pUnicode + rString.getLength();

    rBuffer.append( "\"" );

    for ( ; pUnicode != pEnd; ++pUnicode )
    {
        if ( *pUnicode == '\\' )
            rBuffer.append( "\\\\" );
        else if ( *pUnicode == '"' )
            rBuffer.append( "\\\"" );
        else if ( *pUnicode == '\n' )
            rBuffer.append( "\\n" );
        else
            rBuffer.append( *pUnicode );
    }

    rBuffer.append( "\"" );
}

bool controlIdInfo( sal_Int16 nControlId, OUString &rType, sal_Int32 &rTitleId )
{
    typedef struct {
        sal_Int16 nId;
        const OUString *pType;
        sal_Int32 nTitle;
    } ElementToName;

    const OUString aCheckBox(   "checkbox" );
    const OUString aControl(    "control" );
    const OUString aEdit(       "edit" );
    const OUString aLabel(      "label" );
    const OUString aListBox(    "listbox" );
    const OUString aPushButton( "pushbutton" );

    const ElementToName *pPtr;
    const ElementToName pArray[] =
    {
        { CommonFilePickerElementIds::PUSHBUTTON_OK,            &aPushButton, 0/*FIXME?*/ },
        { CommonFilePickerElementIds::PUSHBUTTON_CANCEL,        &aPushButton, 0/*FIXME?*/ },
        { CommonFilePickerElementIds::LISTBOX_FILTER,           &aListBox,    0/*FIXME?*/ },
        { CommonFilePickerElementIds::CONTROL_FILEVIEW,         &aControl,    0/*FIXME?*/ },
        { CommonFilePickerElementIds::EDIT_FILEURL,             &aEdit,       0/*FIXME?*/ },
        { CommonFilePickerElementIds::LISTBOX_FILTER_LABEL,     &aLabel,      0/*FIXME?*/ },
        { CommonFilePickerElementIds::EDIT_FILEURL_LABEL,       &aLabel,      0/*FIXME?*/ },

        { ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, &aCheckBox,   STR_SVT_FILEPICKER_AUTO_EXTENSION },
        { ExtendedFilePickerElementIds::CHECKBOX_PASSWORD,      &aCheckBox,   STR_SVT_FILEPICKER_PASSWORD },
        { ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS, &aCheckBox,   STR_SVT_FILEPICKER_FILTER_OPTIONS },
        { ExtendedFilePickerElementIds::CHECKBOX_READONLY,      &aCheckBox,   STR_SVT_FILEPICKER_READONLY },
        { ExtendedFilePickerElementIds::CHECKBOX_LINK,          &aCheckBox,   STR_SVT_FILEPICKER_INSERT_AS_LINK },
        { ExtendedFilePickerElementIds::CHECKBOX_PREVIEW,       &aCheckBox,   STR_SVT_FILEPICKER_SHOW_PREVIEW },
        { ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,        &aPushButton, STR_SVT_FILEPICKER_PLAY },
        { ExtendedFilePickerElementIds::LISTBOX_VERSION,        &aListBox,    STR_SVT_FILEPICKER_VERSION },
        { ExtendedFilePickerElementIds::LISTBOX_TEMPLATE,       &aListBox,    STR_SVT_FILEPICKER_TEMPLATES },
        { ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE, &aListBox,    STR_SVT_FILEPICKER_IMAGE_TEMPLATE },
        { ExtendedFilePickerElementIds::CHECKBOX_SELECTION,     &aCheckBox,   STR_SVT_FILEPICKER_SELECTION },
        { 0, nullptr, 0 }
    };

    for ( pPtr = pArray; pPtr->nId && ( pPtr->nId != nControlId ); ++pPtr )
        ;

    if ( pPtr->nId == nControlId )
    {
        rType = *(pPtr->pType);
        rTitleId = pPtr->nTitle;

        return true;
    }

    return false;
}

bool controlActionInfo( sal_Int16 nControlAction, OUString &rType )
{
    typedef struct {
        sal_Int16 nId;
        const OUString pType;
    } ElementToName;

    const ElementToName *pPtr;
    const ElementToName pArray[] =
    {
        { ControlActions::ADD_ITEM,                OUString( "addItem" ) },
        { ControlActions::ADD_ITEMS,               OUString( "addItems" ) },
        { ControlActions::DELETE_ITEM,             OUString( "deleteItem" ) },
        { ControlActions::DELETE_ITEMS,            OUString( "deleteItems" ) },
        { ControlActions::SET_SELECT_ITEM,         OUString( "setSelectedItem" ) },
        { ControlActions::GET_ITEMS,               OUString( "getItems" ) },
        { ControlActions::GET_SELECTED_ITEM,       OUString( "getSelectedItem" ) },
        { ControlActions::GET_SELECTED_ITEM_INDEX, OUString( "getSelectedItemIndex" ) },
        { ControlActions::SET_HELP_URL,            OUString( "setHelpURL" ) },
        { ControlActions::GET_HELP_URL,            OUString( "getHelpURL" ) },
        { 0,                                       OUString( "noAction" ) }
    };

    for ( pPtr = pArray; pPtr->nId && ( pPtr->nId != nControlAction ); ++pPtr )
        ;

    rType = pPtr->pType;

    return true;
}

}

// UnxFilePicker

UnxFilePicker::UnxFilePicker( const uno::Reference<uno::XComponentContext>& )
    : UnxFilePicker_Base( m_rbHelperMtx ),
          m_nFilePickerPid( -1 ),
          m_nFilePickerWrite( -1 ),
          m_nFilePickerRead( -1 ),
          m_pNotifyThread( nullptr ),
          m_pCommandThread( nullptr ),
          m_pResMgr( ResMgr::CreateResMgr("fps_office") )
{
}

UnxFilePicker::~UnxFilePicker()
{
    if ( m_nFilePickerPid > 0 )
    {
        sendCommand( "exit" );
        waitpid( m_nFilePickerPid, nullptr, 0 );
    }

    if ( m_pCommandThread )
    {
        m_pCommandThread->join();

        delete m_pCommandThread; m_pCommandThread = nullptr;
    }

    if ( m_pNotifyThread )
    {
        m_pNotifyThread->exit();

        m_pNotifyThread->join();

        delete m_pNotifyThread; m_pNotifyThread = nullptr;
    }

    if ( m_nFilePickerWrite >= 0 )
        close( m_nFilePickerWrite );

    if ( m_nFilePickerRead >= 0 )
        close( m_nFilePickerRead );

    delete m_pResMgr; m_pResMgr = nullptr;
}

void SAL_CALL UnxFilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
{
    OSL_ASSERT( m_pNotifyThread );
    osl::MutexGuard aGuard( m_aMutex );

    m_pNotifyThread->addFilePickerListener( xListener );
}

void SAL_CALL UnxFilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
{
    OSL_ASSERT( m_pNotifyThread );
    osl::MutexGuard aGuard( m_aMutex );

    m_pNotifyThread->removeFilePickerListener( xListener );
}

void SAL_CALL UnxFilePicker::setTitle( const OUString &rTitle )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "setTitle " );
    appendEscaped( aBuffer, rTitle );

    sendCommand( aBuffer.makeStringAndClear() );
}

sal_Int16 SAL_CALL UnxFilePicker::execute()
{
    checkFilePicker();

    // this is _not_ an osl::Condition, see i#93366
    m_pCommandThread->execCondition().reset();

    sendCommand( "exec" );

    m_pCommandThread->execCondition().wait();

    return m_pCommandThread->result()
        ? css::ui::dialogs::ExecutableDialogResults::OK
        : css::ui::dialogs::ExecutableDialogResults::CANCEL;
}

void SAL_CALL UnxFilePicker::setMultiSelectionMode( sal_Bool bMode )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( bMode )
        sendCommand( "setMultiSelection true" );
    else
        sendCommand( "setMultiSelection false" );
}

void SAL_CALL UnxFilePicker::setDefaultName( const OUString &rName )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "setDefaultName " );
    appendEscaped( aBuffer, rName );

    sendCommand( aBuffer.makeStringAndClear() );
}

void SAL_CALL UnxFilePicker::setDisplayDirectory( const OUString &rDirectory )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "setDirectory " );
    appendEscaped( aBuffer, rDirectory );

    sendCommand( aBuffer.makeStringAndClear() );
}

OUString SAL_CALL UnxFilePicker::getDisplayDirectory()
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    sendCommand( "getDirectory",
                 m_pCommandThread->getDirectoryCondition() );

    return m_pCommandThread->getDirectory();
}

uno::Sequence< OUString > SAL_CALL UnxFilePicker::getFiles()
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    sendCommand( "getFiles",
                 m_pCommandThread->getFilesCondition() );

    return m_pCommandThread->getFiles();
}

void SAL_CALL UnxFilePicker::appendFilter( const OUString &rTitle, const OUString &rFilter )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "appendFilter " );
    appendEscaped( aBuffer, rTitle );
    aBuffer.append( " " );
    appendEscaped( aBuffer, rFilter );

    sendCommand( aBuffer.makeStringAndClear() );
}

void SAL_CALL UnxFilePicker::setCurrentFilter( const OUString &rTitle )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "setCurrentFilter " );
    appendEscaped( aBuffer, rTitle );

    sendCommand( aBuffer.makeStringAndClear() );
}

OUString SAL_CALL UnxFilePicker::getCurrentFilter()
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    sendCommand( "getCurrentFilter",
                 m_pCommandThread->getCurrentFilterCondition() );

    return m_pCommandThread->getCurrentFilter();
}

void SAL_CALL UnxFilePicker::appendFilterGroup( const OUString &rGroupTitle, const uno::Sequence<beans::StringPair> &rFilters )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "appendFilterGroup " );
    appendEscaped( aBuffer, rGroupTitle );

    for ( sal_Int32 i = 0; i < rFilters.getLength(); ++i )
    {
        beans::StringPair aPair = rFilters[i];

        aBuffer.append( " " );
        appendEscaped( aBuffer, aPair.First );
        aBuffer.append( " " );
        appendEscaped( aBuffer, aPair.Second );
    }

    sendCommand( aBuffer.makeStringAndClear() );
}

void SAL_CALL UnxFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any &rValue )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aType;
    OUString aAction;
    sal_Int32 nTitleId;

    if ( controlIdInfo( nControlId, aType, nTitleId ) && controlActionInfo( nControlAction, aAction ) )
    {
        OUStringBuffer aBuffer( 1024 );

        aBuffer.append( "setValue " );
        aBuffer.append( static_cast< sal_Int32 >( nControlId ) );
        aBuffer.append( " " );
        aBuffer.append( aAction );

        if ( aType == "checkbox" )
        {
            bool bControlValue;
            if ( ( rValue >>= bControlValue ) && bControlValue )
                aBuffer.append( " true" );
            else
                aBuffer.append( " false" );
        }
        else if ( aType == "listbox" )
        {
            switch ( nControlAction )
            {
                case ControlActions::ADD_ITEM:
                case ControlActions::SET_HELP_URL:
                    {
                        OUString aString;
                        if ( rValue >>= aString )
                        {
                            aBuffer.append( " " );
                            appendEscaped( aBuffer, aString );
                        }
                    }
                    break;

                case ControlActions::ADD_ITEMS:
                    {
                        uno::Sequence< OUString > aSequence;
                        if ( rValue >>= aSequence )
                        {
                            for ( sal_Int32 nIdx = 0; nIdx < aSequence.getLength(); ++nIdx )
                            {
                                aBuffer.append( " " );
                                appendEscaped( aBuffer, aSequence[nIdx] );
                            }

                        }
                    }
                    break;

                case ControlActions::DELETE_ITEM:
                case ControlActions::SET_SELECT_ITEM:
                    {
                        sal_Int32 nInt;
                        if ( rValue >>= nInt )
                        {
                            aBuffer.append( " " );
                            aBuffer.append( nInt );
                        }
                    }
                    break;

                default:
                    // nothing
                    break;
            }
        }
        // TODO else if push button...

        sendCommand( aBuffer.makeStringAndClear() );
    }
}

uno::Any SAL_CALL UnxFilePicker::getValue( sal_Int16 nControlId, sal_Int16 nControlAction )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aAction;

    if ( controlActionInfo( nControlAction, aAction ) )
    {
        OUStringBuffer aBuffer( 1024 );

        aBuffer.append( "getValue " );
        aBuffer.append( static_cast< sal_Int32 >( nControlId ) );
        aBuffer.append( " " );
        aBuffer.append( aAction );

        sendCommand( aBuffer.makeStringAndClear(),
                m_pCommandThread->getValueCondition() );

        return m_pCommandThread->getValue();
    }

    return uno::Any();
}

void SAL_CALL UnxFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "enableControl " );
    aBuffer.append( static_cast< sal_Int32 >( nControlId ) );
    aBuffer.appendAscii( bEnable? " true": " false" );

    sendCommand( aBuffer.makeStringAndClear() );
}

void SAL_CALL UnxFilePicker::setLabel( sal_Int16 nControlId, const OUString &rLabel )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer( 1024 );

    aBuffer.append( "setLabel " );
    aBuffer.append( static_cast< sal_Int32 >( nControlId ) );
    aBuffer.append( " " );
    appendEscaped( aBuffer, rLabel );

    sendCommand( aBuffer.makeStringAndClear() );
}

OUString SAL_CALL UnxFilePicker::getLabel(sal_Int16 /*nControlId*/)
{
    // FIXME getLabel() is not yet implemented
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    // TODO return m_pImpl->getLabel(nControlId);
    return OUString();
}

/* TODO
uno::Sequence<sal_Int16> SAL_CALL UnxFilePicker::getSupportedImageFormats()
    throw( uno::RuntimeException )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pImpl->getSupportedImageFormats();
}

sal_Int32 SAL_CALL UnxFilePicker::getTargetColorDepth()
    throw( uno::RuntimeException )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pImpl->getTargetColorDepth();
}

sal_Int32 SAL_CALL UnxFilePicker::getAvailableWidth()
    throw( uno::RuntimeException )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pImpl->getAvailableWidth();
}

sal_Int32 SAL_CALL UnxFilePicker::getAvailableHeight()
    throw( uno::RuntimeException )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pImpl->getAvailableHeight();
}

void SAL_CALL UnxFilePicker::setImage( sal_Int16 aImageFormat, const uno::Any &rImage )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    m_pImpl->setImage( aImageFormat, aImage );
}

sal_Bool SAL_CALL UnxFilePicker::setShowState( sal_Bool bShowState )
    throw( uno::RuntimeException )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pImpl->setShowState( bShowState );
}

sal_Bool SAL_CALL UnxFilePicker::getShowState()
    throw( uno::RuntimeException )
{
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_pImpl->getShowState();
}
*/

void SAL_CALL UnxFilePicker::initialize( const uno::Sequence<uno::Any> &rArguments )
{
    initFilePicker();

    // parameter checking
    uno::Any aAny;
    if ( 0 == rArguments.getLength( ) )
        throw lang::IllegalArgumentException(
                "no arguments",
                static_cast< XFilePicker2* >( this ), 1 );

    aAny = rArguments[0];

    if ( ( aAny.getValueType() != cppu::UnoType<sal_Int16>::get()) && ( aAny.getValueType() != cppu::UnoType<sal_Int8>::get()) )
        throw lang::IllegalArgumentException(
                "invalid argument type",
                static_cast< XFilePicker2* >( this ), 1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    OUString aTypeOpen( "setType \"open\"" );
    OUString aTypeSaveAs( "setType \"save\"" );

    switch ( templateId )
    {
        case FILEOPEN_SIMPLE:
            sendCommand( aTypeOpen );
            break;

        case FILESAVE_SIMPLE:
            sendCommand( aTypeSaveAs );
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD:
            sendCommand( aTypeSaveAs );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD );
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            sendCommand( aTypeSaveAs );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD );
            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS );
            break;

        case FILESAVE_AUTOEXTENSION_SELECTION:
            sendCommand( aTypeSaveAs );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_SELECTION );
            break;

        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            sendCommand( aTypeSaveAs );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            sendAppendControlCommand( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE );
            break;

        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            sendCommand( aTypeOpen );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_LINK );
            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW );
            sendAppendControlCommand( ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE );
            break;

        case FILEOPEN_PLAY:
            sendCommand( aTypeOpen );

            sendAppendControlCommand( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY );
            break;

        case FILEOPEN_LINK_PLAY:
            sendCommand( aTypeOpen );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_LINK );
            sendAppendControlCommand( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY );
            break;

        case FILEOPEN_READONLY_VERSION:
            sendCommand( aTypeOpen );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_READONLY );
            sendAppendControlCommand( ExtendedFilePickerElementIds::LISTBOX_VERSION );
            break;

        case FILEOPEN_LINK_PREVIEW:
            sendCommand( aTypeOpen );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_LINK );
            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW );
            break;

        case FILESAVE_AUTOEXTENSION:
            sendCommand( aTypeSaveAs );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            break;

        case FILEOPEN_PREVIEW:
            sendCommand( aTypeOpen );

            sendAppendControlCommand( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW );
            break;

        default:
            throw lang::IllegalArgumentException(
                    "Unknown template",
                    static_cast< XFilePicker2* >( this ),
                    1 );
    }
}

void SAL_CALL UnxFilePicker::cancel()
{
    // FIXME cancel() is not implemented
    checkFilePicker();
    ::osl::MutexGuard aGuard( m_aMutex );

    // TODO m_pImpl->cancel();
}

void SAL_CALL UnxFilePicker::disposing( const lang::EventObject &rEvent )
{
    uno::Reference<XFilePickerListener> xFilePickerListener( rEvent.Source, uno::UNO_QUERY );

    if ( xFilePickerListener.is() )
        removeFilePickerListener( xFilePickerListener );
}

OUString SAL_CALL UnxFilePicker::getImplementationName()
{
    return OUString( FILE_PICKER_IMPL_NAME );
}

sal_Bool SAL_CALL UnxFilePicker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL UnxFilePicker::getSupportedServiceNames()
{
    return FilePicker_getSupportedServiceNames();
}

void UnxFilePicker::initFilePicker()
{
    int aFiledesStdin[2], aFiledesStdout[2];
    if ( pipe( aFiledesStdin ) < 0 || pipe( aFiledesStdout ) < 0 )
        return;

    m_nFilePickerPid = fork();
    if ( m_nFilePickerPid < 0 )
        return;

    if ( m_nFilePickerPid == 0 )
    {
        // Child...
        close( aFiledesStdin[1] ); // write end of the pipe
        dup2(  aFiledesStdin[0], 0 );
        close( aFiledesStdin[0] );

        close( aFiledesStdout[0] ); // read end of the pipe
        dup2(  aFiledesStdout[1], 1 );
        close( aFiledesStdout[1] );

#if OSL_DEBUG_LEVEL == 0
        int nRedirect = open( "/dev/null", O_WRONLY );
        if( nRedirect != -1 )
        {
            dup2( nRedirect, 2 );
        }
#endif

        // The executable name
#if ENABLE_TDE
        OUString helperurl("${ORIGIN}/tdefilepicker");
#else // ENABLE_TDE
        OUString helperurl("${ORIGIN}/kdefilepicker");
#endif // ENABLE_TDE
        rtl::Bootstrap::expandMacros( helperurl );
        OUString helperpath;
        osl::FileBase::getSystemPathFromFileURL( helperurl, helperpath );
        OString helper( OUStringToOString( helperpath, osl_getThreadTextEncoding()));

        // ID of the main window
        const int nIdLen = 20;
        char pWinId[nIdLen] = "0";

        // TODO pass here the real parent (not possible for system dialogs
        // yet), and default to GetDefDialogParent() only when the real parent
        // is nullptr
        vcl::Window *pParentWin = Application::GetDefDialogParent();
        if ( pParentWin )
        {
            const SystemEnvData* pSysData = pParentWin->GetSystemData();
            if ( pSysData )
            {
                snprintf( pWinId, nIdLen, "%ld", pSysData->aWindow ); // unx only
                pWinId[nIdLen-1] = 0;
            }
        }

        // Execute the fpicker implementation
        execlp( helper.getStr(), helper.getStr(), "--winid", pWinId, nullptr );

        // Error, finish the child
        exit( -1 );
    }

    // Parent continues
    close( aFiledesStdin[0] );
    m_nFilePickerWrite = aFiledesStdin[1];

    close( aFiledesStdout[1] );
    m_nFilePickerRead = aFiledesStdout[0];

    // Create the notify thread
    if ( !m_pNotifyThread )
        m_pNotifyThread = new UnxFilePickerNotifyThread( this );

    // Create the command thread
    if ( !m_pCommandThread )
        m_pCommandThread = new UnxFilePickerCommandThread( m_pNotifyThread, m_nFilePickerRead );

    // Start the threads
    m_pNotifyThread->create();
    m_pCommandThread->create();

    return;
}

void UnxFilePicker::checkFilePicker()
{
    if ( m_nFilePickerPid > 0 )
    {
        // TODO check if external file picker is running
    }
    else
    {
        throw uno::RuntimeException(
                "the external file picker does not run",
                *this );
    }
}

void UnxFilePicker::sendCommand( const OUString &rCommand )
{
    if ( m_nFilePickerWrite < 0 )
        return;

    OString aUtfString = OUStringToOString( rCommand + "\n", RTL_TEXTENCODING_UTF8 );

#if OSL_DEBUG_LEVEL > 0
    ::std::cerr << "UnxFilePicker sent: \"" << aUtfString.getStr() << "\"" << ::std::endl;
#endif

    (void)write( m_nFilePickerWrite, aUtfString.getStr(), aUtfString.getLength() );
}

void UnxFilePicker::sendCommand( const OUString &rCommand, ::osl::Condition &rCondition )
{
    rCondition.reset();

    sendCommand( rCommand );

    rCondition.wait();
}

void UnxFilePicker::sendAppendControlCommand( sal_Int16 nControlId )
{
    OUString aType;
    sal_Int32 nTitleId;

    if ( controlIdInfo( nControlId, aType, nTitleId ) )
    {
        OUStringBuffer aBuffer( 1024 );

        aBuffer.append( "appendControl " );
        aBuffer.append( static_cast< sal_Int32 >( nControlId ) );
        aBuffer.append( " " );
        appendEscaped( aBuffer, aType );
        aBuffer.append( " " );
        appendEscaped( aBuffer, m_pResMgr? ResId(nTitleId, *m_pResMgr).toString(): OUString() );

        sendCommand( aBuffer.makeStringAndClear() );
    }
}

uno::Sequence< OUString > SAL_CALL UnxFilePicker::getSelectedFiles()
{
    return getFiles();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

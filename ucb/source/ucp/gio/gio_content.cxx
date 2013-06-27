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

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sal/macros.h>
#include <osl/time.h>

#include <osl/diagnose.h>
#include <osl/doublecheckedlocking.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/beans/PropertySetInfoChangeEvent.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkWriteException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkGeneralException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkReadException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>

#include <osl/conditn.hxx>

#include "gio_content.hxx"
#include "gio_provider.hxx"
#include "gio_resultset.hxx"
#include "gio_inputstream.hxx"
#include "gio_outputstream.hxx"
#include "gio_mount.hxx"

#include <stdio.h>

using namespace com::sun::star;

namespace gio
{

Content::Content(
    const uno::Reference< uno::XComponentContext >& rxContext,
    ContentProvider* pProvider,
    const uno::Reference< ucb::XContentIdentifier >& Identifier)
        throw ( ucb::ContentCreationException )
    : ContentImplHelper( rxContext, pProvider, Identifier ),
      m_pProvider( pProvider ), mpFile (NULL), mpInfo( NULL ), mbTransient(false)
{
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "New Content ('%s')\n", OUStringToOString(m_xIdentifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8).getStr());
#endif
}

Content::Content(
    const uno::Reference< uno::XComponentContext >& rxContext,
    ContentProvider* pProvider,
    const uno::Reference< ucb::XContentIdentifier >& Identifier,
    sal_Bool bIsFolder)
        throw ( ucb::ContentCreationException )
    : ContentImplHelper( rxContext, pProvider, Identifier ),
      m_pProvider( pProvider ), mpFile (NULL), mpInfo( NULL ), mbTransient(true)
{
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "Create Content ('%s')\n", OUStringToOString(m_xIdentifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8).getStr());
#endif
    mpInfo = g_file_info_new();
    g_file_info_set_file_type(mpInfo, bIsFolder ? G_FILE_TYPE_DIRECTORY : G_FILE_TYPE_REGULAR);
}

Content::~Content()
{
    if (mpInfo) g_object_unref(mpInfo);
    if (mpFile) g_object_unref(mpFile);
}

OUString Content::getParentURL()
{
    OUString sURL;
    if (GFile* pFile = g_file_get_parent(getGFile()))
    {
        char* pPath = g_file_get_uri(pFile);
        g_object_unref(pFile);
        sURL = OUString::createFromAscii(pPath);
        g_free(pPath);
    }
    return sURL;
}

void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
       throw( uno::RuntimeException )
{
    //TODO
    //stick a map from each CommandId to a new GCancellable and propogate
    //it throughout the g_file_* calls
}

OUString SAL_CALL Content::getContentType() throw( uno::RuntimeException )
{
    return isFolder(uno::Reference< ucb::XCommandEnvironment >())
        ? OUString( GIO_FOLDER_TYPE )
        : OUString( GIO_FILE_TYPE );
}

#define EXCEPT(aExcept) \
do { \
    if (bThrow) throw aExcept;\
    aRet = uno::makeAny( aExcept );\
} while(0)

uno::Any convertToException(GError *pError, const uno::Reference< uno::XInterface >& rContext, bool bThrow)
{
    uno::Any aRet;

    gint eCode = pError->code;
    OUString sMessage(pError->message, strlen(pError->message), RTL_TEXTENCODING_UTF8);
    g_error_free(pError);

    OUString sName;
    OUString sHost;

    uno::Sequence< uno::Any > aArgs( 1 );
    aArgs[ 0 ] <<= sName;

    switch (eCode)
    {
        case G_IO_ERROR_FAILED:
            { io::IOException aExcept(sMessage, rContext);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_MOUNTED:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_NOT_EXISTING_PATH, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_FOUND:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_NOT_EXISTING, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_EXISTS:
            { ucb::NameClashException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, sName);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_INVALID_ARGUMENT:
            { lang::IllegalArgumentException aExcept(sMessage, rContext, -1 );
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_PERMISSION_DENIED:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_ACCESS_DENIED, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_IS_DIRECTORY:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_NO_FILE, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_REGULAR_FILE:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_NO_FILE, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_DIRECTORY:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_NO_DIRECTORY, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_FILENAME_TOO_LONG:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_NAME_TOO_LONG, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_PENDING:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_PENDING, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_CLOSED:
        case G_IO_ERROR_CANCELLED:
        case G_IO_ERROR_TOO_MANY_LINKS:
        case G_IO_ERROR_WRONG_ETAG:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_GENERAL, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_SUPPORTED:
        case G_IO_ERROR_CANT_CREATE_BACKUP:
        case G_IO_ERROR_WOULD_MERGE:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_NOT_SUPPORTED, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NO_SPACE:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_OUT_OF_DISK_SPACE, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_INVALID_FILENAME:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_INVALID_CHARACTER, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_READ_ONLY:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_WRITE_PROTECTED, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_TIMED_OUT:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_DEVICE_NOT_READY, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_WOULD_RECURSE:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_RECURSIVE, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_BUSY:
        case G_IO_ERROR_WOULD_BLOCK:
            { ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, ucb::IOErrorCode_LOCKING_VIOLATION, aArgs);
            EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_HOST_NOT_FOUND:
            { ucb::InteractiveNetworkResolveNameException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR, sHost);
              EXCEPT(aExcept);}
            break;
        default:
        case G_IO_ERROR_ALREADY_MOUNTED:
        case G_IO_ERROR_NOT_EMPTY:
        case G_IO_ERROR_NOT_SYMBOLIC_LINK:
        case G_IO_ERROR_NOT_MOUNTABLE_FILE:
        case G_IO_ERROR_FAILED_HANDLED:
            { ucb::InteractiveNetworkGeneralException aExcept(sMessage, rContext,
                task::InteractionClassification_ERROR);
              EXCEPT(aExcept);}
            break;
    }
    return aRet;
}

uno::Any Content::mapGIOError( GError *pError )
{
    if (!pError)
        return getBadArgExcept();

    return convertToException(pError, static_cast< cppu::OWeakObject * >(this), false);
}

uno::Any Content::getBadArgExcept()
{
    return uno::makeAny( lang::IllegalArgumentException(
        OUString("Wrong argument type!"),
        static_cast< cppu::OWeakObject * >( this ), -1) );
}

class MountOperation
{
    GMainLoop *mpLoop;
    GMountOperation *mpAuthentication;
    GError *mpError;
    static void Completed(GObject *source, GAsyncResult *res, gpointer user_data);
public:
    MountOperation(const uno::Reference< ucb::XCommandEnvironment >& xEnv);
    ~MountOperation();
    GError *Mount(GFile *pFile);
};

MountOperation::MountOperation(const uno::Reference< ucb::XCommandEnvironment >& xEnv) : mpError(NULL)
{
    mpLoop = g_main_loop_new(NULL, FALSE);
    mpAuthentication = ooo_mount_operation_new(xEnv);
}

void MountOperation::Completed(GObject *source, GAsyncResult *res, gpointer user_data)
{
    MountOperation *pThis = (MountOperation*)user_data;
    g_file_mount_enclosing_volume_finish(G_FILE(source), res, &(pThis->mpError));
    g_main_loop_quit(pThis->mpLoop);
}

GError *MountOperation::Mount(GFile *pFile)
{
    g_file_mount_enclosing_volume(pFile, G_MOUNT_MOUNT_NONE, mpAuthentication, NULL, MountOperation::Completed, this);
    g_main_loop_run(mpLoop);
    return mpError;
}

MountOperation::~MountOperation()
{
    g_object_unref(mpAuthentication);
    g_main_loop_unref(mpLoop);
}

GFileInfo* Content::getGFileInfo(const uno::Reference< ucb::XCommandEnvironment >& xEnv, GError **ppError)
{
    GError * err = 0;
    if (mpInfo == 0 && !mbTransient) {
        for (bool retried = false;; retried = true) {
            mpInfo = g_file_query_info(
                getGFile(), "*", G_FILE_QUERY_INFO_NONE, 0, &err);
            if (mpInfo != 0) {
                break;
            }
            assert(err != 0);
            if (err->code != G_IO_ERROR_NOT_MOUNTED || retried) {
                break;
            }
            SAL_INFO(
                "ucb.ucp.gio",
                "G_IO_ERROR_NOT_MOUNTED \"" << err->message
                    << "\", trying to mount");
            g_error_free(err);
            err = MountOperation(xEnv).Mount(getGFile());
            if (err != 0) {
                break;
            }
        }
    }
    if (ppError != 0) {
        *ppError = err;
    } else if (err != 0) {
        SAL_WARN("ucb.ucp.gio", "ignoring GError \"" << err->message << "\"");
        g_error_free(err);
    }
    return mpInfo;
}

GFile* Content::getGFile()
{
    if (!mpFile)
        mpFile = g_file_new_for_uri(OUStringToOString(m_xIdentifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8).getStr());
    return mpFile;
}

bool Content::isFolder(const uno::Reference< ucb::XCommandEnvironment >& xEnv)
{
    GFileInfo *pInfo = getGFileInfo(xEnv);
    return pInfo && (g_file_info_get_file_type(pInfo) == G_FILE_TYPE_DIRECTORY);
}

static util::DateTime getDateFromUnix (time_t t)
{
    TimeValue tv;
    tv.Nanosec = 0;
    tv.Seconds = t;
    oslDateTime dt;

    if ( osl_getDateTimeFromTimeValue( &tv, &dt ) )
        return util::DateTime( 0, dt.Seconds, dt.Minutes, dt.Hours,
                   dt.Day, dt.Month, dt.Year, false);
    else
        return util::DateTime();
}

uno::Reference< sdbc::XRow > Content::getPropertyValuesFromGFileInfo(GFileInfo *pInfo,
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv,
    const uno::Sequence< beans::Property >& rProperties)
{
    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( rxContext );

    sal_Int32 nProps;
    const beans::Property* pProps;

    nProps = rProperties.getLength();
    pProps = rProperties.getConstArray();

    for( sal_Int32 n = 0; n < nProps; ++n )
    {
        const beans::Property& rProp = pProps[ n ];

        if ( rProp.Name == "IsDocument" )
        {
            if (pInfo != 0 && g_file_info_has_attribute(pInfo, G_FILE_ATTRIBUTE_STANDARD_TYPE))
                xRow->appendBoolean( rProp, ( g_file_info_get_file_type( pInfo ) == G_FILE_TYPE_REGULAR ||
                                               g_file_info_get_file_type( pInfo ) == G_FILE_TYPE_UNKNOWN ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsFolder" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_STANDARD_TYPE) )
                xRow->appendBoolean( rProp, ( g_file_info_get_file_type( pInfo ) == G_FILE_TYPE_DIRECTORY ));
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "Title" )
        {
            if (pInfo != 0 && g_file_info_has_attribute(pInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME))
            {
                const char *pName = g_file_info_get_display_name(pInfo);
                xRow->appendString( rProp, OUString(pName, strlen(pName), RTL_TEXTENCODING_UTF8) );
            }
            else
                xRow->appendVoid(rProp);
        }
        else if ( rProp.Name == "IsReadOnly" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE ) )
                xRow->appendBoolean( rProp, !g_file_info_get_attribute_boolean( pInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "DateCreated" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_TIME_CREATED ) )
                xRow->appendTimestamp( rProp, getDateFromUnix(g_file_info_get_attribute_uint64(pInfo, G_FILE_ATTRIBUTE_TIME_CREATED)) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "DateModified" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo,  G_FILE_ATTRIBUTE_TIME_CHANGED ) )
                xRow->appendTimestamp( rProp, getDateFromUnix(g_file_info_get_attribute_uint64(pInfo, G_FILE_ATTRIBUTE_TIME_CHANGED)) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "Size" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_STANDARD_SIZE) )
                xRow->appendLong( rProp, ( g_file_info_get_size( pInfo ) ));
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsVolume" )
        {
            //What do we use this for ?
            xRow->appendBoolean( rProp, sal_False );
        }
        else if ( rProp.Name == "IsCompactDisc" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT ) )
                xRow->appendBoolean( rProp, g_file_info_get_attribute_boolean(pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsRemoveable" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT ) )
                xRow->appendBoolean( rProp, g_file_info_get_attribute_boolean(pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsFloppy" )
        {
            xRow->appendBoolean( rProp, sal_False );
        }
        else if ( rProp.Name == "IsHidden" )
        {
            if (pInfo != 0 && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN) )
                xRow->appendBoolean( rProp, ( g_file_info_get_is_hidden ( pInfo ) ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "CreatableContentsInfo" )
        {
            xRow->appendObject( rProp, uno::makeAny( queryCreatableContentsInfo( xEnv ) ) );
        }
        else
        {
            SAL_WARN(
                "ucb.ucp.gio",
                "Looking for unsupported property " << rProp.Name);
        }
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

uno::Reference< sdbc::XRow > Content::getPropertyValues(
                const uno::Sequence< beans::Property >& rProperties,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    GFileInfo *pInfo = getGFileInfo(xEnv);
    return getPropertyValuesFromGFileInfo(pInfo, m_xContext, xEnv, rProperties);
}

static lang::IllegalAccessException
getReadOnlyException( const uno::Reference< uno::XInterface >& rContext )
{
    return lang::IllegalAccessException ("Property is read-only!", rContext );
}

void Content::queryChildren( ContentRefList& rChildren )
{
    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
        aURL += OUString("/");

    sal_Int32 nLen = aURL.getLength();

    ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ucbhelper::ContentImplHelperRef xChild = (*it);
        OUString aChildURL = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) && ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) || ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes / only a final slash. It's a child!
                rChildren.push_back( ::gio::Content::ContentRef (static_cast< ::gio::Content * >(xChild.get() ) ) );
            }
        }
        ++it;
    }
}

sal_Bool Content::exchangeIdentity( const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    uno::Reference< ucb::XContent > xThis = this;

    if ( mbTransient )
    {
        m_xIdentifier = xNewId;
        return sal_False;
    }

    OUString aOldURL = m_xIdentifier->getContentIdentifier();

    // Exchange own identitity.
    if ( exchange( xNewId ) )
    {
        // Process instanciated children...
        ContentRefList aChildren;
        queryChildren( aChildren );

        ContentRefList::const_iterator it  = aChildren.begin();
        ContentRefList::const_iterator end = aChildren.end();

        while ( it != end )
        {
            ContentRef xChild = (*it);

            // Create new content identifier for the child...
            uno::Reference< ucb::XContentIdentifier > xOldChildId = xChild->getIdentifier();
            OUString aOldChildURL = xOldChildId->getContentIdentifier();
            OUString aNewChildURL = aOldChildURL.replaceAt(
                0, aOldURL.getLength(), xNewId->getContentIdentifier() );

            uno::Reference< ucb::XContentIdentifier > xNewChildId
                = new ::ucbhelper::ContentIdentifier( aNewChildURL );

            if ( !xChild->exchangeIdentity( xNewChildId ) )
                return sal_False;

            ++it;
         }
         return sal_True;
    }

    return sal_False;
}

uno::Sequence< uno::Any > Content::setPropertyValues(
    const uno::Sequence< beans::PropertyValue >& rValues,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    GError *pError=NULL;
    GFileInfo *pNewInfo=NULL;
    GFileInfo *pInfo = getGFileInfo(xEnv, &pError);
    if (pInfo)
        pNewInfo = g_file_info_dup(pInfo);
    else
    {
        if (!mbTransient)
            ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
        else
        {
            if (pError)
                g_error_free(pError);
            pNewInfo = g_file_info_new();
        }
    }

    sal_Int32 nCount = rValues.getLength();

    beans::PropertyChangeEvent aEvent;
    aEvent.Source = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further = sal_False;
    aEvent.PropertyHandle = -1;

    sal_Int32 nChanged = 0, nTitlePos = -1;
    const char *newName = NULL;
    uno::Sequence< beans::PropertyChangeEvent > aChanges(nCount);

    uno::Sequence< uno::Any > aRet( nCount );
    const beans::PropertyValue* pValues = rValues.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];
#if OSL_DEBUG_LEVEL > 1
        g_warning("Set prop '%s'", OUStringToOString(rValue.Name, RTL_TEXTENCODING_UTF8).getStr());
#endif
        if ( rValue.Name == "ContentType" ||
             rValue.Name == "MediaType" ||
             rValue.Name == "IsDocument" ||
             rValue.Name == "IsFolder" ||
             rValue.Name == "Size" ||
             rValue.Name == "CreatableContentsInfo" )
        {
            aRet[ n ] <<= getReadOnlyException( static_cast< cppu::OWeakObject * >(this) );
        }
        else if ( rValue.Name == "Title" )
        {
            OUString aNewTitle;
            if (!( rValue.Value >>= aNewTitle ))
            {
                aRet[ n ] <<= beans::IllegalTypeException
                    ( OUString("Property value has wrong type!"),
                      static_cast< cppu::OWeakObject * >( this ) );
                continue;
            }

            if ( aNewTitle.getLength() <= 0 )
            {
                aRet[ n ] <<= lang::IllegalArgumentException
                    ( OUString("Empty title not allowed!"),
                      static_cast< cppu::OWeakObject * >( this ), -1 );
                continue;

            }

            OString sNewTitle = OUStringToOString(aNewTitle, RTL_TEXTENCODING_UTF8);
            newName = sNewTitle.getStr();
            const char *oldName = g_file_info_get_name( pInfo);

            if (!newName || !oldName || strcmp(newName, oldName))
            {
#if OSL_DEBUG_LEVEL > 1
                g_warning ("Set new name to '%s'", newName);
#endif

                aEvent.PropertyName = OUString("Title");
                if (oldName)
                    aEvent.OldValue = uno::makeAny(OUString(oldName, strlen(oldName), RTL_TEXTENCODING_UTF8));
                aEvent.NewValue = uno::makeAny(aNewTitle);
                aChanges.getArray()[ nChanged ] = aEvent;
                nTitlePos = nChanged++;

                g_file_info_set_name(pNewInfo, newName);
            }
        }
        else
        {
#ifdef DEBUG
            fprintf(stderr, "Unknown property %s\n", OUStringToOString(rValue.Name, RTL_TEXTENCODING_UTF8).getStr());
#endif
            aRet[ n ] <<= getReadOnlyException( static_cast< cppu::OWeakObject * >(this) );
            //TODO
        }
    }

    if (nChanged)
    {
        bool bOk = true;
        if (!mbTransient)
        {
            if ((bOk = doSetFileInfo(pNewInfo)))
            {
                for (sal_Int32 i = 0; i < nChanged; ++i)
                    aRet[ i ] <<= getBadArgExcept();
            }
        }

        if (bOk)
        {
            if (nTitlePos > -1)
            {
                OUString aNewURL = getParentURL();
                aNewURL += OUString( newName, strlen(newName), RTL_TEXTENCODING_UTF8 );
                uno::Reference< ucb::XContentIdentifier > xNewId
                    = new ::ucbhelper::ContentIdentifier( aNewURL );

                if (!exchangeIdentity( xNewId ) )
                {
                    aRet[ nTitlePos ] <<= uno::Exception
                        ( OUString("Exchange failed!"),
                          static_cast< cppu::OWeakObject * >( this ) );
                }
            }

            if (!mbTransient) //Discard and refetch
            {
                g_object_unref(mpInfo);
                mpInfo = NULL;
            }

            if (mpInfo)
            {
                g_file_info_copy_into(pNewInfo, mpInfo);
                g_object_unref(pNewInfo);
            }
            else
                mpInfo = pNewInfo;

            if (mpFile) //Discard and refetch
            {
                g_object_unref(mpFile);
                mpFile = NULL;
            }
        }

        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}

bool Content::doSetFileInfo(GFileInfo *pNewInfo)
{
    g_assert (!mbTransient);

    bool bOk = true;
    GFile *pFile = getGFile();
    if(!g_file_set_attributes_from_info(pFile, pNewInfo, G_FILE_QUERY_INFO_NONE, NULL, NULL))
        bOk = false;
    return bOk;
}

const int TRANSFER_BUFFER_SIZE = 65536;

void Content::copyData( uno::Reference< io::XInputStream > xIn,
    uno::Reference< io::XOutputStream > xOut )
{
    uno::Sequence< sal_Int8 > theData( TRANSFER_BUFFER_SIZE );

    g_return_if_fail( xIn.is() && xOut.is() );

    while ( xIn->readBytes( theData, TRANSFER_BUFFER_SIZE ) > 0 )
        xOut->writeBytes( theData );

    xOut->closeOutput();
}

sal_Bool Content::feedSink( uno::Reference< uno::XInterface > xSink,
    const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    if ( !xSink.is() )
        return sal_False;

    uno::Reference< io::XOutputStream > xOut = uno::Reference< io::XOutputStream >(xSink, uno::UNO_QUERY );
    uno::Reference< io::XActiveDataSink > xDataSink = uno::Reference< io::XActiveDataSink >(xSink, uno::UNO_QUERY );

    if ( !xOut.is() && !xDataSink.is() )
        return sal_False;

    GError *pError=NULL;
    GFileInputStream *pStream = g_file_read(getGFile(), NULL, &pError);
    if (!pStream)
       convertToException(pError, static_cast< cppu::OWeakObject * >(this));

    uno::Reference< io::XInputStream > xIn = new ::gio::InputStream(pStream);
    if ( !xIn.is() )
        return sal_False;

    if ( xOut.is() )
        copyData( xIn, xOut );

    if ( xDataSink.is() )
        xDataSink->setInputStream( xIn );

    return sal_True;
}

uno::Any Content::open(const ucb::OpenCommandArgument2 & rOpenCommand,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    bool bIsFolder = isFolder(xEnv);

    if (!g_file_query_exists(getGFile(), NULL))
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[ 0 ] <<= m_xIdentifier->getContentIdentifier();
        uno::Any aErr = uno::makeAny(
            ucb::InteractiveAugmentedIOException(OUString(), static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                bIsFolder ? ucb::IOErrorCode_NOT_EXISTING_PATH : ucb::IOErrorCode_NOT_EXISTING, aArgs)
        );

        ucbhelper::cancelCommandExecution(aErr, xEnv);
    }

    uno::Any aRet;

    sal_Bool bOpenFolder = (
        ( rOpenCommand.Mode == ucb::OpenMode::ALL ) ||
        ( rOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
        ( rOpenCommand.Mode == ucb::OpenMode::DOCUMENTS )
     );

    if ( bOpenFolder && bIsFolder )
    {
        uno::Reference< ucb::XDynamicResultSet > xSet
            = new DynamicResultSet( m_xContext, this, rOpenCommand, xEnv );
        aRet <<= xSet;
    }
    else if ( rOpenCommand.Sink.is() )
    {
        if (
            ( rOpenCommand.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
            ( rOpenCommand.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE )
           )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny ( ucb::UnsupportedOpenModeException
                    ( OUString(), static_cast< cppu::OWeakObject * >( this ),
                      sal_Int16( rOpenCommand.Mode ) ) ),
                    xEnv );
        }

        if ( !feedSink( rOpenCommand.Sink, xEnv ) )
        {
            // Note: rOpenCommand.Sink may contain an XStream
            //       implementation. Support for this type of
            //       sink is optional...
#ifdef DEBUG
            g_warning ("Failed to load data from '%s'",
                OUStringToOString(m_xIdentifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8).getStr());
#endif

            ucbhelper::cancelCommandExecution(
                uno::makeAny (ucb::UnsupportedDataSinkException
                    ( OUString(), static_cast< cppu::OWeakObject * >( this ),
                      rOpenCommand.Sink ) ),
                    xEnv );
        }
    }
    else
        g_warning ("Open falling through ...");
    return aRet;
}

uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "Content::execute %s\n", OUStringToOString(aCommand.Name, RTL_TEXTENCODING_UTF8).getStr());
#endif
    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {
        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet <<= getPropertyValues( Properties, xEnv );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
        aRet <<= getPropertySetInfo( xEnv, sal_False );
    else if ( aCommand.Name == "getCommandInfo" )
        aRet <<= getCommandInfo( xEnv, sal_False );
    else if ( aCommand.Name == "open" )
    {
        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet = open( aOpenCommand, xEnv );
    }
    else if ( aCommand.Name == "transfer" )
    {
        ucb::TransferInfo transferArgs;
        if ( !( aCommand.Argument >>= transferArgs ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        transfer( transferArgs, xEnv );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) || !aProperties.getLength() )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet <<= setPropertyValues( aProperties, xEnv );
    }
    else if (aCommand.Name == "createNewContent"
             && isFolder( xEnv ) )
    {
        ucb::ContentInfo arg;
        if ( !( aCommand.Argument >>= arg ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet <<= createNewContent( arg );
    }
    else if ( aCommand.Name == "insert" )
    {
        ucb::InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        insert( arg.Data, arg.ReplaceExisting, xEnv );
    }
    else if ( aCommand.Name == "delete" )
    {
        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;

        //If no delete physical, try and trashcan it, if that doesn't work go
        //ahead and try and delete it anyway
        if (!bDeletePhysical && !g_file_trash(getGFile(), NULL, NULL))
                bDeletePhysical = true;

        if (bDeletePhysical)
        {
            GError *pError = NULL;
            if (!g_file_delete( getGFile(), NULL, &pError))
                ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
        }

        destroy( bDeletePhysical );
    }
    else
    {
#ifdef DEBUG
        fprintf(stderr, "UNKNOWN COMMAND\n");
        //TODO
#endif

        ucbhelper::cancelCommandExecution
            ( uno::makeAny( ucb::UnsupportedCommandException
              ( OUString(),
                static_cast< cppu::OWeakObject * >( this ) ) ),
              xEnv );
    }

    return aRet;
}

void Content::destroy( sal_Bool bDeletePhysical )
    throw( uno::Exception )
{
    uno::Reference< ucb::XContent > xThis = this;

    deleted();

    ::gio::Content::ContentRefList aChildren;
    queryChildren( aChildren );

    ContentRefList::const_iterator it  = aChildren.begin();
    ContentRefList::const_iterator end = aChildren.end();

    while ( it != end )
    {
        (*it)->destroy( bDeletePhysical );
        ++it;
    }
}

void Content::insert(const uno::Reference< io::XInputStream > &xInputStream,
    sal_Bool bReplaceExisting, const uno::Reference< ucb::XCommandEnvironment > &xEnv )
        throw( uno::Exception )
{
    GError *pError = NULL;
    GFileInfo *pInfo = getGFileInfo(xEnv);

    if ( pInfo &&
         g_file_info_has_attribute(pInfo, G_FILE_ATTRIBUTE_STANDARD_TYPE) &&
         g_file_info_get_file_type(pInfo) == G_FILE_TYPE_DIRECTORY )
    {
#if OSL_DEBUG_LEVEL > 1
        g_warning ("Make directory");
#endif
        if( !g_file_make_directory( getGFile(), NULL, &pError))
            ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
        return;
    }

    if ( !xInputStream.is() )
    {
        ucbhelper::cancelCommandExecution( uno::makeAny
            ( ucb::MissingInputStreamException
              ( OUString(), static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
    }

    GFileOutputStream* pOutStream = NULL;
    if ( bReplaceExisting )
    {
        if (!(pOutStream = g_file_replace(getGFile(), NULL, false, G_FILE_CREATE_PRIVATE, NULL, &pError)))
            ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
    }
    else
    {
        if (!(pOutStream = g_file_create (getGFile(), G_FILE_CREATE_PRIVATE, NULL, &pError)))
            ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
    }

    uno::Reference < io::XOutputStream > xOutput = new ::gio::OutputStream(pOutStream);
    copyData( xInputStream, xOutput );

    if (mbTransient)
    {
        mbTransient = sal_False;
        inserted();
    }
}

const GFileCopyFlags DEFAULT_COPYDATA_FLAGS =
    static_cast<GFileCopyFlags>(G_FILE_COPY_OVERWRITE|G_FILE_COPY_TARGET_DEFAULT_PERMS);

void Content::transfer( const ucb::TransferInfo& aTransferInfo, const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception )
{
    OUString sDest = m_xIdentifier->getContentIdentifier();
    if (!sDest.endsWith("/")) {
        sDest += "/";
    }
    if (aTransferInfo.NewTitle.getLength())
        sDest += aTransferInfo.NewTitle;
    else
        sDest += OUString::createFromAscii(g_file_get_basename(getGFile()));

    GFile *pDest = g_file_new_for_uri(OUStringToOString(sDest, RTL_TEXTENCODING_UTF8).getStr());
    GFile *pSource = g_file_new_for_uri(OUStringToOString(aTransferInfo.SourceURL, RTL_TEXTENCODING_UTF8).getStr());

    gboolean bSuccess = false;
    GError *pError = NULL;
    if (aTransferInfo.MoveData)
        bSuccess = g_file_move(pSource, pDest, G_FILE_COPY_OVERWRITE, NULL, NULL, 0, &pError);
    else
        bSuccess = g_file_copy(pSource, pDest, DEFAULT_COPYDATA_FLAGS, NULL, NULL, 0, &pError);
    g_object_unref(pSource);
    g_object_unref(pDest);
    if (!bSuccess)
        ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
}

uno::Sequence< ucb::ContentInfo > Content::queryCreatableContentsInfo(
    const uno::Reference< ucb::XCommandEnvironment >& xEnv)
            throw( uno::RuntimeException )
{
    if ( isFolder( xEnv ) )
    {
        uno::Sequence< ucb::ContentInfo > seq(2);

        // Minimum set of props we really need
        uno::Sequence< beans::Property > props( 1 );
        props[0] = beans::Property(
            OUString("Title"),
            -1,
            getCppuType( static_cast< OUString* >( 0 ) ),
            beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::BOUND );

        // file
        seq[0].Type       = OUString( GIO_FILE_TYPE );
        seq[0].Attributes = ( ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM |
                              ucb::ContentInfoAttribute::KIND_DOCUMENT );
        seq[0].Properties = props;

        // folder
        seq[1].Type       = OUString( GIO_FOLDER_TYPE );
        seq[1].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;
        seq[1].Properties = props;

        return seq;
    }
    else
    {
        return uno::Sequence< ucb::ContentInfo >();
    }
}

uno::Sequence< ucb::ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
            throw( uno::RuntimeException )
{
    return queryCreatableContentsInfo( uno::Reference< ucb::XCommandEnvironment >() );
}

uno::Reference< ucb::XContent >
    SAL_CALL Content::createNewContent( const ucb::ContentInfo& Info )
        throw( uno::RuntimeException )
{
    bool create_document;
    const char *name;

    if ( Info.Type == GIO_FILE_TYPE )
        create_document = true;
    else if ( Info.Type == GIO_FOLDER_TYPE )
        create_document = false;
    else
    {
#ifdef DEBUG
        g_warning( "Failed to create new content '%s'", OUStringToOString(Info.Type,
            RTL_TEXTENCODING_UTF8).getStr() );
#endif
        return uno::Reference< ucb::XContent >();
    }

#if OSL_DEBUG_LEVEL > 1
    g_warning( "createNewContent (%d)", (int) create_document );
#endif

    OUString aURL = m_xIdentifier->getContentIdentifier();

    if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
            aURL += OUString("/");

    name = create_document ? "[New_Content]" : "[New_Collection]";
    aURL += OUString::createFromAscii( name );

    uno::Reference< ucb::XContentIdentifier > xId(new ::ucbhelper::ContentIdentifier(aURL));

    try
    {
        return new ::gio::Content( m_xContext, m_pProvider, xId, !create_document );
    } catch ( ucb::ContentCreationException & )
    {
            return uno::Reference< ucb::XContent >();
    }
}

uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException )
{
    if ( isFolder( uno::Reference< ucb::XCommandEnvironment >() ) )
    {
        static cppu::OTypeCollection aFolderCollection
            (CPPU_TYPE_REF( lang::XTypeProvider ),
             CPPU_TYPE_REF( lang::XServiceInfo ),
             CPPU_TYPE_REF( lang::XComponent ),
             CPPU_TYPE_REF( ucb::XContent ),
             CPPU_TYPE_REF( ucb::XCommandProcessor ),
             CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
             CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
             CPPU_TYPE_REF( beans::XPropertyContainer ),
             CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
             CPPU_TYPE_REF( container::XChild ),
             CPPU_TYPE_REF( ucb::XContentCreator ) );
        return aFolderCollection.getTypes();
    }
    else
    {
        static cppu::OTypeCollection aFileCollection
            (CPPU_TYPE_REF( lang::XTypeProvider ),
             CPPU_TYPE_REF( lang::XServiceInfo ),
             CPPU_TYPE_REF( lang::XComponent ),
             CPPU_TYPE_REF( ucb::XContent ),
             CPPU_TYPE_REF( ucb::XCommandProcessor ),
             CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
             CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
             CPPU_TYPE_REF( beans::XPropertyContainer ),
             CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
             CPPU_TYPE_REF( container::XChild ) );

        return aFileCollection.getTypes();
    }
}

uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    static const beans::Property aGenericProperties[] =
    {
        beans::Property( OUString(  "IsDocument"  ),
            -1, getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "IsFolder"  ),
            -1, getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "Title"  ),
            -1, getCppuType( static_cast< const OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND ),
        beans::Property( OUString(  "IsReadOnly"  ),
            -1, getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "DateCreated"  ),
            -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "DateModified"  ),
            -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "Size"  ),
            -1, getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "IsVolume"  ),
            -1, getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "IsCompactDisc"  ),
            -1, getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "IsRemoveable"  ),
            -1, getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "IsHidden"  ),
            -1, getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( OUString(  "CreatableContentsInfo"  ),
            -1, getCppuType( static_cast< const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY )
    };

    const int nProps = sizeof (aGenericProperties) / sizeof (aGenericProperties[0]);
    return uno::Sequence< beans::Property > ( aGenericProperties, nProps );
}

uno::Sequence< ucb::CommandInfo > Content::getCommands( const uno::Reference< ucb::XCommandEnvironment > & xEnv)
{
    static const ucb::CommandInfo aCommandInfoTable[] =
    {
        // Required commands
        ucb::CommandInfo
        ( OUString(  "getCommandInfo"  ),
          -1, getCppuVoidType() ),
        ucb::CommandInfo
        ( OUString(  "getPropertySetInfo"  ),
          -1, getCppuVoidType() ),
        ucb::CommandInfo
        ( OUString(  "getPropertyValues"  ),
          -1, getCppuType( static_cast<uno::Sequence< beans::Property > * >( 0 ) ) ),
        ucb::CommandInfo
        ( OUString(  "setPropertyValues"  ),
          -1, getCppuType( static_cast<uno::Sequence< beans::PropertyValue > * >( 0 ) ) ),

        // Optional standard commands
        ucb::CommandInfo
        ( OUString(  "delete"  ),
          -1, getCppuBooleanType() ),
        ucb::CommandInfo
        ( OUString(  "insert"  ),
          -1, getCppuType( static_cast<ucb::InsertCommandArgument * >( 0 ) ) ),
        ucb::CommandInfo
        ( OUString(  "open"  ),
          -1, getCppuType( static_cast<ucb::OpenCommandArgument2 * >( 0 ) ) ),

        // Folder Only, omitted if not a folder
        ucb::CommandInfo
        ( OUString(  "transfer"  ),
          -1, getCppuType( static_cast<ucb::TransferInfo * >( 0 ) ) ),
        ucb::CommandInfo
        ( OUString(  "createNewContent"  ),
          -1, getCppuType( static_cast<ucb::ContentInfo * >( 0 ) ) )
    };

    const int nProps = sizeof (aCommandInfoTable) / sizeof (aCommandInfoTable[0]);
    return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, isFolder(xEnv) ? nProps : nProps - 2);
}

XTYPEPROVIDER_COMMON_IMPL( Content );

void SAL_CALL Content::acquire() throw()
{
    ContentImplHelper::acquire();
}

void SAL_CALL Content::release() throw()
{
    ContentImplHelper::release();
}

uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType ) throw ( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType, static_cast< ucb::XContentCreator * >( this ) );
    return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface(rType);
}

OUString SAL_CALL Content::getImplementationName() throw( uno::RuntimeException )
{
       return OUString("com.sun.star.comp.GIOContent");
}

uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
       throw( uno::RuntimeException )
{
       uno::Sequence< OUString > aSNS( 1 );
       aSNS.getArray()[ 0 ] = OUString("com.sun.star.ucb.GIOContent");
       return aSNS;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

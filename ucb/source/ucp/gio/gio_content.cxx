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

#include <sal/config.h>

#include <utility>

#include <string.h>
#include <sys/types.h>
#include <sal/macros.h>
#include <osl/time.h>
#include <sal/log.hxx>

#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkGeneralException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>

#include <comphelper/seekableinput.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/macros.hxx>
#include <vcl/svapp.hxx>

#include "gio_content.hxx"
#include "gio_provider.hxx"
#include "gio_resultset.hxx"
#include "gio_inputstream.hxx"
#include "gio_outputstream.hxx"
#include "gio_mount.hxx"

namespace gio
{

Content::Content(
    const css::uno::Reference< css::uno::XComponentContext >& rxContext,
    ContentProvider* pProvider,
    const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier)
    : ContentImplHelper( rxContext, pProvider, Identifier ),
      m_pProvider( pProvider ), mpFile (nullptr), mpInfo( nullptr ), mbTransient(false)
{
    SAL_INFO("ucb.ucp.gio", "New Content ('" << m_xIdentifier->getContentIdentifier() << "')");
}

Content::Content(
    const css::uno::Reference< css::uno::XComponentContext >& rxContext,
    ContentProvider* pProvider,
    const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
    bool bIsFolder)
    : ContentImplHelper( rxContext, pProvider, Identifier ),
      m_pProvider( pProvider ), mpFile (nullptr), mpInfo( nullptr ), mbTransient(true)
{
    SAL_INFO("ucb.ucp.gio", "Create Content ('" << m_xIdentifier->getContentIdentifier() << "')");
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
{
    //TODO
    //stick a map from each CommandId to a new GCancellable and propagate
    //it throughout the g_file_* calls
}

OUString SAL_CALL Content::getContentType()
{
    return isFolder(css::uno::Reference< css::ucb::XCommandEnvironment >())
        ? OUString( GIO_FOLDER_TYPE )
        : OUString( GIO_FILE_TYPE );
}

#define EXCEPT(aExcept) \
do { \
    if (bThrow) throw aExcept;\
    aRet <<= aExcept;\
} while(false)

css::uno::Any convertToException(GError *pError, const css::uno::Reference< css::uno::XInterface >& rContext, bool bThrow)
{
    css::uno::Any aRet;

    gint eCode = pError->code;
    OUString sMessage(pError->message, strlen(pError->message), RTL_TEXTENCODING_UTF8);
    g_error_free(pError);

    OUString sName;

    css::uno::Sequence< css::uno::Any > aArgs( 1 );
    aArgs[ 0 ] <<= sName;

    switch (eCode)
    {
        case G_IO_ERROR_FAILED:
            { css::io::IOException aExcept(sMessage, rContext);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_MOUNTED:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_NOT_EXISTING_PATH, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_FOUND:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_NOT_EXISTING, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_EXISTS:
            { css::ucb::NameClashException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, sName);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_INVALID_ARGUMENT:
            { css::lang::IllegalArgumentException aExcept(sMessage, rContext, -1 );
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_PERMISSION_DENIED:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_ACCESS_DENIED, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_IS_DIRECTORY:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_NO_FILE, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_REGULAR_FILE:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_NO_FILE, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_DIRECTORY:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_NO_DIRECTORY, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_FILENAME_TOO_LONG:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_NAME_TOO_LONG, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_FAILED_HANDLED: /* Operation failed and a helper program
                                           has already interacted with the user. Do not display any error
                                           dialog */
        case G_IO_ERROR_PENDING:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_PENDING, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_CLOSED:
        case G_IO_ERROR_CANCELLED:
        case G_IO_ERROR_TOO_MANY_LINKS:
        case G_IO_ERROR_WRONG_ETAG:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_GENERAL, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NOT_SUPPORTED:
        case G_IO_ERROR_CANT_CREATE_BACKUP:
        case G_IO_ERROR_WOULD_MERGE:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_NOT_SUPPORTED, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_NO_SPACE:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_OUT_OF_DISK_SPACE, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_INVALID_FILENAME:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_INVALID_CHARACTER, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_READ_ONLY:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_WRITE_PROTECTED, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_TIMED_OUT:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_DEVICE_NOT_READY, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_WOULD_RECURSE:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_RECURSIVE, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_BUSY:
        case G_IO_ERROR_WOULD_BLOCK:
            { css::ucb::InteractiveAugmentedIOException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, css::ucb::IOErrorCode_LOCKING_VIOLATION, aArgs);
              EXCEPT(aExcept); }
            break;
        case G_IO_ERROR_HOST_NOT_FOUND:
            { css::ucb::InteractiveNetworkResolveNameException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR, OUString());
              EXCEPT(aExcept);}
            break;
        default:
        case G_IO_ERROR_ALREADY_MOUNTED:
        case G_IO_ERROR_NOT_EMPTY:
        case G_IO_ERROR_NOT_SYMBOLIC_LINK:
        case G_IO_ERROR_NOT_MOUNTABLE_FILE:
            { css::ucb::InteractiveNetworkGeneralException aExcept(sMessage, rContext,
                css::task::InteractionClassification_ERROR);
              EXCEPT(aExcept);}
            break;
    }
    return aRet;
}

void convertToIOException(GError *pError, const css::uno::Reference< css::uno::XInterface >& rContext)
{
    try
    {
        convertToException(pError, rContext);
    }
    catch (const css::io::IOException&)
    {
        throw;
    }
    catch (const css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::uno::Exception& e)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException(
            "wrapped Exception " + e.Message,
            css::uno::Reference<css::uno::XInterface>(), a);
    }
}

css::uno::Any Content::mapGIOError( GError *pError )
{
    if (!pError)
        return getBadArgExcept();

    return convertToException(pError, static_cast< cppu::OWeakObject * >(this), false);
}

css::uno::Any Content::getBadArgExcept()
{
    return css::uno::makeAny( css::lang::IllegalArgumentException(
        "Wrong argument type!",
        static_cast< cppu::OWeakObject * >( this ), -1) );
}

namespace {

class MountOperation
{
    ucb::ucp::gio::glib::MainContextRef mContext;
    GMainLoop *mpLoop;
    GMountOperation *mpAuthentication;
    GError *mpError;
    static void Completed(GObject *source, GAsyncResult *res, gpointer user_data);
public:
    explicit MountOperation(const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv);
    ~MountOperation();
    GError *Mount(GFile *pFile);
};

}

MountOperation::MountOperation(const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv) : mpError(nullptr)
{
    ucb::ucp::gio::glib::MainContextRef oldContext(g_main_context_ref_thread_default());
    mContext.reset(g_main_context_new());
    mpLoop = g_main_loop_new(mContext.get(), FALSE);
    g_main_context_push_thread_default(mContext.get());
    mpAuthentication = ooo_mount_operation_new(std::move(oldContext), xEnv);
}

void MountOperation::Completed(GObject *source, GAsyncResult *res, gpointer user_data)
{
    MountOperation *pThis = static_cast<MountOperation*>(user_data);
    g_file_mount_enclosing_volume_finish(G_FILE(source), res, &(pThis->mpError));
    g_main_loop_quit(pThis->mpLoop);
}

GError *MountOperation::Mount(GFile *pFile)
{
    g_file_mount_enclosing_volume(pFile, G_MOUNT_MOUNT_NONE, mpAuthentication, nullptr, MountOperation::Completed, this);
    {
        //HACK: At least the gdk_threads_set_lock_functions(GdkThreadsEnter,
        // GdkThreadsLeave) call in vcl/unx/gtk/app/gtkinst.cxx will lead to
        // GdkThreadsLeave unlock the SolarMutex down to zero at the end of
        // g_main_loop_run, so we need ~SolarMutexReleaser to raise it back to
        // the original value again:
        if (comphelper::SolarMutex::get()->IsCurrentThread())
        {
            SolarMutexReleaser rel;
            g_main_loop_run(mpLoop);
        }
        else
        {
            g_main_loop_run(mpLoop);
        }
    }
    return mpError;
}

MountOperation::~MountOperation()
{
    g_object_unref(mpAuthentication);
    g_main_context_pop_thread_default(mContext.get());
    g_main_loop_unref(mpLoop);
}

GFileInfo* Content::getGFileInfo(const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv, GError **ppError)
{
    GError * err = nullptr;
    if (mpInfo == nullptr && !mbTransient) {
        for (bool retried = false;; retried = true) {
            mpInfo = g_file_query_info(
                getGFile(), "*", G_FILE_QUERY_INFO_NONE, nullptr, &err);
            if (mpInfo != nullptr) {
                break;
            }
            assert(err != nullptr);
            if (err->code != G_IO_ERROR_NOT_MOUNTED || retried) {
                break;
            }
            SAL_INFO(
                "ucb.ucp.gio",
                "G_IO_ERROR_NOT_MOUNTED \"" << err->message
                    << "\", trying to mount");
            g_error_free(err);
            err = MountOperation(xEnv).Mount(getGFile());
            if (err != nullptr) {
                break;
            }
        }
    }
    if (ppError != nullptr) {
        *ppError = err;
    } else if (err != nullptr) {
        SAL_WARN(
            "ucb.ucp.gio",
            "ignoring GError \"" << err->message << "\" for <"
                << m_xIdentifier->getContentIdentifier() << ">");
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

bool Content::isFolder(const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv)
{
    GFileInfo *pInfo = getGFileInfo(xEnv);
    return pInfo && (g_file_info_get_file_type(pInfo) == G_FILE_TYPE_DIRECTORY);
}

static css::util::DateTime getDateFromUnix (time_t t)
{
    TimeValue tv;
    tv.Nanosec = 0;
    tv.Seconds = t;
    oslDateTime dt;

    if ( osl_getDateTimeFromTimeValue( &tv, &dt ) )
        return css::util::DateTime( 0, dt.Seconds, dt.Minutes, dt.Hours,
                   dt.Day, dt.Month, dt.Year, false);
    else
        return css::util::DateTime();
}

css::uno::Reference< css::sdbc::XRow > Content::getPropertyValues(
                const css::uno::Sequence< css::beans::Property >& rProperties,
                const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
{
    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( m_xContext );

    GFileInfo *pInfo = nullptr;
    for( const css::beans::Property& rProp : rProperties )
    {
        if ( rProp.Name == "IsDocument" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute(pInfo, G_FILE_ATTRIBUTE_STANDARD_TYPE))
                xRow->appendBoolean( rProp, ( g_file_info_get_file_type( pInfo ) == G_FILE_TYPE_REGULAR ||
                                               g_file_info_get_file_type( pInfo ) == G_FILE_TYPE_UNKNOWN ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsFolder" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_STANDARD_TYPE) )
                xRow->appendBoolean( rProp, ( g_file_info_get_file_type( pInfo ) == G_FILE_TYPE_DIRECTORY ));
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "Title" )
        {
            getFileInfo(xEnv, &pInfo, false);
            if (pInfo != nullptr && g_file_info_has_attribute(pInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME))
            {
                const char *pName = g_file_info_get_display_name(pInfo);
                xRow->appendString( rProp, OUString(pName, strlen(pName), RTL_TEXTENCODING_UTF8) );
            }
            else
                xRow->appendVoid(rProp);
        }
        else if ( rProp.Name == "IsReadOnly" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE ) )
                xRow->appendBoolean( rProp, !g_file_info_get_attribute_boolean( pInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "DateCreated" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_TIME_CREATED ) )
                xRow->appendTimestamp( rProp, getDateFromUnix(g_file_info_get_attribute_uint64(pInfo, G_FILE_ATTRIBUTE_TIME_CREATED)) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "DateModified" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo,  G_FILE_ATTRIBUTE_TIME_CHANGED ) )
                xRow->appendTimestamp( rProp, getDateFromUnix(g_file_info_get_attribute_uint64(pInfo, G_FILE_ATTRIBUTE_TIME_CHANGED)) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "Size" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_STANDARD_SIZE) )
                xRow->appendLong( rProp, ( g_file_info_get_size( pInfo ) ));
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsVolume" )
        {
            //What do we use this for ?
            xRow->appendBoolean( rProp, false );
        }
        else if ( rProp.Name == "IsCompactDisc" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT ) )
                xRow->appendBoolean( rProp, g_file_info_get_attribute_boolean(pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsRemoveable" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT ) )
                xRow->appendBoolean( rProp, g_file_info_get_attribute_boolean(pInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "IsFloppy" )
        {
            xRow->appendBoolean( rProp, false );
        }
        else if ( rProp.Name == "IsHidden" )
        {
            getFileInfo(xEnv, &pInfo, true);
            if (pInfo != nullptr && g_file_info_has_attribute( pInfo, G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN) )
                xRow->appendBoolean( rProp, ( g_file_info_get_is_hidden ( pInfo ) ) );
            else
                xRow->appendVoid( rProp );
        }
        else if ( rProp.Name == "CreatableContentsInfo" )
        {
            xRow->appendObject( rProp, css::uno::makeAny( queryCreatableContentsInfo( xEnv ) ) );
        }
        else
        {
            SAL_WARN(
                "ucb.ucp.gio",
                "Looking for unsupported property " << rProp.Name);
        }
    }

    return xRow;
}

static css::lang::IllegalAccessException
getReadOnlyException( const css::uno::Reference< css::uno::XInterface >& rContext )
{
    return css::lang::IllegalAccessException ("Property is read-only!", rContext );
}

void Content::queryChildren( ContentRefList& rChildren )
{
    // Obtain a list with a snapshot of all currently instantiated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
        aURL += "/";

    sal_Int32 nLen = aURL.getLength();

    for ( const auto& rContent : aAllContents )
    {
        ucbhelper::ContentImplHelperRef xChild = rContent;
        OUString aChildURL = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) && aChildURL.startsWith( aURL ) )
        {
            sal_Int32 nPos = aChildURL.indexOf( '/', nLen );

            if ( ( nPos == -1 ) || ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes / only a final slash. It's a child!
                rChildren.emplace_back(static_cast< ::gio::Content * >(xChild.get() ) );
            }
        }
    }
}

bool Content::exchangeIdentity( const css::uno::Reference< css::ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return false;

    css::uno::Reference< css::ucb::XContent > xThis = this;

    if ( mbTransient )
    {
        m_xIdentifier = xNewId;
        return false;
    }

    OUString aOldURL = m_xIdentifier->getContentIdentifier();

    // Exchange own identity.
    if ( exchange( xNewId ) )
    {
        // Process instantiated children...
        ContentRefList aChildren;
        queryChildren( aChildren );

        for ( const auto& rChild : aChildren )
        {
            ContentRef xChild = rChild;

            // Create new content identifier for the child...
            css::uno::Reference< css::ucb::XContentIdentifier > xOldChildId = xChild->getIdentifier();
            OUString aOldChildURL = xOldChildId->getContentIdentifier();
            OUString aNewChildURL = aOldChildURL.replaceAt(
                0, aOldURL.getLength(), xNewId->getContentIdentifier() );

            css::uno::Reference< css::ucb::XContentIdentifier > xNewChildId
                = new ::ucbhelper::ContentIdentifier( aNewChildURL );

            if ( !xChild->exchangeIdentity( xNewChildId ) )
                return false;
        }
        return true;
    }

    return false;
}

void Content::getFileInfo(
    css::uno::Reference<css::ucb::XCommandEnvironment> const & env, GFileInfo ** info, bool fail)
{
    assert(info != nullptr);
    if (*info != nullptr)
        return;

    GError * err = nullptr;
    *info = getGFileInfo(env, &err);
    if (*info == nullptr && !mbTransient && fail)
    {
        ucbhelper::cancelCommandExecution(mapGIOError(err), env);
    }
    else if (err != nullptr)
    {
        g_error_free(err);
    }
}

css::uno::Sequence< css::uno::Any > Content::setPropertyValues(
    const css::uno::Sequence< css::beans::PropertyValue >& rValues,
    const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
{
    GError *pError=nullptr;
    GFileInfo *pNewInfo=nullptr;
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

    css::beans::PropertyChangeEvent aEvent;
    aEvent.Source = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further = false;
    aEvent.PropertyHandle = -1;

    sal_Int32 nChanged = 0, nTitlePos = -1;
    const char *newName = nullptr;
    css::uno::Sequence< css::beans::PropertyChangeEvent > aChanges(nCount);

    css::uno::Sequence< css::uno::Any > aRet( nCount );
    const css::beans::PropertyValue* pValues = rValues.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const css::beans::PropertyValue& rValue = pValues[ n ];
        SAL_INFO("ucb.ucp.gio", "Set prop '" << rValue.Name << "'");
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
                aRet[ n ] <<= css::beans::IllegalTypeException
                    ( "Property value has wrong type!",
                      static_cast< cppu::OWeakObject * >( this ) );
                continue;
            }

            if ( aNewTitle.isEmpty() )
            {
                aRet[ n ] <<= css::lang::IllegalArgumentException
                    ( "Empty title not allowed!",
                      static_cast< cppu::OWeakObject * >( this ), -1 );
                continue;

            }

            OString sNewTitle = OUStringToOString(aNewTitle, RTL_TEXTENCODING_UTF8);
            newName = sNewTitle.getStr();
            const char *oldName = g_file_info_get_name( pInfo);

            if (!newName || !oldName || strcmp(newName, oldName))
            {
                SAL_INFO("ucb.ucp.gio", "Set new name to '" << newName << "'");

                aEvent.PropertyName = "Title";
                if (oldName)
                    aEvent.OldValue <<= OUString(oldName, strlen(oldName), RTL_TEXTENCODING_UTF8);
                aEvent.NewValue <<= aNewTitle;
                aChanges.getArray()[ nChanged ] = aEvent;
                nTitlePos = nChanged++;

                g_file_info_set_name(pNewInfo, newName);
            }
        }
        else
        {
            SAL_WARN("ucb.ucp.gio", "Unknown property " << rValue.Name);
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
                    aRet[ i ] = getBadArgExcept();
            }
        }

        if (bOk)
        {
            if (nTitlePos > -1)
            {
                OUString aNewURL = getParentURL() +
                    OUString( newName, strlen(newName), RTL_TEXTENCODING_UTF8 );
                css::uno::Reference< css::ucb::XContentIdentifier > xNewId
                    = new ::ucbhelper::ContentIdentifier( aNewURL );

                if (!exchangeIdentity( xNewId ) )
                {
                    aRet[ nTitlePos ] <<= css::uno::Exception
                        ( "Exchange failed!",
                          static_cast< cppu::OWeakObject * >( this ) );
                }
            }

            if (!mbTransient) //Discard and refetch
            {
                g_object_unref(mpInfo);
                mpInfo = nullptr;
            }

            if (mpInfo)
            {
                g_file_info_copy_into(pNewInfo, mpInfo);
                g_object_unref(pNewInfo);
            }
            else
                mpInfo = pNewInfo;

            pNewInfo = nullptr;

            if (mpFile) //Discard and refetch
            {
                g_object_unref(mpFile);
                mpFile = nullptr;
            }
        }

        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    if (pNewInfo)
        g_object_unref(pNewInfo);

    return aRet;
}

bool Content::doSetFileInfo(GFileInfo *pNewInfo)
{
    g_assert (!mbTransient);

    bool bOk = true;
    GFile *pFile = getGFile();
    if(!g_file_set_attributes_from_info(pFile, pNewInfo, G_FILE_QUERY_INFO_NONE, nullptr, nullptr))
        bOk = false;
    return bOk;
}

const int TRANSFER_BUFFER_SIZE = 65536;

void Content::copyData( const css::uno::Reference< css::io::XInputStream >& xIn,
    const css::uno::Reference< css::io::XOutputStream >& xOut )
{
    css::uno::Sequence< sal_Int8 > theData( TRANSFER_BUFFER_SIZE );

    g_return_if_fail( xIn.is() && xOut.is() );

    while ( xIn->readBytes( theData, TRANSFER_BUFFER_SIZE ) > 0 )
        xOut->writeBytes( theData );

    xOut->closeOutput();
}

bool Content::feedSink( const css::uno::Reference< css::uno::XInterface >& xSink )
{
    if ( !xSink.is() )
        return false;

    css::uno::Reference< css::io::XOutputStream > xOut(xSink, css::uno::UNO_QUERY );
    css::uno::Reference< css::io::XActiveDataSink > xDataSink(xSink, css::uno::UNO_QUERY );

    if ( !xOut.is() && !xDataSink.is() )
        return false;

    GError *pError=nullptr;
    GFileInputStream *pStream = g_file_read(getGFile(), nullptr, &pError);
    if (!pStream)
       convertToException(pError, static_cast< cppu::OWeakObject * >(this));

    css::uno::Reference< css::io::XInputStream > xIn(
        new comphelper::OSeekableInputWrapper(
            new ::gio::InputStream(pStream), m_xContext));

    if ( xOut.is() )
        copyData( xIn, xOut );

    if ( xDataSink.is() )
        xDataSink->setInputStream( xIn );

    return true;
}

css::uno::Any Content::open(const css::ucb::OpenCommandArgument2 & rOpenCommand,
    const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
{
    bool bIsFolder = isFolder(xEnv);

    if (!g_file_query_exists(getGFile(), nullptr))
    {
        css::uno::Sequence< css::uno::Any > aArgs( 1 );
        aArgs[ 0 ] <<= m_xIdentifier->getContentIdentifier();
        css::uno::Any aErr = css::uno::makeAny(
            css::ucb::InteractiveAugmentedIOException(OUString(), static_cast< cppu::OWeakObject * >( this ),
                css::task::InteractionClassification_ERROR,
                bIsFolder ? css::ucb::IOErrorCode_NOT_EXISTING_PATH : css::ucb::IOErrorCode_NOT_EXISTING, aArgs)
        );

        ucbhelper::cancelCommandExecution(aErr, xEnv);
    }

    css::uno::Any aRet;

    bool bOpenFolder = (
        ( rOpenCommand.Mode == css::ucb::OpenMode::ALL ) ||
        ( rOpenCommand.Mode == css::ucb::OpenMode::FOLDERS ) ||
        ( rOpenCommand.Mode == css::ucb::OpenMode::DOCUMENTS )
     );

    if ( bOpenFolder && bIsFolder )
    {
        css::uno::Reference< css::ucb::XDynamicResultSet > xSet
            = new DynamicResultSet( m_xContext, this, rOpenCommand, xEnv );
        aRet <<= xSet;
    }
    else if ( rOpenCommand.Sink.is() )
    {
        if (
            ( rOpenCommand.Mode == css::ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
            ( rOpenCommand.Mode == css::ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE )
           )
        {
            ucbhelper::cancelCommandExecution(
                css::uno::makeAny ( css::ucb::UnsupportedOpenModeException
                    ( OUString(), static_cast< cppu::OWeakObject * >( this ),
                      sal_Int16( rOpenCommand.Mode ) ) ),
                    xEnv );
        }

        if ( !feedSink( rOpenCommand.Sink ) )
        {
            // Note: rOpenCommand.Sink may contain an XStream
            //       implementation. Support for this type of
            //       sink is optional...
            SAL_WARN("ucb.ucp.gio", "Failed to load data from '" << m_xIdentifier->getContentIdentifier() << "'");

            ucbhelper::cancelCommandExecution(
                css::uno::makeAny (css::ucb::UnsupportedDataSinkException
                    ( OUString(), static_cast< cppu::OWeakObject * >( this ),
                      rOpenCommand.Sink ) ),
                    xEnv );
        }
    }
    else
        SAL_INFO("ucb.ucp.gio", "Open falling through ...");
    return aRet;
}

css::uno::Any SAL_CALL Content::execute(
        const css::ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
{
    SAL_INFO("ucb.ucp.gio", "Content::execute " << aCommand.Name);
    css::uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {
        css::uno::Sequence< css::beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet <<= getPropertyValues( Properties, xEnv );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
        aRet <<= getPropertySetInfo( xEnv, false );
    else if ( aCommand.Name == "getCommandInfo" )
        aRet <<= getCommandInfo( xEnv, false );
    else if ( aCommand.Name == "open" )
    {
        css::ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet = open( aOpenCommand, xEnv );
    }
    else if ( aCommand.Name == "transfer" )
    {
        css::ucb::TransferInfo transferArgs;
        if ( !( aCommand.Argument >>= transferArgs ) )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        transfer( transferArgs, xEnv );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        css::uno::Sequence< css::beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) || !aProperties.hasElements() )
            ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet <<= setPropertyValues( aProperties, xEnv );
    }
    else if (aCommand.Name == "createNewContent"
             && isFolder( xEnv ) )
    {
        css::ucb::ContentInfo arg;
        if ( !( aCommand.Argument >>= arg ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        aRet <<= createNewContent( arg );
    }
    else if ( aCommand.Name == "insert" )
    {
        css::ucb::InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
        insert( arg.Data, arg.ReplaceExisting, xEnv );
    }
    else if ( aCommand.Name == "delete" )
    {
        bool bDeletePhysical = false;
        aCommand.Argument >>= bDeletePhysical;

        //If no delete physical, try and trashcan it, if that doesn't work go
        //ahead and try and delete it anyway
        if (!bDeletePhysical && !g_file_trash(getGFile(), nullptr, nullptr))
                bDeletePhysical = true;

        if (bDeletePhysical)
        {
            GError *pError = nullptr;
            if (!g_file_delete( getGFile(), nullptr, &pError))
                ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
        }

        destroy( bDeletePhysical );
    }
    else
    {
        SAL_WARN("ucb.ucp.gio", "Unknown command " << aCommand.Name);

        ucbhelper::cancelCommandExecution
            ( css::uno::makeAny( css::ucb::UnsupportedCommandException
              ( OUString(),
                static_cast< cppu::OWeakObject * >( this ) ) ),
              xEnv );
    }

    return aRet;
}

void Content::destroy( bool bDeletePhysical )
{
    css::uno::Reference< css::ucb::XContent > xThis = this;

    deleted();

    ::gio::Content::ContentRefList aChildren;
    queryChildren( aChildren );

    for ( auto& rChild : aChildren )
    {
        rChild->destroy( bDeletePhysical );
    }
}

void Content::insert(const css::uno::Reference< css::io::XInputStream > &xInputStream,
    bool bReplaceExisting, const css::uno::Reference< css::ucb::XCommandEnvironment > &xEnv )
{
    GError *pError = nullptr;
    GFileInfo *pInfo = getGFileInfo(xEnv);

    if ( pInfo &&
         g_file_info_has_attribute(pInfo, G_FILE_ATTRIBUTE_STANDARD_TYPE) &&
         g_file_info_get_file_type(pInfo) == G_FILE_TYPE_DIRECTORY )
    {
        SAL_INFO("ucb.ucp.gio", "Make directory");
        if( !g_file_make_directory( getGFile(), nullptr, &pError))
            ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
        return;
    }

    if ( !xInputStream.is() )
    {
        ucbhelper::cancelCommandExecution( css::uno::makeAny
            ( css::ucb::MissingInputStreamException
              ( OUString(), static_cast< cppu::OWeakObject * >( this ) ) ),
            xEnv );
    }

    GFileOutputStream* pOutStream = nullptr;
    if ( bReplaceExisting )
    {
        if (!(pOutStream = g_file_replace(getGFile(), nullptr, false, G_FILE_CREATE_PRIVATE, nullptr, &pError)))
            ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
    }
    else
    {
        if (!(pOutStream = g_file_create (getGFile(), G_FILE_CREATE_PRIVATE, nullptr, &pError)))
            ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
    }

    css::uno::Reference < css::io::XOutputStream > xOutput = new ::gio::OutputStream(pOutStream);
    copyData( xInputStream, xOutput );

    if (mbTransient)
    {
        mbTransient = false;
        inserted();
    }
}

const GFileCopyFlags DEFAULT_COPYDATA_FLAGS =
    static_cast<GFileCopyFlags>(G_FILE_COPY_OVERWRITE|G_FILE_COPY_TARGET_DEFAULT_PERMS);

void Content::transfer( const css::ucb::TransferInfo& aTransferInfo, const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
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

    bool bSuccess = false;
    GError *pError = nullptr;
    if (aTransferInfo.MoveData)
        bSuccess = g_file_move(pSource, pDest, G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, &pError);
    else
        bSuccess = g_file_copy(pSource, pDest, DEFAULT_COPYDATA_FLAGS, nullptr, nullptr, nullptr, &pError);
    g_object_unref(pSource);
    g_object_unref(pDest);
    if (!bSuccess) {
        SAL_INFO(
            "ucb.ucp.gio",
            "transfer <" << aTransferInfo.SourceURL << "> to <" << sDest << "> (MoveData = "
                << int(aTransferInfo.MoveData) << ") failed with \"" << pError->message << "\"");
        ucbhelper::cancelCommandExecution(mapGIOError(pError), xEnv);
    }
}

css::uno::Sequence< css::ucb::ContentInfo > Content::queryCreatableContentsInfo(
    const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv)
{
    if ( isFolder( xEnv ) )
    {
        css::uno::Sequence< css::ucb::ContentInfo > seq(2);

        // Minimum set of props we really need
        css::uno::Sequence< css::beans::Property > props( 1 );
        props[0] = css::beans::Property(
            "Title",
            -1,
            cppu::UnoType<OUString>::get(),
            css::beans::PropertyAttribute::MAYBEVOID | css::beans::PropertyAttribute::BOUND );

        // file
        seq[0].Type       = GIO_FILE_TYPE;
        seq[0].Attributes = ( css::ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM |
                              css::ucb::ContentInfoAttribute::KIND_DOCUMENT );
        seq[0].Properties = props;

        // folder
        seq[1].Type       = GIO_FOLDER_TYPE;
        seq[1].Attributes = css::ucb::ContentInfoAttribute::KIND_FOLDER;
        seq[1].Properties = props;

        return seq;
    }
    else
    {
        return css::uno::Sequence< css::ucb::ContentInfo >();
    }
}

css::uno::Sequence< css::ucb::ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
{
    return queryCreatableContentsInfo( css::uno::Reference< css::ucb::XCommandEnvironment >() );
}

css::uno::Reference< css::ucb::XContent >
    SAL_CALL Content::createNewContent( const css::ucb::ContentInfo& Info )
{
    bool create_document;
    const char *name;

    if ( Info.Type == GIO_FILE_TYPE )
        create_document = true;
    else if ( Info.Type == GIO_FOLDER_TYPE )
        create_document = false;
    else
    {
        SAL_WARN("ucb.ucp.gio", "Failed to create new content '" << Info.Type << "'");
        return css::uno::Reference< css::ucb::XContent >();
    }

    SAL_INFO("ucb.ucp.gio", "createNewContent (" << create_document << ")");
    OUString aURL = m_xIdentifier->getContentIdentifier();

    if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
            aURL += "/";

    name = create_document ? "[New_Content]" : "[New_Collection]";
    aURL += OUString::createFromAscii( name );

    css::uno::Reference< css::ucb::XContentIdentifier > xId(new ::ucbhelper::ContentIdentifier(aURL));

    try
    {
        return new ::gio::Content( m_xContext, m_pProvider, xId, !create_document );
    } catch ( css::ucb::ContentCreationException & )
    {
            return css::uno::Reference< css::ucb::XContent >();
    }
}

css::uno::Sequence< css::uno::Type > SAL_CALL Content::getTypes()
{
    if ( isFolder( css::uno::Reference< css::ucb::XCommandEnvironment >() ) )
    {
        static cppu::OTypeCollection s_aFolderCollection
            (CPPU_TYPE_REF( css::lang::XTypeProvider ),
             CPPU_TYPE_REF( css::lang::XServiceInfo ),
             CPPU_TYPE_REF( css::lang::XComponent ),
             CPPU_TYPE_REF( css::ucb::XContent ),
             CPPU_TYPE_REF( css::ucb::XCommandProcessor ),
             CPPU_TYPE_REF( css::beans::XPropertiesChangeNotifier ),
             CPPU_TYPE_REF( css::ucb::XCommandInfoChangeNotifier ),
             CPPU_TYPE_REF( css::beans::XPropertyContainer ),
             CPPU_TYPE_REF( css::beans::XPropertySetInfoChangeNotifier ),
             CPPU_TYPE_REF( css::container::XChild ),
             CPPU_TYPE_REF( css::ucb::XContentCreator ) );
        return s_aFolderCollection.getTypes();
    }
    else
    {
        static cppu::OTypeCollection s_aFileCollection
            (CPPU_TYPE_REF( css::lang::XTypeProvider ),
             CPPU_TYPE_REF( css::lang::XServiceInfo ),
             CPPU_TYPE_REF( css::lang::XComponent ),
             CPPU_TYPE_REF( css::ucb::XContent ),
             CPPU_TYPE_REF( css::ucb::XCommandProcessor ),
             CPPU_TYPE_REF( css::beans::XPropertiesChangeNotifier ),
             CPPU_TYPE_REF( css::ucb::XCommandInfoChangeNotifier ),
             CPPU_TYPE_REF( css::beans::XPropertyContainer ),
             CPPU_TYPE_REF( css::beans::XPropertySetInfoChangeNotifier ),
             CPPU_TYPE_REF( css::container::XChild ) );

        return s_aFileCollection.getTypes();
    }
}

css::uno::Sequence< css::beans::Property > Content::getProperties(
    const css::uno::Reference< css::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    static const css::beans::Property aGenericProperties[] =
    {
        css::beans::Property( "IsDocument",
            -1, cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "IsFolder",
            -1, cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "Title",
            -1, cppu::UnoType<OUString>::get(),
            css::beans::PropertyAttribute::BOUND ),
        css::beans::Property( "IsReadOnly",
            -1, cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "DateCreated",
            -1, cppu::UnoType<css::util::DateTime>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "DateModified",
            -1, cppu::UnoType<css::util::DateTime>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "Size",
            -1, cppu::UnoType<sal_Int64>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "IsVolume",
            1, cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "IsCompactDisc",
            -1, cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "IsRemoveable",
            -1, cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "IsHidden",
            -1, cppu::UnoType<bool>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( "CreatableContentsInfo",
            -1, cppu::UnoType<css::uno::Sequence< css::ucb::ContentInfo >>::get(),
            css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::READONLY )
    };

    const int nProps = SAL_N_ELEMENTS(aGenericProperties);
    return css::uno::Sequence< css::beans::Property > ( aGenericProperties, nProps );
}

css::uno::Sequence< css::ucb::CommandInfo > Content::getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv)
{
    static const css::ucb::CommandInfo aCommandInfoTable[] =
    {
        // Required commands
        css::ucb::CommandInfo
        ( "getCommandInfo",
          -1, cppu::UnoType<void>::get() ),
        css::ucb::CommandInfo
        ( "getPropertySetInfo",
          -1, cppu::UnoType<void>::get() ),
        css::ucb::CommandInfo
        ( "getPropertyValues",
          -1, cppu::UnoType<css::uno::Sequence< css::beans::Property >>::get() ),
        css::ucb::CommandInfo
        ( "setPropertyValues",
          -1, cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get() ),

        // Optional standard commands
        css::ucb::CommandInfo
        ( "delete",
          -1, cppu::UnoType<bool>::get() ),
        css::ucb::CommandInfo
        ( "insert",
          -1, cppu::UnoType<css::ucb::InsertCommandArgument>::get() ),
        css::ucb::CommandInfo
        ( "open",
          -1, cppu::UnoType<css::ucb::OpenCommandArgument2>::get() ),

        // Folder Only, omitted if not a folder
        css::ucb::CommandInfo
        ( "transfer",
          -1, cppu::UnoType<css::ucb::TransferInfo>::get() ),
        css::ucb::CommandInfo
        ( "createNewContent",
          -1, cppu::UnoType<css::ucb::ContentInfo>::get() )
    };

    const int nProps = SAL_N_ELEMENTS(aCommandInfoTable);
    return css::uno::Sequence< css::ucb::CommandInfo >(aCommandInfoTable, isFolder(xEnv) ? nProps : nProps - 2);
}

XTYPEPROVIDER_COMMON_IMPL( Content );

void SAL_CALL Content::acquire() noexcept
{
    ContentImplHelper::acquire();
}

void SAL_CALL Content::release() noexcept
{
    ContentImplHelper::release();
}

css::uno::Any SAL_CALL Content::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType, static_cast< css::ucb::XContentCreator * >( this ) );
    return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface(rType);
}

OUString SAL_CALL Content::getImplementationName()
{
       return "com.sun.star.comp.GIOContent";
}

css::uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
{
       css::uno::Sequence<OUString> aSNS { "com.sun.star.ucb.GIOContent" };
       return aSNS;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

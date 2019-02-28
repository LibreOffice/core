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

#include <uno/mapping.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>

#include <vector>

#include "FileAccess.hxx"

#define SERVICE_NAME "com.sun.star.ucb.SimpleFileAccess"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;

using ::std::vector;

namespace
{

// Implementation XSimpleFileAccess
typedef cppu::WeakImplHelper<XSimpleFileAccess3, css::lang::XServiceInfo>
    FileAccessHelper;
class OCommandEnvironment;

class OFileAccess : public FileAccessHelper
{
    Reference< XComponentContext >      m_xContext;
    rtl::Reference<OCommandEnvironment> mxEnvironment;

    /// @throws CommandAbortedException
    /// @throws Exception
    /// @throws RuntimeException
    void transferImpl( const OUString& rSource, const OUString& rDest, bool bMoveData );
    /// @throws Exception
    bool createNewFile( const OUString & rParentURL,
                        const OUString & rTitle,
                        const Reference< XInputStream >& data );

public:
    explicit OFileAccess( const Reference< XComponentContext > & xContext )
        : m_xContext( xContext) {}
    // Methods
    virtual void SAL_CALL copy( const OUString& SourceURL, const OUString& DestURL ) override;
    virtual void SAL_CALL move( const OUString& SourceURL, const OUString& DestURL ) override;
    virtual void SAL_CALL kill( const OUString& FileURL ) override;
    virtual sal_Bool SAL_CALL isFolder( const OUString& FileURL ) override;
    virtual sal_Bool SAL_CALL isReadOnly( const OUString& FileURL ) override;
    virtual void SAL_CALL setReadOnly( const OUString& FileURL, sal_Bool bReadOnly ) override;
    virtual void SAL_CALL createFolder( const OUString& NewFolderURL ) override;
    virtual sal_Int32 SAL_CALL getSize( const OUString& FileURL ) override;
    virtual OUString SAL_CALL getContentType( const OUString& FileURL ) override;
    virtual css::util::DateTime SAL_CALL getDateTimeModified( const OUString& FileURL ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getFolderContents( const OUString& FolderURL, sal_Bool bIncludeFolders ) override;
    virtual sal_Bool SAL_CALL exists( const OUString& FileURL ) override;
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL openFileRead( const OUString& FileURL ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL openFileWrite( const OUString& FileURL ) override;
    virtual css::uno::Reference< css::io::XStream > SAL_CALL openFileReadWrite( const OUString& FileURL ) override;
    virtual void SAL_CALL setInteractionHandler( const css::uno::Reference< css::task::XInteractionHandler >& Handler ) override;
    virtual void SAL_CALL writeFile( const OUString& FileURL, const css::uno::Reference< css::io::XInputStream >& data ) override;
    virtual sal_Bool SAL_CALL isHidden( const OUString& FileURL ) override;
    virtual void SAL_CALL setHidden( const OUString& FileURL, sal_Bool bHidden ) override;

    OUString SAL_CALL getImplementationName() override
    { return OUString(IMPLEMENTATION_NAME); }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return FileAccess_getSupportedServiceNames(); }
};

// Implementation XActiveDataSink

class OActiveDataSink : public cppu::WeakImplHelper< XActiveDataSink >
{
    Reference< XInputStream > mxStream;

public:

    // Methods
    virtual void SAL_CALL setInputStream( const Reference< XInputStream >& aStream ) override;
    virtual Reference< XInputStream > SAL_CALL getInputStream(  ) override;
};

// Implementation XActiveDataStreamer

class OActiveDataStreamer : public cppu::WeakImplHelper< XActiveDataStreamer >
{
    Reference< XStream > mxStream;

public:

    // Methods
    virtual void SAL_CALL setStream( const Reference< XStream >& aStream ) override;
    virtual Reference< XStream > SAL_CALL getStream() override;
};

// Implementation XCommandEnvironment

class OCommandEnvironment : public cppu::WeakImplHelper< XCommandEnvironment >
{
    Reference< XInteractionHandler > mxInteraction;

public:
    void setHandler( const Reference< XInteractionHandler >& xInteraction_ )
    {
        mxInteraction = xInteraction_;
    }

    // Methods
    virtual Reference< XInteractionHandler > SAL_CALL getInteractionHandler() override;
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler() override;
};

void OActiveDataSink::setInputStream( const Reference< XInputStream >& aStream )
{
    mxStream = aStream;
}

Reference< XInputStream > OActiveDataSink::getInputStream()
{
    return mxStream;
}

void OActiveDataStreamer::setStream( const Reference< XStream >& aStream )
{
    mxStream = aStream;
}

Reference< XStream > OActiveDataStreamer::getStream()
{
    return mxStream;
}

Reference< XInteractionHandler > OCommandEnvironment::getInteractionHandler()
{
    return mxInteraction;
}

Reference< XProgressHandler > OCommandEnvironment::getProgressHandler()
{
    Reference< XProgressHandler > xRet;
    return xRet;
}

void OFileAccess::transferImpl( const OUString& rSource,
                                const OUString& rDest,
                                bool bMoveData )
{
    // SfxContentHelper::Transfer_Impl
    INetURLObject aSourceObj( rSource, INetProtocol::File );
    INetURLObject aDestObj( rDest, INetProtocol::File );
    OUString aName = aDestObj.getName(
        INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
    OUString aDestURL;
    OUString aSourceURL = aSourceObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    if ( aDestObj.removeSegment() )
    {
        // hierarchical URL.

        aDestObj.setFinalSlash();
        aDestURL = aDestObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }
    else
    {
        // non-hierarchical URL

        // #i29648#


        if ( aDestObj.GetProtocol() == INetProtocol::VndSunStarExpand )
        {
            // Hack: Expand destination URL using Macro Expander and try again
            //       with the hopefully hierarchical expanded URL...

            try
            {
                Reference< XMacroExpander > xExpander = theMacroExpander::get(m_xContext);

                aDestURL = xExpander->expandMacros(
                    aDestObj.GetURLPath( INetURLObject::DecodeMechanism::WithCharset ) );
            }
            catch ( Exception const & )
            {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetRuntimeException(
                    "OFileAccess::transferrImpl - Unable to obtain destination folder URL!",
                    static_cast< cppu::OWeakObject * >( this ), anyEx );
            }

            transferImpl( rSource, aDestURL, bMoveData );
            return;
        }

        throw RuntimeException(
               "OFileAccess::transferrImpl - Unable to obtain destination folder URL!",
                static_cast< cppu::OWeakObject * >( this ) );

    }

    ucbhelper::Content aDestPath( aDestURL,   mxEnvironment.get(), comphelper::getProcessComponentContext() );
    ucbhelper::Content aSrc     ( aSourceURL, mxEnvironment.get(), comphelper::getProcessComponentContext() );

    try
    {
        aDestPath.transferContent(aSrc,
                                        bMoveData
                                         ? ucbhelper::InsertOperation::Move
                                         : ucbhelper::InsertOperation::Copy,
                                        aName,
                                        css::ucb::NameClash::OVERWRITE);
    }
    catch ( css::ucb::CommandFailedException const & )
    {
        // Interaction Handler already handled the error that has occurred...
    }
}

void OFileAccess::copy( const OUString& SourceURL, const OUString& DestURL )
{
    transferImpl( SourceURL, DestURL, false );
}

void OFileAccess::move( const OUString& SourceURL, const OUString& DestURL )
{
    transferImpl( SourceURL, DestURL, true );
}

void OFileAccess::kill( const OUString& FileURL )
{
    // SfxContentHelper::Kill
    INetURLObject aDeleteObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aDeleteObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
    try
    {
        aCnt.executeCommand( "delete", makeAny( true ) );
    }
    catch ( css::ucb::CommandFailedException const & )
    {
        // Interaction Handler already handled the error that has occurred...
    }
}

sal_Bool OFileAccess::isFolder( const OUString& FileURL )
{
    bool bRet = false;
    try
    {
        INetURLObject aURLObj( FileURL, INetProtocol::File );
        ucbhelper::Content aCnt( aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
        bRet = aCnt.isFolder();
    }
    catch (const Exception &) {}
    return bRet;
}

sal_Bool OFileAccess::isReadOnly( const OUString& FileURL )
{
    INetURLObject aURLObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
    Any aRetAny = aCnt.getPropertyValue("IsReadOnly");
    bool bRet = false;
    aRetAny >>= bRet;
    return bRet;
}

void OFileAccess::setReadOnly( const OUString& FileURL, sal_Bool bReadOnly )
{
    INetURLObject aURLObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
    aCnt.setPropertyValue("IsReadOnly", Any(bReadOnly) );
}

void OFileAccess::createFolder( const OUString& NewFolderURL )
{
    // Does the folder already exist?
    if( NewFolderURL.isEmpty() || isFolder( NewFolderURL ) )
        return;

    // SfxContentHelper::MakeFolder
    INetURLObject aURL( NewFolderURL, INetProtocol::File );
    OUString aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
    if ( !aTitle.isEmpty() )
    {
        aURL.removeSegment();

        // Does the base folder exist? Otherwise create it first
        OUString aBaseFolderURLStr = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        if( !isFolder( aBaseFolderURLStr ) )
        {
            createFolder( aBaseFolderURLStr );
        }
    }

    ucbhelper::Content aCnt( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );

    Sequence< ContentInfo > aInfo = aCnt.queryCreatableContentsInfo();
    sal_Int32 nCount = aInfo.getLength();
    if ( nCount == 0 )
        return;

    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        // Simply look for the first KIND_FOLDER...
        const ContentInfo & rCurr = aInfo[i];
        if ( rCurr.Attributes & ContentInfoAttribute::KIND_FOLDER )
        {
            // Make sure the only required bootstrap property is "Title",
            const Sequence< Property > & rProps = rCurr.Properties;
            if ( rProps.getLength() != 1 )
                continue;

            if ( rProps[ 0 ].Name != "Title" )
                continue;

            Sequence<OUString> aNames { "Title" };
            Sequence< Any > aValues(1);
            Any* pValues = aValues.getArray();
            pValues[0] <<= aTitle;

            ucbhelper::Content aNew;
            try
            {
                if ( !aCnt.insertNewContent( rCurr.Type, aNames, aValues, aNew ) )
                    continue;

                // Success. We're done.
                return;
            }
            catch ( css::ucb::CommandFailedException const & )
            {
                // Interaction Handler already handled the error that has occurred...
                continue;
            }
        }
    }
}

sal_Int32 OFileAccess::getSize( const OUString& FileURL )
{
    // SfxContentHelper::GetSize
    sal_Int32 nSize = 0;
    sal_Int64 nTemp = 0;
    INetURLObject aObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
    aCnt.getPropertyValue( "Size" ) >>= nTemp;
    nSize = static_cast<sal_Int32>(nTemp);
    return nSize;
}

OUString OFileAccess::getContentType( const OUString& FileURL )
{
    INetURLObject aObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );

    Reference< XContent > xContent = aCnt.get();
    OUString aTypeStr = xContent->getContentType();
    return aTypeStr;
}

css::util::DateTime OFileAccess::getDateTimeModified( const OUString& FileURL )
{
    INetURLObject aFileObj( FileURL, INetProtocol::File );
    css::util::DateTime aDateTime;

    Reference< XCommandEnvironment > aCmdEnv;
    ucbhelper::Content aYoung( aFileObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aCmdEnv, comphelper::getProcessComponentContext() );
    aYoung.getPropertyValue("DateModified") >>= aDateTime;
    return aDateTime;
}

Sequence< OUString > OFileAccess::getFolderContents( const OUString& FolderURL, sal_Bool bIncludeFolders )
{
    // SfxContentHelper::GetFolderContents

    std::vector<OUString> aFiles;
    INetURLObject aFolderObj( FolderURL, INetProtocol::File );

    ucbhelper::Content aCnt( aFolderObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(0);

    ucbhelper::ResultSetInclude eInclude = bIncludeFolders ? ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS : ucbhelper::INCLUDE_DOCUMENTS_ONLY;

    try
    {
        xResultSet = aCnt.createCursor( aProps, eInclude );
    }
    catch ( css::ucb::CommandFailedException const & )
    {
        // Interaction Handler already handled the error that has occurred...
    }

    if ( xResultSet.is() )
    {
        Reference< css::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

        while ( xResultSet->next() )
        {
            OUString aId = xContentAccess->queryContentIdentifierString();
            INetURLObject aURL( aId, INetProtocol::File );
            aFiles.push_back( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
        }
    }

    return comphelper::containerToSequence(aFiles);
}

sal_Bool OFileAccess::exists( const OUString& FileURL )
{
    bool bRet = false;
    try
    {
        bRet = isFolder( FileURL );
        if( !bRet )
        {
            Reference< XInputStream > xStream = openFileRead( FileURL );
            bRet = xStream.is();
            if( bRet )
                xStream->closeInput();
        }
    }
    catch (const Exception &) {}
    return bRet;
}

Reference< XInputStream > OFileAccess::openFileRead( const OUString& FileURL )
{
    Reference< XInputStream > xRet;
    INetURLObject aObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );

    Reference<XActiveDataSink> xSink = new OActiveDataSink;

    try
    {
        bool bRet = aCnt.openStream( xSink );
        if( bRet )
            xRet = xSink->getInputStream();
    }
    catch ( css::ucb::CommandFailedException const & )
    {
        // Interaction Handler already handled the error that has occurred...
    }

    return xRet;
}

Reference< XOutputStream > OFileAccess::openFileWrite( const OUString& FileURL )
{
    Reference< XOutputStream > xRet;
    Reference< XStream > xStream = OFileAccess::openFileReadWrite( FileURL );
    if( xStream.is() )
        xRet = xStream->getOutputStream();
    return xRet;
}

Reference< XStream > OFileAccess::openFileReadWrite( const OUString& FileURL )
{
    Reference<XActiveDataStreamer> xSink = new OActiveDataStreamer;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = xSink;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Any aCmdArg;
    aCmdArg <<= aArg;

    INetURLObject aFileObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aFileObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );

    // Be silent...
    Reference< XInteractionHandler > xIH;
    if ( mxEnvironment.is() )
    {
        xIH = mxEnvironment->getInteractionHandler();
        mxEnvironment->setHandler( nullptr );
    }

    try
    {
        aCnt.executeCommand( "open", aCmdArg );
    }
    catch ( InteractiveIOException const & e )
    {
        if ( xIH.is() && mxEnvironment.is() )
            mxEnvironment->setHandler( xIH );

        if ( e.Code == IOErrorCode_NOT_EXISTING )
        {
            // Create file...
            SvMemoryStream aStream(0,0);
            ::utl::OInputStreamWrapper* pInput = new ::utl::OInputStreamWrapper( aStream );
            Reference< XInputStream > xInput( pInput );
            InsertCommandArgument aInsertArg;
            aInsertArg.Data = xInput;
            aInsertArg.ReplaceExisting = false;

            aCmdArg <<= aInsertArg;
            aCnt.executeCommand( "insert", aCmdArg );

            // Retry...
            return openFileReadWrite( FileURL );
        }

        throw;
    }

    if ( xIH.is() && mxEnvironment.is() )
        mxEnvironment->setHandler( xIH );

    Reference< XStream > xRet = xSink->getStream();
    return xRet;
}

void OFileAccess::setInteractionHandler( const Reference< XInteractionHandler >& Handler )
{
    if( !mxEnvironment.is() )
    {
        mxEnvironment = new OCommandEnvironment;
    }
    mxEnvironment->setHandler( Handler );
}

bool OFileAccess::createNewFile( const OUString & rParentURL,
                                 const OUString & rTitle,
                                 const Reference< XInputStream >& data )
{
    ucbhelper::Content aParentCnt( rParentURL, mxEnvironment.get(), comphelper::getProcessComponentContext() );

    Sequence< ContentInfo > aInfo = aParentCnt.queryCreatableContentsInfo();
    sal_Int32 nCount = aInfo.getLength();
    if ( nCount == 0 )
        return false;

    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        const ContentInfo & rCurr = aInfo[i];
        if ( ( rCurr.Attributes
               & ContentInfoAttribute::KIND_DOCUMENT ) &&
             ( rCurr.Attributes
               & ContentInfoAttribute::INSERT_WITH_INPUTSTREAM ) )
        {
            // Make sure the only required bootstrap property is
            // "Title",
            const Sequence< Property > & rProps = rCurr.Properties;
            if ( rProps.getLength() != 1 )
                continue;

            if ( rProps[ 0 ].Name != "Title" )
                continue;

            Sequence<OUString> aNames { "Title" };
            Sequence< Any > aValues(1);
            Any* pValues = aValues.getArray();
            pValues[0] <<= rTitle;

            try
            {
                ucbhelper::Content aNew;
                if ( aParentCnt.insertNewContent(
                         rCurr.Type, aNames, aValues, data, aNew ) )
                    return true; // success.
                else
                    continue;
            }
            catch ( CommandFailedException const & )
            {
                // Interaction Handler already handled the
                // error that has occurred...
                continue;
            }
        }
    }

    return false;
}

void SAL_CALL OFileAccess::writeFile( const OUString& FileURL,
                                      const Reference< XInputStream >& data )
{
    INetURLObject aURL( FileURL, INetProtocol::File );
    try
    {
        ucbhelper::Content aCnt(
            aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(),
            comphelper::getProcessComponentContext() );

        try
        {
            aCnt.writeStream( data, true /* bReplaceExisting */ );
        }
        catch ( CommandFailedException const & )
        {
            // Interaction Handler already handled the error that has occurred...
        }
    }
    catch ( ContentCreationException const & e )
    {
        // Most probably file does not exist. Try to create.
        if ( e.eError == ContentCreationError_CONTENT_CREATION_FAILED )
        {
            INetURLObject aParentURLObj( aURL );
            if ( aParentURLObj.removeSegment() )
            {
                OUString aParentURL
                    = aParentURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                // ensure all parent folders exist.
                createFolder( aParentURL );

                // create the new file...
                OUString aTitle
                    = aURL.getName( INetURLObject::LAST_SEGMENT,
                                    true,
                                    INetURLObject::DecodeMechanism::WithCharset );
                if ( createNewFile( aParentURL, aTitle, data ) )
                {
                    // success
                    return;
                }
            }
        }

        throw;
    }
}

sal_Bool OFileAccess::isHidden( const OUString& FileURL )
{
    INetURLObject aURLObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
    Any aRetAny = aCnt.getPropertyValue("IsHidden");
    bool bRet = false;
    aRetAny >>= bRet;
    return bRet;
}

void OFileAccess::setHidden( const OUString& FileURL, sal_Bool bHidden )
{
    INetURLObject aURLObj( FileURL, INetProtocol::File );
    ucbhelper::Content aCnt( aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), mxEnvironment.get(), comphelper::getProcessComponentContext() );
    aCnt.setPropertyValue("IsHidden", Any(bHidden) );
}

}; // namespace end

Reference< XInterface > FileAccess_CreateInstance( const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference < XInterface >( static_cast<cppu::OWeakObject *>(new OFileAccess( comphelper::getComponentContext(xSMgr) )) );
}

Sequence< OUString > FileAccess_getSupportedServiceNames()
{
    Sequence< OUString > seqNames { SERVICE_NAME };
    return seqNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

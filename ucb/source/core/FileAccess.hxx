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

#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <uno/mapping.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
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

#define IMPLEMENTATION_NAME "com.sun.star.comp.ucb.SimpleFileAccess"
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
typedef cppu::WeakImplHelper1< XSimpleFileAccess3 > FileAccessHelper;
class OCommandEnvironment;

class OFileAccess : public FileAccessHelper
{
    Reference< XComponentContext > m_xContext;
    Reference< XCommandEnvironment > mxEnvironment;
    OCommandEnvironment* mpEnvironment;

    void transferImpl( const OUString& rSource, const OUString& rDest, sal_Bool bMoveData )
        throw(CommandAbortedException, Exception, RuntimeException);
    bool createNewFile( const OUString & rParentURL,
                        const OUString & rTitle,
                        const Reference< XInputStream >& data )
        throw ( Exception );

public:
    OFileAccess( const Reference< XComponentContext > & xContext )
        : m_xContext( xContext), mpEnvironment( NULL ) {}

    // Methods
    virtual void SAL_CALL copy( const OUString& SourceURL, const OUString& DestURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL move( const OUString& SourceURL, const OUString& DestURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL kill( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isFolder( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setReadOnly( const OUString& FileURL, sal_Bool bReadOnly ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createFolder( const OUString& NewFolderURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSize( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getContentType( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::DateTime SAL_CALL getDateTimeModified( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFolderContents( const OUString& FolderURL, sal_Bool bIncludeFolders ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL exists( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL openFileRead( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL openFileWrite( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openFileReadWrite( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL writeFile( const OUString& FileURL, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& data ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isHidden( const OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHidden( const OUString& FileURL, sal_Bool bHidden ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
};

// Implementation XActiveDataSink
typedef cppu::WeakImplHelper1< XActiveDataSink > ActiveDataSinkHelper;

class OActiveDataSink : public ActiveDataSinkHelper
{
    Reference< XInputStream > mxStream;

public:

    // Methods
    virtual void SAL_CALL setInputStream( const Reference< XInputStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XInputStream > SAL_CALL getInputStream(  )
        throw(RuntimeException);
};

// Implementation XActiveDataStreamer
typedef cppu::WeakImplHelper1< XActiveDataStreamer > ActiveDataStreamerHelper;

class OActiveDataStreamer : public ActiveDataStreamerHelper
{
    Reference< XStream > mxStream;

public:

    // Methods
    virtual void SAL_CALL setStream( const Reference< XStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XStream > SAL_CALL getStream()
        throw(RuntimeException);
};

// Implementation XCommandEnvironment
typedef cppu::WeakImplHelper1< XCommandEnvironment > CommandEnvironmentHelper;

class OCommandEnvironment : public CommandEnvironmentHelper
{
    Reference< XInteractionHandler > mxInteraction;

public:
    void setHandler( Reference< XInteractionHandler > xInteraction_ )
    {
        mxInteraction = xInteraction_;
    }

    // Methods
    virtual Reference< XInteractionHandler > SAL_CALL getInteractionHandler()
        throw(RuntimeException);
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler()
        throw(RuntimeException);
};

};

Reference< XInterface > SAL_CALL FileAccess_CreateInstance( const Reference< XMultiServiceFactory > & xSMgr );
Sequence< OUString > FileAccess_getSupportedServiceNames();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

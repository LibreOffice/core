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

#pragma once

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include <gio/gio.h>

#include <vector>

namespace com::sun::star {
    namespace beans {
        struct Property;
        struct PropertyValue;
    }
    namespace sdbc {
        class XRow;
    }
}
namespace ucbhelper
{
    class Content;
}


namespace gio
{


#define GIO_FILE_TYPE   "application/vnd.sun.staroffice.gio-file"
#define GIO_FOLDER_TYPE "application/vnd.sun.staroffice.gio-folder"

css::uno::Any convertToException(GError *pError,
    const css::uno::Reference< css::uno::XInterface >& rContext, bool bThrow=true);
/// @throws css::io::IOException
/// @throws css::uno::RuntimeException
void convertToIOException(GError *pError,
    const css::uno::Reference< css::uno::XInterface >& rContext);

class ContentProvider;
class Content : public ::ucbhelper::ContentImplHelper, public css::ucb::XContentCreator
{
private:
    ContentProvider  *m_pProvider;
    GFile* mpFile;
    GFileInfo *mpInfo;
    bool mbTransient;

    GFileInfo *getGFileInfo(const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
        GError **ppError=nullptr);
    bool isFolder(const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv);

    css::uno::Any mapGIOError( GError *error );
    css::uno::Any getBadArgExcept();

    css::uno::Reference< css::sdbc::XRow >
        getPropertyValues(
            const css::uno::Sequence< css::beans::Property >& rProperties,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );
private:
    typedef rtl::Reference< Content > ContentRef;
    typedef std::vector< ContentRef > ContentRefList;

    void queryChildren( ContentRefList& rChildren );

    bool doSetFileInfo ( GFileInfo *pNewInfo );

    /// @throws css::uno::Exception
    css::uno::Any open(const css::ucb::OpenCommandArgument2 & rArg,
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void transfer( const css::ucb::TransferInfo& rTransferInfo,
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv );

    /// @throws css::uno::Exception
    void insert( const css::uno::Reference< css::io::XInputStream > & xInputStream,
        bool bReplaceExisting, const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    void destroy( bool bDeletePhysical );

    static void copyData( const css::uno::Reference< css::io::XInputStream >& xIn,
        const css::uno::Reference< css::io::XOutputStream >& xOut );

    css::uno::Sequence< css::uno::Any >
        setPropertyValues( const css::uno::Sequence<
            css::beans::PropertyValue >& rValues,
            const css::uno::Reference<
            css::ucb::XCommandEnvironment >& xEnv );

    bool feedSink( const css::uno::Reference< css::uno::XInterface>& aSink );

    bool exchangeIdentity(const css::uno::Reference< css::ucb::XContentIdentifier >&  xNewId);

    void getFileInfo(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & env, GFileInfo ** info,
        bool fail);

public:
    /// @throws css::ucb::ContentCreationException
    Content( const css::uno::Reference<
        css::uno::XComponentContext >& rxContext, ContentProvider *pProvider,
        const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier);

    /// @throws css::ucb::ContentCreationException
    Content( const css::uno::Reference<
        css::uno::XComponentContext >& rxContext, ContentProvider *pProvider,
        const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
        bool bIsFolder);

    virtual ~Content() override;

    virtual css::uno::Sequence< css::beans::Property >
        getProperties( const css::uno::Reference<
            css::ucb::XCommandEnvironment > & xEnv ) override;

    virtual css::uno::Sequence< css::ucb::CommandInfo >
        getCommands( const css::uno::Reference<
            css::ucb::XCommandEnvironment > & xEnv ) override;

    virtual OUString getParentURL() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    virtual OUString SAL_CALL
    getImplementationName() override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual OUString SAL_CALL
    getContentType() override;

    virtual css::uno::Any SAL_CALL
        execute( const css::ucb::Command& aCommand,
        sal_Int32 CommandId,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override;

    virtual void SAL_CALL abort( sal_Int32 CommandId ) override;

    virtual css::uno::Sequence< css::ucb::ContentInfo >
        SAL_CALL queryCreatableContentsInfo() override;
    virtual css::uno::Reference< css::ucb::XContent >
        SAL_CALL createNewContent( const css::ucb::ContentInfo& Info ) override;

    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::ucb::ContentInfo >
        queryCreatableContentsInfo(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv);

    GFile* getGFile();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

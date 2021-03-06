/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "cmis_url.hxx"
#include "children_provider.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include <libcmis/libcmis.hxx>

#include <vector>
#include <list>

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


namespace cmis
{
#define CMIS_REPO_TYPE   "application/vnd.libreoffice.cmis-repository"

class ContentProvider;
class RepoContent : public ::ucbhelper::ContentImplHelper,
                    public ChildrenProvider
{
private:
    ContentProvider*       m_pProvider;
    URL                    m_aURL;
    OUString          m_sRepositoryId;

    std::vector< libcmis::RepositoryPtr > m_aRepositories;

private:

    css::uno::Any getBadArgExcept();

    css::uno::Reference< css::sdbc::XRow >
        getPropertyValues(
            const css::uno::Sequence< css::beans::Property >& rProperties,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /*
     * Call me to ensure the repositories have been fetched
     */
    void getRepositories( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    libcmis::RepositoryPtr getRepository( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

public:
    /// @throws css::ucb::ContentCreationException
    RepoContent( const css::uno::Reference<
        css::uno::XComponentContext >& rxContext, ContentProvider *pProvider,
        const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
        std::vector< libcmis::RepositoryPtr > const & aRepos = std::vector< libcmis::RepositoryPtr > ( ) );

    virtual ~RepoContent() override;

    virtual css::uno::Sequence< css::beans::Property >
        getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;

    virtual css::uno::Sequence< css::ucb::CommandInfo >
        getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;

    virtual OUString getParentURL() override;

    // XInterface

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

    virtual std::vector< css::uno::Reference< css::ucb::XContent > > getChildren( ) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

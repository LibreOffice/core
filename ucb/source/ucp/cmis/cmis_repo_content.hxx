/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CMIS_REPO_CONTENT_HXX
#define CMIS_REPO_CONTENT_HXX

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

namespace com { namespace sun { namespace star {
    namespace beans {
        struct Property;
        struct PropertyValue;
    }
    namespace sdbc {
        class XRow;
    }
}}}
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

    com::sun::star::uno::Any getBadArgExcept();

    com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
        getPropertyValues(
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& rProperties,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );

    /*
     * Call me to ensure the repositories have been fetched
     */
    void getRepositories( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& xEnv );

    libcmis::RepositoryPtr getRepository( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& xEnv );

public:
    RepoContent( const com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >& rxContext, ContentProvider *pProvider,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier,
        std::vector< libcmis::RepositoryPtr > aRepos = std::vector< libcmis::RepositoryPtr > ( ) )
            throw ( com::sun::star::ucb::ContentCreationException );

    virtual ~RepoContent();

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
        getProperties( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv );

    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
        getCommands( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv );

    virtual OUString getParentURL();

    XINTERFACE_DECL()

    XTYPEPROVIDER_DECL()

    virtual OUString SAL_CALL
    getImplementationName()
            throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );

    virtual OUString SAL_CALL
    getContentType()
            throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL
        execute( const com::sun::star::ucb::Command& aCommand,
        sal_Int32 CommandId,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& Environment )
            throw( com::sun::star::uno::Exception, com::sun::star::ucb::CommandAbortedException, com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL abort( sal_Int32 CommandId )
            throw( com::sun::star::uno::RuntimeException );

    virtual std::list< com::sun::star::uno::Reference< com::sun::star::ucb::XContent > > getChildren( );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

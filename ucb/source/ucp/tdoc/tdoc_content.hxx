/*************************************************************************
 *
 *  $RCSfile: tdoc_content.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:51:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

#ifndef INCLUDED_TDOC_CONTENT_HXX
#define INCLUDED_TDOC_CONTENT_HXX

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

#ifndef _COM_SUN_STAR_TASK_DOCUMENTPASSWORDREQUEST_HPP_
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDFAILEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandFailedException.hpp>
#endif

#ifndef INCLUDED_TDOC_PROVIDER_HXX
#include "tdoc_provider.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace sdbc  { class XRow; }
    namespace io    { class XInputStream; class XOutputStream; }
    namespace beans { struct PropertyValue; }
    namespace ucb   { struct OpenCommandArgument2; struct TransferInfo; }
} } }

namespace tdoc_ucp
{

//=========================================================================

#define TDOC_ROOT_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsRootContent"
#define TDOC_DOCUMENT_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsDocumentContent"
#define TDOC_FOLDER_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsFolderContent"
#define TDOC_STREAM_CONTENT_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsStreamContent"

//=========================================================================

enum ContentType { STREAM, FOLDER, DOCUMENT, ROOT };

class ContentProperties
{
public:
    ContentProperties()
    {}

    ContentProperties( const ContentType & rType, const rtl::OUString & rTitle )
    : m_eType( rType ),
      m_aContentType( rType == STREAM
        ? rtl::OUString::createFromAscii( TDOC_STREAM_CONTENT_TYPE )
        : rType == FOLDER
            ? rtl::OUString::createFromAscii( TDOC_FOLDER_CONTENT_TYPE )
            : rType == DOCUMENT
                ? rtl::OUString::createFromAscii( TDOC_DOCUMENT_CONTENT_TYPE )
                : rtl::OUString::createFromAscii( TDOC_ROOT_CONTENT_TYPE ) ),
      m_aTitle( rTitle )
    {}

    ContentType getType() const { return m_eType; }

    // Properties

    const rtl::OUString & getContentType() const { return m_aContentType; }

    bool getIsFolder()   const { return m_eType > STREAM; }
    bool getIsDocument() const { return !getIsFolder(); }

    const rtl::OUString & getTitle() const { return m_aTitle; }
    void setTitle( const rtl::OUString & rTitle ) { m_aTitle = rTitle; }

private:
    ContentType   m_eType;
    rtl::OUString m_aContentType;
    rtl::OUString m_aTitle;
};

//=========================================================================

class Content : public ::ucb::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator
{
    enum ContentState { TRANSIENT,  // created via createNewContent,
                                       // but did not process "insert" yet
                        PERSISTENT, // processed "insert"
                        DEAD        // processed "delete" / document was closed
                      };

    ContentProperties m_aProps;
    ContentState      m_eState;
    ContentProvider*  m_pProvider;

private:
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const ContentProperties & rProps );
    Content( const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ContentProvider* pProvider,
             const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
             const com::sun::star::ucb::ContentInfo& Info );

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual ::rtl::OUString getParentURL();

    bool isContentCreator();

    static bool hasData( ContentProvider* pProvider, const Uri & rUri );
    bool hasData( const Uri & rUri ) { return hasData( m_pProvider, rUri ); }

    static bool loadData( ContentProvider* pProvider,
                          const Uri & rUri,
                          ContentProperties& rProps );
    bool storeData( const com::sun::star::uno::Reference<
                        com::sun::star::io::XInputStream >& xData,
                    const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );
    bool renameData( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& xOldId,
                     const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& xNewId );
    bool removeData();

    bool copyData( const Uri & rSourceUri, const rtl::OUString & rNewName );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentIdentifier >
    makeNewIdentifier( const rtl::OUString& rTitle );

    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren );

    sal_Bool exchangeIdentity(
                const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                             ::com::sun::star::beans::Property >& rProperties );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues(
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >& rValues,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    com::sun::star::uno::Any
    open( const ::com::sun::star::ucb::OpenCommandArgument2& rArg,
          const ::com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( ::com::sun::star::uno::Exception );

    void insert( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XInputStream >& xData,
                 sal_Int32 nNameClashResolve,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    void destroy( sal_Bool bDeletePhysical,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    void transfer( const ::com::sun::star::ucb::TransferInfo& rInfo,
                   const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties,
                       const ContentProperties& rData,
                       ContentProvider* pProvider,
                       const ::rtl::OUString& rContentId );


    static bool commitStorage(
        const com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage > & xStorage );

    static bool closeOutputStream(
        const com::sun::star::uno::Reference<
            com::sun::star::io::XOutputStream > & xOut );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
    getInputStream( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment > &
                            xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
    getTruncatedOutputStream(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );

    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >
    queryChildContent( const rtl::OUString & rRelativeChildUri );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >
    getStream( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment > & xEnv )
        throw ( ::com::sun::star::ucb::CommandFailedException,
                ::com::sun::star::task::DocumentPasswordRequest );

public:
    // Create existing content. Fail, if not already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static Content* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual ~Content();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );

    // XContent
    virtual rtl::OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    getIdentifier()
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    // XContentCreator
    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::ContentInfo > SAL_CALL
    queryCreatableContentsInfo()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    createNewContent( const com::sun::star::ucb::ContentInfo& Info )
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties,
                       ContentProvider* pProvider,
                       const ::rtl::OUString& rContentId );

    void notifyDocumentClosed();
    void notifyChildRemoved( const rtl::OUString & rRelativeChildUri );
    void notifyChildInserted( const rtl::OUString & rRelativeChildUri );

    rtl::Reference< ContentProvider > getContentProvider() const
    { return rtl::Reference< ContentProvider >( m_pProvider ); }
};

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_CONTENT_HXX */

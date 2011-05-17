/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ODMA_CONTENT_HXX
#define ODMA_CONTENT_HXX

#include <list>
#include <ucbhelper/contenthelper.hxx>

namespace com { namespace sun { namespace star {
    namespace beans {
        struct Property;
        struct PropertyValue;
    }
    namespace sdbc {
        class XRow;
    }
    namespace io {
        class XInputStream;
    }
}}}
namespace ucbhelper
{
    class Content;
}


// @@@ Adjust namespace name.
namespace odma
{

//=========================================================================

// @@@ Adjust service name.

// UNO service name for the content.
#define ODMA_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.ODMAContent"

//=========================================================================
class ContentProvider;
class ContentProperties;
class Content : public ::ucbhelper::ContentImplHelper
{
    ::rtl::Reference<ContentProperties> m_aProps;
    ContentProvider*                    m_pProvider;
    ::ucbhelper::Content*               m_pContent;

private:
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
    getProperties( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
    getCommands( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv );
    virtual ::rtl::OUString getParentURL();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::Property >& rProperties,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::PropertyValue >& rValues,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XCommandEnvironment >& xEnv );

    /** openDoc returns the file URL for the DOC ID
        @return the url of the temporary file
    */
    ::rtl::OUString openDoc();

    /** changePropertyValue sets the property referenced by _sMemberValue to the new value
        @param  _rValue         the new value to set
        @param  _rnCurrentPos   the current position inside the Any sequence _rRet
        @param  _sMemberValue   the place where to set the property
        @param  _rnChanged      will be incremented when property changed
        @param  _rRet           collect the exceptions
        @param  _rChanges       contains the changes done
    */
    void changePropertyValue(const ::com::sun::star::beans::PropertyValue& _rValue,
                             sal_Int32 _rnCurrentPos,
                             ::rtl::OUString& _rsMemberValue,
                             sal_Int32& _rnChanged,
                             ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rRet,
                             ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& _rChanges) throw (::com::sun::star::beans::IllegalTypeException);

//    typedef rtl::Reference< Content > ContentRef;
//    typedef std::list< ContentRef > ContentRefList;
//    void queryChildren( ContentRefList& rChildren );

  // Command "insert"
    void insert( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XInputStream > & xInputStream,
                 sal_Bool bReplaceExisting,
                 const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( ::com::sun::star::uno::Exception );

//  // Command "delete"
//  void destroy( sal_Bool bDeletePhysical )
//      throw( ::com::sun::star::uno::Exception );

public:
    Content( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                ContentProvider* pProvider,
             const ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContentIdentifier >& Identifier,
                const ::rtl::Reference<ContentProperties>& _rProps);
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

    // @@@ Add additional interfaces ( like com::sun:.star::ucb::XContentCreator ).

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    // Called from resultset data supplier.
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       const ::rtl::Reference<ContentProperties>& rData,
                       const ::rtl::Reference<
                           ::ucbhelper::ContentProviderImplHelper >&    rProvider,
                       const ::rtl::OUString& rContentId );

    ContentProvider* getContentProvider() const { return m_pProvider; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

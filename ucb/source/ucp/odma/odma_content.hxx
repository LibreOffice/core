/*************************************************************************
 *
 *  $RCSfile: odma_content.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:00:21 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// @@@ Adjust multi-include-protection-ifdef.
#ifndef ODMA_CONTENT_HXX
#define ODMA_CONTENT_HXX

#include <list>

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

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
namespace ucb
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
                            "com.sun.star.ucb.OdmaContent"

//=========================================================================
class ContentProvider;
class ContentProperties;
class Content : public ::ucb::ContentImplHelper
{
    ::vos::ORef<ContentProperties>  m_aProps;
    ContentProvider*                m_pProvider;
    ::ucb::Content*                 m_pContent;

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
                const ::vos::ORef<ContentProperties>& _rProps);
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
                       const ::vos::ORef<ContentProperties>& rData,
                       const ::vos::ORef<
                           ::ucb::ContentProviderImplHelper >&  rProvider,
                       const ::rtl::OUString& rContentId );

    ContentProvider* getContentProvider() const { return m_pProvider; }
};

}

#endif

/*************************************************************************
 *
 *  $RCSfile: myucp_content.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-27 14:04:33 $
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
#ifndef _MYUCP_CONTENT_HXX
#define _MYUCP_CONTENT_HXX

#ifndef __LIST__
#include <list>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

// @@@ Adjust namespace name.
namespace myucp
{

//=========================================================================

// @@@ Adjust service name.

// UNO service name for the content.
#define MYUCP_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.MyContent"

//=========================================================================

struct ContentProperties
{
    ::rtl::OUString aTitle;         // Title
    ::rtl::OUString aContentType;   // ContentType
    sal_Bool        bIsDocument;    // IsDocument
    sal_Bool        bIsFolder;      // IsFolder

    // @@@ Add other properties supported by your content.

    ContentProperties()
    : bIsDocument( sal_True ), bIsFolder( sal_False ) {}
};

//=========================================================================

class Content : public ::ucb::ContentImplHelper
{
    ContentProperties m_aProps;

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
                             ::com::sun::star::beans::Property >& rProperties );
    void setPropertyValues(
            const ::com::sun::star::uno::Sequence<
                     ::com::sun::star::beans::PropertyValue >& rValues );

//  typedef vos::ORef< Content > ContentRef;
//  typedef std::list< ContentRef > ContentRefList;
//  void queryChildren( ContentRefList& rChildren );

//  // Command "insert"
//  void insert()
//      throw( ::com::sun::star::ucb::CommandAbortedException );

//  // Command "delete"
//  void destroy( sal_Bool bDeletePhysical )
//      throw( ::com::sun::star::ucb::CommandAbortedException );

public:
    Content( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
             ::ucb::ContentProviderImplHelper* pProvider,
             const ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContentIdentifier >& Identifier );
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
                       const ContentProperties& rData,
                       const ::vos::ORef< ::ucb::ContentProviderImplHelper >&
                               rProvider,
                       const ::rtl::OUString& rContentId );
};

}

#endif

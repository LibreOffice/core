/*************************************************************************
 *
 *  $RCSfile: contentidentifier.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2001-03-20 13:18:22 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;

namespace ucb
{

//=========================================================================
//=========================================================================
//
// struct ContentIdentifier_Impl.
//
//=========================================================================
//=========================================================================

struct ContentIdentifier_Impl
{
    Reference< XMultiServiceFactory > m_xSMgr;
    OUString                          m_aContentId;
    OUString                          m_aProviderScheme;
    osl::Mutex                        m_aMutex;

    ContentIdentifier_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                              const OUString& rURL );
};

//=========================================================================
//
// ContentIdentifier_Impl Implementation.
//
//=========================================================================

ContentIdentifier_Impl::ContentIdentifier_Impl(
                  const Reference< XMultiServiceFactory >& rSMgr,
                  const OUString& rURL )
: m_xSMgr( rSMgr )
{
    // Normalize URL scheme ( it's case insensitive ).

    // The content provider scheme is the part before the first ':'
    // within the content id.
    sal_Int32 nPos = rURL.indexOf( ':', 0 );
    if ( nPos != -1 )
    {
        OUString aScheme( rURL.copy( 0, nPos ) );
        m_aProviderScheme = aScheme.toLowerCase();
        m_aContentId = rURL.replaceAt( 0, nPos, aScheme );
    }
}

//=========================================================================
//
// ContentIdentifier Implementation.
//
//=========================================================================

ContentIdentifier::ContentIdentifier(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const OUString& rURL )
{
    m_pImpl = new ContentIdentifier_Impl( rxSMgr, rURL );
}

//=========================================================================
ContentIdentifier::ContentIdentifier( const OUString& rURL )
{
    m_pImpl = new ContentIdentifier_Impl(
                    Reference< XMultiServiceFactory >(), rURL );
}

//=========================================================================
// virtual
ContentIdentifier::~ContentIdentifier()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

//=========================================================================
// virtual
void SAL_CALL ContentIdentifier::acquire()
    throw( RuntimeException )
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL ContentIdentifier::release()
    throw( RuntimeException )
{
    OWeakObject::release();
}

//=========================================================================
// virtual
Any SAL_CALL
ContentIdentifier::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                static_cast< XTypeProvider * >( this ),
                static_cast< XContentIdentifier * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

// virtual
Sequence< sal_Int8 > SAL_CALL
ContentIdentifier::getImplementationId()
    throw( RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( sal_False );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}

//=========================================================================
// virtual
Sequence< com::sun::star::uno::Type > SAL_CALL
ContentIdentifier::getTypes()
    throw( RuntimeException )
{
    static cppu::OTypeCollection* pCollection = NULL;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                    getCppuType( static_cast<
                        Reference < XTypeProvider > * >( 0 ) ),
                    getCppuType( static_cast<
                        Reference< XContentIdentifier > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}

//=========================================================================
//
// XContentIdentifier methods.
//
//=========================================================================

// virtual
OUString SAL_CALL ContentIdentifier::getContentIdentifier()
    throw( RuntimeException )
{
    return m_pImpl->m_aContentId;
}

//=========================================================================
// virtual
OUString SAL_CALL ContentIdentifier::getContentProviderScheme()
    throw( RuntimeException )
{
    return m_pImpl->m_aProviderScheme;
}

} /* namespace ucb */


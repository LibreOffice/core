/*************************************************************************
 *
 *  $RCSfile: propertysetcontainer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-12-04 07:45:19 $
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

#ifndef __FRAMEWORK_HELPER_PROPERTYSETCONTAINER_HXX_
#include <helper/propertysetcontainer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#include <vcl/svapp.hxx>

#define WRONG_TYPE_EXCEPTION    "Only XPropertSet allowed!"

using namespace rtl;
using namespace vos;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace framework
{

PropertySetContainer::PropertySetContainer( const Reference< XMultiServiceFactory >& rServiceManager )
        :   ThreadHelpBase( &Application::GetSolarMutex() )
        ,   OWeakObject()

{
}


PropertySetContainer::~PropertySetContainer()
{
}


// XInterface
void SAL_CALL PropertySetContainer::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL PropertySetContainer::release() throw ()
{
    OWeakObject::release();
}

Any SAL_CALL PropertySetContainer::queryInterface( const Type& rType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( XIndexContainer*, this ),
                SAL_STATIC_CAST( XIndexReplace*, this ),
                SAL_STATIC_CAST( XIndexAccess*, this ),
                SAL_STATIC_CAST( XElementAccess*, this ) );

    if( a.hasValue() )
    {
        return a;
    }

    return OWeakObject::queryInterface( rType );
}

// XIndexContainer
void SAL_CALL PropertySetContainer::insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
    throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    sal_Int32 nSize = m_aPropertySetVector.size();

    if ( nSize >= Index )
    {
        Reference< XPropertySet > aPropertySetElement;

        if ( Element >>= aPropertySetElement )
        {
            if ( nSize == Index )
                m_aPropertySetVector.push_back( aPropertySetElement );
            else
            {
                PropertySetVector::iterator aIter = m_aPropertySetVector.begin();
                aIter += Index;
                m_aPropertySetVector.insert( aIter, aPropertySetElement );
            }
        }
        else
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( WRONG_TYPE_EXCEPTION )),
                (OWeakObject *)this, 2 );
        }
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

void SAL_CALL PropertySetContainer::removeByIndex( sal_Int32 Index )
    throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( (sal_Int32)m_aPropertySetVector.size() > Index )
    {
        PropertySetVector::iterator aIter = m_aPropertySetVector.begin();
        aIter += Index;
        m_aPropertySetVector.erase( aIter );
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XIndexReplace
void SAL_CALL PropertySetContainer::replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
    throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if ( (sal_Int32)m_aPropertySetVector.size() > Index )
    {
        Reference< XPropertySet > aPropertySetElement;

        if ( Element >>= aPropertySetElement )
        {
            m_aPropertySetVector[ Index ] = aPropertySetElement;
        }
        else
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( WRONG_TYPE_EXCEPTION )),
                (OWeakObject *)this, 2 );
        }
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XIndexAccess
sal_Int32 SAL_CALL PropertySetContainer::getCount()
    throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    return m_aPropertySetVector.size();
}

Any SAL_CALL PropertySetContainer::getByIndex( sal_Int32 Index )
    throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( (sal_Int32)m_aPropertySetVector.size() > Index )
    {
        Any a;

        a <<= m_aPropertySetVector[ Index ];
        return a;
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XElementAccess
sal_Bool SAL_CALL PropertySetContainer::hasElements()
    throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    return !( m_aPropertySetVector.empty() );
}

}

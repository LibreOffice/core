/*************************************************************************
 *
 *  $RCSfile: rootitemcontainer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 17:01:33 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_ROOTITEMCONTAINER_HXX_
#include <uielement/rootitemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_ITEMCONTAINER_HHX_
#include <uielement/itemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_CONSTITEMCONTAINER_HHX_
#include <uielement/constitemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

const char WRONG_TYPE_EXCEPTION[] = "Type must be com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >";

const int PROPHANDLE_UINAME     = 1;
const int PROPCOUNT             = 1;
const rtl::OUString PROPNAME_UINAME( RTL_CONSTASCII_USTRINGPARAM( "UIName" ));

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_10     (  RootItemContainer                                                   ,
                            OWeakObject                                                         ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider             ),
                            DIRECT_INTERFACE( ::com::sun::star::container::XIndexContainer      ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XUnoTunnel                ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XSingleComponentFactory   ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XMultiPropertySet        ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet         ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet             ),
                            DERIVED_INTERFACE( ::com::sun::star::container::XIndexReplace, com::sun::star::container::XIndexContainer ),
                            DERIVED_INTERFACE( ::com::sun::star::container::XIndexAccess, com::sun::star::container::XIndexReplace    ),
                            DERIVED_INTERFACE( ::com::sun::star::container::XElementAccess, ::com::sun::star::container::XIndexAccess )
                        )

DEFINE_XTYPEPROVIDER_10 (   RootItemContainer                               ,
                            ::com::sun::star::lang::XTypeProvider           ,
                            ::com::sun::star::container::XIndexContainer    ,
                            ::com::sun::star::container::XIndexReplace      ,
                            ::com::sun::star::container::XIndexAccess       ,
                            ::com::sun::star::container::XElementAccess     ,
                            ::com::sun::star::beans::XMultiPropertySet      ,
                            ::com::sun::star::beans::XFastPropertySet       ,
                            ::com::sun::star::beans::XPropertySet           ,
                            ::com::sun::star::lang::XUnoTunnel              ,
                            ::com::sun::star::lang::XSingleComponentFactory
                        )

RootItemContainer::RootItemContainer()
    :   ThreadHelpBase              ( )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   ::cppu::OWeakObject()
{
}

RootItemContainer::RootItemContainer( const ConstItemContainer& rConstItemContainer )
    :   ThreadHelpBase              ( )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   ::cppu::OWeakObject()
{
    m_aUIName = rConstItemContainer.m_aUIName;
    copyItemContainer( rConstItemContainer.m_aItemVector );
}

RootItemContainer::RootItemContainer( const Reference< XIndexAccess >& rSourceContainer )
    :   ThreadHelpBase              ( )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   ::cppu::OWeakObject()
{
    // We also have to copy the UIName property
    try
    {
        Reference< XPropertySet > xPropSet( rSourceContainer, UNO_QUERY );
        if ( xPropSet.is() )
        {
            rtl::OUString aUIName;
            xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ))) >>= m_aUIName;
        }
    }
    catch ( Exception& )
    {
    }

    sal_Int32 nCount = rSourceContainer->getCount();
    try
    {
        for ( sal_Int32 i = 0; i < nCount; i++ )
        {
            Sequence< PropertyValue > aPropSeq;
            Any a = rSourceContainer->getByIndex( i );
            if ( a >>= aPropSeq )
            {
                sal_Int32 nContainerIndex = -1;
                Reference< XIndexAccess > xIndexAccess;
                for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
                {
                    if ( aPropSeq[j].Name.equalsAscii( "ItemDescriptorContainer" ))
                    {
                        aPropSeq[j].Value >>= xIndexAccess;
                        nContainerIndex = j;
                        break;
                    }
                }

                if ( xIndexAccess.is() && nContainerIndex >= 0 )
                    aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess );

                m_aItemVector.push_back( aPropSeq );
            }
        }
    }
    catch ( IndexOutOfBoundsException& )
    {
    }
}

RootItemContainer::~RootItemContainer()
{
}

// private
void RootItemContainer::copyItemContainer( const std::vector< Sequence< PropertyValue > >& rSourceVector )
{
    for ( sal_uInt32 i = 0; i < rSourceVector.size(); i++ )
    {
        sal_Int32 nContainerIndex = -1;
        Sequence< PropertyValue > aPropSeq( rSourceVector[i] );
        Reference< XIndexAccess > xIndexAccess;
        for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
        {
            if ( aPropSeq[j].Name.equalsAscii( "ItemDescriptorContainer" ))
            {
                aPropSeq[j].Value >>= xIndexAccess;
                nContainerIndex = j;
                break;
            }
        }

        if ( xIndexAccess.is() && nContainerIndex >= 0 )
            aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess );

        m_aItemVector.push_back( aPropSeq );
    }
}

Reference< XIndexAccess > RootItemContainer::deepCopyContainer( const Reference< XIndexAccess >& rSubContainer )
{
    Reference< XIndexAccess > xReturn;
    if ( rSubContainer.is() )
    {
        ConstItemContainer* pSource = ConstItemContainer::GetImplementation( rSubContainer );
        ItemContainer* pSubContainer( 0 );
        if ( pSource )
            pSubContainer = new ItemContainer( *pSource, m_aShareMutex );
        else
            pSubContainer = new ItemContainer( rSubContainer, m_aShareMutex );
        xReturn = Reference< XIndexAccess >( static_cast< OWeakObject* >( pSubContainer ), UNO_QUERY );
    }

    return xReturn;
}

// XUnoTunnel
sal_Int64 RootItemContainer::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == rtl_compareMemory( RootItemContainer::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

const Sequence< sal_Int8 >& RootItemContainer::GetUnoTunnelId() throw()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = NULL;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

RootItemContainer* RootItemContainer::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY );
    return xUT.is() ? (RootItemContainer*)xUT->getSomething( RootItemContainer::GetUnoTunnelId() ) : NULL;
}

// XElementAccess
sal_Bool SAL_CALL RootItemContainer::hasElements()
throw ( RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    return ( m_aItemVector.size() != 0 );
}

// XIndexAccess
sal_Int32 SAL_CALL RootItemContainer::getCount()
throw ( RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    return m_aItemVector.size();
}

Any SAL_CALL RootItemContainer::getByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    if ( sal_Int32( m_aItemVector.size()) > Index )
        return makeAny( m_aItemVector[Index] );
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XIndexContainer
void SAL_CALL RootItemContainer::insertByIndex( sal_Int32 Index, const Any& aItem )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    Sequence< PropertyValue > aSeq;
    if ( aItem >>= aSeq )
    {
        ShareGuard aLock( m_aShareMutex );
        if ( sal_Int32( m_aItemVector.size()) == Index )
            m_aItemVector.push_back( aSeq );
        else if ( sal_Int32( m_aItemVector.size()) >Index )
        {
            std::vector< Sequence< PropertyValue > >::iterator aIter = m_aItemVector.begin();
            aIter += Index;
            m_aItemVector.insert( aIter, aSeq );
        }
        else
            throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
    }
    else
        throw IllegalArgumentException( OUString( RTL_CONSTASCII_USTRINGPARAM( WRONG_TYPE_EXCEPTION )),
                                        (OWeakObject *)this, 2 );
}

void SAL_CALL RootItemContainer::removeByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    if ( (sal_Int32)m_aItemVector.size() > Index )
    {
        std::vector< Sequence< PropertyValue > >::iterator aIter = m_aItemVector.begin();
        aIter += Index;
        m_aItemVector.erase( aIter );
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

void SAL_CALL RootItemContainer::replaceByIndex( sal_Int32 Index, const Any& aItem )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    Sequence< PropertyValue > aSeq;
    if ( aItem >>= aSeq )
    {
        ShareGuard aLock( m_aShareMutex );
        if ( sal_Int32( m_aItemVector.size()) > Index )
            m_aItemVector[Index] = aSeq;
        else
            throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
    }
    else
        throw IllegalArgumentException( OUString( RTL_CONSTASCII_USTRINGPARAM( WRONG_TYPE_EXCEPTION )),
                                        (OWeakObject *)this, 2 );
}

Reference< XInterface > SAL_CALL RootItemContainer::createInstanceWithContext( const Reference< XComponentContext >& Context )
throw ( Exception, RuntimeException)
{
    return (OWeakObject *)(new ItemContainer( m_aShareMutex ));
}

Reference< XInterface > SAL_CALL RootItemContainer::createInstanceWithArgumentsAndContext( const Sequence< Any >& Arguments, const Reference< XComponentContext >& Context )
throw (Exception, RuntimeException)
{
    return (OWeakObject *)(new ItemContainer( m_aShareMutex ));
}

// XPropertySet helper
sal_Bool SAL_CALL RootItemContainer::convertFastPropertyValue( Any&       aConvertedValue ,
                                                               Any&       aOldValue       ,
                                                               sal_Int32  nHandle         ,
                                                               const Any& aValue             )
throw( com::sun::star::lang::IllegalArgumentException )
{
    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case PROPHANDLE_UINAME:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_aUIName),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;
    }

    // Return state of operation.
    return bReturn ;
}

void SAL_CALL RootItemContainer::setFastPropertyValue_NoBroadcast( sal_Int32               nHandle ,
                                                                   const com::sun::star::uno::Any&    aValue  )
throw( com::sun::star::uno::Exception )
{
    switch( nHandle )
    {
        case PROPHANDLE_UINAME:
            aValue >>= m_aUIName;
            break;
    }
}

void SAL_CALL RootItemContainer::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                       sal_Int32                 nHandle                ) const
{
    switch( nHandle )
    {
        case PROPHANDLE_UINAME:
            aValue <<= m_aUIName;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL RootItemContainer::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL RootItemContainer::getPropertySetInfo()
throw (::com::sun::star::uno::RuntimeException)
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > RootItemContainer::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    static const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( PROPNAME_UINAME, PROPHANDLE_UINAME ,
                                         ::getCppuType((const rtl::OUString*)NULL),
                                         com::sun::star::beans::PropertyAttribute::TRANSIENT )
    };
    // Use it to initialize sequence!
    static const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, PROPCOUNT );
    // Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}

} // namespace framework

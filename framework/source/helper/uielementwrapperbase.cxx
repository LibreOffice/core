/*************************************************************************
 *
 *  $RCSfile: uielementwrapperbase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 16:57:38 $
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

#ifndef __FRAMEWORK_HELPER_UIELEMENTWRAPPERBASE_HXX_
#include <helper/uielementwrapperbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

const int UIELEMENT_PROPHANDLE_RESOURCEURL  = 1;
const int UIELEMENT_PROPHANDLE_TYPE         = 2;
const int UIELEMENT_PROPHANDLE_FRAME        = 3;
const int UIELEMENT_PROPCOUNT               = 3;
const rtl::OUString UIELEMENT_PROPNAME_RESOURCEURL( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ));
const rtl::OUString UIELEMENT_PROPNAME_TYPE( RTL_CONSTASCII_USTRINGPARAM( "Type" ));
const rtl::OUString UIELEMENT_PROPNAME_FRAME( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_8     (   UIElementWrapperBase                                            ,
                            OWeakObject                                                     ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider         ),
                            DIRECT_INTERFACE( ::drafts::com::sun::star::ui::XUIElement      ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XMultiPropertySet    ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet     ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet         ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XInitialization       ),
                            DIRECT_INTERFACE( ::com::sun::star::util::XUpdatable            ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XComponent            )
                        )

DEFINE_XTYPEPROVIDER_8  (   UIElementWrapperBase                        ,
                            ::com::sun::star::lang::XTypeProvider       ,
                            ::drafts::com::sun::star::ui::XUIElement    ,
                            ::com::sun::star::beans::XMultiPropertySet  ,
                            ::com::sun::star::beans::XFastPropertySet   ,
                            ::com::sun::star::beans::XPropertySet       ,
                            ::com::sun::star::lang::XInitialization     ,
                            ::com::sun::star::util::XUpdatable          ,
                            ::com::sun::star::lang::XComponent
                        )

UIElementWrapperBase::UIElementWrapperBase( sal_Int16 nType )
    :   ThreadHelpBase              ( &Application::GetSolarMutex()                      )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   ::cppu::OWeakObject         (                                                   )
    ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
    ,   m_nType                     ( nType                                             )
    ,   m_bInitialized              ( sal_False                                         )
    ,   m_bDisposed                 ( sal_False                                         )
{
}

UIElementWrapperBase::~UIElementWrapperBase()
{
}

void SAL_CALL UIElementWrapperBase::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    // must be implemented by derived class
    ResetableGuard aLock( m_aLock );
    m_bDisposed = sal_True;
}

void SAL_CALL UIElementWrapperBase::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIElementWrapperBase::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIElementWrapperBase::initialize( const Sequence< Any >& aArguments )
throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bInitialized )
    {
        for ( sal_Int32 n = 0; n < aArguments.getLength(); n++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[n] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "ResourceURL" ))
                    aPropValue.Value >>= m_aResourceURL;
                else if ( aPropValue.Name.equalsAscii( "Frame" ))
                {
                    Reference< XFrame > xFrame;
                    aPropValue.Value >>= xFrame;
                    m_xWeakFrame = xFrame;
                }
            }
        }

        m_bInitialized = sal_True;
    }
}

// XUpdatable
void SAL_CALL UIElementWrapperBase::update() throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

// XPropertySet helper
sal_Bool SAL_CALL UIElementWrapperBase::convertFastPropertyValue( Any&       aConvertedValue ,
                                                                  Any&       aOldValue       ,
                                                                  sal_Int32  nHandle         ,
                                                                  const Any& aValue             ) throw( com::sun::star::lang::IllegalArgumentException )
{
    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    return sal_False ;
}

void SAL_CALL UIElementWrapperBase::setFastPropertyValue_NoBroadcast(   sal_Int32               nHandle ,
                                                                        const com::sun::star::uno::Any&    aValue  ) throw( com::sun::star::uno::Exception )
{
}

void SAL_CALL UIElementWrapperBase::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                          sal_Int32      nHandle                ) const
{
    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue <<= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue <<= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
            Reference< XFrame > xFrame( m_xWeakFrame );
            aValue <<= xFrame;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL UIElementWrapperBase::getInfoHelper()
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

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL UIElementWrapperBase::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
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

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > UIElementWrapperBase::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    static const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_FRAME         , UIELEMENT_PROPHANDLE_FRAME          , ::getCppuType((Reference< XFrame >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_RESOURCEURL   , UIELEMENT_PROPHANDLE_RESOURCEURL    , ::getCppuType((sal_Int16*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_TYPE          , UIELEMENT_PROPHANDLE_TYPE           , ::getCppuType((const ::rtl::OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY )
    };
    // Use it to initialize sequence!
    static const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, UIELEMENT_PROPCOUNT );
    // Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}

}

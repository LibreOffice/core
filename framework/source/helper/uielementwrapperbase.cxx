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

#include <helper/uielementwrapperbase.hxx>
#include <general.h>
#include <properties.h>
#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

const int UIELEMENT_PROPHANDLE_RESOURCEURL  = 1;
const int UIELEMENT_PROPHANDLE_TYPE         = 2;
const int UIELEMENT_PROPHANDLE_FRAME        = 3;
const int UIELEMENT_PROPCOUNT               = 3;
const char UIELEMENT_PROPNAME_RESOURCEURL[] = "ResourceURL";
const char UIELEMENT_PROPNAME_TYPE[] = "Type";
const char UIELEMENT_PROPNAME_FRAME[] = "Frame";

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using ::rtl::OUString;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_8     (   UIElementWrapperBase                                            ,
                            OWeakObject                                                     ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider         ),
                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElement      ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XMultiPropertySet    ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet     ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet         ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XInitialization       ),
                            DIRECT_INTERFACE( ::com::sun::star::util::XUpdatable            ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XComponent            )
                        )

DEFINE_XTYPEPROVIDER_8  (   UIElementWrapperBase                        ,
                            ::com::sun::star::lang::XTypeProvider       ,
                            ::com::sun::star::ui::XUIElement    ,
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
                if ( aPropValue.Name == "ResourceURL" )
                    aPropValue.Value >>= m_aResourceURL;
                else if ( aPropValue.Name == "Frame" )
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

// XUIElement
::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL UIElementWrapperBase::getFrame() throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( m_xWeakFrame );
    return xFrame;
}

::rtl::OUString SAL_CALL UIElementWrapperBase::getResourceURL() throw (::com::sun::star::uno::RuntimeException)
{
    return m_aResourceURL;
}

::sal_Int16 SAL_CALL UIElementWrapperBase::getType() throw (::com::sun::star::uno::RuntimeException)
{
    return m_nType;
}

// XUpdatable
void SAL_CALL UIElementWrapperBase::update() throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

// XPropertySet helper
sal_Bool SAL_CALL UIElementWrapperBase::convertFastPropertyValue( Any&       /*aConvertedValue*/ ,
                                                                  Any&       /*aOldValue*/       ,
                                                                  sal_Int32  /*nHandle*/         ,
                                                                  const Any& /*aValue*/             ) throw( com::sun::star::lang::IllegalArgumentException )
{
    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    return sal_False ;
}

void SAL_CALL UIElementWrapperBase::setFastPropertyValue_NoBroadcast(   sal_Int32               /*nHandle*/ ,
                                                                        const com::sun::star::uno::Any&    /*aValue*/  ) throw( com::sun::star::uno::Exception )
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
    // Create a property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( rtl::OUString(UIELEMENT_PROPNAME_FRAME), UIELEMENT_PROPHANDLE_FRAME          , ::getCppuType((Reference< XFrame >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( rtl::OUString(UIELEMENT_PROPNAME_RESOURCEURL), UIELEMENT_PROPHANDLE_RESOURCEURL    , ::getCppuType((sal_Int16*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( rtl::OUString(UIELEMENT_PROPNAME_TYPE), UIELEMENT_PROPHANDLE_TYPE           , ::getCppuType((const ::rtl::OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY )
    };
    // Use it to initialize sequence!
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, UIELEMENT_PROPCOUNT );
    // Return "PropertyDescriptor"
    return lPropertyDescriptor;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

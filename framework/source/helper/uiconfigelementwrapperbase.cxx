/*************************************************************************
 *
 *  $RCSfile: uiconfigelementwrapperbase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 16:57:21 $
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

#ifndef __FRAMEWORK_HELPER_UICONFIGELEMENTWRAPPERBASE_HXX_
#include <helper/uiconfigelementwrapperbase.hxx>
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

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATION_HPP_
#include <drafts/com/sun/star/ui/XUIConfiguration.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

const int UIELEMENT_PROPHANDLE_CONFIGSOURCE     = 1;
const int UIELEMENT_PROPHANDLE_FRAME            = 2;
const int UIELEMENT_PROPHANDLE_PERSISTENT       = 3;
const int UIELEMENT_PROPHANDLE_RESOURCEURL      = 4;
const int UIELEMENT_PROPHANDLE_TYPE             = 5;
const int UIELEMENT_PROPHANDLE_XMENUBAR         = 6;
const int UIELEMENT_PROPHANDLE_CONFIGLISTENER   = 7;
const int UIELEMENT_PROPCOUNT                   = 7;
const rtl::OUString UIELEMENT_PROPNAME_CONFIGLISTENER( RTL_CONSTASCII_USTRINGPARAM( "ConfigListener" ));
const rtl::OUString UIELEMENT_PROPNAME_CONFIGSOURCE( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
const rtl::OUString UIELEMENT_PROPNAME_FRAME( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
const rtl::OUString UIELEMENT_PROPNAME_PERSISTENT( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
const rtl::OUString UIELEMENT_PROPNAME_RESOURCEURL( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ));
const rtl::OUString UIELEMENT_PROPNAME_TYPE( RTL_CONSTASCII_USTRINGPARAM( "Type" ));
const rtl::OUString UIELEMENT_PROPNAME_XMENUBAR( RTL_CONSTASCII_USTRINGPARAM( "XMenuBar" ));

using namespace rtl;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace ::drafts::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_10    (   UIConfigElementWrapperBase                                               ,
                            OWeakObject                                                              ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider                  ),
                            DIRECT_INTERFACE( ::drafts::com::sun::star::ui::XUIElement               ),
                            DIRECT_INTERFACE( ::drafts::com::sun::star::ui::XUIElementSettings       ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XMultiPropertySet             ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet              ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet                  ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XInitialization                ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XComponent                     ),
                            DIRECT_INTERFACE( ::com::sun::star::util::XUpdatable                     ),
                            DIRECT_INTERFACE( ::drafts::com::sun::star::ui::XUIConfigurationListener )
                        )

DEFINE_XTYPEPROVIDER_10 (   UIConfigElementWrapperBase                              ,
                            ::com::sun::star::lang::XTypeProvider                   ,
                            ::drafts::com::sun::star::ui::XUIElement                ,
                            ::drafts::com::sun::star::ui::XUIElementSettings        ,
                            ::com::sun::star::beans::XMultiPropertySet              ,
                            ::com::sun::star::beans::XFastPropertySet               ,
                            ::com::sun::star::beans::XPropertySet                   ,
                            ::com::sun::star::lang::XInitialization                 ,
                            ::com::sun::star::lang::XComponent                      ,
                            ::com::sun::star::util::XUpdatable                      ,
                            ::drafts::com::sun::star::ui::XUIConfigurationListener
                        )

UIConfigElementWrapperBase::UIConfigElementWrapperBase( sal_Int16 nType )
    :   ThreadHelpBase              ( &Application::GetSolarMutex()                      )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   ::cppu::OWeakObject         (                                                   )
    ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
    ,   m_nType                     ( nType                                             )
    ,   m_bInitialized              ( sal_False                                         )
    ,   m_bPersistent               ( sal_True                                          )
    ,   m_bConfigListener           ( sal_False                                         )
    ,   m_bConfigListening          ( sal_False                                         )
    ,   m_bDisposed                 ( sal_False                                         )
{
}

UIConfigElementWrapperBase::~UIConfigElementWrapperBase()
{
}

// XComponent
void SAL_CALL UIConfigElementWrapperBase::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    // must be implemented by derived class
    ResetableGuard aLock( m_aLock );
    m_bDisposed = sal_True;
}

void SAL_CALL UIConfigElementWrapperBase::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIConfigElementWrapperBase::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), aListener );
}

// XEventListener
void SAL_CALL UIConfigElementWrapperBase::disposing( const EventObject& aEvent )
throw( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    m_xConfigSource.clear();
}

void SAL_CALL UIConfigElementWrapperBase::initialize( const Sequence< Any >& aArguments )
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
                if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_CONFIGSOURCE ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_CONFIGSOURCE, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_FRAME ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_FRAME, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_PERSISTENT ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_PERSISTENT, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_RESOURCEURL ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_RESOURCEURL, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_TYPE ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_TYPE, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_CONFIGLISTENER ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_CONFIGLISTENER, aPropValue.Value );
            }
        }

        m_bInitialized = sal_True;
    }
}

// XUpdatable
void SAL_CALL UIConfigElementWrapperBase::update() throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementInserted( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementRemoved( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementReplaced( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

// XPropertySet helper
sal_Bool SAL_CALL UIConfigElementWrapperBase::convertFastPropertyValue( Any&       aConvertedValue ,
                                                                        Any&       aOldValue       ,
                                                                        sal_Int32  nHandle         ,
                                                                        const Any& aValue             ) throw( com::sun::star::lang::IllegalArgumentException )
{
    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bConfigListener),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_xConfigSource),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame( m_xWeakFrame );
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(xFrame),
                        aValue,
                        aOldValue,
                        aConvertedValue);
        }
        break;

        case UIELEMENT_PROPHANDLE_PERSISTENT:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bPersistent),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_aResourceURL),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_TYPE :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_nType),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;

        case UIELEMENT_PROPHANDLE_XMENUBAR :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_xMenuBar),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;
    }

    // Return state of operation.
    return bReturn ;
}

void SAL_CALL UIConfigElementWrapperBase::setFastPropertyValue_NoBroadcast(   sal_Int32               nHandle ,
                                                                        const com::sun::star::uno::Any&    aValue  ) throw( com::sun::star::uno::Exception )
{
    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
        {
            sal_Bool bBool( m_bConfigListener );
            aValue >>= bBool;
            if ( m_bConfigListener != bBool )
            {
                if ( m_bConfigListening )
                {
                    if ( m_xConfigSource.is() && !bBool )
                    {
                        try
                        {
                            Reference< XUIConfiguration > xUIConfig( m_xConfigSource, UNO_QUERY );
                            if ( xUIConfig.is() )
                            {
                                xUIConfig->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                                m_bConfigListening = sal_False;
                            }
                        }
                        catch ( Exception& )
                        {
                        }
                    }
                }
                else
                {
                    if ( m_xConfigSource.is() && bBool )
                    {
                        try
                        {
                            Reference< XUIConfiguration > xUIConfig( m_xConfigSource, UNO_QUERY );
                            if ( xUIConfig.is() )
                            {
                                xUIConfig->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                                m_bConfigListening = sal_True;
                            }
                        }
                        catch ( Exception& )
                        {
                        }
                    }
                }

                m_bConfigListener = bBool;
            }
        }
        break;
        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            aValue >>= m_xConfigSource;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame;

            aValue >>= xFrame;
            m_xWeakFrame = xFrame;
            break;
        }
        case UIELEMENT_PROPHANDLE_PERSISTENT:
        {
            sal_Bool bBool( m_bPersistent );
            aValue >>= bBool;
            m_bPersistent = bBool;
            break;
        }
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue >>= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue >>= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_XMENUBAR:
            aValue >>= m_xMenuBar;
            break;
    }
}

void SAL_CALL UIConfigElementWrapperBase::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                          sal_Int32      nHandle                ) const
{
    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
            aValue <<= m_bConfigListener;
            break;
        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            aValue <<= m_xConfigSource;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame( m_xWeakFrame );
            aValue <<= xFrame;
            break;
        }
        case UIELEMENT_PROPHANDLE_PERSISTENT:
            aValue <<= m_bPersistent;
            break;
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue <<= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue <<= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_XMENUBAR:
            aValue <<= m_xMenuBar;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL UIConfigElementWrapperBase::getInfoHelper()
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

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL UIConfigElementWrapperBase::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
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

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > UIConfigElementWrapperBase::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    static const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_CONFIGLISTENER, UIELEMENT_PROPHANDLE_CONFIGLISTENER , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_CONFIGSOURCE  , UIELEMENT_PROPHANDLE_CONFIGSOURCE   , ::getCppuType((const Reference< drafts::com::sun::star::ui::XUIConfigurationManager >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_FRAME         , UIELEMENT_PROPHANDLE_FRAME          , ::getCppuType((const Reference< com::sun::star::frame::XFrame >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_PERSISTENT    , UIELEMENT_PROPHANDLE_PERSISTENT     , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_RESOURCEURL   , UIELEMENT_PROPHANDLE_RESOURCEURL    , ::getCppuType((const ::rtl::OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_TYPE          , UIELEMENT_PROPHANDLE_TYPE           , ::getCppuType((const ::rtl::OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_XMENUBAR      , UIELEMENT_PROPHANDLE_XMENUBAR       , ::getCppuType((const Reference< com::sun::star::awt::XMenuBar >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY )
    };
    // Use it to initialize sequence!
    static const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, UIELEMENT_PROPCOUNT );
    // Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}

}

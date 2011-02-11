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

#ifndef __FRAMEWORK_HELPER_UICONFIGELEMENTWRAPPERBASE_HXX_
#define __FRAMEWORK_HELPER_UICONFIGELEMENTWRAPPERBASE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>

namespace framework
{

class UIConfigElementWrapperBase : public ::com::sun::star::lang::XTypeProvider                  ,
                                   public ::com::sun::star::ui::XUIElement               ,
                                   public ::com::sun::star::ui::XUIElementSettings       ,
                                   public ::com::sun::star::lang::XInitialization                ,
                                   public ::com::sun::star::lang::XComponent                     ,
                                   public ::com::sun::star::util::XUpdatable                     ,
                                   public ::com::sun::star::ui::XUIConfigurationListener ,
                                   protected ThreadHelpBase                                      ,
                                   public ::cppu::OBroadcastHelper                               ,
                                   public ::cppu::OPropertySetHelper                             ,
                                   public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
         UIConfigElementWrapperBase( sal_Int16 nType,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory );
        virtual  ~UIConfigElementWrapperBase();

        //  XInterface
        virtual  ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw( ::com::sun::star::uno::RuntimeException );
        virtual  void SAL_CALL acquire() throw();
        virtual  void SAL_CALL release() throw();

        // XTypeProvider
        virtual  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw( ::com::sun::star::uno::RuntimeException );
        virtual  ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw( ::com::sun::star::uno::RuntimeException );

        // XComponent
        virtual  void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual  void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual  void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual  void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUIElementSettings
        virtual  void SAL_CALL updateSettings() throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual  ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getSettings( sal_Bool bWriteable ) throw (::com::sun::star::uno::RuntimeException);
        virtual  void SAL_CALL setSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& UISettings ) throw (::com::sun::star::uno::RuntimeException);

        // XUIElement
        virtual  ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException);
        virtual  ::rtl::OUString SAL_CALL getResourceURL() throw (::com::sun::star::uno::RuntimeException);
        virtual  ::sal_Int16 SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException);
        virtual  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException) = 0;

        // XUpdatable
        virtual  void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);

        //  XUIConfigurationListener
        virtual  void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual  void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual  void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        using cppu::OPropertySetHelper::disposing;
        virtual  void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:

        //  OPropertySetHelper
        virtual  sal_Bool                                            SAL_CALL convertFastPropertyValue        ( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                               com::sun::star::uno::Any&        aOldValue       ,
                                                                                                               sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException );
        virtual  void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                               const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception                 );
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual  void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                   sal_Int32                    nHandle         ) const;
        virtual  ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper();
        virtual  ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);

        virtual  void impl_fillNewData();

        static  const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

        sal_Int16                                                                               m_nType;
        bool                                                                                    m_bPersistent : 1,
                                                                                                m_bInitialized : 1,
                                                                                                m_bConfigListener : 1,
                                                                                                m_bConfigListening : 1,
                                                                                                m_bDisposed : 1,
                                                                                                m_bNoClose : 1;
        rtl::OUString                                                                           m_aResourceURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >        m_xServiceFactory;
        com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >         m_xConfigSource;
        com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >               m_xConfigData;
        com::sun::star::uno::WeakReference< com::sun::star::frame::XFrame >                     m_xWeakFrame;
        com::sun::star::uno::Reference< com::sun::star::awt::XMenuBar >                         m_xMenuBar;
        ::cppu::OMultiTypeInterfaceContainerHelper                                              m_aListenerContainer;   /// container for ALL Listener
};

} // namespace framework

#endif // __FRAMEWORK_HELPER_UIELEMENTWRAPPERBASE_HXX_

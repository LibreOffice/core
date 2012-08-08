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

#ifndef __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <stdtypes.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XStatusbarController.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <vcl/status.hxx>

namespace framework
{

class FrameworkStatusBar;
class StatusBarManager : public ::com::sun::star::frame::XFrameActionListener         ,
                         public ::com::sun::star::lang::XComponent                    ,
                         public ::com::sun::star::lang::XTypeProvider                 ,
                         public ::com::sun::star::ui::XUIConfigurationListener,
                         public ThreadHelpBase                                        ,
                         public ::cppu::OWeakObject
{
    friend class FrameworkStatusBar;

    public:
        StatusBarManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServicveManager,
                          const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                          const rtl::OUString& rResourceName,
                          StatusBar* pStatusBar );
        virtual ~StatusBarManager();

        //  XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        StatusBar* GetStatusBar() const;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );
        void SAL_CALL addEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
        void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );

        void FillStatusBar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rStatusBarData );

    protected:
        void StateChanged( StateChangedType nType );
        void DataChanged( const DataChangedEvent& rDCEvt );
        void UserDraw( const UserDrawEvent& rUDEvt );
        void Command( const CommandEvent& rEvt );
        void MouseMove( const MouseEvent& rMEvt );
        void MouseButtonDown( const MouseEvent& rMEvt );
        void MouseButtonUp( const MouseEvent& rMEvt );
        DECL_LINK(Click, void *);
        DECL_LINK(DoubleClick, void *);

        void RemoveControllers();
        rtl::OUString RetrieveLabelFromCommand( const rtl::OUString& aCmdURL );
        void CreateControllers();
        void UpdateControllers();
        void AddFrameActionListener();
        void MouseButton( const MouseEvent& rMEvt ,sal_Bool ( SAL_CALL ::com::sun::star::frame::XStatusbarController::*_pMethod )(const ::com::sun::star::awt::MouseEvent&));

    protected:
        typedef std::vector< ::com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > > StatusBarControllerVector;

        sal_Bool                                                                                        m_bDisposed : 1,
                                                                                                        m_bFrameActionRegistered : 1,
                                                                                                        m_bUpdateControllers : 1;
        sal_Bool                                                                                        m_bModuleIdentified;
        StatusBar*                                                                                      m_pStatusBar;
        rtl::OUString                                                                                   m_aModuleIdentifier;
        rtl::OUString                                                                                   m_aResourceName;
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame >                                 m_xFrame;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >                        m_xUICommandLabels;
        StatusBarControllerVector                                                                       m_aControllerVector;
        ::cppu::OMultiTypeInterfaceContainerHelper                                                      m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >                  m_xServiceManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUIControllerRegistration >  m_xStatusbarControllerRegistration;
};

}

#endif // __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

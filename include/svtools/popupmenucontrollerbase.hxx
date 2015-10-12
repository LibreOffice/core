/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVTOOLS_POPUPMENUCONTROLLERBASE_HXX
#define INCLUDED_SVTOOLS_POPUPMENUCONTROLLERBASE_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/compbase7.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace svt
{
    struct PopupMenuControllerBaseDispatchInfo;

    typedef ::cppu::WeakComponentImplHelper7<
                        com::sun::star::lang::XServiceInfo            ,
                        com::sun::star::frame::XPopupMenuController ,
                        com::sun::star::lang::XInitialization         ,
                        com::sun::star::frame::XStatusListener        ,
                        com::sun::star::awt::XMenuListener            ,
                        com::sun::star::frame::XDispatchProvider      ,
                        com::sun::star::frame::XDispatch > PopupMenuControllerBaseType;

    class SVT_DLLPUBLIC PopupMenuControllerBase : protected ::comphelper::OBaseMutex,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                                  public PopupMenuControllerBaseType
    {
        public:
            PopupMenuControllerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
            virtual ~PopupMenuControllerBase();

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override = 0;
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override = 0;

            // XPopupMenuController
            virtual void SAL_CALL setPopupMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& PopupMenu ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updatePopupMenu() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override = 0;

            // XMenuListener
            virtual void SAL_CALL itemHighlighted( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL itemSelected( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL itemActivated( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL itemDeactivated( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XDispatchProvider
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const OUString& sTarget, sal_Int32 nFlags ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

            // XDispatch
            virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& seqProperties ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
            virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
            virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

            void dispatchCommand( const OUString& sCommandURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );

    protected:
            void throwIfDisposed() throw ( ::com::sun::star::uno::RuntimeException );

            /** helper method to cause statusChanged is called once for the given command url */
            void SAL_CALL updateCommand( const OUString& rCommandURL );

            /** this function is called upon disposing the component
            */
            virtual void SAL_CALL disposing() override;

            static void resetPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            virtual void impl_setPopupMenu();
            static OUString determineBaseURL( const OUString& aURL );

            DECL_STATIC_LINK_TYPED( PopupMenuControllerBase, ExecuteHdl_Impl, void*, void );


            bool                                                                             m_bInitialized;
            OUString                                                                    m_aCommandURL;
            OUString                                                                    m_aBaseURL;
            OUString                                                                    m_aModuleName;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >           m_xDispatch;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >              m_xFrame;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >      m_xURLTransformer;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >            m_xPopupMenu;
    };
}

#endif // INCLUDED_SVTOOLS_POPUPMENUCONTROLLERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

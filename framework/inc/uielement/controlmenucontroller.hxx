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

#ifndef __FRAMEWORK_UIELEMENT_CONTROLMENUCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_CONTROLMENUCONTROLLER_HXX_

#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/status/Verb.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

class PopupMenu;
namespace framework
{
    class ControlMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            ControlMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~ControlMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XPopupMenuController
            virtual void SAL_CALL updatePopupMenu() throw (::com::sun::star::uno::RuntimeException);

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

            // XMenuListener
            virtual void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        private:
            virtual void impl_setPopupMenu();
            virtual void impl_select(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aURL);

            class UrlToDispatchMap : public ::boost::unordered_map< OUString,
                                                             com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >,
                                                             OUStringHash,
                                                             ::std::equal_to< OUString > >
            {
                public:
                    inline void free()
                    {
                        UrlToDispatchMap().swap( *this );
                    }
            };

            void updateImagesPopupMenu( PopupMenu* pPopupMenu );
            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );

            sal_Bool            m_bShowMenuImages : 1;
            PopupMenu*          m_pResPopupMenu;
            UrlToDispatchMap    m_aURLToDispatchMap;
    };
}

#endif // __FRAMEWORK_UIELEMENT_CONTROLMENUCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

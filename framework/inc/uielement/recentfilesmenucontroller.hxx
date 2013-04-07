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

#ifndef __FRAMEWORK_UIELEMENT_RECENTFILESMENUCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_RECENTFILESMENUCONTROLLER_HXX_

#include <macros/xserviceinfo.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <macros/xserviceinfo.hxx>

namespace framework
{
    struct LoadRecentFile
    {
        ::com::sun::star::util::URL                                                 aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   aArgSeq;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      xDispatch;
    };

    class RecentFilesMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            RecentFilesMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~RecentFilesMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

            // XMenuListener
            virtual void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XDispatchProvider
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const OUString& sTarget, sal_Int32 nFlags ) throw( ::com::sun::star::uno::RuntimeException );

            // XDispatch
            virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& seqProperties ) throw( ::com::sun::star::uno::RuntimeException );

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

            DECL_STATIC_LINK( RecentFilesMenuController, ExecuteHdl_Impl, LoadRecentFile* );

        private:
            virtual void impl_setPopupMenu();
            struct RecentFile
            {
                OUString aURL;
                OUString aTitle;
                OUString aPassword;
            };

            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            void executeEntry( sal_Int32 nIndex );

            std::vector< RecentFile > m_aRecentFilesItems;
            sal_Bool                  m_bDisabled : 1;
    };
}

#endif // __FRAMEWORK_UIELEMENT_RECENTFILESMENUCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _LIMITBOXCONTROLLER_HXX_
#define _LIMITBOXCONTROLLER_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <svtools/toolboxcontroller.hxx>
#include <rtl/ustring.hxx>

#include "apitools.hxx"

namespace dbaui
{

class LimitBoxImpl;

/**
 * A ToolboxController to paste LimitBox onto the Query Design Toolbar
 * It is communicating with querycontroller and this channel make enable
 * to set\get the value of limitbox when switching between views
 */
class LimitBoxController: public svt::ToolboxController,
                          public ::com::sun::star::lang::XServiceInfo
{
    public:
        LimitBoxController(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );
        ~LimitBoxController();

        /// XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        /// XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();

        /// XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

        /// XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        /// XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);

        void dispatchCommand( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );
        using svt::ToolboxController::dispatchCommand;

    private:
        LimitBoxImpl* m_pLimitBox;
};

} ///dbaui namespace

#endif /// _LIMITBOXCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

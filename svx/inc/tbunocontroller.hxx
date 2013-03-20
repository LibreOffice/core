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

#ifndef __TBUNOCONTROLLER_HXX_
#define __TBUNOCONTROLLER_HXX_

#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace svx
{

class SvxFontSizeBox_Impl;
class FontHeightToolBoxControl : public svt::ToolboxController,
                                 public ::com::sun::star::lang::XServiceInfo
{
    public:
        FontHeightToolBoxControl(
            const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rServiceManager );
        ~FontHeightToolBoxControl();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

        static OUString getImplementationName_Static() throw()
        {
            return OUString("com.sun.star.svx.FontHeightToolBoxController");
        }

        static ::com::sun::star::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);

        void dispatchCommand( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );
        using svt::ToolboxController::dispatchCommand;

    private:
        SvxFontSizeBox_Impl*                  m_pBox;
        ::com::sun::star::awt::FontDescriptor m_aCurrentFont;
};

::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL FontHeightToolBoxControl_createInstance( const com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );

}

#endif // __TBUNOCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

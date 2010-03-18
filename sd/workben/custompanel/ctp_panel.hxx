/*************************************************************************
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

#ifndef SD_WORKBENCH_CTP_PANEL_HXX
#define SD_WORKBENCH_CTP_PANEL_HXX

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= class SingleColorPanel
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::drawing::framework::XView
                                    >   SingleColorPanel_Base;
    class SingleColorPanel  :public ::cppu::BaseMutex
                            ,public SingleColorPanel_Base
    {
    public:
        SingleColorPanel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XConfigurationController >& i_rConfigController,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& i_rResourceId
        );

        // XView
        // (no methods)

        // XResource
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId > SAL_CALL getResourceId(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isAnchorOnly(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~SingleColorPanel();

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >   m_xResourceId;
    };

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................

#endif // SD_WORKBENCH_CTP_PANEL_HXX

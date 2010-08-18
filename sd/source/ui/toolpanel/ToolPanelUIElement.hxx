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

#ifndef SD_TOOLPANELUIELEMENT_HXX
#define SD_TOOLPANELUIELEMENT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
/** === end UNO includes === **/

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <memory>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    class TreeNode;

    //==================================================================================================================
    //= ToolPanelUIElement
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper1    <   ::com::sun::star::ui::XUIElement
                                                >   ToolPanelUIElement_Base;
    class ToolPanelUIElement    :public ::cppu::BaseMutex
                                ,public ToolPanelUIElement_Base
    {
    public:
        ToolPanelUIElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame,
            const ::rtl::OUString& i_rResourceURL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >& i_rToolPanel
        );

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getResourceURL() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface(  ) throw (::com::sun::star::uno::RuntimeException);

        void checkDisposed();
        ::osl::Mutex& getMutex() { return m_aMutex; }

    protected:
        virtual ~ToolPanelUIElement();

        // OComponentHelper
        virtual void SAL_CALL disposing();

    private:
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >   m_xFrame;
        const ::rtl::OUString                                                       m_sResourceURL;
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >  m_xToolPanel;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANELUIELEMENT_HXX

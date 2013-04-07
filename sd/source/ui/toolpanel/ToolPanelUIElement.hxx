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

#ifndef SD_TOOLPANELUIELEMENT_HXX
#define SD_TOOLPANELUIELEMENT_HXX

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <memory>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

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
            const OUString& i_rResourceURL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >& i_rToolPanel
        );

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getResourceURL() throw (::com::sun::star::uno::RuntimeException);
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
        const OUString                                                       m_sResourceURL;
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >  m_xToolPanel;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANELUIELEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

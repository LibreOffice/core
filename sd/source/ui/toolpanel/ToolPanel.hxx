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

#ifndef SD_TOOLPANEL_HXX
#define SD_TOOLPANEL_HXX

#include <com/sun/star/ui/XToolPanel.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include <memory>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    class TreeNode;

    //==================================================================================================================
    //= ToolPanel
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper1    <   ::com::sun::star::ui::XToolPanel
                                                >   ToolPanel_Base;
    class ToolPanel :public ::cppu::BaseMutex
                    ,public ToolPanel_Base
    {
    public:
        ToolPanel(
            ::std::auto_ptr< TreeNode >& i_rControl
        );

        // XToolPanel
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL createAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& ParentAccessible ) throw (::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing();

        ::osl::Mutex& getMutex() { return m_aMutex; }
        void checkDisposed();

    protected:
        ~ToolPanel();

    private:
        ::std::auto_ptr< TreeNode > m_pControl;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

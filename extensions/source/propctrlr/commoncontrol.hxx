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

#pragma once

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <tools/link.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>

class NotifyEvent;
class ColorListBox;
class SvtCalendarBox;

namespace pcr
{

    //= CommonBehaviourControlHelper

    /** A helper class for implementing the <type scope="css::inspection">XPropertyControl</type>
        or one of its derived interfaces.

        This class is used as a base class the CommonBehaviourControl template.
    */
    class CommonBehaviourControlHelper
    {
    private:
        sal_Int16                       m_nControlType;
        css::uno::Reference< css::inspection::XPropertyControlContext >
                                        m_xContext;
        css::inspection::XPropertyControl&
                                        m_rAntiImpl;
        bool                            m_bModified;

    public:
        /** creates the instance
            @param  nControlType
                the type of the control - one of the <type scope="css::inspection">PropertyControlType</type>
                constants
            @param pAntiImpl
                Reference to the instance as whose "impl-class" we act i.e. the CommonBehaviourControl<> template,
                which is why we hold it without acquiring it/
        */
        CommonBehaviourControlHelper(
            sal_Int16 nControlType,
            css::inspection::XPropertyControl& rAntiImpl);

        virtual ~CommonBehaviourControlHelper();

        virtual void setModified() { m_bModified = true; }

        virtual void editChanged();

        // XPropertyControl
        /// @throws css::uno::RuntimeException
        ::sal_Int16 getControlType() const { return m_nControlType; }
        /// @throws css::uno::RuntimeException
        const css::uno::Reference< css::inspection::XPropertyControlContext >& getControlContext() const { return m_xContext; }
        /// @throws css::uno::RuntimeException
        void setControlContext( const css::uno::Reference< css::inspection::XPropertyControlContext >& controlcontext );
        /// @throws css::uno::RuntimeException
        bool isModified(  ) const { return m_bModified; }
        /// @throws css::uno::RuntimeException
        void notifyModifiedValue(  );

        virtual weld::Widget* getWidget() = 0;

        /// may be used by derived classes, they forward the event to the PropCtrListener
        DECL_LINK( ModifiedHdl, weld::ComboBox&, void );
        DECL_LINK( ColorModifiedHdl, ColorListBox&, void );
        DECL_LINK( EditModifiedHdl, weld::Entry&, void );
        DECL_LINK( MetricModifiedHdl, weld::MetricSpinButton&, void );
        DECL_LINK( FormattedModifiedHdl, weld::FormattedSpinButton&, void );
        DECL_LINK( TimeModifiedHdl, weld::FormattedSpinButton&, void );
        DECL_LINK( DateModifiedHdl, SvtCalendarBox&, void );
        DECL_LINK( GetFocusHdl, weld::Widget&, void );
        DECL_LINK( LoseFocusHdl, weld::Widget&, void );
    };


    //= CommonBehaviourControl

    /** implements a base class for <type scope="css::inspection">XPropertyControl</type>
        implementations

        @param TControlInterface
            an interface class which is derived from (or identical to) <type scope="css::inspection">XPropertyControl</type>
        @param TControlWindow
            a class which is derived from weld::Widget
    */
    template < class TControlInterface, class TControlWindow >
    class CommonBehaviourControl    :public ::cppu::BaseMutex
                                    ,public ::cppu::WeakComponentImplHelper< TControlInterface >
                                    ,public CommonBehaviourControlHelper
    {
    protected:
        typedef ::cppu::WeakComponentImplHelper< TControlInterface >    ComponentBaseClass;

        inline CommonBehaviourControl(sal_Int16 nControlType,
                                      std::unique_ptr<weld::Builder> xBuilder,
                                      std::unique_ptr<TControlWindow> xWidget,
                                      bool bReadOnly);

        virtual ~CommonBehaviourControl() override
        {
            clear_widgetry();
        }

        // XPropertyControl - delegated to ->m_aImplControl
        virtual ::sal_Int16 SAL_CALL getControlType() override
            { return CommonBehaviourControlHelper::getControlType(); }
        virtual css::uno::Reference< css::inspection::XPropertyControlContext > SAL_CALL getControlContext() override
            { return CommonBehaviourControlHelper::getControlContext(); }
        virtual void SAL_CALL setControlContext( const css::uno::Reference< css::inspection::XPropertyControlContext >& controlcontext ) override
            { CommonBehaviourControlHelper::setControlContext( controlcontext ); }
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getControlWindow() override
            { return new weld::TransportAsXWindow(getWidget()); }
        virtual sal_Bool SAL_CALL isModified(  ) override
            { return CommonBehaviourControlHelper::isModified(); }
        virtual void SAL_CALL notifyModifiedValue(  ) override
            { CommonBehaviourControlHelper::notifyModifiedValue(); }

        void clear_widgetry()
        {
            if (!m_xControlWindow)
                return;
            weld::Widget* pWidget = getWidget();
            std::unique_ptr<weld::Container> xParent(pWidget->weld_parent());
            xParent->move(pWidget, nullptr);
            m_xControlWindow.reset();
            m_xBuilder.reset();
        }

        // XComponent
        virtual void SAL_CALL disposing() override
        {
            clear_widgetry();
        }

        TControlWindow*       getTypedControlWindow()
            { return m_xControlWindow.get(); }
        const TControlWindow* getTypedControlWindow() const
            { return m_xControlWindow.get(); }

        virtual void SetModifyHandler()
        {
            m_xControlWindow->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xControlWindow->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );
        }

        /** checks whether the instance is already disposed
            @throws DisposedException
                if the instance is already disposed
        */
        inline void impl_checkDisposed_throw();
    protected:
        std::unique_ptr<weld::Builder> m_xBuilder;
    private:
        std::unique_ptr<TControlWindow> m_xControlWindow;
    };

    //= CommonBehaviourControl - implementation
    template< class TControlInterface, class TControlWindow >
    inline CommonBehaviourControl< TControlInterface, TControlWindow >::CommonBehaviourControl(sal_Int16 nControlType,
                                                                                               std::unique_ptr<weld::Builder> xBuilder,
                                                                                               std::unique_ptr<TControlWindow> xWidget,
                                                                                               bool bReadOnly)
        : ComponentBaseClass( m_aMutex )
        , CommonBehaviourControlHelper( nControlType, *this )
        , m_xBuilder(std::move(xBuilder))
        , m_xControlWindow(std::move(xWidget))
    {
        if (bReadOnly)
        {
            // disable widget by default, entries will override to enable the widget but set it non-editable
            m_xControlWindow->set_sensitive(false);
        }
    }

    template< class TControlInterface, class TControlWindow >
    inline void CommonBehaviourControl< TControlInterface, TControlWindow >::impl_checkDisposed_throw()
    {
        if ( ComponentBaseClass::rBHelper.bDisposed )
            throw css::lang::DisposedException( OUString(), *this );
    }

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

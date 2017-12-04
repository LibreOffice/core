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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_COMMONCONTROL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_COMMONCONTROL_HXX

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <type_traits>

class NotifyEvent;
class Control;
class ListBox;
class SvxColorListBox;
class Edit;

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
            @param  _nControlType
                the type of the control - one of the <type scope="css::inspection">PropertyControlType</type>
                constants
            @param _pAntiImpl
                Reference to the instance as whose "impl-class" we act i.e. the CommonBehaviourControl<> template,
                which is why we hold it without acquiring it/
        */
        CommonBehaviourControlHelper(
            sal_Int16 _nControlType,
            css::inspection::XPropertyControl& _rAntiImpl);

        virtual ~CommonBehaviourControlHelper();

        virtual void setModified() { m_bModified = true; }

        // XPropertyControl
        /// @throws css::uno::RuntimeException
        ::sal_Int16 getControlType() { return m_nControlType; }
        /// @throws css::uno::RuntimeException
        const css::uno::Reference< css::inspection::XPropertyControlContext >& getControlContext() { return m_xContext; }
        /// @throws css::uno::RuntimeException
        void setControlContext( const css::uno::Reference< css::inspection::XPropertyControlContext >& _controlcontext );
        /// @throws css::uno::RuntimeException
        bool isModified(  ) { return m_bModified; }
        /// @throws css::uno::RuntimeException
        void notifyModifiedValue(  );

        /** (fail-safe) wrapper around calling our context's activateNextControl
        */
        void activateNextControl() const;

        /// automatically size the window given in the ctor
        void    autoSizeWindow();

        virtual vcl::Window* getVclWindow() = 0;

        /// may be used by derived classes, they forward the event to the PropCtrListener
        DECL_LINK( ModifiedHdl, ListBox&, void );
        DECL_LINK( ColorModifiedHdl, SvxColorListBox&, void );
        DECL_LINK( EditModifiedHdl, Edit&, void );
        DECL_LINK( GetFocusHdl, Control&, void );
        DECL_LINK( LoseFocusHdl, Control&, void );
    };


    //= CommonBehaviourControl

    /** implements a base class for <type scope="css::inspection">XPropertyControl</type>
        implementations

        @param TControlInterface
            an interface class which is derived from (or identical to) <type scope="css::inspection">XPropertyControl</type>
        @param TControlWindow
            a class which is derived from vcl::Window
    */
    template < class TControlInterface, class TControlWindow >
    class CommonBehaviourControl    :public ::cppu::BaseMutex
                                    ,public ::cppu::WeakComponentImplHelper< TControlInterface >
                                    ,public CommonBehaviourControlHelper
    {
    protected:
        typedef ::cppu::WeakComponentImplHelper< TControlInterface >    ComponentBaseClass;

        inline CommonBehaviourControl( sal_Int16 _nControlType, vcl::Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers = true );

        // XPropertyControl - delegated to ->m_aImplControl
        virtual ::sal_Int16 SAL_CALL getControlType() override
            { return CommonBehaviourControlHelper::getControlType(); }
        virtual css::uno::Reference< css::inspection::XPropertyControlContext > SAL_CALL getControlContext() override
            { return CommonBehaviourControlHelper::getControlContext(); }
        virtual void SAL_CALL setControlContext( const css::uno::Reference< css::inspection::XPropertyControlContext >& _controlcontext ) override
            { CommonBehaviourControlHelper::setControlContext( _controlcontext ); }
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getControlWindow() override
            { return VCLUnoHelper::GetInterface( m_pControlWindow ); }
        virtual sal_Bool SAL_CALL isModified(  ) override
            { return CommonBehaviourControlHelper::isModified(); }
        virtual void SAL_CALL notifyModifiedValue(  ) override
            { CommonBehaviourControlHelper::notifyModifiedValue(); }

        // XComponent
        virtual void SAL_CALL disposing() override
            { m_pControlWindow.disposeAndClear(); }

       //  CommonBehaviourControlHelper::getVclWindow
        virtual vcl::Window*  getVclWindow() override
            { return m_pControlWindow.get(); }

        TControlWindow*       getTypedControlWindow()
            { return m_pControlWindow.get(); }
        const TControlWindow* getTypedControlWindow() const
            { return m_pControlWindow.get(); }

        /** checks whether the instance is already disposed
            @throws DisposedException
                if the instance is already disposed
        */
        inline void impl_checkDisposed_throw();
    private:
        VclPtr<TControlWindow>         m_pControlWindow;
        void implSetModifyHandler(const Edit&);
        void implSetModifyHandler(const ListBox&);
        void implSetModifyHandler(const SvxColorListBox&);
    };


    //= CommonBehaviourControl - implementation

    template< class TControlInterface, class TControlWindow >
    inline CommonBehaviourControl< TControlInterface, TControlWindow >::CommonBehaviourControl ( sal_Int16 _nControlType, vcl::Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers)
        :ComponentBaseClass( m_aMutex )
        ,CommonBehaviourControlHelper( _nControlType, *this )
        ,m_pControlWindow( VclPtr<TControlWindow>::Create( _pParentWindow, _nWindowStyle ) )
    {
        if ( _bDoSetHandlers )
        {
            implSetModifyHandler(*m_pControlWindow);
            m_pControlWindow->SetGetFocusHdl( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_pControlWindow->SetLoseFocusHdl( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );
        }
        autoSizeWindow();
    }

    template< class TControlInterface, class TControlWindow >
    inline void CommonBehaviourControl< TControlInterface, TControlWindow >::implSetModifyHandler(const Edit&)
    {
        m_pControlWindow->SetModifyHdl( LINK( this, CommonBehaviourControlHelper, EditModifiedHdl ) );
    }

    template< class TControlInterface, class TControlWindow >
    inline void CommonBehaviourControl< TControlInterface, TControlWindow >::implSetModifyHandler(const ListBox&)
    {
        m_pControlWindow->SetModifyHdl( LINK( this, CommonBehaviourControlHelper, ModifiedHdl ) );
    }

    template< class TControlInterface, class TControlWindow >
    inline void CommonBehaviourControl< TControlInterface, TControlWindow >::implSetModifyHandler(const SvxColorListBox&)
    {
        m_pControlWindow->SetModifyHdl( LINK( this, CommonBehaviourControlHelper, ColorModifiedHdl ) );
    }

    template< class TControlInterface, class TControlWindow >
    inline void CommonBehaviourControl< TControlInterface, TControlWindow >::impl_checkDisposed_throw()
    {
        if ( ComponentBaseClass::rBHelper.bDisposed )
            throw css::lang::DisposedException( OUString(), *this );
    }


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_COMMONCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

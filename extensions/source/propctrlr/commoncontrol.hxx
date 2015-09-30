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
#include <comphelper/broadcasthelper.hxx>
#include <tools/link.hxx>
#include <vcl/window.hxx>

class NotifyEvent;
class Control;

namespace pcr
{


    class ControlHelper;

    template< class WINDOW >
    class ControlWindow : public WINDOW
    {
    protected:
        typedef WINDOW  WindowType;

    protected:
        ControlHelper*  m_pHelper;

    public:
        ControlWindow( vcl::Window* _pParent, WinBits _nStyle )
            :WindowType( _pParent, _nStyle )
            ,m_pHelper( NULL )
        {
        }

        /// sets a ControlHelper instance which some functionality is delegated to
        inline virtual void setControlHelper( ControlHelper& _rControlHelper );
    };


    //= IModifyListener

    class SAL_NO_VTABLE IModifyListener
    {
    public:
        virtual void modified() = 0;

    protected:
        ~IModifyListener() {}
    };


    //= ControlHelper

    /** A helper class for implementing the <type scope="css::inspection">XPropertyControl</type>
        or one of its derived interfaces.

        This class is intended to be held as member of another class which implements the
        <type scope="css::inspection">XPropertyControl</type> interface. The pointer
        to this interface is to be passed to the ctor.
    */
    class ControlHelper
    {
    private:
        VclPtr<vcl::Window>             m_pControlWindow;
        sal_Int16                       m_nControlType;
        css::uno::Reference< css::inspection::XPropertyControlContext >
                                        m_xContext;
        css::inspection::XPropertyControl&
                                        m_rAntiImpl;
        IModifyListener*                m_pModifyListener;
        bool                            m_bModified;

    public:
        /** creates the instance
            @param  _rControlWindow
                the window which is associated with the <type scope="css::inspection">XPropertyControl</type>.
                Must not be <NULL/>.<br/>
                Ownership for this window is taken by the ControlHelper - it will be deleted in <member>disposing</member>.
            @param  _nControlType
                the type of the control - one of the <type scope="css::inspection">PropertyControlType</type>
                constants
            @param _pAntiImpl
                Reference to the instance as whose "impl-class" we act. This reference is held during lifetime
                of the <type>ControlHelper</type> class, within acquiring it. Thus, the owner of the
                <type>ControlHelper</type> is responsible for assuring the lifetime of the instance
                pointed to by <arg>_pAntiImpl</arg>.
            @param _pModifyListener
                a listener to be modfied when the user modified the control's value. the
                <member>IModifyListener::modified</member> of this listener is called from within our
                ModifiedHdl. A default implementation of <member>IModifyListener::modified</member>
                would just call our <member>setModified</member>.
        */
        ControlHelper(
            vcl::Window* _pControlWindow,
            sal_Int16 _nControlType,
            css::inspection::XPropertyControl& _rAntiImpl,
            IModifyListener* _pModifyListener );

        virtual ~ControlHelper();

        /** sets our "modified" flag to <TRUE/>
        */
        inline void setModified() { m_bModified = true; }
        inline       vcl::Window* getVclControlWindow()       { return m_pControlWindow; }
        inline const vcl::Window* getVclControlWindow() const { return m_pControlWindow; }

    public:
        // XPropertyControl
        ::sal_Int16 SAL_CALL getControlType() throw (css::uno::RuntimeException) { return m_nControlType; }
        css::uno::Reference< css::inspection::XPropertyControlContext > SAL_CALL getControlContext() throw (css::uno::RuntimeException) { return m_xContext; }
        void SAL_CALL setControlContext( const css::uno::Reference< css::inspection::XPropertyControlContext >& _controlcontext ) throw (css::uno::RuntimeException);
        css::uno::Reference< css::awt::XWindow > SAL_CALL getControlWindow() throw (css::uno::RuntimeException);
        bool SAL_CALL isModified(  ) throw (css::uno::RuntimeException) { return m_bModified; }
        void SAL_CALL notifyModifiedValue(  ) throw (css::uno::RuntimeException);

        // XComponent
        void SAL_CALL dispose();

        /** (fail-safe) wrapper around calling our context's activateNextControl
        */
        inline void activateNextControl() const { impl_activateNextControl_nothrow(); }

    public:
        /// automatically size the window given in the ctor
        void    autoSizeWindow();

        /// may be used by derived classes, they forward the event to the PropCtrListener
        DECL_LINK( ModifiedHdl, vcl::Window* );
        DECL_LINK_TYPED( GetFocusHdl, Control&, void );
        DECL_LINK_TYPED( LoseFocusHdl, Control&, void );

    private:
        /** fail-safe wrapper around calling our context's activateNextControl
        */
        void    impl_activateNextControl_nothrow() const;
    };


    //= CommonBehaviourControl

    /** implements a base class for <type scope="css::inspection">XPropertyControl</type>
        implementations, which delegates the generic functionality of this interface to a
        <type>ControlHelper</type> member.

        @param CONTROL_INTERFACE
            an interface class which is derived from (or identical to) <type scope="css::inspection">XPropertyControl</type>
        @param CONTROL_WINDOW
            a class which is derived from ControlWindow
    */
    template < class CONTROL_INTERFACE, class CONTROL_WINDOW >
    class CommonBehaviourControl    :public ::comphelper::OBaseMutex
                                    ,public ::cppu::WeakComponentImplHelper< CONTROL_INTERFACE >
                                    ,public IModifyListener
    {
    protected:
        typedef CONTROL_INTERFACE   InterfaceType;
        typedef CONTROL_WINDOW      WindowType;

        typedef ::comphelper::OBaseMutex                                MutexBaseClass;
        typedef ::cppu::WeakComponentImplHelper< CONTROL_INTERFACE >   ComponentBaseClass;

    protected:
        ControlHelper   m_aImplControl;

    protected:
        inline CommonBehaviourControl( sal_Int16 _nControlType, vcl::Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers = true );

        // XPropertyControl - delegated to ->m_aImplControl
        virtual ::sal_Int16 SAL_CALL getControlType() throw (css::uno::RuntimeException) SAL_OVERRIDE;
        virtual css::uno::Reference< css::inspection::XPropertyControlContext > SAL_CALL getControlContext() throw (css::uno::RuntimeException) SAL_OVERRIDE;
        virtual void SAL_CALL setControlContext( const css::uno::Reference< css::inspection::XPropertyControlContext >& _controlcontext ) throw (css::uno::RuntimeException) SAL_OVERRIDE;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getControlWindow() throw (css::uno::RuntimeException) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isModified(  ) throw (css::uno::RuntimeException) SAL_OVERRIDE;
        virtual void SAL_CALL notifyModifiedValue(  ) throw (css::uno::RuntimeException) SAL_OVERRIDE;

        // XComponent
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // IModifyListener
        virtual void modified() SAL_OVERRIDE
        { m_aImplControl.setModified(); }

        /// returns a typed pointer to our control window
              WindowType* getTypedControlWindow()       { return static_cast< WindowType* >      ( m_aImplControl.getVclControlWindow() ); }
        const WindowType* getTypedControlWindow() const { return static_cast< const WindowType* >( m_aImplControl.getVclControlWindow() ); }

    protected:
        /** checks whether the instance is already disposed
            @throws DisposedException
                if the instance is already disposed
        */
        inline void impl_checkDisposed_throw();
    };


    //= ControlWindow - implementation


    template< class WINDOW >
    inline void ControlWindow< WINDOW >::setControlHelper( ControlHelper& _rControlHelper )
    {
        m_pHelper = &_rControlHelper;
    }


    //= CommonBehaviourControl - implementation


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::CommonBehaviourControl ( sal_Int16 _nControlType, vcl::Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers )
        :ComponentBaseClass( m_aMutex )
        ,m_aImplControl( new WindowType( _pParentWindow, _nWindowStyle ), _nControlType, *this, this )
    {
        WindowType* pControlWindow( getTypedControlWindow() );
        pControlWindow->setControlHelper( m_aImplControl );
        if ( _bDoSetHandlers )
        {
            pControlWindow->SetModifyHdl( LINK( &m_aImplControl, ControlHelper, ModifiedHdl ) );
            pControlWindow->SetGetFocusHdl( LINK( &m_aImplControl, ControlHelper, GetFocusHdl ) );
            pControlWindow->SetLoseFocusHdl( LINK( &m_aImplControl, ControlHelper, LoseFocusHdl ) );
        }
        m_aImplControl.autoSizeWindow();
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline ::sal_Int16 SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlType() throw (css::uno::RuntimeException)
    {
        return m_aImplControl.getControlType();
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline css::uno::Reference< css::inspection::XPropertyControlContext > SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlContext() throw (css::uno::RuntimeException)
    {
        return m_aImplControl.getControlContext();
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::setControlContext( const css::uno::Reference< css::inspection::XPropertyControlContext >& _controlcontext ) throw (css::uno::RuntimeException)
    {
        m_aImplControl.setControlContext( _controlcontext );
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline css::uno::Reference< css::awt::XWindow > SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlWindow() throw (css::uno::RuntimeException)
    {
        return m_aImplControl.getControlWindow();
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline sal_Bool SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::isModified(  ) throw (css::uno::RuntimeException)
    {
        return m_aImplControl.isModified();
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::notifyModifiedValue(  ) throw (css::uno::RuntimeException)
    {
        m_aImplControl.notifyModifiedValue();
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::disposing()
    {
        m_aImplControl.dispose();
    }


    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::impl_checkDisposed_throw()
    {
        if ( ComponentBaseClass::rBHelper.bDisposed )
            throw css::lang::DisposedException( OUString(), *this );
    }


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_COMMONCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

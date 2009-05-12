/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commoncontrol.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_
#define _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_

/** === begin UNO includes === **/
#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/
#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <tools/link.hxx>
#include <vcl/window.hxx>

class NotifyEvent;
//............................................................................
namespace pcr
{
//............................................................................

    class ControlHelper;
    //========================================================================
    //= ControlWindow
    //========================================================================
    template< class WINDOW >
    class ControlWindow : public WINDOW
    {
    protected:
        typedef WINDOW  WindowType;

    protected:
        ControlHelper*  m_pHelper;

    public:
        ControlWindow( Window* _pParent, WinBits _nStyle )
            :WindowType( _pParent, _nStyle )
            ,m_pHelper( NULL )
        {
        }

        /// sets a ControlHelper instance which some functionality is delegated to
        inline virtual void setControlHelper( ControlHelper& _rControlHelper );

    protected:
        // Window overridables
        inline virtual long PreNotify( NotifyEvent& rNEvt );
    };

    //========================================================================
    //= IModifyListener
    //========================================================================
    class SAL_NO_VTABLE IModifyListener
    {
    public:
        virtual void modified() = 0;
    };

    //========================================================================
    //= ControlHelper
    //========================================================================
    /** A helper class for implementing the <type scope="com::sun::star::inspection">XPropertyControl</type>
        or one of its derived interfaces.

        This class is intended to be held as member of another class which implements the
        <type scope="com::sun::star::inspection">XPropertyControl</type> interface. The pointer
        to this interface is to be passed to the ctor.
    */
    class ControlHelper
    {
    private:
        Window*                         m_pControlWindow;
        sal_Int16                       m_nControlType;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext >
                                        m_xContext;
        ::com::sun::star::inspection::XPropertyControl&
                                        m_rAntiImpl;
        IModifyListener*                m_pModifyListener;
        sal_Bool                        m_bModified;

    public:
        /** creates the instance
            @param  _rControlWindow
                the window which is associated with the <type scope="com::sun::star::inspection">XPropertyControl</type>.
                Must not be <NULL/>.<br/>
                Ownership for this window is taken by the ControlHelper - it will be deleted in <member>disposing</member>.
            @param  _nControlType
                the type of the control - one of the <type scope="com::sun::star::inspection">PropertyControlType</type>
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
            Window* _pControlWindow,
            sal_Int16 _nControlType,
            ::com::sun::star::inspection::XPropertyControl& _rAntiImpl,
            IModifyListener* _pModifyListener );

        virtual ~ControlHelper();

        /** sets our "modified" flag to <TRUE/>
        */
        inline void setModified() { m_bModified = sal_True; }
        inline       Window* getVclControlWindow()       { return m_pControlWindow; }
        inline const Window* getVclControlWindow() const { return m_pControlWindow; }

    public:
        // XPropertyControl
        ::sal_Int16 SAL_CALL getControlType() throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext > SAL_CALL getControlContext() throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL setControlContext( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext >& _controlcontext ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getControlWindow() throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL notifyModifiedValue(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose();

        /** (fail-safe) wrapper around calling our context's activateNextControl
        */
        inline void activateNextControl() const { impl_activateNextControl_nothrow(); }

    public:
        /// may be used to implement the default handling in PreNotify; returns sal_True if handled
        bool handlePreNotify(NotifyEvent& _rNEvt);

        /// automatically size the window given in the ctor
        void    autoSizeWindow();

        /// may be used by derived classes, they forward the event to the PropCtrListener
        DECL_LINK( ModifiedHdl, Window* );
        DECL_LINK( GetFocusHdl, Window* );
        DECL_LINK( LoseFocusHdl, Window* );

    private:
        /** fail-safe wrapper around calling our context's activateNextControl
        */
        void    impl_activateNextControl_nothrow() const;
    };

    //========================================================================
    //= CommonBehaviourControl
    //========================================================================
    /** implements a base class for <type scope="com::sun::star::inspection">XPropertyControl</type>
        implementations, which delegates the generic functionality of this interface to a
        <type>ControlHelper</type> member.

        @param CONTROL_INTERFACE
            an interface class which is derived from (or identical to) <type scope="com::sun::star::inspection">XPropertyControl</type>
        @param CONTROL_WINDOW
            a class which is derived from ControlWindow
    */
    template < class CONTROL_INTERFACE, class CONTROL_WINDOW >
    class CommonBehaviourControl    :public ::comphelper::OBaseMutex
                                    ,public ::cppu::WeakComponentImplHelper1< CONTROL_INTERFACE >
                                    ,public IModifyListener
    {
    protected:
        typedef CONTROL_INTERFACE   InterfaceType;
        typedef CONTROL_WINDOW      WindowType;

        typedef ::comphelper::OBaseMutex                                MutexBaseClass;
        typedef ::cppu::WeakComponentImplHelper1< CONTROL_INTERFACE >   ComponentBaseClass;

    protected:
        ControlHelper   m_aImplControl;

    protected:
        inline CommonBehaviourControl( sal_Int16 _nControlType, Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers = true );

        // XPropertyControl - delegated to ->m_aImplControl
        inline ::sal_Int16 SAL_CALL getControlType() throw (::com::sun::star::uno::RuntimeException);
        inline ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext > SAL_CALL getControlContext() throw (::com::sun::star::uno::RuntimeException);
        inline void SAL_CALL setControlContext( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext >& _controlcontext ) throw (::com::sun::star::uno::RuntimeException);
        inline ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getControlWindow() throw (::com::sun::star::uno::RuntimeException);
        inline ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
        inline void SAL_CALL notifyModifiedValue(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        inline virtual void SAL_CALL disposing();

        // IModifyListener
        inline virtual void modified();

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

    //========================================================================
    //= ControlWindow - implementation
    //========================================================================
    //------------------------------------------------------------------------
    template< class WINDOW >
    inline void ControlWindow< WINDOW >::setControlHelper( ControlHelper& _rControlHelper )
    {
        m_pHelper = &_rControlHelper;
    }

    //------------------------------------------------------------------------
    template< class WINDOW >
    inline long ControlWindow< WINDOW >::PreNotify( NotifyEvent& rNEvt )
    {
        if ( m_pHelper && m_pHelper->handlePreNotify( rNEvt ) )
            return 1;
        return WindowType::PreNotify( rNEvt );
    }

    //========================================================================
    //= CommonBehaviourControl - implementation
    //========================================================================
    //------------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::CommonBehaviourControl ( sal_Int16 _nControlType, Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers )
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

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline ::sal_Int16 SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlType() throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.getControlType();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext > SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlContext() throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.getControlContext();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::setControlContext( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext >& _controlcontext ) throw (::com::sun::star::uno::RuntimeException)
    {
        m_aImplControl.setControlContext( _controlcontext );
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlWindow() throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.getControlWindow();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline ::sal_Bool SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::isModified(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.isModified();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::notifyModifiedValue(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        m_aImplControl.notifyModifiedValue();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::disposing()
    {
        m_aImplControl.dispose();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::modified()
    {
        m_aImplControl.setModified();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    inline void CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::impl_checkDisposed_throw()
    {
        if ( ComponentBaseClass::rBHelper.bDisposed )
            throw ::com::sun::star::lang::DisposedException( ::rtl::OUString(), *this );
    }

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_


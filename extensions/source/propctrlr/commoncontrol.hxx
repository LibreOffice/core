/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commoncontrol.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:54:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_
#define _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYCONTROL_HPP_
#include <com/sun/star/inspection/XPropertyControl.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
/** === end UNO includes === **/
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#pragma warning (disable:4505)
    // "unreferenced local function has been removed"

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
        virtual void setControlHelper( ControlHelper& _rControlHelper );

    protected:
        // Window overridables
        virtual long PreNotify( NotifyEvent& rNEvt );
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
        CommonBehaviourControl( sal_Int16 _nControlType, Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers = true );

        // XPropertyControl - delegated to ->m_aImplControl
        ::sal_Int16 SAL_CALL getControlType() throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext > SAL_CALL getControlContext() throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL setControlContext( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext >& _controlcontext ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getControlWindow() throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL notifyModifiedValue(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL disposing();

        // IModifyListener
        virtual void modified();

        /// returns a typed pointer to our control window
              WindowType* getTypedControlWindow()       { return static_cast< WindowType* >      ( m_aImplControl.getVclControlWindow() ); }
        const WindowType* getTypedControlWindow() const { return static_cast< const WindowType* >( m_aImplControl.getVclControlWindow() ); }

    protected:
        /** checks whether the instance is already disposed
            @throws DisposedException
                if the instance is already disposed
        */
        void impl_checkDisposed_throw();
    };

    //========================================================================
    //= ControlWindow - implementation
    //========================================================================
    //------------------------------------------------------------------------
    template< class WINDOW >
    void ControlWindow< WINDOW >::setControlHelper( ControlHelper& _rControlHelper )
    {
        m_pHelper = &_rControlHelper;
    }

    //------------------------------------------------------------------------
    template< class WINDOW >
    long ControlWindow< WINDOW >::PreNotify( NotifyEvent& rNEvt )
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
    CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::CommonBehaviourControl ( sal_Int16 _nControlType, Window* _pParentWindow, WinBits _nWindowStyle, bool _bDoSetHandlers )
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
    ::sal_Int16 SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlType() throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.getControlType();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext > SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlContext() throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.getControlContext();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::setControlContext( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlContext >& _controlcontext ) throw (::com::sun::star::uno::RuntimeException)
    {
        m_aImplControl.setControlContext( _controlcontext );
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::getControlWindow() throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.getControlWindow();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    ::sal_Bool SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::isModified(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return m_aImplControl.isModified();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::notifyModifiedValue(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        m_aImplControl.notifyModifiedValue();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    void SAL_CALL CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::disposing()
    {
        m_aImplControl.dispose();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    void CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::modified()
    {
        m_aImplControl.setModified();
    }

    //--------------------------------------------------------------------
    template< class CONTROL_INTERFACE, class CONTROL_WINDOW >
    void CommonBehaviourControl< CONTROL_INTERFACE, CONTROL_WINDOW >::impl_checkDisposed_throw()
    {
        if ( ComponentBaseClass::rBHelper.bDisposed )
            throw ::com::sun::star::lang::DisposedException( ::rtl::OUString(), *this );
    }

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_


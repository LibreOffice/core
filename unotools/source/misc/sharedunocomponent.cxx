/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharedunocomponent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 11:45:06 $
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

#ifndef UNOTOOLS_INC_SHAREDUNOCOMPONENT_HXX
#include <unotools/sharedunocomponent.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//............................................................................
namespace utl
{
//............................................................................

    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::XCloseListener;
    using ::com::sun::star::util::CloseVetoException;

    //========================================================================
    //= DisposableComponent
    //========================================================================
    //------------------------------------------------------------------------
    DisposableComponent::DisposableComponent( const Reference< XInterface >& _rxComponent )
        :m_xComponent( _rxComponent, UNO_QUERY )
    {
        DBG_ASSERT( m_xComponent.is(), "DisposableComponent::DisposableComponent: should be an XComponent!" );
    }

    //------------------------------------------------------------------------
    DisposableComponent::~DisposableComponent()
    {
        if ( m_xComponent.is() )
        {
            try
            {
                m_xComponent->dispose();
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "DisposableComponent::~DisposableComponent: caught an exception!" );
            }
            m_xComponent.clear();
        }
    }

    //========================================================================
    //= CloseableComponentImpl
    //========================================================================
    DBG_NAME( CloseableComponentImpl )
    typedef ::cppu::WeakImplHelper1 <   XCloseListener
                                    >   CloseableComponentImpl_Base;
    class CloseableComponentImpl : public CloseableComponentImpl_Base
    {
    private:
        Reference< XCloseable > m_xCloseable;

    public:
        CloseableComponentImpl( const Reference< XInterface >& _rxComponent );

        /** closes the component

            @nofail
        */
        void    nf_closeComponent();

    protected:
        virtual ~CloseableComponentImpl();

        // XCloseListener overridables
        virtual void SAL_CALL queryClosing( const EventObject& Source, ::sal_Bool GetsOwnership ) throw (CloseVetoException, RuntimeException);
        virtual void SAL_CALL notifyClosing( const EventObject& Source ) throw (RuntimeException);

        // XEventListener overridables
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** starts or stops being a CloseListener at the component

            Only to be called upon construction of the instance, or when the component
            is to be closed.

        @nofail
        */
        void    impl_nf_switchListening( bool _bListen );


    private:
        CloseableComponentImpl();                                           // never implemented
        CloseableComponentImpl( const CloseableComponentImpl& );            // never implemented
        CloseableComponentImpl& operator=( const CloseableComponentImpl& ); // never implemented
    };

    //------------------------------------------------------------------------
    CloseableComponentImpl::CloseableComponentImpl( const Reference< XInterface >& _rxComponent )
        :m_xCloseable( _rxComponent, UNO_QUERY )
    {
        DBG_CTOR( CloseableComponentImpl, NULL );
        DBG_ASSERT( m_xCloseable.is() || !_rxComponent.is(), "CloseableComponentImpl::CloseableComponentImpl: component is not an XCloseable!" );
        impl_nf_switchListening( true );
    }
    //------------------------------------------------------------------------
    CloseableComponentImpl::~CloseableComponentImpl()
    {
        nf_closeComponent();
        DBG_DTOR( CloseableComponentImpl, NULL );
    }

    //------------------------------------------------------------------------
    void CloseableComponentImpl::nf_closeComponent()
    {
        if ( !m_xCloseable.is() )
            // nothing to do
            return;

        // stop listening
        impl_nf_switchListening( false );

        // close
        try
        {
            m_xCloseable->close( sal_True );
        }
        catch( const CloseVetoException& ) { /* fine */ }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "CloseableComponentImpl::nf_closeComponent: caught an unexpected exception!" );
        }

        // reset
        m_xCloseable.clear();
    }

    //------------------------------------------------------------------------
    void CloseableComponentImpl::impl_nf_switchListening( bool _bListen )
    {
        if ( !m_xCloseable.is() )
            return;

        try
        {
            if ( _bListen )
                m_xCloseable->addCloseListener( this );
            else
                m_xCloseable->removeCloseListener( this );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "CloseableComponentImpl::impl_nf_switchListening: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL CloseableComponentImpl::queryClosing( const EventObject& Source, ::sal_Bool GetsOwnership ) throw (CloseVetoException, RuntimeException)
    {
        // as long as we live, somebody wants to keep the object alive. So, veto the
        // closing
        DBG_ASSERT( Source.Source == m_xCloseable, "CloseableComponentImpl::queryClosing: where did this come from?" );
        throw CloseVetoException();
    }

    //--------------------------------------------------------------------
    void SAL_CALL CloseableComponentImpl::notifyClosing( const EventObject& Source ) throw (RuntimeException)
    {
        DBG_ASSERT( Source.Source == m_xCloseable, "CloseableComponentImpl::notifyClosing: where did this come from?" );

        // this should be unreachable: As long as we're a CloseListener, we veto the closing. If we're going
        // to close the component ourself, then we revoke ourself as listener *before* the close call. So,
        // if this here fires, something went definately wrong.
        DBG_ERROR( "CloseableComponentImpl::notifyClosing: unreachable!" );
    }

    //--------------------------------------------------------------------
    void SAL_CALL CloseableComponentImpl::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        DBG_ASSERT( Source.Source == m_xCloseable, "CloseableComponentImpl::disposing: where did this come from?" );
        DBG_ERROR( "CloseableComponentImpl::disposing: unreachable!" );
            // same reasoning for this assertion as in ->notifyClosing
    }

    //========================================================================
    //= CloseableComponentImpl
    //========================================================================
    DBG_NAME( CloseableComponent )
    //------------------------------------------------------------------------
    CloseableComponent::CloseableComponent( const Reference< XInterface >& _rxComponent )
        :m_pImpl( new CloseableComponentImpl( _rxComponent ) )
    {
        DBG_CTOR( CloseableComponent, NULL );
    }

    //------------------------------------------------------------------------
    CloseableComponent::~CloseableComponent()
    {
        // close the component, deliver ownership to anybody who wants to veto the close
        m_pImpl->nf_closeComponent();
        DBG_DTOR( CloseableComponent, NULL );
    }

//............................................................................
}   // namespace utl
//............................................................................

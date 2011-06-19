/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
#include <unotools/eventlisteneradapter.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>

//.........................................................................
namespace utl
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= OEventListenerImpl
    //=====================================================================
    class OEventListenerImpl : public ::cppu::WeakImplHelper1< XEventListener >
    {
    protected:
        OEventListenerAdapter*          m_pAdapter;
        Reference< XEventListener >     m_xKeepMeAlive;
            // imagine an implementation of XComponent which holds it's listeners with a weak reference ...
            // would be very bad if we don't hold ourself
        Reference< XComponent >         m_xComponent;

    public:
        OEventListenerImpl( OEventListenerAdapter* _pAdapter, const Reference< XComponent >& _rxComp );

        void                            dispose();
        const Reference< XComponent >&  getComponent() const { return m_xComponent; }

    protected:
        virtual void SAL_CALL disposing( const EventObject& _rSource ) throw (RuntimeException);
    };

    //---------------------------------------------------------------------
    OEventListenerImpl::OEventListenerImpl( OEventListenerAdapter* _pAdapter, const Reference< XComponent >& _rxComp )
        :m_pAdapter(_pAdapter)
    {
        OSL_ENSURE(m_pAdapter, "OEventListenerImpl::OEventListenerImpl: invalid adapter!");
        // no checks of _rxComp !!
        // (OEventListenerAdapter is responsible for this)

        // just in case addEventListener throws an exception ... don't initialize m_xKeepMeAlive before this
        // is done
        Reference< XEventListener > xMeMyselfAndI = this;
        _rxComp->addEventListener(xMeMyselfAndI);

        m_xComponent = _rxComp;
        m_xKeepMeAlive = xMeMyselfAndI;
    }

    //---------------------------------------------------------------------
    void OEventListenerImpl::dispose()
    {
        if (m_xComponent.is())
        {
            m_xComponent->removeEventListener(m_xKeepMeAlive);
            m_xComponent.clear();
            m_xKeepMeAlive.clear();
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OEventListenerImpl::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        Reference< XEventListener > xDeleteUponLeaving = m_xKeepMeAlive;
        m_xKeepMeAlive.clear();
        m_xComponent.clear();

        m_pAdapter->_disposing(_rSource);
    }

    //=====================================================================
    //= OEventListenerAdapterImpl
    //=====================================================================
    struct OEventListenerAdapterImpl
    {
    public:
        ::std::vector< void* >  aListeners;
    };

    //=====================================================================
    //= OEventListenerAdapter
    //=====================================================================
    //---------------------------------------------------------------------
    OEventListenerAdapter::OEventListenerAdapter()
        :m_pImpl(new OEventListenerAdapterImpl)
    {
    }

    //---------------------------------------------------------------------
    OEventListenerAdapter::~OEventListenerAdapter()
    {
        stopAllComponentListening( );
        delete m_pImpl;
        m_pImpl = NULL;
    }

    //---------------------------------------------------------------------
    void OEventListenerAdapter::stopComponentListening( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComp )
    {
        if ( m_pImpl->aListeners.empty() )
            return;

        ::std::vector< void* >::iterator dispose = m_pImpl->aListeners.begin();
        do
        {
            OEventListenerImpl* pListenerImpl = static_cast< OEventListenerImpl* >( *dispose );
            if ( pListenerImpl->getComponent().get() == _rxComp.get() )
            {
                pListenerImpl->dispose();
                pListenerImpl->release();
                dispose = m_pImpl->aListeners.erase( dispose );
            }
            else
                ++dispose;
        }
        while ( dispose != m_pImpl->aListeners.end() );
    }

    //---------------------------------------------------------------------
    void OEventListenerAdapter::stopAllComponentListening(  )
    {
        for (   ::std::vector< void* >::const_iterator aDisposeLoop = m_pImpl->aListeners.begin();
                aDisposeLoop != m_pImpl->aListeners.end();
                ++aDisposeLoop
            )
        {
            OEventListenerImpl* pListenerImpl = static_cast< OEventListenerImpl* >(*aDisposeLoop);
            pListenerImpl->dispose();
            pListenerImpl->release();
        }
        m_pImpl->aListeners.clear();
    }

    //---------------------------------------------------------------------
    void OEventListenerAdapter::startComponentListening( const Reference< XComponent >& _rxComp )
    {
        if (!_rxComp.is())
        {
            OSL_FAIL("OEventListenerAdapter::startComponentListening: invalid component!");
            return;
        }

        OEventListenerImpl* pListenerImpl = new OEventListenerImpl(this, _rxComp);
        pListenerImpl->acquire();
        m_pImpl->aListeners.push_back(pListenerImpl);
    }

//.........................................................................
}   // namespace utl
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

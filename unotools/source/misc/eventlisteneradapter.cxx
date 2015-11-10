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

#include <sal/config.h>

#include <vector>

#include <unotools/eventlisteneradapter.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>

namespace utl
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //= OEventListenerImpl

    class OEventListenerImpl : public ::cppu::WeakImplHelper< XEventListener >
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
        virtual void SAL_CALL disposing( const EventObject& _rSource ) throw (RuntimeException, std::exception) override;
    };

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

    void OEventListenerImpl::dispose()
    {
        if (m_xComponent.is())
        {
            m_xComponent->removeEventListener(m_xKeepMeAlive);
            m_xComponent.clear();
            m_xKeepMeAlive.clear();
        }
    }

    void SAL_CALL OEventListenerImpl::disposing( const EventObject& _rSource ) throw (RuntimeException, std::exception)
    {
        Reference< XEventListener > xDeleteUponLeaving = m_xKeepMeAlive;
        m_xKeepMeAlive.clear();
        m_xComponent.clear();

        m_pAdapter->_disposing(_rSource);
    }

    //= OEventListenerAdapterImpl

    struct OEventListenerAdapterImpl
    {
    public:
        ::std::vector< void* >  aListeners;
    };

    //= OEventListenerAdapter

    OEventListenerAdapter::OEventListenerAdapter()
        :m_pImpl(new OEventListenerAdapterImpl)
    {
    }

    OEventListenerAdapter::~OEventListenerAdapter()
    {
        stopAllComponentListening( );
        delete m_pImpl;
        m_pImpl = nullptr;
    }

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

}   // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

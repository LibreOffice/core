/*************************************************************************
 *
 *  $RCSfile: eventlisteneradapter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-05 09:05:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

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
            OSL_ENSURE(sal_False, "OEventListenerAdapter::startComponentListening: invalid component!");
            return;
        }

        OEventListenerImpl* pListenerImpl = new OEventListenerImpl(this, _rxComp);
        pListenerImpl->acquire();
        m_pImpl->aListeners.push_back(pListenerImpl);
    }

//.........................................................................
}   // namespace utl
//.........................................................................

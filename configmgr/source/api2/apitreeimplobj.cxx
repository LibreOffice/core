/*************************************************************************
 *
 *  $RCSfile: apitreeimplobj.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "apitreeimplobj.hxx"
#include "confproviderimpl2.hxx"
#include "confignotifier.hxx"
#include "notifierimpl.hxx"
#include "apifactory.hxx"
#include "apitreeaccess.hxx"

#include <cppuhelper/queryinterface.hxx>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        class Factory;
        class Notifier;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// API object implementation wrappers
//-------------------------------------------------------------------------
configuration::TemplateProvider ApiProvider::getTemplateProvider() const
{
    return TemplateProvider(&m_rProviderImpl);
}
//-------------------------------------------------------------------------

ISynchronizedData* ApiProvider::getSourceLock() const
{
    return &m_rProviderImpl;
}
//-------------------------------------------------------------------------

ApiTreeImpl::ApiTreeImpl(UnoInterface* pInstance, Tree const& aTree, ApiTreeImpl& rParentTree)
: m_pInstance(pInstance)
, m_aTree(aTree)
, m_rProvider(rParentTree.getProvider())
, m_pParentTree(0)
, m_aNotifier(new NotifierImpl())
{
    init(&rParentTree);
}
//-------------------------------------------------------------------------

ApiTreeImpl::ApiTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, Tree const& aTree, ApiTreeImpl* pParentTree)
: m_pInstance(pInstance)
, m_aTree(aTree)
, m_rProvider(rProvider)
, m_pParentTree(0)
, m_aNotifier(new NotifierImpl())
{
    init(pParentTree);
}
//-------------------------------------------------------------------------

ApiTreeImpl::~ApiTreeImpl()
{
    OSL_ENSURE(m_aNotifier.getBody().m_aListeners.isDisposed(),"ApiTree Object was not disposed properly");
    setParentTree(0);
    uno::Reference<com::sun::star::lang::XComponent> xProvider(getProviderComponent());
    if (xProvider.is())
    {
        xProvider->removeEventListener(this);
        xProvider->release();
    }
}

void ApiTreeImpl::checkAlive() const
{
    bool bAlive = m_aNotifier->m_aListeners.checkAlive();
    if (!bAlive)
    {
        OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("CONFIGURATION: Object was disposed"));
        throw com::sun::star::lang::DisposedException(sMessage, getUnoInstance());
    }
}

osl::Mutex& ApiTreeImpl::getApiLock() const
{
    return m_aNotifier->mutex();
}

Notifier ApiTreeImpl::getNotifier() const
{
    return Notifier(m_aNotifier,m_aTree);
}

bool ApiTreeImpl::disposeTree(bool bForce)
{
    if (m_pParentTree != 0)
        return false;

    uno::Reference<com::sun::star::lang::XComponent> xProvider(getProviderComponent());
    if (xProvider.is()) xProvider->removeEventListener(this);

    m_aNotifier->dispose( com::sun::star::lang::EventObject(getUnoInstance()) );
    return true;
}
void ApiTreeImpl::disposeNode(NodeRef const& aNode, UnoInterface* pInstance)
{
    getNotifier().disposeNode(aNode, com::sun::star::lang::EventObject(pInstance));
}
void ApiTreeImpl::init(ApiTreeImpl* pParentTree)
{
    uno::Reference<com::sun::star::lang::XComponent> m_xProvider = getProviderComponent();
    if (m_xProvider.is())
    {
        m_xProvider->addEventListener(this);
        m_xProvider->acquire();
    }
    OSL_ASSERT(m_pParentTree == 0);
    setParentTree(pParentTree);
}

void ApiTreeImpl::setParentTree(ApiTreeImpl*    pParentTree)
{
    osl::MutexGuard aLock(getApiLock());

    if (m_pParentTree != pParentTree)
    {
        uno::Reference<com::sun::star::lang::XComponent> xOld = getParentComponent();
        m_pParentTree = pParentTree;
        uno::Reference<com::sun::star::lang::XComponent> xNew = getParentComponent();

        if (xNew.is())
        {
            xNew->addEventListener(this);
            xNew->acquire();
        }
        if (xOld.is())
        {
            xOld->removeEventListener(this);
            xOld->release();
        }
    }
}


uno::Reference<com::sun::star::lang::XComponent> ApiTreeImpl::getParentComponent()
{
    uno::XInterface* pInterface = m_pParentTree ? m_pParentTree->getUnoInstance() : 0;
    return uno::Reference<com::sun::star::lang::XComponent>::query(pInterface);
}

uno::Reference<com::sun::star::lang::XComponent> ApiTreeImpl::getProviderComponent()
{
    uno::XInterface* pInterface = m_rProvider.getProviderImpl().getProviderInstance();
    return uno::Reference<com::sun::star::lang::XComponent>::query(pInterface);
}

void SAL_CALL ApiTreeImpl::acquire() throw()
{
    // TODO add debug counting
}
void SAL_CALL ApiTreeImpl::release() throw()
{
    // TODO add debug counting
}
uno::Any SAL_CALL ApiTreeImpl::queryInterface(uno::Type const& rType) throw()
{
    return cppu::queryInterface( rType
                , static_cast< com::sun::star::lang::XEventListener*>(this)
                , static_cast< uno::XInterface*>(this)
            );
}

void SAL_CALL ApiTreeImpl::disposing(com::sun::star::lang::EventObject const& rEvt) throw()
{
    setParentTree(0);
    disposeTree(true);

    // uno::Reference<com::sun::star::lang::XComponent> xThis(getUnoInstance(),UNO_QUERY);
    // if (xThis.is()) xThis->dispose();
}

//-----------------------------------------------------------------------------
    }
}


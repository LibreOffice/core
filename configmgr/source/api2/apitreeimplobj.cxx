/*************************************************************************
 *
 *  $RCSfile: apitreeimplobj.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:29 $
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
#include <stdio.h>

#include "apitreeimplobj.hxx"

#ifndef CONFIGMGR_API_PROVIDERIMPL2_HXX_
#include "confproviderimpl2.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNOTIFIER_HXX_
#include "confignotifier.hxx"
#endif
#ifndef CONFIGMGR_API_NOTIFIERIMPL_HXX_
#include "notifierimpl.hxx"
#endif
#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "apifactory.hxx"
#endif
#ifndef CONFIGMGR_API_TREEACCESS_HXX_
#include "apitreeaccess.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#include "nodechangeinfo.hxx"
#endif
#ifndef CONFIGMGR_API_BROADCASTER_HXX_
#include "broadcaster.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_ROOTTREE_HXX_
#include "roottree.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGANYNODE_HXX_
#include "anynoderef.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        namespace lang = ::com::sun::star::lang;
//-----------------------------------------------------------------------------
        class Factory;
        class Notifier;
//-----------------------------------------------------------------------------
class ApiTreeImpl::ComponentAdapter : public lang::XEventListener
{
    osl::Mutex mutex;
    vos::ORefCount m_refs;

    ApiTreeImpl* pOwner;

    uno::Reference< lang::XComponent > xProvider;
    uno::Reference< lang::XComponent > xParent;
public:
    ComponentAdapter(ApiTreeImpl& rParent) : pOwner(&rParent) {}

    void clear();

    void setProvider(uno::Reference< lang::XComponent > const& xProvider);
    void setParent(uno::Reference< lang::XComponent > const& xParent);
    uno::Reference< lang::XComponent > getProvider() const;
    uno::Reference< lang::XComponent > getParent() const;

// XEventListener
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();
private:
    void setComponent(uno::Reference< lang::XComponent >& rxSlot,uno::Reference< lang::XComponent > const& xComp);
    uno::Reference< lang::XComponent > getComponent(uno::Reference< lang::XComponent > const& rxSlot) const;

    virtual uno::Any SAL_CALL queryInterface(uno::Type const& rType) throw();
    virtual void SAL_CALL disposing(com::sun::star::lang::EventObject const& rEvt) throw();
};
//-----------------------------------------------------------------------------
inline
uno::Reference< lang::XComponent >
    ApiTreeImpl::ComponentAdapter::getComponent(
        uno::Reference< lang::XComponent > const& rxSlot
    ) const
{
    return rxSlot;
}
//-----------------------------------------------------------------------------
inline
void ApiTreeImpl::ComponentAdapter::setComponent(
        uno::Reference< lang::XComponent >& rxSlot,
        uno::Reference< lang::XComponent > const& xComp
    )
{
    osl::ClearableMutexGuard aGuard(mutex);

    uno::Reference< lang::XComponent > xOld = rxSlot;
    if (xOld != xComp)
    {
        rxSlot = xComp;

        aGuard.clear();

        if (xOld.is())  try { xOld->removeEventListener(this); } catch (uno::Exception & ) {}
        if (xComp.is()) xComp->addEventListener(this);
    }
}
//-----------------------------------------------------------------------------
uno::Reference< lang::XComponent > ApiTreeImpl::ComponentAdapter::getProvider() const
{
    return this->getComponent( this->xProvider );
}
uno::Reference< lang::XComponent > ApiTreeImpl::ComponentAdapter::getParent() const
{
    return this->getComponent( this->xParent );
}
void ApiTreeImpl::ComponentAdapter::setProvider(uno::Reference< lang::XComponent > const& xProvider)
{
    this->setComponent( this->xProvider, xProvider);
}
void ApiTreeImpl::ComponentAdapter::setParent(uno::Reference< lang::XComponent > const& xParent)
{
    this->setComponent( this->xParent, xParent);
}
//-----------------------------------------------------------------------------

void SAL_CALL ApiTreeImpl::ComponentAdapter::acquire() throw()
{
    ++m_refs;
}
//-------------------------------------------------------------------------

void SAL_CALL ApiTreeImpl::ComponentAdapter::release() throw()
{
    if (--m_refs == 0)
        delete this;
}
//-------------------------------------------------------------------------

uno::Any SAL_CALL ApiTreeImpl::ComponentAdapter::queryInterface(uno::Type const& rType) throw()
{
    return cppu::queryInterface( rType
                , static_cast< com::sun::star::lang::XEventListener*>(this)
                , static_cast< uno::XInterface*>(this)
            );
}
//-------------------------------------------------------------------------

void SAL_CALL ApiTreeImpl::ComponentAdapter::disposing(com::sun::star::lang::EventObject const& rEvt) throw()
{
    osl::ClearableMutexGuard aGuard(mutex);
    if (this->pOwner != NULL)
    {
        CFG_TRACE_INFO("ApiTreeImpl:ComponentAdapter: Providing UNO object is disposed - relaying to my owner");
        UnoInterfaceRef xKeepAlive( this->pOwner->getUnoInstance() );

        aGuard.clear();

        pOwner->disposing( rEvt );

        osl::MutexGuard aClearGuard(mutex);
        if (rEvt.Source == this->xParent) this->xParent.clear();
        if (rEvt.Source == this->xProvider) this->xParent.clear();
    }
    else
        CFG_TRACE_INFO("ApiTreeImpl:ComponentAdapter: Providing UNO object is disposed - but my owner is already gone");
}

//-------------------------------------------------------------------------

void ApiTreeImpl::ComponentAdapter::clear()
{
    osl::ClearableMutexGuard aGuard(mutex);

    this->pOwner = 0;

    uno::Reference< lang::XComponent > xProvider = this->xProvider;
    uno::Reference< lang::XComponent > xParent  = this->xParent;
    this->xProvider = 0;
    this->xParent = 0;

    aGuard.clear();

    if (xParent.is())   try { xParent  ->removeEventListener(this); } catch (uno::Exception & ) {}
    if (xProvider.is()) try { xProvider->removeEventListener(this); } catch (uno::Exception & ) {}
}

//-----------------------------------------------------------------------------
class ApiRootTreeImpl::NodeListener : public INodeListener
{
    osl::Mutex mutex;
    ApiRootTreeImpl*    pParent;
    IConfigBroadcaster* pSource;

    TreeOptions           m_xOptions;
    AbsolutePath          m_aLocationPath;
public:
    NodeListener(ApiRootTreeImpl& _rParent)
        : pParent(&_rParent)
        , pSource(NULL)
        , m_aLocationPath( AbsolutePath::root() )
    {}
    ~NodeListener()
    {
        unbind();
    }

    IConfigBroadcaster* getSource()
    {
        osl::MutexGuard aGuard(mutex);
        return pSource;
    }

    void setSource(IConfigBroadcaster* pNew)
    {
        osl::MutexGuard aGuard(mutex);
        if (pParent)
        {
            if (pNew != pSource)
            {
                OSL_ENSURE(m_xOptions.isValid(),"Cannot set IConfigListener without Options");
                if (m_xOptions.isValid())
                {
                    if (pSource)
                        pSource->removeListener(m_xOptions->getRequestOptions(), this);

                    pSource = pNew;
                    if (pNew)
                    {
                        OSL_ENSURE(!m_aLocationPath.isRoot(), "Cannot register for notifications: no location set");
                        pNew->addListener(m_aLocationPath, m_xOptions->getRequestOptions(), this);
                    }
                }
                else
                    pSource = 0;
            }
        }
    }

    void setLocation(AbsolutePath const& _aLocation, TreeOptions const& _xOptions)
    {
        OSL_ASSERT(_xOptions.isValid());

        osl::MutexGuard aGuard(mutex);

        if (pSource && pParent)
        {
            OSL_ASSERT(m_xOptions.isValid());
            pSource->removeListener(m_xOptions->getRequestOptions(), this);
        }

        m_aLocationPath = _aLocation;
        m_xOptions = _xOptions;

        if (pSource && pParent)
            pSource->addListener(m_aLocationPath, m_xOptions->getRequestOptions(), this);
    }

    void unbind()
    {
        osl::MutexGuard aGuard(mutex);
        OSL_ASSERT(pParent == 0);
        pParent = 0;
        if (pSource)
        {
            OSL_ASSERT(m_xOptions.isValid());
            pSource->removeListener(m_xOptions->getRequestOptions(), this);
            m_xOptions.unbind();
            m_aLocationPath = AbsolutePath::root();
        }

    }

    void clearParent()
    {
        osl::ClearableMutexGuard aGuard(mutex);
        if (pParent)
        {
            pParent = 0;

            if (pSource)
            {
                IConfigBroadcaster* pOrgSource = pSource;
                TreeOptions xOptions = m_xOptions;

                pSource = 0;
                m_xOptions.unbind();
                m_aLocationPath = AbsolutePath::root();

                aGuard.clear();

                OSL_ASSERT(xOptions.isValid());
                pOrgSource->removeListener(xOptions->getRequestOptions(), this);
            }
        }
    }

    // Interfaces
    virtual void disposing(IConfigBroadcaster* pSource);
    virtual void nodeChanged(data::Accessor const& _aChangedDataAccessor, Change const& aChange, AbsolutePath const& sPath, IConfigBroadcaster* pSource);
    virtual void nodeDeleted(data::Accessor const& _aChangedDataAccessor, AbsolutePath const& sPath, IConfigBroadcaster* pSource);
};

//-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// API object implementation wrappers
//-------------------------------------------------------------------------
ApiProvider::ApiProvider(Factory& rFactory, OProviderImpl& rProviderImpl )
    : m_rFactory(rFactory)
    , m_rProviderImpl(rProviderImpl)
{}
//-------------------------------------------------------------------------

UnoTypeConverter ApiProvider::getTypeConverter() const
{
    return m_rProviderImpl.getTypeConverter();
}
//-------------------------------------------------------------------------
static
inline
configuration::DefaultProvider createDefaultProvider(
                                    ApiProvider& rProvider,
                                    configuration::Tree const& aTree,
                                    TreeOptions const& _xOptions
                               )
{
    OProviderImpl& rProviderImpl        = rProvider.getProviderImpl();
    rtl::Reference< IConfigDefaultProvider > xDefaultProvider  = rProviderImpl.getDefaultProvider();

    OSL_ASSERT(_xOptions.isValid());
    RequestOptions const aOptions = _xOptions.isValid() ? _xOptions->getRequestOptions() : RequestOptions();

    return configuration::DefaultProvider::create(aTree,aOptions,xDefaultProvider,&rProviderImpl);
}
//-------------------------------------------------------------------------
static
inline
configuration::DefaultProvider extractDefaultProvider(ApiTreeImpl* pParentTree)
{
    if (pParentTree)
        return pParentTree->getDefaultProvider();

    else
        return configuration::DefaultProvider::createEmpty();
}
//-------------------------------------------------------------------------
ApiTreeImpl::ApiTreeImpl(UnoInterface* pInstance, configuration::TreeRef const& aTree, ApiTreeImpl& rParentTree)
: m_pInstance(pInstance)
, m_aTree(aTree)
, m_aDefaultProvider(rParentTree.getDefaultProvider())
, m_rProvider(rParentTree.getProvider())
, m_pParentTree(0)
, m_aNotifier(new NotifierImpl(aTree))
{
    setNodeInstance(aTree.getRootNode(), pInstance);
    init(&rParentTree);
}
//-------------------------------------------------------------------------
ApiTreeImpl::ApiTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, configuration::TreeRef const& aTree, ApiTreeImpl* pParentTree)
: m_pInstance(pInstance)
, m_aTree(aTree)
, m_aDefaultProvider(extractDefaultProvider(pParentTree))
, m_rProvider(rProvider)
, m_pParentTree(0)
, m_aNotifier(new NotifierImpl(aTree))
{
    OSL_ENSURE(pParentTree == NULL || &rProvider == &pParentTree->m_rProvider,"WARNING: Parent tree has a different provider - trouble may be ahead");
    setNodeInstance(aTree.getRootNode(), pInstance);
    init(pParentTree);
}
//-------------------------------------------------------------------------
ApiTreeImpl::ApiTreeImpl(UnoInterface* _pInstance, ApiProvider& _rProvider, configuration::TreeRef const& _aTree, DefaultProvider const& _aDefaultProvider)
: m_pInstance(_pInstance)
, m_aTree(_aTree)
, m_aDefaultProvider(_aDefaultProvider)
, m_rProvider(_rProvider)
, m_pParentTree(0)
, m_aNotifier(new NotifierImpl(_aTree))
{
    setNodeInstance(_aTree.getRootNode(), _pInstance);
    init(NULL);
}
//-------------------------------------------------------------------------

ApiTreeImpl::~ApiTreeImpl()
{
    OSL_ENSURE(m_aNotifier->m_aListeners.isDisposed(),"ApiTree Object was not disposed properly");
    deinit();
}
//-------------------------------------------------------------------------

ApiRootTreeImpl::ApiRootTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, configuration::Tree const& aTree, TreeOptions const& _xOptions)
: m_aTreeImpl(pInstance, rProvider, aTree.getRef(), createDefaultProvider(rProvider, aTree, _xOptions))
, m_pNotificationListener(NULL)
, m_xOptions(_xOptions)
, m_aLocationPath( configuration::Path::Rep() )
{
    implSetLocation(aTree);
    enableNotification(true);
}
//-------------------------------------------------------------------------
ApiRootTreeImpl::~ApiRootTreeImpl()
{
    if (m_pNotificationListener.is())
    {
        m_pNotificationListener->setSource(0);
        m_pNotificationListener->clearParent();
    }
}
//-------------------------------------------------------------------------

void ApiTreeImpl::setNodeInstance(configuration::NodeRef const& aNode, UnoInterface* pInstance)
{
    OSL_ENSURE(aNode.isValid(),"ERROR: adding invalid node to ApiTree");
    OSL_ENSURE(m_aTree.isValidNode(aNode),"ERROR: foreign node being added to ApiTree");
    m_aNotifier->m_aListeners.setObjectAt( configuration::NodeID(m_aTree, aNode).toIndex(), pInstance );
}

//-------------------------------------------------------------------------

bool ApiTreeImpl::isAlive() const
{
    return m_aNotifier->m_aListeners.isAlive();
}
//-------------------------------------------------------------------------
void ApiTreeImpl::checkAlive() const
{
    m_aNotifier->m_aListeners.checkAlive( getUnoInstance() );
}
//-------------------------------------------------------------------------

osl::Mutex& ApiTreeImpl::getApiLock() const
{
    return m_aNotifier->mutex();
}
//-------------------------------------------------------------------------

Notifier ApiTreeImpl::getNotifier() const
{
    return Notifier(m_aNotifier,this);
}
//-------------------------------------------------------------------------

bool ApiRootTreeImpl::enableNotification(bool bEnable)
{
    IConfigBroadcaster* pSource = bEnable ? getApiTree().getProvider().getProviderImpl().getNotifier() : 0;

    IConfigBroadcaster* pOld = this->implSetNotificationSource(pSource);

    return pOld != 0;
}
//-------------------------------------------------------------------------

bool ApiTreeImpl::disposeTree(bool bForce)
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing Tree (may throw if already disposed)");

    // ensure our provider stays alive
    UnoInterfaceRef xKeepParentAlive(this->getParentComponent());

    data::Accessor aSourceAccessor( getSourceData() );

    osl::MutexGuard aLocalGuard(getDataLock());
    if (!bForce)
    {
        if (m_pParentTree != 0)
            return false;

        checkAlive(); // may throw
    }
    else if (m_pParentTree)
        setParentTree(NULL);

    implDisposeTree(aSourceAccessor); // TODO: accessor from lock
    OSL_ASSERT(!isAlive());

    return true;
}
//-------------------------------------------------------------------------

bool ApiTreeImpl::disposeTreeNow()
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing Tree Now (unless disposed)");
    if (isAlive() )
    {
        data::Accessor aSourceAccessor( getSourceData() );

        osl::MutexGuard aLocalGuard(getDataLock());

        return implDisposeTree(aSourceAccessor); // TODO: accessor from lock
    }
    else
        return false;
}
//-------------------------------------------------------------------------
bool ApiRootTreeImpl::disposeTree()
{
    CFG_TRACE_INFO("Api Root TreeImpl: Disposing Tree And Releasing (unless disposed)");
    // ensure our provider stays alive
    UnoInterfaceRef xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

    rtl::Reference<NodeListener> xListener = m_pNotificationListener;
    if (xListener.is())
    {
        xListener->clearParent();
        xListener.clear();
    }

    bool bDisposed = m_aTreeImpl.disposeTreeNow();

    if (bDisposed) releaseData();

    if (!m_xOptions.isEmpty())
    {
        OSL_ENSURE(!bDisposed, "Disposing/Releasing should clear the options");
        CFG_TRACE_INFO("Api Root TreeImpl: data was not released in disposeTree");
    }

    return bDisposed;
}
//-------------------------------------------------------------------------
bool ApiTreeImpl::implDisposeTree(data::Accessor const& _aAccessor)
{
    OSL_ENSURE(m_pParentTree == 0,"WARNING: Disposing a tree that still has a parent tree set");

    NotifierImpl::SpecialContainer& aContainer = m_aNotifier->m_aListeners;
    if (aContainer.beginDisposing())
    {
        CFG_TRACE_INFO("ApiTreeImpl: Tree is now disposed");
        using configuration::NodeIDList;
        using configuration::NodeID;
        using configuration::Tree;
        using configuration::getAllContainedNodes;
        using com::sun::star::lang::EventObject;

        Factory& rFactory = getFactory();

        NodeIDList aChildNodes;
        getAllContainedNodes( Tree(_aAccessor,m_aTree), aChildNodes);

        for (NodeIDList::reverse_iterator it = aChildNodes.rbegin(), stop = aChildNodes.rend();
            it != stop;
            ++it)
        {
            rFactory.revokeElement( *it );
        }

        CFG_TRACE_INFO_NI("ApiTreeImpl: Listeners are now informed");
        aContainer.notifyDisposing(_aAccessor);

        OSL_ASSERT(!aContainer.isDisposed());

        CFG_TRACE_INFO_NI("ApiTreeImpl: Deinitializing");
        deinit(); // releases the provider and parent
        aContainer.endDisposing();

        OSL_ASSERT(aContainer.isDisposed());

        return true;
    }
    else
    {
        CFG_TRACE_INFO("ApiTreeImpl: Tree was already disposed.");
        return false;
    }
}
//-------------------------------------------------------------------------
void ApiTreeImpl::disposeNode(NodeRef const& aNode, UnoInterface* pInstance)
{
    if (isAlive())
    {
        data::Accessor aSourceAccessor( getSourceData() );

        osl::MutexGuard aLocalGuard(getDataLock());
        if (isAlive())
            implDisposeNode(aSourceAccessor, aNode,pInstance);
    }
}
//-------------------------------------------------------------------------
void ApiTreeImpl::implDisposeNode(data::Accessor const & _anAccessor, NodeRef const& aNode, UnoInterface* pInstance)
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing a single node.");
    OSL_ENSURE(aNode.isValid(),"INTERNAL ERROR: Disposing NULL node");
    OSL_ENSURE(m_aTree.isValidNode(aNode),"INTERNAL ERROR: Disposing: node does not match tree");
    OSL_ENSURE( !m_aTree.isRootNode(aNode),"INTERNAL ERROR: Disposing the root node of the tree");

    using configuration::NodeID;
    using com::sun::star::lang::EventObject;

    NodeID aNodeID(m_aTree,aNode);

    if (m_aNotifier->m_aListeners.disposeOne(_anAccessor, aNodeID.toIndex()) )
    {
        getFactory().revokeElement(aNodeID);
    }
}
//-------------------------------------------------------------------------
void ApiTreeImpl::init(ApiTreeImpl* pParentTree)
{
    m_xProvider = new ComponentAdapter(*this);
    m_xProvider->setProvider( this->getProviderComponent() );

    OSL_ENSURE(m_xProvider->getProvider().is(),"WARNING: Provider is no Component - Lifetime trouble ahead");

    OSL_ASSERT(m_pParentTree == 0);
    setParentTree(pParentTree);
}
//-------------------------------------------------------------------------
void ApiTreeImpl::deinit()
{
    setParentTree(0);

    ComponentRef xAdapter = m_xProvider;
    m_xProvider.clear();

    if (xAdapter.is())
        xAdapter->clear();
}
//-------------------------------------------------------------------------
void ApiTreeImpl::haveNewParent(ApiTreeImpl* pNewParent) // public interface
{
    setParentTree(pNewParent);
}

//-------------------------------------------------------------------------

ApiTreeImpl const* ApiTreeImpl::getRootTreeImpl() const
{
    ApiTreeImpl const* pRet = this;
    while (pRet->m_pParentTree)
        pRet = pRet->m_pParentTree;

    return pRet;
}

//-------------------------------------------------------------------------
void ApiTreeImpl::setParentTree(ApiTreeImpl*    pParentTree) // internal implementation
{
    osl::MutexGuard aLock(getApiLock());

#ifdef _DEBUG
    if (pParentTree)
    {
        using configuration::NodeID;
        TreeRef aContext = m_aTree.getContextTree();
        TreeRef aParent = pParentTree->m_aTree;

        NodeID aContextID( aContext, aContext.getRootNode() );
        NodeID aParentID( aParent, aParent.getRootNode() );

        OSL_ENSURE( aContextID == aParentID, "Parent relationship mismatch !");
    }
#endif

    if (m_pParentTree != pParentTree)
    {
        ComponentRef xAdapter = m_xProvider;

        m_pParentTree = pParentTree;

        uno::Reference<com::sun::star::lang::XComponent> xNew = getParentComponent();
        OSL_ENSURE( xNew.is() == (pParentTree != 0), "WARNING: Parent Tree is no Component");

        if (xAdapter.is())
            xAdapter->setParent(xNew);
        else
            OSL_ENSURE( pParentTree == 0, "ERROR: Setting New Parent at deinitialized ApiTreeImpl");

    }
}
//-------------------------------------------------------------------------

UnoInterfaceRef ApiTreeImpl::getUnoProviderInstance() const
{
    ComponentRef xAdapter = m_xProvider;

    UnoInterfaceRef xReturn;
    if (xAdapter.is())
        xReturn = xAdapter->getProvider();
    return xReturn;
}

//-------------------------------------------------------------------------
uno::Reference<com::sun::star::lang::XComponent> ApiTreeImpl::getParentComponent()
{
    uno::XInterface* pInterface = m_pParentTree ? m_pParentTree->getUnoInstance() : 0;
    return uno::Reference<com::sun::star::lang::XComponent>::query(pInterface);
}
//-------------------------------------------------------------------------

uno::Reference<com::sun::star::lang::XComponent> ApiTreeImpl::getProviderComponent()
{
    uno::XInterface* pInterface = m_rProvider.getProviderImpl().getProviderInstance();
    return uno::Reference<com::sun::star::lang::XComponent>::query(pInterface);
}

//-------------------------------------------------------------------------

void ApiTreeImpl::disposing(com::sun::star::lang::EventObject const& rEvt) throw()
{
    // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
    CFG_TRACE_INFO("ApiTreeImpl: Providing UNO object is disposed - disposing the tree");

    // Tree write Lock should be set by sender

    CFG_TRACE_INFO_NI("Clearing parent reference");
    setParentTree(0);

    CFG_TRACE_INFO_NI("Trying to dispose");
    //implDisposeTree();
    disposeTreeNow();

    CFG_TRACE_INFO_NI("Done disposing Tree");
    // uno::Reference<com::sun::star::lang::XComponent> xThis(getUnoInstance(),UNO_QUERY);
    // if (xThis.is()) xThis->dispose();
}
//-------------------------------------------------------------------------
IConfigBroadcaster* ApiRootTreeImpl::implSetNotificationSource(IConfigBroadcaster* pNew)
{
    osl::MutexGuard aGuard(getApiTree().getApiLock());

    IConfigBroadcaster* pOld = m_pNotificationListener.is() ? m_pNotificationListener->getSource() : 0;
    if (pOld != pNew)
    {
        OSL_ENSURE(m_xOptions.isValid(), "Cannot change notification source without options");

        if (!m_pNotificationListener.is())
            m_pNotificationListener = new NodeListener(*this);

        m_pNotificationListener->setSource(pNew);

    }
    return pOld;
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::implSetLocation(configuration::Tree const& _aTree)
{
    osl::MutexGuard aGuard(getApiTree().getApiLock());

    OSL_ASSERT( configuration::equalTreeRef(_aTree.getRef(), getApiTree().getTree()) );
    if (!_aTree.isEmpty())
    {
        m_aLocationPath = _aTree.getRootPath();
        OSL_ENSURE(!m_aLocationPath.isRoot(), "Setting up a RootTree without location");
    }
    else
    {
        OSL_ENSURE(false, "Setting up a RootTree without data");
        m_aLocationPath = configuration::AbsolutePath::root();
    }

    if (!m_pNotificationListener.is())
        m_pNotificationListener = new NodeListener(*this);

    OSL_ENSURE(!m_aLocationPath.isRoot() && !m_aLocationPath.isDetached(), "Cannot reregister for notifications: setting empty location");
    OSL_ENSURE( m_xOptions.isValid(), "Cannot reregister for notifications: no options available" );

    m_pNotificationListener->setLocation(m_aLocationPath, m_xOptions);
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::releaseData()
{
    CFG_TRACE_INFO("Api Root TreeImpl at %s: releasing the Data",OUSTRING2ASCII(m_aLocationPath.toString()));
    configuration::TreeRef aTree( m_aTreeImpl.getTree() );

    aTree.disposeData();
    OSL_ASSERT(aTree.isEmpty());

    OSL_ENSURE( !m_aLocationPath.isRoot() && !m_aLocationPath.isDetached(), "Location still needed to release data" );
    OSL_ENSURE( m_xOptions.isValid(), "Options still needed to release data" );

    getApiTree().getProvider().getProviderImpl().releaseSubtree(m_aLocationPath,m_xOptions->getRequestOptions());
    m_xOptions.unbind();

    m_aLocationPath = configuration::AbsolutePath::detachedRoot();
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::NodeListener::disposing(IConfigBroadcaster* _pSource)
{
    osl::ClearableMutexGuard aGuard(mutex);

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        UnoInterfaceRef xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->disposing(_pSource);
    }
}
void ApiRootTreeImpl::disposing(IConfigBroadcaster* pSource)
{
    CFG_TRACE_INFO("Api Root TreeImpl at %s: Cache data is disposed - dispose and release own data",
                    OUSTRING2ASCII(m_aLocationPath.toString()));
        // ensure our provider stays alive
    UnoInterfaceRef xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

    rtl::Reference<NodeListener> xListener = m_pNotificationListener;
    if (xListener.is())
    {
        xListener->clearParent();
        xListener.clear();
    }

    if (m_aTreeImpl.disposeTreeNow())
        releaseData(); // not really needed: the whole data is going away anyways
}
// ---------------------------------------------------------------------------------------------------

static
void disposeOneRemovedNode(configuration::NodeChangeInformation const& aRemoveInfo, Factory& aFactory)
{
    if (aRemoveInfo.change.element.oldValue.is())
    {
        OSL_ENSURE(aRemoveInfo.change.element.isDataChange(), "ERROR: Disposing replaced element: Element did not really change !");

        configuration::ElementRef aElementRef( aRemoveInfo.change.element.oldValue.get() );

        SetElement* pSetElement = aFactory.findSetElement(aElementRef );
        if (pSetElement)
        {
            // factory always does an extra acquire
            UnoInterfaceRef xReleaseSetElement(pSetElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);

            pSetElement->haveNewParent(0);
            pSetElement->disposeTree(true);
        }
    }
    else
    {
        // This must apply to a node for which no element tree had been loaded in this view
        // thus there should not be one now after the change (even if the change was replacing)
        OSL_ENSURE(!aRemoveInfo.change.element.newValue.is(), "Cannot dispose replaced element: No tree object available");
    }
}
// ---------------------------------------------------------------------------------------------------

static
void disposeRemovedNodes(configuration::NodeChangesInformation const& aChanges, Factory& aFactory)
{
    using configuration::NodeChangeData;
    using configuration::NodeChangesInformation;
    for (NodeChangesInformation::Iterator it = aChanges.begin(); it != aChanges.end(); ++it)
    {
        switch (it->change.type)
        {
        case NodeChangeData::eReplaceElement:
            // check if element is actually unchanged !
            // (cannot dispose of the tree, if it is still in use)
            if (! it->change.element.isDataChange()) break;

            // else dispose the old one: fall thru

        case NodeChangeData::eRemoveElement:
            disposeOneRemovedNode( *it, aFactory );
            break;

        default: break;
        }
    }
}
// ---------------------------------------------------------------------------------------------------
//INodeListener : IConfigListener
void ApiRootTreeImpl::NodeListener::nodeChanged(data::Accessor const& _aChangedDataAccessor, Change const& aChange, AbsolutePath const& sPath, IConfigBroadcaster* _pSource)
{
    osl::ClearableMutexGuard aGuard(mutex);

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        UnoInterfaceRef xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->nodeChanged(_aChangedDataAccessor,aChange,sPath,_pSource);
    }
}
// ---------------------------------------------------------------------------------------------------

//INodeListener : IConfigListener
void ApiRootTreeImpl::nodeChanged(data::Accessor const& _aChangedDataAccessor, Change const& aChange, AbsolutePath const& aChangePath, IConfigBroadcaster* pSource)
{
    using configuration::AnyNodeRef;
    using configuration::NodeChanges;
    using configuration::RelativePath;
    using configuration::AbsolutePath;

    // do not dipatch if we are dying/dead anyway
    if (m_aTreeImpl.isAlive())
    try
    {
        osl::MutexGuard aLocalGuard(m_aTreeImpl.getDataLock());

        configuration::Tree aTree( _aChangedDataAccessor,m_aTreeImpl.getTree() );

        OSL_ENSURE(configuration::Path::hasPrefix(aChangePath, m_aLocationPath),
                    "'changed' Path does not indicate this tree or its context: ");

        RelativePath aLocalChangePath = configuration::Path::stripPrefix(aChangePath,m_aLocationPath);

        // find the node and change
        NodeRef  aNode;

        if ( !aLocalChangePath.isEmpty() )
        {
            NodeRef aBaseNode = aTree.getRootNode();

#ifdef DBG_UTIL
            try {
                RelativePath aLocalPathOld = configuration::validateAndReducePath(aChangePath.toString(), aTree, aBaseNode);
                OSL_ENSURE( configuration::matches(aLocalPathOld,aLocalChangePath),
                            "New local path different from validateAndReducePath(...) result in notification dispatch");
            }
            catch (configuration::Exception& e) {
                rtl::OString sMsg("Cannot validate new path handling for notification dispatch: ");
                sMsg += e.what();
                OSL_ENSURE(false, sMsg.getStr() );
            }
#endif // DBG_UTIL

            AnyNodeRef aFoundNode = configuration::getDeepDescendant(aTree, aBaseNode, aLocalChangePath);
            if ( aFoundNode.isValid() )
            {
                if (aFoundNode.isNode())
                {
                    aNode = aFoundNode.toNode();
                }
                else
                {
                    // TODO: Notify using parent node and temporary dummy change
                    OSL_ENSURE( false, "Notification broken: Node being adressed is a Value");
                }
            }
        }
        else
        {
            aNode = aTree.getRootNode();
        }

        SubtreeChange const* pTreeChange = NULL;
        if (aNode.isValid())
        {
            if (aChange.ISA(SubtreeChange))
                pTreeChange = static_cast<SubtreeChange const*>(&aChange);

            else // TODO: Notify set change using parent (if available) and temporary dummy change
                OSL_ENSURE( false, "Notification broken: Change to inner node is not a subtree change");
        }

        if (pTreeChange != NULL) // implies aNode.isValid()
        {
            OSL_ENSURE( aChange.getNodeName() == aTree.getName(aNode).toString(),
                        "Change's node-name does not match found node's name - erratic notification");

            configuration::NodeChangesInformation aChanges;

            if (configuration::adjustToChanges(aChanges, aTree,aNode, *pTreeChange))
            {
                OSL_ASSERT(aChanges.size() > 0);

                Broadcaster aSender(m_aTreeImpl.getNotifier(),aChanges,false);

    // Should be improved later. Maybe this is the wrong lock for disposeTree ?
    //          aLocalGuard.downgrade(); // partial clear for broadcast

                aSender.notifyListeners(aChanges, false);

                disposeRemovedNodes(aChanges, m_aTreeImpl.getFactory());
            }
        }
    }
    catch (configuration::InvalidName& i)
    {
        rtl::OString sMsg("Cannot locate change within this tree: ");
        sMsg += i.what();
        OSL_ENSURE(false, sMsg.getStr() );
    }
    catch (configuration::Exception& e)
    {
        rtl::OString sMsg("Unexpected error trying to react on update: ");
        sMsg += e.what();
        OSL_ENSURE(false, sMsg.getStr() );
    }
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::NodeListener::nodeDeleted(data::Accessor const& _aChangedDataAccessor, AbsolutePath const& _aPath, IConfigBroadcaster* _pSource)
{
    osl::ClearableMutexGuard aGuard(mutex);

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        UnoInterfaceRef xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->nodeDeleted(_aChangedDataAccessor,_aPath,_pSource);
    }
}
// ---------------------------------------------------------------------------------------------------
void ApiRootTreeImpl::nodeDeleted(data::Accessor const& _aChangedDataAccessor, AbsolutePath const& _aDeletedPath, IConfigBroadcaster* pSource)
{
    // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
    UnoInterfaceRef xKeepAlive( m_aTreeImpl.getUnoInstance() );

#ifdef DBG_UTIL

    {
        osl::MutexGuard aLocalGuard(m_aTreeImpl.getDataLock());

        OSL_ENSURE(configuration::Path::hasPrefix(m_aLocationPath, _aDeletedPath),
                    "'deleted' Path does not indicate this tree or its context: ");
    }
#endif
    // ensure our provider stays alive
    UnoInterfaceRef xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

    rtl::Reference<NodeListener> xListener = m_pNotificationListener;
    if (xListener.is())
    {
        xListener->clearParent();
        xListener.clear();
    }

    if (m_aTreeImpl.disposeTreeNow())
        releaseData();
}

// ---------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}


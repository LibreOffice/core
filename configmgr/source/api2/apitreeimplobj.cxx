/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apitreeimplobj.cxx,v $
 * $Revision: 1.44 $
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
#include "precompiled_configmgr.hxx"
#include <stdio.h>

#include "apitreeimplobj.hxx"
#include "confignotifier.hxx"
#include "notifierimpl.hxx"
#include "apifactory.hxx"
#include "apitreeaccess.hxx"
#include "nodechangeinfo.hxx"
#include "broadcaster.hxx"
#include "change.hxx"
#include "providerimpl.hxx"
#include "roottree.hxx"
#include "noderef.hxx"
#include "anynoderef.hxx"
#include "tracer.hxx"
#include "treemanager.hxx"
#include <cppuhelper/queryinterface.hxx>
#include <vos/refernce.hxx>

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
    vos::ORefCount m_refs;

    ApiTreeImpl* pOwner;

    uno::Reference< lang::XComponent > xProvider;
    uno::Reference< lang::XComponent > xParent;
public:
    ComponentAdapter(ApiTreeImpl& rParent) : pOwner(&rParent) {}
    virtual ~ComponentAdapter() {}

    void clear();

    void setProvider(uno::Reference< lang::XComponent > const& xProvider);
    void setParent(uno::Reference< lang::XComponent > const& xParent);
    uno::Reference< lang::XComponent > getProvider() const;

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
    UnoApiLockClearable aGuard;

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
void ApiTreeImpl::ComponentAdapter::setProvider(uno::Reference< lang::XComponent > const& rProvider)
{
    this->setComponent( this->xProvider, rProvider);
}
void ApiTreeImpl::ComponentAdapter::setParent(uno::Reference< lang::XComponent > const& rParent)
{
    this->setComponent( this->xParent, rParent);
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
    UnoApiLockClearable aGuard;

    if (this->pOwner != NULL)
    {
        CFG_TRACE_INFO("ApiTreeImpl:ComponentAdapter: Providing UNO object is disposed - relaying to my owner");
        // ensure our owner stays alive
        uno::Reference<uno::XInterface> xKeepOwnerAlive( this->pOwner->getUnoInstance() );
        // and we stay alive too
        rtl::Reference< ApiTreeImpl::ComponentAdapter > xKeepAlive( this );

        aGuard.clear();

        pOwner->disposing( rEvt );

        UnoApiLock aClearGuard;
        if (rEvt.Source == this->xParent) this->xParent.clear();
        if (rEvt.Source == this->xProvider) this->xProvider.clear();
    }
    else
        CFG_TRACE_INFO("ApiTreeImpl:ComponentAdapter: Providing UNO object is disposed - but my owner is already gone");
}

//-------------------------------------------------------------------------

void ApiTreeImpl::ComponentAdapter::clear()
{
    UnoApiLockClearable aGuard;

    this->pOwner = 0;

    uno::Reference< lang::XComponent > aProvider = this->xProvider;
    uno::Reference< lang::XComponent > aParent   = this->xParent;
    this->xProvider = 0;
    this->xParent = 0;

    aGuard.clear();

    if (aParent.is())   try { aParent  ->removeEventListener(this); } catch (uno::Exception & ) {}
    if (aProvider.is()) try { aProvider->removeEventListener(this); } catch (uno::Exception & ) {}
}

//-----------------------------------------------------------------------------
class ApiRootTreeImpl::NodeListener : public INodeListener
{
    ApiRootTreeImpl*    pParent;
    TreeManager * pSource;

    vos::ORef< OOptions >           m_xOptions;
    configuration::AbsolutePath       m_aLocationPath;
public:
    NodeListener(ApiRootTreeImpl& _rParent)
        : pParent(&_rParent)
        , pSource(NULL)
        , m_aLocationPath( configuration::AbsolutePath::root() )
    {}
    ~NodeListener()
    {
        unbind();
    }

    TreeManager * getSource()
    {
        UnoApiLock aGuard;
        return pSource;
    }

    void setSource(TreeManager * pNew)
    {
        UnoApiLock aGuard;
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

    void setLocation(configuration::AbsolutePath const& _aLocation, vos::ORef< OOptions > const& _xOptions)
    {
        OSL_ASSERT(_xOptions.isValid());

        UnoApiLock aGuard;

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
        UnoApiLock aGuard;
        OSL_ASSERT(pParent == 0);
        pParent = 0;
        if (pSource)
        {
            OSL_ASSERT(m_xOptions.isValid());
            pSource->removeListener(m_xOptions->getRequestOptions(), this);
            m_xOptions.unbind();
            m_aLocationPath = configuration::AbsolutePath::root();
        }

    }

    void clearParent()
    {
        UnoApiLockClearable aGuard;
        if (pParent)
        {
            pParent = 0;

            if (pSource)
            {
                TreeManager * pOrgSource = pSource;
                vos::ORef< OOptions > xOptions = m_xOptions;

                pSource = 0;
                m_xOptions.unbind();
                m_aLocationPath = configuration::AbsolutePath::root();

                aGuard.clear();

                OSL_ASSERT(xOptions.isValid());
                pOrgSource->removeListener(xOptions->getRequestOptions(), this);
            }
        }
    }

    // Interfaces
    virtual void disposing(TreeManager * pSource);
    virtual void nodeChanged(Change const& aChange, configuration::AbsolutePath const& sPath, TreeManager * pSource);
    virtual void nodeDeleted(configuration::AbsolutePath const& sPath, TreeManager * pSource);
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

uno::Reference<com::sun::star::script::XTypeConverter> ApiProvider::getTypeConverter() const
{
    return m_rProviderImpl.getTypeConverter();
}
//-------------------------------------------------------------------------
static
inline
configuration::DefaultProvider createDefaultProvider(
                                    ApiProvider& rProvider,
                                    rtl::Reference< configuration::Tree > const& aTree,
                                    vos::ORef< OOptions > const& _xOptions
                               )
{
    OProviderImpl& rProviderImpl        = rProvider.getProviderImpl();
    rtl::Reference< TreeManager > xDefaultProvider  = rProviderImpl.getDefaultProvider();

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
ApiTreeImpl::ApiTreeImpl(uno::XInterface* pInstance, ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, ApiTreeImpl* pParentTree)
: m_aTree(aTree)
, m_aNotifier(new NotifierImpl(aTree))
, m_aDefaultProvider(extractDefaultProvider(pParentTree))
, m_xProvider()
, m_rProvider(rProvider)
, m_pParentTree(0)
, m_pInstance(pInstance)
{
    OSL_ENSURE(pParentTree == NULL || &rProvider == &pParentTree->m_rProvider,"WARNING: Parent tree has a different provider - trouble may be ahead");
    setNodeInstance(aTree->getRootNode(), pInstance);
    init(pParentTree);
}
//-------------------------------------------------------------------------
ApiTreeImpl::ApiTreeImpl(uno::XInterface* _pInstance, ApiProvider& _rProvider, rtl::Reference< configuration::Tree > const& _aTree, configuration::DefaultProvider const& _aDefaultProvider)
: m_aTree(_aTree)
, m_aNotifier(new NotifierImpl(_aTree))
, m_aDefaultProvider(_aDefaultProvider)
, m_xProvider()
, m_rProvider(_rProvider)
, m_pParentTree(0)
, m_pInstance(_pInstance)
{
    setNodeInstance(_aTree->getRootNode(), _pInstance);
    init(NULL);
}
//-------------------------------------------------------------------------

ApiTreeImpl::~ApiTreeImpl()
{
    OSL_ENSURE(m_aNotifier->m_aListeners.isDisposed(),"ApiTree Object was not disposed properly");
    deinit();
}
//-------------------------------------------------------------------------

ApiRootTreeImpl::ApiRootTreeImpl(uno::XInterface* pInstance, ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions > const& _xOptions)
: m_aTreeImpl(pInstance, rProvider, aTree, createDefaultProvider(rProvider, aTree, _xOptions))
, m_aLocationPath( configuration::Path::Rep() )
, m_pNotificationListener(NULL)
, m_xOptions(_xOptions)
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

void ApiTreeImpl::setNodeInstance(configuration::NodeRef const& aNode, uno::XInterface* pInstance)
{
    OSL_ENSURE(aNode.isValid(),"ERROR: adding invalid node to ApiTree");
    OSL_ENSURE(m_aTree->isValidNode(aNode.getOffset()),"ERROR: foreign node being added to ApiTree");
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

Notifier ApiTreeImpl::getNotifier() const
{
    return Notifier(m_aNotifier,this);
}
//-------------------------------------------------------------------------

bool ApiRootTreeImpl::enableNotification(bool bEnable)
{
    TreeManager * pSource = bEnable ? getApiTree().getProvider().getProviderImpl().getNotifier() : 0;

    TreeManager * pOld = this->implSetNotificationSource(pSource);

    return pOld != 0;
}
//-------------------------------------------------------------------------

bool ApiTreeImpl::disposeTree(bool bForce)
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing Tree (may throw if already disposed)");

    // ensure our provider stays alive
    uno::Reference<uno::XInterface> xKeepParentAlive(this->getParentComponent());
    // ensure we stay alive too
    uno::Reference<uno::XInterface> xKeepAlive(this->getUnoInstance());

    // #109077# If already disposed, we may have no source data or data lock
    if (!isAlive())
        return false;

    if (!bForce)
    {
        if (m_pParentTree != 0)
            return false;

        // recheck after having the mutex
        checkAlive(); // may throw
    }
    else if (m_pParentTree)
        setParentTree(NULL);

    implDisposeTree();
    OSL_ASSERT(!isAlive()); // post condition

    return true;
}
//-------------------------------------------------------------------------

bool ApiTreeImpl::disposeTreeNow()
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing Tree Now (unless disposed)");
    if (isAlive() )
        return implDisposeTree();
    else
        return false;
}
//-------------------------------------------------------------------------
bool ApiRootTreeImpl::disposeTree()
{
    CFG_TRACE_INFO("Api Root Tree: Disposing Tree And Releasing (unless disposed)");
    // ensure our provider stays alive
    uno::Reference<uno::XInterface> xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

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
        CFG_TRACE_INFO2("Api Root Tree: data was not released in disposeTree");
    }

    return bDisposed;
}
//-------------------------------------------------------------------------
bool ApiTreeImpl::implDisposeTree()
{
    OSL_ENSURE(m_pParentTree == 0,"WARNING: Disposing a tree that still has a parent tree set");

    SpecialListenerContainer <configuration::SubNodeID,SubNodeHash,SubNodeEq,SubNodeToIndex>& aContainer = m_aNotifier->m_aListeners;
    if (aContainer.beginDisposing())
    {
        CFG_TRACE_INFO("ApiTreeImpl: Tree is now disposed");

        Factory& rFactory = getFactory();

        std::vector<configuration::NodeID> aChildNodes;
        configuration::getAllContainedNodes( m_aTree, aChildNodes);

        for (std::vector<configuration::NodeID>::reverse_iterator it = aChildNodes.rbegin(), stop = aChildNodes.rend();
            it != stop;
            ++it)
        {
            rFactory.revokeElement( *it );
        }

        CFG_TRACE_INFO_NI("ApiTreeImpl: Listeners are now informed");
        aContainer.notifyDisposing();

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
void ApiTreeImpl::disposeNode(configuration::NodeRef const& aNode, uno::XInterface* pInstance)
{
    // This used to contain 3 nested 'isAlive()' calls; why !?
    if (isAlive())
        implDisposeNode(aNode,pInstance);
}
//-------------------------------------------------------------------------
void ApiTreeImpl::implDisposeNode(configuration::NodeRef const& aNode, uno::XInterface* )
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing a single node.");
    OSL_ENSURE(aNode.isValid(),"INTERNAL ERROR: Disposing NULL node");
    OSL_ENSURE(m_aTree->isValidNode(aNode.getOffset()),"INTERNAL ERROR: Disposing: node does not match tree");
    OSL_ENSURE( !m_aTree->isRootNode(aNode),"INTERNAL ERROR: Disposing the root node of the tree");

    configuration::NodeID aNodeID(m_aTree,aNode);

    if (m_aNotifier->m_aListeners.disposeOne(aNodeID.toIndex()) )
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

    uno::Reference<ComponentAdapter> xAdapter = m_xProvider;
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
#if OSL_DEBUG_LEVEL > 0
    if (pParentTree)
    {
        rtl::Reference< configuration::Tree > aContext = m_aTree->getContextTree();
        rtl::Reference< configuration::Tree > aParent = pParentTree->m_aTree;

        configuration::NodeID aContextID( aContext, aContext->getRootNode() );
        configuration::NodeID aParentID( aParent, aParent->getRootNode() );

        OSL_ENSURE( aContextID == aParentID, "Parent relationship mismatch !");
    }
#endif

    if (m_pParentTree != pParentTree)
    {
        uno::Reference<ComponentAdapter> xAdapter = m_xProvider;

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

uno::Reference<uno::XInterface> ApiTreeImpl::getUnoProviderInstance() const
{
    uno::Reference<ComponentAdapter> xAdapter = m_xProvider;

    uno::Reference<uno::XInterface> xReturn;
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

void ApiTreeImpl::disposing(com::sun::star::lang::EventObject const& ) throw()
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
TreeManager * ApiRootTreeImpl::implSetNotificationSource(TreeManager * pNew)
{
    TreeManager * pOld = m_pNotificationListener.is() ? m_pNotificationListener->getSource() : 0;
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

void ApiRootTreeImpl::implSetLocation(rtl::Reference< configuration::Tree > const& _aTree)
{
    OSL_ASSERT(_aTree == getApiTree().getTree());
    if (!configuration::isEmpty(_aTree.get()))
    {
        m_aLocationPath = _aTree->getRootPath();
        OSL_ENSURE(!m_aLocationPath.isRoot(), "Setting up a root tree without location");
    }
    else
    {
        OSL_ENSURE(false, "Setting up a root tree without data");
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
    CFG_TRACE_INFO("Api Root Tree at %s: releasing the Data",OUSTRING2ASCII(m_aLocationPath.toString()));
    rtl::Reference< configuration::Tree > aTree( m_aTreeImpl.getTree() );

    if (aTree.is()) {
        aTree->disposeData();
    }
    OSL_ASSERT(configuration::isEmpty(aTree.get()));

    OSL_ENSURE( !m_aLocationPath.isRoot() && !m_aLocationPath.isDetached(), "Location still needed to release data" );
    OSL_ENSURE( m_xOptions.isValid(), "Options still needed to release data" );

    getApiTree().getProvider().getProviderImpl().releaseSubtree(m_aLocationPath,m_xOptions->getRequestOptions());
    m_xOptions.unbind();

    m_aLocationPath = configuration::AbsolutePath::detachedRoot();
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::NodeListener::disposing(TreeManager * _pSource)
{
    UnoApiLockClearable aGuard;

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        uno::Reference<uno::XInterface> xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->disposing(_pSource);
    }
}
void ApiRootTreeImpl::disposing(TreeManager *)
{
    CFG_TRACE_INFO("Api Root Tree at %s: Cache data is disposed - dispose and release own data",
                    OUSTRING2ASCII(m_aLocationPath.toString()));
        // ensure our provider stays alive
    uno::Reference<uno::XInterface> xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

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

        rtl::Reference< configuration::ElementTree > aElementRef( aRemoveInfo.change.element.oldValue.get() );

        SetElement* pSetElement = aFactory.findSetElement(aElementRef );
        if (pSetElement)
        {
            // factory always does an extra acquire
            uno::Reference<uno::XInterface> xReleaseSetElement(pSetElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);

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
    for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin(); it != aChanges.end(); ++it)
    {
        switch (it->change.type)
        {
        case configuration::NodeChangeData::eReplaceElement:
            // check if element is actually unchanged !
            // (cannot dispose of the tree, if it is still in use)
            if (! it->change.element.isDataChange()) break;

            // else dispose the old one: fall thru

        case configuration::NodeChangeData::eRemoveElement:
            disposeOneRemovedNode( *it, aFactory );
            break;

        default: break;
        }
    }
}
// ---------------------------------------------------------------------------------------------------
//INodeListener : IConfigListener
void ApiRootTreeImpl::NodeListener::nodeChanged(Change const& aChange, configuration::AbsolutePath const& sPath, TreeManager * _pSource)
{
    UnoApiLockClearable aGuard;

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        uno::Reference<uno::XInterface> xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->nodeChanged(aChange,sPath,_pSource);
    }
}
// ---------------------------------------------------------------------------------------------------

//INodeListener : IConfigListener
void ApiRootTreeImpl::nodeChanged(Change const& aChange, configuration::AbsolutePath const& aChangePath, TreeManager *)
{
    // do not dipatch if we are dying/dead anyway
    if (m_aTreeImpl.isAlive())
    try
    {
        rtl::Reference< configuration::Tree > aTree(m_aTreeImpl.getTree());

        OSL_ENSURE(configuration::Path::hasPrefix(aChangePath, m_aLocationPath),
                    "'changed' Path does not indicate this tree or its context: ");

        configuration::RelativePath aLocalChangePath = configuration::Path::stripPrefix(aChangePath,m_aLocationPath);

        // find the node and change
        configuration::NodeRef  aNode;

        if ( !aLocalChangePath.isEmpty() )
        {
            configuration::NodeRef aBaseNode = aTree->getRootNode();

#ifdef DBG_UTIL
            try {
                configuration::RelativePath aLocalPathOld = configuration::validateAndReducePath(aChangePath.toString(), aTree, aBaseNode);
                OSL_ENSURE( configuration::matches(aLocalPathOld,aLocalChangePath),
                            "New local path different from validateAndReducePath(...) result in notification dispatch");
            }
            catch (configuration::Exception& e) {
                rtl::OString sMsg("Cannot validate new path handling for notification dispatch: ");
                sMsg += e.what();
                OSL_ENSURE(false, sMsg.getStr() );
            }
#endif // DBG_UTIL

            configuration::AnyNodeRef aFoundNode = configuration::getDeepDescendant(aTree, aBaseNode, aLocalChangePath);
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
            aNode = aTree->getRootNode();
        }

        SubtreeChange const* pTreeChange = NULL;
        if (aNode.isValid())
        {
            pTreeChange = dynamic_cast<SubtreeChange const*>(&aChange);
            OSL_ENSURE(pTreeChange != 0, "Notification broken: Change to inner node is not a subtree change"); // TODO: Notify set change using parent (if available) and temporary dummy change
        }

        if (pTreeChange != NULL) // implies aNode.isValid()
        {
            OSL_ENSURE( aChange.getNodeName() == aTree->getSimpleNodeName(aNode.getOffset()),
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

void ApiRootTreeImpl::NodeListener::nodeDeleted(configuration::AbsolutePath const& _aPath, TreeManager * _pSource)
{
    UnoApiLockClearable aGuard;

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        uno::Reference<uno::XInterface> xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->nodeDeleted(_aPath,_pSource);
    }
}
// ---------------------------------------------------------------------------------------------------
void ApiRootTreeImpl::nodeDeleted(configuration::AbsolutePath const& _aDeletedPath, TreeManager *)
{
    { (void)_aDeletedPath; }

    // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
    uno::Reference<uno::XInterface> xKeepAlive( m_aTreeImpl.getUnoInstance() );

#ifdef DBG_UTIL
    OSL_ENSURE(configuration::Path::hasPrefix(m_aLocationPath, _aDeletedPath),
               "'deleted' Path does not indicate this tree or its context: ");
#endif
    // ensure our provider stays alive
    uno::Reference<uno::XInterface> xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

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


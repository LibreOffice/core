/*************************************************************************
 *
 *  $RCSfile: apitreeimplobj.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-27 15:47:31 $
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
#include "confproviderimpl2.hxx"
#include "confignotifier.hxx"
#include "notifierimpl.hxx"
#include "apifactory.hxx"
#include "apitreeaccess.hxx"
//#include "nodechange.hxx"
#include "nodechangeinfo.hxx"
#include "broadcaster.hxx"
#include "roottree.hxx"
#include "noderef.hxx"

#include "tracer.hxx"

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

        if (xOld.is()) xOld->removeEventListener(this);
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

    if (xParent.is()) xParent->removeEventListener(this);
    if (xProvider.is()) xProvider->removeEventListener(this);
}

//-----------------------------------------------------------------------------
class ApiRootTreeImpl::NodeListener : public INodeListener
{
    osl::Mutex mutex;
    ApiRootTreeImpl*    pParent;
    IConfigBroadcaster* pSource;

    vos::ORef< OOptions > m_xOptions;
    OUString            m_aLocationPath;
public:
    NodeListener(ApiRootTreeImpl& _rParent)
        : pParent(&_rParent)
        , pSource(NULL)
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
                if (pSource)
                    pSource->removeListener(m_xOptions, this);

                pSource = pNew;
                if (pNew)
                {
                    OSL_ENSURE(m_aLocationPath.getLength() > 0, "Cannot register for notifications: no location set");
                    pNew->addListener(m_aLocationPath, m_xOptions, this);
                }
            }
        }
    }

    void setLocation(OUString const& _aLocation, vos::ORef< OOptions > const& _xOptions)
    {
        osl::MutexGuard aGuard(mutex);

        if (pSource && pParent)
            pSource->removeListener(m_xOptions, this);

        m_aLocationPath = _aLocation;
        m_xOptions = _xOptions;

        if (pSource && pParent)
            pSource->addListener(m_aLocationPath, m_xOptions, this);
    }

    void unbind()
    {
        osl::MutexGuard aGuard(mutex);
        OSL_ASSERT(pParent == 0);
        pParent = 0;
        if (pSource)
        {
            pSource->removeListener(m_xOptions, this);
            m_xOptions.unbind();
            m_aLocationPath = OUString();
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
                vos::ORef< OOptions > xOptions = m_xOptions;

                pSource = 0;
                m_xOptions.unbind();
                m_aLocationPath = OUString();

                aGuard.clear();

                pOrgSource->removeListener(xOptions, this);
            }
        }
    }

    // Interfaces
    virtual void disposing(IConfigBroadcaster* pSource);
    virtual void nodeChanged(Change const& aChange, OUString const& sPath, IConfigBroadcaster* pSource);
    virtual void nodeDeleted(OUString const& sPath, IConfigBroadcaster* pSource);
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

configuration::TemplateProvider ApiProvider::getTemplateProvider() const
{
    return m_rProviderImpl.getTemplateProvider();
}
//-------------------------------------------------------------------------

UnoTypeConverter ApiProvider::getTypeConverter() const
{
    return m_rProviderImpl.getTypeConverter();
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
, m_aNotifier(new NotifierImpl(aTree))
{
    setNodeInstance(aTree.getRootNode(), pInstance);
    init(&rParentTree);
}
//-------------------------------------------------------------------------
ApiTreeImpl::ApiTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, Tree const& aTree, ApiTreeImpl* pParentTree)
: m_pInstance(pInstance)
, m_aTree(aTree)
, m_rProvider(rProvider)
, m_pParentTree(0)
, m_aNotifier(new NotifierImpl(aTree))
{
    OSL_ENSURE(!pParentTree || &rProvider == &pParentTree->m_rProvider,"WARNING: Parent tree has a different provider - trouble may be ahead");
    setNodeInstance(aTree.getRootNode(), pInstance);
    init(pParentTree);
}
//-------------------------------------------------------------------------

ApiTreeImpl::~ApiTreeImpl()
{
    OSL_ENSURE(m_aNotifier->m_aListeners.isDisposed(),"ApiTree Object was not disposed properly");
    deinit();
}
//-------------------------------------------------------------------------
ApiRootTreeImpl::ApiRootTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, Tree const& aTree, vos::ORef< OOptions >const& _xOptions)
: m_aTreeImpl(pInstance, rProvider, aTree, 0)
, m_pNotificationListener(NULL)
, m_xOptions(_xOptions)
{
    implSetLocation();
    enableNotification(true);
}
//-------------------------------------------------------------------------
ApiRootTreeImpl::~ApiRootTreeImpl()
{
    if (m_pNotificationListener.isValid())
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
    OSL_VERIFY( m_aNotifier->m_aListeners.checkAlive( getUnoInstance()) ); //, "Object is being disposed" );
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
    OWriteSynchronized aLocalGuard(getDataLock());
    if (!bForce)
    {
        if (m_pParentTree != 0)
            return false;

        checkAlive(); // may throw
    }
    else if (m_pParentTree)
        setParentTree(NULL);

    implDisposeTree();
    OSL_ASSERT(!isAlive());

    return true;
}
//-------------------------------------------------------------------------

bool ApiTreeImpl::disposeTreeNow()
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing Tree Now (unless disposed)");
    if (isAlive() )
    {
        OWriteSynchronized aLocalGuard(getDataLock());
        return implDisposeTree();
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

    vos::ORef<NodeListener> xListener = m_pNotificationListener;
    if (xListener.isValid())
    {
        xListener->clearParent();
        xListener.unbind();
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
bool ApiTreeImpl::implDisposeTree()
{
    OSL_ENSURE(m_pParentTree == 0,"WARNING: Disposing a tree that still has a parent tree set");

    NotifierImpl::SpecialContainer& aContainer = m_aNotifier->m_aListeners;
    if (aContainer.beginDisposing())
    {
        CFG_TRACE_INFO("ApiTreeImpl: Tree is now disposed");
        using configuration::NodeIDList;
        using configuration::NodeID;
        using configuration::getAllContainedNodes;
        using com::sun::star::lang::EventObject;

        Factory& rFactory = getFactory();

        NodeIDList aChildNodes;
        getAllContainedNodes(m_aTree, aChildNodes);

        for (NodeIDList::reverse_iterator it = aChildNodes.rbegin(), stop = aChildNodes.rend();
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
void ApiTreeImpl::disposeNode(NodeRef const& aNode, UnoInterface* pInstance)
{
    if (isAlive())
    {
        OWriteSynchronized aLocalGuard(getDataLock());
        if (isAlive())
            implDisposeNode(aNode,pInstance);
    }
}
//-------------------------------------------------------------------------
void ApiTreeImpl::implDisposeNode(NodeRef const& aNode, UnoInterface* pInstance)
{
    CFG_TRACE_INFO("ApiTreeImpl: Disposing a single node.");
    OSL_ENSURE(aNode.isValid(),"INTERNAL ERROR: Disposing NULL node");
    OSL_ENSURE(m_aTree.isValidNode(aNode),"INTERNAL ERROR: Disposing: node does not match tree");
    OSL_ENSURE( !m_aTree.isRootNode(aNode),"INTERNAL ERROR: Disposing the root node of the tree");

    using configuration::NodeID;
    using com::sun::star::lang::EventObject;

    NodeID aNodeID(m_aTree,aNode);

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
        Tree aContext = m_aTree.getContextTree();
        Tree aParent = pParentTree->m_aTree;

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

    IConfigBroadcaster* pOld = m_pNotificationListener.isValid() ? m_pNotificationListener->getSource() : 0;
    if (pOld != pNew)
    {
        OSL_ENSURE(m_xOptions.isValid(), "Cannot change notification source without options");

        if (!m_pNotificationListener.isValid())
            m_pNotificationListener = new NodeListener(*this);

        m_pNotificationListener->setSource(pNew);

    }
    return pOld;
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::implSetLocation()
{
    osl::MutexGuard aGuard(getApiTree().getApiLock());

    Tree aTree = getApiTree().getTree();
    if (!aTree.isEmpty())
    {
        configuration::Name aRootName = aTree.getRootNode().getName();
        m_aLocationPath = aTree.getContextPath().compose( configuration::RelativePath(aRootName) ).toString();
    }
    else
    {
        OSL_ENSURE(false, "Setting up a RootTree without data");
        m_aLocationPath = OUString();
    }
    OSL_ENSURE(m_aLocationPath.getLength() > 0, "Setting up a RootTree without location");

    if (!m_pNotificationListener.isValid())
        m_pNotificationListener = new NodeListener(*this);

    OSL_ENSURE(m_aLocationPath.getLength() > 0, "Cannot reregister for notifications: setting empty location");
    OSL_ENSURE( m_xOptions.isValid(), "Cannot reregister for notifications: no options available" );

    m_pNotificationListener->setLocation(m_aLocationPath, m_xOptions);
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::releaseData()
{
    CFG_TRACE_INFO("Api Root TreeImpl at %s: releasing the Data",OUSTRING2ASCII(m_aLocationPath));
    Tree aTree( m_aTreeImpl.getTree() );

    aTree.disposeData();
    OSL_ASSERT(aTree.isEmpty());

    OSL_ENSURE( m_aLocationPath.getLength(), "Location still needed to release data" );
    OSL_ENSURE( m_xOptions.isValid(), "Options still needed to release data" );

    getApiTree().getProvider().getProviderImpl().releaseSubtree(m_aLocationPath,m_xOptions);
    m_xOptions.unbind();
    m_aLocationPath = OUString();
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
            OUSTRING2ASCII(m_aLocationPath));
        // ensure our provider stays alive
    UnoInterfaceRef xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

    vos::ORef<NodeListener> xListener = m_pNotificationListener;
    if (xListener.isValid())
    {
        xListener->clearParent();
        xListener.unbind();
    }

    if (m_aTreeImpl.disposeTreeNow())
        releaseData(); // not really needed: the whole data is going away anyways
}
// ---------------------------------------------------------------------------------------------------

static
void disposeOneRemovedNode(configuration::NodeChangeInformation const& aRemoveInfo, Factory& aFactory)
{
    OSL_ENSURE(aRemoveInfo.change.element.oldValue.isValid(), "Cannot dispose removed/replaced element: No tree object available");
    OSL_ENSURE(aRemoveInfo.change.element.isDataChange(), "ERROR: Dispose removed/replaced element: Element did not really change !");
    if (aRemoveInfo.change.element.oldValue.isValid())
    {
        configuration::ElementTree aElementTree( aRemoveInfo.change.element.oldValue.getBodyPtr() );

        SetElement* pSetElement = aFactory.findSetElement(aElementTree );
        if (pSetElement)
        {
            // factory always does an extra acquire
            UnoInterfaceRef xReleaseSetElement(pSetElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);

            pSetElement->haveNewParent(0);
            pSetElement->disposeTree(true);
        }
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
void ApiRootTreeImpl::NodeListener::nodeChanged(Change const& aChange, OUString const& sPath, IConfigBroadcaster* _pSource)
{
    osl::ClearableMutexGuard aGuard(mutex);

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        UnoInterfaceRef xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->nodeChanged(aChange,sPath,_pSource);
    }
}
// ---------------------------------------------------------------------------------------------------

//INodeListener : IConfigListener
void ApiRootTreeImpl::nodeChanged(Change const& aChange, OUString const& sPath, IConfigBroadcaster* pSource)
{
    using configuration::Path;
    using configuration::NodeChanges;
    using configuration::RelativePath;

    // do not dipatch if we are dying/dead anyway
    if (m_aTreeImpl.isAlive())
    try
    {
        OClearableWriteSynchronized aLocalGuard(m_aTreeImpl.getDataLock());

        Tree aTree( m_aTreeImpl.getTree() );

        OSL_ENSURE( Path(sPath, Path::NoValidate()).getType() == configuration::PathType::eABSOLUTE,
                    "Unexpected format for 'nodeChanged' Path location - path is not absolute" );

        bool bValidNotification = sPath.indexOf(m_aLocationPath) == 0;
        OSL_ENSURE(bValidNotification, "Notified Path does not match this tree's path - ignoring notification");

        if (bValidNotification)
        {
            // find the node
            NodeRef aNode = aTree.getRootNode();

            if (sPath != m_aLocationPath)
            {
                OSL_ASSERT(sPath.getLength() > m_aLocationPath.getLength());
                OSL_ENSURE(sPath[m_aLocationPath.getLength()] == sal_Unicode('/'),
                            "'nodeChanged' Path does not respect directory boundaries - erratic notification");

                OSL_ENSURE(sPath.getLength() > m_aLocationPath.getLength()+1, "Unexpected path format: slash terminated");

                RelativePath aLocalConfigPath = configuration::reduceRelativePath(sPath, aTree, aNode);

                bValidNotification = configuration::findDescendantNode(aTree, aNode, aLocalConfigPath);
            }

            if (bValidNotification)
            {
                OSL_ENSURE( aChange.getNodeName() == aNode.getName().toString(),
                            "Change's node-name does not match found node's name - erratic notification");

                configuration::TemplateProvider aProviderForNewSets = m_aTreeImpl.getProvider().getTemplateProvider();

                configuration::NodeChangesInformation aChanges;

                if (configuration::adjustToChanges(aChanges, aTree,aNode, aChange,aProviderForNewSets))
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
    }
    catch (configuration::Exception& e)
    {
        rtl::OString sMsg("Unexpected error trying to react on update: ");
        sMsg += e.what();
        OSL_ENSURE(false, sMsg.getStr() );
    }
}
// ---------------------------------------------------------------------------------------------------

void ApiRootTreeImpl::NodeListener::nodeDeleted(OUString const& sPath, IConfigBroadcaster* _pSource)
{
    osl::ClearableMutexGuard aGuard(mutex);

    OSL_ASSERT( !pSource || _pSource == pSource );
    if (pParent)
    {
        // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
        UnoInterfaceRef xKeepAlive( pParent->m_aTreeImpl.getUnoInstance() );
        ApiRootTreeImpl* pKeepParent = pParent;
        aGuard.clear();

        pKeepParent->nodeDeleted(sPath,_pSource);
    }
}
// ---------------------------------------------------------------------------------------------------
void ApiRootTreeImpl::nodeDeleted(OUString const& sPath, IConfigBroadcaster* pSource)
{
    // this is a non-UNO external entry point - we need to keep this object alive for the duration of the call
    UnoInterfaceRef xKeepAlive( m_aTreeImpl.getUnoInstance() );

#ifdef DBG_UTIL
    using configuration::Path;


    {
        OReadSynchronized aLocalGuard(m_aTreeImpl.getDataLock());

        OSL_ENSURE( Path(sPath, Path::NoValidate()).getType() == configuration::PathType::eABSOLUTE,
                    "Unexpected format for 'deleted' Path location - path is not absolute" );


        OSL_ENSURE( m_aLocationPath.indexOf( sPath ) == 0, "'deleted' Path does not indicate this tree or its context");

        const OUString delimiter = OUString::createFromAscii("/");
        OSL_ENSURE( m_aLocationPath.indexOf( sPath ) == m_aLocationPath.concat(delimiter).indexOf( sPath.concat(delimiter) ),
                            "'deleted' Path does not check subdirectory boundaries");
    }
#endif
    // ensure our provider stays alive
    UnoInterfaceRef xKeepProvider( m_aTreeImpl.getUnoProviderInstance() );

    vos::ORef<NodeListener> xListener = m_pNotificationListener;
    if (xListener.isValid())
    {
        xListener->clearParent();
        xListener.unbind();
    }

    if (m_aTreeImpl.disposeTreeNow())
        releaseData();
}

// ---------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}


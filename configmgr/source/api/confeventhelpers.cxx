/*************************************************************************
 *
 *  $RCSfile: confeventhelpers.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-21 12:06:54 $
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
#include "confeventhelpers.hxx"

#ifndef CONFIGMGR_CONFNAME_HXX_
#include "confname.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{
    namespace internal
    {

        void throwDispatchIllegalSequenceException()
        {
            OSL_ENSURE( 0, "Illegal Call to brodcaster while dispatching" );
        }

////////////////////////////////////////////////////////////////////////
/*      template <class Listener>
        class BroadcastImplHelper
        {
        public:
            osl::Mutex m_aMutex;

            BroadcastImplHelper() {}
            ~BroadcastImplHelper() {}

            typedef std::set<Listener*> Interfaces;
            typedef Interfaces::const_iterator Iterator;

            void addInterface(Listener* aListener) { m_aInterfaces.insert(aListener); }
            void removeInterface(Listener* aListener) { m_aInterfaces.erase(aListener); }

            void disposing(ConfigChangeBroadcaster* pSource);

            Iterator begin() const { return m_aInterfaces.begin(); }
            Iterator end() const { return m_aInterfaces.end(); }
        private:
            Interfaces m_aInterfaces;

            // no implementation - not copyable
            BroadcastImplHelper(BroadcastImplHelper&);
            void operator=(BroadcastImplHelper&);
        };
*/

/////////////////////////////////////////////////////////////////////////
/*
    struct NodeListenerInfo
    {
        INodeListener*  m_pListener;
        OUString        m_path;

    // fake a pointer for generic clients
        INodeListener* operator->() const { return m_pListener; }
        INodeListener& operator*() const { return *m_pListener; }

        bool operator < (NodeListenerInfo const& aInfo) const;
    };
*/
/////////////////////////////////////////////////////////////////////////

/*
    class ConfigChangesBroadcasterImpl
    {
    private:
        typedef BroadcastImplHelper<NodeListenerInfo> Listeners;
        Listeners m_aListeners;
        std::map<OUString, Listeners::Iterator> m_aDispatchInfos;
    };
*/
/////////////////////////////////////////////////////////////////////////
ConfigChangesBroadcasterImpl::ConfigChangesBroadcasterImpl()
{
}

/////////////////////////////////////////////////////////////////////////
ConfigChangesBroadcasterImpl::~ConfigChangesBroadcasterImpl()
{
    OSL_ENSURE(m_aListeners.begin() == m_aListeners.end(), "Remaining listeners found - forgot to dispose ?");
    OSL_ENSURE(m_aPathMap.empty(), "Spurious mappings found");
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::add(OUString const& aName, INodeListenerRef const& pListener)
{
    OSL_ASSERT( ! ConfigurationName(aName).isRelative() );

    osl::MutexGuard aGuard(m_aListeners.mutex);

    InfoRef aAdded = m_aListeners.addListener(NodeListenerInfo(pListener));
    aAdded->addPath(aName);
    m_aPathMap.insert(PathMap::value_type(aName,aAdded));
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::remove(INodeListenerRef const& pListener)
{
    osl::MutexGuard aGuard(m_aListeners.mutex);

    Listeners::Iterator const iter = m_aListeners.find(pListener);
    if (iter != m_aListeners.end())
    {
        typedef NodeListenerInfo::Pathes Pathes;
        Pathes const& pathes = iter->pathList();

        // first clear the Path Map
        for(Pathes::iterator itPath = pathes.begin(); itPath != pathes.end(); ++itPath)
        {
            typedef PathMap::iterator PMIter;
            typedef std::pair<PMIter, PMIter> PMRange;

            PMRange aRange = m_aPathMap.equal_range(*itPath);
            while (aRange.first != aRange.second)
            {
                PMIter cur = aRange.first++;
                if (cur->second == iter)
                    m_aPathMap.erase(cur);
            }
        }

        // the remove the broadcast helper entry
        m_aListeners.removeListener(pListener);
    }
}
/*
void ConfigChangesBroadcasterImpl::removed(OUString const& aBasePath, bool bRemovedFromModel, IConfigBroadcaster* pSource)
{
    OSL_ASSERT( ! ConfigurationName(aBasePath).isRelative() );

    // Dispatch 'deleted' to descendants of the changed path

    for(    PathMap::const_iterator it = m_aPathMap.lower_bound(aBasePath);
            it != m_aPathMap.end() && 0 == aBasePath.compareTo(it->first, aBasePath.getLength());
        )
    {
        OSL_ASSERT( m_aListeners.find(it->second->get()) != m_aListeners.end() );

        OUString aDispatchPath = it->first;
        OSL_ASSERT( ! ConfigurationName(aDispatchPath).isRelative() );

        INodeListenerRef pTarget = it->second->get();
        ++it;

        // we allow a listener to remove itself from within the callback
        // the simple increment above wont work, if the following listener is the same listener
        // (which really shouldn't happen)
        PathMap::const_iterator next = it;
        while (next != m_aPathMap.end() && next->second->get() == pTarget)
            ++next;

        pTarget->nodeDeleted(aBasePath, pSource);

        // if a listener removes itself from within the callback, it will be missing by now
        // so we check whether our listener is still there and if necessary patch our position
        if (m_aListeners.find(pTarget) == m_aListeners.end())
            it = next;
    }
}
*/

/////////////////////////////////////////////////////////////////////////
// This should actually be available from the TreeChangeList
/////////////////////////////////////////////////////////////////////////

static Change const* resolvePath(Change const* pChange, ConfigurationName& aRelativePath, RemoveNode const*& pRemoveNode)
{
    OSL_ASSERT(aRelativePath.isRelative());
    OSL_ASSERT(pRemoveNode == 0);
    pRemoveNode = 0;

    ConfigurationName::Iterator aIter(aRelativePath.begin());
    ConfigurationName::Iterator const aEnd(aRelativePath.end());

    OSL_ASSERT(pChange);
    OSL_ASSERT(aIter != aEnd);

    while (pChange)
    {
        if (pChange->ISA(RemoveNode))
            pRemoveNode = static_cast<RemoveNode const*>(pChange);

        OSL_ASSERT(*aIter == pChange->getNodeName());
        if (++aIter == aEnd)
            break; // found it

        pChange = pChange->getSubChange(*aIter);

        OSL_ASSERT(pRemoveNode == NULL || pChange == NULL);
    }
    if (pRemoveNode)
    {
        aRelativePath = ConfigurationName(aRelativePath.begin(),aIter);
        OSL_ASSERT( aRelativePath.localName() == pRemoveNode->getNodeName());
    }
    else
        OSL_ASSERT(pChange == 0 || aRelativePath == ConfigurationName(aRelativePath.begin(),aIter));

    return pChange;
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatchInner
(
    INodeListenerRef const& pTarget,
    OUString const& sTargetPath,
    Change const& rBaseChange,
    OUString const& sChangeContext,
    sal_Bool , //_bError,
    IConfigBroadcaster* pSource
)
{
    ConfigurationName aContext(sChangeContext);

    OSL_ASSERT(pTarget.isValid());
    OSL_ASSERT( ConfigurationName(sTargetPath).isNestedIn( aContext ) );

    ConfigurationName aLocalPath = ConfigurationName(sTargetPath).relativeTo( aContext );
    RemoveNode const* pRemoved = 0;
    Change const* pTargetChange = resolvePath(&rBaseChange, aLocalPath, pRemoved );

    if (pRemoved)
    {
        pTarget->nodeDeleted(aContext.composeWith(aLocalPath).fullName(), pSource);
    }
    else if (pTargetChange)
    {
        OSL_ASSERT(aContext.composeWith(aLocalPath) == sTargetPath);
        pTarget->nodeChanged(*pTargetChange, sTargetPath, pSource);
    }
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatchOuter
(
    INodeListenerRef const& pTarget,
    OUString const& sTargetPath,
    Change const& rBaseChange,
    OUString const& sChangeContext,
    sal_Bool , //_bError,
    IConfigBroadcaster* pSource
)
{
    ConfigurationName sChangesRoot(sChangeContext,rBaseChange.getNodeName());

    OSL_ASSERT(pTarget.isValid());
    OSL_ASSERT( sChangesRoot.isNestedIn( sTargetPath ) );

    pTarget->nodeChanged(rBaseChange, sChangesRoot.fullName(), pSource);
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatch(TreeChangeList const& rList_, sal_Bool _bError, IConfigBroadcaster* pSource)
{
    dispatch(rList_.root, rList_.pathToRoot.fullName(),_bError, pSource);
}
/////////////////////////////////////////////////////////////////////////
namespace
{
    struct DispatchTarget
    {
        INodeListenerRef pTarget;
        OUString sDispatchPath;
    };
    typedef std::vector<DispatchTarget> DispatchTargets;
}
/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatch
(
    Change const& rBaseChange,
    OUString const& sChangeContext,
    sal_Bool _bError,
    IConfigBroadcaster* pSource
)
{
    // listeners registered under multiple sub-pathes will be called multiple times !

    ConfigurationName aRootName(sChangeContext);
    OSL_ASSERT(!aRootName.isRelative());

    ConfigurationName aNodeName(aRootName, rBaseChange.getNodeName());
    OUString aBasePath( aNodeName.fullName() );
    OSL_ASSERT(!aNodeName.isRelative());

    OSL_ASSERT(aNodeName.getParentName() == aRootName);

    // Collect the targets
    osl::ClearableMutexGuard aGuard(m_aListeners.mutex);

    // Dispatch listeners to ancestors of the change root
    DispatchTargets aOuterTargets;
    {

        PathMap::const_iterator itOuter = m_aPathMap.lower_bound( ConfigurationName::rootname() += aRootName.moduleName() );
        PathMap::const_iterator const endOuter = m_aPathMap.upper_bound(aRootName.fullName());

        // TODO: Both loops are so similar - they should be a single function
        while (itOuter != endOuter)
        {
            OSL_ASSERT( m_aListeners.find(itOuter->second->get()) != m_aListeners.end() );

            OUString aDispatchPath = itOuter->first;
            OSL_ASSERT( ! ConfigurationName(aDispatchPath).isRelative() );

            // check whether this should be dispatched at all
            if (aBasePath == aDispatchPath || aNodeName.isNestedIn(aDispatchPath))
            {
                DispatchTarget aTarget;
                aTarget.sDispatchPath = aDispatchPath;
                aTarget.pTarget = itOuter->second->get();

                aOuterTargets.push_back(aTarget);
            }
            ++itOuter;
        }
    }

    // Dispatch listeners to descendants of the change root
    DispatchTargets aInnerTargets;
    {
        PathMap::const_iterator itInner = m_aPathMap.lower_bound(aBasePath);

        while(  itInner != m_aPathMap.end() &&
                0 == aBasePath.compareTo(itInner->first, aBasePath.getLength()))
        {
            OSL_ASSERT( m_aListeners.find(itInner->second->get()) != m_aListeners.end() );

            OUString aDispatchPath = itInner->first;
            OSL_ASSERT( ! ConfigurationName(aDispatchPath).isRelative() );

            // check whether this should be dispatched at all
            if (aBasePath == aDispatchPath || ConfigurationName(aDispatchPath).isNestedIn(aRootName))
            {
                DispatchTarget aTarget;
                aTarget.sDispatchPath = aDispatchPath;
                aTarget.pTarget = itInner->second->get();

                aInnerTargets.push_back(aTarget);
            }
            ++itInner;
        }
    }

    aGuard.clear();

    {for (DispatchTargets::const_iterator it = aOuterTargets.begin(); it != aOuterTargets.end(); ++it){
        this->dispatchOuter(it->pTarget, it->sDispatchPath, rBaseChange, sChangeContext, _bError, pSource);
    }}
    {for (DispatchTargets::const_iterator it = aInnerTargets.begin(); it != aInnerTargets.end(); ++it){
        this->dispatchInner(it->pTarget, it->sDispatchPath, rBaseChange, sChangeContext, _bError, pSource);
    }}
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::disposing(IConfigBroadcaster* pSource)
{
    osl::ClearableMutexGuard aGuard(m_aListeners.mutex);
    m_aPathMap.clear();

    aGuard.clear();
    m_aListeners.disposing(pSource);
}

/////////////////////////////////////////////////////////////////////////
/*      class ConfigMessageBroadcasterImpl
        {
        public:
        private:
            typedef BroadcastImplHelper<INodeListener*> Listeners;
            Listeners m_aListeners;
        };
*/
/////////////////////////////////////////////////////////////////////////
/*void ConfigMessageBroadcasterImpl::add(IMessageHandler* pListener)
{
    osl::MutexGuard aGuard(m_aListeners.mutex);

    m_aListeners.addListener(pListener);
}

/////////////////////////////////////////////////////////////////////////
void ConfigMessageBroadcasterImpl::remove(IMessageHandler* pListener)
{
    osl::MutexGuard aGuard(m_aListeners.mutex);

    m_aListeners.removeListener(pListener);
}

/////////////////////////////////////////////////////////////////////////
void ConfigMessageBroadcasterImpl::dispatch(OUString const& _rNotifyReason, sal_Int32 _nNotificationId, IConfigBroadcaster* pSource)
{
    osl::MutexGuard aGuard(m_aListeners.mutex);

    for (Listeners::Iterator it = m_aListeners.begin(); it != m_aListeners.end(); )
    {
        // incrementing here allows a listener to remove itself from within the callback

        // it is illegal to cause removal of another listener from the callback
        // if this occurs (dereferencing, incrementing or comparing 'it' fails)
        // we need to explicitly guard against that (which is really too expensive)

        IMessageHandler* pHandler = *it;
        ++it;

        if (pHandler)
            pHandler->message(_rNotifyReason,_nNotificationId,pSource);
    }

}

/////////////////////////////////////////////////////////////////////////
void ConfigMessageBroadcasterImpl::disposing(IConfigBroadcaster* pSource)
{
    osl::MutexGuard aGuard(m_aListeners.mutex);

    m_aListeners.disposing(pSource);
}
*/
/////////////////////////////////////////////////////////////////////////
    } // namespace
} // namespace




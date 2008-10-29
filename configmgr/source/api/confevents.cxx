/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confevents.cxx,v $
 * $Revision: 1.11 $
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
#include <string.h>
#include "confevents.hxx"
#include "configexcept.hxx"
#include "treechangelist.hxx"
#include <osl/diagnose.h>
namespace configmgr
{
    /////////////////////////////////////////////////////////////////////////
    ConfigChangeBroadcastHelper::ConfigChangeBroadcastHelper()
    {
    }

    ConfigChangeBroadcastHelper::~ConfigChangeBroadcastHelper()
    {
        OSL_ENSURE(m_aListeners.begin() == m_aListeners.end(), "Remaining listeners found - forgot to dispose ?");
        OSL_ENSURE(m_aPathMap.empty(), "Spurious mappings found");
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcastHelper::dispose(TreeManager * pSource)
    {
        disposing(pSource);
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcastHelper::addListener(configuration::AbsolutePath const& aName, rtl::Reference<INodeListener> const& pHandler)
    {
        add(aName, pHandler);
    }

    void ConfigChangeBroadcastHelper::removeListener(rtl::Reference<INodeListener> const& pHandler)
    {
        remove(pHandler);
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcastHelper::broadcast(TreeChangeList const& anUpdate, sal_Bool bError, TreeManager * pSource)
    {
        dispatch(anUpdate, bError, pSource);
    }

/////////////////////////////////////////////////////////////////////////
void ConfigChangeBroadcastHelper::add(configuration::AbsolutePath const& aName, rtl::Reference<INodeListener> const& pListener)
{
    osl::MutexGuard aGuard(m_aListeners.mutex);

    internal::BroadcastImplHelper<internal::NodeListenerInfo>::Interfaces::iterator aAdded = m_aListeners.addListener(internal::NodeListenerInfo(pListener));
    aAdded->addPath(aName);
    m_aPathMap.insert(PathMap::value_type(aName,aAdded));
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangeBroadcastHelper::remove(rtl::Reference<INodeListener> const& pListener)
{
    osl::MutexGuard aGuard(m_aListeners.mutex);

    internal::BroadcastImplHelper<internal::NodeListenerInfo>::Interfaces::const_iterator const iter = m_aListeners.find(pListener);
    if (iter != m_aListeners.end())
    {
        internal::NodeListenerInfo::Pathes const& pathes = iter->pathList();

        // first clear the Path Map
        for(internal::NodeListenerInfo::Pathes::iterator itPath = pathes.begin(); itPath != pathes.end(); ++itPath)
        {
            std::pair<PathMap::iterator, PathMap::iterator> aRange = m_aPathMap.equal_range(*itPath);
            while (aRange.first != aRange.second)
            {
                PathMap::iterator cur = aRange.first++;
                if (cur->second == iter)
                    m_aPathMap.erase(cur);
            }
        }

        // the remove the broadcast helper entry
        m_aListeners.removeListener(pListener);
    }
}

/////////////////////////////////////////////////////////////////////////
// This should actually be available from the TreeChangeList
/////////////////////////////////////////////////////////////////////////

static Change const* resolvePath(Change const& rChange, configuration::RelativePath& aRelativePath, RemoveNode const*& pRemoveNode)
{
    std::vector<configuration::Path::Component>::const_reverse_iterator aIter;

    OSL_ASSERT(pRemoveNode == NULL);
    pRemoveNode = NULL;

    Change const* pChange = &rChange;
    pRemoveNode = dynamic_cast<RemoveNode const*>(pChange);

    std::vector<configuration::Path::Component>::const_reverse_iterator const aEnd(aRelativePath.end());

    for( aIter = aRelativePath.begin();
         aIter != aEnd;
         ++aIter)
    {
        OSL_ASSERT( pChange != NULL );

        pChange = pChange->getSubChange(aIter->getName());

        if (pChange == NULL) break;

        OSL_ASSERT(pRemoveNode == NULL);
        OSL_ASSERT(aIter->getName() == pChange->getNodeName());

        pRemoveNode = dynamic_cast<RemoveNode const*>(pChange);
    }

    if (pRemoveNode)
    {
        aRelativePath = configuration::RelativePath( configuration::Path::Rep(aRelativePath.begin(),aIter) );
        OSL_ASSERT( aRelativePath.getLocalName().getName() == pRemoveNode->getNodeName());
    }
    else
        OSL_ASSERT( pChange == 0 || configuration::matches(aRelativePath, configuration::RelativePath( configuration::Path::Rep(aRelativePath.begin(),aIter) )) );

    return pChange;
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangeBroadcastHelper::dispatchInner
(
    rtl::Reference<INodeListener> const& pTarget,
    configuration::AbsolutePath const& _aTargetPath,
    Change const& rBaseChange,
    configuration::AbsolutePath const& _aChangeLocation,
    sal_Bool , //_bError,
    TreeManager * pSource
)
{
    try
    {
        OSL_ASSERT(pTarget.is());
        OSL_ASSERT( configuration::Path::hasPrefix( _aTargetPath, _aChangeLocation ) );

        configuration::RelativePath aLocalPath = configuration::Path::stripPrefix( _aTargetPath, _aChangeLocation );

        RemoveNode const* pRemoved = 0;
        Change const* pTargetChange = resolvePath(rBaseChange, aLocalPath, pRemoved );

        OSL_ASSERT( !pTargetChange || matches(_aChangeLocation.compose(aLocalPath),_aTargetPath) );

        if (pRemoved)
            pTarget->nodeDeleted(_aChangeLocation.compose(aLocalPath), pSource);

        else if (pTargetChange)
            pTarget->nodeChanged(*pTargetChange, _aTargetPath, pSource);

    }
    catch (configuration::InvalidName& )
    {
        OSL_ENSURE(false,"ConfigChangeBroadcastHelper: Could not dispatch notification: context path mismatch");
    }
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangeBroadcastHelper::dispatchOuter
(
    rtl::Reference<INodeListener> const& pTarget,
    configuration::AbsolutePath const& _aTargetPath,
    Change const& rBaseChange,
    configuration::AbsolutePath const& _aChangeLocation,
    sal_Bool , //_bError,
    TreeManager * pSource
)
{
    { (void)_aTargetPath; }
    OSL_ASSERT(pTarget.is());
    OSL_ASSERT( configuration::Path::hasPrefix( _aChangeLocation, _aTargetPath) );

    pTarget->nodeChanged(rBaseChange, _aChangeLocation, pSource);
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangeBroadcastHelper::dispatch(TreeChangeList const& rList_, sal_Bool _bError, TreeManager * pSource)
{
    dispatch(rList_.root, rList_.getRootNodePath(),_bError, pSource);
}

/////////////////////////////////////////////////////////////////////////
namespace
{
    struct DispatchTarget
    {
        DispatchTarget(rtl::Reference<INodeListener> _pTarget, configuration::AbsolutePath const* _pDispatchPath)
        : pTarget(_pTarget), pDispatchPath( _pDispatchPath) {}

        rtl::Reference<INodeListener>    pTarget;
        configuration::AbsolutePath const* pDispatchPath;
    };
}
/////////////////////////////////////////////////////////////////////////
void ConfigChangeBroadcastHelper::dispatch
(
    Change const& rBaseChange,
    configuration::AbsolutePath const& _aChangeLocation,
    sal_Bool _bError,
    TreeManager * pSource
)
{
    OSL_ENSURE(!_aChangeLocation.isRoot(),"Cannot dispatch changes directly to the root node");

    // listeners registered under multiple sub-pathes will be called multiple times !

    // Collect the targets
    osl::ClearableMutexGuard aGuard(m_aListeners.mutex);

    // Dispatch listeners to ancestors of the change root
    std::vector<DispatchTarget> aOuterTargets;
    if (_aChangeLocation.getDepth() > 1)
    {
        configuration::AbsolutePath const aModulePath( configuration::Path::Rep(*_aChangeLocation.begin()) );

        PathMap::const_iterator itOuter = m_aPathMap.lower_bound( aModulePath );
        PathMap::const_iterator const endOuter = m_aPathMap.upper_bound(_aChangeLocation.getParentPath());

        // TODO: Both loops are so similar - they should be a single function
        while (itOuter != endOuter)
        {
            OSL_ASSERT( m_aListeners.find(itOuter->second->get()) != m_aListeners.end() );

            // check whether this should be dispatched at all
            if ( configuration::Path::hasPrefix(_aChangeLocation,itOuter->first) )
            {
                aOuterTargets.push_back( DispatchTarget(itOuter->second->get(), &itOuter->first) );
            }
            ++itOuter;
        }
    }

    // Dispatch listeners to descendants of the change root
    std::vector<DispatchTarget> aInnerTargets;
    {
        PathMap::const_iterator itInner = m_aPathMap.lower_bound(_aChangeLocation);

        while(  itInner != m_aPathMap.end() && configuration::Path::hasPrefix(itInner->first,_aChangeLocation) )
        {
            OSL_ASSERT( m_aListeners.find(itInner->second->get()) != m_aListeners.end() );

            aInnerTargets.push_back( DispatchTarget(itInner->second->get(), &itInner->first) );

            ++itInner;
        }
    }

    aGuard.clear();

    {for (std::vector<DispatchTarget>::const_iterator it = aOuterTargets.begin(); it != aOuterTargets.end(); ++it){
        this->dispatchOuter(it->pTarget, *it->pDispatchPath, rBaseChange, _aChangeLocation, _bError, pSource);
    }}
    {for (std::vector<DispatchTarget>::const_iterator it = aInnerTargets.begin(); it != aInnerTargets.end(); ++it){
        this->dispatchInner(it->pTarget, *it->pDispatchPath, rBaseChange, _aChangeLocation, _bError, pSource);
    }}
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangeBroadcastHelper::disposing(TreeManager * pSource)
{
    osl::ClearableMutexGuard aGuard(m_aListeners.mutex);
    m_aPathMap.clear();

    aGuard.clear();
    m_aListeners.disposing(pSource);
}

} // namespace




/*************************************************************************
 *
 *  $RCSfile: confeventhelpers.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:23 $
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
#include <string.h>
#include "confeventhelpers.hxx"

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
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
    using namespace configuration;
    namespace Path = configuration::Path;

/////////////////////////////////////////////////////////////////////////
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
void ConfigChangesBroadcasterImpl::add(AbsolutePath const& aName, INodeListenerRef const& pListener)
{
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

/////////////////////////////////////////////////////////////////////////
// This should actually be available from the TreeChangeList
/////////////////////////////////////////////////////////////////////////

static Change const* resolvePath(Change const& rChange, RelativePath& aRelativePath, RemoveNode const*& pRemoveNode)
{
    OSL_ASSERT(pRemoveNode == NULL);
    pRemoveNode = NULL;

    Change const* pChange = &rChange;
    if (rChange.ISA(RemoveNode))
        pRemoveNode = static_cast<RemoveNode const*>(pChange);

    RelativePath::Iterator const aEnd(aRelativePath.end());

    for( RelativePath::Iterator aIter = aRelativePath.begin();
         aIter != aEnd;
         ++aIter)
    {
        OSL_ASSERT( pChange != NULL );

        pChange = pChange->getSubChange(aIter->getName().toString());

        if (pChange == NULL) break;

        OSL_ASSERT(pRemoveNode == NULL);
        OSL_ASSERT(aIter->getName().toString() == pChange->getNodeName());

        if (pChange->ISA(RemoveNode))
            pRemoveNode = static_cast<RemoveNode const*>(pChange);
    }

    if (pRemoveNode)
    {
        aRelativePath = RelativePath( Path::Rep(aRelativePath.begin(),aIter) );
        OSL_ASSERT( aRelativePath.getLocalName().getName().toString() == pRemoveNode->getNodeName());
    }
    else
        OSL_ASSERT( pChange == 0 || configuration::matches(aRelativePath, RelativePath( Path::Rep(aRelativePath.begin(),aIter) )) );

    return pChange;
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatchInner
(
    INodeListenerRef const& pTarget,
    AbsolutePath const& _aTargetPath,
    memory::Accessor const& _aChangedDataAccessor,
    Change const& rBaseChange,
    AbsolutePath const& _aChangeLocation,
    sal_Bool , //_bError,
    IConfigBroadcaster* pSource
)
{
    using namespace configuration;
    try
    {
        OSL_ASSERT(pTarget.is());
        OSL_ASSERT( Path::hasPrefix( _aTargetPath, _aChangeLocation ) );

        RelativePath aLocalPath = Path::stripPrefix( _aTargetPath, _aChangeLocation );

        RemoveNode const* pRemoved = 0;
        Change const* pTargetChange = resolvePath(rBaseChange, aLocalPath, pRemoved );

        OSL_ASSERT( !pTargetChange || matches(_aChangeLocation.compose(aLocalPath),_aTargetPath) );

        if (pRemoved)
            pTarget->nodeDeleted(_aChangedDataAccessor,_aChangeLocation.compose(aLocalPath), pSource);

        else if (pTargetChange)
            pTarget->nodeChanged(_aChangedDataAccessor,*pTargetChange, _aTargetPath, pSource);

    }
    catch (InvalidName& )
    {
        OSL_ENSURE(false,"ConfigChangesBroadcasterImpl: Could not dispatch notification: context path mismatch");
    }
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatchOuter
(
    INodeListenerRef const& pTarget,
    AbsolutePath const& _aTargetPath,
    memory::Accessor const& _aChangedDataAccessor,
    Change const& rBaseChange,
    AbsolutePath const& _aChangeLocation,
    sal_Bool , //_bError,
    IConfigBroadcaster* pSource
)
{
    OSL_ASSERT(pTarget.is());
    OSL_ASSERT( Path::hasPrefix( _aChangeLocation, _aTargetPath) );

    pTarget->nodeChanged(_aChangedDataAccessor, rBaseChange, _aChangeLocation, pSource);
}

/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatch(memory::Accessor const& _aChangedDataAccessor, TreeChangeList const& rList_, sal_Bool _bError, IConfigBroadcaster* pSource)
{
    dispatch(_aChangedDataAccessor, rList_.root, rList_.getRootNodePath(),_bError, pSource);
}
/////////////////////////////////////////////////////////////////////////
namespace
{
    struct DispatchTarget
    {
        DispatchTarget(INodeListenerRef _pTarget, AbsolutePath const* _pDispatchPath)
        : pTarget(_pTarget), pDispatchPath( _pDispatchPath) {}

        INodeListenerRef    pTarget;
        AbsolutePath const* pDispatchPath;
    };
    typedef std::vector<DispatchTarget> DispatchTargets;
}
/////////////////////////////////////////////////////////////////////////
void ConfigChangesBroadcasterImpl::dispatch
(
    memory::Accessor const& _aChangedDataAccessor,
    Change const& rBaseChange,
    AbsolutePath const& _aChangeLocation,
    sal_Bool _bError,
    IConfigBroadcaster* pSource
)
{
    OSL_ENSURE(!_aChangeLocation.isRoot(),"Cannot dispatch changes directly to the root node");

    // listeners registered under multiple sub-pathes will be called multiple times !

    // Collect the targets
    osl::ClearableMutexGuard aGuard(m_aListeners.mutex);

    // Dispatch listeners to ancestors of the change root
    DispatchTargets aOuterTargets;
    if (_aChangeLocation.getDepth() > 1)
    {
        AbsolutePath const aModulePath( Path::Rep(*_aChangeLocation.begin()) );

        PathMap::const_iterator itOuter = m_aPathMap.lower_bound( aModulePath );
        PathMap::const_iterator const endOuter = m_aPathMap.upper_bound(_aChangeLocation.getParentPath());

        // TODO: Both loops are so similar - they should be a single function
        while (itOuter != endOuter)
        {
            OSL_ASSERT( m_aListeners.find(itOuter->second->get()) != m_aListeners.end() );

            // check whether this should be dispatched at all
            if ( Path::hasPrefix(_aChangeLocation,itOuter->first) )
            {
                aOuterTargets.push_back( DispatchTarget(itOuter->second->get(), &itOuter->first) );
            }
            ++itOuter;
        }
    }

    // Dispatch listeners to descendants of the change root
    DispatchTargets aInnerTargets;
    {
        PathMap::const_iterator itInner = m_aPathMap.lower_bound(_aChangeLocation);

        while(  itInner != m_aPathMap.end() && Path::hasPrefix(itInner->first,_aChangeLocation) )
        {
            OSL_ASSERT( m_aListeners.find(itInner->second->get()) != m_aListeners.end() );

            aInnerTargets.push_back( DispatchTarget(itInner->second->get(), &itInner->first) );

            ++itInner;
        }
    }

    aGuard.clear();

    {for (DispatchTargets::const_iterator it = aOuterTargets.begin(); it != aOuterTargets.end(); ++it){
        this->dispatchOuter(it->pTarget, *it->pDispatchPath, _aChangedDataAccessor, rBaseChange, _aChangeLocation, _bError, pSource);
    }}
    {for (DispatchTargets::const_iterator it = aInnerTargets.begin(); it != aInnerTargets.end(); ++it){
        this->dispatchInner(it->pTarget, *it->pDispatchPath, _aChangedDataAccessor, rBaseChange, _aChangeLocation, _bError, pSource);
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




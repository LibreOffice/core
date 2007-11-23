/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cacheaccess.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:34:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "cacheaccess.hxx"

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{
    using namespace configuration;

// -------------------------------------------------------------------------

CacheClientAccess::CacheClientAccess(ConfigChangeBroadcastHelper *  _pBroadcastHelper)
: m_pBroadcastHelper( _pBroadcastHelper )
{
}
// -------------------------------------------------------------------------

CacheClientAccess::~CacheClientAccess()
{
    OSL_ENSURE(!m_pBroadcastHelper, "Forgot to dispose broadcast helper");
}
// -------------------------------------------------------------------------

ConfigChangeBroadcastHelper *  CacheClientAccess::releaseBroadcaster()
{
    ConfigChangeBroadcastHelper * pRet = m_pBroadcastHelper;
    m_pBroadcastHelper = NULL;
    return pRet;
}

// -------------------------------------------------------------------------

bool CacheClientAccess::hasModule(const CacheLine::Path& _aLocation)
{
    return this->m_aData.hasModule(_aLocation.getModuleName());
}
// -------------------------------------------------------------------------

bool CacheClientAccess::hasModuleDefaults(CacheLine::Path const& _aLocation)
{
    return this->m_aData.hasModuleDefaults(_aLocation.getModuleName());
}
// -------------------------------------------------------------------------
void CacheClientAccess::attachModule(data::TreeAddress _aLocation, CacheLine::Name const & _aModule)
{
    this->m_aData.attachModule(_aLocation, _aModule);
}
// -------------------------------------------------------------------------

data::NodeAddress CacheClientAccess::acquireNode(CacheLine::Path const& rLocation )
{
    CFG_TRACE_INFO("CacheClientAccess: Requesting data for path '%s'", OUSTRING2ASCII(rLocation.toString()) );

    data::NodeAddress aResult = this->m_aData.acquireNode(rLocation);

    if (aResult != NULL)
    {
        CFG_TRACE_INFO_NI("- Data is available - returning Subtree");
    }
    else
        CFG_TRACE_INFO_NI("- Data is not available - returning NULL");

    return aResult;
}
// -------------------------------------------------------------------------

oslInterlockedCount CacheClientAccess::releaseNode( CacheLine::Path const& rLocation )
{
    CFG_TRACE_INFO("Tree Info: Releasing subtree data for path '%s'", OUSTRING2ASCII(rLocation.toString()) );

    oslInterlockedCount nRet = this->m_aData.releaseModule(rLocation.getModuleName(),false);

    return nRet;
}
// -----------------------------------------------------------------------------

void CacheClientAccess::applyUpdate(backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( )
{
    CFG_TRACE_INFO("CacheClientAccess: Merging changes into subtree '%s'", OUSTRING2ASCII(_aUpdate.root().toString()) );

    this->m_aData.applyUpdate(_aUpdate );
}

// -----------------------------------------------------------------------------
data::NodeAddress CacheClientAccess::findInnerNode( CacheLine::Path const& aComponentName )
{
    data::NodeAddress aNode = this->m_aData.getNode(aComponentName);

    if (aNode != NULL && data::NodeAccess(aNode)->isValue() )
        aNode = data::NodeAddress();

    return aNode;
}

// -------------------------------------------------------------------------

bool CacheClientAccess::insertDefaults( backend::NodeInstance const & _aDefaultData ) CFG_UNO_THROW_RTE(  )
{
    CFG_TRACE_INFO("Tree Info: Adding default data for path '%s'", OUSTRING2ASCII(_aDefaultData.root().toString()) );

    return this->m_aData.insertDefaults(_aDefaultData);
}
// -------------------------------------------------------------------------

bool CacheClientAccess::isEmpty()
{
    CacheData::ModuleList& rModules = this->m_aData.accessModuleList();

    bool bRet = rModules.empty();

    return bRet;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

CacheLoadingAccess::CacheLoadingAccess()
: m_aDeadModules()
{
}
// -------------------------------------------------------------------------

CacheLoadingAccess::~CacheLoadingAccess()
{
}

// -------------------------------------------------------------------------

/// gets a tree reference for the given path if exists
data::TreeAddress CacheLoadingAccess::getTreeAddress(CacheLine::Name const & _aModule)
{
    return this->m_aData.getTreeAddress(_aModule);
}
// -------------------------------------------------------------------------
void CacheLoadingAccess::createModule(CacheLine::Name const & _aModule)
{
    this->m_aData.createModule(_aModule);
}
// -------------------------------------------------------------------------
bool CacheLoadingAccess::hasModule(CacheLine::Name const & _aModule)
{
    return this->m_aData.hasModule(_aModule);
}
// -------------------------------------------------------------------------

bool CacheLoadingAccess::acquireModule(CacheLine::Name const & _aModule)
{
    CFG_TRACE_INFO("Tree Info: Requesting data for module '%s'", OUSTRING2ASCII(_aModule.toString()));

    if (this->m_aData.acquireModule(_aModule))
    {
    m_aDeadModules.erase( _aModule );
    CFG_TRACE_INFO_NI("- Data is available - returning Subtree");
    return true;
    }
    else
    {
    CFG_TRACE_INFO_NI("- Data is not available - returning NULL");
    return false;
    }
}
// -------------------------------------------------------------------------

oslInterlockedCount CacheLoadingAccess::releaseModule( CacheLine::Name const & _aModule )
{
    CFG_TRACE_INFO("Tree Info: Releasing data for module '%s'", OUSTRING2ASCII(_aModule.toString()) );

    oslInterlockedCount nRet = this->m_aData.releaseModule(_aModule,true); // keep
    if (nRet == 0)
    {
        m_aDeadModules[ _aModule ] = TimeStamp::getCurrentTime();
        CFG_TRACE_INFO_NI("- Last reference released - marking data for cleanup");
    }

    return nRet;
}
// -----------------------------------------------------------------------------

void CacheLoadingAccess::applyUpdate(backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( )
{
    CFG_TRACE_INFO("CacheLoadingAccess: Merging changes into subtree '%s'", OUSTRING2ASCII(_aUpdate.root().toString()) );

    this->m_aData.applyUpdate(_aUpdate);
}

// -----------------------------------------------------------------------------
data::NodeAddress CacheLoadingAccess::findNode( CacheLine::Path const& aComponentName )
{
    data::NodeAddress aNode = this->m_aData.getNode(aComponentName);

    return aNode;
}

// -------------------------------------------------------------------------

bool CacheLoadingAccess::isEmpty()
{
    ExtendedCacheData::ModuleList& rModules = this->m_aData.accessModuleList();

    bool bRet = rModules.empty();

    if (bRet) // while we are at it - clean up
        m_aDeadModules.clear();

    return bRet;
}
// -------------------------------------------------------------------------

data::TreeAddress CacheLoadingAccess::addComponentData( backend::ComponentInstance const & _aComponentInstance,
                                                        bool _bIncludesDefaults
                                                       ) CFG_UNO_THROW_RTE()
{
    CFG_TRACE_INFO("CacheLoadingAccess: Adding component data for module '%s' : %s",
                    OUSTRING2ASCII(_aComponentInstance.component().toString()),
                    _bIncludesDefaults ? "Data includes defaults." : "Data does not include defaults." );

    data::TreeAddress aResult = this->m_aData.addComponentData(_aComponentInstance, _bIncludesDefaults);
    if (aResult != NULL)
    {
        m_aDeadModules.erase( _aComponentInstance.component() );
        CFG_TRACE_INFO_NI("- Data added successfully - returning Subtree");
    }
    else
        CFG_TRACE_INFO_NI("- Data not added - returning NULL");

    return aResult;
}
// -------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void CacheLoadingAccess::addChangesToPending( backend::ConstUpdateInstance const& _anUpdate ) CFG_UNO_THROW_RTE(  )
{
    // NICE: m_pPending[_rLocation] += pSubtreeChange;
    CFG_TRACE_INFO("CacheLoadingAccess: Adding pending changes for subtree '%s'", OUSTRING2ASCII(_anUpdate.root().toString()) );

    this->m_aData.addPending(_anUpdate);
}

// -----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> CacheLoadingAccess::releasePendingChanges(CacheLine::Name const& _aComponentName)
{
    CFG_TRACE_INFO("Tree Info: extract pending changes from subtree '%s'", OUSTRING2ASCII(_aComponentName.toString()) );
    return this->m_aData.releasePending(_aComponentName);
}

// -----------------------------------------------------------------------------
bool CacheLoadingAccess::findPendingChangedModules( ExtendedCacheData::PendingModuleList & _rPendingList )
{
    this->m_aData.findPendingModules(_rPendingList);
    return !_rPendingList.empty();
}

// -----------------------------------------------------------------------------
void CacheLoadingAccess::clearData(DisposeList& _rList) CFG_NOTHROW()
{
    CFG_TRACE_INFO("Tree Info: Removing all module trees for cleanup" );

    typedef ExtendedCacheData::ModuleList ModuleList;

    ModuleList& rModules = this->m_aData.accessModuleList();

    for(ModuleList::iterator it = rModules.begin();
        it != rModules.end();
        ++it)
    {
        if (it->second.is())
            _rList.push_back(it->second);
    }

    rModules.clear();
    m_aDeadModules.clear();
}
// -------------------------------------------------------------------------

TimeStamp CacheLoadingAccess::collectDisposeList(CacheLoadingAccess::DisposeList & _rList, TimeStamp const & _aLimitTime, TimeInterval const & _aDelay)
{
    TimeStamp aRetTime = TimeStamp::never();

    CFG_TRACE_INFO("Tree Info: Collecting disposable module trees for cleanup" );

    ExtendedCacheData::ModuleList& rActiveModules = this->m_aData.accessModuleList();

    DeadModuleList::iterator it = m_aDeadModules.begin();

    while (it != m_aDeadModules.end())
    {
        DeadModuleList::iterator current = it;
        // increment here, as we may later erase(current)
        ++it;

#if (OSL_DEBUG_LEVEL > 0) || defined _DBG_UTIL || defined CFG_TRACE_ENABLE
        OUString sCurrentName( current->first.toString() );
#endif
        TimeStamp aExpireTime = current->second + _aDelay;
        if (aExpireTime <= _aLimitTime)
        {
            ExtendedCacheData::ModuleList::iterator itModule = rActiveModules.find( current->first );

            if (itModule != rActiveModules.end())
            {
                CacheLineRef xModule = itModule->second;

                bool bHandled = false;

                if (!xModule.is())
                {
                    CFG_TRACE_ERROR_NI("- Unexpected: Module '%s' is NULL in active module list", OUSTRING2ASCII(sCurrentName) );
                    bHandled = true;
                }
                else if (xModule->clientReferences() != 0)// at least in temporary use
                {
                    OSL_ENSURE( false, "Referenced entry in dead module list");

                    CFG_TRACE_WARNING_NI("- Module '%s' in (temporary ?) use - rescheduling", OUSTRING2ASCII(sCurrentName) );
                    bHandled = false; // still remove from the lists
                }
                else if (m_aData.hasPending(current->first))
                {
                    CFG_TRACE_WARNING_NI("- Module '%s' has pending changes - rescheduling disposal", OUSTRING2ASCII(sCurrentName) );
                    bHandled = false;
                }
                else // now this really can be disposed
                {
                    CFG_TRACE_INFO_NI("- Removing module '%s' for disposal", OUSTRING2ASCII(sCurrentName) );

                    // It really is ok to dispose this entry
                    _rList.push_back(xModule);

                    bHandled = true;
                }


                if (bHandled)
                {
                    // really remove
                    rActiveModules.erase(itModule);
                    m_aDeadModules.erase(current);
                }
                else
                {
                    // reschedule
                    TimeStamp aRetryTime = _aLimitTime + _aDelay;
                    OSL_ASSERT(aRetryTime > _aLimitTime);

                    current->second = _aLimitTime; // ?
                    if (aRetryTime < aRetTime)
                        aRetTime = aRetryTime;
                }
            }
            else
            {
                // obsolete dispose list entry - discard
                OSL_ENSURE( false, "Obsolete entry in dead module list");

                CFG_TRACE_WARNING_NI("- Module '%s' not found any more - obsolete entry in dead module list", OUSTRING2ASCII(sCurrentName) );

                m_aDeadModules.erase(current);
            }
        }
        else // consider for restart time
        {
            CFG_TRACE_INFO_NI("- Module '%s' has not expired yet - rescheduling", OUSTRING2ASCII(sCurrentName) );

            if (aExpireTime < aRetTime)
                aRetTime = aExpireTime;
        }
    }

    OSL_ASSERT(aRetTime > _aLimitTime);
    return aRetTime;
}
// -------------------------------------------------------------------------

} // namespace configmgr



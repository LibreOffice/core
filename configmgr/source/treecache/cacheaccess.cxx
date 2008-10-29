/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cacheaccess.cxx,v $
 * $Revision: 1.12 $
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

#include "cacheaccess.hxx"
#include "tracer.hxx"
#include "configpath.hxx"

namespace configmgr
{
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

bool CacheClientAccess::hasModule(const configuration::AbsolutePath& _aLocation)
{
    return this->m_aData.hasModule(_aLocation.getModuleName());
}
// -------------------------------------------------------------------------

bool CacheClientAccess::hasModuleDefaults(configuration::AbsolutePath const& _aLocation)
{
    return this->m_aData.hasModuleDefaults(_aLocation.getModuleName());
}
// -------------------------------------------------------------------------
void CacheClientAccess::attachModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aModule)
{
    this->m_aData.attachModule(_aLocation, _aModule);
}
// -------------------------------------------------------------------------

sharable::Node * CacheClientAccess::acquireNode(configuration::AbsolutePath const& rLocation )
{
    CFG_TRACE_INFO("CacheClientAccess: Requesting data for path '%s'", OUSTRING2ASCII(rLocation.toString()) );

    sharable::Node * aResult = this->m_aData.acquireNode(rLocation);

    if (aResult != NULL)
    {
        CFG_TRACE_INFO_NI("- Data is available - returning Subtree");
    }
    else
        CFG_TRACE_INFO_NI("- Data is not available - returning NULL");

    return aResult;
}
// -------------------------------------------------------------------------

oslInterlockedCount CacheClientAccess::releaseNode( configuration::AbsolutePath const& rLocation )
{
    CFG_TRACE_INFO("Tree Info: Releasing subtree data for path '%s'", OUSTRING2ASCII(rLocation.toString()) );

    oslInterlockedCount nRet = this->m_aData.releaseModule(rLocation.getModuleName(),false);

    return nRet;
}
// -----------------------------------------------------------------------------

void CacheClientAccess::applyUpdate(backend::UpdateInstance & _aUpdate) SAL_THROW((com::sun::star::uno::RuntimeException))
{
    CFG_TRACE_INFO("CacheClientAccess: Merging changes into subtree '%s'", OUSTRING2ASCII(_aUpdate.root().toString()) );

    this->m_aData.applyUpdate(_aUpdate );
}

// -----------------------------------------------------------------------------
sharable::Node * CacheClientAccess::findInnerNode( configuration::AbsolutePath const& aComponentName )
{
    sharable::Node * node = m_aData.getNode(aComponentName);
    return node == 0 || node->isValue() ? 0 : node;
}

// -------------------------------------------------------------------------

bool CacheClientAccess::insertDefaults( backend::NodeInstance const & _aDefaultData ) SAL_THROW((com::sun::star::uno::RuntimeException))
{
    CFG_TRACE_INFO("Tree Info: Adding default data for path '%s'", OUSTRING2ASCII(_aDefaultData.root().toString()) );

    return this->m_aData.insertDefaults(_aDefaultData);
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
sharable::TreeFragment * CacheLoadingAccess::getTreeAddress(rtl::OUString const & _aModule)
{
    return this->m_aData.getTreeAddress(_aModule);
}
// -------------------------------------------------------------------------
void CacheLoadingAccess::createModule(rtl::OUString const & _aModule)
{
    this->m_aData.createModule(_aModule);
}
// -------------------------------------------------------------------------
bool CacheLoadingAccess::hasModule(rtl::OUString const & _aModule)
{
    return this->m_aData.hasModule(_aModule);
}
// -------------------------------------------------------------------------

bool CacheLoadingAccess::acquireModule(rtl::OUString const & _aModule)
{
    CFG_TRACE_INFO("Tree Info: Requesting data for module '%s'", OUSTRING2ASCII(_aModule));

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

oslInterlockedCount CacheLoadingAccess::releaseModule( rtl::OUString const & _aModule )
{
    CFG_TRACE_INFO("Tree Info: Releasing data for module '%s'", OUSTRING2ASCII(_aModule) );

    oslInterlockedCount nRet = this->m_aData.releaseModule(_aModule,true); // keep
    if (nRet == 0)
    {
        m_aDeadModules[ _aModule ] = TimeStamp::getCurrentTime();
        CFG_TRACE_INFO_NI("- Last reference released - marking data for cleanup");
    }

    return nRet;
}
// -----------------------------------------------------------------------------

bool CacheLoadingAccess::isEmpty()
{
    ExtendedCacheData::ModuleList& rModules = this->m_aData.accessModuleList();

    bool bRet = rModules.empty();

    if (bRet) // while we are at it - clean up
        m_aDeadModules.clear();

    return bRet;
}
// -------------------------------------------------------------------------

sharable::TreeFragment * CacheLoadingAccess::addComponentData( backend::ComponentInstance const & _aComponentInstance,
                                                        bool _bIncludesDefaults
                                                       ) SAL_THROW((com::sun::star::uno::RuntimeException))
{
    CFG_TRACE_INFO("CacheLoadingAccess: Adding component data for module '%s' : %s",
                    OUSTRING2ASCII(_aComponentInstance.component()),
                    _bIncludesDefaults ? "Data includes defaults." : "Data does not include defaults." );

    sharable::TreeFragment * aResult = this->m_aData.addComponentData(_aComponentInstance, _bIncludesDefaults);
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
void CacheLoadingAccess::addChangesToPending( backend::ConstUpdateInstance const& _anUpdate ) SAL_THROW((com::sun::star::uno::RuntimeException))
{
    // NICE: m_pPending[_rLocation] += pSubtreeChange;
    CFG_TRACE_INFO("CacheLoadingAccess: Adding pending changes for subtree '%s'", OUSTRING2ASCII(_anUpdate.root().toString()) );

    this->m_aData.addPending(_anUpdate);
}

// -----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> CacheLoadingAccess::releasePendingChanges(rtl::OUString const& _aComponentName)
{
    CFG_TRACE_INFO("Tree Info: extract pending changes from subtree '%s'", OUSTRING2ASCII(_aComponentName) );
    return this->m_aData.releasePending(_aComponentName);
}

// -----------------------------------------------------------------------------
bool CacheLoadingAccess::findPendingChangedModules( std::vector< rtl::OUString > & _rPendingList )
{
    this->m_aData.findPendingModules(_rPendingList);
    return !_rPendingList.empty();
}

// -----------------------------------------------------------------------------
void CacheLoadingAccess::clearData(std::vector< rtl::Reference<CacheLine> >& _rList) SAL_THROW(())
{
    CFG_TRACE_INFO("Tree Info: Removing all module trees for cleanup" );

    ExtendedCacheData::ModuleList& rModules = this->m_aData.accessModuleList();

    for(ExtendedCacheData::ModuleList::iterator it = rModules.begin();
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

TimeStamp CacheLoadingAccess::collectDisposeList(std::vector< rtl::Reference<CacheLine> > & _rList, TimeStamp const & _aLimitTime, TimeInterval const & _aDelay)
{
    TimeStamp aRetTime = TimeStamp::never();

    CFG_TRACE_INFO("Tree Info: Collecting disposable module trees for cleanup" );

    ExtendedCacheData::ModuleList& rActiveModules = this->m_aData.accessModuleList();

    std::map< rtl::OUString, TimeStamp >::iterator it = m_aDeadModules.begin();

    while (it != m_aDeadModules.end())
    {
        std::map< rtl::OUString, TimeStamp >::iterator current = it;
        // increment here, as we may later erase(current)
        ++it;

#if (OSL_DEBUG_LEVEL > 0) || defined _DBG_UTIL || defined CFG_TRACE_ENABLE
        rtl::OUString sCurrentName( current->first );
#endif
        TimeStamp aExpireTime = current->second + _aDelay;
        if (aExpireTime <= _aLimitTime)
        {
            ExtendedCacheData::ModuleList::iterator itModule = rActiveModules.find( current->first );

            if (itModule != rActiveModules.end())
            {
                rtl::Reference<CacheLine> xModule = itModule->second;

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



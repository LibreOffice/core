/*************************************************************************
 *
 *  $RCSfile: cacheaccess.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:40 $
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

#include "cacheaccess.hxx"

#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_NODEADDRESS_HXX
#include "nodeaddress.hxx"
#endif
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

CacheClientAccess::CacheClientAccess(memory::HeapManager & _rHeapManager,
                                     ConfigChangeBroadcastHelper *  _pBroadcastHelper)
: m_aMutex()
, m_aData(_rHeapManager)
, m_pBroadcastHelper( _pBroadcastHelper )
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
    osl::MutexGuard aGuard(m_aMutex);
    ConfigChangeBroadcastHelper * pRet = m_pBroadcastHelper;
    m_pBroadcastHelper = NULL;
    return pRet;
}
// -------------------------------------------------------------------------

/// gets a data segment reference for the given path - creates if necessary
memory::Segment * CacheLoadingAccess::createNewDataSegment(ModuleName const & _aModule)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.createDataSegment(_aModule);
}
// -------------------------------------------------------------------------

/// gets a data segment reference for the given path - creates if necessary
memory::Segment * CacheClientAccess::attachDataSegment(const memory::SegmentAddress & _aSegment, const Path& _aLocation)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.attachDataSegment(_aSegment,_aLocation.getModuleName());
}
// -------------------------------------------------------------------------

/// gets a data segment reference for the given path if exists
memory::Segment * CacheClientAccess::getDataSegment(const Path& _aLocation)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.getDataSegment(_aLocation.getModuleName());
}
// -------------------------------------------------------------------------

bool CacheClientAccess::hasModule(const Path& _aLocation)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.hasModule(_aLocation.getModuleName());
}
// -------------------------------------------------------------------------

bool CacheClientAccess::hasModuleDefaults(memory::Accessor const& _aAccessor, Path const& _aLocation)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.hasModuleDefaults(_aAccessor, _aLocation.getModuleName());
}
// -------------------------------------------------------------------------

data::NodeAddress CacheClientAccess::acquireNode(memory::Accessor const& _aAccessor, Path const& rLocation )
{
    osl::MutexGuard aGuard( this->m_aMutex );

    CFG_TRACE_INFO("CacheClientAccess: Requesting data for path '%s'", OUSTRING2ASCII(rLocation.toString()) );

    data::NodeAddress aResult = this->m_aData.acquireNode(_aAccessor,rLocation);

    if (aResult.is())
    {
        CFG_TRACE_INFO_NI("- Data is available - returning Subtree");
    }
    else
        CFG_TRACE_INFO_NI("- Data is not available - returning NULL");

    return aResult;
}
// -------------------------------------------------------------------------

CacheLine::RefCount CacheClientAccess::releaseNode( Path const& rLocation )
{
    osl::MutexGuard aGuard( this->m_aMutex );

    CFG_TRACE_INFO("Tree Info: Releasing subtree data for path '%s'", OUSTRING2ASCII(rLocation.toString()) );

    Module::RefCount nRet = this->m_aData.releaseModule(rLocation.getModuleName(),false);

    return nRet;
}
// -----------------------------------------------------------------------------

void CacheClientAccess::applyUpdate(memory::UpdateAccessor& _aUpdateToken,  backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( )
{
    osl::MutexGuard aGuard( this->m_aMutex ); // needed to protect the map access in the Tree

    CFG_TRACE_INFO("CacheClientAccess: Merging changes into subtree '%s'", OUSTRING2ASCII(_aUpdate.root().toString()) );

    this->m_aData.applyUpdate(_aUpdateToken, _aUpdate );
}

// -----------------------------------------------------------------------------
data::NodeAddress CacheClientAccess::findInnerNode( data::Accessor const& _aAccess, Path const& aComponentName )
{
    osl::MutexGuard aGuard( this->m_aMutex ); // needed to protect the map access in the Tree

    data::NodeAddress aNode = this->m_aData.getNode(_aAccess, aComponentName);

    if (aNode.is() && data::NodeAccess(_aAccess,aNode).data().isValue() )
        aNode = data::NodeAddress();

    return aNode;
}

// -------------------------------------------------------------------------

bool CacheClientAccess::insertDefaults( memory::UpdateAccessor& _aAccessToken,
                                        backend::NodeInstance const & _aDefaultData ) CFG_UNO_THROW_RTE(  )
{
    osl::MutexGuard aGuard( this->m_aMutex );

    CFG_TRACE_INFO("Tree Info: Adding default data for path '%s'", OUSTRING2ASCII(_aDefaultData.root().toString()) );

    return this->m_aData.insertDefaults(_aAccessToken, _aDefaultData);
}
// -------------------------------------------------------------------------

bool CacheClientAccess::isEmpty()
{
    osl::MutexGuard aGuard( this->m_aMutex );

    Data::ModuleList& rModules = this->m_aData.accessModuleList();

    bool bRet = rModules.empty();

    return bRet;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

CacheLoadingAccess::CacheLoadingAccess(memory::HeapManager & _rHeapManager)
: m_aMutex()
, m_aData(_rHeapManager)
, m_aDeadModules()
{
}
// -------------------------------------------------------------------------

CacheLoadingAccess::~CacheLoadingAccess()
{
}
// -------------------------------------------------------------------------

/// gets a data segment reference for the given path - creates if necessary
memory::Segment * CacheLoadingAccess::attachDataSegment(const memory::SegmentAddress & _aSegment, ModuleName const & _aModule)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.attachDataSegment(_aSegment,_aModule);
}
// -------------------------------------------------------------------------

/// gets a data segment reference for the given path if exists
memory::Segment * CacheLoadingAccess::getDataSegment(ModuleName const & _aModule)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.getDataSegment(_aModule);
}
// -------------------------------------------------------------------------

/// gets a data segment reference for the given path if exists
memory::SegmentAddress CacheLoadingAccess::getDataSegmentAddress(ModuleName const & _aModule)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.getDataSegmentAddress(_aModule);
}
// -------------------------------------------------------------------------

bool CacheLoadingAccess::hasModule(ModuleName const & _aModule)
{
    osl::MutexGuard aGuard( this->m_aMutex );

    return this->m_aData.hasModule(_aModule);
}
// -------------------------------------------------------------------------

data::TreeAddress CacheLoadingAccess::acquireModule(ModuleName const & _aModule )
{
    osl::MutexGuard aGuard( this->m_aMutex );

    CFG_TRACE_INFO("Tree Info: Requesting data for module '%s'", OUSTRING2ASCII(_aModule.toString()) );

    data::TreeAddress aResult = this->m_aData.acquireModule(_aModule);

    if (aResult.is())
    {
        m_aDeadModules.erase( _aModule );
        CFG_TRACE_INFO_NI("- Data is available - returning Subtree");
    }
    else
        CFG_TRACE_INFO_NI("- Data is not available - returning NULL");

    return aResult;
}
// -------------------------------------------------------------------------

CacheLine::RefCount CacheLoadingAccess::releaseModule( ModuleName const & _aModule )
{
    osl::MutexGuard aGuard( this->m_aMutex );

    CFG_TRACE_INFO("Tree Info: Releasing data for module '%s'", OUSTRING2ASCII(_aModule.toString()) );

    Module::RefCount nRet = this->m_aData.releaseModule(_aModule,true); // keep
    if (nRet == 0)
    {
        m_aDeadModules[ _aModule ] = TimeStamp::getCurrentTime();
        CFG_TRACE_INFO_NI("- Last reference released - marking data for cleanup");
    }

    return nRet;
}
// -----------------------------------------------------------------------------

void CacheLoadingAccess::applyUpdate(memory::UpdateAccessor& _aUpdateToken,  backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( )
{
    osl::MutexGuard aGuard( this->m_aMutex ); // needed to protect the map access in the Tree

    CFG_TRACE_INFO("CacheLoadingAccess: Merging changes into subtree '%s'", OUSTRING2ASCII(_aUpdate.root().toString()) );

    this->m_aData.applyUpdate(_aUpdateToken, _aUpdate);
}

// -----------------------------------------------------------------------------
data::NodeAddress CacheLoadingAccess::findNode( data::Accessor const& _aAccess, Path const& aComponentName )
{
    osl::MutexGuard aGuard( this->m_aMutex ); // needed to protect the map access in the Tree

    data::NodeAddress aNode = this->m_aData.getNode(_aAccess, aComponentName);

    return aNode;
}

// -------------------------------------------------------------------------

bool CacheLoadingAccess::isEmpty()
{
    osl::MutexGuard aGuard( this->m_aMutex );

    Data::ModuleList& rModules = this->m_aData.accessModuleList();

    bool bRet = rModules.empty();

    if (bRet) // while we are at it - clean up
        m_aDeadModules.clear();

    return bRet;
}
// -------------------------------------------------------------------------

data::TreeAddress CacheLoadingAccess::addComponentData( memory::UpdateAccessor& _aAccessToken,
                                                        backend::ComponentInstance const & _aComponentInstance,
                                                        bool _bIncludesDefaults
                                                       ) CFG_UNO_THROW_RTE()
{
    osl::MutexGuard aGuard( this->m_aMutex );
    CFG_TRACE_INFO("CacheLoadingAccess: Adding component data  for module '%s'",
                    OUSTRING2ASCII(_aComponentInstance.component().toString()),
                    _bIncludesDefaults ? "Data includes defaults." : "Data does not include defaults." );

    data::TreeAddress aResult = this->m_aData.addComponentData(_aAccessToken, _aComponentInstance, _bIncludesDefaults);
    if (aResult.is())
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

    osl::MutexGuard aGuard( this->m_aMutex ); // needed to protect the map access in the Data

    CFG_TRACE_INFO("CacheLoadingAccess: Adding pending changes for subtree '%s'", OUSTRING2ASCII(_anUpdate.root().toString()) );

    this->m_aData.addPending(_anUpdate);
}

// -----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> CacheLoadingAccess::releasePendingChanges(ModuleName const& _aComponentName)
{
    osl::MutexGuard aGuard( this->m_aMutex ); // needed to protect the map access in the Data
    CFG_TRACE_INFO("Tree Info: extract pending changes from subtree '%s'", OUSTRING2ASCII(_aComponentName.toString()) );
    return this->m_aData.releasePending(_aComponentName);
}

// -----------------------------------------------------------------------------
bool CacheLoadingAccess::findPendingChangedModules( Data::PendingModuleList & _rPendingList )
{
    osl::MutexGuard aGuard( this->m_aMutex ); // needed to protect the map access in the Data
    this->m_aData.findPendingModules(_rPendingList);
    return !_rPendingList.empty();
}

// -----------------------------------------------------------------------------
void CacheLoadingAccess::clearData(DisposeList& _rList) CFG_NOTHROW()
{
    osl::MutexGuard aGuard( this->m_aMutex );

    CFG_TRACE_INFO("Tree Info: Removing all module trees for cleanup" );

    typedef Data::ModuleList ModuleList;

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

    osl::MutexGuard aGuard( this->m_aMutex );

    CFG_TRACE_INFO("Tree Info: Collecting disposable module trees for cleanup" );

    Data::ModuleList& rActiveModules = this->m_aData.accessModuleList();

    DeadModuleList::iterator it = m_aDeadModules.begin();

    while (it != m_aDeadModules.end())
    {
        DeadModuleList::iterator current = it;
        // increment here, as we may later erase(current)
        ++it;

#if defined _DEBUG || defined _DBG_UTIL || defined CFG_TRACE_ENABLE
        OUString sCurrentName( current->first.toString() );
#endif
        TimeStamp aExpireTime = current->second + _aDelay;
        if (aExpireTime <= _aLimitTime)
        {
            Data::ModuleList::iterator itModule = rActiveModules.find( current->first );

            if (itModule != rActiveModules.end())
            {
                ModuleRef xModule = itModule->second;

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



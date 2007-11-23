/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confevents.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:01:21 $
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

#include <stdio.h>
#include <string.h>
#include "confevents.hxx"

#ifndef CONFIGMGR_API_EVENTHELPERS_HXX_
#include "confeventhelpers.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
namespace configmgr
{
    /////////////////////////////////////////////////////////////////////////
    using internal::ConfigChangesBroadcasterImpl;

    /////////////////////////////////////////////////////////////////////////
    class ConfigChangeBroadcastHelper // broadcasts changes for a given set of options
    {
        ConfigChangesBroadcasterImpl m_changes;
    public:
        ConfigChangeBroadcastHelper();
        ~ConfigChangeBroadcastHelper();

        void broadcast(TreeChangeList const& anUpdate, sal_Bool bError, IConfigBroadcaster* pSource);

    public:
        // IConfigBroadcaster implementation helper
        void addListener(AbsolutePath const& aName, INodeListenerRef const& );
        void removeListener(INodeListenerRef const&);

        void dispose(IConfigBroadcaster* pSource);
    };

    /////////////////////////////////////////////////////////////////////////
    ConfigChangeBroadcaster::ConfigChangeBroadcaster()
    {
    }

    ConfigChangeBroadcaster::~ConfigChangeBroadcaster()
    {
    }

    /////////////////////////////////////////////////////////////////////////
    ConfigChangeBroadcastHelper::ConfigChangeBroadcastHelper()
        : m_changes()
    {
    }

    ConfigChangeBroadcastHelper::~ConfigChangeBroadcastHelper()
    {
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcastHelper::dispose(IConfigBroadcaster* pSource)
    {
        m_changes.disposing(pSource);
    }

    /////////////////////////////////////////////////////////////////////////
    // IConfigBroadcaster implementation
    void ConfigChangeBroadcaster::addListener(AbsolutePath const& aName, RequestOptions const & _aOptions, INodeListenerRef const& pHandler)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(_aOptions,true))
        {
            pHelper->addListener(aName, pHandler);
        }
        else
            OSL_ASSERT(false);
    }

    void ConfigChangeBroadcaster::removeListener(RequestOptions const & _aOptions, INodeListenerRef const& pHandler)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(_aOptions,false))
        {
            pHelper->removeListener( pHandler);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcaster::fireChanges(TreeChangeList const& rList_, sal_Bool bError_)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(rList_.getOptions(),false))
        {
            pHelper->broadcast(rList_, bError_, this);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    ConfigChangeBroadcastHelper* ConfigChangeBroadcaster::newBroadcastHelper()
    {
        return new ConfigChangeBroadcastHelper();
    }
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcaster::disposeBroadcastHelper(ConfigChangeBroadcastHelper* pHelper)
    {
        if (pHelper)
        {
            pHelper->dispose(this);
            delete pHelper;
        }
    }
    /////////////////////////////////////////////////////////////////////////
    // IConfigBroadcaster implementation help
    void ConfigChangeBroadcastHelper::addListener(AbsolutePath const& aName, INodeListenerRef const& pHandler)
    {
        m_changes.add(aName, pHandler);
    }

    void ConfigChangeBroadcastHelper::removeListener(INodeListenerRef const& pHandler)
    {
        m_changes.remove(pHandler);
    }

/*  void ConfigChangeBroadcastHelper::removeNode(OUString const& aPath, bool bRemovedFromModel, IConfigBroadcaster* pSource)
    {
        m_changes.removed(aPath, bRemovedFromModel,pSource);
    }
*/
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcastHelper::broadcast(TreeChangeList const& anUpdate, sal_Bool bError, IConfigBroadcaster* pSource)
    {
        m_changes.dispatch(anUpdate, bError, pSource);
    }


} // namespace




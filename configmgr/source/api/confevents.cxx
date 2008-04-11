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
#include "confeventhelpers.hxx"
#include "treechangelist.hxx"
#include <osl/diagnose.h>
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




/*************************************************************************
 *
 *  $RCSfile: confevents.cxx,v $
 *
 *  $Revision: 1.7 $
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

        void broadcast(memory::Accessor const& _aChangedDataAccessor, TreeChangeList const& anUpdate, sal_Bool bError, IConfigBroadcaster* pSource);

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

/*  void ConfigChangeBroadcaster::removeNode(OUString const& aPath, const vos::ORef < OOptions >& _xOptions, bool bRemovedFromModel)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(_xOptions,false))
        {
            pHelper->removeNode(aPath, bRemovedFromModel, this);
        }
    }
*/
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcaster::fireChanges(memory::Accessor const& _aChangedDataAccessor, TreeChangeList const& rList_, sal_Bool bError_)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(rList_.getOptions(),false))
        {
            pHelper->broadcast(_aChangedDataAccessor, rList_, bError_, this);
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
    void ConfigChangeBroadcastHelper::broadcast(memory::Accessor const& _aChangedDataAccessor, TreeChangeList const& anUpdate, sal_Bool bError, IConfigBroadcaster* pSource)
    {
        m_changes.dispatch(_aChangedDataAccessor, anUpdate, bError, pSource);
    }


} // namespace




/*************************************************************************
 *
 *  $RCSfile: confevents.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:13:40 $
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
#include "confevents.hxx"

#ifndef CONFIGMGR_API_EVENTHELPERS_HXX_
#include "confeventhelpers.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
namespace configmgr
{
    // class is still abstract, ITreeListener methods are still unimplemented
    TreeListenerImpl::TreeListenerImpl(ITreeNotifier* aSource)
        : mySource(aSource)
    {
        if (mySource) mySource->setListener(this);
    }
    TreeListenerImpl::~TreeListenerImpl()
    {
        if (mySource) mySource->setListener(0);
    }

    void TreeListenerImpl::disposing(ITreeNotifier* pSource)
    {
        OSL_ASSERT(pSource && pSource == mySource);
        if (pSource == mySource)
            mySource = 0;
    }

    ITreeNotifier* TreeListenerImpl::rebind(ITreeNotifier* aSource)
    {
        ITreeNotifier* aOldSource = mySource;
        if (aSource != aOldSource)
        {
            if (mySource) mySource->setListener(0);
            mySource = aSource;
            if (mySource) mySource->setListener(this);
        }
        return aOldSource;
    }

    /////////////////////////////////////////////////////////////////////////
    using internal::ConfigChangesBroadcasterImpl;
    using internal::ConfigMessageBroadcasterImpl;

    /////////////////////////////////////////////////////////////////////////
    class ConfigChangeBroadcaster::Impl
    {
    public:
        Impl() : m_changes(), m_messages() {}

        ConfigChangesBroadcasterImpl m_changes;
        ConfigMessageBroadcasterImpl m_messages;
    };

    /////////////////////////////////////////////////////////////////////////
    ConfigChangeBroadcaster::ConfigChangeBroadcaster(ITreeNotifier* aSource)
        : TreeListenerImpl(aSource)
        , pImpl(new Impl())
    {
    }

    ConfigChangeBroadcaster::~ConfigChangeBroadcaster()
    {
        dispose();
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcaster::dispose()
    {
        unbind();

        if (!pImpl) return;
        pImpl->m_changes.disposing(this);
        pImpl->m_messages.disposing(this);
        delete pImpl, pImpl = 0;
    }

    /////////////////////////////////////////////////////////////////////////
    // IConfigBroadcaster implementation
    void ConfigChangeBroadcaster::addListener(OUString const& aName, INodeListener* pHandler)
    { pImpl->m_changes.add(aName, pHandler); }

    void ConfigChangeBroadcaster::removeListener(INodeListener* pHandler)
    { pImpl->m_changes.remove(pHandler); }

    void ConfigChangeBroadcaster::removeNode(OUString const& aPath, bool bRemovedFromModel)
    { pImpl->m_changes.removed(aPath,bRemovedFromModel,this); }

    void ConfigChangeBroadcaster::addHandler(IMessageHandler* pHandler)
    { pImpl->m_messages.add(pHandler); }

    void ConfigChangeBroadcaster::removeHandler(IMessageHandler* pHandler)
    { pImpl->m_messages.remove(pHandler); }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeBroadcaster::broadcast(TreeChangeList const& anUpdate, sal_Bool bError)
    {
        pImpl->m_changes.dispatch(anUpdate, bError, this);
    }

    /////////////////////////////////////////////////////////////////////////
    // ITreeListener implementation
    void ConfigChangeBroadcaster::changes(TreeChangeList const& rList_, sal_Bool bError_)
    {
        broadcast(rList_, bError_);;
    }

    void ConfigChangeBroadcaster::changes(sal_Int32 _nNotificationId,const OUString& _rNotifyReason)
    {
        pImpl->m_messages.dispatch(_rNotifyReason, _nNotificationId, this);
    }

    /////////////////////////////////////////////////////////////////////////
    class ConfigChangeMultiplexer::Impl
    {
    public:

        Impl(OUString const& sBasePath_) : sBasePath(sBasePath_), m_changes(), m_messages() {}

        OUString const sBasePath;
        ConfigChangesBroadcasterImpl m_changes;
        ConfigMessageBroadcasterImpl m_messages;
    };

    /////////////////////////////////////////////////////////////////////////
    ConfigChangeMultiplexer::ConfigChangeMultiplexer(IConfigBroadcaster* pSource)
        : m_pSource(pSource)
        , pImpl(0)
    {
        if (m_pSource) m_pSource->addHandler(this);
    }

    ConfigChangeMultiplexer::ConfigChangeMultiplexer(OUString const& aBasePath, IConfigBroadcaster* pSource)
        : m_pSource(pSource)
        , pImpl(0)
    {
        bind(aBasePath,pSource);
    }

    ConfigChangeMultiplexer::~ConfigChangeMultiplexer()
    {
        dispose();
    }
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::bind(OUString const& sNewPath, IConfigBroadcaster* pSource)
    {
        if (pImpl)
        {
            OSL_ASSERT(sNewPath == pImpl->sBasePath);
            rebind(pSource);
        }
        else
        {
            pImpl = new Impl(sNewPath);
            if (pSource)
                m_pSource = pSource;
            m_pSource->addListener(pImpl->sBasePath, this);
            m_pSource->addHandler(this);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::rebind(IConfigBroadcaster* pSource)
    {
        if (pSource != m_pSource)
        {
            unbind();
            m_pSource = pSource;
            if (m_pSource)
            {
                if (pImpl) m_pSource->addListener(pImpl->sBasePath, this);
                m_pSource->addHandler(this);
            }
        }
    }
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::unbind()
    {
        if (m_pSource)
        {
            if (pImpl) m_pSource->removeListener(this);
            m_pSource->removeHandler(this);
        }
        m_pSource = 0;
    }

    /////////////////////////////////////////////////////////////////////////
    bool ConfigChangeMultiplexer::normalizePath(OUString& aName)
    {
        if (!pImpl) return false;

        ConfigurationName aMyPath(pImpl->sBasePath);
        ConfigurationName aPath(aName);
        if (aPath.isRelative())
        {
            aName = aMyPath.composeWith(aPath).fullName();
            return true;
        }
        else
        {
            bool ok = (aMyPath == aPath) || aPath.isNestedIn(aMyPath);

            return ok;
        }
    }

    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::broadcast(Change const& anUpdate, OUString const& aRelativePath, sal_Bool bError)
    {
        OSL_ENSURE(pImpl!=0, "Trying to broadcast on disconnected Notification Multiplexer");

        OUString aContext( aRelativePath );
        if (!normalizePath(aContext))
            OSL_TRACE("Invalid path. Multiplexer may not walk subtrees correctly\n\r");

        if (pImpl)
        {
            pImpl->m_changes.dispatch(anUpdate, aContext, bError, this);
        }
    }

    void ConfigChangeMultiplexer::broadcast(Change const& anUpdate)
    {
        // do we need to append the local name ??
        if (pImpl)
            pImpl->m_changes.dispatch(anUpdate, pImpl->sBasePath, false, this);
    }
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::dispose()
    {
        unbind();

        if (!pImpl) return;
        pImpl->m_changes.disposing(this);
        pImpl->m_messages.disposing(this);
        delete pImpl, pImpl = 0;
    }

    // IConfigListener implementation
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::disposing(IConfigBroadcaster* pSource)
    {
        OSL_ASSERT(pSource == m_pSource);
        if (pSource == m_pSource)
            unbind();
    }

    // INodeListener implementation
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::nodeChanged(Change const& aChange, OUString const& aPath, IConfigBroadcaster* pSource)
    {
        OSL_ASSERT(pImpl != 0);
        OSL_VERIFY(pSource == m_pSource);
        broadcast(aChange, aPath);
    }
    void ConfigChangeMultiplexer::nodeDeleted(OUString const& aPath, IConfigBroadcaster* pSource)
    {
        OSL_ASSERT(pImpl != 0);

        if (pImpl)
        {
            OUString aContext( aPath );

            // must be this base path or a child of it
            OSL_ASSERT(!ConfigurationName(aPath).isRelative() || aPath.getLength() == 0);
            OSL_ASSERT(aPath.getLength() == 0 ||
                        pImpl->sBasePath == aPath ||
                        ConfigurationName(pImpl->sBasePath).isNestedIn(aPath));
            pImpl->m_changes.removed(aContext,true,this);
        }
    }

    // IMessageHandler implementation
    /////////////////////////////////////////////////////////////////////////
    void ConfigChangeMultiplexer::message(const OUString& rNotifyReason, sal_Int32 nNotificationId, IConfigBroadcaster* pSource)
    {
        OSL_VERIFY(pSource == m_pSource);
        if (pImpl)
            pImpl->m_messages.dispatch(rNotifyReason, nNotificationId, this);
    }

    /////////////////////////////////////////////////////////////////////////
    // IConfigBroadcaster implementation

    void ConfigChangeMultiplexer::addListener(OUString const& aName, INodeListener* pHandler)
    { pImpl->m_changes.add(aName, pHandler); }

    void ConfigChangeMultiplexer::removeListener(INodeListener* pHandler)
    { pImpl->m_changes.remove(pHandler); }

    void ConfigChangeMultiplexer::removeNode(OUString const& aPath, bool bRemovedFromModel)
    { pImpl->m_changes.removed(aPath,bRemovedFromModel,this); }

    void ConfigChangeMultiplexer::addHandler(IMessageHandler* pHandler)
    { pImpl->m_messages.add(pHandler); }

    void ConfigChangeMultiplexer::removeHandler(IMessageHandler* pHandler)
    { pImpl->m_messages.remove(pHandler); }

    /////////////////////////////////////////////////////////////////////////
    // ScreenedChangeMultiplexer
    ScreenedChangeMultiplexer:: ScreenedChangeMultiplexer(IConfigBroadcaster* aSource)
        : ConfigChangeMultiplexer()
        , m_pScreeningChanges(0)
    {
    }

    ScreenedChangeMultiplexer:: ScreenedChangeMultiplexer(TreeChangeList& aScreeningTree, IConfigBroadcaster* aSource)
        : ConfigChangeMultiplexer(aScreeningTree.pathToRoot, aSource)
        , m_pScreeningChanges(&aScreeningTree.root)
    {
    }

    ScreenedChangeMultiplexer:: ScreenedChangeMultiplexer(ScreeningChange* aScreening, OUString const& aBasePath, IConfigBroadcaster* aSource)
        : ConfigChangeMultiplexer(aBasePath, aSource)
        , m_pScreeningChanges(aScreening)
    {
    }

    void ScreenedChangeMultiplexer:: bind(TreeChangeList& aScreeningTree, IConfigBroadcaster* aSource)
    {
        bind(&aScreeningTree.root, aScreeningTree.pathToRoot, aSource);
    }

    void ScreenedChangeMultiplexer:: bind(ScreeningChange* aScreening, OUString const& aBasePath, IConfigBroadcaster* aSource)
    {
        unbind();
        m_pScreeningChanges = aScreening;
        ConfigChangeMultiplexer::bind(aBasePath, aSource);
    }

    void ScreenedChangeMultiplexer:: nodeChanged(Change const& aChange, OUString const& aPath, IConfigBroadcaster* pSource)
    {
        // To do: filter with local changes herehere
        if (&aChange != m_pScreeningChanges)
            ConfigChangeMultiplexer::nodeChanged(aChange,aPath,pSource);
    }

    void ScreenedChangeMultiplexer:: nodeDeleted(OUString const& aPath, IConfigBroadcaster* pSource)
    {
        // To do: filter here
        ConfigChangeMultiplexer::nodeDeleted(aPath,pSource);
    }

} // namespace




/*************************************************************************
 *
 *  $RCSfile: confevents.hxx,v $
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

#ifndef CONFIGMGR_API_EVENTS_HXX_
#define CONFIGMGR_API_EVENTS_HXX_

#include "cmtreemodel.hxx"
namespace rtl { class OUString; }

namespace configmgr
{
    using ::rtl::OUString;

    // class is still abstract, ITreeListener methods are still unimplemented
    class TreeListenerImpl : private ITreeListener
    {
        ITreeNotifier* mySource;
    public:
        explicit TreeListenerImpl(ITreeNotifier* aSource = 0);
        virtual ~TreeListenerImpl();

        ITreeNotifier* rebind(ITreeNotifier* aSource);
        ITreeNotifier* unbind() { return rebind(0); }
    protected:
        virtual void disposing(ITreeNotifier* pSource);
    private:
        TreeListenerImpl(TreeListenerImpl&);
        void operator=(TreeListenerImpl&);
    };

    struct IConfigBroadcaster;
    struct IConfigListener
    {
        virtual void disposing(IConfigBroadcaster* pSource) = 0;
    };
    struct INodeListener : IConfigListener
    {
        virtual void nodeChanged(Change const& aChange, OUString const& aPath, IConfigBroadcaster* pSource) = 0;
        virtual void nodeDeleted(OUString const& aPath, IConfigBroadcaster* pSource) = 0;
    };
    struct IMessageHandler: IConfigListener
    {
        virtual void message(const OUString& rNotifyReason, sal_Int32 nNotificationId, IConfigBroadcaster* pSource) = 0;
    };

    struct IConfigBroadcaster
    {
    protected:
        IConfigBroadcaster() {}
        ~IConfigBroadcaster() {}
    public:
        virtual void addListener(OUString const& aName, INodeListener* pListener) = 0;
        virtual void removeListener(INodeListener* pListener) = 0;

        virtual void removeNode(OUString const& aPath, bool bRemovedFromModel = false) = 0;

        virtual void addHandler(IMessageHandler* pHandler) = 0;
        virtual void removeHandler(IMessageHandler* pHandler) = 0;
    };

    class ConfigChangeBroadcaster : public TreeListenerImpl, public IConfigBroadcaster
    {
        class Impl;
        Impl* pImpl;
    public:
        ConfigChangeBroadcaster(ITreeNotifier* aSource = 0);
        virtual ~ConfigChangeBroadcaster();

        void rebind(ITreeNotifier* aSource);

        void broadcast(TreeChangeList const& anUpdate, sal_Bool bError = false);

    public:
        // IConfigBroadcaster implementation
        void addListener(OUString const& aName, INodeListener* );
        void removeListener(INodeListener*);

        void removeNode(OUString const& aPath, bool bRemovedFromModel = false);

        void addHandler(IMessageHandler* );
        void removeHandler(IMessageHandler* );

        void dispose();
    protected:
        // ITreeListener implementation
        virtual void changes(TreeChangeList const& , sal_Bool _bError);
        virtual void changes(sal_Int32 _nNotificationId,const ::rtl::OUString& _rNotifyReason);
    };

    class ConfigChangeMultiplexer : public IConfigBroadcaster, private INodeListener, private IMessageHandler
    {
        class Impl;
        Impl* pImpl;
        IConfigBroadcaster* m_pSource;
    public:
        ConfigChangeMultiplexer(IConfigBroadcaster* aSource = 0);
        ConfigChangeMultiplexer(OUString const& aBasePath, IConfigBroadcaster* aSource);
        virtual ~ConfigChangeMultiplexer();

        void bind(OUString const& aBasePath, IConfigBroadcaster* aSource = 0);
        void rebind(IConfigBroadcaster* aSource);
        void unbind();

        void broadcast(Change const& anUpdate, OUString const& aRelativePath, sal_Bool bError = false);
        void broadcast(Change const& anUpdate);

    public:
        // IConfigBroadcaster implementation
        void addListener(OUString const& aName, INodeListener* );
        void removeListener(INodeListener*);

        void removeNode(OUString const& aPath, bool bRemovedFromModel = false);

        void addHandler(IMessageHandler* );
        void removeHandler(IMessageHandler* );

        void dispose();
    protected:
        bool normalizePath(OUString& aPath);

        // IConfigListener implementation
        virtual void disposing(IConfigBroadcaster* pSource);

        // INodeListener implementation
        virtual void nodeChanged(Change const& aChange, OUString const& aPath, IConfigBroadcaster* pSource);
        virtual void nodeDeleted(OUString const& aPath, IConfigBroadcaster* pSource);

        // IMessageHandler implementation
        virtual void message(const OUString& rNotifyReason, sal_Int32 nNotificationId, IConfigBroadcaster* pSource);
    };

    class ScreenedChangeMultiplexer : public ConfigChangeMultiplexer
    {
        typedef SubtreeChange ScreeningChange;
        ScreeningChange* m_pScreeningChanges;
    public:
        ScreenedChangeMultiplexer(IConfigBroadcaster* aSource = 0);
        ScreenedChangeMultiplexer(TreeChangeList& aScreeningTree, IConfigBroadcaster* aSource);
        ScreenedChangeMultiplexer(ScreeningChange* aScreening, OUString const& aBasePath, IConfigBroadcaster* aSource);

        void bind(TreeChangeList& aScreeningTree, IConfigBroadcaster* aSource = 0);
        void bind(ScreeningChange* aScreening, OUString const& aBasePath, IConfigBroadcaster* aSource = 0);

    protected:
        virtual void nodeChanged(Change const& aChange, OUString const& aPath, IConfigBroadcaster* pSource);
        virtual void nodeDeleted(OUString const& aPath, IConfigBroadcaster* pSource);
    };
} // namespace

#endif // CONFIGMGR_API_EVENTS_HXX_




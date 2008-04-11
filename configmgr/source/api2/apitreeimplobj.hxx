/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apitreeimplobj.hxx,v $
 * $Revision: 1.27 $
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

#ifndef CONFIGMGR_API_TREEIMPLOBJECTS_HXX_
#define CONFIGMGR_API_TREEIMPLOBJECTS_HXX_

#include "noderef.hxx"
#include "configset.hxx"
#include "configdefaultprovider.hxx"
#include "confevents.hxx"
#include "options.hxx"
#include "utility.hxx"
#include <vos/ref.hxx>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
//-----------------------------------------------------------------------------
    class OProviderImpl;
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        class Factory;
        class Notifier;
        class NotifierImpl;
        typedef vos::ORef<NotifierImpl> NotifierImplHolder;
//-----------------------------------------------------------------------------
        class ObjectRegistry;
        typedef rtl::Reference<ObjectRegistry> ObjectRegistryHolder;

        typedef uno::XInterface UnoInterface;
        typedef uno::Reference<UnoInterface> UnoInterfaceRef;
        typedef uno::Reference<com::sun::star::script::XTypeConverter>  UnoTypeConverter;

        typedef vos::ORef< OOptions > TreeOptions;
//-----------------------------------------------------------------------------
// API object implementation wrappers
//-------------------------------------------------------------------------
        class ApiProvider : Noncopyable
        {
            Factory&                    m_rFactory;
            OProviderImpl&              m_rProviderImpl;
        public:
            ApiProvider(Factory& rFactory, OProviderImpl& rProviderImpl );

            ~ApiProvider()
            {}

            UnoTypeConverter            getTypeConverter() const;
            Factory&                    getFactory()        { return m_rFactory; }
            OProviderImpl&              getProviderImpl()   { return m_rProviderImpl; }
        };

    //-----------------------------------------------------------------------------

    //-------------------------------------------------------------------------
        class ApiTreeImpl : Noncopyable
        {
            class ComponentAdapter;
            typedef uno::Reference<ComponentAdapter> ComponentRef;
            typedef uno::Reference<com::sun::star::lang::XComponent> UnoComponent;

            typedef configuration::TreeRef TreeRef;
            typedef configuration::DefaultProvider DefaultProvider;

            TreeRef             m_aTree;
            NotifierImplHolder  m_aNotifier;
            DefaultProvider     m_aDefaultProvider;
            ComponentRef        m_xProvider;
            ApiProvider&        m_rProvider;
            ApiTreeImpl*        m_pParentTree;
            UnoInterface*       m_pInstance;

        public:
            explicit ApiTreeImpl(UnoInterface* pInstance, configuration::TreeRef const& aTree, ApiTreeImpl& rParentTree);
            explicit ApiTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, configuration::TreeRef const& aTree, ApiTreeImpl* pParentTree);
            explicit ApiTreeImpl(UnoInterface* _pInstance, ApiProvider& _rProvider, configuration::TreeRef const& _aTree, DefaultProvider const & _aDefaultProvider);
            ~ApiTreeImpl();

        // initialization
            void setNodeInstance(configuration::NodeRef const& aNode, UnoInterface* pInstance);

        // model access
            TreeRef getTree() const { return m_aTree; }

        // self-locked methods for dispose handling
            bool isAlive()  const;
            void checkAlive()   const;
            bool disposeTree(bool bForce);
            bool disposeTreeNow();
            void disposeNode(configuration::NodeRef const& aNode, UnoInterface* pInstance);

        // api object handling
            Factory&                    getFactory()    const   { return m_rProvider.getFactory(); }
            Notifier                    getNotifier()   const;
            DefaultProvider             getDefaultProvider()    const { return m_aDefaultProvider; }

        // needs external locking
            ApiTreeImpl const*          getRootTreeImpl() const;

            uno::XInterface*            getUnoInstance() const  { return m_pInstance; }
            ApiProvider&                getProvider()           { return m_rProvider; }
            UnoInterfaceRef             getUnoProviderInstance() const; //  { return m_xProvider; }

            /// wire this to a new parent tree
            void                        haveNewParent(ApiTreeImpl* pNewParent);
        private:
            void init(ApiTreeImpl*  pParentTree);
            void setParentTree(ApiTreeImpl* pNewParentTree);
            void deinit();

            bool implDisposeTree();
            void implDisposeNode(configuration::NodeRef const& aNode, UnoInterface* pInstance);

            friend class ComponentAdapter;
            void disposing(com::sun::star::lang::EventObject const& rEvt) throw();
            UnoComponent getProviderComponent();
            UnoComponent getParentComponent();

        };

    //-----------------------------------------------------------------------------
        class ApiRootTreeImpl
        {
            typedef configuration::AbsolutePath AbsolutePath;
            typedef configuration::DefaultProvider DefaultProvider;

        public:
            explicit ApiRootTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, configuration::Tree const& aTree, TreeOptions const& _xOptions);
            ~ApiRootTreeImpl();

            ApiTreeImpl& getApiTree() { return m_aTreeImpl; }
            ApiTreeImpl const& getApiTree() const { return m_aTreeImpl; }

            AbsolutePath const & getLocation() const { return m_aLocationPath; }
            TreeOptions getOptions() const { return m_xOptions; }

        // self-locked methods for dispose handling
            bool disposeTree();

            /// toggle whether this object relays notifications from the base provider
            bool enableNotification(bool bEnable);
        private:
            IConfigBroadcaster* implSetNotificationSource(IConfigBroadcaster* pNew);
            void implSetLocation(configuration::Tree const& _aTree);
            void releaseData();

        private:
            class NodeListener;
            friend class NodeListener;
            //IConfigBroadcaster*   m_pNotificationSource;

        // IConfigListener
            void disposing(IConfigBroadcaster* pSource) ;
        //INodeListener : IConfigListener
            void nodeChanged(Change const& aChange, AbsolutePath const& aPath, IConfigBroadcaster* pSource);
            void nodeDeleted(AbsolutePath const& aPath, IConfigBroadcaster* pSource);

        private:
            ApiTreeImpl                 m_aTreeImpl;
            AbsolutePath                m_aLocationPath;
            rtl::Reference<NodeListener> m_pNotificationListener;
            TreeOptions m_xOptions;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_TREEIMPLOBJECTS_HXX_

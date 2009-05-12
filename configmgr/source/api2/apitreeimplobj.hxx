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
#include <boost/utility.hpp>
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
//-----------------------------------------------------------------------------
        class ObjectRegistry;
//-----------------------------------------------------------------------------
// API object implementation wrappers
//-------------------------------------------------------------------------
        class ApiProvider: private boost::noncopyable
        {
            Factory&                    m_rFactory;
            OProviderImpl&              m_rProviderImpl;
        public:
            ApiProvider(Factory& rFactory, OProviderImpl& rProviderImpl );

            ~ApiProvider()
            {}

            uno::Reference<com::sun::star::script::XTypeConverter>          getTypeConverter() const;
            Factory&                    getFactory()        { return m_rFactory; }
            OProviderImpl&              getProviderImpl()   { return m_rProviderImpl; }
        };

    //-----------------------------------------------------------------------------

    //-------------------------------------------------------------------------
        class ApiTreeImpl: private boost::noncopyable
        {
            class ComponentAdapter;

            rtl::Reference< configuration::Tree > m_aTree;
            vos::ORef<NotifierImpl> m_aNotifier;
            configuration::DefaultProvider     m_aDefaultProvider;
            uno::Reference<ComponentAdapter>        m_xProvider;
            ApiProvider&        m_rProvider;
            ApiTreeImpl*        m_pParentTree;
            uno::XInterface*        m_pInstance;

        public:
            explicit ApiTreeImpl(uno::XInterface* pInstance, ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, ApiTreeImpl* pParentTree);
            explicit ApiTreeImpl(uno::XInterface* _pInstance, ApiProvider& _rProvider, rtl::Reference< configuration::Tree > const& _aTree, configuration::DefaultProvider const & _aDefaultProvider);
            ~ApiTreeImpl();

        // initialization
            void setNodeInstance(configuration::NodeRef const& aNode, uno::XInterface* pInstance);

        // model access
            rtl::Reference< configuration::Tree > getTree() const { return m_aTree; }

        // self-locked methods for dispose handling
            bool isAlive()  const;
            void checkAlive()   const;
            bool disposeTree(bool bForce);
            bool disposeTreeNow();
            void disposeNode(configuration::NodeRef const& aNode, uno::XInterface* pInstance);

        // api object handling
            Factory&                    getFactory()    const   { return m_rProvider.getFactory(); }
            Notifier                    getNotifier()   const;
            configuration::DefaultProvider              getDefaultProvider()    const { return m_aDefaultProvider; }

        // needs external locking
            ApiTreeImpl const*          getRootTreeImpl() const;

            uno::XInterface*            getUnoInstance() const  { return m_pInstance; }
            ApiProvider&                getProvider()           { return m_rProvider; }
            uno::Reference<uno::XInterface>             getUnoProviderInstance() const; //  { return m_xProvider; }

            /// wire this to a new parent tree
            void                        haveNewParent(ApiTreeImpl* pNewParent);
        private:
            void init(ApiTreeImpl*  pParentTree);
            void setParentTree(ApiTreeImpl* pNewParentTree);
            void deinit();

            bool implDisposeTree();
            void implDisposeNode(configuration::NodeRef const& aNode, uno::XInterface* pInstance);

            friend class ComponentAdapter;
            void disposing(com::sun::star::lang::EventObject const& rEvt) throw();
            uno::Reference<com::sun::star::lang::XComponent> getProviderComponent();
            uno::Reference<com::sun::star::lang::XComponent> getParentComponent();

        };

    //-----------------------------------------------------------------------------
        class ApiRootTreeImpl
        {
        public:
            explicit ApiRootTreeImpl(uno::XInterface* pInstance, ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions > const& _xOptions);
            ~ApiRootTreeImpl();

            ApiTreeImpl& getApiTree() { return m_aTreeImpl; }
            ApiTreeImpl const& getApiTree() const { return m_aTreeImpl; }

            configuration::AbsolutePath const & getLocation() const { return m_aLocationPath; }
            vos::ORef< OOptions > getOptions() const { return m_xOptions; }

        // self-locked methods for dispose handling
            bool disposeTree();

            /// toggle whether this object relays notifications from the base provider
            bool enableNotification(bool bEnable);
        private:
            TreeManager * implSetNotificationSource(TreeManager * pNew);
            void implSetLocation(rtl::Reference< configuration::Tree > const& _aTree);
            void releaseData();

        private:
            class NodeListener;
            friend class NodeListener;

        // IConfigListener
            void disposing(TreeManager * pSource) ;
        //INodeListener : IConfigListener
            void nodeChanged(Change const& aChange, configuration::AbsolutePath const& aPath, TreeManager * pSource);
            void nodeDeleted(configuration::AbsolutePath const& aPath, TreeManager * pSource);

        private:
            ApiTreeImpl                 m_aTreeImpl;
            configuration::AbsolutePath             m_aLocationPath;
            rtl::Reference<NodeListener> m_pNotificationListener;
            vos::ORef< OOptions > m_xOptions;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_TREEIMPLOBJECTS_HXX_

/*************************************************************************
 *
 *  $RCSfile: apitreeimplobj.hxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:30 $
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

#ifndef CONFIGMGR_API_TREEIMPLOBJECTS_HXX_
#define CONFIGMGR_API_TREEIMPLOBJECTS_HXX_

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGSET_HXX_
#include "configset.hxx"
#endif
#ifndef CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_
#include "configdefaultprovider.hxx"
#endif
#ifndef CONFIGMGR_API_EVENTS_HXX_
#include "confevents.hxx"
#endif
#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include "options.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

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

        // locking support
            memory::Segment const *     getSourceData() const   { return configuration::getRootSegment(m_aTree); }
            osl::Mutex&                 getDataLock() const     { return configuration::getRootLock(m_aTree); }
            osl::Mutex&                 getApiLock() const;

            /// wire this to a new parent tree
            void                        haveNewParent(ApiTreeImpl* pNewParent);
        private:
            void init(ApiTreeImpl*  pParentTree);
            void setParentTree(ApiTreeImpl* pNewParentTree);
            void deinit();

            bool implDisposeTree(data::Accessor const& _aAccessor);
            void implDisposeNode(data::Accessor const & _anAccessor, configuration::NodeRef const& aNode, UnoInterface* pInstance);

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
            TreeOptions m_xOptions;

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
            void nodeChanged(data::Accessor const& _aChangedDataAccessor, Change const& aChange, AbsolutePath const& aPath, IConfigBroadcaster* pSource);
            void nodeDeleted(data::Accessor const& _aChangedDataAccessor, AbsolutePath const& aPath, IConfigBroadcaster* pSource);

        private:
            ApiTreeImpl                 m_aTreeImpl;
            AbsolutePath                m_aLocationPath;
            rtl::Reference<NodeListener> m_pNotificationListener;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_TREEIMPLOBJECTS_HXX_

/*************************************************************************
 *
 *  $RCSfile: apitreeimplobj.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-10 22:43:58 $
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

#include "apitypes.hxx"

#include "noderef.hxx"
#include "configset.hxx"

#include "confevents.hxx"

#include <osl/mutex.hxx>
#include <vos/ref.hxx>
#include <stl/memory>

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
//-----------------------------------------------------------------------------
    struct ServiceInfo;

    class OProviderImpl;
//-----------------------------------------------------------------------------
    namespace configuration
    {
        class Name;
        class Tree;
    }
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
        typedef vos::ORef<ObjectRegistry> ObjectRegistryHolder;

        typedef uno::XInterface UnoInterface;
        typedef uno::Reference<UnoInterface> UnoInterfaceRef;
        typedef uno::Reference<com::sun::star::lang::XComponent> ComponentRef;
        typedef uno::Reference<com::sun::star::script::XTypeConverter>  UnoTypeConverter;

//-----------------------------------------------------------------------------
// API object implementation wrappers
//-------------------------------------------------------------------------
        class ApiProvider : NotCopyable
        {
            typedef configuration::TemplateProvider TemplateProvider;

            Factory&                    m_rFactory;
            OProviderImpl&              m_rProviderImpl;
        public:
            ApiProvider(Factory& rFactory, OProviderImpl& rProviderImpl )
                : m_rFactory(rFactory)
                , m_rProviderImpl(rProviderImpl)
            {}
            ~ApiProvider()
            {}

            UnoTypeConverter            getTypeConverter() const;
            Factory&                    getFactory()        { return m_rFactory; }
            OProviderImpl&              getProviderImpl()   { return m_rProviderImpl; }
            ISynchronizedData*          getSourceLock() const;
            TemplateProvider            getTemplateProvider() const;
        };

    //-------------------------------------------------------------------------
        class ApiTreeImpl : private com::sun::star::lang::XEventListener, NotCopyable
        {
            typedef configuration::Tree Tree;
            Tree                m_aTree;
            NotifierImplHolder  m_aNotifier;
            ComponentRef        m_xProvider;
            ApiProvider&        m_rProvider;
            ApiTreeImpl*        m_pParentTree;
            UnoInterface*       m_pInstance;
        public:
            explicit ApiTreeImpl(UnoInterface* pInstance, Tree const& aTree, ApiTreeImpl& rParentTree);
            explicit ApiTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, Tree const& aTree, ApiTreeImpl* pParentTree = 0);
            ~ApiTreeImpl();

        // model access
            Tree                        getTree() const { return m_aTree; }

        // self-locked methods for dispose handling
            void checkAlive()   const;
            bool disposeTree(bool bForce);
            void disposeNode(configuration::NodeRef const& aNode, UnoInterface* pInstance);

        // api object handling
            Factory&                    getFactory()    const   { return m_rProvider.getFactory(); }
            Notifier                    getNotifier()   const;

            uno::XInterface*            getUnoInstance() const  { return m_pInstance; }
            ApiProvider&                getProvider()           { return m_rProvider; }
        // locking support
            ISynchronizedData*          getProviderLock() const { return m_rProvider.getSourceLock(); }
            ISynchronizedData*          getDataLock() const     { return configuration::getRootLock(m_aTree); }
            osl::Mutex&                 getApiLock() const;

            /// wire this to a new parent tree
            void                        haveNewParent(ApiTreeImpl* pNewParent);
        private:
            void init(ApiTreeImpl*  pParentTree);
            void setParentTree(ApiTreeImpl* pNewParentTree);
            void deinit();

            void implDisposeTree();
            void implDisposeNode(configuration::NodeRef const& aNode, UnoInterface* pInstance);

            ComponentRef getParentComponent();
            ComponentRef getProviderComponent();

        // XEventListener
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            virtual uno::Any SAL_CALL queryInterface(uno::Type const& rType) throw();
            virtual void SAL_CALL disposing(com::sun::star::lang::EventObject const& rEvt) throw();
            // ---------------------------------------------------------------------------------------------------
        };

    //-----------------------------------------------------------------------------
        class ApiRootTreeImpl : private INodeListener
        {
            typedef configuration::Tree Tree;
        public:
            explicit ApiRootTreeImpl(UnoInterface* pInstance, ApiProvider& rProvider, Tree const& aTree);
            ~ApiRootTreeImpl();

            ApiTreeImpl& getApiTree() { return m_aTreeImpl; }
            ApiTreeImpl const& getApiTree() const { return m_aTreeImpl; }

        // self-locked methods for dispose handling
            bool disposeTree();

            /// toggle whether this object relays notifications from the base provider
            bool                        enableNotification(bool bEnable);
        private:
            IConfigBroadcaster* implSetNotificationSource(IConfigBroadcaster* pNew);
            void implSetLocation();

        private:
        // IConfigListener
            virtual void disposing(IConfigBroadcaster* pSource) ;
        //INodeListener : IConfigListener
            virtual void nodeChanged(Change const& aChange, OUString const& aPath, IConfigBroadcaster* pSource);
            virtual void nodeDeleted(OUString const& aPath, IConfigBroadcaster* pSource);

        private:
            ApiTreeImpl         m_aTreeImpl;
            OUString            m_aLocationPath;
            IConfigBroadcaster* m_pNotificationSource;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_TREEIMPLOBJECTS_HXX_

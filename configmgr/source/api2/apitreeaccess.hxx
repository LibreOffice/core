/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apitreeaccess.hxx,v $
 * $Revision: 1.13 $
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

#ifndef CONFIGMGR_API_TREEACCESS_HXX_
#define CONFIGMGR_API_TREEACCESS_HXX_

#include "sal/config.h"

#include "boost/utility.hpp"
#include "rtl/ref.hxx"

#include "datalock.hxx"
#include "options.hxx"
#include "utility.hxx"

namespace osl { class Mutex; }

namespace configmgr
{
//-----------------------------------------------------------------------------
    struct ServiceImplementationInfo;
//-----------------------------------------------------------------------------
    namespace configuration
    {
        class ElementTree;
        class Template;
        class Tree;
    }
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        namespace uno = com::sun::star::uno;
//-----------------------------------------------------------------------------
        class Factory;
        class Notifier;
        class Committer;
        class NodeSetInfoAccess;
//-----------------------------------------------------------------------------
    // API object implementation wrappers
    //-------------------------------------------------------------------------
        class ApiTreeImpl;
        class ApiRootTreeImpl;

        // these objects just provide the pieces needed to navigate and manipulate trees and nodes

        // A common base class for 'element' classes
        class NodeElement: private boost::noncopyable
        {
        public:
            virtual ~NodeElement() {}

        // self-locked methods for dispose handling
            void checkAlive() const;

        // api object handling
            uno::XInterface* getUnoInstance() const
            { return doGetUnoInstance(); }

            ServiceImplementationInfo const * getServiceInfo() const
            { return doGetServiceInfo(); }
        private:
            virtual uno::XInterface* doGetUnoInstance() const = 0;
            virtual ServiceImplementationInfo const* doGetServiceInfo() const = 0;
            virtual ApiTreeImpl& getApiTree() const = 0;

            friend class Factory;
        };
//-----------------------------------------------------------------------------

        // A class for tagging inner nodes
        class InnerElement : public NodeElement
        {
        public:
            // Only used as a tag
        };
//-----------------------------------------------------------------------------

        // A common base class for tree-owning elemnt classes
        class TreeElement : public NodeElement
        {
        public:
        // model access
            rtl::Reference< configuration::Tree > getTreeRef() const;
            rtl::Reference< configuration::Tree > getTree() const;

        // api object handling
            Factory&                    getFactory();

        protected:
            virtual ApiTreeImpl& getApiTree() const = 0;
        };
//-----------------------------------------------------------------------------

        // Info interfaces for Set Elements
        class SetElement : public TreeElement
        {
        public:
        // self-locked methods for dispose handling
            bool disposeTree(bool bForceDispose);

            void haveNewParent(NodeSetInfoAccess* pNewParent);

            rtl::Reference< configuration::ElementTree > getElementRef() const;
            rtl::Reference< configuration::Template > getTemplateInfo() const;
        };
//-----------------------------------------------------------------------------

        // Info interfaces for Set Elements
        class RootElement : public TreeElement
        {
        public:
            bool disposeTree();
        protected:
            virtual ApiRootTreeImpl& getRootTree() = 0;
        };
//-----------------------------------------------------------------------------

        // Info interfaces for Set Elements
        class UpdateRootElement : public RootElement
        {
        public:
            Committer getCommitter();
        };

    // Thin Wrappers around TreeElements: Provide guarding and convenient access
        /// wraps a TreeElement; provides an object (read) lock, ensures object was not disposed
        class GuardedTreeElement {
            UnoApiLock m_aLock;
            TreeElement & m_rTree;

        public:
            explicit GuardedTreeElement(TreeElement & rTree): m_rTree(rTree)
            { rTree.checkAlive(); }

            TreeElement & get() const { return m_rTree; }
        };

        class GuardedRootElement {
            UnoApiLock m_aLock;
            RootElement & m_rTree;

        public:
            explicit GuardedRootElement(RootElement & rTree): m_rTree(rTree)
            { rTree.checkAlive(); }

            RootElement & get() const { return m_rTree; }
        };
    }
}

#endif // CONFIGMGR_API_TREEACCESS_HXX_

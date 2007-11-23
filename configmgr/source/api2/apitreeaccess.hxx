/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apitreeaccess.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:04:08 $
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

#ifndef CONFIGMGR_API_TREEACCESS_HXX_
#define CONFIGMGR_API_TREEACCESS_HXX_

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include "options.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

namespace osl { class Mutex; }

namespace configmgr
{
//-----------------------------------------------------------------------------
    struct ServiceImplementationInfo;
//-----------------------------------------------------------------------------
    namespace configuration
    {
        class Name;
        class Tree;
        class TreeRef;
        class ElementRef;

        class TemplateInfo;
    }
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        namespace uno = com::sun::star::uno;
        typedef uno::XInterface UnoInterface;
        typedef uno::Any UnoAny;
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
        class NodeElement : Noncopyable
        {
        public:
            typedef ServiceImplementationInfo ServiceInfo;
        public:
            virtual ~NodeElement() {}

        // self-locked methods for dispose handling
            void checkAlive() const;

        // api object handling
            UnoInterface* getUnoInstance() const
            { return doGetUnoInstance(); }

            ServiceInfo const * getServiceInfo() const
            { return doGetServiceInfo(); }
        private:
            virtual UnoInterface* doGetUnoInstance() const = 0;
            virtual ServiceInfo const* doGetServiceInfo() const = 0;
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
            configuration::TreeRef      getTreeRef() const;
            configuration::Tree         getTree() const;

        // api object handling
            Factory&                    getFactory();
            Notifier                    getNotifier();

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

            configuration::ElementRef       getElementRef() const;
            configuration::TemplateInfo     getTemplateInfo() const;
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
//-----------------------------------------------------------------------------
        /// guards a TreeElement; provides an object (read) lock, ensures object was not disposed
        // FIXME: bin this ...
        class TreeReadGuardImpl : Noncopyable
        {
            TreeElement&        m_rTree;
        public:
            TreeReadGuardImpl(TreeElement& rTree);
            ~TreeReadGuardImpl() throw ();
        public:
            TreeElement&    get() const { return m_rTree; }
        };

    // Thin Wrappers around TreeElements: Provide guarding and convenient access
        /// wraps a TreeElement; provides an object (read) lock, ensures object was not disposed
        class GuardedTreeElement
        {
            UnoApiLock          m_aLock;
            TreeReadGuardImpl   m_aImpl;
        public:
            GuardedTreeElement(TreeElement& rTree) : m_aImpl(rTree) {}
        public:
            TreeElement&    get() const { return m_aImpl.get(); }
        };

        class GuardedRootElement
        {
            UnoApiLock          m_aLock;
            TreeReadGuardImpl   m_aImpl;
        public:
            GuardedRootElement(RootElement& rTree);
        public:
            RootElement& get() const { return static_cast<RootElement&>(m_aImpl.get()); }

            configuration::Tree getTree() const;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_API_TREEACCESS_HXX_

/*************************************************************************
 *
 *  $RCSfile: setnodeimpl.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:41:52 $
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

#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#define CONFIGMGR_SETNODEBEHAVIOR_HXX_

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGTEMPLATE_HXX_
#include "template.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef INCUDED_MAP
#include <map>
#define INCUDED_MAP
#endif
#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    class SubtreeChange;
    class AddNode;
    class RemoveNode;
//-----------------------------------------------------------------------------

    namespace configuration
    {
//-----------------------------------------------------------------------------
        class SetElementChangeImpl;

//-----------------------------------------------------------------------------
        typedef rtl::Reference<ElementTreeImpl>     ElementTreeHolder; // also in configset.hxx
        typedef std::vector< ElementTreeHolder >    ElementList; // also in treeimpl.hxx
//-----------------------------------------------------------------------------

        struct SetEntry
        {
            SetEntry(data::Accessor const& _aAccessor,ElementTreeImpl* _pTree);

            bool isValid()  const { return m_pTree != 0; }

            data::Accessor const& accessor() const { return m_aAccessor; }
            ElementTreeImpl* tree() const { return m_pTree; };

            view::ViewTreeAccess    getTreeView() const;
        private:
            data::Accessor   m_aAccessor;
            ElementTreeImpl* m_pTree;
        };
    //-------------------------------------------------------------------------

        struct SetNodeVisitor
        {
            enum Result { DONE, CONTINUE };
            virtual Result visit(SetEntry const& anEntry) = 0;
        };

    //-----------------------------------------------------------------------------

    // basic implementations of set node contents
    //-----------------------------------------------------------------------------

        struct ElementTreeData
        {
            typedef rtl::Reference<ElementTreeImpl> ElementTreeHolder;

            // construction
            ElementTreeData() : tree(), inDefault(false) {}

            ElementTreeData(ElementTreeHolder const& _tree, bool _bDefault)
             : tree(_tree), inDefault(_bDefault) {}

            // ORef compatibility
            sal_Bool isValid() const { return this->tree.is(); }
            ElementTreeImpl* get() const { return this->tree.get(); }
            ElementTreeHolder const& operator->() const { return this->tree; }
            ElementTreeImpl& operator*() const { return *get(); }

            // data
            ElementTreeHolder   tree;
            bool                inDefault;
        };
    //-----------------------------------------------------------------------------

        class ElementSet
        {
        public:
            typedef ElementTreeData Element;
            typedef std::map<Name, Element> Data;

        // the following must be implemented by derived classes
            bool isEmpty() const { return m_aData.empty(); }

            bool hasElement(Name const& aName) const;
            Element* getElement(Name const& aName);
            Element const* getElement(Name const& aName) const;
            Element findElement(Name const& aName);

            void insertElement(Name const& aName, Element const& aNewEntry);
            Element replaceElement(Name const& aName, Element const& aNewEntry);
            Element removeElement(Name const& aName);

            void clearElements() {  m_aData.clear(); }

            void swap(ElementSet& aOther) { m_aData.swap(aOther.m_aData); }

        // STL style iteration
            class ConstIterator
            {
                typedef Data::const_iterator It;
            public:
                ConstIterator(It const& it) : m_base(it) {}

                Element const& operator* () const { return  m_base->second; }
                Element const* operator->() const { return &m_base->second; }

                ConstIterator& operator++()     { ++m_base; return *this; }
                ConstIterator  operator++(int)  { return ConstIterator(m_base++); }

                ConstIterator& operator--()     { --m_base; return *this; }
                ConstIterator  operator--(int)  { return ConstIterator(m_base--); }

                friend bool operator ==(ConstIterator const& lhs, ConstIterator const& rhs)
                { return lhs.m_base == rhs.m_base; }
                friend bool operator !=(ConstIterator const& lhs, ConstIterator const& rhs)
                { return lhs.m_base != rhs.m_base; }
            private:
                It m_base;
            };
            ConstIterator begin() const { return ConstIterator(m_aData.begin()); }
            ConstIterator end()   const { return ConstIterator(m_aData.end()); }

            class Iterator
            {
                typedef Data::iterator It;
            public:
                Iterator(It const& it) : m_base(it) {}

                Element& operator* () const { return  m_base->second; }
                Element* operator->() const { return &m_base->second; }

                Iterator& operator++()      { ++m_base; return *this; }
                Iterator  operator++(int)   { return Iterator(m_base++); }

                Iterator& operator--()      { --m_base; return *this; }
                Iterator  operator--(int)   { return Iterator(m_base--); }

                friend bool operator ==(Iterator const& lhs, Iterator const& rhs)
                { return lhs.m_base == rhs.m_base; }
                friend bool operator !=(Iterator const& lhs, Iterator const& rhs)
                { return lhs.m_base != rhs.m_base; }

                operator ConstIterator() const { return ConstIterator(m_base); }
            private:
                It m_base;
            };
            Iterator begin()    { return Iterator(m_aData.begin()); }
            Iterator end()      { return Iterator(m_aData.end()); }

            typedef Data::const_iterator PairIterator;
            PairIterator beginNative()  const { return m_aData.begin(); }
            PairIterator endNative()    const { return m_aData.end(); }
        private:
            Data m_aData;
        };
    //-------------------------------------------------------------------------

    // Basic implementation of a set node
    //-------------------------------------------------------------------------

        class SetNodeImpl : public NodeImpl
        {
            friend class view::ViewStrategy;
            ElementSet          m_aDataSet;
            TemplateHolder      m_aTemplate;
            TemplateProvider    m_aTemplateProvider;
            TreeImpl*           m_pParentTree;
            NodeOffset          m_nContextPos;

            typedef NodeOffset InitHelper;
            InitHelper      m_aInit;

        public:
            typedef ElementSet::Element Element;

            SetNodeImpl(data::SetNodeAddress const& _aNodeRef, Template* pTemplate);

            typedef data::SetNodeAccess DataAccess;
            DataAccess getDataAccess(data::Accessor const& _aAccessor) const;

            /// Get the template that describes elements of this set
            TemplateHolder getElementTemplate() const { return m_aTemplate; }

            /// Get a template provider that can create new elements for this set
            TemplateProvider getTemplateProvider() const { return m_aTemplateProvider; }

            void convertChanges(NodeChangesInformation& rLocalChanges, data::Accessor const& _accessor, SubtreeChange const& rExternalChange, TreeDepth nDepth);

            void    insertElement(Name const& aName, Element const& aNewElement);
            Element replaceElement(Name const& aName, Element const& aNewElement);
            Element removeElement(Name const& aName);

            void rebuildFrom(SetNodeImpl& rOldData,data::SetNodeAccess const& _aNewNode,data::SetNodeAccess const& _aOldNode);

        protected:
            ~SetNodeImpl();

        protected:
        // new overrideables
            virtual bool                    doIsEmpty() const;
            virtual ElementTreeImpl*        doFindElement(Name const& aName) ;
            virtual SetNodeVisitor::Result  doDispatchToElements(data::Accessor const& _aAccessor, SetNodeVisitor& aVisitor);
            virtual void doDifferenceToDefaultState(data::Accessor const& _aAccessor, SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree);

            virtual SetElementChangeImpl* doAdjustToAddedElement(data::Accessor const& _aAccessor, Name const& aName, AddNode const& aAddNodeChange, Element const & aNewElement);
            virtual SetElementChangeImpl* doAdjustToRemovedElement(data::Accessor const& _aAccessor, Name const& aName, RemoveNode const& aRemoveNodeChange);
            virtual SetElementChangeImpl* doAdjustChangedElement(data::Accessor const& _aAccessor, NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange);

            virtual void doTransferElements(ElementSet& rReplacement);

        protected:
        // helpers
            TreeImpl*   getParentTree() const;
            NodeOffset  getContextOffset() const;

            Element makeElement(data::Accessor const& _aAccessor, SetEntry const & _anEntry);
            static Element entryToElement(SetEntry const& _anEntry);
            view::ViewTreeAccess getElementView();

            /// Initialize the set data: Set context information, and build the view (actually loading the elements may be deferred)
            friend class TreeImplBuilder;
            void initElements(TemplateProvider const& aTemplateProvider, TreeImpl& rParentTree, NodeOffset nPos, TreeDepth nDepth);

        protected:
            /// does this set contain any elements (loads elements if needed)
            bool implHasLoadedElements() const;
            bool implLoadElements(data::Accessor const& _aAccessor);
            void implEnsureElementsLoaded(data::Accessor const& _aAccessor);
            void implInitElements(data::SetNodeAccess const& _aNode, TreeDepth nDepth);
            void implInitElement(Element const& aNewElement);

            void implRebuildElements(data::SetNodeAccess const& _aNewNode,data::SetNodeAccess const& _aOldNode);
        protected:
            SetElementChangeImpl* implCreateInsert    (data::Accessor const& _aAccessor, Name const& aName, Element const& aNewElement) const;
            SetElementChangeImpl* implCreateReplace   (data::Accessor const& _aAccessor, Name const& aName, Element const& aNewElement, Element const& aOldElement) const;
            SetElementChangeImpl* implCreateRemove    (data::Accessor const& _aAccessor, Name const& aName, Element const& aOldElement) const;

            SetElementChangeImpl* implAdjustToAddedElement(data::Accessor const& _aAccessor, Name const& aName, Element const& aNewElement, bool _bReplacing);
            SetElementChangeImpl* implAdjustToRemovedElement(data::Accessor const& _aAccessor, Name const& aName);

            Element makeAdditionalElement(data::Accessor const& _aAccessor, rtl::Reference<view::ViewStrategy> const& _xStrategy, AddNode const& aAddNodeChange, TreeDepth nDepth);

            Element implValidateElement(data::Accessor const& _aAccessor, Element const& aNewElement);

            void implDifferenceToDefaultState(data::Accessor const& _aAccessor, SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree) const;
        protected:
            Element* getStoredElement(Name const& aName)
            { return m_aDataSet.getElement(aName); }
            Element const* getStoredElement(Name const& aName) const
            { return m_aDataSet.getElement(aName); }

            typedef ElementSet::PairIterator NativeIterator;
            NativeIterator beginElementSet() const
            { return m_aDataSet.beginNative(); }
            NativeIterator endElementSet() const
            { return m_aDataSet.endNative(); }

            void attach(Element const& aNewElement, Name const& aName);
            void detach(Element const& aNewElement);
        };

//-----------------------------------------------------------------------------
        // domain-specific 'dynamic_cast' replacement
        SetNodeImpl&    AsSetNode  (NodeImpl& rNode);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_SETNODEBEHAVIOR_HXX_

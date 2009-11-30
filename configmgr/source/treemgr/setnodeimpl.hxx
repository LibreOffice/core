/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setnodeimpl.hxx,v $
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

#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#define CONFIGMGR_SETNODEBEHAVIOR_HXX_

#include "nodeimpl.hxx"
#include "tree.hxx"
#include "template.hxx"
#include <rtl/ref.hxx>

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
    namespace view { class ViewTreeAccess; }
//-----------------------------------------------------------------------------

    namespace configuration
    {
//-----------------------------------------------------------------------------
        class SetElementChangeImpl;

//-----------------------------------------------------------------------------

        struct SetEntry
        {
            SetEntry(ElementTree* _pTree);

            bool isValid()  const { return m_pTree != 0; }

            ElementTree* tree() const { return m_pTree; };

            view::ViewTreeAccess    getTreeView() const;
        private:
            ElementTree* m_pTree;
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
            // construction
            ElementTreeData() : tree(), inDefault(false) {}

            ElementTreeData(rtl::Reference<ElementTree> const& _tree, bool _bDefault)
             : tree(_tree), inDefault(_bDefault) {}

            // ORef compatibility
            sal_Bool isValid() const { return this->tree.is(); }
            ElementTree* get() const { return this->tree.get(); }
            rtl::Reference<ElementTree> const& operator->() const { return this->tree; }
            ElementTree& operator*() const { return *get(); }

            // data
            rtl::Reference<ElementTree>   tree;
            bool                inDefault;
        };
    //-----------------------------------------------------------------------------

        class ElementSet
        {
            struct FastLess
            {
                bool operator ()(
                    rtl::OUString const & a, rtl::OUString const & b) const
                {
                    // first sort by length; order is immaterial, and it is fast
                    return a.getLength() == b.getLength()
                        ? a < b : a.getLength() < b.getLength();
                }
            };

        public:
            typedef std::map<rtl::OUString, ElementTreeData, FastLess> Data;

        // the following must be implemented by derived classes
            bool isEmpty() const { return m_aData.empty(); }

            bool hasElement(rtl::OUString const& aName) const;
            ElementTreeData* getElement(rtl::OUString const& aName);
            ElementTreeData const* getElement(rtl::OUString const& aName) const;
            ElementTreeData findElement(rtl::OUString const& aName);

            void insertElement(rtl::OUString const& aName, ElementTreeData const& aNewEntry);
            ElementTreeData replaceElement(rtl::OUString const& aName, ElementTreeData const& aNewEntry);
            ElementTreeData removeElement(rtl::OUString const& aName);

            void clearElements() {  m_aData.clear(); }

            void swap(ElementSet& aOther) { m_aData.swap(aOther.m_aData); }

        // STL style iteration
            class ConstIterator
            {
            public:
                ConstIterator(Data::const_iterator const& it) : m_base(it) {}

                ElementTreeData const& operator* () const { return  m_base->second; }
                ElementTreeData const* operator->() const { return &m_base->second; }

                ConstIterator& operator++()     { ++m_base; return *this; }
                ConstIterator  operator++(int)  { return ConstIterator(m_base++); }

                ConstIterator& operator--()     { --m_base; return *this; }
                ConstIterator  operator--(int)  { return ConstIterator(m_base--); }

                friend bool operator ==(ConstIterator const& lhs, ConstIterator const& rhs)
                { return lhs.m_base == rhs.m_base; }
                friend bool operator !=(ConstIterator const& lhs, ConstIterator const& rhs)
                { return lhs.m_base != rhs.m_base; }
            private:
                Data::const_iterator m_base;
            };
            ConstIterator begin() const { return ConstIterator(m_aData.begin()); }
            ConstIterator end()   const { return ConstIterator(m_aData.end()); }

            class Iterator
            {
            public:
                Iterator(Data::iterator const& it) : m_base(it) {}

                ElementTreeData& operator* () const { return  m_base->second; }
                ElementTreeData* operator->() const { return &m_base->second; }

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
                Data::iterator m_base;
            };
            Iterator begin()    { return Iterator(m_aData.begin()); }
            Iterator end()      { return Iterator(m_aData.end()); }

            Data::const_iterator beginNative()  const { return m_aData.begin(); }
            Data::const_iterator endNative()    const { return m_aData.end(); }
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
            rtl::Reference<Template>        m_aTemplate;
            TemplateProvider    m_aTemplateProvider;
            Tree*           m_pParentTree;
            unsigned int            m_nContextPos;

            unsigned int        m_aInit;

        public:
            SetNodeImpl(sharable::SetNode * _pNodeRef, Template* pTemplate);

            sharable::SetNode * getDataAccess() const;

            /// Get the template that describes elements of this set
            rtl::Reference<Template> getElementTemplate() const { return m_aTemplate; }

            /// Get a template provider that can create new elements for this set
            TemplateProvider getTemplateProvider() const { return m_aTemplateProvider; }

            void convertChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChange, unsigned int nDepth);

            void    insertElement(rtl::OUString const& aName, ElementTreeData const& aNewElement);
            ElementTreeData replaceElement(rtl::OUString const& aName, ElementTreeData const& aNewElement);
            ElementTreeData removeElement(rtl::OUString const& aName);

            void rebuildFrom(SetNodeImpl& rOldData, sharable::SetNode * newNode);

        protected:
            ~SetNodeImpl();

        protected:
        // new overrideables
            virtual bool                    doIsEmpty() const;
            virtual ElementTree*        doFindElement(rtl::OUString const& aName) ;
            virtual SetNodeVisitor::Result  doDispatchToElements( SetNodeVisitor& aVisitor);
            virtual void doDifferenceToDefaultState( SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree);

            virtual SetElementChangeImpl* doAdjustToAddedElement( rtl::OUString const& aName, AddNode const& aAddNodeChange, ElementTreeData const & aNewElement);
            virtual SetElementChangeImpl* doAdjustToRemovedElement( rtl::OUString const& aName, RemoveNode const& aRemoveNodeChange);
            virtual SetElementChangeImpl* doAdjustChangedElement( NodeChangesInformation& rLocalChanges, rtl::OUString const& aName, Change const& aChange);

            virtual void doTransferElements(ElementSet& rReplacement);

        protected:
        // helpers
            Tree*   getParentTree() const;
            unsigned int    getContextOffset() const;

            ElementTreeData makeElement( SetEntry const & _anEntry);
            static ElementTreeData entryToElement(SetEntry const& _anEntry);
            view::ViewTreeAccess getElementView();

            /// Initialize the set data: Set context information, and build the view (actually loading the elements may be deferred)
            friend class TreeImplBuilder;
            void initElements(TemplateProvider const& aTemplateProvider, Tree& rParentTree, unsigned int nPos, unsigned int nDepth);

        protected:
            /// does this set contain any elements (loads elements if needed)
            bool implHasLoadedElements() const;
            bool implLoadElements();
            void implEnsureElementsLoaded();
            void implInitElements(sharable::SetNode * node, unsigned int nDepth);
            void implInitElement(ElementTreeData const& aNewElement);

            void implRebuildElements(sharable::SetNode * newNode);
        protected:
            SetElementChangeImpl* implCreateInsert    ( rtl::OUString const& aName, ElementTreeData const& aNewElement) const;
            SetElementChangeImpl* implCreateReplace   ( rtl::OUString const& aName, ElementTreeData const& aNewElement, ElementTreeData const& aOldElement) const;
            SetElementChangeImpl* implCreateRemove    ( rtl::OUString const& aName, ElementTreeData const& aOldElement) const;

            SetElementChangeImpl* implAdjustToAddedElement( rtl::OUString const& aName, ElementTreeData const& aNewElement, bool _bReplacing);
            SetElementChangeImpl* implAdjustToRemovedElement( rtl::OUString const& aName);

            ElementTreeData makeAdditionalElement( rtl::Reference<view::ViewStrategy> const& _xStrategy, AddNode const& aAddNodeChange, unsigned int nDepth);

            ElementTreeData implValidateElement(ElementTreeData const& aNewElement);

            void implDifferenceToDefaultState( SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree) const;
        protected:
            bool hasStoredElement(rtl::OUString const& aName) const
            { return m_aDataSet.hasElement(aName); }
            ElementTreeData* getStoredElement(rtl::OUString const& aName)
            { return m_aDataSet.getElement(aName); }
            ElementTreeData const* getStoredElement(rtl::OUString const& aName) const
            { return m_aDataSet.getElement(aName); }

            ElementSet::Data::const_iterator beginElementSet() const
            { return m_aDataSet.beginNative(); }
            ElementSet::Data::const_iterator endElementSet() const
            { return m_aDataSet.endNative(); }

            void attach(ElementTreeData const& aNewElement, rtl::OUString const& aName);
            void detach(ElementTreeData const& aNewElement);
        };

//-----------------------------------------------------------------------------
        // domain-specific 'dynamic_cast' replacement
        SetNodeImpl&    AsSetNode  (NodeImpl& rNode);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_SETNODEBEHAVIOR_HXX_

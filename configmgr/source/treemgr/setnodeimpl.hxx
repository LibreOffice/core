/*************************************************************************
 *
 *  $RCSfile: setnodeimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 17:20:54 $
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

#ifndef CONFIGMGR_SETNODEIMPL_HXX_
#define CONFIGMGR_SETNODEIMPL_HXX_

#include "nodeimpl.hxx"
#include "treeimpl.hxx"

#include <vos/ref.hxx>
#include <map>

namespace configmgr
{
//-----------------------------------------------------------------------------
    class AddNode;
    class RemoveNode;
//-----------------------------------------------------------------------------

    namespace configuration
    {
//-----------------------------------------------------------------------------
        struct NodeFactory;

        class ElementTreeImpl;
        typedef vos::ORef<ElementTreeImpl> ElementTreeHolder;

    // basic implementations of set node contents
    //-----------------------------------------------------------------------------

        class ElementSet
        {
        public:
            typedef ElementTreeHolder Element;
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

        class AbstractSetNodeImpl : public SetNodeImpl
        {
        public:
            typedef ElementSet::Element Element;

            AbstractSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);
            AbstractSetNodeImpl(AbstractSetNodeImpl& rOriginal); // for making (in)direct - takes the data along !

        // base class implementation (and helpers)
        protected:
            bool        doIsEmpty() const;
            SetEntry    doFindElement(Name const& aName) ;

            void        doClearElements();

            void        doAdjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChanges, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

            SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor);

            void    implInsertElement(Name const& aName, Element const& aNewElement, bool bCommit);
            void    implReplaceElement(Name const& aName, Element const& aNewElement, bool bCommit);
            void    implRemoveElement(Name const& aName, bool bCommit);

            void    implInitElement(Element const& aNewElement);
            void    implMakeIndirect(bool bIndirect); // ensures kids are (in)direct

            void implAdjustToElementChange(NodeChangesInformation& rLocalChanges, Change const& aChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

        // new overrideables
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth) = 0;

            virtual void doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange, TemplateProvider const& aTemplateProvider);

            virtual NodeChangeImpl* doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement);
            virtual NodeChangeImpl* doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange);

            virtual NodeChangeImpl* doCreateInsert(Name const& aName, Element const& aNewElement) const;
            virtual NodeChangeImpl* doCreateReplace(Name const& aName, Element const& aNewElement, Element const& aOldElement) const;
            virtual NodeChangeImpl* doCreateRemove(Name const& aName, Element const& aOldElement) const;
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

            void attach(Element const& aNewElement, Name const& aName, bool bCommit);
            void detach(Element const& aNewElement, bool bCommit);
        private:
            ElementSet m_aDataSet;
        };

//-----------------------------------------------------------------------------

        class TreeSetNodeImpl : public AbstractSetNodeImpl
        {
        public:
            explicit
            TreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
            : AbstractSetNodeImpl(rOriginal,pTemplate)
            {}

        // base class implementation (or helpers)
        protected:
            NodeType::Enum  doGetType() const;

            void doInsertElement(Name const& aName, SetEntry const& aNewEntry) = 0;
            void doRemoveElement(Name const& aName) = 0;

            void initHelper(TemplateProvider const& aTemplateProvider, NodeFactory& rFactory, ISubtree& rTree, TreeDepth nDepth);
            Element makeAdditionalElement(TemplateProvider const& aTemplateProvider, NodeFactory& rFactory, AddNode const& aAddNodeChange, TreeDepth nDepth);

            ElementTreeHolder implMakeElement(ElementTreeHolder const& aNewEntry);
        };
    //-------------------------------------------------------------------------

        class ValueSetNodeImpl : public AbstractSetNodeImpl
        {
        public:
            explicit
            ValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
            : AbstractSetNodeImpl(rOriginal,pTemplate)
            {}

        // base class implementations (or helpers)
        protected:
            NodeType::Enum  doGetType() const;

            void doInsertElement(Name const& aName, SetEntry const& aNewEntry) = 0;
            void doRemoveElement(Name const& aName) = 0;

            void initHelper(TemplateProvider const& aTemplateProvider, NodeFactory& rFactory, ISubtree& rTree);
            Element makeAdditionalElement(TemplateProvider const& aTemplateProvider, NodeFactory& rFactory, AddNode const& aAddNodeChange);

            ElementTreeHolder implMakeElement(ElementTreeHolder const& aNewEntry);
        };

//-----------------------------------------------------------------------------

    }
}

#endif // CONFIGMGR_SETNODEIMPL_HXX_

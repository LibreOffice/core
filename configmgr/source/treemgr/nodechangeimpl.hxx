/*************************************************************************
 *
 *  $RCSfile: nodechangeimpl.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:46 $
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

#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#define CONFIGMGR_CONFIGCHANGEIMPL_HXX_

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

#ifndef CONFIGMGR_VIEWACCESS_HXX_
#include "viewaccess.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
    class ISubtree;

    namespace view { class ViewTreeAccess; struct Node; struct GroupNode; struct SetNode; }
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
//-----------------------------------------------------------------------------

        class ValueMemberNode;
        class ValueMemberUpdate;
//-----------------------------------------------------------------------------
        class NodeChangeData;
        class NodeChangeLocation;
        class NodeChangeInformation;
//-----------------------------------------------------------------------------

        typedef rtl::Reference<TreeImpl>        TreeHolder;
        typedef rtl::Reference<ElementTreeImpl> ElementTreeHolder;
//-----------------------------------------------------------------------------
        struct ElementTreeChange
        {
            Path::Component m_aElementName;
            ElementTreeHolder  m_aAddedElement;
            ElementTreeHolder  m_aRemovedElement;

            ElementTreeChange(
                Path::Component const& _aElementName,
                ElementTreeHolder  const& _aAddedElement,
                ElementTreeHolder  const& _aRemovedElement
             )
            : m_aElementName(_aElementName)
            , m_aAddedElement(_aAddedElement)
            , m_aRemovedElement(_aRemovedElement)
            {}

            bool isChange() const
            {
                return !!(m_aAddedElement != m_aRemovedElement);
            }
        };
//-----------------------------------------------------------------------------


        /// represents a node position in some tree
        class NodeChangeImpl
        : public salhelper::SimpleReferenceObject
        {
        public:
            explicit
            NodeChangeImpl(bool bNoCheck = false);

        public:
        // related/affected nodes and trees
            /// the tree within which the change occurs
            TreeHolder getTargetTree() const;

            /// the node that is affected by the change
            NodeOffset getTargetNode() const;

            data::Accessor getDataAccessor() const { return m_aDataAccessor; }
        protected:
            /// setup the 'target' node that is to be affected or changed
            void setTarget(data::Accessor const& _aAccessor, TreeHolder const& _aAffectedTree, NodeOffset _nAffectedNode);
            void setTarget(view::Node _aAffectedNode);

            view::ViewTreeAccess getTargetView();
        public:
        // getting information
            typedef sal_uInt32 ChangeCount;
            /*static const ChangeCount*/ enum { scCommonBase = ~0u };

            /// checks, if this represents an actual change - with or without requiring a preceding test
            bool isChange(bool bAllowUntested) const;

            /// return the number of distict changes in this object
            ChangeCount getChangeDataCount() const;

            /// fills in base change location, returns whether it is set
            bool fillChangeLocation(NodeChangeLocation& rChange, ChangeCount _ix = scCommonBase) const;

            /// fills in pre- and post-change values, returns whether they may differ
            bool fillChangeData(NodeChangeData& rChange, ChangeCount _ix) const;

            /// fills in change location and values, returns whether data may be changed
            bool fillChangeInfo(NodeChangeInformation& rChange, ChangeCount _ix) const;

        /// test whether this really is a change to the stored 'changing' node
            void test();

        /// apply this change to the stored 'changing' node
            void apply();

        private:
        /// virtual hooks for some of the public methods
            /// return the number of distict changes in this object
            ChangeCount doGetChangeCount() const;

            /// the path from base to 'changing' node
            virtual RelativePath doGetChangingNodePath(ChangeCount _ix) const = 0;

            /// is the change really affecting a child (or children) of the affected node (true for values)
            virtual bool doIsChangingSubnode() const = 0;

            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const = 0;

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const = 0;

            /// dry-check whether this is a change
            virtual void doTest( view::Node const& rTarget) = 0;
            /// do apply the actual change
            virtual void doApply( view::Node const& rTarget) = 0;

        private:
            typedef sal_uInt16 State;
            data::Accessor m_aDataAccessor;
            TreeHolder m_aAffectedTree;
            NodeOffset m_nAffectedNode;
            State      m_nState;

            void implApply();
            view::Node implGetTarget();
        };
//-----------------------------------------------------------------------------

        /// represents a node position in some tree
        class ValueChangeImpl
        : public NodeChangeImpl
        {
            Name   m_aName;
            UnoAny m_aNewValue;
            UnoAny m_aOldValue;
        public:
            explicit ValueChangeImpl();
            explicit ValueChangeImpl(UnoAny const& aNewValue);
            explicit ValueChangeImpl(UnoAny const& aNewValue, UnoAny const& aOldValue);
            ~ValueChangeImpl();

        public:
            /// setup the 'target' node that is to be affected or changed
            void setTarget(view::GroupNode const& _aParentNode, Name const& sNodeName);
            void setTarget(data::Accessor const& _aAccessor, TreeHolder const& aAffectedTree, NodeOffset nParentNode, Name const& sNodeName);

        public:
            /// get the name of the value
            Name getValueName() const { return m_aName; }

            /// get the pre-change value (if known)
            UnoAny getOldValue() const { return m_aOldValue; }
            /// get the post-change value (if known)
            UnoAny getNewValue() const { return m_aNewValue; }

        protected:
        // override information items
            /// the path from base to 'affected' node - here is the name of the changing node
            virtual RelativePath doGetChangingNodePath(ChangeCount _ix) const;

            /// is the change really affecting a child of the affected node (true here)
            virtual bool doIsChangingSubnode() const;

        protected:
        // override change information items
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const = 0;

        protected:
        // override apply functionality
            /// retrieve the old value from the given node
            virtual void doTest( view::Node const& rTarget);
            /// do apply the actual change
            virtual void doApply( view::Node const& rTarget);

        protected:
        // new overrideables
            /// extract the pre-change value from the target context
            virtual void preCheckValue(ValueMemberNode& rNode, UnoAny& rOld, UnoAny& rNew);
            /// extract the post-change value from the target context
            virtual void postCheckValue(ValueMemberNode& rNode, UnoAny& rNew);
            /// apply the new value to the target context
            virtual void doApplyChange(ValueMemberUpdate& rNode) = 0;
        };
//-----------------------------------------------------------------------------

        /// represents setting a value node to a given value
        class ValueReplaceImpl
        : public ValueChangeImpl
        {
        public:
            explicit ValueReplaceImpl(UnoAny const& aNewValue);
            explicit ValueReplaceImpl(UnoAny const& aNewValue, UnoAny const& aOldValue);

        protected:
            // implement: set the target to the new value
            virtual void doApplyChange( ValueMemberUpdate& rNode);

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const;

//          friend class SetReplaceValueImpl;
        };
//-----------------------------------------------------------------------------

        /// represents resetting a value node to its default value
        class ValueResetImpl
        : public ValueChangeImpl
        {
        public:
            explicit ValueResetImpl();
            explicit ValueResetImpl(UnoAny const& aNewValue, UnoAny const& aOldValue);

        protected:
            // override: set the new value as well
            virtual void preCheckValue(ValueMemberNode& rNode, UnoAny& rOld, UnoAny& rNew);

            // implement: set the target to default
            virtual void doApplyChange( ValueMemberUpdate& rNode);

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const;
        };
//-----------------------------------------------------------------------------


        /// represents a change to a set (as a container)
        class SetChangeImpl
        : public NodeChangeImpl
        {
        public:
            explicit SetChangeImpl(bool bNoCheck = false);

            using NodeChangeImpl::setTarget;

        protected:
        /// virtual hooks for some of the public methods
            /// is the change really affecting a child of the affected node (false here)
            virtual bool doIsChangingSubnode() const;
        };
//-----------------------------------------------------------------------------
        class SetElementFactory;

        /// represents setting to its default state a set (as a container)
        class SetResetImpl
        : public SetChangeImpl
        {
            typedef std::vector< ElementTreeChange >    TreeChanges;

            std::auto_ptr<ISubtree>             m_aDefaultData;
            SetElementFactory&                  m_rElementFactory;
            TreeChanges                         m_aTreeChanges;
        public:
            explicit SetResetImpl(
                SetElementFactory& _rElementFactory,
                std::auto_ptr<ISubtree> _pDefaultData,
                bool _bNoCheck = false);

            ~SetResetImpl();

        protected:
        /// virtual hooks for some of the public methods
            /// retrieve the count of elements affected
            ChangeCount doGetChangeCount() const;

            /// the path from base to 'affected' node
            virtual RelativePath doGetChangingNodePath(ChangeCount _ix) const;

            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const;


            /// retrieve the old value from the given node
            virtual void doTest( view::Node const& rTarget);
            /// do apply the actual change
            virtual void doApply( view::Node const& rTarget);
        };
//-----------------------------------------------------------------------------

        /// represents a change to an element of a set (as a container)
        class SetElementChangeImpl
        : public SetChangeImpl
        {
            Path::Component m_aName;
        public:
            explicit SetElementChangeImpl(Path::Component const& aName, bool bNoCheck = false);

            /// the name of the element being changed
            Path::Component getFullElementName() const { return m_aName; }

            /// the name of the element being changed
            Name getElementName() const { return m_aName.getName(); }

        protected:
        /// virtual hooks for some of the public methods
            /// the path from base to 'affected' node - use element name
            virtual RelativePath doGetChangingNodePath(ChangeCount _ix) const;

            /// retrieve the old value from the given node
            virtual void doTest( view::Node const& rTarget);
            /// do apply the actual change
            virtual void doApply( view::Node const& rTarget);

        private:
            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, Name const& aName) = 0;
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, Name const& aName) = 0;
        };
//-----------------------------------------------------------------------------

        /// represents an insertion into a set of trees
        class SetInsertImpl
        : public SetElementChangeImpl
        {
            ElementTreeHolder m_aNewTree;
        public:
            explicit SetInsertImpl(Path::Component const& aName, ElementTreeHolder const& aNewTree, bool bNoCheck = false);

        protected:
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const;

            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, Name const& aName);
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, Name const& aName);
        };
//-----------------------------------------------------------------------------

        /// represents a substitution within a set of trees
        class SetReplaceImpl
        : public SetElementChangeImpl
        {
            ElementTreeHolder m_aNewTree;
            ElementTreeHolder m_aOldTree;
        public:
            explicit SetReplaceImpl(Path::Component const& aName, ElementTreeHolder const& aNewTree);
            explicit SetReplaceImpl(Path::Component const& aName, ElementTreeHolder const& aNewTree, ElementTreeHolder const& aOldTree);

        protected:
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const;

            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, Name const& aName);
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, Name const& aName);
        };
//-----------------------------------------------------------------------------

        /// represents a removal from of a set of trees
        class SetRemoveImpl
        : public SetElementChangeImpl
        {
            ElementTreeHolder m_aOldTree;
        public:
            explicit SetRemoveImpl(Path::Component const& aName);
            explicit SetRemoveImpl(Path::Component const& aName, ElementTreeHolder const& aOldTree);

        protected:
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, ChangeCount _ix) const;

            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, Name const& aName);
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, Name const& aName);
        };
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGCHANGEIMPL_HXX_

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodechangeimpl.hxx,v $
 * $Revision: 1.14 $
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

#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#define CONFIGMGR_CONFIGCHANGEIMPL_HXX_

#include "configexcept.hxx"
#include "configpath.hxx"
#include "viewaccess.hxx"
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include "utility.hxx"

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

        class ValueMemberNode;
        class ValueMemberUpdate;
//-----------------------------------------------------------------------------
        class NodeChangeData;
        class NodeChangeLocation;
        class NodeChangeInformation;
//-----------------------------------------------------------------------------
        struct ElementTreeChange
        {
            Path::Component m_aElementName;
            rtl::Reference<ElementTree>  m_aAddedElement;
            rtl::Reference<ElementTree>  m_aRemovedElement;

            ElementTreeChange(
                Path::Component const& _aElementName,
                rtl::Reference<ElementTree>  const& _aAddedElement,
                rtl::Reference<ElementTree>  const& _aRemovedElement
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
            rtl::Reference<Tree> getTargetTree() const;

            /// the node that is affected by the change
            unsigned int getTargetNode() const;

        protected:
            /// setup the 'target' node that is to be affected or changed
            void setTarget(rtl::Reference<Tree> const& _aAffectedTree, unsigned int _nAffectedNode);
            void setTarget(view::Node _aAffectedNode);

            view::ViewTreeAccess getTargetView();
        public:
        // getting information
            /*static const sal_uInt32*/ enum { scCommonBase = ~0u };

            /// checks, if this represents an actual change - with or without requiring a preceding test
            bool isChange(bool bAllowUntested) const;

            /// return the number of distict changes in this object
            sal_uInt32 getChangeDataCount() const;

            /// fills in base change location, returns whether it is set
            bool fillChangeLocation(NodeChangeLocation& rChange, sal_uInt32 _ix = scCommonBase) const;

            /// fills in pre- and post-change values, returns whether they may differ
            bool fillChangeData(NodeChangeData& rChange, sal_uInt32 _ix) const;

            /// fills in change location and values, returns whether data may be changed
            bool fillChangeInfo(NodeChangeInformation& rChange, sal_uInt32 _ix) const;

        /// test whether this really is a change to the stored 'changing' node
            void test();

        /// apply this change to the stored 'changing' node
            void apply();

        private:
        /// virtual hooks for some of the public methods
            /// return the number of distict changes in this object
            sal_uInt32 doGetChangeCount() const;

            /// the path from base to 'changing' node
            virtual RelativePath doGetChangingNodePath(sal_uInt32 _ix) const = 0;

            /// is the change really affecting a child (or children) of the affected node (true for values)
            virtual bool doIsChangingSubnode() const = 0;

            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const = 0;

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const = 0;

            /// dry-check whether this is a change
            virtual void doTest( view::Node const& rTarget) = 0;
            /// do apply the actual change
            virtual void doApply( view::Node const& rTarget) = 0;

        private:
            rtl::Reference<Tree> m_aAffectedTree;
            unsigned int m_nAffectedNode;
            sal_uInt16      m_nState;

            void implApply();
            view::Node implGetTarget();
        };
//-----------------------------------------------------------------------------

        /// represents a node position in some tree
        class ValueChangeImpl
        : public NodeChangeImpl
        {
            rtl::OUString m_aName;
            com::sun::star::uno::Any m_aNewValue;
            com::sun::star::uno::Any m_aOldValue;
        public:
            explicit ValueChangeImpl();
            explicit ValueChangeImpl(com::sun::star::uno::Any const& aNewValue);
            explicit ValueChangeImpl(com::sun::star::uno::Any const& aNewValue, com::sun::star::uno::Any const& aOldValue);
            ~ValueChangeImpl();

        public:
            /// setup the 'target' node that is to be affected or changed
            void setTarget(view::GroupNode const& _aParentNode, rtl::OUString const& sNodeName);
            void setTarget(rtl::Reference<Tree> const& aAffectedTree, unsigned int nParentNode, rtl::OUString const& sNodeName);

        public:
            /// get the name of the value
            rtl::OUString getValueName() const { return m_aName; }

            /// get the pre-change value (if known)
            com::sun::star::uno::Any getOldValue() const { return m_aOldValue; }
            /// get the post-change value (if known)
            com::sun::star::uno::Any getNewValue() const { return m_aNewValue; }

        protected:
                     using NodeChangeImpl::setTarget;
        // override information items
            /// the path from base to 'affected' node - here is the name of the changing node
            virtual RelativePath doGetChangingNodePath(sal_uInt32 _ix) const;

            /// is the change really affecting a child of the affected node (true here)
            virtual bool doIsChangingSubnode() const;

        protected:
        // override change information items
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const = 0;

        protected:
        // override apply functionality
            /// retrieve the old value from the given node
            virtual void doTest( view::Node const& rTarget);
            /// do apply the actual change
            virtual void doApply( view::Node const& rTarget);

        protected:
        // new overrideables
            /// extract the pre-change value from the target context
            virtual void preCheckValue(ValueMemberNode& rNode, com::sun::star::uno::Any& rOld, com::sun::star::uno::Any& rNew);
            /// extract the post-change value from the target context
            virtual void postCheckValue(ValueMemberNode& rNode, com::sun::star::uno::Any& rNew);
            /// apply the new value to the target context
            virtual void doApplyChange(ValueMemberUpdate& rNode) = 0;
        };
//-----------------------------------------------------------------------------

        /// represents setting a value node to a given value
        class ValueReplaceImpl
        : public ValueChangeImpl
        {
        public:
            explicit ValueReplaceImpl(com::sun::star::uno::Any const& aNewValue);
            explicit ValueReplaceImpl(com::sun::star::uno::Any const& aNewValue, com::sun::star::uno::Any const& aOldValue);

        protected:
            // implement: set the target to the new value
            virtual void doApplyChange( ValueMemberUpdate& rNode);

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const;

//          friend class SetReplaceValueImpl;
        };
//-----------------------------------------------------------------------------

        /// represents resetting a value node to its default value
        class ValueResetImpl
        : public ValueChangeImpl
        {
            bool m_bTargetIsDefault;
        public:
            explicit ValueResetImpl();
            explicit ValueResetImpl(com::sun::star::uno::Any const& aNewValue, com::sun::star::uno::Any const& aOldValue);

        protected:
            // override: set the new value as well and check the default state
            virtual void preCheckValue(ValueMemberNode& rNode, com::sun::star::uno::Any& rOld, com::sun::star::uno::Any& rNew);

            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;

            // implement: set the target to default
            virtual void doApplyChange( ValueMemberUpdate& rNode);

            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const;
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
            std::auto_ptr<ISubtree>             m_aDefaultData;
            SetElementFactory&                  m_rElementFactory;
            std::vector< ElementTreeChange >                         m_aTreeChanges;
        public:
            explicit SetResetImpl(
                SetElementFactory& _rElementFactory,
                std::auto_ptr<ISubtree> _pDefaultData,
                bool _bNoCheck = false);

            ~SetResetImpl();

        protected:
        /// virtual hooks for some of the public methods
            /// retrieve the count of elements affected
            sal_uInt32 doGetChangeCount() const;

            /// the path from base to 'affected' node
            virtual RelativePath doGetChangingNodePath(sal_uInt32 _ix) const;

            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const;


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
            rtl::OUString getElementName() const { return m_aName.getName(); }

        protected:
        /// virtual hooks for some of the public methods
            /// the path from base to 'affected' node - use element name
            virtual RelativePath doGetChangingNodePath(sal_uInt32 _ix) const;

            /// retrieve the old value from the given node
            virtual void doTest( view::Node const& rTarget);
            /// do apply the actual change
            virtual void doApply( view::Node const& rTarget);

        private:
            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, rtl::OUString const& aName) = 0;
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, rtl::OUString const& aName) = 0;
        };
//-----------------------------------------------------------------------------

        /// represents an insertion into a set of trees
        class SetInsertImpl
        : public SetElementChangeImpl
        {
            rtl::Reference<ElementTree> m_aNewTree;
        public:
            explicit SetInsertImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aNewTree, bool bNoCheck = false);

        protected:
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const;

            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, rtl::OUString const& aName);
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, rtl::OUString const& aName);
        };
//-----------------------------------------------------------------------------

        /// represents a substitution within a set of trees
        class SetReplaceImpl
        : public SetElementChangeImpl
        {
            rtl::Reference<ElementTree> m_aNewTree;
            rtl::Reference<ElementTree> m_aOldTree;
        public:
            explicit SetReplaceImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aNewTree);
            explicit SetReplaceImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aNewTree, rtl::Reference<ElementTree> const& aOldTree);

        protected:
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const;

            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, rtl::OUString const& aName);
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, rtl::OUString const& aName);
        };
//-----------------------------------------------------------------------------

        /// represents a removal from of a set of trees
        class SetRemoveImpl
        : public SetElementChangeImpl
        {
            rtl::Reference<ElementTree> m_aOldTree;
        public:
            explicit SetRemoveImpl(Path::Component const& aName);
            explicit SetRemoveImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aOldTree);

        protected:
            /// checks, if this represents an actual change (given whether the change has been applied or not)
            virtual bool doIsChange() const;
            /// fills in pre- and post-change values, returns wether they differ
            virtual bool doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const;

            /// new overridable: retrieve the old value from a properly typed node
            virtual void doTestElement(view::SetNode const& _aNode, rtl::OUString const& aName);
            /// new overridable: apply the change to a properly typed node
            virtual void doApplyToElement(view::SetNode const& _aNode, rtl::OUString const& aName);
        };
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGCHANGEIMPL_HXX_

/*************************************************************************
 *
 *  $RCSfile: nodeimplobj.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-23 10:50:58 $
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

#ifndef CONFIGMGR_NODEIMPLOBJECTS_HXX_
#define CONFIGMGR_NODEIMPLOBJECTS_HXX_

#include "nodeimpl.hxx"
#include "setnodeimpl.hxx"
#include <memory>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

// Specific types of nodes for direct or read only access
//-----------------------------------------------------------------------------

// Value Nodes
//-----------------------------------------------------------------------------

        class ReadOnlyValueNodeImpl : public ValueNodeImpl
        {
        public:
            explicit
            ReadOnlyValueNodeImpl(ValueNode& rOriginal)
            : ValueNodeImpl(rOriginal)
            {}

            virtual bool    isDefault()         const;
            virtual bool    canGetDefaultValue() const;
            virtual UnoAny  getValue()          const;
            virtual UnoAny  getDefaultValue()   const;

            virtual UnoType getValueType()  const;

            virtual void    setValue(UnoAny const& aNewValue);
            virtual void    setDefault();

        protected:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual bool doHasChanges() const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredValueNodeImpl;

        class DirectValueNodeImpl : public ValueNodeImpl
        {
        public:
            explicit
            DirectValueNodeImpl(ValueNode& rOriginal);

            explicit
            DirectValueNodeImpl(DeferredValueNodeImpl& rOriginal);

            virtual bool    isDefault()         const;
            virtual bool    canGetDefaultValue() const;
            virtual UnoAny  getValue()          const;
            virtual UnoAny  getDefaultValue()   const;

            virtual UnoType getValueType()  const;

            virtual void    setValue(UnoAny const& aNewValue);
            virtual void    setDefault();

        protected:
        // NodeImpl implementation
            virtual bool doHasChanges() const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class DeferredValueNodeImpl : public ValueNodeImpl
        {
        public:
            explicit
            DeferredValueNodeImpl(ValueNode& rOriginal);
            explicit
            DeferredValueNodeImpl(DirectValueNodeImpl& rOriginal);

            ~DeferredValueNodeImpl();

            virtual bool    isDefault()         const;
            virtual bool    canGetDefaultValue() const;
            virtual UnoAny  getValue()          const;
            virtual UnoAny  getDefaultValue()   const;

            virtual UnoType getValueType()  const;

            virtual void    setValue(UnoAny const& aNewValue);
            virtual void    setDefault();

        protected:
        // legacy commit support
            virtual std::auto_ptr<ValueChange> doPreCommitChange();
            virtual void doFinishCommit(ValueChange& rChange);
            virtual void doRevertCommit(ValueChange& rChange);
            virtual void doFailedCommit(ValueChange& rChange);

            virtual NodeChangeImpl* doAdjustToChange(ValueChange const& rExternalChange);
        protected:
        // NodeImpl implementation
            virtual bool doHasChanges() const;
            virtual NodeChangeImpl* doCollectChange() const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        private:
            UnoAny* m_pNewValue;
            bool    m_bDefault;
        };
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

// Group Nodes
//-----------------------------------------------------------------------------

        class ReadOnlyGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            ReadOnlyGroupNodeImpl(ISubtree& rOriginal)
            : GroupNodeImpl(rOriginal)
            {}

        // Base obverrideables
        private:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual bool doHasChanges() const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredGroupNodeImpl;

        class DirectGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            DirectGroupNodeImpl(ISubtree& rOriginal);

            explicit
            DirectGroupNodeImpl(DeferredGroupNodeImpl& rOriginal);

        // Base obverrideables
        private:
        // NodeImpl implementation
            virtual bool doHasChanges() const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class DeferredGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            DeferredGroupNodeImpl(ISubtree& rOriginal);
            explicit
            DeferredGroupNodeImpl(DirectGroupNodeImpl& rOriginal);

            ~DeferredGroupNodeImpl();

        protected:
        // legacy commit support
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges();
            virtual void doFinishCommit(SubtreeChange& rChange);
            virtual void doRevertCommit(SubtreeChange& rChange);
            virtual void doFailedCommit(SubtreeChange& rChange);

        // Base obverrideables
        private:
        // NodeImpl implementation
            virtual bool doHasChanges() const;
            virtual void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);

            bool    m_bChanged;
        };
//-----------------------------------------------------------------------------

// Set nodes
//-----------------------------------------------------------------------------

        class ReadOnlyTreeSetNodeImpl : public TreeSetNodeImpl
        {
        public:
            explicit
            ReadOnlyTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
            : TreeSetNodeImpl(rOriginal,pTemplate)
            {}


        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual void doInsertElement(Name const& aName, SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class ReadOnlyValueSetNodeImpl : public ValueSetNodeImpl
        {
        public:
            explicit
            ReadOnlyValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
            : ValueSetNodeImpl(rOriginal,pTemplate)
            {}


        // Base Overrideables
        private:
            virtual void doInsertElement(Name const& aName,  SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredTreeSetNodeImpl;

        class DirectTreeSetNodeImpl : public TreeSetNodeImpl
        {
        public:
            explicit
            DirectTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);

            explicit
            DirectTreeSetNodeImpl(DeferredTreeSetNodeImpl& rOriginal);


        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual void doInsertElement(Name const& aName, SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredValueSetNodeImpl;

        class DirectValueSetNodeImpl : public ValueSetNodeImpl
        {
        public:
            explicit
            DirectValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);

            explicit
            DirectValueSetNodeImpl(DeferredValueSetNodeImpl& rOriginal);

        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual void doInsertElement(Name const& aName,  SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class DeferredTreeSetNodeImpl : public TreeSetNodeImpl
        {
        public:
            explicit
            DeferredTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);
            explicit
            DeferredTreeSetNodeImpl(DirectTreeSetNodeImpl& rOriginal);


        protected:
        // legacy commit support
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges();
            virtual void doFinishCommit(SubtreeChange& rChanges);
            virtual void doRevertCommit(SubtreeChange& rChanges);
            virtual void doFailedCommit(SubtreeChange& rChanges);

        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual bool        doIsEmpty() const;
            virtual SetEntry    doFindElement(Name const& aName) ;
            virtual void        doClearElements();
            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor);

            virtual void doInsertElement(Name const& aName, SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

            virtual void doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange, TemplateProvider const& aTemplateProvider);

            virtual NodeChangeImpl* doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement);
            virtual NodeChangeImpl* doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);

        // Implementation
        private:
            void implInsertNewElement(Name const& aName, Element const& aNewElement);
            void implRemoveOldElement(Name const& aName);

            ElementSet m_aChangedData;
            bool    m_bChanged;
        };
//-----------------------------------------------------------------------------

        class DeferredValueSetNodeImpl : public ValueSetNodeImpl
        {
        public:
            explicit
            DeferredValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);

            explicit
            DeferredValueSetNodeImpl(DirectValueSetNodeImpl& rOriginal);


        protected:
        // legacy commit support
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges();
            virtual void doFinishCommit(SubtreeChange& rChanges);
            virtual void doRevertCommit(SubtreeChange& rChanges);
            virtual void doFailedCommit(SubtreeChange& rChanges);

        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual bool        doIsEmpty() const;
            virtual SetEntry    doFindElement(Name const& aName) ;
            virtual void        doClearElements();
            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor);

            virtual void doInsertElement(Name const& aName,  SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

            virtual void doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange, TemplateProvider const& aTemplateProvider);

            virtual NodeChangeImpl* doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement);
            virtual NodeChangeImpl* doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);

        // Implementation
        private:
            void implInsertNewElement(Name const& aName, Element const& aNewElement);
            void implRemoveOldElement(Name const& aName);

            ElementSet m_aChangedData;
            bool    m_bChanged;
        };
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_NODEIMPLOBJECTS_HXX_

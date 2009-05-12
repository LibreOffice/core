/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: subtree.hxx,v $
 * $Revision: 1.10 $
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

#ifndef CONFIGMGR_SUBTREE_HXX
#define CONFIGMGR_SUBTREE_HXX

#include "valuenode.hxx"
#include <rtl/ustring.hxx>

#include <memory>
#include <set>
#include <vector>

namespace configmgr
{
    // List sorted by name for binary search
    class ChildListSet {
        std::vector< INode* > m_aChildList;

        ChildListSet(ChildListSet const&);
        ChildListSet& operator=(ChildListSet const& aSet);
    public:
        std::vector< INode* >::iterator begin() const { return const_cast<std::vector< INode* >*>(&m_aChildList)->begin(); }
        std::vector< INode* >::iterator end()   const { return const_cast<std::vector< INode* >*>(&m_aChildList)->end(); }
        INode *erase(INode *pNode);
        std::vector< INode* >::iterator find(INode *pNode) const;
        std::pair<std::vector< INode* >::iterator, bool> insert(INode *aInsert);

        ChildListSet() : m_aChildList(0) {}
        ChildListSet(ChildListSet const&, treeop::DeepChildCopy);
        ~ChildListSet();
    };

// Inner Node
    class Subtree : public ISubtree
    {
        ChildListSet   m_aChildren;
        virtual INode* doGetChild(rtl::OUString const& name) const;

    public:
        Subtree(){}
        Subtree(const rtl::OUString& _rName,
                const node::Attributes& _rAttrs)
                :ISubtree(_rName, _rAttrs){};

        Subtree(const ISubtree& _rOther, treeop::NoChildCopy)
            : ISubtree(_rOther), m_aChildren(){};

        Subtree(const rtl::OUString& _rName,
                const rtl::OUString& _rTemplateName, const rtl::OUString& _rTemplateModule,
                const node::Attributes& _rAttrs)
                :ISubtree(_rName, _rTemplateName, _rTemplateModule, _rAttrs){};

        Subtree(const Subtree& _rOther, treeop::DeepChildCopy _dc)
            : ISubtree(_rOther), m_aChildren(_rOther.m_aChildren,_dc){}

        virtual INode* addChild(std::auto_ptr<INode> node); // takes ownership
        virtual ::std::auto_ptr<INode> removeChild(rtl::OUString const& name);

        virtual std::auto_ptr<INode> clone() const;

// Iteration support
        virtual void forEachChild(NodeAction& anAction) const;
        virtual void forEachChild(NodeModification& anAction);
    };



    // to search in ChildListSet a value
    class SearchNode : public INode
    {
    public:
        SearchNode(rtl::OUString const& aName);
        virtual ~SearchNode();
        virtual std::auto_ptr<INode> clone() const;

// double dispatch support
            virtual void dispatch(NodeAction& /*anAction*/) const { }
            virtual void dispatch(NodeModification& /*anAction*/) { }
    };

// -----------------------------------------------------------------------------
} // namespace configmgr

#endif


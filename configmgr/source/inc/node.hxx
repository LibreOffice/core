/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: node.hxx,v $
 * $Revision: 1.9 $
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

#ifndef INCLUDED_SHARABLE_NODE_HXX
#define INCLUDED_SHARABLE_NODE_HXX

#include "rtl/ustring.hxx"

#include "flags.hxx"
#include "anydata.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace node { struct Attributes; }
//-----------------------------------------------------------------------------
    namespace sharable
    {
        struct  TreeFragment;
        union   Node;
    //-----------------------------------------------------------------------------
        struct NodeInfo
        {
            rtl_uString *            name;
            sal_uInt16          parent; // always counts backwards
            sal_uInt8    flags;
            sal_uInt8    type;   // contains discriminator for union

            rtl::OUString       getName() const;
            node::Attributes    getNodeInfoAttributes() const;
            bool isDefault() const;
            bool isLocalized() const;

            void markAsDefault(bool bDefault = true);
        };
    //-----------------------------------------------------------------------------
        struct GroupNode
        {
            NodeInfo info;
            sal_uInt16   numDescendants;   // = number of descendants

            bool hasDefaultsAvailable()   const;

            Node * getFirstChild()  const;
            Node * getNextChild(Node * child) const;
            Node * getChild(rtl::OUString const & name) const;

            static inline GroupNode * from(Node * node);
        };
    //-----------------------------------------------------------------------------
        struct SetNode
        {

            NodeInfo info;
            sal_uInt8 *  elementType; // points to template [MM:SetNode *?]
            TreeFragment *     elements;    // points to first element (TreeFragmentHeader)

            rtl::OUString getElementTemplateName() const;
            rtl::OUString getElementTemplateModule() const;

            TreeFragment * getFirstElement() const;
            TreeFragment * getNextElement(TreeFragment * _pElement) const;
            TreeFragment * getElement(rtl::OUString const & name) const;

            void addElement(TreeFragment * newElement);
            TreeFragment * removeElement(rtl::OUString const & name);

            static inline SetNode * from(Node * node);

            // low-level helper for template data abstraction
            static
            sal_uInt8 * allocTemplateData(const rtl::OUString &rName,
                                      const rtl::OUString &rModule);
            static
            sal_uInt8 * copyTemplateData(sal_uInt8 * _aTemplateData);
            static
            void releaseTemplateData(sal_uInt8 * _aTemplateData);
        };
    //-----------------------------------------------------------------------------
        struct ValueNode
        {
            NodeInfo info;
            AnyData  value;
            AnyData  defaultValue;

            bool isNull()       const;
            bool hasUsableDefault()   const;

            com::sun::star::uno::Any    getValue() const;
            com::sun::star::uno::Type   getValueType() const;
            com::sun::star::uno::Any    getUserValue() const;
            com::sun::star::uno::Any    getDefaultValue() const;

            void setValue(com::sun::star::uno::Any const & newValue);
            void setToDefault();
            void changeDefault(com::sun::star::uno::Any const & newDefault);

            static inline ValueNode * from(Node * node);

        private:
            void releaseValue();
            sal_uInt8 adaptType(com::sun::star::uno::Any const & newValue);
        };
    //-----------------------------------------------------------------------------
        // TODO: optimized representation of localized values (now as set; mapping locale->element-name)
        // TODO (?): better representation of sets of values
    //-----------------------------------------------------------------------------
        union Node
        {
            NodeInfo info;
            GroupNode group;
            SetNode   set;
            ValueNode value;

            // info access
            bool isNamed(rtl::OUString const & _aName) const;
            rtl::OUString       getName() const;
            node::Attributes    getAttributes() const;
            bool isDefault() const;

            // type checks
            bool isGroup()  const  { return typeIs (data::Type::nodetype_group); }
            bool isSet()    const  { return typeIs (data::Type::nodetype_set); }
            bool isValue()  const  { return typeIs (data::Type::nodetype_value); }

            // checked access
            inline GroupNode       * groupData();
            inline GroupNode const * groupData() const;
            inline SetNode         * setData();
            inline SetNode   const * setData()   const;
            inline ValueNode       * valueData();
            inline ValueNode const * valueData() const;

            // navigation
            bool isFragmentRoot() const;
#if OSL_DEBUG_LEVEL > 0
            Node        * getParentNode();
            Node const  * getParentNode() const;
#endif
            TreeFragment        * getTreeFragment();
            TreeFragment const  * getTreeFragment() const;

            Node * getSubnode(rtl::OUString const & name);

        private:
            bool typeIs(data::Type::Type eType) const
                { return (info.type & data::Type::mask_nodetype) == eType; }
        };

    //-----------------------------------------------------------------------------
        inline GroupNode * GroupNode::from(Node * node)
        { return node == 0 ? 0 : node->groupData(); }

        inline SetNode * SetNode::from(Node * node)
        { return node == 0 ? 0 : node->setData(); }

        inline ValueNode * ValueNode::from(Node * node)
        { return node == 0 ? 0 : node->valueData(); }

        inline GroupNode       * Node::groupData()
            { return isGroup() ? &this->group : NULL; }
        inline GroupNode const * Node::groupData() const
            { return isGroup() ? &this->group : NULL; }
        inline SetNode       * Node::setData()
            { return isSet() ? &this->set : NULL; }
        inline SetNode const * Node::setData()   const
            { return isSet() ? &this->set : NULL; }
        inline ValueNode       * Node::valueData()
            { return isValue() ? &this->value : NULL; }
        inline ValueNode const * Node::valueData() const
            { return isValue() ? &this->value : NULL; }

        inline Node * node(ValueNode * pNode)
        { return reinterpret_cast<Node*>(pNode); }
        inline Node * node(GroupNode * pNode)
        { return reinterpret_cast<Node*>(pNode); }
        inline Node * node(SetNode * pNode)
        { return reinterpret_cast<Node*>(pNode); }

        inline Node const * node(ValueNode const* pNode)
        { return reinterpret_cast<Node const*>(pNode); }
        inline Node const * node(GroupNode const* pNode)
        { return reinterpret_cast<Node const*>(pNode); }
        inline Node const * node(SetNode const* pNode)
        { return reinterpret_cast<Node const*>(pNode); }
    //-----------------------------------------------------------------------------
        inline Node & node(ValueNode & pNode)
        { return reinterpret_cast<Node&>(pNode); }
        inline Node & node(GroupNode & pNode)
        { return reinterpret_cast<Node&>(pNode); }
        inline Node & node(SetNode & pNode)
        { return reinterpret_cast<Node&>(pNode); }

        inline Node const & node(ValueNode const& pNode)
        { return reinterpret_cast<Node const&>(pNode); }
        inline Node const & node(GroupNode const& pNode)
        { return reinterpret_cast<Node const&>(pNode); }
        inline Node const & node(SetNode const& pNode)
        { return reinterpret_cast<Node const&>(pNode); }
    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // INCLUDED_SHARABLE_NODE_HXX

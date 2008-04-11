/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: node.hxx,v $
 * $Revision: 1.8 $
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

#include "types.hxx"
#include "flags.hxx"
#include "anydata.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace uno = ::com::sun::star::uno;
//-----------------------------------------------------------------------------
    namespace node { struct Attributes; }
//-----------------------------------------------------------------------------
    namespace sharable
    {
    //-----------------------------------------------------------------------------

        namespace Type  = data::Type;
        namespace Flags = data::Flags;

    //-----------------------------------------------------------------------------
        struct  TreeFragment;
        union   Node;
    //-----------------------------------------------------------------------------
        struct NodeInfo
        {
            Name            name;
            Offset          parent; // always counts backwards
            Flags::Field    flags;
            Type ::Field    type;   // contains discriminator for union

            rtl::OUString       getName() const;
            node::Attributes    getNodeInfoAttributes() const;
            bool isDefault() const;
            bool isLocalized() const;

            void markAsDefault(bool bDefault = true);
        };
    //-----------------------------------------------------------------------------
        struct BasicNode    // common initial sequence of union Node members
        {
            NodeInfo info;
        };
    //-----------------------------------------------------------------------------
        struct GroupNode
        {
            NodeInfo info;
            Offset   numDescendants;   // = number of descendants

            bool hasDefaultsAvailable()   const;

            Node * getFirstChild();
            Node * getNextChild(Node * _pChild);
            Node const * getFirstChild()  const;
            Node const * getNextChild(Node const * _pChild) const;

        };
    //-----------------------------------------------------------------------------
        typedef sal_uInt8 * SetElementAddress;
        struct SetNode
        {

            NodeInfo info;
            SetElementAddress  elementType; // points to template [MM:SetNode *?]
            List     elements;    // points to first element (TreeFragmentHeader)

            bool isLocalizedValue() const;

            rtl::OUString getElementTemplateName() const;
            rtl::OUString getElementTemplateModule() const;

            TreeFragment const  * getFirstElement() const;
            TreeFragment const  * getNextElement(TreeFragment const * _pElement) const;

            // low-level helper for template data abstraction
            static
            SetElementAddress allocTemplateData(const rtl::OUString &rName,
                                      const rtl::OUString &rModule);
            static
            SetElementAddress copyTemplateData(SetElementAddress _aTemplateData);
            static
            void releaseTemplateData(SetElementAddress _aTemplateData);
        };
    //-----------------------------------------------------------------------------
        struct ValueNode
        {
            NodeInfo info;
            AnyData  value;
            AnyData  defaultValue;

            bool isEmpty()      const;
            bool isNull()       const;
            bool hasUsableDefault()   const;

            uno::Any    getValue() const;
            uno::Type   getValueType() const;
            uno::Any    getUserValue() const;
            uno::Any    getDefaultValue() const;
        };
    //-----------------------------------------------------------------------------
        // TODO: optimized representation of localized values (now as set; mapping locale->element-name)
        // TODO (?): better representation of sets of values
    //-----------------------------------------------------------------------------
        union Node
        {
            BasicNode node;
            GroupNode group;
            SetNode   set;
            ValueNode value;

            // info access
            bool isNamed(rtl::OUString const & _aName) const;
            rtl::OUString       getName() const;
            node::Attributes    getAttributes() const;
            bool isDefault() const;
            bool isLocalized() const;

            // type checks
            bool isGroup()  const  { return typeIs (Type::nodetype_group); }
            bool isSet()    const  { return typeIs (Type::nodetype_set); }
            bool isValue()  const  { return typeIs (Type::nodetype_value); }

            // checked access
            inline GroupNode       * groupData();
            inline GroupNode const * groupData() const;
            inline SetNode         * setData();
            inline SetNode   const * setData()   const;
            inline ValueNode       * valueData();
            inline ValueNode const * valueData() const;

            // navigation
            bool isFragmentRoot() const;

            Node        * getParentNode();
            Node const  * getParentNode() const;

            TreeFragment        * getTreeFragment();
            TreeFragment const  * getTreeFragment() const;
            private:
            bool typeIs(Type::Type eType) const
                { return (node.info.type & Type::mask_nodetype) == eType; }
        };

    //-----------------------------------------------------------------------------
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

    namespace data {
        typedef sharable::Node      * NodeAddress;
        typedef sharable::ValueNode * ValueNodeAddress;
        typedef sharable::GroupNode * GroupNodeAddress;
        typedef sharable::SetNode   * SetNodeAddress;
    }
//-----------------------------------------------------------------------------
}

#endif // INCLUDED_SHARABLE_NODE_HXX

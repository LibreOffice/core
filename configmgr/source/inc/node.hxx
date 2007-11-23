/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: node.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:20:38 $
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

#ifndef INCLUDED_SHARABLE_NODE_HXX
#define INCLUDED_SHARABLE_NODE_HXX

#ifndef INCLUDED_SHARABLE_BASETYPES_HXX
#include "types.hxx"
#endif
#ifndef INCLUDED_DATA_FLAGS_HXX
#include "flags.hxx"
#endif
#ifndef INCLUDED_SHARABLE_ANYDATA_HXX
#include "anydata.hxx"
#endif

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

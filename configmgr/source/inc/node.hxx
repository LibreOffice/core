/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: node.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:48:08 $
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
        struct SetNode
        {
            NodeInfo info;
            Address  elementType; // points to template
            List     elements;    // points to first element (TreeFragmentHeader)

            bool isLocalizedValue() const;

            rtl::OUString getElementTemplateName  (memory::Accessor const & _anAccessor) const;
            rtl::OUString getElementTemplateModule(memory::Accessor const & _anAccessor) const;

            TreeFragment const  * getFirstElement(memory::Accessor const & _anAccessor) const;
            TreeFragment const  * getNextElement(memory::Accessor const & _anAccessor, TreeFragment const * _pElement) const;

            // low-level helper for template data abstraction
            static
            Address allocTemplateData(memory::Allocator const & _anAllocator,
                                      const rtl::OUString &rName,
                                      const rtl::OUString &rModule);
            static
            Address copyTemplateData(memory::Allocator const & _anAllocator,
                                     Address _aTemplateData);
            static
            void releaseTemplateData(memory::Allocator const & _anAllocator, Address _aTemplateData);
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

            uno::Type   getValueType()  const;
            uno::Any    getValue(memory::Accessor const & _aAccessor)      const;
            uno::Any    getUserValue(memory::Accessor const & _aAccessor)    const;
            uno::Any    getDefaultValue(memory::Accessor const & _aAccessor) const;
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
            bool isGroup()  const;
            bool isSet()    const;
            bool isValue()  const;

            // checked access
            GroupNode       * groupData();
            SetNode         * setData();
            ValueNode       * valueData();
            GroupNode const * groupData() const;
            SetNode   const * setData()   const;
            ValueNode const * valueData() const;

            // navigation
            bool isFragmentRoot() const;

            Node        * getParentNode();
            Node const  * getParentNode() const;

            TreeFragment        * getTreeFragment();
            TreeFragment const  * getTreeFragment() const;
        };
    //-----------------------------------------------------------------------------
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

/*************************************************************************
 *
 *  $RCSfile: node.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:33:13 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

            rtl::OUString       getName(memory::Accessor const & _aAccessor) const;
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
            Address allocTemplateData(memory::Allocator const & _anAllocator, NameChar const * pName, NameChar const * pModule);
            static
            void releaseTemplateData(memory::Allocator const & _anAllocator, Address _aTemplateData);
            static
            NameChar const * getTemplateDataName(memory::Accessor const & _anAccessor, Address _aTemplateData);
            static
            NameChar const * getTemplateDataModule(memory::Accessor const & _anAccessor, Address _aTemplateData);
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
            bool isNamed(rtl::OUString const & _aName, memory::Accessor const & _aAccessor) const;
            rtl::OUString       getName(memory::Accessor const & _aAccessor) const;
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

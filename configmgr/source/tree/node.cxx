/*************************************************************************
 *
 *  $RCSfile: node.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2003-04-02 07:28:19 $
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

#include "node.hxx"

#ifndef INCLUDED_SHARABLE_ANYDATA_HXX
#include "anydata.hxx"
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif
#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
//-----------------------------------------------------------------------------

//        Name            name;
  //          Offset          parent; // always counts backwards
    //        Flags::Field    flags;
      //      Type ::Field    type;   // contains discriminator for union

rtl::OUString NodeInfo::getName(memory::Accessor const & _aAccessor) const
{
    return readString(_aAccessor,this->name);
}
//-----------------------------------------------------------------------------

configmgr::node::Attributes NodeInfo::getNodeInfoAttributes() const
{
    configmgr::node::Attributes aResult;

    bool help = !!(flags & Flags::readonly);
    aResult.setAccess( help,!!(flags & Flags::finalized) );
//    aResult.setAccess( !!(flags & Flags::readonly),!!(flags & Flags::finalized) );

    aResult.setNullable(!!(flags & Flags::nullable));
    aResult.setLocalized(!!(flags & Flags::localized));

    configmgr::node::State state = (flags & Flags::defaulted)   ? configmgr::node::isDefault :
                        (flags & Flags::defaultable) ? configmgr::node::isMerged  :
                                                       configmgr::node::isReplaced;
    aResult.setState(state);

    return aResult;
}
//-----------------------------------------------------------------------------

bool NodeInfo::isDefault() const
{
    return !!(this->flags & Flags::defaulted);
}
//-----------------------------------------------------------------------------

bool NodeInfo::isLocalized() const
{
    return !!(this->flags & Flags::localized);
}
//-----------------------------------------------------------------------------

void NodeInfo::markAsDefault(bool bDefault)
{
    if (bDefault)
    {
        OSL_ENSURE(flags & Flags::defaultable,"Marking a non-defaultable node as default");
        this->flags |= Flags::defaulted;
    }
    else
        this->flags &= ~Flags::defaulted;
}
//-----------------------------------------------------------------------------

bool GroupNode::hasDefaultsAvailable() const
{
    if (this->info.isDefault())
        return true;

    if (node(this)->getTreeFragment()->hasDefaultsAvailable())
        return true;

#if 0 // extended check for default state
    for (Node const * pChild = getFirstChild(); pChild != NULL; pChild = getNextChild(pChild))
        if (! pChild->isDefault() )
            return false;

    return true;
#endif

    return false;
}
//-----------------------------------------------------------------------------

Node * GroupNode::getFirstChild()
{
    OSL_ENSURE(numDescendants, "Groups MUST have at least one child");
    return node(this) + 1;
}
//-----------------------------------------------------------------------------

Node const * GroupNode::getFirstChild()  const
{
    OSL_ENSURE(numDescendants, "Groups MUST have at least one child");
    return node(this) + 1;
}
//-----------------------------------------------------------------------------

static
Offset implGetNextChildOffset(GroupNode const * _pParent, Node const * _pChild)
{
    OSL_PRECOND(_pChild, "getNextChild: previous child must not be NULL");
    OSL_PRECOND(_pChild->getParentNode() == node(_pParent), "getNextChild: not a child of this node");


    OSL_ENSURE( node(_pParent) < _pChild && _pChild <= node(_pParent) + _pParent->numDescendants,
                "getNextChild: child out of descendants range");

    // offset to child's next sibling
    Offset next = 1;
    if ( _pChild->isGroup())
    {
        next += _pChild->group.numDescendants;
    }

    if (_pChild->node.info.parent + next > _pParent->numDescendants)
    {
        OSL_ENSURE(_pChild->node.info.parent + next == _pParent->numDescendants+1, "Next child candidate should match next sibling here");
        return 0;
    }

    OSL_POSTCOND( (_pChild+next)->getParentNode() == node(_pParent), "getNextChild: not a child of this node");
    return next;
}
//-----------------------------------------------------------------------------

Node * GroupNode::getNextChild(Node * _pChild)
{
    if (Offset next = implGetNextChildOffset(this, _pChild))
        return _pChild + next;

    else
        return NULL;
}
//-----------------------------------------------------------------------------

Node const * GroupNode::getNextChild(Node const * _pChild) const
{
    if (Offset next = implGetNextChildOffset(this, _pChild))
        return _pChild + next;

    else
        return NULL;
}
//-----------------------------------------------------------------------------

bool SetNode::isLocalizedValue() const
{
    return info.isLocalized();
}
//-----------------------------------------------------------------------------

// TODO: optimize this - keep a list of such structs ....
struct SetNodeTemplateData
{
    Name name;
    Name module;
};
//-----------------------------------------------------------------------------
static inline
SetNodeTemplateData * readTemplateData(memory::Allocator const & _anAllocator, Address _aTemplateData)
{
    return static_cast<SetNodeTemplateData *>( _anAllocator.access(_aTemplateData) );
}
//-----------------------------------------------------------------------------
static inline
SetNodeTemplateData const * readTemplateData(memory::Accessor const & _anAccessor, Address _aTemplateData)
{
    return static_cast<SetNodeTemplateData const*>( _anAccessor.access(memory::Pointer(_aTemplateData)) );
}
//-----------------------------------------------------------------------------

Address SetNode::allocTemplateData(memory::Allocator const & _anAllocator, NameChar const * pName, NameChar const * pModule)
{
    rtl::OUString aName(pName), aModule(pModule);

    Address aData = _anAllocator.allocate(sizeof(SetNodeTemplateData));

    if (aData)
    {
        SetNodeTemplateData * pData = readTemplateData(_anAllocator,aData);

        OSL_ENSURE(pData, "Creating template data: unexpected NULL data");

        pData->name     = allocName(_anAllocator,aName);
        pData->module   = allocName(_anAllocator,aModule);
    }
    return aData;
}
//-----------------------------------------------------------------------------

void SetNode::releaseTemplateData(memory::Allocator const & _anAllocator, Address _aTemplateData)
{
    if (!_aTemplateData) return;

    SetNodeTemplateData const * pData = readTemplateData(_anAllocator,_aTemplateData);

    OSL_ENSURE(pData, "Freeing template data: unexpected NULL data");

    freeName(_anAllocator,pData->name);
    freeName(_anAllocator,pData->module);

    _anAllocator.deallocate(_aTemplateData);
}
//-----------------------------------------------------------------------------

NameChar const * SetNode::getTemplateDataName(memory::Accessor const & _anAccessor, Address _aTemplateData)
{
    OSL_PRECOND(_aTemplateData, "Reading template data: unexpected NULL pointer");

    SetNodeTemplateData const * pData = readTemplateData(_anAccessor,_aTemplateData);

    OSL_ENSURE(pData, "Reading template data: unexpected NULL data");

    return accessName(_anAccessor,pData->name);
}
//-----------------------------------------------------------------------------

NameChar const * SetNode::getTemplateDataModule(memory::Accessor const & _anAccessor, Address _aTemplateData)
{
    OSL_PRECOND(_aTemplateData, "Reading template data: unexpected NULL pointer");

    SetNodeTemplateData const * pData = readTemplateData(_anAccessor,_aTemplateData);

    OSL_ENSURE(pData, "Reading template data: unexpected NULL data");

    return accessName(_anAccessor,pData->module);
}
//-----------------------------------------------------------------------------

rtl::OUString SetNode::getElementTemplateName(memory::Accessor const & _anAccessor)   const
{
    SetNodeTemplateData const * pData = readTemplateData(_anAccessor,this->elementType);

    OSL_ENSURE(pData, "ERROR: No template data found for set");

    return readName(_anAccessor,pData->name);
}
//-----------------------------------------------------------------------------

rtl::OUString SetNode::getElementTemplateModule(memory::Accessor const & _anAccessor) const
{
    SetNodeTemplateData const * pData = readTemplateData(_anAccessor,this->elementType);

    OSL_ENSURE(pData, "ERROR: No template data found for set");

    return readName(_anAccessor,pData->module);
}
//-----------------------------------------------------------------------------

static inline
TreeFragment const * implGetFragmentFromList(memory::Accessor const & _anAccessor, List _aListEntry)
{
    return static_cast<TreeFragment const *>(_anAccessor.access(memory::Pointer(_aListEntry)));
}
//-----------------------------------------------------------------------------

TreeFragment const  * SetNode::getFirstElement(memory::Accessor const & _anAccessor) const
{
    return implGetFragmentFromList(_anAccessor, this->elements);
}
//-----------------------------------------------------------------------------

TreeFragment const  * SetNode::getNextElement(memory::Accessor const & _anAccessor, TreeFragment const * _pElement) const
{
    OSL_PRECOND(_pElement, "getNextElement: previous element must not be NULL");
    OSL_PRECOND(_pElement->header.parent == _anAccessor.address(this).value(),
                "getNextElement: not an element of this node");

    return implGetFragmentFromList(_anAccessor, _pElement->header.next);
}
//-----------------------------------------------------------------------------

bool ValueNode::isEmpty() const
{
    Type::Field const empty_value_type = Type::value_any | Type::nodetype_value;

    return info.type == empty_value_type;
}
//-----------------------------------------------------------------------------

bool ValueNode::isNull() const
{
    Flags::Type availmask = (info.flags & Flags::defaulted) ?
                                Flags::defaultAvailable :
                                Flags::valueAvailable;

    return !(info.flags & availmask);
}
//-----------------------------------------------------------------------------

bool ValueNode::hasUsableDefault() const
{
    return (info.flags & Flags::defaultable) &&
           (info.flags & (Flags::defaultAvailable| Flags::nullable));
}
//-----------------------------------------------------------------------------

uno::Type   ValueNode::getValueType()  const
{
    AnyData::TypeCode aType = info.type & Type::mask_valuetype;

    return getUnoType(aType);
}
//-----------------------------------------------------------------------------

uno::Any    ValueNode::getValue(memory::Accessor const & _aAccessor)      const
{
    if (info.flags & Flags::defaulted)
        return getDefaultValue(_aAccessor);

    else
        return getUserValue(_aAccessor);
}
//-----------------------------------------------------------------------------

uno::Any    ValueNode::getUserValue(memory::Accessor const & _aAccessor)      const
{
    if (info.flags & Flags::valueAvailable)
    {
        AnyData::TypeCode aType = info.type & Type::mask_valuetype;

        return readData(_aAccessor,aType,this->value);
    }
    else
        return uno::Any();
}
//-----------------------------------------------------------------------------

uno::Any    ValueNode::getDefaultValue(memory::Accessor const & _aAccessor)    const
{
    if (info.flags & Flags::defaultAvailable)
    {
        AnyData::TypeCode aType = info.type & Type::mask_valuetype;

        return readData(_aAccessor,aType,this->defaultValue);
    }
    else
        return uno::Any();
}
//-----------------------------------------------------------------------------

bool Node::isNamed(rtl::OUString const & _aName, memory::Accessor const & _aAccessor) const
{
    return 0 == rtl_ustr_compare(_aName.getStr(),accessString(_aAccessor,node.info.name));
}
//-----------------------------------------------------------------------------

rtl::OUString Node::getName(memory::Accessor const & _aAccessor) const
{
    return node.info.getName(_aAccessor);
}
//-----------------------------------------------------------------------------

configmgr::node::Attributes Node::getAttributes() const
{
    if(this->isFragmentRoot())
    {
        return this->getTreeFragment()->getAttributes();
    }
    else
    {
        return node.info.getNodeInfoAttributes();
    }
}
//-----------------------------------------------------------------------------

bool Node::isDefault() const
{
    return node.info.isDefault();
}
//-----------------------------------------------------------------------------

bool Node::isLocalized() const
{
    return node.info.isLocalized();
}
//-----------------------------------------------------------------------------

bool Node::isGroup()  const
{
    return (node.info.type & Type::mask_nodetype) == Type::nodetype_group;
}
//-----------------------------------------------------------------------------

bool Node::isSet()    const
{
    return (node.info.type & Type::mask_nodetype) == Type::nodetype_set;
}
//-----------------------------------------------------------------------------

bool Node::isValue()  const
{
    return (node.info.type & Type::mask_nodetype) == Type::nodetype_value;
}
//-----------------------------------------------------------------------------

GroupNode       * Node::groupData()
{
    return isGroup() ? &this->group : NULL;
}
//-----------------------------------------------------------------------------

GroupNode const * Node::groupData() const
{
    return isGroup() ? &this->group : NULL;
}
//-----------------------------------------------------------------------------

SetNode         * Node::setData()
{
    return isSet() ? &this->set : NULL;
}
//-----------------------------------------------------------------------------
SetNode   const * Node::setData()   const
{
    return isSet() ? &this->set : NULL;
}
//-----------------------------------------------------------------------------
ValueNode       * Node::valueData()
{
    return isValue() ? &this->value : NULL;
}
//-----------------------------------------------------------------------------
ValueNode const * Node::valueData() const
{
    return isValue() ? &this->value : NULL;
}
//-----------------------------------------------------------------------------

bool Node::isFragmentRoot() const
{
    return ! node.info.parent;
}
//-----------------------------------------------------------------------------

Node * Node::getParentNode()
{
    return node.info.parent ? this - node.info.parent : NULL;
}
//-----------------------------------------------------------------------------

Node const * Node::getParentNode() const
{
    return node.info.parent ? this - node.info.parent : NULL;
}
//-----------------------------------------------------------------------------

static Offset getFragmentIndex(Node const * pNode)
{
    Offset result = 0;
    while (Offset step = pNode->node.info.parent)
    {
        result += step;
        pNode  -= step;
    }
    return result;
}
//-----------------------------------------------------------------------------

TreeFragment * Node::getTreeFragment()
{
    void * pRoot = this - getFragmentIndex(this);

    void * pFrag = static_cast<char*>(pRoot) - offsetof(TreeFragment,nodes);

    return static_cast<TreeFragment *>(pFrag);
}
//-----------------------------------------------------------------------------

TreeFragment const * Node::getTreeFragment() const
{
    void const * pRoot = this - getFragmentIndex(this);

    void const * pFrag = static_cast<char const*>(pRoot) - offsetof(TreeFragment,nodes);

    return static_cast<TreeFragment const *>(pFrag);
}

//-----------------------------------------------------------------------------
    } // namespace sharable
//-----------------------------------------------------------------------------
} // namespace configmgr



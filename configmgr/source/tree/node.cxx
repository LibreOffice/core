/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: node.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "node.hxx"
#include "anydata.hxx"
#include "treefragment.hxx"
#include "attributes.hxx"
#include "utility.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>

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

rtl::OUString NodeInfo::getName() const
{
    return rtl::OUString(this->name);
}
//-----------------------------------------------------------------------------

configmgr::node::Attributes NodeInfo::getNodeInfoAttributes() const
{
    configmgr::node::Attributes aResult;

    bool help = !!(flags & data::Flags::readonly);
    aResult.setAccess( help,!!(flags & data::Flags::finalized) );
//    aResult.setAccess( !!(flags & data::Flags::readonly),!!(flags & data::Flags::finalized) );

    aResult.setNullable(!!(flags & data::Flags::nullable));
    aResult.setLocalized(!!(flags & data::Flags::localized));

    configmgr::node::State state = (flags & data::Flags::defaulted)   ? configmgr::node::isDefault :
                        (flags & data::Flags::defaultable) ? configmgr::node::isMerged  :
                                                       configmgr::node::isReplaced;
    aResult.setState(state);

    return aResult;
}
//-----------------------------------------------------------------------------

bool NodeInfo::isDefault() const
{
    return !!(this->flags & data::Flags::defaulted);
}
//-----------------------------------------------------------------------------

bool NodeInfo::isLocalized() const
{
    return !!(this->flags & data::Flags::localized);
}
//-----------------------------------------------------------------------------

void NodeInfo::markAsDefault(bool bDefault)
{
    if (bDefault)
    {
        OSL_ENSURE(flags & data::Flags::defaultable,"Marking a non-defaultable node as default");
        this->flags |= data::Flags::defaulted;
    }
    else
        this->flags &= ~data::Flags::defaulted;
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

Node * GroupNode::getFirstChild()  const
{
    OSL_ENSURE(numDescendants, "Groups MUST have at least one child");
    return const_cast< Node * >(node(this) + 1);
}
//-----------------------------------------------------------------------------

static
sal_uInt16 implGetNextChildOffset(GroupNode const * _pParent, Node const * _pChild)
{
    OSL_PRECOND(_pChild, "getNextChild: previous child must not be NULL");
    OSL_PRECOND(_pChild->getParentNode() == node(_pParent), "getNextChild: not a child of this node");


    OSL_ENSURE( node(_pParent) < _pChild && _pChild <= node(_pParent) + _pParent->numDescendants,
                "getNextChild: child out of descendants range");

    // offset to child's next sibling
    sal_uInt16 next = 1;
    if ( _pChild->isGroup())
    {
        next = next + _pChild->group.numDescendants;
    }

    if (_pChild->info.parent + next > _pParent->numDescendants)
    {
        OSL_ENSURE(_pChild->info.parent + next == _pParent->numDescendants+1, "Next child candidate should match next sibling here");
        return 0;
    }

    OSL_POSTCOND( (_pChild+next)->getParentNode() == node(_pParent), "getNextChild: not a child of this node");
    return next;
}
//-----------------------------------------------------------------------------

Node * GroupNode::getNextChild(Node * _pChild) const
{
    if (sal_uInt16 next = implGetNextChildOffset(this, _pChild))
        return _pChild + next;

    else
        return NULL;
}

Node * GroupNode::getChild(rtl::OUString const & name) const {
    for (Node * child = getFirstChild(); child != 0;
         child = getNextChild(child))
    {
        if (child->isNamed(name)) {
            return child;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------

// TODO: optimize this - keep a list of such structs ....
struct SetNodeTemplateData
{
    rtl_uString * name;
    rtl_uString * module;
};
//-----------------------------------------------------------------------------
static inline
SetNodeTemplateData * readTemplateData(sal_uInt8 * _aTemplateData)
{
    return reinterpret_cast<SetNodeTemplateData *>( _aTemplateData );
}
//-----------------------------------------------------------------------------

sal_uInt8 * SetNode::allocTemplateData(const rtl::OUString &rName,
                                   const rtl::OUString &rModule)
{
    SetNodeTemplateData * pData = new SetNodeTemplateData();

    OSL_ENSURE(pData, "Creating template data: unexpected NULL data");

    pData->name   = acquireString(rName);
    pData->module = acquireString(rModule);

    return reinterpret_cast<sal_uInt8 *>( pData );
}

sal_uInt8 * SetNode::copyTemplateData(sal_uInt8 * _aTemplateData)
{
    SetNodeTemplateData const * pData = readTemplateData(_aTemplateData);

    OSL_ENSURE(pData, "Copying template data: unexpected NULL data");

    return allocTemplateData(rtl::OUString(pData->name), rtl::OUString(pData->module));
}

//-----------------------------------------------------------------------------

void SetNode::releaseTemplateData(sal_uInt8 * _aTemplateData)
{
    if (!_aTemplateData) return;

    SetNodeTemplateData const * pData = readTemplateData(_aTemplateData);

    OSL_ENSURE(pData, "Freeing template data: unexpected NULL data");

    rtl_uString_release(pData->name);
    rtl_uString_release(pData->module);

    delete pData;
}

//-----------------------------------------------------------------------------

rtl::OUString SetNode::getElementTemplateName() const
{
    SetNodeTemplateData const * pData = readTemplateData(this->elementType);

    OSL_ENSURE(pData, "ERROR: No template data found for set");

    return rtl::OUString(pData->name);
}
//-----------------------------------------------------------------------------

rtl::OUString SetNode::getElementTemplateModule() const
{
    SetNodeTemplateData const * pData = readTemplateData(this->elementType);

    OSL_ENSURE(pData, "ERROR: No template data found for set");

    return rtl::OUString(pData->module);
}
//-----------------------------------------------------------------------------

static inline
TreeFragment * implGetFragmentFromList(TreeFragment * _aListEntry)
{
    return reinterpret_cast<TreeFragment *>(_aListEntry);
}
//-----------------------------------------------------------------------------

TreeFragment * SetNode::getFirstElement() const
{
    return implGetFragmentFromList(this->elements);
}
//-----------------------------------------------------------------------------

TreeFragment * SetNode::getNextElement(TreeFragment * _pElement) const
{
    OSL_PRECOND(_pElement, "getNextElement: previous element must not be NULL");
    OSL_PRECOND(_pElement->header.parent == (Node *)this,
                "getNextElement: not an element of this node");

    return implGetFragmentFromList(_pElement->header.next);
}

TreeFragment * SetNode::getElement(rtl::OUString const & name) const {
    for (TreeFragment * element = getFirstElement(); element != 0;
         element = getNextElement(element))
    {
        if (element->isNamed(name)) {
            return element;
        }
    }
    return 0;
}

void SetNode::addElement(TreeFragment * newElement) {
    OSL_ASSERT(newElement != 0);
    newElement->header.next = elements;
    newElement->header.parent = node(this);
    elements = newElement;
}

TreeFragment * SetNode::removeElement(rtl::OUString const & name) {
    for (TreeFragment ** link = &elements; *link != 0;
         link = &(*link)->header.next)
    {
        if ((*link)->isNamed(name)) {
            TreeFragment * removed = *link;
            *link = removed->header.next;
            removed->header.next = 0;
            removed->header.parent = 0;
            return removed;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------

bool ValueNode::isNull() const
{
    data::Flags::Type availmask = (info.flags & data::Flags::defaulted) ?
                                data::Flags::defaultAvailable :
                                data::Flags::valueAvailable;

    return !(info.flags & availmask);
}
//-----------------------------------------------------------------------------

bool ValueNode::hasUsableDefault() const
{
    return (info.flags & data::Flags::defaultable) &&
           (info.flags & (data::Flags::defaultAvailable| data::Flags::nullable));
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Type ValueNode::getValueType() const
{
    sal_uInt8 aType = sal_uInt8( info.type & data::Type::mask_valuetype );

    return getUnoType(aType);
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any ValueNode::getValue() const
{
    if (info.flags & data::Flags::defaulted)
        return getDefaultValue();

    else
        return getUserValue();
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any ValueNode::getUserValue() const
{
    if (info.flags & data::Flags::valueAvailable)
    {
        sal_uInt8 aType = sal_uInt8( info.type & data::Type::mask_valuetype );

        return readData(aType,this->value);
    }
    else
        return com::sun::star::uno::Any();
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any ValueNode::getDefaultValue() const
{
    if (info.flags & data::Flags::defaultAvailable)
    {
        sal_uInt8 aType = sal_uInt8( info.type & data::Type::mask_valuetype );

        return readData(aType,this->defaultValue);
    }
    else
        return com::sun::star::uno::Any();
}

void ValueNode::setValue(com::sun::star::uno::Any const & newValue) {
    releaseValue();
    if (newValue.hasValue()) {
        sal_uInt8 type = adaptType(newValue);
        value = allocData(type, newValue);
        info.flags |= data::Flags::valueAvailable;
    }
    info.flags &= ~data::Flags::defaulted;
}

void ValueNode::setToDefault() {
    OSL_ASSERT(hasUsableDefault());
    releaseValue();
    info.flags |= data::Flags::defaulted;
}

void ValueNode::changeDefault(com::sun::star::uno::Any const & newDefault) {
    sal_uInt8 type = static_cast< sal_uInt8 >(
        info.type & data::Type::mask_valuetype);
    if (info.flags & data::Flags::defaultAvailable) {
        OSL_ASSERT(type != data::Type::value_any);
        freeData(type, defaultValue);
        defaultValue.data = 0;
        info.flags &= ~data::Flags::defaultAvailable;
    }
    if (newDefault.hasValue()) {
        type = adaptType(newDefault);
        defaultValue = allocData(type, newDefault);
        info.flags |= data::Flags::defaultAvailable;
    }
}

void ValueNode::releaseValue() {
    if ((info.flags & data::Flags::valueAvailable) != 0) {
        sal_uInt8 type = static_cast< sal_uInt8 >(
            info.type & data::Type::mask_valuetype);
        OSL_ASSERT(type != data::Type::value_any);
        freeData(type, value);
        value.data = 0;
        info.flags &= ~data::Flags::valueAvailable;
    }
}

sal_uInt8 ValueNode::adaptType(com::sun::star::uno::Any const & newValue) {
    sal_uInt8 newType = getTypeCode(newValue.getValueType());
    OSL_ASSERT(newType != data::Type::value_any);
    sal_uInt8 type = static_cast< sal_uInt8 >(
        info.type & data::Type::mask_valuetype);
    if (type == data::Type::value_any) {
        type = static_cast< sal_uInt8 >(newType & data::Type::mask_valuetype);
        info.type = (info.type & ~data::Type::mask_valuetype) | type;
    }
    OSL_ASSERT(newType == type);
    return type;
}

//-----------------------------------------------------------------------------

bool Node::isNamed(rtl::OUString const & _aName) const
{
    rtl_uString *pCmpData = _aName.pData;
    rtl_uString *pNodeData = info.name;

    // Creating an OUString does rather expensive interlocking here.
    if (pCmpData == pNodeData)
        return true;
    if (pCmpData->length != pNodeData->length)
        return false;
    return !rtl_ustr_compare_WithLength( pCmpData->buffer,
                                         pCmpData->length,
                                         pNodeData->buffer,
                                         pNodeData->length);
}
//-----------------------------------------------------------------------------

rtl::OUString Node::getName() const
{
    return info.getName();
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
        return info.getNodeInfoAttributes();
    }
}
//-----------------------------------------------------------------------------

bool Node::isDefault() const
{
    return info.isDefault();
}
//-----------------------------------------------------------------------------

bool Node::isFragmentRoot() const
{
    return !info.parent;
}
#if OSL_DEBUG_LEVEL > 0
//-----------------------------------------------------------------------------
Node * Node::getParentNode()
{
    return info.parent ? this - info.parent : NULL;
}
//-----------------------------------------------------------------------------

Node const * Node::getParentNode() const
{
    return info.parent ? this - info.parent : NULL;
}
#endif
//-----------------------------------------------------------------------------
static sal_uInt16 getFragmentIndex(Node const * pNode)
{
    sal_uInt16 result = 0;
    while (sal_uInt16 step = pNode->info.parent)
    {
        result = result + step;
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

Node * Node::getSubnode(rtl::OUString const & name) {
    if (isGroup()) {
        return group.getChild(name);
    } else if (isSet()) {
        TreeFragment * element = set.getElement(name);
        return element == 0 ? 0 : element->getRootNode();
    } else {
        OSL_ASSERT(false);
        return 0;
    }
}

//-----------------------------------------------------------------------------
    } // namespace sharable
//-----------------------------------------------------------------------------
} // namespace configmgr



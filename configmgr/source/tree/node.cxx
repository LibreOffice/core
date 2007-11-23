/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: node.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:32:11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "types.hxx"
#include "node.hxx"

#ifndef INCLUDED_SHARABLE_ANYDATA_HXX
#include "anydata.hxx"
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
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

rtl::OUString NodeInfo::getName() const
{
    return readName(this->name);
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
        next = next + _pChild->group.numDescendants;
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
SetNodeTemplateData * readTemplateData(SetElementAddress _aTemplateData)
{
    return reinterpret_cast<SetNodeTemplateData *>( _aTemplateData );
}
//-----------------------------------------------------------------------------

SetElementAddress SetNode::allocTemplateData(const rtl::OUString &rName,
                                   const rtl::OUString &rModule)
{
    SetNodeTemplateData * pData = new SetNodeTemplateData();

    OSL_ENSURE(pData, "Creating template data: unexpected NULL data");

    pData->name   = allocName(rName);
    pData->module = allocName(rModule);

    return reinterpret_cast<SetElementAddress>( pData );
}

SetElementAddress SetNode::copyTemplateData(SetElementAddress _aTemplateData)
{
    SetNodeTemplateData const * pData = readTemplateData(_aTemplateData);

    OSL_ENSURE(pData, "Copying template data: unexpected NULL data");

    return allocTemplateData(readName(pData->name), readName(pData->module));
}

//-----------------------------------------------------------------------------

void SetNode::releaseTemplateData(SetElementAddress _aTemplateData)
{
    if (!_aTemplateData) return;

    SetNodeTemplateData const * pData = readTemplateData(_aTemplateData);

    OSL_ENSURE(pData, "Freeing template data: unexpected NULL data");

    freeName(pData->name);
    freeName(pData->module);

    delete pData;
}

//-----------------------------------------------------------------------------

rtl::OUString SetNode::getElementTemplateName() const
{
    SetNodeTemplateData const * pData = readTemplateData(this->elementType);

    OSL_ENSURE(pData, "ERROR: No template data found for set");

    return readName(pData->name);
}
//-----------------------------------------------------------------------------

rtl::OUString SetNode::getElementTemplateModule() const
{
    SetNodeTemplateData const * pData = readTemplateData(this->elementType);

    OSL_ENSURE(pData, "ERROR: No template data found for set");

    return readName(pData->module);
}
//-----------------------------------------------------------------------------

static inline
TreeFragment const * implGetFragmentFromList(List _aListEntry)
{
    return reinterpret_cast<TreeFragment const *>(_aListEntry);
}
//-----------------------------------------------------------------------------

TreeFragment const  * SetNode::getFirstElement() const
{
    return implGetFragmentFromList(this->elements);
}
//-----------------------------------------------------------------------------

TreeFragment const  * SetNode::getNextElement(TreeFragment const * _pElement) const
{
    OSL_PRECOND(_pElement, "getNextElement: previous element must not be NULL");
    OSL_PRECOND(_pElement->header.parent == (Node *)this,
                "getNextElement: not an element of this node");

    return implGetFragmentFromList(_pElement->header.next);
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

uno::Type ValueNode::getValueType() const
{
    AnyData::TypeCode aType = AnyData::TypeCode( info.type & Type::mask_valuetype );

    return getUnoType(aType);
}
//-----------------------------------------------------------------------------

uno::Any ValueNode::getValue() const
{
    if (info.flags & Flags::defaulted)
        return getDefaultValue();

    else
        return getUserValue();
}
//-----------------------------------------------------------------------------

uno::Any ValueNode::getUserValue() const
{
    if (info.flags & Flags::valueAvailable)
    {
        AnyData::TypeCode aType = AnyData::TypeCode( info.type & Type::mask_valuetype );

        return readData(aType,this->value);
    }
    else
        return uno::Any();
}
//-----------------------------------------------------------------------------

uno::Any ValueNode::getDefaultValue() const
{
    if (info.flags & Flags::defaultAvailable)
    {
        AnyData::TypeCode aType = AnyData::TypeCode( info.type & Type::mask_valuetype );

        return readData(aType,this->defaultValue);
    }
    else
        return uno::Any();
}
//-----------------------------------------------------------------------------

bool Node::isNamed(rtl::OUString const & _aName) const
{
    rtl_uString *pCmpData = _aName.pData;
    rtl_uString *pNodeData = node.info.name;

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
    return node.info.getName();
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

//-----------------------------------------------------------------------------
    } // namespace sharable
//-----------------------------------------------------------------------------
} // namespace configmgr



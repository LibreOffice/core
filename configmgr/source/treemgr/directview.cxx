/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: directview.cxx,v $
 * $Revision: 1.7 $
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
#include <stdio.h>
#include "directview.hxx"
#include "viewfactory.hxx"
#include "setnodeimpl.hxx"

namespace configmgr
{
    namespace view
    {
//-----------------------------------------------------------------------------

void DirectViewStrategy::implMarkNondefault(SetNode const& _aSetNode)
{
    sharable::SetNode * set = _aSetNode.getAccess();

    OSL_ASSERT(set != 0);

    sharable::SetNode* pNode = NULL;
    if (m_aTreeSegment.is())
        pNode = set;

    OSL_ASSERT(pNode);

    pNode->info.markAsDefault(false);
}
//-----------------------------------------------------------------------------

bool DirectViewStrategy::doHasChanges(Node const& ) const
{
    return false;
}
//-----------------------------------------------------------------------------

void DirectViewStrategy::doMarkChanged(Node const& )
{
    // do nothing
}
//-----------------------------------------------------------------------------

node::Attributes DirectViewStrategy::doAdjustAttributes(node::Attributes const& _aAttributes) const
{
    node::Attributes aAttributes = _aAttributes;

    if (aAttributes.isReadonly())
        aAttributes.setAccess(node::accessFinal);

    return aAttributes;
}
//-----------------------------------------------------------------------------

configuration::ValueMemberNode DirectViewStrategy::doGetValueMember(GroupNode const& _aNode, rtl::OUString const& _aName, bool _bForUpdate) const
{
    return ViewStrategy::doGetValueMember(_aNode,_aName,_bForUpdate);
}
//-----------------------------------------------------------------------------
void DirectViewStrategy::doInsertElement(SetNode const& _aNode, rtl::OUString const& _aName, configuration::SetEntry const& _aNewEntry)
{
    // move to this memory segment
    // should already be direct (as any free-floating one)

    //implMakeElement(aNewEntry)
    configuration::ElementTreeData aNewElement = implMakeElement(_aNode, _aNewEntry );
 //   _aNewEntry.tree()->rebuild(this, _aNode.accessor());

    _aNode.get_impl()->insertElement(_aName, aNewElement);

    aNewElement->attachTo( _aNode.getAccess(), _aName );

    implMarkNondefault( _aNode );
}
//-----------------------------------------------------------------------------

void DirectViewStrategy::doRemoveElement(SetNode const& _aNode, rtl::OUString const& _aName)
{
    configuration::ElementTreeData aOldElement = _aNode.get_impl()->removeElement(_aName);

    aOldElement->detachFrom( _aNode.getAccess(), _aName);

    implMarkNondefault( _aNode );
}
//-----------------------------------------------------------------------------

extern NodeFactory& getDirectAccessFactory();

NodeFactory& DirectViewStrategy::doGetNodeFactory()
{
    return getDirectAccessFactory();
}
//-----------------------------------------------------------------------------

rtl::Reference<ViewStrategy> createDirectAccessStrategy(rtl::Reference< data::TreeSegment > const & _aTreeSegment)
{
    return new DirectViewStrategy(_aTreeSegment);
}

//-----------------------------------------------------------------------------
    }
}

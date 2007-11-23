/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: directview.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:41:58 $
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
#include <stdio.h>
#include "directview.hxx"

#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#include "viewfactory.hxx"
#endif

#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#include "setnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif

namespace configmgr
{
    namespace view
    {
//-----------------------------------------------------------------------------

void DirectViewStrategy::implMarkNondefault(SetNode const& _aSetNode)
{
    data::SetNodeAccess aSetAccess = _aSetNode.getAccess();

    OSL_ASSERT(aSetAccess.isValid());

    sharable::SetNode* pNode = NULL;
    if (m_aTreeSegment.is())
        pNode = aSetAccess;

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

configuration::ValueMemberNode DirectViewStrategy::doGetValueMember(GroupNode const& _aNode, Name const& _aName, bool _bForUpdate) const
{
    return ViewStrategy::doGetValueMember(_aNode,_aName,_bForUpdate);
}
//-----------------------------------------------------------------------------
void DirectViewStrategy::doInsertElement(SetNode const& _aNode, Name const& _aName, SetNodeEntry const& _aNewEntry)
{
    // move to this memory segment
    // should already be direct (as any free-floating one)

    //implMakeElement(aNewEntry)
    SetNodeElement aNewElement = implMakeElement(_aNode, _aNewEntry );
 //   _aNewEntry.tree()->rebuild(this, _aNode.accessor());

    _aNode.get_impl()->insertElement(_aName, aNewElement);

    aNewElement->attachTo( _aNode.getAccess(), _aName );

    implMarkNondefault( _aNode );
}
//-----------------------------------------------------------------------------

void DirectViewStrategy::doRemoveElement(SetNode const& _aNode, Name const& _aName)
{
    SetNodeElement aOldElement = _aNode.get_impl()->removeElement(_aName);

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

ViewStrategyRef createDirectAccessStrategy(data::TreeSegment const & _aTreeSegment)
{
    return new DirectViewStrategy(_aTreeSegment);
}

//-----------------------------------------------------------------------------
    }
}

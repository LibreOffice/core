/*************************************************************************
 *
 *  $RCSfile: directview.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:40:04 $
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
 *  Source License Version 1.1 (the "License") You may not use this file
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

    sharable::SetNode* pNode = this->getDataForUpdate(aSetAccess);

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

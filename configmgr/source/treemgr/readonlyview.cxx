/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: readonlyview.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:45:39 $
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
#include "readonlyview.hxx"

#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#include "viewfactory.hxx"
#endif
#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif

namespace configmgr
{
    namespace view
    {
//-----------------------------------------------------------------------------
void ReadOnlyViewStrategy::failReadOnly() const
{
    OSL_ENSURE(false, "Changing operation attempted on read-only node data");
    throw configuration::ConstraintViolation("INTERNAL ERROR: Trying to update a read-only node");
}

//-----------------------------------------------------------------------------

bool ReadOnlyViewStrategy::doHasChanges(Node const& ) const
{
    return false;
}
//-----------------------------------------------------------------------------

void ReadOnlyViewStrategy::doMarkChanged(Node const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

node::Attributes ReadOnlyViewStrategy::doAdjustAttributes(node::Attributes const& _aAttributes) const
{
    node::Attributes aAttributes = _aAttributes;
    aAttributes.markReadonly();
    return aAttributes;
}
//-----------------------------------------------------------------------------

configuration::ValueMemberNode ReadOnlyViewStrategy::doGetValueMember(GroupNode const& _aNode, Name const& _aName, bool _bForUpdate) const
{
    if (_bForUpdate) failReadOnly();

    return ViewStrategy::doGetValueMember(_aNode,_aName,_bForUpdate);
}
//-----------------------------------------------------------------------------

void ReadOnlyViewStrategy::doInsertElement(SetNode const& , Name const& , SetNodeEntry const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyViewStrategy::doRemoveElement(SetNode const& /*_aNode*/, Name const& /*_aName*/)
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

extern NodeFactory& getReadAccessFactory();

NodeFactory& ReadOnlyViewStrategy::doGetNodeFactory()
{
    return getReadAccessFactory();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ViewStrategyRef createReadOnlyStrategy()
{
    return new ReadOnlyViewStrategy();
}

//-----------------------------------------------------------------------------

    }
}

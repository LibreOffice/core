/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: readonlyview.cxx,v $
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
#include "readonlyview.hxx"
#include "viewfactory.hxx"
#include "configexcept.hxx"

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

configuration::ValueMemberNode ReadOnlyViewStrategy::doGetValueMember(GroupNode const& _aNode, rtl::OUString const& _aName, bool _bForUpdate) const
{
    if (_bForUpdate) failReadOnly();

    return ViewStrategy::doGetValueMember(_aNode,_aName,_bForUpdate);
}
//-----------------------------------------------------------------------------

void ReadOnlyViewStrategy::doInsertElement(SetNode const& , rtl::OUString const& , configuration::SetEntry const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyViewStrategy::doRemoveElement(SetNode const& /*_aNode*/, rtl::OUString const& /*_aName*/)
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

rtl::Reference<ViewStrategy> createReadOnlyStrategy()
{
    return new ReadOnlyViewStrategy();
}

//-----------------------------------------------------------------------------

    }
}

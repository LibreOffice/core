/*************************************************************************
 *
 *  $RCSfile: readonlyview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:41:21 $
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

void ReadOnlyViewStrategy::doRemoveElement(SetNode const& _aNode, Name const& _aName)
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

ViewStrategyRef createReadOnlyStrategy(memory::Segment const * _pSegment)
{
    return new ReadOnlyViewStrategy(_pSegment);
}

//-----------------------------------------------------------------------------

    }
}

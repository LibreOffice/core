/*************************************************************************
 *
 *  $RCSfile: treeactions.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:38:08 $
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

#include <stdio.h>

#include "treeactions.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


//..........................................................................
namespace configmgr
{
// -------------------------------------------------------------------------
namespace
{
    struct ForceWritable : NodeModification
    {
        void handle(ValueNode& _rValue)     { implForceWritable(_rValue); }
        void handle(ISubtree& _rSubtree)    { implForceWritable(_rSubtree); applyToChildren(_rSubtree); }

        void implForceWritable(INode& _rNode) { _rNode.forceReadonlyToFinalized(); }
    };
}

// -------------------------------------------------------------------------
void forceWritable(INode& _rNode)
{
    ForceWritable aVisitor;

    aVisitor.applyToNode(_rNode);
}

//==========================================================================
//= OIdPropagator
//==========================================================================

void OIdPropagator::propagateIdToChildren(ISubtree& rTree)
{
    if (rTree.hasId())
    {
        OIdPropagator aAction(rTree.getId());
        aAction.applyToChildren(rTree);
    }
}
//--------------------------------------------------------------------------

void OIdPropagator::propagateIdToTree(OUString const& aId, ISubtree& rTree)
{
    OSL_ENSURE(!rTree.hasId(), "OIdPropagator::propagateIdToTree: Tree already has an Id, propagating may not work");
    rTree.setId(aId);
    propagateIdToChildren(rTree);
}
//--------------------------------------------------------------------------

void OIdPropagator::handle(ValueNode& _rValueNode)
{ /* not interested in value nodes */ }
//--------------------------------------------------------------------------

void OIdPropagator::handle(ISubtree& _rSubtree)
{
    if (!_rSubtree.hasId())
    {
        _rSubtree.setId(sId);
        applyToChildren(_rSubtree);
    }
}

//==========================================================================
//= OIdRemover
//==========================================================================

void OIdRemover::removeIds(INode& rNode)
{
    OIdRemover().applyToNode(rNode);
}
//--------------------------------------------------------------------------

void OIdRemover::handle(ValueNode& _rValueNode)
{ /* not interested in value nodes */ }
//--------------------------------------------------------------------------

void OIdRemover::handle(ISubtree& _rSubtree)
{
    if (_rSubtree.hasId())
    {
        _rSubtree.setId(OUString());
        applyToChildren(_rSubtree);
    }
}

//==========================================================================
//= OChangeActionCounter
//==========================================================================

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(ValueChange const& aValueNode){ ++nValues; }

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(AddNode const& aAddNode){ ++nAdds; }

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(RemoveNode const& aRemoveNode){ ++nRemoves; }

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(SubtreeChange const& aSubtree)
{
    applyToChildren(aSubtree);
}

//..........................................................................
}   // namespace configmgr
//..........................................................................



/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treeactions.hxx,v $
 * $Revision: 1.18 $
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

#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#define _CONFIGMGR_TREEACTIONS_HXX_

#include "change.hxx"

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

//..........................................................................
namespace configmgr
{
//-----------------------------------------------------------------------------

//==========================================================================
//= OChangeActionCounter
//==========================================================================
/** collects meta data about a changes tree
*/
struct OChangeActionCounter : public ChangeTreeAction
{
    sal_Int32       nValues, nAdds, nRemoves;

    OChangeActionCounter() :nValues(0), nAdds(0), nRemoves(0) {}

    virtual void handle(ValueChange const& aValueNode);
    virtual void handle(AddNode const& aAddNode);
    virtual void handle(RemoveNode const& aRemoveNode);
    virtual void handle(SubtreeChange const& aSubtree);

    sal_Bool hasChanges() const {return nValues || nAdds || nRemoves;}
};

// ===================================================================
// = CollectNames
// ===================================================================
class CollectNames :  public NodeAction
{
public:
    std::vector<rtl::OUString> const& list() const { return aList; }

    CollectNames() : aList() {}

    void handle(ValueNode const& aValue)    { add(aValue); }
    void handle(ISubtree const&  m_aSubtree)    { add(m_aSubtree); }

    void add(INode const& aNode)
    {
        aList.push_back(aNode.getName());
    }
private:
    std::vector<rtl::OUString> aList;
};

//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // _CONFIGMGR_TREEACTIONS_HXX_



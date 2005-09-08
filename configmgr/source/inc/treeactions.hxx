/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeactions.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:58:32 $
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

#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#define _CONFIGMGR_TREEACTIONS_HXX_

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

//..........................................................................
namespace configmgr
{
//-----------------------------------------------------------------------------

// helper to implement force-writable support
void forceWritable(INode& _rNode);
void forceWritable(data::TreeAccessor const & _aTree);

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
    typedef std::vector<OUString> NameList;

public:
    NameList const& list() const { return aList; }

    CollectNames() : aList() {}

    void handle(ValueNode const& aValue)    { add(aValue); }
    void handle(ISubtree const&  m_aSubtree)    { add(m_aSubtree); }

    void add(INode const& aNode)
    {
        aList.push_back(aNode.getName());
    }
private:
    NameList aList;
};

//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // _CONFIGMGR_TREEACTIONS_HXX_



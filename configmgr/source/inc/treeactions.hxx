/*************************************************************************
 *
 *  $RCSfile: treeactions.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:07 $
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
//= OIdPropagator
//==========================================================================
/** propagates a node id to a subtree and its descendants
*/
class OIdPropagator : private NodeModification
{
    ::rtl::OUString sId;

    OIdPropagator(const ::rtl::OUString& _rId) : sId(_rId) { }
    virtual void handle(ValueNode& _rValueNode);
    virtual void handle(ISubtree& _rSubtree);

public:
    static void propagateIdToChildren(ISubtree& rTree);
    static void propagateIdToTree( ::rtl::OUString const& aId, ISubtree& rTree);
};

//==========================================================================
//= OIdRemover
//==========================================================================
/** propagates a node id to a subtree and its descendants
*/
class OIdRemover : private NodeModification
{
    virtual void handle(ValueNode& _rValueNode);
    virtual void handle(ISubtree& _rSubtree);

public:
    static void removeIds(INode& rNode);
};

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



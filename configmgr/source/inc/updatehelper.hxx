/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatehelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:27:05 $
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

#ifndef CONFIGMGR_UPDATEHELPER_HXX
#define CONFIGMGR_UPDATEHELPER_HXX

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#include "nodeaccess.hxx"

//..........................................................................
namespace configmgr
{
//..........................................................................

// adjust a set of changes to the target tree, return true, if there are changes left
    bool adjustUpdateToTree(SubtreeChange & _rUpdateTree, data::NodeAccess const & _aRootNode);

// adjust a set of changes to the target tree, return true, if there are changes left
    bool adjustUpdateToTree(SubtreeChange & _rUpdateTree, data::NodeAddress _aRootNode);

// apply a already matching set of changes to the target tree
    void applyUpdateToTree(SubtreeChange& _anUpdateTree, data::NodeAddress _aRootNode);

// apply a set of changes to the target tree
    void applyUpdateWithAdjustmentToTree(SubtreeChange& _anUpdateTree, data::NodeAddress _aRootNode);

// apply a set of changes to the target tree, return true, if there are changes found
    bool createUpdateFromDifference(SubtreeChange& _rResultingUpdateTree, data::NodeAccess const & _aExistingData, ISubtree const & _aNewData);

//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // CONFIGMGR_MERGEHELPER_HXX



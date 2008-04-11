/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updatehelper.hxx,v $
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

#ifndef CONFIGMGR_UPDATEHELPER_HXX
#define CONFIGMGR_UPDATEHELPER_HXX

#include "change.hxx"

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



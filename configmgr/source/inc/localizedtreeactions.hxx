/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localizedtreeactions.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_LOCALIZEDTREEACTIONS_HXX
#define CONFIGMGR_LOCALIZEDTREEACTIONS_HXX

#include "valuenode.hxx"
#include "treesegment.hxx"
//..........................................................................
namespace configmgr
{
class SubtreeChange;

//==========================================================================
// Helper function to invoke the previous ones properly

// convert to the given locale format, no matter what the original representation
data::TreeSegment cloneForLocale(INode const* _pNode, OUString const& _sLocale);
// convert to the given locale format, assuming the original representation was expanded
data::TreeSegment cloneExpandedForLocale(data::TreeAccessor const & _aTree, OUString const& _sLocale);
// convert to the given locale format, assuming the original representation was expanded
std::auto_ptr<INode> reduceExpandedForLocale(std::auto_ptr<ISubtree> _pNode, OUString const& _sLocale);

// convert to the expanded locale format, assuming the original representation was reduced for the given locale
void expandForLocale(ISubtree& _rNode, OUString const& _sLocale);
// convert to the expanded locale format, assuming the original representation was reduced for the given locale
void expandForLocale(SubtreeChange& _rNode, OUString const& _sLocale);

//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // CONFIGMGR_LOCALIZEDTREEACTIONS_HXX



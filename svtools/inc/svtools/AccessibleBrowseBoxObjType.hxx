/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleBrowseBoxObjType.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:09:47 $
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


#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOXOBJTYPE_HXX
#define _SVTOOLS_ACCESSIBLEBROWSEBOXOBJTYPE_HXX

// ============================================================================

namespace svt {

// ============================================================================

/** This enumeration contains a constant for each kind of accessible object of
    a BrowseBox. */
enum AccessibleBrowseBoxObjType
{
    BBTYPE_BROWSEBOX,           /// The BrowseBox itself.
    BBTYPE_TABLE,               /// The data table.
    BBTYPE_ROWHEADERBAR,        /// The row header bar ("handle column").
    BBTYPE_COLUMNHEADERBAR,     /// The horizontal column header bar.
    BBTYPE_TABLECELL,           /// A cell of the data table.
    BBTYPE_ROWHEADERCELL,       /// A cell of the row header bar.
    BBTYPE_COLUMNHEADERCELL,    /// A cell of the column header bar.
    BBTYPE_CHECKBOXCELL         /// A cell with a checkbox.
};

// ============================================================================

} // namespace svt

// ============================================================================

#endif


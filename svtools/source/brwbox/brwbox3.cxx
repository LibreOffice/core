/*************************************************************************
 *
 *  $RCSfile: brwbox3.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-04-02 08:51:24 $
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

#ifndef _SVX_BRWBOX_HXX
#include "brwbox.hxx"
#endif
#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOX_HXX
#include "AccessibleBrowseBox.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

// Accessibility ==============================================================

using ::rtl::OUString;
using ::svt::AccessibleBrowseBox;
using ::com::sun::star::uno::Reference;
using ::drafts::com::sun::star::accessibility::XAccessible;

// ============================================================================

Reference< XAccessible > BrowseBox::CreateAccessible()
{
    Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "BrowseBox::CreateAccessible - parent not found" );

    Reference< XAccessible > xAccessible;
    if( pParent && !m_xAccessible.is() )
    {
        Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if( xAccParent.is() )
            m_xAccessible = new AccessibleBrowseBox( xAccParent, *this );
    }
    return m_xAccessible;
}

// Children -------------------------------------------------------------------

Reference< XAccessible > BrowseBox::CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId )
{
    return NULL;
}

Reference< XAccessible > BrowseBox::CreateAccessibleRowHeader( sal_Int32 nRow )
{
    return NULL;
}

Reference< XAccessible > BrowseBox::CreateAccessibleColumnHeader( sal_uInt16 nColumnId )
{
    return NULL;
}

sal_Int32 BrowseBox::GetAccessibleControlCount() const
{
    return 0;
}

Reference< XAccessible > BrowseBox::CreateAccessibleControl( sal_Int32 nIndex )
{
    return NULL;
}

// Conversions ----------------------------------------------------------------

sal_Bool BrowseBox::ConvertPointToCellAddress(
        sal_Int32& rnRow, sal_uInt16& rnColumnId, const Point& rPoint )
{
    //! TODO
    return sal_False;
}

sal_Bool BrowseBox::ConvertPointToRowHeader( sal_Int32& rnRow, const Point& rPoint )
{
    //! TODO
    return sal_False;
}

sal_Bool BrowseBox::ConvertPointToColumnHeader( sal_uInt16& rnColumnId, const Point& rPoint )
{
    //! TODO
    return sal_False;
}

sal_Bool BrowseBox::ConvertPointToControlIndex( sal_Int32& rnIndex, const Point& rPoint )
{
    //! TODO
    return sal_False;
}

// Object data and state ------------------------------------------------------

OUString BrowseBox::GetAccessibleName( ::svt::AccessibleBrowseBoxObjType eObjType ) const
{
    OUString aRetText;
    switch( eObjType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
            //! TODO only a test name
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "BrowseBox" ) );
        break;
#if 0
        //! TODO all texts
        case ::svt::BBTYPE_TABLE:
        case ::svt::BBTYPE_ROWHEADERBAR:
        case ::svt::BBTYPE_COLUMNHEADERBAR:
        case ::svt::BBTYPE_CORNERCONTROL:
        case ::svt::BBTYPE_TABLECELL:
        case ::svt::BBTYPE_ROWHEADERCELL:
        case ::svt::BBTYPE_COLUMNHEADERCELL:
#endif
    }
    return aRetText;
}

OUString BrowseBox::GetAccessibleDescription( ::svt::AccessibleBrowseBoxObjType eObjType ) const
{
    OUString aRetText;
    switch( eObjType )
    {
        case ::svt::BBTYPE_BROWSEBOX:
            //! TODO only a test name
            aRetText = OUString( RTL_CONSTASCII_USTRINGPARAM( "BrowseBox description" ) );
        break;
#if 0
        //! TODO all texts
        case ::svt::BBTYPE_TABLE:
        case ::svt::BBTYPE_ROWHEADERBAR:
        case ::svt::BBTYPE_COLUMNHEADERBAR:
        case ::svt::BBTYPE_CORNERCONTROL:
        case ::svt::BBTYPE_TABLECELL:
        case ::svt::BBTYPE_ROWHEADERCELL:
        case ::svt::BBTYPE_COLUMNHEADERCELL:
#endif
    }
    return aRetText;
}

OUString BrowseBox::GetRowDescription( sal_Int32 nRow ) const
{
    return OUString();
}

void BrowseBox::FillAccessibleStateSet(
        ::utl::AccessibleStateSetHelper& rStateSet,
        ::svt::AccessibleBrowseBoxObjType eObjType ) const
{
    //! TODO
}

void BrowseBox::GrabTableFocus()
{
    //! TODO EditBrowseBox has to grab focus of the edit control of the current cell (ActivateCell)
    GrabFocus();
}


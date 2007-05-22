/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RangeSelectionButton.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:28:57 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "RangeSelectionButton.hxx"
#include "Bitmaps.hrc"
#include "Bitmaps_HC.hrc"
#include "Strings.hrc"
#include "ResId.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

RangeSelectionButton::RangeSelectionButton( Window *pParent, const ResId& rResId ) :
    ImageButton( pParent, rResId )
{
    SetModeImage( Image( SchResId(IMG_SELECTRANGE)    ), BMP_COLOR_NORMAL );
    SetModeImage( Image( SchResId(IMG_SELECTRANGE_HC) ), BMP_COLOR_HIGHCONTRAST );
    this->SetQuickHelpText(SchResId(STR_TIP_SELECT_RANGE));
}

RangeSelectionButton::~RangeSelectionButton()
{
}

//.............................................................................
} //namespace chart
//.............................................................................

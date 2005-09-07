/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idetemp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:04:27 $
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
#ifndef _IDETEMP_HXX
#define _IDETEMP_HXX


enum SvxChooseControlEnum
{
    SVX_SNAP_PUSHBUTTON,
    SVX_SNAP_CHECKBOX,
    SVX_SNAP_RADIOBUTTON,
    SVX_SNAP_SPINBUTTON,
    SVX_SNAP_FIXEDTEXT,
    SVX_SNAP_GROUPBOX,
    SVX_SNAP_LISTBOX,
    SVX_SNAP_COMBOBOX,
    SVX_SNAP_EDIT,
    SVX_SNAP_HSCROLLBAR,
    SVX_SNAP_VSCROLLBAR,
    SVX_SNAP_PREVIEW,
    SVX_SNAP_SELECT,
    SVX_SNAP_URLBUTTON,
    SVX_SNAP_NOTHING,
    SVX_SNAP_IMAGECONTROL,
    SVX_SNAP_PROGRESSBAR,
    SVX_SNAP_HFIXEDLINE,
    SVX_SNAP_VFIXEDLINE,
    SVX_SNAP_DATEFIELD,
    SVX_SNAP_TIMEFIELD,
    SVX_SNAP_NUMERICFIELD,
    SVX_SNAP_CURRENCYFIELD,
    SVX_SNAP_FORMATTEDFIELD,
    SVX_SNAP_PATTERNFIELD,
    SVX_SNAP_FILECONTROL
};

#define SvxChooseControlItem SfxAllEnumItem

#endif


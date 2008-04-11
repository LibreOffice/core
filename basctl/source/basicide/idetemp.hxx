/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: idetemp.hxx,v $
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
    SVX_SNAP_FILECONTROL,
    SVX_SNAP_TREECONTROL
};

#define SvxChooseControlItem SfxAllEnumItem

#endif


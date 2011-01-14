/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _BASCTL_DLGEDDEF_HXX
#define _BASCTL_DLGEDDEF_HXX

#include <tools/solar.h>


const sal_uInt32 DlgInventor = sal_uInt32('D')*0x00000001+
                           sal_uInt32('L')*0x00000100+
                           sal_uInt32('G')*0x00010000+
                           sal_uInt32('1')*0x01000000;


#define OBJ_DLG_CONTROL         ((sal_uInt16) 1)
#define OBJ_DLG_DIALOG          ((sal_uInt16) 2)
#define OBJ_DLG_PUSHBUTTON      ((sal_uInt16) 3)
#define OBJ_DLG_RADIOBUTTON     ((sal_uInt16) 4)
#define OBJ_DLG_CHECKBOX        ((sal_uInt16) 5)
#define OBJ_DLG_LISTBOX         ((sal_uInt16) 6)
#define OBJ_DLG_COMBOBOX        ((sal_uInt16) 7)
#define OBJ_DLG_GROUPBOX        ((sal_uInt16) 8)
#define OBJ_DLG_EDIT            ((sal_uInt16) 9)
#define OBJ_DLG_FIXEDTEXT       ((sal_uInt16)10)
#define OBJ_DLG_IMAGECONTROL    ((sal_uInt16)11)
#define OBJ_DLG_PROGRESSBAR     ((sal_uInt16)12)
#define OBJ_DLG_HSCROLLBAR      ((sal_uInt16)13)
#define OBJ_DLG_VSCROLLBAR      ((sal_uInt16)14)
#define OBJ_DLG_HFIXEDLINE      ((sal_uInt16)15)
#define OBJ_DLG_VFIXEDLINE      ((sal_uInt16)16)
#define OBJ_DLG_DATEFIELD       ((sal_uInt16)17)
#define OBJ_DLG_TIMEFIELD       ((sal_uInt16)18)
#define OBJ_DLG_NUMERICFIELD    ((sal_uInt16)19)
#define OBJ_DLG_CURRENCYFIELD   ((sal_uInt16)20)
#define OBJ_DLG_FORMATTEDFIELD  ((sal_uInt16)21)
#define OBJ_DLG_PATTERNFIELD    ((sal_uInt16)22)
#define OBJ_DLG_FILECONTROL     ((sal_uInt16)23)
#define OBJ_DLG_TREECONTROL     ((sal_uInt16)24)

// control properties
#define DLGED_PROP_BACKGROUNDCOLOR      ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BackgroundColor" ) )
#define DLGED_PROP_DROPDOWN             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Dropdown" ) )
#define DLGED_PROP_FORMATSSUPPLIER      ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FormatsSupplier" ) )
#define DLGED_PROP_HEIGHT               ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) )
#define DLGED_PROP_LABEL                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" ) )
#define DLGED_PROP_NAME                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) )
#define DLGED_PROP_ORIENTATION          ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Orientation" ) )
#define DLGED_PROP_POSITIONX            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) )
#define DLGED_PROP_POSITIONY            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) )
#define DLGED_PROP_STEP                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Step" ) )
#define DLGED_PROP_TABINDEX             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) )
#define DLGED_PROP_TEXTCOLOR            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TextColor" ) )
#define DLGED_PROP_TEXTLINECOLOR        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TextLineColor" ) )
#define DLGED_PROP_WIDTH                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) )
#define DLGED_PROP_DECORATION           ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Decoration" ) )


#endif // _BASCTL_DLGEDDEF_HXX

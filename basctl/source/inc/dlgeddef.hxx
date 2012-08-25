/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef BASCTL_DLGEDDEF_HXX
#define BASCTL_DLGEDDEF_HXX

#include <sal/types.h>

namespace basctl
{

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
#define OBJ_DLG_SPINBUTTON      ((sal_uInt16)25)

#define OBJ_DLG_FORMRADIO       ((sal_uInt16)26)
#define OBJ_DLG_FORMCHECK       ((sal_uInt16)27)
#define OBJ_DLG_FORMLIST        ((sal_uInt16)28)
#define OBJ_DLG_FORMCOMBO       ((sal_uInt16)29)
#define OBJ_DLG_FORMSPIN        ((sal_uInt16)30)
#define OBJ_DLG_FORMVSCROLL     ((sal_uInt16)31)
#define OBJ_DLG_FORMHSCROLL     ((sal_uInt16)32)

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


} // namespace basctl

#endif // BASCTL_DLGEDDEF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

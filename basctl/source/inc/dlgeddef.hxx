/*************************************************************************
 *
 *  $RCSfile: dlgeddef.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2001-07-30 07:44:39 $
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

#ifndef _BASCTL_DLGEDDEF_HXX
#define _BASCTL_DLGEDDEF_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif


const UINT32 DlgInventor = UINT32('D')*0x00000001+
                           UINT32('L')*0x00000100+
                           UINT32('G')*0x00010000+
                           UINT32('1')*0x01000000;


#define OBJ_DLG_DIALOG          ((UINT16) 1)
#define OBJ_DLG_PUSHBUTTON      ((UINT16) 2)
#define OBJ_DLG_RADIOBUTTON     ((UINT16) 3)
#define OBJ_DLG_CHECKBOX        ((UINT16) 4)
#define OBJ_DLG_LISTBOX         ((UINT16) 5)
#define OBJ_DLG_COMBOBOX        ((UINT16) 6)
#define OBJ_DLG_GROUPBOX        ((UINT16) 7)
#define OBJ_DLG_EDIT            ((UINT16) 8)
#define OBJ_DLG_FIXEDTEXT       ((UINT16) 9)
#define OBJ_DLG_HSCROLLBAR      ((UINT16)10)
#define OBJ_DLG_VSCROLLBAR      ((UINT16)11)
#define OBJ_DLG_IMAGECONTROL    ((UINT16)12)
#define OBJ_DLG_PROGRESSBAR     ((UINT16)13)
#define OBJ_DLG_HFIXEDLINE      ((UINT16)14)
#define OBJ_DLG_VFIXEDLINE      ((UINT16)15)
#define OBJ_DLG_SPINBUTTON      ((UINT16)16)
#define OBJ_DLG_URLBUTTON       ((UINT16)17)
#define OBJ_DLG_PREVIEW         ((UINT16)18)


#endif // _BASCTL_DLGEDDEF_HXX

/*************************************************************************
 *
 *  $RCSfile: flagsdef.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:24:22 $
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
#ifndef _SVX_FLAGSDEF_HXX
#define _SVX_FLAGSDEF_HXX

// defines ---------------------------------------------------------------

//CHINA001 copy from border.hxx
// Border-Modes for paragraphs, textframes and tables
#define SW_BORDER_MODE_PARA     0x01
#define SW_BORDER_MODE_TABLE    0x02
#define SW_BORDER_MODE_FRAME    0x04

//CHINA001 flags for SvxBackgroundTabPage
#define SVX_SHOW_SELECTOR       0x01
#define SVX_SHOW_PARACTL        0x02
#define SVX_ENABLE_TRANSPARENCY     0x04
#define SVX_SHOW_TBLCTL         0x08

//CHINA001 flags for SvxBorderTabPage
#define SVX_HIDESHADOWCTL   0x01

//CHINA001 copy from chardlg.hxx
#define DISABLE_CASEMAP             ((USHORT)0x0001)
#define DISABLE_WORDLINE            ((USHORT)0x0002)
#define DISABLE_BLINK               ((USHORT)0x0004)
#define DISABLE_UNDERLINE_COLOR     ((USHORT)0x0008)

#define DISABLE_LANGUAGE            ((USHORT)0x0010)
#define DISABLE_HIDE_LANGUAGE       ((USHORT)0x0020)

//CHINA001 flags for SvxCharBasePage's child class
#define SVX_PREVIEW_CHARACTER   0x01
//CHINA001 flags for SvxCharNamePage
#define SVX_RELATIVE_MODE       0x02
//CHINA001 flags for SvxCharEffectsPage
#define SVX_ENABLE_FLASH    0x04


//CHINA001 copy from numfmt.hxx
#define SVX_NUMVAL_STANDARD     -1234.12345678901234
#define SVX_NUMVAL_CURRENCY     -1234
#define SVX_NUMVAL_PERCENT      -0.1295
#define SVX_NUMVAL_TIME         36525.5678935185
#define SVX_NUMVAL_DATE         36525.5678935185
#define SVX_NUMVAL_BOOLEAN      1

//CHINA001 copy from page.hxx
// enum ------------------------------------------------------------------

enum SvxModeType
{
    SVX_PAGE_MODE_STANDARD,
    SVX_PAGE_MODE_CENTER,
    SVX_PAGE_MODE_PRESENTATION
};
// define ----------------------------------------------------------------

// 1/2 cm in TWIPS
// wird auch fuer Minimalgrosse der LayFrms aller Arten benutzt
#define MM50   283  //from original svx/inc/paragrph.hxx

//--------------from original svx/inc/tabstpge.hxx
#define TABTYPE_LEFT        0x0001
#define TABTYPE_RIGHT       0x0002
#define TABTYPE_CENTER      0x0004
#define TABTYPE_DEZIMAL     0x0008
#define TABTYPE_ALL         0x000F

#define TABFILL_NONE        0x0010
#define TABFILL_POINT       0x0020
#define TABFILL_DASHLINE    0x0040
#define TABFILL_SOLIDLINE   0x0080
#define TABFILL_SPECIAL     0x0100
#define TABFILL_ALL         0x01F0

//-----

#endif

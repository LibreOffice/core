/*************************************************************************
 *
 *  $RCSfile: typemap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:27 $
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

#pragma hdrstop

#define ITEMID_PTR              0
#define ITEMID_SHADOW           0
#define ITEMID_PAGE             0
#define ITEMID_SETITEM          0
#define ITEMID_HORJUSTIFY       0
#define ITEMID_VERJUSTIFY       0
#define ITEMID_ORIENTATION      0
#define ITEMID_MARGIN           0
#define ITEMID_AUTHOR           0
#define ITEMID_DATE             0
#define ITEMID_TEXT             0
#define ITEMID_SPELLCHECK       0
#define ITEMID_HYPHENREGION     0
#define ITEMID_FONT             0
#define ITEMID_FONTHEIGHT       0
#define ITEMID_COLOR            0
#define ITEMID_BRUSH            0
#define ITEMID_BRUSH            0
#define ITEMID_BOX              0
#define ITEMID_LINE             0
#define ITEMID_BRUSH            0
#define ITEMID_SPELLCHECK       0
#define ITEMID_HYPHENREGION     0
#define ITEMID_LINESPACING      0
#define ITEMID_ADJUST           0
#define ITEMID_WIDOWS           0
#define ITEMID_ORPHANS          0
#define ITEMID_HYPHENZONE       0
#define ITEMID_TABSTOP          0
#define ITEMID_FMTSPLIT         0
#define ITEMID_PAGEMODEL        0
#define ITEMID_FONTLIST         0
#define ITEMID_FONT             0
#define ITEMID_POSTURE          0
#define ITEMID_WEIGHT           0
#define ITEMID_FONTHEIGHT       0
#define ITEMID_FONTWIDTH        0
#define ITEMID_UNDERLINE        0
#define ITEMID_CROSSEDOUT       0
#define ITEMID_SHADOWED         0
#define ITEMID_AUTOKERN         0
#define ITEMID_WORDLINEMODE     0
#define ITEMID_CONTOUR          0
#define ITEMID_PROPSIZE         0
#define ITEMID_COLOR            0
#define ITEMID_CHARSETCOLOR     0
#define ITEMID_KERNING          0
#define ITEMID_CASEMAP          0
#define ITEMID_ESCAPEMENT       0
#define ITEMID_LANGUAGE         0
#define ITEMID_NOLINEBREAK      0
#define ITEMID_NOHYPHENHERE     0
#define ITEMID_COLOR            0
#define ITEMID_FONT             0
#define ITEMID_FONTHEIGHT       0
#define ITEMID_SEARCH           0
#define ITEMID_COLOR_TABLE      0
#define ITEMID_GRADIENT_LIST    0
#define ITEMID_HATCH_LIST       0
#define ITEMID_BITMAP_LIST      0
#define ITEMID_DASH_LIST        0
#define ITEMID_LINEEND_LIST     0
#define ITEMID_NUMBERINFO       0
#define ITEMID_CHARTSTYLE       0
#define ITEMID_CHARTDATADESCR   0
#define ITEMID_CHARTLEGENDPOS   0
#define ITEMID_CHARTTEXTORDER   0
#define ITEMID_CHARTTEXTORIENT  0
#define ITEMID_DOUBLE           0
#define ITEMID_TABSTOP          0
#define ITEMID_PAPERBIN         0
#define ITEMID_SIZE             0
#define ITEMID_LRSPACE          0
#define ITEMID_ULSPACE          0
#define ITEMID_PRINT            0
#define ITEMID_OPAQUE           0
#define ITEMID_PROTECT          0
#define ITEMID_MACRO            0
#define ITEMID_BOX              0
#define ITEMID_BOXINFO          0
#define ITEMID_FMTBREAK         0
#define ITEMID_FMTKEEP          0
#define ITEMID_LINE             0
#define ITEMID_BRUSH            0

#define CharSetItem SfxVoidItem
#define FontFamilyItem SfxVoidItem
#define FontPitchItem SfxVoidItem
#define FontAlignItem SfxVoidItem
#define FontWeightItem SfxVoidItem
#define FontUnderlineItem SfxVoidItem
#define FontStrikeoutItem SfxVoidItem
#define FontItalicItem SfxVoidItem
#define SvxDbTypeItem SfxVoidItem
#define SvxLineSpaceItem SfxVoidItem
#define SvxInterLineSpaceItem SfxVoidItem
#define SvxBreakItem SfxVoidItem
#define BrushStyleItem SfxVoidItem
#define SvxNumTypeItem SfxVoidItem
#define SvxShadowLocationItem SfxVoidItem
#define SvxLanguage SfxVoidItem
#define SvxChooseControlEnumItem SfxVoidItem
#define SvxDrawToolEnumItem SfxVoidItem
#define SvxChooseControlItem SfxVoidItem
#define SvxDrawToolItem SfxVoidItem
#define SvxCellHorJustifyEnumItem SfxVoidItem
#define SvxCellVerJustifyEnumItem SfxVoidItem
#define SvxCellOrientationEnumItem SfxVoidItem

#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif

#define SFX_TYPEMAP
#include "smslots.hxx"




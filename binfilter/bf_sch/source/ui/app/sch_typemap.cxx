/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svx/eeitem.hxx>

#include "eetext.hxx"
#include <bf_svx/fontitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/cntritem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/colritem.hxx>

#include <bf_svx/svxids.hrc>

#define ITEMID_FMTSPLIT		0
#define ITEMID_PAGEMODEL	0
#include "eetext.hxx"

#define ITEMID_LINE			0
#define ITEMID_BRUSH		0
#define ITEMID_FMTBREAK		0
#define ITEMID_FMTKEEP		0
#define ITEMID_SIZE			0
#include <bf_svx/sizeitem.hxx>

#define ITEMID_PAGE			0


#include <bf_svtools/stritem.hxx>

#define ITEMID_AUTHOR		0
#define ITEMID_DATE			0
#define ITEMID_TEXT			0

#define ITEMID_SEARCH		0

#define ITEMID_HORJUSTIFY	0
#define ITEMID_VERJUSTIFY	0
#define ITEMID_ORIENTATION	0

#include <bf_svx/zoomitem.hxx>

#define ITEMID_CHARTSTYLE	0
#include <bf_svx/chrtitem.hxx>

#include <bf_sfx2/msg.hxx>

namespace binfilter {

/*?*/ typedef SfxVoidItem	CharSetItem;
/*?*/ typedef SfxVoidItem	FontFamilyItem;
/*?*/ typedef SfxVoidItem	FontPitchItem;
/*?*/ typedef SfxVoidItem	FontAlignItem;
/*?*/ typedef SfxVoidItem	FontWeightItem;
/*?*/ typedef SfxVoidItem	FontUnderlineItem;
/*?*/ typedef SfxVoidItem	FontStrikeoutItem;
/*?*/ typedef SfxVoidItem	FontItalicItem;
/*?*/ typedef SfxVoidItem	SvxDbTypeItem;
/*?*/ typedef SfxVoidItem SvxLineSpaceItem;
/*?*/ typedef SfxVoidItem	SvxInterLineSpaceItem;
/*?*/ typedef SfxVoidItem	SvxBreakItem;
/*?*/ typedef SfxVoidItem	BrushStyleItem;
/*?*/ typedef SfxVoidItem	SvxNumTypeItem;
/*?*/ typedef SfxVoidItem	SvxShadowLocationItem;
/*?*/ typedef SfxVoidItem	SvxChooseControlEnumItem;
/*?*/ typedef SfxVoidItem	SvxDrawToolEnumItem;
/*?*/ typedef SfxVoidItem	SvxChooseControlItem;
/*?*/ typedef SfxVoidItem	SvxDrawToolItem;
/*?*/ typedef SfxVoidItem	SvxCellHorJustifyEnumItem;
/*?*/ typedef SfxVoidItem	SvxCellVerJustifyEnumItem;
/*?*/ typedef SfxVoidItem	SvxCellOrientationEnumItem;
/*?*/ typedef SfxEnumItem	SvxStyleTypeItem;

#define SFX_TYPEMAP
#include "schslots.hxx"





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

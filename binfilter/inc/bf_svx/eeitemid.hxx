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

#ifndef _EEITEMID_HXX
#define _EEITEMID_HXX

namespace binfilter {

#ifdef ITEMID_FONT
#undef ITEMID_FONT
#endif
#ifdef ITEMID_FONTHEIGHT
#undef ITEMID_FONTHEIGHT
#endif
#ifdef ITEMID_COLOR
#undef ITEMID_COLOR
#endif
#ifdef ITEMID_WEIGHT
#undef ITEMID_WEIGHT
#endif
#ifdef ITEMID_POSTURE
#undef ITEMID_POSTURE
#endif
#ifdef ITEMID_SHADOWED
#undef ITEMID_SHADOWED
#endif
#ifdef ITEMID_CONTOUR
#undef ITEMID_CONTOUR
#endif
#ifdef ITEMID_CROSSEDOUT
#undef ITEMID_CROSSEDOUT
#endif
#ifdef ITEMID_UNDERLINE
#undef ITEMID_UNDERLINE
#endif
#ifdef ITEMID_LANGUAGE
#undef ITEMID_LANGUAGE
#endif
#ifdef ITEMID_ADJUST
#undef ITEMID_ADJUST
#endif
#ifdef ITEMID_WORDLINEMODE
#undef ITEMID_WORDLINEMODE
#endif
#ifdef ITEMID_CHARSETCOLOR
#undef ITEMID_CHARSETCOLOR
#endif
#ifdef ITEMID_SEARCH
#undef ITEMID_SEARCH
#endif
#ifdef ITEMID_BRUSH
#undef ITEMID_BRUSH
#endif

// Zeichenattribute....
#define ITEMID_FONT 			EE_CHAR_FONTINFO
#define ITEMID_POSTURE			EE_CHAR_ITALIC
#define ITEMID_WEIGHT			EE_CHAR_WEIGHT
#define ITEMID_SHADOWED			EE_CHAR_SHADOW
#define ITEMID_CONTOUR			EE_CHAR_OUTLINE
#define ITEMID_CROSSEDOUT		EE_CHAR_STRIKEOUT
#define ITEMID_UNDERLINE		EE_CHAR_UNDERLINE
#define ITEMID_FONTHEIGHT		EE_CHAR_FONTHEIGHT
#define ITEMID_CHARSCALE_W		EE_CHAR_FONTWIDTH
#define ITEMID_COLOR 			EE_CHAR_COLOR
#define ITEMID_WORDLINEMODE		EE_CHAR_WLM
#define ITEMID_PROPSIZE			0
#define ITEMID_CHARSETCOLOR		0	// EE_FEATURE_NOTCONV
#define ITEMID_CASEMAP			0
#define ITEMID_LANGUAGE			0
#define ITEMID_ESCAPEMENT		EE_CHAR_ESCAPEMENT
#define ITEMID_NOLINEBREAK		0
#define ITEMID_NOHYPHENHERE		0
#define ITEMID_AUTOKERN			EE_CHAR_PAIRKERNING
#define ITEMID_KERNING			EE_CHAR_KERNING
#define ITEMID_EMPHASISMARK		EE_CHAR_EMPHASISMARK
#define ITEMID_CHARRELIEF		EE_CHAR_RELIEF

// Absatzattribute
#define ITEMID_SCRIPTSPACE		EE_PARA_ASIANCJKSPACING
#define ITEMID_ADJUST			EE_PARA_JUST
#define ITEMID_LINESPACING		EE_PARA_SBL
#define ITEMID_WIDOWS			0
#define ITEMID_ORPHANS			0
#define ITEMID_HYPHENZONE		0

#define ITEMID_TABSTOP			EE_PARA_TABS

#define ITEMID_PAPERBIN			0
#define ITEMID_LRSPACE			EE_PARA_LRSPACE
#define ITEMID_ULSPACE			EE_PARA_ULSPACE
#define ITEMID_FRAMEDIR         EE_PARA_WRITINGDIR
#define ITEMID_PRINT			0
#define ITEMID_OPAQUE			0
#define ITEMID_PROTECT			0
#define ITEMID_BACKGROUND		0
#define ITEMID_SHADOW			0
#define ITEMID_MACRO			0
#define ITEMID_BOX				0
#define ITEMID_BOXINFO			0
#define ITEMID_BRUSH			0

#define ITEMID_FIELD			EE_FEATURE_FIELD

#define ITEMID_SEARCH			0

}//end of namespace binfilter
#endif //_EEITEMID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

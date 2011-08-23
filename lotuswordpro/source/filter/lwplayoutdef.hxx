/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _LWPLAYOUTDEF_HXX
#define _LWPLAYOUTDEF_HXX

#define	STYLE_AUTOLINK		0x01UL	/* content span's to next container */
#define	STYLE_SELECTABLE	0x02UL	/* selectable container */
#define	STYLE_SHARED		0x04UL	/* is this a global layout style */
#define STYLE_PAINTMARGINSINCOLOR	0x8UL	/* does this layout paint margins */
#define STYLE_PROTECTED		0x10UL	/* is this layout protected */
#define STYLE_LOCAL			0x20UL	/* is local layout */
#define STYLE_NOERRORCHECK	0x40UL	/* is error checking enabled */
#define STYLE_HEADING		0x80UL	/* heading */
#define STYLE_MUTE			0x100UL	/* gray modify menu */
#define STYLE_SCRIPTING		0x200UL	/* scripting enabled */
#define STYLE_CONTENTREPLACEABLE		0x400UL	/* contents can be replaced */
#define STYLE_PRINTABLE		0x800UL	/* contents can be printed */
#define STYLE_COLUMNBALANCE			0x1000UL
#define STYLE_LANDSCAPE				0x2000UL
#define STYLE_COLLAPSIBLE			0x4000UL
#define STYLE_COLLAPSED				0x8000UL
#define STYLE_NOTCOPYABLE			0x10000UL
#define STYLE_PARENTSELECTABLE		0x20000UL
#define STYLE_CONNECTED				0x40000UL
#define STYLE_OVERRIDE				0x80000UL	/* is layout an override */
#define STYLE_SELECTED				0x100000UL
#define STYLE_CENTEREDHORIZONTALLY	0x200000UL
#define STYLE_CREATIONREVISION		0x400000UL
#define STYLE_DELETIONREVISION		0x800000UL
#define STYLE_PAGEBREAK				0x1000000UL
#define STYLE_MODIFIED				0x2000000UL
#define STYLE_INDIRECT				0x4000000UL	/* double click required */
#define STYLE_COMPLEX				0x8000000UL
#define STYLE_CENTEREDVERTICALLY	0x10000000UL
#define STYLE_BOTTOMALIGNED			0x20000000UL
#define STYLE_NOTGROUPABLE			0x40000000UL
#define STYLE_ABSOLUTE				0x80000000UL
// Don't assign these flags from one layout to another
#define STYLE_DONTASSIGN			(STYLE_SHARED)

#define STYLE2_VSCROLLABLE			0x1UL
#define STYLE2_HSCROLLABLE			0x2UL
#define STYLE2_SCROLLABLE			0x3UL
#define STYLE2_MIRROR				0x4UL
#define	STYLE2_PRIVATE				0x8UL
#define	STYLE2_PUBLIC				0x00UL
#define	STYLE2_PRIVATE2				0x10UL
#define	STYLE2_PROTECTED			0x20UL
#define	STYLE2_ACCESSRIGHTS			0x30UL
#define	STYLE2_HONORPROTECTION		0x40UL
#define	STYLE2_CHILDSPANNABLE		0x80UL
#define STYLE2_DEACTIVATED			0x200UL
#define STYLE2_NOCONTENTREFERENCE	0x400UL
#define STYLE2_ADOPTED				0x800UL
#define STYLE2_NOTUSERDELETEABLE	0x1000UL
#define	STYLE2_EXPANDRIGHT			0x2000UL
#define	STYLE2_EXPANDLEFT			0x4000UL
#define	STYLE2_EXPANDUP				0x8000UL
#define	STYLE2_EXPANDDOWN			0x10000UL
#define STYLE2_NOTDIRTYABLE			0x20000UL
#define STYLE2_NONHIERARCHICAL		0x40000UL			// Now not used
#define STYLE2_NOTSIZABLE			0x80000UL
#define STYLE2_MARGINSSAMEASPARENT	0x100000UL
#define STYLE2_DISPLAYANCHORPOINTS	0x200000UL
#define STYLE2_DISPLAYTETHERPOINTS	0x400000UL
#define STYLE2_SNAPTO				0x800000UL
#define STYLE2_NOINTERSECTSIBLINGS	0x1000000UL
#define STYLE2_LOCALCOLUMNINFO		0x2000000UL
#define STYLE2_INOVERRIDE			0x4000000UL
#define STYLE2_FROMSTYLE			0x8000000UL
#define STYLE2_CONTENTFROMSTYLE		0x10000000UL
#define STYLE2_TEMPORARY			0x20000000UL
#define STYLE2_JUSTIFIABLE			0x40000000UL
// Don't assign these flags from one layout to another
#define STYLE2_DONTASSIGN			(STYLE2_DEACTIVATED |			\
                                        STYLE2_NOCONTENTREFERENCE)

#define STYLE3_ORIGINVALID 			0x0001UL
#define STYLE3_WIDTHVALID 			0x0002UL
#define STYLE3_MINHEIGHTVALID 		0x0004UL
#define STYLE3_STYLELAYOUT			0x0008UL
#define STYLE3_LINELOCATION1		0x0010UL
#define STYLE3_LINELOCATION2		0x0020UL
#define STYLE3_OPENED				0x0040UL
#define STYLE3_ORIENTSWITHPARENT	0x0080UL
#define STYLE3_FROZEN				0x0100UL
#define STYLE3_VALUE_VALID			0x0200UL	// moved from CLayout
#define STYLE3_USEPRINTERSETTINGS	0x0400UL	// moved from CLayout
#define STYLE3_ISAMIPROTABLE		0x0800UL	// wrap tables around frames
                                                // like Ami Pro
#define STYLE3_INLINETOMARGIN		0x1000UL	// for Croom

#define OVER_PLACEMENT				0x01UL
#define OVER_SIZE					0x02UL
#define OVER_MARGINS				0x04UL
#define OVER_BORDERS				0x08UL
#define OVER_BACKGROUND				0x10UL
#define OVER_SHADOW					0x20UL
#define OVER_JOIN					0x40UL
#define OVER_COLUMNS				0x80UL
#define OVER_ROTATION				0x100UL
#define OVER_SCALING				0x200UL
#define OVER_NUMERICS				0x400UL
#define OVER_TABS					0x800UL
#define OVER_SCRIPT					0x1000UL
#define OVER_LEADERS				0x2000UL
#define OVER_ORIENTATION			0x4000UL
#define OVER_MISC					0x8000UL
#define OVER_INTERNAL				0x10000UL // User can't access these props.

// the first 4 orientations are left-handed
// and are simply rotated forms of each other
#define TEXT_ORIENT_LRTB			0		// left to right, top to bottom
#define TEXT_ORIENT_TBRL			1		// top to bottom, right to left
#define TEXT_ORIENT_RLBT			2		// right to left, bottom to top
#define TEXT_ORIENT_BTLR			3		// bottom to top, left to right

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: eetext.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:28 $
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
#ifndef _EETEXT_HXX
#define _EETEXT_HXX

// Zeichenattribute....
#define ITEMID_FONT             EE_CHAR_FONTINFO
#define ITEMID_POSTURE          EE_CHAR_ITALIC
#define ITEMID_WEIGHT           EE_CHAR_WEIGHT
#define ITEMID_SHADOWED         EE_CHAR_SHADOW
#define ITEMID_CONTOUR          EE_CHAR_OUTLINE
#define ITEMID_CROSSEDOUT       EE_CHAR_STRIKEOUT
#define ITEMID_UNDERLINE        EE_CHAR_UNDERLINE
#define ITEMID_FONTHEIGHT       EE_CHAR_FONTHEIGHT
#define ITEMID_COLOR            EE_CHAR_COLOR
#ifdef MAC
#ifdef ITEMID_WORDLINEMODE
#undef ITEMID_WORDLINEMODE
#endif
#ifdef ITEMID_CHARSETCOLOR
#undef ITEMID_CHARSETCOLOR
#endif
#endif
// #define ITEMID_WORDLINEMODE  0
#define ITEMID_PROPSIZE         0
// #define ITEMID_CHARSETCOLOR  0
// #define ITEMID_KERNING           0
// #define ITEMID_CASEMAP           0
// #define ITEMID_LANGUAGE      0
#define ITEMID_ESCAPEMENT       EE_CHAR_ESCAPEMENT
#define ITEMID_NOLINEBREAK      0
#define ITEMID_NOHYPHENHERE     0

// Absatzattribute
#define ITEMID_ADJUST           EE_PARA_JUST
#define ITEMID_LINESPACING      EE_PARA_SBL
// #define ITEMID_WIDOWS            0
// #define ITEMID_ORPHANS           0
// #define ITEMID_HYPHENZONE        0

#define ITEMID_TABSTOP          EE_PARA_TABS

#define ITEMID_PAPERBIN         0
#define ITEMID_LRSPACE          EE_PARA_LRSPACE
#define ITEMID_ULSPACE          EE_PARA_ULSPACE
#define ITEMID_PRINT            0
#define ITEMID_OPAQUE           0
#define ITEMID_PROTECT          0
#define ITEMID_BACKGROUND       0
#define ITEMID_SHADOW           0
#define ITEMID_MACRO            0
#define ITEMID_BOX              0
#define ITEMID_BOXINFO          0

#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif

#endif  // EETEXT_HXX



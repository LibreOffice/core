/*************************************************************************
 *
 *  $RCSfile: salfontutils.hxx,v $
 *
 *  $Revision: 1.3 $
 *  last change: $Author: bmahbod $ $Date: 2001-03-26 21:53:16 $
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// =======================================================================

// =======================================================================

#ifndef _SV_SALFONTUTILS_HXX
#define _SV_SALFONTUTILS_HXX

#ifndef _SV_SALDATA_HXX
    #include <saldata.hxx>
#endif

#ifndef _SV_OUTFONT_HXX
    #include <outfont.hxx>
#endif

// =======================================================================

// =======================================================================

static const char *kFontWeightThin1 = "Thin";
static const char *kFontWeightThin2 = "thin";

static const char *kFontWeightLight1 = "Light";
static const char *kFontWeightLight2 = "light";

static const char *kFontWeightBold1 = "Bold";
static const char *kFontWeightBold2 = "bold";

static const char *kFontWeightUltra1 = "Ultra";
static const char *kFontWeightUltra2 = "ultra";

static const char *kFontWeightSemi1 = "Semi";
static const char *kFontWeightSemi2 = "semi";

static const char *kFontWeightNormal1 = "Normal";
static const char *kFontWeightNormal2 = "normal";

static const char *kFontWeightMedium1 = "Medium";
static const char *kFontWeightMedium2 = "medium";

static const char *kFontWeightBlack1 = "Black";
static const char *kFontWeightBlack2 = "black";

static const char *kFontWeightRoman1 = "Roman";
static const char *kFontWeightRoman2 = "roman";

static const char *kFontWeightRegular1 = "Regular";
static const char *kFontWeightRegular2 = "regular";

// =======================================================================

// =======================================================================

DECLARE_LIST( FontList, ImplFontData* );

// =======================================================================

// =======================================================================

FontList *GetMacFontList();

// =======================================================================

// =======================================================================

#endif  // _SV_SALFONTUTILS_HXX


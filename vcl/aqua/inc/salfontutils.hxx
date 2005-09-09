/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salfontutils.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:33:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


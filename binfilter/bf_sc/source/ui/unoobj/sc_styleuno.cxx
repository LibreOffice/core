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

#include "scitems.hxx"
#include <bf_svx/algitem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/numitem.hxx>
#include <bf_svx/pageitem.hxx>
#include <bf_svx/pbinitem.hxx>
#include <bf_svx/unonrule.hxx>
#include <bf_sfx2/printer.hxx>
#include <vcl/virdev.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_svtools/zformat.hxx>
#include <rtl/uuid.h>

#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "styleuno.hxx"
#include "docsh.hxx"
#include "attrib.hxx"
#include "stlpool.hxx"
#include <vcl/svapp.hxx>
#include "miscuno.hxx"
#include "tablink.hxx"
#include "unonames.hxx"
#include "unowids.hxx"
#include "globstr.hrc"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetCellStyleMap()
{
    static SfxItemPropertyMap aCellStyleMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK),	ATTR_BACKGROUND,	&::getCppuType((const sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),	ATTR_PROTECTION,	&::getCppuType((const util::CellProtection*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),	ATTR_FONT_COLOR,	&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),	ATTR_FONT_CONTOUR,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),	ATTR_FONT_CROSSEDOUT,&::getBooleanCppuType(),			0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CEMPHAS),	ATTR_FONT_EMPHASISMARK,&getCppuType((sal_Int16*)0),			0, MID_EMPHASIS },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),	ATTR_FONT,			&::getCppuType((const sal_Int16*)0),			0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFCHARS),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFCHARS),	ATTR_CJK_FONT,		&getCppuType((sal_Int16*)0),			0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFCHARS),	ATTR_CTL_FONT,		&getCppuType((sal_Int16*)0),			0, MID_FONT_CHAR_SET },
        {MAP_CHAR_LEN(SC_UNONAME_CFFAMIL),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFFAMIL),	ATTR_CJK_FONT,		&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFFAMIL),	ATTR_CTL_FONT,		&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),	ATTR_FONT,			&getCppuType((::rtl::OUString*)0),		0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFNAME),	ATTR_CJK_FONT,		&getCppuType((::rtl::OUString*)0),		0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFNAME),	ATTR_CTL_FONT,		&getCppuType((::rtl::OUString*)0),		0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CFPITCH),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFPITCH),	ATTR_CJK_FONT,		&getCppuType((sal_Int16*)0),			0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFPITCH),	ATTR_CTL_FONT,		&getCppuType((sal_Int16*)0),			0, MID_FONT_PITCH },
        {MAP_CHAR_LEN(SC_UNONAME_CFSTYLE),	ATTR_FONT,			&getCppuType((::rtl::OUString*)0),		0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CJK_CFSTYLE),	ATTR_CJK_FONT,		&getCppuType((::rtl::OUString*)0),		0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNO_CTL_CFSTYLE),	ATTR_CTL_FONT,		&getCppuType((::rtl::OUString*)0),		0, MID_FONT_STYLE_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),	ATTR_FONT_HEIGHT,	&::getCppuType((const float*)0),			0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),	ATTR_CJK_FONT_HEIGHT,&::getCppuType((const float*)0),			0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),	ATTR_CTL_FONT_HEIGHT,&::getCppuType((const float*)0),			0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&::getCppuType((const lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,&::getCppuType((const lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,&::getCppuType((const lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),	ATTR_FONT_POSTURE,	&::getCppuType((const awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),	ATTR_CJK_FONT_POSTURE,&::getCppuType((const awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),	ATTR_CTL_FONT_POSTURE,&::getCppuType((const awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CRELIEF),	ATTR_FONT_RELIEF,	&getCppuType((sal_Int16*)0),			0, MID_RELIEF },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),	ATTR_FONT_SHADOWED,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CSTRIKE),	ATTR_FONT_CROSSEDOUT,&getCppuType((sal_Int16*)0),			0, MID_CROSS_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),	ATTR_FONT_UNDERLINE,&::getCppuType((const sal_Int16*)0),			0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLCOL),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int32*)0),			0, MID_UL_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLHAS),	ATTR_FONT_UNDERLINE,&getBooleanCppuType(),					0, MID_UL_HASCOLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),	ATTR_FONT_WEIGHT,	&::getCppuType((const float*)0),			0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),	ATTR_CJK_FONT_WEIGHT,&::getCppuType((const float*)0),			0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),	ATTR_CTL_FONT_WEIGHT,&::getCppuType((const float*)0),			0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CWORDMOD),	ATTR_FONT_WORDLINE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_DISPNAME),	SC_WID_UNO_DISPNAME,&::getCppuType((::rtl::OUString*)0),	beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS),	ATTR_HOR_JUSTIFY,	&::getCppuType((const table::CellHoriJustify*)0),	0, MID_HORJUST_HORJUST },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN),	ATTR_BACKGROUND,	&::getBooleanCppuType(),			0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),		ATTR_LINEBREAK,		&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),	ATTR_VALUE_FORMAT,	&::getCppuType((const sal_Int32*)0),			0, 0 },
//		{MAP_CHAR_LEN(SC_UNONAME_NUMRULES),	SC_WID_UNO_NUMRULES,&getCppuType((const uno::Reference<container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),	ATTR_ORIENTATION,	&::getCppuType((const table::CellOrientation*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PADJUST),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN),	ATTR_MARGIN,		&::getCppuType((const sal_Int32*)0),			0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),	ATTR_INDENT,		&::getCppuType((const sal_Int16*)0),			0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,	&::getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,&::getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHANG),	ATTR_HANGPUNCTUATION,&::getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PLASTADJ),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN),	ATTR_MARGIN,		&::getCppuType((const sal_Int32*)0),			0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN),	ATTR_MARGIN,		&::getCppuType((const sal_Int32*)0),			0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN),	ATTR_MARGIN,		&::getCppuType((const sal_Int32*)0),			0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),	ATTR_ROTATE_VALUE,	&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),	ATTR_ROTATE_MODE,	&::getCppuType((const table::CellVertJustify*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),	ATTR_SHADOW,		&::getCppuType((const table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),	SC_WID_UNO_TBLBORD,	&::getCppuType((const table::TableBorder*)0),		0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS),	ATTR_VER_JUSTIFY,	&::getCppuType((const table::CellVertJustify*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),	ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
        {0,0,0,0}
    };
    return aCellStyleMap_Impl;
}

//	Map mit allen Seitenattributen, incl. Kopf-/Fusszeilenattribute

const SfxItemPropertyMap* lcl_GetPageStyleMap()
{
    static SfxItemPropertyMap aPageStyleMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_PAGE_BACKCOLOR),	ATTR_BACKGROUND,	&::getCppuType((const sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNO_PAGE_GRAPHICFILT),	ATTR_BACKGROUND,	&::getCppuType((const ::rtl::OUString*)0),			0, MID_GRAPHIC_FILTER },
        {MAP_CHAR_LEN(SC_UNO_PAGE_GRAPHICLOC),	ATTR_BACKGROUND,	&::getCppuType((const style::GraphicLocation*)0),	0, MID_GRAPHIC_POSITION },
        {MAP_CHAR_LEN(SC_UNO_PAGE_GRAPHICURL),	ATTR_BACKGROUND,	&::getCppuType((const ::rtl::OUString*)0),			0, MID_GRAPHIC_URL },
        {MAP_CHAR_LEN(SC_UNO_PAGE_BACKTRANS),	ATTR_BACKGROUND,	&::getBooleanCppuType(),			0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_BACKCOLOR),	ATTR_BACKGROUND,	&::getCppuType((const sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNO_PAGE_BORDERDIST),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_BOTTBORDER),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_BOTTBRDDIST),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_BOTTMARGIN),	ATTR_ULSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_CENTERHOR),	ATTR_PAGE_HORCENTER,&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_CENTERVER),	ATTR_PAGE_VERCENTER,&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_DISPNAME),		SC_WID_UNO_DISPNAME,&::getCppuType((::rtl::OUString*)0),	beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FIRSTPAGE),	ATTR_PAGE_FIRSTPAGENO,&::getCppuType((const sal_Int16*)0),		0, 0 },
//
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBACKCOL),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRGRFFILT),	SC_WID_UNO_FOOTERSET,&::getCppuType((const ::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRGRFLOC),	SC_WID_UNO_FOOTERSET,&::getCppuType((const style::GraphicLocation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRGRFURL),	SC_WID_UNO_FOOTERSET,&::getCppuType((const ::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBACKTRAN),	SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRBACKCOL),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBODYDIST),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBRDDIST),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBOTTBOR),	SC_WID_UNO_FOOTERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBOTTBDIS),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRDYNAMIC),	SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRHEIGHT),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRDYNAMIC),	SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRON),		SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRSHARED),	SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRLEFTBOR),	SC_WID_UNO_FOOTERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRLEFTBDIS),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRLEFTMAR),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRON),		SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRRIGHTBOR),	SC_WID_UNO_FOOTERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRRIGHTBDIS),SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRRIGHTMAR),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRSHADOW),	SC_WID_UNO_FOOTERSET,&::getCppuType((const table::ShadowFormat*)0),	0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRSHARED),	SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRTOPBOR),	SC_WID_UNO_FOOTERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRTOPBDIS),	SC_WID_UNO_FOOTERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
//
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBACKCOL),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRGRFFILT),	SC_WID_UNO_HEADERSET,&::getCppuType((const ::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRGRFLOC),	SC_WID_UNO_HEADERSET,&::getCppuType((const style::GraphicLocation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRGRFURL),	SC_WID_UNO_HEADERSET,&::getCppuType((const ::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBACKTRAN),	SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRBACKCOL),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBODYDIST),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBRDDIST),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBOTTBOR),	SC_WID_UNO_HEADERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBOTTBDIS),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRDYNAMIC),	SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRHEIGHT),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRDYNAMIC),	SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRON),		SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRSHARED),	SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRLEFTBOR),	SC_WID_UNO_HEADERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRLEFTBDIS),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRLEFTMAR),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRON),		SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRRIGHTBOR),	SC_WID_UNO_HEADERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRRIGHTBDIS),SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRRIGHTMAR),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRSHADOW),	SC_WID_UNO_HEADERSET,&::getCppuType((const table::ShadowFormat*)0),	0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRSHARED),	SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRTOPBOR),	SC_WID_UNO_HEADERSET,&::getCppuType((const table::BorderLine*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRTOPBDIS),	SC_WID_UNO_HEADERSET,&::getCppuType((const sal_Int32*)0),			0, 0 },
//
        {MAP_CHAR_LEN(SC_UNO_PAGE_HEIGHT),		ATTR_PAGE_SIZE,		&::getCppuType((const sal_Int32*)0),			0, MID_SIZE_HEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_BACKTRANS),	ATTR_BACKGROUND,	&::getBooleanCppuType(),			0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNO_PAGE_LANDSCAPE),	ATTR_PAGE,			&::getBooleanCppuType(),			0, MID_PAGE_ORIENTATION },
        {MAP_CHAR_LEN(SC_UNO_PAGE_LEFTBORDER),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_LEFTBRDDIST),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, LEFT_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_LEFTMARGIN),	ATTR_LRSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_L_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_LEFTFTRCONT),	ATTR_PAGE_FOOTERLEFT,&::getCppuType((const uno::Reference< sheet::XHeaderFooterContent >*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_LEFTHDRCONT),	ATTR_PAGE_HEADERLEFT,&::getCppuType((const uno::Reference< sheet::XHeaderFooterContent >*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_NUMBERTYPE),	ATTR_PAGE,			&::getCppuType((const sal_Int16*)0),			0, MID_PAGE_NUMTYPE },
        {MAP_CHAR_LEN(SC_UNO_PAGE_SCALEVAL),	ATTR_PAGE_SCALE,	&::getCppuType((const sal_Int16*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_SYTLELAYOUT),	ATTR_PAGE,			&::getCppuType((const style::PageStyleLayout*)0),	0, MID_PAGE_LAYOUT },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTANNOT),	ATTR_PAGE_NOTES,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTCHARTS),	ATTR_PAGE_CHARTS,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTDOWN),	ATTR_PAGE_TOPDOWN,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTDRAW),	ATTR_PAGE_DRAWINGS,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTFORMUL),	ATTR_PAGE_FORMULAS,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTGRID),	ATTR_PAGE_GRID,		&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTHEADER),	ATTR_PAGE_HEADERS,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTOBJS),	ATTR_PAGE_OBJECTS,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PRINTZERO),	ATTR_PAGE_NULLVALS,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_PAPERTRAY),	ATTR_PAGE_PAPERBIN,	&::getCppuType((const ::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_RIGHTBORDER),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_RIGHTBRDDIST),ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, RIGHT_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_RIGHTMARGIN),	ATTR_LRSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_R_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_RIGHTFTRCON),	ATTR_PAGE_FOOTERRIGHT,&::getCppuType((const uno::Reference< sheet::XHeaderFooterContent >*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_RIGHTHDRCON),	ATTR_PAGE_HEADERRIGHT,&::getCppuType((const uno::Reference< sheet::XHeaderFooterContent >*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_SCALETOPAG),	ATTR_PAGE_SCALETOPAGES,&::getCppuType((const sal_Int16*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_SHADOWFORM),	ATTR_SHADOW,		&::getCppuType((const table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_SIZE),		ATTR_PAGE_SIZE,		&::getCppuType((const awt::Size*)0),			0, MID_SIZE_SIZE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_TOPBORDER),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_TOPBRDDIST),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, TOP_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_TOPMARGIN),	ATTR_ULSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRBACKTRAN),SC_WID_UNO_FOOTERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRBACKTRAN),SC_WID_UNO_HEADERSET,&::getBooleanCppuType(),						0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),		ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_WIDTH),		ATTR_PAGE_SIZE,		&::getCppuType((const sal_Int32*)0),			0, MID_SIZE_WIDTH | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),		ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
        {0,0,0,0}
    };
    return aPageStyleMap_Impl;
}

//	Map mit Inhalten des Header-Item-Sets

const SfxItemPropertyMap* lcl_GetHeaderStyleMap()
{
    static SfxItemPropertyMap aHeaderStyleMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBACKCOL),	ATTR_BACKGROUND,	&::getCppuType((const sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRGRFFILT),	ATTR_BACKGROUND,	&::getCppuType((const ::rtl::OUString*)0),			0, MID_GRAPHIC_FILTER },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRGRFLOC),	ATTR_BACKGROUND,	&::getCppuType((const style::GraphicLocation*)0),	0, MID_GRAPHIC_POSITION },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRGRFURL),	ATTR_BACKGROUND,	&::getCppuType((const ::rtl::OUString*)0),			0, MID_GRAPHIC_URL },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBACKTRAN),	ATTR_BACKGROUND,	&::getBooleanCppuType(),			0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRBACKCOL),	ATTR_BACKGROUND,	&::getCppuType((const sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBODYDIST),	ATTR_ULSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBRDDIST),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBOTTBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRBOTTBDIS),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRDYNAMIC),	ATTR_PAGE_DYNAMIC,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRHEIGHT),	ATTR_PAGE_SIZE,		&::getCppuType((const sal_Int32*)0),			0, MID_SIZE_HEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRDYNAMIC),	ATTR_PAGE_DYNAMIC,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRON),		ATTR_PAGE_ON,		&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRSHARED),	ATTR_PAGE_SHARED,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRLEFTBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRLEFTBDIS),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, LEFT_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRLEFTMAR),	ATTR_LRSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_L_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRON),		ATTR_PAGE_ON,		&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRRIGHTBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRRIGHTBDIS),ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, RIGHT_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRRIGHTMAR),	ATTR_LRSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_R_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRSHADOW),	ATTR_SHADOW,		&::getCppuType((const table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRSHARED),	ATTR_PAGE_SHARED,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRTOPBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_HDRTOPBDIS),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, TOP_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_HDRBACKTRAN),ATTR_BACKGROUND,	&::getBooleanCppuType(),			0, MID_GRAPHIC_TRANSPARENT },
        {0,0,0,0}
    };
    return aHeaderStyleMap_Impl;
}

//	Map mit Inhalten des Footer-Item-Sets

const SfxItemPropertyMap* lcl_GetFooterStyleMap()
{
    static SfxItemPropertyMap aFooterStyleMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBACKCOL),	ATTR_BACKGROUND,	&::getCppuType((const sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRGRFFILT),	ATTR_BACKGROUND,	&::getCppuType((const ::rtl::OUString*)0),			0, MID_GRAPHIC_FILTER },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRGRFLOC),	ATTR_BACKGROUND,	&::getCppuType((const style::GraphicLocation*)0),	0, MID_GRAPHIC_POSITION },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRGRFURL),	ATTR_BACKGROUND,	&::getCppuType((const ::rtl::OUString*)0),			0, MID_GRAPHIC_URL },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBACKTRAN),	ATTR_BACKGROUND,	&::getBooleanCppuType(),			0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRBACKCOL),	ATTR_BACKGROUND,	&::getCppuType((const sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBODYDIST),	ATTR_ULSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBRDDIST),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBOTTBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRBOTTBDIS),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRDYNAMIC),	ATTR_PAGE_DYNAMIC,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRHEIGHT),	ATTR_PAGE_SIZE,		&::getCppuType((const sal_Int32*)0),			0, MID_SIZE_HEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRDYNAMIC),	ATTR_PAGE_DYNAMIC,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRON),		ATTR_PAGE_ON,		&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRSHARED),	ATTR_PAGE_SHARED,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRLEFTBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRLEFTBDIS),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, LEFT_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRLEFTMAR),	ATTR_LRSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_L_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRON),		ATTR_PAGE_ON,		&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRRIGHTBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRRIGHTBDIS),ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, RIGHT_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRRIGHTMAR),	ATTR_LRSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_R_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRSHADOW),	ATTR_SHADOW,		&::getCppuType((const table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRSHARED),	ATTR_PAGE_SHARED,	&::getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRTOPBOR),	ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0),		0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_PAGE_FTRTOPBDIS),	ATTR_BORDER,		&::getCppuType((const sal_Int32*)0),	0, TOP_BORDER_DISTANCE | CONVERT_TWIPS },
        {MAP_CHAR_LEN(OLD_UNO_PAGE_FTRBACKTRAN),ATTR_BACKGROUND,	&::getBooleanCppuType(),			0, MID_GRAPHIC_TRANSPARENT },
        {0,0,0,0}
    };
    return aFooterStyleMap_Impl;
}


//------------------------------------------------------------------------

//	Index-Access auf die Style-Typen: 0 = Cell, 1 = Page

#define SC_STYLE_FAMILY_COUNT 2

#define SC_FAMILYNAME_CELL	"CellStyles"
#define SC_FAMILYNAME_PAGE	"PageStyles"

static UINT16 aStyleFamilyTypes[SC_STYLE_FAMILY_COUNT] = { SFX_STYLE_FAMILY_PARA, SFX_STYLE_FAMILY_PAGE };

//------------------------------------------------------------------------

//!	diese Funktionen in einen allgemeinen Header verschieben
inline long TwipsToHMM(long nTwips)	{ return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)	{ return (nHMM * 72 + 63) / 127; }

//------------------------------------------------------------------------

#define SCSTYLE_SERVICE			"com.sun.star.style.Style"
#define SCCELLSTYLE_SERVICE		"com.sun.star.style.CellStyle"
#define SCPAGESTYLE_SERVICE		"com.sun.star.style.PageStyle"

SC_SIMPLE_SERVICE_INFO( ScStyleFamiliesObj, "ScStyleFamiliesObj", "com.sun.star.style.StyleFamilies" )
SC_SIMPLE_SERVICE_INFO( ScStyleFamilyObj, "ScStyleFamilyObj", "com.sun.star.style.StyleFamily" )

//------------------------------------------------------------------------

#define SC_PAPERBIN_DEFAULTNAME		"[From printer settings]"

//------------------------------------------------------------------------

//	conversion programmatic <-> display (visible) name
//	currently, the core always has the visible names
//	the api is required to use programmatic names for default styles
//	these programmatic names must never change!

#define SC_STYLE_PROG_STANDARD		"Default"
#define SC_STYLE_PROG_RESULT		"Result"
#define SC_STYLE_PROG_RESULT1		"Result2"
#define SC_STYLE_PROG_HEADLINE		"Heading"
#define SC_STYLE_PROG_HEADLINE1		"Heading1"
#define SC_STYLE_PROG_REPORT		"Report"

struct ScDisplayNameMap
{
    String	aDispName;
    String	aProgName;
};

const ScDisplayNameMap* lcl_GetStyleNameMap( UINT16 nType )
{
    if ( nType == SFX_STYLE_FAMILY_PARA )
    {
        static BOOL bCellMapFilled = FALSE;
        static ScDisplayNameMap aCellMap[6];
        if ( !bCellMapFilled )
        {
            aCellMap[0].aDispName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
            aCellMap[0].aProgName = String::CreateFromAscii( SC_STYLE_PROG_STANDARD );

            aCellMap[1].aDispName = ScGlobal::GetRscString( STR_STYLENAME_RESULT );
            aCellMap[1].aProgName = String::CreateFromAscii( SC_STYLE_PROG_RESULT );

            aCellMap[2].aDispName = ScGlobal::GetRscString( STR_STYLENAME_RESULT1 );
            aCellMap[2].aProgName = String::CreateFromAscii( SC_STYLE_PROG_RESULT1 );

            aCellMap[3].aDispName = ScGlobal::GetRscString( STR_STYLENAME_HEADLINE );
            aCellMap[3].aProgName = String::CreateFromAscii( SC_STYLE_PROG_HEADLINE );

            aCellMap[4].aDispName = ScGlobal::GetRscString( STR_STYLENAME_HEADLINE1 );
            aCellMap[4].aProgName = String::CreateFromAscii( SC_STYLE_PROG_HEADLINE1 );

            //	last entry remains empty

            bCellMapFilled = TRUE;
        }
        return aCellMap;
    }
    else if ( nType == SFX_STYLE_FAMILY_PAGE )
    {
        static BOOL bPageMapFilled = FALSE;
        static ScDisplayNameMap aPageMap[3];
        if ( !bPageMapFilled )
        {
            aPageMap[0].aDispName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
            aPageMap[0].aProgName = String::CreateFromAscii( SC_STYLE_PROG_STANDARD );

            aPageMap[1].aDispName = ScGlobal::GetRscString( STR_STYLENAME_REPORT );
            aPageMap[1].aProgName = String::CreateFromAscii( SC_STYLE_PROG_REPORT );

            //	last entry remains empty

            bPageMapFilled = TRUE;
        }
        return aPageMap;
    }
    DBG_ERROR("invalid family");
    return NULL;
}

//	programmatic name suffix for display names that match other programmatic names
//	is " (user)" including a space

#define SC_SUFFIX_USER		" (user)"
#define SC_SUFFIX_USER_LEN	7

BOOL lcl_EndsWithUser( const String& rString )
{
    const sal_Unicode *pChar = rString.GetBuffer();
    xub_StrLen nLen = rString.Len();
    return nLen >= SC_SUFFIX_USER_LEN &&
           pChar[nLen-7] == ' ' && 
           pChar[nLen-6] == '(' &&
           pChar[nLen-5] == 'u' &&
           pChar[nLen-4] == 's' &&
           pChar[nLen-3] == 'e' &&
           pChar[nLen-2] == 'r' &&
           pChar[nLen-1] == ')';
}

// static
String ScStyleNameConversion::DisplayToProgrammaticName( const String& rDispName, UINT16 nType )
{
    BOOL bDisplayIsProgrammatic = FALSE;

    const ScDisplayNameMap* pNames = lcl_GetStyleNameMap( nType );
    if (pNames)
    {
        do
        {
            if (pNames->aDispName == rDispName)
                return pNames->aProgName;
            else if (pNames->aProgName == rDispName)
                bDisplayIsProgrammatic = TRUE;			// display name matches any programmatic name
        }
        while( (++pNames)->aDispName.Len() );
    }

    if ( bDisplayIsProgrammatic || lcl_EndsWithUser( rDispName ) )
    {
        //	add the (user) suffix if the display name matches any style's programmatic name
        //	or if it already contains the suffix

        String aRet = rDispName;
        aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SC_SUFFIX_USER ) );
        return aRet;
    }

    return rDispName;
}

// static
String ScStyleNameConversion::ProgrammaticToDisplayName( const String& rProgName, UINT16 nType )
{
    if ( lcl_EndsWithUser( rProgName ) )
    {
        //	remove the (user) suffix, don't compare to map entries
        return rProgName.Copy( 0, rProgName.Len() - SC_SUFFIX_USER_LEN );
    }

    const ScDisplayNameMap* pNames = lcl_GetStyleNameMap( nType );
    if (pNames)
    {
        do
        {
            if (pNames->aProgName == rProgName)
                return pNames->aDispName;
        }
        while( (++pNames)->aDispName.Len() );
    }
    return rProgName;
}

//------------------------------------------------------------------------

sal_Bool lcl_AnyTabProtected( ScDocument& rDoc )
{
    USHORT nTabCount = rDoc.GetTableCount();
    for (USHORT i=0; i<nTabCount; i++)
        if (rDoc.IsTabProtected(i))
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------

ScStyleFamiliesObj::ScStyleFamiliesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScStyleFamiliesObj::~ScStyleFamiliesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScStyleFamiliesObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //	Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}

// XStyleFamilies

ScStyleFamilyObj*ScStyleFamiliesObj::GetObjectByType_Impl(UINT16 Type) const
{
    if ( pDocShell )
    {
        if ( Type == SFX_STYLE_FAMILY_PARA )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PARA );
        else if ( Type == SFX_STYLE_FAMILY_PAGE )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PAGE );
    }
    DBG_ERROR("getStyleFamilyByType: keine DocShell oder falscher Typ");
    return NULL;
}

ScStyleFamilyObj* ScStyleFamiliesObj::GetObjectByIndex_Impl(UINT32 nIndex) const
{
    if ( nIndex < SC_STYLE_FAMILY_COUNT )
        return GetObjectByType_Impl(aStyleFamilyTypes[nIndex]);

    return NULL;	// ungueltiger Index
}

ScStyleFamilyObj* ScStyleFamiliesObj::GetObjectByName_Impl(const ::rtl::OUString& aName) const
{
    if ( pDocShell )
    {
        String aNameStr( aName );
        if ( aNameStr.EqualsAscii( SC_FAMILYNAME_CELL ) )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PARA );
        else if ( aNameStr.EqualsAscii( SC_FAMILYNAME_PAGE ) )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PAGE );
    }
    // no assertion - called directly from getByName
    return NULL;
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScStyleFamiliesObj::getCount() throw(uno::RuntimeException)
{
    return SC_STYLE_FAMILY_COUNT;
}

uno::Any SAL_CALL ScStyleFamiliesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNameContainer >  xFamily = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xFamily.is())
        aAny <<= xFamily;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScStyleFamiliesObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::getCppuType((const uno::Reference< container::XNameContainer >*)0);	// muss zu getByIndex passen
}

sal_Bool SAL_CALL ScStyleFamiliesObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNameAccess

uno::Any SAL_CALL ScStyleFamiliesObj::getByName( const ::rtl::OUString& aName )
                    throw(container::NoSuchElementException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNameContainer >  xFamily = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xFamily.is())
        aAny <<= xFamily;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScStyleFamiliesObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< ::rtl::OUString> aNames(SC_STYLE_FAMILY_COUNT);
    ::rtl::OUString* pNames = aNames.getArray();
    pNames[0] = ::rtl::OUString::createFromAscii( SC_FAMILYNAME_CELL );
    pNames[1] = ::rtl::OUString::createFromAscii( SC_FAMILYNAME_PAGE );
    return aNames;
}

sal_Bool SAL_CALL ScStyleFamiliesObj::hasByName( const ::rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameStr( aName );
    return ( aNameStr.EqualsAscii( SC_FAMILYNAME_CELL ) || aNameStr.EqualsAscii( SC_FAMILYNAME_PAGE ) );
}

// style::XStyleLoader

void SAL_CALL ScStyleFamiliesObj::loadStylesFromURL( const ::rtl::OUString& aURL,
                        const uno::Sequence<beans::PropertyValue>& aOptions )
                                throw(io::IOException, uno::RuntimeException)
{
    //!	use aOptions (like Writer)
    //!	set flag to disable filter option dialogs when importing

    String aFilter;		// empty - detect
    String aFiltOpt;
    ScDocumentLoader aLoader( aURL, aFilter, aFiltOpt );

    ScDocShell* pSource = aLoader.GetDocShell();
    if ( pSource && pDocShell )
    {
        //	collect options

        BOOL bLoadReplace = TRUE;			// defaults
        BOOL bLoadCellStyles = TRUE;
        BOOL bLoadPageStyles = TRUE;

        const beans::PropertyValue* pPropArray = aOptions.getConstArray();
        long nPropCount = aOptions.getLength();
        for (long i = 0; i < nPropCount; i++)
        {
            const beans::PropertyValue& rProp = pPropArray[i];
            String aPropName = rProp.Name;

            if (aPropName.EqualsAscii( SC_UNONAME_OVERWSTL ))
                bLoadReplace = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName.EqualsAscii( SC_UNONAME_LOADCELL ))
                bLoadCellStyles = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName.EqualsAscii( SC_UNONAME_LOADPAGE ))
                bLoadPageStyles = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        }

        pDocShell->LoadStylesArgs( *pSource, bLoadReplace, bLoadCellStyles, bLoadPageStyles );
        pDocShell->SetDocumentModified();	// paint is inside LoadStyles
    }
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScStyleFamiliesObj::getStyleLoaderOptions()
                                                throw(uno::RuntimeException)
{
    //	return defaults for options (?)

    uno::Sequence<beans::PropertyValue> aSequence(3);
    beans::PropertyValue* pArray = aSequence.getArray();

    pArray[0].Name = ::rtl::OUString::createFromAscii( SC_UNONAME_OVERWSTL );
    ScUnoHelpFunctions::SetBoolInAny( pArray[0].Value, TRUE );

    pArray[1].Name = ::rtl::OUString::createFromAscii( SC_UNONAME_LOADCELL );
    ScUnoHelpFunctions::SetBoolInAny( pArray[1].Value, TRUE );

    pArray[2].Name = ::rtl::OUString::createFromAscii( SC_UNONAME_LOADPAGE );
    ScUnoHelpFunctions::SetBoolInAny( pArray[2].Value, TRUE );

    return aSequence;
}

//------------------------------------------------------------------------

ScStyleFamilyObj::ScStyleFamilyObj(ScDocShell* pDocSh, SfxStyleFamily eFam) :
    pDocShell( pDocSh ),
    eFamily( eFam )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScStyleFamilyObj::~ScStyleFamilyObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScStyleFamilyObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //	Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}

// XStyleFamily

ScStyleObj* ScStyleFamilyObj::GetObjectByIndex_Impl(UINT32 nIndex)
{
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();

        SfxStyleSheetIterator aIter( pStylePool, eFamily );
        if ( nIndex < aIter.Count() )
        {
            SfxStyleSheetBase* pStyle = aIter[(USHORT)nIndex];
            if ( pStyle )
            {
                String aName = pStyle->GetName();
                return new ScStyleObj( pDocShell, eFamily, aName );
            }
        }
    }
    return NULL;
}

ScStyleObj* ScStyleFamilyObj::GetObjectByName_Impl(const ::rtl::OUString& aName)
{
    if ( pDocShell )
    {
        String aString = aName;

        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
        if ( pStylePool->Find( aString, eFamily ) )
            return new ScStyleObj( pDocShell, eFamily, aString );
    }
    return NULL;
}

void SAL_CALL ScStyleFamilyObj::insertByName( const ::rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bDone = sal_False;
    //	Reflection muss nicht uno::XInterface sein, kann auch irgendein Interface sein...
    uno::Reference< uno::XInterface > xInterface;
    if ( aElement >>= xInterface )
    {
        ScStyleObj* pStyleObj = ScStyleObj::getImplementation( xInterface );
        if ( pStyleObj && pStyleObj->GetFamily() == eFamily &&
                !pStyleObj->IsInserted() )	// noch nicht eingefuegt?
        {
            String aNameStr = ScStyleNameConversion::ProgrammaticToDisplayName( aName, eFamily );

            ScDocument* pDoc = pDocShell->GetDocument();
            ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();

            //!	DocFunc-Funktion??
            //!	Undo ?????????????

            if ( !pStylePool->Find( aNameStr, eFamily ) )	// noch nicht vorhanden
            {
                SfxStyleSheetBase& rStyle = pStylePool->Make( aNameStr, eFamily, SFXSTYLEBIT_USERDEF );

                pStyleObj->InitDoc( pDocShell, aNameStr );	// Objekt kann benutzt werden

                pDocShell->SetDocumentModified();	// verwendet wird der neue Style noch nicht
                bDone = sal_True;
            }
            else
                throw container::ElementExistException();
        }
    }

    if (!bDone)
    {
        //	other errors are handled above
        throw lang::IllegalArgumentException();
    }
}

void SAL_CALL ScStyleFamilyObj::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //!	zusammenfassen?
    removeByName( aName );
    insertByName( aName, aElement );
}

void SAL_CALL ScStyleFamilyObj::removeByName( const ::rtl::OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    BOOL bFound = FALSE;
    if ( pDocShell )
    {
        String aString = ScStyleNameConversion::ProgrammaticToDisplayName( aName, eFamily );

        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();

        //!	DocFunc-Funktion??
        //!	Undo ?????????????

        SfxStyleSheetBase* pStyle = pStylePool->Find( aString, eFamily );
        if (pStyle)
        {
            bFound = TRUE;
            if ( eFamily == SFX_STYLE_FAMILY_PARA )
            {
                // wie ScViewFunc::RemoveStyleSheetInUse
                VirtualDevice aVDev;
                Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
                double nPPTX = aLogic.X() / 1000.0;
                double nPPTY = aLogic.Y() / 1000.0;
                Fraction aZoom(1,1);
                pDoc->StyleSheetChanged( pStyle, sal_False, &aVDev, nPPTX, nPPTY, aZoom, aZoom );
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
                pDocShell->SetDocumentModified();

                pStylePool->Erase( pStyle );

            }
            else
            {
                if ( pDoc->RemovePageStyleInUse( aString ) )
                    pDocShell->PageStyleModified( ScGlobal::GetRscString(STR_STYLENAME_STANDARD), sal_True );

                pStylePool->Erase( pStyle );

                pDocShell->SetDocumentModified();
            }
        }
    }

    if (!bFound)
        throw container::NoSuchElementException();
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScStyleFamilyObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();

        SfxStyleSheetIterator aIter( pStylePool, eFamily );
        return aIter.Count();
    }
    return 0;
}

uno::Any SAL_CALL ScStyleFamilyObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< style::XStyle >  xObj = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xObj.is())
        aAny <<= xObj;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScStyleFamilyObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::getCppuType((const uno::Reference< style::XStyle >*)0);	// muss zu getByIndex passen
}

sal_Bool SAL_CALL ScStyleFamilyObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNameAccess

uno::Any SAL_CALL ScStyleFamilyObj::getByName( const ::rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< style::XStyle > xObj =
        GetObjectByName_Impl( ScStyleNameConversion::ProgrammaticToDisplayName( aName, eFamily ) );
    uno::Any aAny;
    if (xObj.is())
        aAny <<= xObj;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScStyleFamilyObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();

        SfxStyleSheetIterator aIter( pStylePool, eFamily );
        UINT16 nCount = aIter.Count();

        String aName;
        uno::Sequence< ::rtl::OUString> aSeq(nCount);
        ::rtl::OUString* pAry = aSeq.getArray();
        SfxStyleSheetBase* pStyle = aIter.First();
        UINT16 nPos = 0;
        while (pStyle)
        {
            DBG_ASSERT( nPos<nCount, "Anzahl durcheinandergekommen" );
            if (nPos<nCount)
                pAry[nPos++] = ScStyleNameConversion::DisplayToProgrammaticName(
                                    pStyle->GetName(), eFamily );
            pStyle = aIter.Next();
        }
        return aSeq;
    }
    return uno::Sequence< ::rtl::OUString>();
}

sal_Bool SAL_CALL ScStyleFamilyObj::hasByName( const ::rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        String aString = ScStyleNameConversion::ProgrammaticToDisplayName( aName, eFamily );

        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
        if ( pStylePool->Find( aString, eFamily ) )
            return sal_True;
    }
    return sal_False;
}

//------------------------------------------------------------------------

ScStyleObj::ScStyleObj(ScDocShell* pDocSh, SfxStyleFamily eFam, const String& rName) :
    pDocShell( pDocSh ),
    eFamily( eFam ),
    aStyleName( rName ),
    aPropSet( (eFam == SFX_STYLE_FAMILY_PARA) ? lcl_GetCellStyleMap() : lcl_GetPageStyleMap() )
{
    //	pDocShell ist Null, wenn per ServiceProvider erzeugt

    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

void ScStyleObj::InitDoc( ScDocShell* pNewDocSh, const String& rNewName )
{
    if ( pNewDocSh && !pDocShell )
    {
        aStyleName = rNewName;
        pDocShell = pNewDocSh;
        pDocShell->GetDocument()->AddUnoObject(*this);
    }
}

ScStyleObj::~ScStyleObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

// XUnoTunnel

sal_Int64 SAL_CALL ScStyleObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScStyleObj::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScStyleObj* ScStyleObj::getImplementation(
                        const uno::Reference<uno::XInterface> xObj )
{
    ScStyleObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScStyleObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

void ScStyleObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //	Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}

SfxStyleSheetBase* ScStyleObj::GetStyle_Impl()
{
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
        return pStylePool->Find( aStyleName, eFamily );
    }
    return NULL;
}

// style::XStyle

sal_Bool SAL_CALL ScStyleObj::isUserDefined() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return pStyle->IsUserDefined();
    return sal_False;
}

sal_Bool SAL_CALL ScStyleObj::isInUse() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return pStyle->IsUsed();
    return sal_False;
}

::rtl::OUString SAL_CALL ScStyleObj::getParentStyle() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return ScStyleNameConversion::DisplayToProgrammaticName( pStyle->GetParent(), eFamily );
    return ::rtl::OUString();
}

void SAL_CALL ScStyleObj::setParentStyle( const ::rtl::OUString& rParentStyle )
                throw(container::NoSuchElementException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
    {
        //	#70909# cell styles cannot be modified if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( *pDocShell->GetDocument() ) )
            return;			//! exception?

        //!	DocFunc-Funktion??
        //!	Undo ?????????????

        String aString = ScStyleNameConversion::ProgrammaticToDisplayName( rParentStyle, eFamily );
        sal_Bool bOk = pStyle->SetParent( aString );
        if (bOk)
        {
            //	wie bei setPropertyValue

            ScDocument* pDoc = pDocShell->GetDocument();
            if ( eFamily == SFX_STYLE_FAMILY_PARA )
            {
                //	Zeilenhoehen anpassen...

                VirtualDevice aVDev;
                Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
                double nPPTX = aLogic.X() / 1000.0;
                double nPPTY = aLogic.Y() / 1000.0;
                Fraction aZoom(1,1);
                pDoc->StyleSheetChanged( pStyle, sal_False, &aVDev, nPPTX, nPPTY, aZoom, aZoom );

                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
                pDocShell->SetDocumentModified();
            }
            else
            {
                //!	ModifyStyleSheet am Dokument (alte Werte merken)

                pDocShell->PageStyleModified( aStyleName, sal_True );
            }
        }
    }
}

// container::XNamed

::rtl::OUString SAL_CALL ScStyleObj::getName() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return ScStyleNameConversion::DisplayToProgrammaticName( pStyle->GetName(), eFamily );
    return ::rtl::OUString();
}

void SAL_CALL ScStyleObj::setName( const ::rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
    {
        //	#71225# cell styles cannot be renamed if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( *pDocShell->GetDocument() ) )
            return;			//! exception?

        //!	DocFunc-Funktion??
        //!	Undo ?????????????

        String aString = aNewName;
        sal_Bool bOk = pStyle->SetName( aString );
        if (bOk)
        {
            aStyleName = aString;		//!	notify other objects for this style?

            //	Zellvorlagen = 2, Seitenvorlagen = 4
            UINT16 nId = ( eFamily == SFX_STYLE_FAMILY_PARA ) ?
                            SID_STYLE_FAMILY2 : SID_STYLE_FAMILY4;
        }
    }
}

// static
uno::Reference<container::XIndexReplace> ScStyleObj::CreateEmptyNumberingRules()
{
    SvxNumRule aRule( 0, 0, TRUE );			// nothing supported
    return SvxCreateNumRule( &aRule );
}

// beans::XPropertyState

const SfxItemSet* ScStyleObj::GetStyleItemSet_Impl( const String& rPropName,
                                        const SfxItemPropertyMap*& rpResultEntry )
{
    //!	OUString as argument?

    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
    {
        const SfxItemPropertyMap* pMap = NULL;
        if ( eFamily == SFX_STYLE_FAMILY_PAGE )
        {
            pMap = SfxItemPropertyMap::GetByName( lcl_GetHeaderStyleMap(), rPropName );
            if ( pMap )		// only item-wids in header/footer map
            {
                rpResultEntry = pMap;
                return &((const SvxSetItem&)pStyle->GetItemSet().Get(ATTR_PAGE_HEADERSET)).GetItemSet();
            }
            pMap = SfxItemPropertyMap::GetByName( lcl_GetFooterStyleMap(), rPropName );
            if ( pMap )		// only item-wids in header/footer map
            {
                rpResultEntry = pMap;
                return &((const SvxSetItem&)pStyle->GetItemSet().Get(ATTR_PAGE_FOOTERSET)).GetItemSet();
            }
        }
        pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropName );
        if ( pMap && IsScItemWid( pMap->nWID ) )
        {
            rpResultEntry = pMap;
            return &pStyle->GetItemSet();
        }
    }

    rpResultEntry = NULL;
    return NULL;
}

beans::PropertyState SAL_CALL ScStyleObj::getPropertyState( const ::rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    String aString = aPropertyName;

    const SfxItemPropertyMap* pResultEntry = NULL;
    const SfxItemSet* pItemSet = GetStyleItemSet_Impl( aString, pResultEntry );

    if ( pItemSet && pResultEntry )
    {
        USHORT nWhich = pResultEntry->nWID;
        SfxItemState eState = pItemSet->GetItemState( nWhich, sal_False );

        //	if no rotate value is set, look at orientation
        //!	also for a fixed value of 0 (in case orientation is ambiguous)?
        if ( nWhich == ATTR_ROTATE_VALUE && eState == SFX_ITEM_DEFAULT )
            eState = pItemSet->GetItemState( ATTR_ORIENTATION, sal_False );

        if ( eState == SFX_ITEM_SET )
            eRet = beans::PropertyState_DIRECT_VALUE;
        else if ( eState == SFX_ITEM_DEFAULT )
            eRet = beans::PropertyState_DEFAULT_VALUE;
        else if ( eState == SFX_ITEM_DONTCARE )
            eRet = beans::PropertyState_AMBIGUOUS_VALUE;	// kann eigentlich nicht sein...
        else
            DBG_ERROR("unbekannter ItemState");
    }
    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScStyleObj::getPropertyStates(
                            const uno::Sequence< ::rtl::OUString>& aPropertyNames )
                    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    //	duemmliche Default-Implementierung: alles einzeln per getPropertyState holen
    //!	sollte optimiert werden!

    SolarMutexGuard aGuard;
    const ::rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScStyleObj::setPropertyToDefault( const ::rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMap();
    pMap = SfxItemPropertyMap::GetByName( pMap, aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    SetOnePropertyValue( pMap, NULL );
}

uno::Any SAL_CALL ScStyleObj::getPropertyDefault( const ::rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString = aPropertyName;
    uno::Any aAny;

    const SfxItemPropertyMap* pResultEntry = NULL;
    const SfxItemSet* pStyleSet = GetStyleItemSet_Impl( aString, pResultEntry );

    if ( pStyleSet && pResultEntry )
    {
        USHORT nWhich = pResultEntry->nWID;

        //	Default ist Default vom ItemPool, nicht vom Standard-Style,
        //	damit es zu setPropertyToDefault passt
        SfxItemSet aEmptySet( *pStyleSet->GetPool(), pStyleSet->GetRanges() );
        //	#65253# Default-Items mit falscher Slot-ID funktionieren im SfxItemPropertySet3 nicht
        //!	Slot-IDs aendern...
        if ( aEmptySet.GetPool()->GetSlotId(nWhich) == nWhich &&
             aEmptySet.GetItemState(nWhich, sal_False) == SFX_ITEM_DEFAULT )
        {
            aEmptySet.Put( aEmptySet.Get( nWhich ) );
        }
        const SfxItemSet* pItemSet = &aEmptySet;

        switch ( nWhich )		// fuer Item-Spezial-Behandlungen
        {
            case ATTR_VALUE_FORMAT:
                //	default has no language set
                aAny <<= sal_Int32( ((const SfxUInt32Item&)pItemSet->Get(nWhich)).GetValue() );
                break;
            case ATTR_INDENT:
                aAny <<= sal_Int16( TwipsToHMM(((const SfxUInt16Item&)
                                pItemSet->Get(nWhich)).GetValue()) );
                break;
            case ATTR_PAGE_SCALE:
            case ATTR_PAGE_SCALETOPAGES:
            case ATTR_PAGE_FIRSTPAGENO:
                aAny <<= sal_Int16( ((const SfxUInt16Item&)pItemSet->Get(nWhich)).GetValue() );
                break;
            case ATTR_PAGE_CHARTS:
            case ATTR_PAGE_OBJECTS:
            case ATTR_PAGE_DRAWINGS:
                //!	sal_Bool-MID fuer ScViewObjectModeItem definieren?
                aAny <<= sal_Bool( ((const ScViewObjectModeItem&)pItemSet->Get(nWhich)).
                                GetValue() == VOBJ_MODE_SHOW );
                break;
            default:
                aAny = aPropSet.getPropertyValue( *pResultEntry, *pItemSet );
        }
    }
    return aAny;
}

// XMultiPropertySet

void SAL_CALL ScStyleObj::setPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                                const uno::Sequence< uno::Any >& aValues )
                                throw (beans::PropertyVetoException, lang::IllegalArgumentException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount = aPropertyNames.getLength();
    if ( aValues.getLength() != nCount )
        throw lang::IllegalArgumentException();

    if ( nCount )
    {
        const ::rtl::OUString* pNames = aPropertyNames.getConstArray();
        const uno::Any* pValues = aValues.getConstArray();

        const SfxItemPropertyMap* pPropertyMap = aPropSet.getPropertyMap();
        const SfxItemPropertyMap* pMap = pPropertyMap;
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            String aNameString = pNames[i];
            pMap = SfxItemPropertyMap::GetByName( pMap, aNameString );
            SetOnePropertyValue( pMap, &pValues[i] );
            if (!pMap)
                pMap = pPropertyMap;
            else
                pMap++;
        }
    }
}

uno::Sequence<uno::Any> SAL_CALL ScStyleObj::getPropertyValues(
                                    const uno::Sequence< ::rtl::OUString >& aPropertyNames )
                                throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    
    //!	optimize

    sal_Int32 nCount = aPropertyNames.getLength();
    uno::Sequence<uno::Any> aSequence( nCount );
    if ( nCount )
    {
        uno::Any* pValues = aSequence.getArray();
        for (sal_Int32 i=0; i<nCount; i++)
            pValues[i] = getPropertyValue( aPropertyNames[i] );
    }
    return aSequence;
}

void SAL_CALL ScStyleObj::addPropertiesChangeListener( const uno::Sequence< ::rtl::OUString>& aPropertyNames,
                                    const uno::Reference<beans::XPropertiesChangeListener>& xListener )
                                throw (uno::RuntimeException)
{
    // no bound properties
}

void SAL_CALL ScStyleObj::removePropertiesChangeListener(
                                    const uno::Reference<beans::XPropertiesChangeListener>& xListener )
                                throw (uno::RuntimeException)
{
    // no bound properties
}

void SAL_CALL ScStyleObj::firePropertiesChangeEvent( const uno::Sequence< ::rtl::OUString>& aPropertyNames,
                                    const uno::Reference<beans::XPropertiesChangeListener>& xListener )
                                throw (uno::RuntimeException)
{
    // no bound properties
}

// XMultiPropertyStates
// getPropertyStates already defined for XPropertyState

void SAL_CALL ScStyleObj::setAllPropertiesToDefault() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if ( pStyle )
    {
        //	#70909# cell styles cannot be modified if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( *pDocShell->GetDocument() ) )
            throw uno::RuntimeException();

        SfxItemSet& rSet = pStyle->GetItemSet();
        rSet.ClearItem();								// set all items to default

        //!	merge with SetOneProperty

        ScDocument* pDoc = pDocShell->GetDocument();
        if ( eFamily == SFX_STYLE_FAMILY_PARA )
        {
            //	row heights

            VirtualDevice aVDev;
            Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
            double nPPTX = aLogic.X() / 1000.0;
            double nPPTY = aLogic.Y() / 1000.0;
            Fraction aZoom(1,1);
            pDoc->StyleSheetChanged( pStyle, sal_False, &aVDev, nPPTX, nPPTY, aZoom, aZoom );

            pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
            pDocShell->SetDocumentModified();
        }
        else
        {
            pDocShell->PageStyleModified( aStyleName, sal_True );
        }
    }
}

void SAL_CALL ScStyleObj::setPropertiesToDefault( const uno::Sequence< ::rtl::OUString>& aPropertyNames )
                                throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount = aPropertyNames.getLength();
    if ( nCount )
    {
        const ::rtl::OUString* pNames = aPropertyNames.getConstArray();

        const SfxItemPropertyMap* pPropertyMap = aPropSet.getPropertyMap();
        const SfxItemPropertyMap* pMap = pPropertyMap;
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            String aNameString = pNames[i];
            pMap = SfxItemPropertyMap::GetByName( pMap, aNameString );
            SetOnePropertyValue( pMap, NULL );
            if (!pMap)
                pMap = pPropertyMap;
            else
                pMap++;
        }
    }
}

uno::Sequence<uno::Any> SAL_CALL ScStyleObj::getPropertyDefaults(
                                const uno::Sequence< ::rtl::OUString>& aPropertyNames )
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                                uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //!	optimize

    sal_Int32 nCount = aPropertyNames.getLength();
    uno::Sequence<uno::Any> aSequence( nCount );
    if ( nCount )
    {
        uno::Any* pValues = aSequence.getArray();
        for (sal_Int32 i=0; i<nCount; i++)
            pValues[i] = getPropertyDefault( aPropertyNames[i] );
    }
    return aSequence;
}

// beans::XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScStyleObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aPropSet.getPropertySetInfo();
}

void SAL_CALL ScStyleObj::setPropertyValue(
                        const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMap();
    pMap = SfxItemPropertyMap::GetByName( pMap, aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    SetOnePropertyValue( pMap, &aValue );
}

void ScStyleObj::SetOnePropertyValue( const SfxItemPropertyMap* pMap, const uno::Any* pValue )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if ( pStyle && pMap )
    {
        //	#70909# cell styles cannot be modified if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( *pDocShell->GetDocument() ) )
            throw uno::RuntimeException();

        String aString = String::CreateFromAscii( pMap->pName );

        SfxItemSet& rSet = pStyle->GetItemSet();	// direkt im lebenden Style aendern...
        sal_Bool bDone = sal_False;
        if ( eFamily == SFX_STYLE_FAMILY_PAGE )
        {
            const SfxItemPropertyMap* pHeaderMap =
                    SfxItemPropertyMap::GetByName( lcl_GetHeaderStyleMap(), aString );
            if ( pHeaderMap )	// only item-wids in header/footer map
            {
                SvxSetItem aNewHeader( (const SvxSetItem&)rSet.Get(ATTR_PAGE_HEADERSET) );
                if (pValue)
                    aPropSet.setPropertyValue( *pHeaderMap, *pValue, aNewHeader.GetItemSet() );
                else
                    aNewHeader.GetItemSet().ClearItem( pHeaderMap->nWID );
                rSet.Put( aNewHeader );
                bDone = sal_True;
            }
            else
            {
                const SfxItemPropertyMap* pFooterMap =
                        SfxItemPropertyMap::GetByName( lcl_GetFooterStyleMap(), aString );
                if ( pFooterMap )	// only item-wids in header/footer map
                {
                    SvxSetItem aNewFooter( (const SvxSetItem&)rSet.Get(ATTR_PAGE_FOOTERSET) );
                    if (pValue)
                        aPropSet.setPropertyValue( *pFooterMap, *pValue, aNewFooter.GetItemSet() );
                    else
                        aNewFooter.GetItemSet().ClearItem( pFooterMap->nWID );
                    rSet.Put( aNewFooter );
                    bDone = sal_True;
                }
            }
        }
        if (!bDone)
        {
            const SfxItemPropertyMap* pMap =
                    SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), aString );
            if ( pMap && IsScItemWid( pMap->nWID ) )
            {
                if (pValue)
                {
                    switch ( pMap->nWID )		// fuer Item-Spezial-Behandlungen
                    {
                        case ATTR_VALUE_FORMAT:
                            {
                                // #67847# language for number formats
                                SvNumberFormatter* pFormatter =
                                        pDocShell->GetDocument()->GetFormatTable();
                                UINT32 nOldFormat = ((const SfxUInt32Item&)
                                        rSet.Get( ATTR_VALUE_FORMAT )).GetValue();
                                LanguageType eOldLang = ((const SvxLanguageItem&)
                                        rSet.Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                                nOldFormat = pFormatter->
                                        GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );

                                UINT32 nNewFormat;
                                *pValue >>= nNewFormat;
                                rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );

                                const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewFormat );
                                LanguageType eNewLang =
                                    pNewEntry ? pNewEntry->GetLanguage() : LANGUAGE_DONTKNOW;
                                if ( eNewLang != eOldLang && eNewLang != LANGUAGE_DONTKNOW )
                                    rSet.Put( SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );

                                //!	keep default state of number format if only language changed?
                            }
                            break;
                        case ATTR_INDENT:
                            {
                                sal_Int16 nVal;
                                *pValue >>= nVal;
                                rSet.Put( SfxUInt16Item( pMap->nWID, (USHORT)HMMToTwips(nVal) ) );
                            }
                            break;
                        case ATTR_ROTATE_VALUE:
                            {
                                sal_Int32 nRotVal;
                                if ( *pValue >>= nRotVal )
                                {
                                    //	stored value is always between 0 and 360 deg.
                                    nRotVal %= 36000;
                                    if ( nRotVal < 0 )
                                        nRotVal += 36000;

                                    //	always set rotation value and orientation

                                    SvxCellOrientation eOrient = SVX_ORIENTATION_STANDARD;
                                    BOOL bSetOrient = TRUE;
                                
                                    if ( nRotVal == 9000 )
                                        eOrient = SVX_ORIENTATION_BOTTOMTOP;
                                    else if ( nRotVal == 27000 )
                                        eOrient = SVX_ORIENTATION_TOPBOTTOM;
                                    else if ( nRotVal == 0 )
                                    {
                                        // don't overwrite stacked orientation by setting
                                        // rotation to 0

                                        SvxCellOrientation eOld = (SvxCellOrientation)
                                            ((const SvxOrientationItem&)rSet.Get(ATTR_ORIENTATION)).
                                                GetValue();
                                        if ( eOld == SVX_ORIENTATION_STACKED )
                                            bSetOrient = FALSE;
                                    }

                                    rSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, nRotVal ) );
                                    if (bSetOrient)
                                        rSet.Put( SvxOrientationItem( eOrient, ATTR_ORIENTATION ) );
                                }
                            }
                            break;
                        case ATTR_PAGE_SCALE:
                        case ATTR_PAGE_SCALETOPAGES:
                        case ATTR_PAGE_FIRSTPAGENO:
                            {
                                sal_Int16 nVal;
                                *pValue >>= nVal;
                                rSet.Put( SfxUInt16Item( pMap->nWID, nVal ) );
                            }
                            break;
                        case ATTR_PAGE_CHARTS:
                        case ATTR_PAGE_OBJECTS:
                        case ATTR_PAGE_DRAWINGS:
                            {
                                sal_Bool bBool;
                                *pValue >>= bBool;
                                //!	sal_Bool-MID fuer ScViewObjectModeItem definieren?
                                rSet.Put( ScViewObjectModeItem( pMap->nWID,
                                    bBool ? VOBJ_MODE_SHOW : VOBJ_MODE_HIDE ) );
                            }
                            break;
                        case ATTR_PAGE_PAPERBIN:
                            {
                                BYTE nTray = PAPERBIN_PRINTER_SETTINGS;
                                BOOL bFound = FALSE;

                                ::rtl::OUString aName;
                                if ( *pValue >>= aName )
                                {
                                    if ( aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_PAPERBIN_DEFAULTNAME ) ) )
                                        bFound = TRUE;
                                    else
                                    {
                                        Printer* pPrinter = pDocShell->GetPrinter();
                                        if (pPrinter)
                                        {
                                            String aNameStr = aName;
                                            USHORT nCount = pPrinter->GetPaperBinCount();
                                            for (USHORT i=0; i<nCount; i++)
                                                if ( aNameStr == pPrinter->GetPaperBinName(i) )
                                                {
                                                    nTray = (BYTE) i;
                                                    bFound = TRUE;
                                                    break;
                                                }
                                        }
                                    }
                                }
                                if ( bFound )
                                    rSet.Put( SvxPaperBinItem( ATTR_PAGE_PAPERBIN, nTray ) );
                                else
                                    throw lang::IllegalArgumentException();
                            }
                            break;
                        default:
                            //	#65253# Default-Items mit falscher Slot-ID
                            //	funktionieren im SfxItemPropertySet3 nicht
                            //!	Slot-IDs aendern...
                            if ( rSet.GetPool()->GetSlotId(pMap->nWID) == pMap->nWID &&
                                 rSet.GetItemState(pMap->nWID, sal_False) == SFX_ITEM_DEFAULT )
                            {
                                rSet.Put( rSet.Get(pMap->nWID) );
                            }
                            aPropSet.setPropertyValue( *pMap, *pValue, rSet );
                    }
                }
                else
                {
                    rSet.ClearItem( pMap->nWID );
                    // #67847# language for number formats
                    if ( pMap->nWID == ATTR_VALUE_FORMAT )
                        rSet.ClearItem( ATTR_LANGUAGE_FORMAT );

                    //!	for ATTR_ROTATE_VALUE, also reset ATTR_ORIENTATION?
                }
            }
        }

        //!	DocFunc-Funktion??
        //!	Undo ?????????????

        ScDocument* pDoc = pDocShell->GetDocument();
        if ( eFamily == SFX_STYLE_FAMILY_PARA )
        {
            //	Zeilenhoehen anpassen...

            VirtualDevice aVDev;
            Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
            double nPPTX = aLogic.X() / 1000.0;
            double nPPTY = aLogic.Y() / 1000.0;
            Fraction aZoom(1,1);
            pDoc->StyleSheetChanged( pStyle, sal_False, &aVDev, nPPTX, nPPTY, aZoom, aZoom );

            pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
            pDocShell->SetDocumentModified();
        }
        else
        {
            //!	ModifyStyleSheet am Dokument (alte Werte merken)

            pDocShell->PageStyleModified( aStyleName, sal_True );
        }
    }
}

uno::Any SAL_CALL ScStyleObj::getPropertyValue( const ::rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aString = aPropertyName;
    uno::Any aAny;

    const SfxItemPropertyMap* pResultEntry = NULL;
    const SfxItemSet* pItemSet = GetStyleItemSet_Impl( aString, pResultEntry );

    if ( pItemSet && pResultEntry )
    {
        USHORT nWhich = pResultEntry->nWID;

        switch ( nWhich )		// fuer Item-Spezial-Behandlungen
        {
            case ATTR_VALUE_FORMAT:
                if ( pDocShell )
                {
                    UINT32 nOldFormat = ((const SfxUInt32Item&)
                            pItemSet->Get( ATTR_VALUE_FORMAT )).GetValue();
                    LanguageType eOldLang = ((const SvxLanguageItem&)
                            pItemSet->Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                    nOldFormat = pDocShell->GetDocument()->GetFormatTable()->
                            GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );
                    aAny <<= nOldFormat;
                }
                break;
            case ATTR_INDENT:
                aAny <<= sal_Int16( TwipsToHMM(((const SfxUInt16Item&)
                                pItemSet->Get(nWhich)).GetValue()) );
                break;
            case ATTR_ROTATE_VALUE:
                {
                    //	if value is 0 and orientation topbottom or bottomtop,
                    //	adjust value

                    sal_Int32 nRotVal = ((const SfxInt32Item&)
                                pItemSet->Get(ATTR_ROTATE_VALUE)).GetValue();
                    if ( nRotVal == 0 )
                    {
                        SvxCellOrientation eOrient = (SvxCellOrientation)
                                ((const SvxOrientationItem&)pItemSet->Get(ATTR_ORIENTATION)).
                                    GetValue();
                        if ( eOrient == SVX_ORIENTATION_BOTTOMTOP )
                            nRotVal = 9000;
                        else if ( eOrient == SVX_ORIENTATION_TOPBOTTOM )
                            nRotVal = 27000;
                    }
                    aAny <<= nRotVal;
                }
                break;
            case ATTR_PAGE_SCALE:
            case ATTR_PAGE_SCALETOPAGES:
            case ATTR_PAGE_FIRSTPAGENO:
                aAny <<= sal_Int16( ((const SfxUInt16Item&)pItemSet->Get(nWhich)).GetValue() );
                break;
            case ATTR_PAGE_CHARTS:
            case ATTR_PAGE_OBJECTS:
            case ATTR_PAGE_DRAWINGS:
                //!	sal_Bool-MID fuer ScViewObjectModeItem definieren?
                aAny <<= sal_Bool( ((const ScViewObjectModeItem&)pItemSet->
                                Get(nWhich)).GetValue() == VOBJ_MODE_SHOW );
                break;
            case ATTR_PAGE_PAPERBIN:
                {
                    // property PrinterPaperTray is the name of the tray

                    BYTE nValue = ((const SvxPaperBinItem&)pItemSet->Get(nWhich)).GetValue();
                    ::rtl::OUString aName;
                    if ( nValue == PAPERBIN_PRINTER_SETTINGS )
                        aName = ::rtl::OUString::createFromAscii( SC_PAPERBIN_DEFAULTNAME );
                    else
                    {
                        Printer* pPrinter = pDocShell->GetPrinter();
                        if (pPrinter)
                            aName = pPrinter->GetPaperBinName( nValue );
                    }
                    aAny <<= aName;
                }
                break;
            default:
                //	#65253# Default-Items mit falscher Slot-ID
                //	funktionieren im SfxItemPropertySet3 nicht
                //!	Slot-IDs aendern...
                if ( pItemSet->GetPool()->GetSlotId(nWhich) == nWhich &&
                     pItemSet->GetItemState(nWhich, sal_False) == SFX_ITEM_DEFAULT )
                {
                    SfxItemSet aNoEmptySet( *pItemSet );
                    aNoEmptySet.Put( aNoEmptySet.Get( nWhich ) );
                    aAny = aPropSet.getPropertyValue( *pResultEntry, aNoEmptySet );
                }
                else
                    aAny = aPropSet.getPropertyValue( *pResultEntry, *pItemSet );
        }
    }
    else if ( aString.EqualsAscii( SC_UNONAME_DISPNAME ) )		// read-only
    {
        //	core always has the display name
        SfxStyleSheetBase* pStyle = GetStyle_Impl();
        if (pStyle)
            aAny <<= ::rtl::OUString( pStyle->GetName() );
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScStyleObj )

// lang::XServiceInfo

::rtl::OUString SAL_CALL ScStyleObj::getImplementationName() throw(uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "ScStyleObj" );
}

sal_Bool SAL_CALL ScStyleObj::supportsService( const ::rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    BOOL bPage = ( eFamily == SFX_STYLE_FAMILY_PAGE );
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCSTYLE_SERVICE ) ||
           aServiceStr.EqualsAscii( bPage ? SCPAGESTYLE_SERVICE
                                          : SCCELLSTYLE_SERVICE );
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScStyleObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    BOOL bPage = ( eFamily == SFX_STYLE_FAMILY_PAGE );
    uno::Sequence< ::rtl::OUString> aRet(2);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString::createFromAscii( SCSTYLE_SERVICE );
    pArray[1] = ::rtl::OUString::createFromAscii( bPage ? SCPAGESTYLE_SERVICE
                                                      : SCCELLSTYLE_SERVICE );
    return aRet;
}

//------------------------------------------------------------------------


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

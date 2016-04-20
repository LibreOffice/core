/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "scitems.hxx"
#include <editeng/memberids.hrc>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/numitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/pbinitem.hxx>
#include <svx/unomid.hxx>
#include <editeng/unonrule.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/printer.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/intitem.hxx>
#include <svl/zformat.hxx>

#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>

#include "styleuno.hxx"
#include "docsh.hxx"
#include "attrib.hxx"
#include "stlpool.hxx"
#include "docpool.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "tablink.hxx"
#include "unonames.hxx"
#include "unowids.hxx"
#include "globstr.hrc"
#include "cellsuno.hxx"
#include "stylehelper.hxx"

using namespace ::com::sun::star;

static const SfxItemPropertySet* lcl_GetCellStyleSet()
{
    static const SfxItemPropertyMapEntry aCellStyleMap_Impl[] =
    {
        {OUString(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,cppu::UnoType<bool>::get(),                  0, 0 },
        {OUString(SC_UNONAME_BOTTBORDER),ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(),        0, BOTTOM_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_BOTTBORDER2),ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(),        0, BOTTOM_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {OUString(SC_UNONAME_CELLPRO),  ATTR_PROTECTION,    ::cppu::UnoType<util::CellProtection>::get(), 0, 0 },
        {OUString(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    ::cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {OUString(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,cppu::UnoType<bool>::get(),           0, MID_CROSSED_OUT },
        {OUString(SC_UNONAME_CEMPHAS),  ATTR_FONT_EMPHASISMARK,cppu::UnoType<sal_Int16>::get(),         0, MID_EMPHASIS },
        {OUString(SC_UNONAME_CFONT),    ATTR_FONT,          ::cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {OUString(SC_UNONAME_CFCHARS),  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {OUString(SC_UNO_CJK_CFCHARS),  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {OUString(SC_UNO_CTL_CFCHARS),  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {OUString(SC_UNONAME_CFFAMIL),  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {OUString(SC_UNO_CJK_CFFAMIL),  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {OUString(SC_UNO_CTL_CFFAMIL),  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {OUString(SC_UNONAME_CFNAME),   ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNO_CJK_CFNAME),   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNO_CTL_CFNAME),   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {OUString(SC_UNONAME_CFPITCH),  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {OUString(SC_UNO_CJK_CFPITCH),  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {OUString(SC_UNO_CTL_CFPITCH),  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {OUString(SC_UNONAME_CFSTYLE),  ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNO_CJK_CFSTYLE),  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNO_CTL_CFSTYLE),  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {OUString(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   ::cppu::UnoType<float>::get(),            0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNO_CJK_CHEIGHT),  ATTR_CJK_FONT_HEIGHT,::cppu::UnoType<float>::get(),           0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNO_CTL_CHEIGHT),  ATTR_CTL_FONT_HEIGHT,::cppu::UnoType<float>::get(),           0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, ::cppu::UnoType<lang::Locale>::get(),         0, MID_LANG_LOCALE },
        {OUString(SC_UNO_CJK_CLOCAL),   ATTR_CJK_FONT_LANGUAGE,::cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {OUString(SC_UNO_CTL_CLOCAL),   ATTR_CTL_FONT_LANGUAGE,::cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {OUString(SC_UNONAME_COVER),    ATTR_FONT_OVERLINE, ::cppu::UnoType<sal_Int16>::get(),    0, MID_TL_STYLE },
        {OUString(SC_UNONAME_COVRLCOL), ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {OUString(SC_UNONAME_COVRLHAS), ATTR_FONT_OVERLINE, cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {OUString(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  ::cppu::UnoType<awt::FontSlant>::get(),       0, MID_POSTURE },
        {OUString(SC_UNO_CJK_CPOST),    ATTR_CJK_FONT_POSTURE,::cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {OUString(SC_UNO_CTL_CPOST),    ATTR_CTL_FONT_POSTURE,::cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {OUString(SC_UNONAME_CRELIEF),  ATTR_FONT_RELIEF,   cppu::UnoType<sal_Int16>::get(),            0, MID_RELIEF },
        {OUString(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNONAME_CSTRIKE),  ATTR_FONT_CROSSEDOUT,cppu::UnoType<sal_Int16>::get(),           0, MID_CROSS_OUT },
        {OUString(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,::cppu::UnoType<sal_Int16>::get(),    0, MID_TL_STYLE },
        {OUString(SC_UNONAME_CUNDLCOL), ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {OUString(SC_UNONAME_CUNDLHAS), ATTR_FONT_UNDERLINE,cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {OUString(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   ::cppu::UnoType<float>::get(),            0, MID_WEIGHT },
        {OUString(SC_UNO_CJK_CWEIGHT),  ATTR_CJK_FONT_WEIGHT,::cppu::UnoType<float>::get(),           0, MID_WEIGHT },
        {OUString(SC_UNO_CTL_CWEIGHT),  ATTR_CTL_FONT_WEIGHT,::cppu::UnoType<float>::get(),           0, MID_WEIGHT },
        {OUString(SC_UNONAME_CWORDMOD), ATTR_FONT_WORDLINE, cppu::UnoType<bool>::get(),                  0, 0 },
        {OUString(SC_UNONAME_DIAGONAL_BLTR), ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_DIAGONAL_BLTR2), ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_DIAGONAL_TLBR), ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_DIAGONAL_TLBR2), ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_DISPNAME), SC_WID_UNO_DISPNAME,::cppu::UnoType<OUString>::get(),  beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   ::cppu::UnoType<table::CellHoriJustify>::get(),   0, MID_HORJUST_HORJUST },
        {OUString(SC_UNONAME_CELLHJUS_METHOD), ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {OUString(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),            0, MID_GRAPHIC_TRANSPARENT },
        {OUString(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNONAME_LEFTBORDER),ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(),        0, LEFT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_LEFTBORDER2),ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(),        0, LEFT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_NUMFMT),   ATTR_VALUE_FORMAT,  ::cppu::UnoType<sal_Int32>::get(),            0, 0 },
//      {SC_UNONAME_NUMRULES, SC_WID_UNO_NUMRULES,cppu::UnoType<container::XIndexReplace>::get(), 0, 0 },
        {OUString(SC_UNONAME_CELLORI),  ATTR_STACKED,       ::cppu::UnoType<table::CellOrientation>::get(),   0, 0 },
        {OUString(SC_UNONAME_PADJUST),  ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {OUString(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        ::cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNONAME_PINDENT),  ATTR_INDENT,        ::cppu::UnoType<sal_Int16>::get(),            0, 0 }, //! CONVERT_TWIPS
        {OUString(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,   cppu::UnoType<bool>::get(),                    0, 0 },
        {OUString(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,cppu::UnoType<bool>::get(),                   0, 0 },
        {OUString(SC_UNONAME_PISHANG),  ATTR_HANGPUNCTUATION,cppu::UnoType<bool>::get(),                   0, 0 },
        {OUString(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,     cppu::UnoType<bool>::get(),                  0, 0 },
        {OUString(SC_UNONAME_PLASTADJ), ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {OUString(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        ::cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {OUString(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        ::cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {OUString(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        ::cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,      ::cppu::UnoType<table::BorderLine>::get(),        0, RIGHT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_RIGHTBORDER2),ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(),        0, RIGHT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  ::cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {OUString(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   ::cppu::UnoType<sal_Int32>::get(),    0, 0 },
        {OUString(SC_UNONAME_SHADOW),   ATTR_SHADOW,        ::cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_SHRINK_TO_FIT), ATTR_SHRINKTOFIT, cppu::UnoType<bool>::get(),               0, 0 },
        {OUString(SC_UNONAME_TBLBORD),  SC_WID_UNO_TBLBORD, ::cppu::UnoType<table::TableBorder>::get(),       0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_TBLBORD),  SC_WID_UNO_TBLBORD2, ::cppu::UnoType<table::TableBorder2>::get(),       0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNONAME_TOPBORDER),ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, TOP_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_TOPBORDER2),ATTR_BORDER,        ::cppu::UnoType<table::BorderLine2>::get(),        0, TOP_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNONAME_USERDEF),  ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {OUString(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   ::cppu::UnoType<sal_Int32>::get(),    0, 0 },
        {OUString(SC_UNONAME_CELLVJUS_METHOD), ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {OUString(SC_UNONAME_WRITING),  ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        {OUString(SC_UNONAME_HIDDEN),   ATTR_HIDDEN,        cppu::UnoType<sal_Bool>::get(),             0, 0 },
        {OUString(SC_UNONAME_HYPERLINK),  ATTR_HYPERLINK, cppu::UnoType<OUString>::get(),        0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aCellStyleSet_Impl( aCellStyleMap_Impl );
    return &aCellStyleSet_Impl;
}

//  Map mit allen Seitenattributen, incl. Kopf-/Fusszeilenattribute

static const SfxItemPropertySet * lcl_GetPageStyleSet()
{
    static const SfxItemPropertyMapEntry aPageStyleMap_Impl[] =
    {
        {OUString(SC_UNO_PAGE_BACKCOLOR),   ATTR_BACKGROUND,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {OUString(SC_UNO_PAGE_GRAPHICFILT), ATTR_BACKGROUND,    ::cppu::UnoType<OUString>::get(),          0, MID_GRAPHIC_FILTER },
        {OUString(SC_UNO_PAGE_GRAPHICLOC),  ATTR_BACKGROUND,    ::cppu::UnoType<style::GraphicLocation>::get(),   0, MID_GRAPHIC_POSITION },
        {OUString(SC_UNO_PAGE_GRAPHICURL),  ATTR_BACKGROUND,    ::cppu::UnoType<OUString>::get(),          0, MID_GRAPHIC_URL },
        {OUString(SC_UNO_PAGE_BACKTRANS),   ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),            0, MID_GRAPHIC_TRANSPARENT },
        {OUString(OLD_UNO_PAGE_BACKCOLOR),  ATTR_BACKGROUND,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {OUString(SC_UNO_PAGE_BORDERDIST),  ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_BOTTBORDER),  ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, BOTTOM_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_BOTTBRDDIST), ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_BOTTMARGIN),  ATTR_ULSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_LO_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_CENTERHOR),   ATTR_PAGE_HORCENTER,cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_CENTERVER),   ATTR_PAGE_VERCENTER,cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNONAME_DISPNAME),     SC_WID_UNO_DISPNAME,::cppu::UnoType<OUString>::get(),  beans::PropertyAttribute::READONLY, 0 },
        {OUString(SC_UNO_PAGE_FIRSTPAGE),   ATTR_PAGE_FIRSTPAGENO,::cppu::UnoType<sal_Int16>::get(),      0, 0 },

        {OUString(SC_UNO_PAGE_FTRBACKCOL),  SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRGRFFILT),  SC_WID_UNO_FOOTERSET,::cppu::UnoType<OUString>::get(),     0, 0 },
        {OUString(SC_UNO_PAGE_FTRGRFLOC),   SC_WID_UNO_FOOTERSET,::cppu::UnoType<style::GraphicLocation>::get(), 0, 0 },
        {OUString(SC_UNO_PAGE_FTRGRFURL),   SC_WID_UNO_FOOTERSET,::cppu::UnoType<OUString>::get(),     0, 0 },
        {OUString(SC_UNO_PAGE_FTRBACKTRAN), SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(OLD_UNO_PAGE_FTRBACKCOL), SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRBODYDIST), SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRBRDDIST),  SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRBOTTBOR),  SC_WID_UNO_FOOTERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_FTRBOTTBDIS), SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(OLD_UNO_PAGE_FTRDYNAMIC), SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_FTRHEIGHT),   SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRDYNAMIC),  SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_FTRON),       SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_FTRSHARED),   SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_FTRLEFTBOR),  SC_WID_UNO_FOOTERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_FTRLEFTBDIS), SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRLEFTMAR),  SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(OLD_UNO_PAGE_FTRON),      SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_FTRRIGHTBOR), SC_WID_UNO_FOOTERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_FTRRIGHTBDIS),SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRRIGHTMAR), SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_FTRSHADOW),   SC_WID_UNO_FOOTERSET,::cppu::UnoType<table::ShadowFormat>::get(), 0, 0 },
        {OUString(OLD_UNO_PAGE_FTRSHARED),  SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_FTRTOPBOR),   SC_WID_UNO_FOOTERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_FTRTOPBDIS),  SC_WID_UNO_FOOTERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },

        {OUString(SC_UNO_PAGE_HDRBACKCOL),  SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRGRFFILT),  SC_WID_UNO_HEADERSET,::cppu::UnoType<OUString>::get(),     0, 0 },
        {OUString(SC_UNO_PAGE_HDRGRFLOC),   SC_WID_UNO_HEADERSET,::cppu::UnoType<style::GraphicLocation>::get(), 0, 0 },
        {OUString(SC_UNO_PAGE_HDRGRFURL),   SC_WID_UNO_HEADERSET,::cppu::UnoType<OUString>::get(),     0, 0 },
        {OUString(SC_UNO_PAGE_HDRBACKTRAN), SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(OLD_UNO_PAGE_HDRBACKCOL), SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRBODYDIST), SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRBRDDIST),  SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRBOTTBOR),  SC_WID_UNO_HEADERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_HDRBOTTBDIS), SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(OLD_UNO_PAGE_HDRDYNAMIC), SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_HDRHEIGHT),   SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRDYNAMIC),  SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_HDRON),       SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_HDRSHARED),   SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_HDRLEFTBOR),  SC_WID_UNO_HEADERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_HDRLEFTBDIS), SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRLEFTMAR),  SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(OLD_UNO_PAGE_HDRON),      SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_HDRRIGHTBOR), SC_WID_UNO_HEADERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_HDRRIGHTBDIS),SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRRIGHTMAR), SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },
        {OUString(SC_UNO_PAGE_HDRSHADOW),   SC_WID_UNO_HEADERSET,::cppu::UnoType<table::ShadowFormat>::get(), 0, 0 },
        {OUString(OLD_UNO_PAGE_HDRSHARED),  SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNO_PAGE_HDRTOPBOR),   SC_WID_UNO_HEADERSET,::cppu::UnoType<table::BorderLine>::get(),   0, 0 },
        {OUString(SC_UNO_PAGE_HDRTOPBDIS),  SC_WID_UNO_HEADERSET,::cppu::UnoType<sal_Int32>::get(),           0, 0 },

        {OUString(SC_UNO_PAGE_HEIGHT),      ATTR_PAGE_SIZE,     ::cppu::UnoType<sal_Int32>::get(),            0, MID_SIZE_HEIGHT | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_BACKTRANS),  ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),            0, MID_GRAPHIC_TRANSPARENT },
        {OUString(SC_UNO_PAGE_LANDSCAPE),   ATTR_PAGE,          cppu::UnoType<bool>::get(),            0, MID_PAGE_ORIENTATION },
        {OUString(SC_UNO_PAGE_LEFTBORDER),  ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, LEFT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_LEFTBRDDIST), ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, LEFT_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_LEFTMARGIN),  ATTR_LRSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_L_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_LEFTFTRCONT), ATTR_PAGE_FOOTERLEFT,cppu::UnoType<sheet::XHeaderFooterContent>::get(), 0, 0 },
        {OUString(SC_UNO_PAGE_LEFTHDRCONT), ATTR_PAGE_HEADERLEFT,cppu::UnoType<sheet::XHeaderFooterContent>::get(), 0, 0 },
        {OUString(SC_UNO_PAGE_NUMBERTYPE),  ATTR_PAGE,          ::cppu::UnoType<sal_Int16>::get(),            0, MID_PAGE_NUMTYPE },
        {OUString(SC_UNO_PAGE_SCALEVAL),    ATTR_PAGE_SCALE,    ::cppu::UnoType<sal_Int16>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_SYTLELAYOUT), ATTR_PAGE,          ::cppu::UnoType<style::PageStyleLayout>::get(),   0, MID_PAGE_LAYOUT },
        {OUString(SC_UNO_PAGE_PRINTANNOT),  ATTR_PAGE_NOTES,    cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTCHARTS), ATTR_PAGE_CHARTS,   cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTDOWN),   ATTR_PAGE_TOPDOWN,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTDRAW),   ATTR_PAGE_DRAWINGS, cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTFORMUL), ATTR_PAGE_FORMULAS, cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTGRID),   ATTR_PAGE_GRID,     cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTHEADER), ATTR_PAGE_HEADERS,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTOBJS),   ATTR_PAGE_OBJECTS,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PRINTZERO),   ATTR_PAGE_NULLVALS, cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_PAPERTRAY),   ATTR_PAGE_PAPERBIN, ::cppu::UnoType<OUString>::get(),      0, 0 },
        {OUString(SC_UNO_PAGE_RIGHTBORDER), ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, RIGHT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_RIGHTBRDDIST),ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, RIGHT_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_RIGHTMARGIN), ATTR_LRSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_R_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_RIGHTFTRCON), ATTR_PAGE_FOOTERRIGHT,cppu::UnoType<sheet::XHeaderFooterContent>::get(), 0, 0 },
        {OUString(SC_UNO_PAGE_RIGHTHDRCON), ATTR_PAGE_HEADERRIGHT,cppu::UnoType<sheet::XHeaderFooterContent>::get(), 0, 0 },
        {OUString(SC_UNO_PAGE_SCALETOPAG),  ATTR_PAGE_SCALETOPAGES,::cppu::UnoType<sal_Int16>::get(),     0, 0 },
        {OUString(SC_UNO_PAGE_SCALETOX),    ATTR_PAGE_SCALETO,  ::cppu::UnoType<sal_Int16>::get(),        0, 0 },
        {OUString(SC_UNO_PAGE_SCALETOY),    ATTR_PAGE_SCALETO,  ::cppu::UnoType<sal_Int16>::get(),        0, 0 },
        {OUString(SC_UNO_PAGE_SHADOWFORM),  ATTR_SHADOW,        ::cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_SIZE),        ATTR_PAGE_SIZE,     ::cppu::UnoType<awt::Size>::get(),            0, MID_SIZE_SIZE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_TOPBORDER),   ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, TOP_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_TOPBRDDIST),  ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, TOP_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_TOPMARGIN),   ATTR_ULSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_UP_MARGIN | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_FTRBACKTRAN),SC_WID_UNO_FOOTERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(OLD_UNO_PAGE_HDRBACKTRAN),SC_WID_UNO_HEADERSET,cppu::UnoType<bool>::get(),                       0, 0 },
        {OUString(SC_UNONAME_USERDEF),      ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {OUString(SC_UNO_PAGE_WIDTH),       ATTR_PAGE_SIZE,     ::cppu::UnoType<sal_Int32>::get(),            0, MID_SIZE_WIDTH | CONVERT_TWIPS },
        {OUString(SC_UNONAME_WRITING),      ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        {OUString(SC_UNONAME_HIDDEN),       ATTR_HIDDEN,        cppu::UnoType<sal_Bool>::get(),             0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aPageStyleSet_Impl( aPageStyleMap_Impl );
    return &aPageStyleSet_Impl;
}

//  Map mit Inhalten des Header-Item-Sets

static const SfxItemPropertyMap* lcl_GetHeaderStyleMap()
{
    static const SfxItemPropertyMapEntry aHeaderStyleMap_Impl[] =
    {
        {OUString(SC_UNO_PAGE_HDRBACKCOL),  ATTR_BACKGROUND,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {OUString(SC_UNO_PAGE_HDRGRFFILT),  ATTR_BACKGROUND,    ::cppu::UnoType<OUString>::get(),          0, MID_GRAPHIC_FILTER },
        {OUString(SC_UNO_PAGE_HDRGRFLOC),   ATTR_BACKGROUND,    ::cppu::UnoType<style::GraphicLocation>::get(),   0, MID_GRAPHIC_POSITION },
        {OUString(SC_UNO_PAGE_HDRGRFURL),   ATTR_BACKGROUND,    ::cppu::UnoType<OUString>::get(),          0, MID_GRAPHIC_URL },
        {OUString(SC_UNO_PAGE_HDRBACKTRAN), ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),            0, MID_GRAPHIC_TRANSPARENT },
        {OUString(OLD_UNO_PAGE_HDRBACKCOL), ATTR_BACKGROUND,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {OUString(SC_UNO_PAGE_HDRBODYDIST), ATTR_ULSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_LO_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRBRDDIST),  ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRBOTTBOR),  ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, BOTTOM_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRBOTTBDIS), ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_HDRDYNAMIC), ATTR_PAGE_DYNAMIC,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_HDRHEIGHT),   ATTR_PAGE_SIZE,     ::cppu::UnoType<sal_Int32>::get(),            0, MID_SIZE_HEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRDYNAMIC),  ATTR_PAGE_DYNAMIC,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_HDRON),       ATTR_PAGE_ON,       cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_HDRSHARED),   ATTR_PAGE_SHARED,   cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_HDRLEFTBOR),  ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, LEFT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRLEFTBDIS), ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, LEFT_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRLEFTMAR),  ATTR_LRSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_L_MARGIN | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_HDRON),      ATTR_PAGE_ON,       cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_HDRRIGHTBOR), ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, RIGHT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRRIGHTBDIS),ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, RIGHT_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRRIGHTMAR), ATTR_LRSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_R_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRSHADOW),   ATTR_SHADOW,        ::cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_HDRSHARED),  ATTR_PAGE_SHARED,   cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_HDRTOPBOR),   ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, TOP_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_HDRTOPBDIS),  ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, TOP_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_HDRBACKTRAN),ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),            0, MID_GRAPHIC_TRANSPARENT },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertyMap aHeaderStyleMap( aHeaderStyleMap_Impl );
    return &aHeaderStyleMap;
}

//  Map mit Inhalten des Footer-Item-Sets

static const SfxItemPropertyMap* lcl_GetFooterStyleMap()
{
    static const SfxItemPropertyMapEntry aFooterStyleMap_Impl[] =
    {
        {OUString(SC_UNO_PAGE_FTRBACKCOL),  ATTR_BACKGROUND,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {OUString(SC_UNO_PAGE_FTRGRFFILT),  ATTR_BACKGROUND,    ::cppu::UnoType<OUString>::get(),          0, MID_GRAPHIC_FILTER },
        {OUString(SC_UNO_PAGE_FTRGRFLOC),   ATTR_BACKGROUND,    ::cppu::UnoType<style::GraphicLocation>::get(),   0, MID_GRAPHIC_POSITION },
        {OUString(SC_UNO_PAGE_FTRGRFURL),   ATTR_BACKGROUND,    ::cppu::UnoType<OUString>::get(),          0, MID_GRAPHIC_URL },
        {OUString(SC_UNO_PAGE_FTRBACKTRAN), ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),            0, MID_GRAPHIC_TRANSPARENT },
        {OUString(OLD_UNO_PAGE_FTRBACKCOL), ATTR_BACKGROUND,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {OUString(SC_UNO_PAGE_FTRBODYDIST), ATTR_ULSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_UP_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRBRDDIST),  ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRBOTTBOR),  ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, BOTTOM_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRBOTTBDIS), ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_FTRDYNAMIC), ATTR_PAGE_DYNAMIC,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_FTRHEIGHT),   ATTR_PAGE_SIZE,     ::cppu::UnoType<sal_Int32>::get(),            0, MID_SIZE_HEIGHT | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRDYNAMIC),  ATTR_PAGE_DYNAMIC,  cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_FTRON),       ATTR_PAGE_ON,       cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_FTRSHARED),   ATTR_PAGE_SHARED,   cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_FTRLEFTBOR),  ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, LEFT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRLEFTBDIS), ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, LEFT_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRLEFTMAR),  ATTR_LRSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_L_MARGIN | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_FTRON),      ATTR_PAGE_ON,       cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_FTRRIGHTBOR), ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, RIGHT_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRRIGHTBDIS),ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, RIGHT_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRRIGHTMAR), ATTR_LRSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_R_MARGIN | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRSHADOW),   ATTR_SHADOW,        ::cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_FTRSHARED),  ATTR_PAGE_SHARED,   cppu::UnoType<bool>::get(),            0, 0 },
        {OUString(SC_UNO_PAGE_FTRTOPBOR),   ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(),        0, TOP_BORDER | CONVERT_TWIPS },
        {OUString(SC_UNO_PAGE_FTRTOPBDIS),  ATTR_BORDER,        ::cppu::UnoType<sal_Int32>::get(),    0, TOP_BORDER_DISTANCE | CONVERT_TWIPS },
        {OUString(OLD_UNO_PAGE_FTRBACKTRAN),ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),            0, MID_GRAPHIC_TRANSPARENT },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertyMap aFooterStyleMap( aFooterStyleMap_Impl );
    return &aFooterStyleMap;
}

//  Index-Access auf die Style-Typen: 0 = Cell, 1 = Page

#define SC_STYLE_FAMILY_COUNT 2

#define SC_FAMILYNAME_CELL  "CellStyles"
#define SC_FAMILYNAME_PAGE  "PageStyles"

static const sal_uInt16 aStyleFamilyTypes[SC_STYLE_FAMILY_COUNT] = { SFX_STYLE_FAMILY_PARA, SFX_STYLE_FAMILY_PAGE };

using sc::HMMToTwips;
using sc::TwipsToHMM;

#define SCSTYLE_SERVICE         "com.sun.star.style.Style"
#define SCCELLSTYLE_SERVICE     "com.sun.star.style.CellStyle"
#define SCPAGESTYLE_SERVICE     "com.sun.star.style.PageStyle"

SC_SIMPLE_SERVICE_INFO( ScStyleFamiliesObj, "ScStyleFamiliesObj", "com.sun.star.style.StyleFamilies" )
SC_SIMPLE_SERVICE_INFO( ScStyleFamilyObj, "ScStyleFamilyObj", "com.sun.star.style.StyleFamily" )

#define SC_PAPERBIN_DEFAULTNAME     "[From printer settings]"

static bool lcl_AnyTabProtected( ScDocument& rDoc )
{
    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB i=0; i<nTabCount; i++)
        if (rDoc.IsTabProtected(i))
            return true;
    return false;
}

ScStyleFamiliesObj::ScStyleFamiliesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScStyleFamiliesObj::~ScStyleFamiliesObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScStyleFamiliesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

// XStyleFamilies

ScStyleFamilyObj*ScStyleFamiliesObj::GetObjectByType_Impl(sal_uInt16 nType) const
{
    if ( pDocShell )
    {
        if ( nType == SFX_STYLE_FAMILY_PARA )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PARA );
        else if ( nType == SFX_STYLE_FAMILY_PAGE )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PAGE );
    }
    OSL_FAIL("getStyleFamilyByType: keine DocShell oder falscher Typ");
    return nullptr;
}

ScStyleFamilyObj* ScStyleFamiliesObj::GetObjectByIndex_Impl(sal_uInt32 nIndex) const
{
    if ( nIndex < SC_STYLE_FAMILY_COUNT )
        return GetObjectByType_Impl(aStyleFamilyTypes[nIndex]);

    return nullptr;    // ungueltiger Index
}

ScStyleFamilyObj* ScStyleFamiliesObj::GetObjectByName_Impl(const OUString& aName) const
{
    if ( pDocShell )
    {
        OUString aNameStr( aName );
        if ( aNameStr == SC_FAMILYNAME_CELL )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PARA );
        else if ( aNameStr == SC_FAMILYNAME_PAGE )
            return new ScStyleFamilyObj( pDocShell, SFX_STYLE_FAMILY_PAGE );
    }
    // no assertion - called directly from getByName
    return nullptr;
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScStyleFamiliesObj::getCount() throw(uno::RuntimeException, std::exception)
{
    return SC_STYLE_FAMILY_COUNT;
}

uno::Any SAL_CALL ScStyleFamiliesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNameContainer >  xFamily(GetObjectByIndex_Impl(nIndex));
    if (xFamily.is())
        return uno::makeAny(xFamily);
    else
        throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScStyleFamiliesObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<container::XNameContainer>::get();    // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScStyleFamiliesObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNameAccess

uno::Any SAL_CALL ScStyleFamiliesObj::getByName( const OUString& aName )
                    throw(container::NoSuchElementException,
                        lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XNameContainer >  xFamily(GetObjectByName_Impl(aName));
    if (xFamily.is())
        return uno::makeAny(xFamily);
    else
        throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<OUString> SAL_CALL ScStyleFamiliesObj::getElementNames()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aNames(SC_STYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = SC_FAMILYNAME_CELL;
    pNames[1] = SC_FAMILYNAME_PAGE;
    return aNames;
}

sal_Bool SAL_CALL ScStyleFamiliesObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aNameStr( aName );
    return aNameStr == SC_FAMILYNAME_CELL || aNameStr == SC_FAMILYNAME_PAGE;
}

// style::XStyleLoader

void SAL_CALL ScStyleFamiliesObj::loadStylesFromURL( const OUString& aURL,
                        const uno::Sequence<beans::PropertyValue>& aOptions )
                                throw(io::IOException, uno::RuntimeException, std::exception)
{
    //! use aOptions (like Writer)
    //! set flag to disable filter option dialogs when importing

    OUString aFilter;     // empty - detect
    OUString aFiltOpt;
    ScDocumentLoader aLoader( aURL, aFilter, aFiltOpt );

    ScDocShell* pSource = aLoader.GetDocShell();

    loadStylesFromDocShell(pSource, aOptions);
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScStyleFamiliesObj::getStyleLoaderOptions()
                                                throw(uno::RuntimeException, std::exception)
{
    //  return defaults for options (?)

    uno::Sequence<beans::PropertyValue> aSequence(3);
    beans::PropertyValue* pArray = aSequence.getArray();

    pArray[0].Name = SC_UNONAME_OVERWSTL;
    ScUnoHelpFunctions::SetBoolInAny( pArray[0].Value, true );

    pArray[1].Name = SC_UNONAME_LOADCELL;
    ScUnoHelpFunctions::SetBoolInAny( pArray[1].Value, true );

    pArray[2].Name = SC_UNONAME_LOADPAGE;
    ScUnoHelpFunctions::SetBoolInAny( pArray[2].Value, true );

    return aSequence;
}

// style::XStyleLoader2

void SAL_CALL ScStyleFamiliesObj::loadStylesFromDocument( const uno::Reference < lang::XComponent > & aSourceComponent,
                        const uno::Sequence<beans::PropertyValue>& aOptions )
                                throw(io::IOException, uno::RuntimeException, std::exception)
{
   // Source document docShell
   if ( !aSourceComponent.is() )
        throw uno::RuntimeException();

   ScDocShell* pDocShellSrc = dynamic_cast<ScDocShell*> (SfxObjectShell::GetShellFromComponent(aSourceComponent));

   loadStylesFromDocShell(pDocShellSrc, aOptions);
}

// private

void ScStyleFamiliesObj::loadStylesFromDocShell( ScDocShell* pSource,
                        const uno::Sequence<beans::PropertyValue>& aOptions )
                                throw(io::IOException, uno::RuntimeException, std::exception)
{

    if ( pSource && pDocShell )
    {
        //  collect options

        bool bLoadReplace = true;           // defaults
        bool bLoadCellStyles = true;
        bool bLoadPageStyles = true;

        const beans::PropertyValue* pPropArray = aOptions.getConstArray();
        long nPropCount = aOptions.getLength();
        for (long i = 0; i < nPropCount; i++)
        {
            const beans::PropertyValue& rProp = pPropArray[i];
            OUString aPropName(rProp.Name);

            if (aPropName == SC_UNONAME_OVERWSTL)
                bLoadReplace = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName == SC_UNONAME_LOADCELL)
                bLoadCellStyles = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName == SC_UNONAME_LOADPAGE)
                bLoadPageStyles = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        }

        pDocShell->LoadStylesArgs( *pSource, bLoadReplace, bLoadCellStyles, bLoadPageStyles );
        pDocShell->SetDocumentModified();   // paint is inside LoadStyles
    }
}

ScStyleFamilyObj::ScStyleFamilyObj(ScDocShell* pDocSh, SfxStyleFamily eFam) :
    pDocShell( pDocSh ),
    eFamily( eFam )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScStyleFamilyObj::~ScStyleFamilyObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScStyleFamilyObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

// XStyleFamily

ScStyleObj* ScStyleFamilyObj::GetObjectByIndex_Impl(sal_uInt32 nIndex)
{
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();

        SfxStyleSheetIterator aIter( pStylePool, eFamily );
        if ( nIndex < aIter.Count() )
        {
            SfxStyleSheetBase* pStyle = aIter[(sal_uInt16)nIndex];
            if ( pStyle )
            {
                return new ScStyleObj( pDocShell, eFamily, pStyle->GetName() );
            }
        }
    }
    return nullptr;
}

ScStyleObj* ScStyleFamilyObj::GetObjectByName_Impl(const OUString& aName)
{
    if ( pDocShell )
    {
        OUString aString(aName);

        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
        if ( pStylePool->Find( aString, eFamily ) )
            return new ScStyleObj( pDocShell, eFamily, aString );
    }
    return nullptr;
}

void SAL_CALL ScStyleFamilyObj::insertByName( const OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    //  Reflection muss nicht uno::XInterface sein, kann auch irgendein Interface sein...
    uno::Reference< uno::XInterface > xInterface(aElement, uno::UNO_QUERY);
    if ( xInterface.is() )
    {
        ScStyleObj* pStyleObj = ScStyleObj::getImplementation( xInterface );
        if ( pStyleObj && pStyleObj->GetFamily() == eFamily &&
                !pStyleObj->IsInserted() )  // noch nicht eingefuegt?
        {
            OUString aNameStr(ScStyleNameConversion::ProgrammaticToDisplayName( aName, sal::static_int_cast<sal_uInt16>(eFamily) ));

            ScDocument& rDoc = pDocShell->GetDocument();
            ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();

            //! DocFunc-Funktion??
            //! Undo ?????????????

            if ( !pStylePool->Find( aNameStr, eFamily ) )   // noch nicht vorhanden
            {
                (void)pStylePool->Make( aNameStr, eFamily, SFXSTYLEBIT_USERDEF );

                if ( eFamily == SFX_STYLE_FAMILY_PARA && !rDoc.IsImportingXML() )
                    rDoc.GetPool()->CellStyleCreated( aNameStr, &rDoc );

                pStyleObj->InitDoc( pDocShell, aNameStr );  // Objekt kann benutzt werden

                if (!rDoc.IsImportingXML())
                    pDocShell->SetDocumentModified();   // verwendet wird der neue Style noch nicht
                bDone = true;
            }
            else
                throw container::ElementExistException();
        }
    }

    if (!bDone)
    {
        //  other errors are handled above
        throw lang::IllegalArgumentException();
    }
}

void SAL_CALL ScStyleFamilyObj::replaceByName( const OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    //! zusammenfassen?
    removeByName( aName );
    insertByName( aName, aElement );
}

void SAL_CALL ScStyleFamilyObj::removeByName( const OUString& aName )
                                throw(container::NoSuchElementException,
                                      lang::WrappedTargetException,
                                      uno::RuntimeException,
                                      std::exception)
{
    SolarMutexGuard aGuard;
    bool bFound = false;
    if ( pDocShell )
    {
        OUString aString(ScStyleNameConversion::ProgrammaticToDisplayName( aName, sal::static_int_cast<sal_uInt16>(eFamily) ));

        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();

        //! DocFunc-Funktion??
        //! Undo ?????????????

        SfxStyleSheetBase* pStyle = pStylePool->Find( aString, eFamily );
        if (pStyle)
        {
            bFound = true;
            if ( eFamily == SFX_STYLE_FAMILY_PARA )
            {
                // wie ScViewFunc::RemoveStyleSheetInUse
                ScopedVclPtrInstance< VirtualDevice > pVDev;
                Point aLogic = pVDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
                double nPPTX = aLogic.X() / 1000.0;
                double nPPTY = aLogic.Y() / 1000.0;
                Fraction aZoom(1,1);
                rDoc.StyleSheetChanged( pStyle, false, pVDev, nPPTX, nPPTY, aZoom, aZoom );
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
                pDocShell->SetDocumentModified();

                pStylePool->Remove( pStyle );

                //! InvalidateAttribs();        // Bindings-Invalidate
            }
            else
            {
                if ( rDoc.RemovePageStyleInUse( aString ) )
                    pDocShell->PageStyleModified( ScGlobal::GetRscString(STR_STYLENAME_STANDARD), true );

                pStylePool->Remove( pStyle );

                SfxBindings* pBindings = pDocShell->GetViewBindings();
                if (pBindings)
                    pBindings->Invalidate( SID_STYLE_FAMILY4 );
                pDocShell->SetDocumentModified();
            }
        }
    }

    if (!bFound)
        throw container::NoSuchElementException();
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScStyleFamilyObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();

        SfxStyleSheetIterator aIter( pStylePool, eFamily );
        return aIter.Count();
    }
    return 0;
}

uno::Any SAL_CALL ScStyleFamilyObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< style::XStyle >  xObj(GetObjectByIndex_Impl(nIndex));
    if (xObj.is())
        return uno::makeAny(xObj);
    else
        throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScStyleFamilyObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<style::XStyle>::get();    // muss zu getByIndex passen
}

sal_Bool SAL_CALL ScStyleFamilyObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// container::XNameAccess

uno::Any SAL_CALL ScStyleFamilyObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< style::XStyle > xObj(
        GetObjectByName_Impl( ScStyleNameConversion::ProgrammaticToDisplayName( aName, sal::static_int_cast<sal_uInt16>(eFamily) ) ));
    if (xObj.is())
        return uno::makeAny(xObj);
    else
        throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<OUString> SAL_CALL ScStyleFamilyObj::getElementNames()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();

        SfxStyleSheetIterator aIter( pStylePool, eFamily );
        sal_uInt16 nCount = aIter.Count();

        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();
        SfxStyleSheetBase* pStyle = aIter.First();
        sal_uInt16 nPos = 0;
        while (pStyle)
        {
            OSL_ENSURE( nPos<nCount, "Anzahl durcheinandergekommen" );
            if (nPos<nCount)
                pAry[nPos++] = ScStyleNameConversion::DisplayToProgrammaticName(
                                    pStyle->GetName(), sal::static_int_cast<sal_uInt16>(eFamily) );
            pStyle = aIter.Next();
        }
        return aSeq;
    }
    return uno::Sequence<OUString>();
}

sal_Bool SAL_CALL ScStyleFamilyObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        OUString aString(ScStyleNameConversion::ProgrammaticToDisplayName( aName, sal::static_int_cast<sal_uInt16>(eFamily) ));

        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
        if ( pStylePool->Find( aString, eFamily ) )
            return true;
    }
    return false;
}

// XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL ScStyleFamilyObj::getPropertySetInfo(  ) throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL ScStyleFamilyObj::setPropertyValue( const OUString&, const uno::Any& ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

uno::Any SAL_CALL ScStyleFamilyObj::getPropertyValue( const OUString& sPropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    uno::Any aRet;

    if ( sPropertyName == "DisplayName" )
    {
        SolarMutexGuard aGuard;
        sal_uInt32 nResId = 0;
        switch ( eFamily )
        {
            case SFX_STYLE_FAMILY_PARA:
                nResId = STR_STYLE_FAMILY_CELL; break;
            case SFX_STYLE_FAMILY_PAGE:
                nResId = STR_STYLE_FAMILY_PAGE; break;
            default:
                OSL_FAIL( "ScStyleFamilyObj::getPropertyValue(): invalid family" );
        }
        if ( nResId > 0 )
        {
            OUString sDisplayName( ScGlobal::GetRscString( static_cast< sal_uInt16 >( nResId ) ) );
            aRet = uno::makeAny( sDisplayName );
        }
    }
    else
    {
        throw beans::UnknownPropertyException( "unknown property: " + sPropertyName, static_cast<OWeakObject *>(this) );
    }

    return aRet;
}

void SAL_CALL ScStyleFamilyObj::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL ScStyleFamilyObj::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL ScStyleFamilyObj::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL ScStyleFamilyObj::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

//  Default-ctor wird fuer die Reflection gebraucht

ScStyleObj::ScStyleObj(ScDocShell* pDocSh, SfxStyleFamily eFam, const OUString& rName) :
    pPropSet( (eFam == SFX_STYLE_FAMILY_PARA) ? lcl_GetCellStyleSet() : lcl_GetPageStyleSet() ),
    pDocShell( pDocSh ),
    eFamily( eFam ),
    aStyleName( rName )
{
    //  pDocShell ist Null, wenn per ServiceProvider erzeugt

    if (pDocShell)
        pDocShell->GetDocument().AddUnoObject(*this);
}

void ScStyleObj::InitDoc( ScDocShell* pNewDocSh, const OUString& rNewName )
{
    if ( pNewDocSh && !pDocShell )
    {
        aStyleName = rNewName;
        pDocShell = pNewDocSh;
        pDocShell->GetDocument().AddUnoObject(*this);
    }
}

ScStyleObj::~ScStyleObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

// XUnoTunnel

sal_Int64 SAL_CALL ScStyleObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScStyleObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScStyleObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScStyleObj::getUnoTunnelId()
{
    return theScStyleObjUnoTunnelId::get().getSeq();
}

ScStyleObj* ScStyleObj::getImplementation(const uno::Reference<uno::XInterface>& rObj)
{
    ScStyleObj* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScStyleObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

void ScStyleObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

SfxStyleSheetBase* ScStyleObj::GetStyle_Impl()
{
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
        return pStylePool->Find( aStyleName, eFamily );
    }
    return nullptr;
}

// style::XStyle

sal_Bool SAL_CALL ScStyleObj::isUserDefined() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return pStyle->IsUserDefined();
    return false;
}

sal_Bool SAL_CALL ScStyleObj::isInUse() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return pStyle->IsUsed();
    return false;
}

OUString SAL_CALL ScStyleObj::getParentStyle() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return ScStyleNameConversion::DisplayToProgrammaticName( pStyle->GetParent(), sal::static_int_cast<sal_uInt16>(eFamily) );
    return OUString();
}

void SAL_CALL ScStyleObj::setParentStyle( const OUString& rParentStyle )
    throw(container::NoSuchElementException,
          uno::RuntimeException,
          std::exception)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
    {
        //  cell styles cannot be modified if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( pDocShell->GetDocument() ) )
            return;         //! exception?

        //! DocFunc-Funktion??
        //! Undo ?????????????

        OUString aString(ScStyleNameConversion::ProgrammaticToDisplayName( rParentStyle, sal::static_int_cast<sal_uInt16>(eFamily) ));
        bool bOk = pStyle->SetParent( aString );
        if (bOk)
        {
            //  wie bei setPropertyValue

            ScDocument& rDoc = pDocShell->GetDocument();
            if ( eFamily == SFX_STYLE_FAMILY_PARA )
            {
                //  Zeilenhoehen anpassen...

                ScopedVclPtrInstance< VirtualDevice > pVDev;
                Point aLogic = pVDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
                double nPPTX = aLogic.X() / 1000.0;
                double nPPTY = aLogic.Y() / 1000.0;
                Fraction aZoom(1,1);
                rDoc.StyleSheetChanged( pStyle, false, pVDev, nPPTX, nPPTY, aZoom, aZoom );

                if (!rDoc.IsImportingXML())
                {
                    pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
                    pDocShell->SetDocumentModified();
                }
            }
            else
            {
                //! ModifyStyleSheet am Dokument (alte Werte merken)

                pDocShell->PageStyleModified( aStyleName, true );
            }
        }
    }
}

// container::XNamed

OUString SAL_CALL ScStyleObj::getName() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
        return ScStyleNameConversion::DisplayToProgrammaticName( pStyle->GetName(), sal::static_int_cast<sal_uInt16>(eFamily) );
    return OUString();
}

void SAL_CALL ScStyleObj::setName( const OUString& aNewName )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
    {
        //  cell styles cannot be renamed if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( pDocShell->GetDocument() ) )
            return;         //! exception?

        //! DocFunc-Funktion??
        //! Undo ?????????????

        OUString aString(aNewName);
        bool bOk = pStyle->SetName( aString );
        if (bOk)
        {
            aStyleName = aString;       //! notify other objects for this style?

            ScDocument& rDoc = pDocShell->GetDocument();
            if ( eFamily == SFX_STYLE_FAMILY_PARA && !rDoc.IsImportingXML() )
                rDoc.GetPool()->CellStyleCreated( aString, &rDoc );

            //  Zellvorlagen = 2, Seitenvorlagen = 4
            sal_uInt16 nId = ( eFamily == SFX_STYLE_FAMILY_PARA ) ?
                            SID_STYLE_FAMILY2 : SID_STYLE_FAMILY4;
            SfxBindings* pBindings = pDocShell->GetViewBindings();
            if (pBindings)
            {
                pBindings->Invalidate( nId );
                pBindings->Invalidate( SID_STYLE_APPLY );
            }
        }
    }
}

uno::Reference<container::XIndexReplace> ScStyleObj::CreateEmptyNumberingRules()
{
    SvxNumRule aRule( SvxNumRuleFlags::NONE, 0, true );         // nothing supported
    return SvxCreateNumRule( &aRule );
}

// beans::XPropertyState

const SfxItemSet* ScStyleObj::GetStyleItemSet_Impl( const OUString& rPropName,
                                        const SfxItemPropertySimpleEntry*& rpResultEntry )
{
    //! OUString as argument?

    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if (pStyle)
    {
        const SfxItemPropertySimpleEntry* pEntry = nullptr;
        if ( eFamily == SFX_STYLE_FAMILY_PAGE )
        {
            pEntry = lcl_GetHeaderStyleMap()->getByName( rPropName );
            if ( pEntry )     // only item-wids in header/footer map
            {
                rpResultEntry = pEntry;
                return &static_cast<const SvxSetItem&>(pStyle->GetItemSet().Get(ATTR_PAGE_HEADERSET)).GetItemSet();
            }
            pEntry = lcl_GetFooterStyleMap()->getByName( rPropName );
            if ( pEntry )      // only item-wids in header/footer map
            {
                rpResultEntry = pEntry;
                return &static_cast<const SvxSetItem&>(pStyle->GetItemSet().Get(ATTR_PAGE_FOOTERSET)).GetItemSet();
            }
        }
        pEntry = pPropSet->getPropertyMap().getByName( rPropName );
        if ( pEntry )
        {
            rpResultEntry = pEntry;
            return &pStyle->GetItemSet();
        }
    }

    rpResultEntry = nullptr;
    return nullptr;
}

beans::PropertyState SAL_CALL ScStyleObj::getPropertyState( const OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;

    const SfxItemPropertySimpleEntry* pResultEntry = nullptr;
    const SfxItemSet* pItemSet = GetStyleItemSet_Impl( aPropertyName, pResultEntry );

    if ( pItemSet && pResultEntry )
    {
        sal_uInt16 nWhich = pResultEntry->nWID;
        if ( nWhich == SC_WID_UNO_TBLBORD || nWhich == SC_WID_UNO_TBLBORD2 )
        {
            nWhich = ATTR_BORDER;
        }
        if ( IsScItemWid( nWhich ) )
        {
            SfxItemState eState = pItemSet->GetItemState( nWhich, false );

//           //  if no rotate value is set, look at orientation
//           //! also for a fixed value of 0 (in case orientation is ambiguous)?
//           if ( nWhich == ATTR_ROTATE_VALUE && eState == SfxItemState::DEFAULT )
//               eState = pItemSet->GetItemState( ATTR_ORIENTATION, sal_False );

            if ( eState == SfxItemState::SET )
                eRet = beans::PropertyState_DIRECT_VALUE;
            else if ( eState == SfxItemState::DEFAULT )
                eRet = beans::PropertyState_DEFAULT_VALUE;
            else if ( eState == SfxItemState::DONTCARE )
                eRet = beans::PropertyState_AMBIGUOUS_VALUE;    // kann eigentlich nicht sein...
            else
            {
                OSL_FAIL("unbekannter ItemState");
            }
        }
    }
    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScStyleObj::getPropertyStates(
                            const uno::Sequence<OUString>& aPropertyNames )
                    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    //  duemmliche Default-Implementierung: alles einzeln per getPropertyState holen
    //! sollte optimiert werden!

    SolarMutexGuard aGuard;
    const OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScStyleObj::setPropertyToDefault( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException,
          std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    SetOnePropertyValue( aPropertyName, pEntry, nullptr );
}

uno::Any SAL_CALL ScStyleObj::getPropertyDefault( const OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                    uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aAny;

    const SfxItemPropertySimpleEntry* pResultEntry = nullptr;
    const SfxItemSet* pStyleSet = GetStyleItemSet_Impl( aPropertyName, pResultEntry );

    if ( pStyleSet && pResultEntry )
    {
        sal_uInt16 nWhich = pResultEntry->nWID;

        if ( IsScItemWid( nWhich ) )
        {
            //  Default ist Default vom ItemPool, nicht vom Standard-Style,
            //  damit es zu setPropertyToDefault passt
            SfxItemSet aEmptySet( *pStyleSet->GetPool(), pStyleSet->GetRanges() );
            //  Default-Items mit falscher Slot-ID funktionieren im SfxItemPropertySet3 nicht
            //! Slot-IDs aendern...
            if ( aEmptySet.GetPool()->GetSlotId(nWhich) == nWhich &&
                 aEmptySet.GetItemState(nWhich, false) == SfxItemState::DEFAULT )
            {
                aEmptySet.Put( aEmptySet.Get( nWhich ) );
            }
            const SfxItemSet* pItemSet = &aEmptySet;

            switch ( nWhich )       // fuer Item-Spezial-Behandlungen
            {
                case ATTR_VALUE_FORMAT:
                    //  default has no language set
                    aAny <<= sal_Int32( static_cast<const SfxUInt32Item&>(pItemSet->Get(nWhich)).GetValue() );
                    break;
                case ATTR_INDENT:
                    aAny <<= sal_Int16( TwipsToHMM(static_cast<const SfxUInt16Item&>(
                                    pItemSet->Get(nWhich)).GetValue()) );
                    break;
                case ATTR_PAGE_SCALE:
                case ATTR_PAGE_SCALETOPAGES:
                case ATTR_PAGE_FIRSTPAGENO:
                    aAny <<= sal_Int16( static_cast<const SfxUInt16Item&>(pItemSet->Get(nWhich)).GetValue() );
                    break;
                case ATTR_PAGE_CHARTS:
                case ATTR_PAGE_OBJECTS:
                case ATTR_PAGE_DRAWINGS:
                    //! sal_Bool-MID fuer ScViewObjectModeItem definieren?
                    aAny <<= static_cast<const ScViewObjectModeItem&>(pItemSet->Get(nWhich)).GetValue() == VOBJ_MODE_SHOW;
                    break;
                case ATTR_PAGE_SCALETO:
                    {
                        const ScPageScaleToItem aItem(static_cast<const ScPageScaleToItem&>(pItemSet->Get(nWhich)));
                        if ( aPropertyName == SC_UNO_PAGE_SCALETOX )
                            aAny = uno::makeAny(static_cast<sal_Int16>(aItem.GetWidth()));
                        else
                            aAny = uno::makeAny(static_cast<sal_Int16>(aItem.GetHeight()));
                    }
                    break;
                default:
                    pPropSet->getPropertyValue( *pResultEntry, *pItemSet, aAny );
            }
        }
        else if ( IsScUnoWid( nWhich ) )
        {
            SfxItemSet aEmptySet( *pStyleSet->GetPool(), pStyleSet->GetRanges() );
            const SfxItemSet* pItemSet = &aEmptySet;
            switch ( nWhich )
            {
                case SC_WID_UNO_TBLBORD:
                case SC_WID_UNO_TBLBORD2:
                    {
                        const SfxPoolItem* pItem = &pItemSet->Get( ATTR_BORDER );
                        if ( pItem )
                        {
                            SvxBoxItem aOuter( *( static_cast<const SvxBoxItem*>( pItem ) ) );
                            SvxBoxInfoItem aInner( ATTR_BORDER_INNER );
                            if (nWhich == SC_WID_UNO_TBLBORD2)
                                ScHelperFunctions::AssignTableBorder2ToAny( aAny, aOuter, aInner, true);
                            else
                                ScHelperFunctions::AssignTableBorderToAny( aAny, aOuter, aInner, true);
                        }
                    }
                    break;
            }
        }
    }
    return aAny;
}

// XMultiPropertySet

void SAL_CALL ScStyleObj::setPropertyValues( const uno::Sequence< OUString >& aPropertyNames,
                                                const uno::Sequence< uno::Any >& aValues )
throw (beans::PropertyVetoException, lang::IllegalArgumentException,
       lang::WrappedTargetException, uno::RuntimeException,
       std::exception)
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount = aPropertyNames.getLength();
    if ( aValues.getLength() != nCount )
        throw lang::IllegalArgumentException();

    if ( nCount )
    {
        const OUString* pNames = aPropertyNames.getConstArray();
        const uno::Any* pValues = aValues.getConstArray();

        const SfxItemPropertyMap& rPropertyMap = pPropSet->getPropertyMap();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            const SfxItemPropertySimpleEntry*  pEntry = rPropertyMap.getByName( pNames[i] );
            SetOnePropertyValue( pNames[i], pEntry, &pValues[i] );
        }
    }
}

uno::Sequence<uno::Any> SAL_CALL ScStyleObj::getPropertyValues(
                                    const uno::Sequence< OUString >& aPropertyNames )
                                throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    //! optimize

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

void SAL_CALL ScStyleObj::addPropertiesChangeListener( const uno::Sequence<OUString>& /* aPropertyNames */,
                                    const uno::Reference<beans::XPropertiesChangeListener>& /* xListener */ )
                                throw (uno::RuntimeException, std::exception)
{
    // no bound properties
}

void SAL_CALL ScStyleObj::removePropertiesChangeListener(
                                    const uno::Reference<beans::XPropertiesChangeListener>& /* xListener */ )
                                throw (uno::RuntimeException, std::exception)
{
    // no bound properties
}

void SAL_CALL ScStyleObj::firePropertiesChangeEvent( const uno::Sequence<OUString>& /* aPropertyNames */,
                                    const uno::Reference<beans::XPropertiesChangeListener>& /* xListener */ )
                                throw (uno::RuntimeException, std::exception)
{
    // no bound properties
}

// XMultiPropertyStates
// getPropertyStates already defined for XPropertyState

void SAL_CALL ScStyleObj::setAllPropertiesToDefault()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if ( pStyle )
    {
        //  cell styles cannot be modified if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( pDocShell->GetDocument() ) )
            throw uno::RuntimeException();

        SfxItemSet& rSet = pStyle->GetItemSet();
        rSet.ClearItem();                               // set all items to default

        //! merge with SetOneProperty

        ScDocument& rDoc = pDocShell->GetDocument();
        if ( eFamily == SFX_STYLE_FAMILY_PARA )
        {
            //  row heights

            ScopedVclPtrInstance< VirtualDevice > pVDev;
            Point aLogic = pVDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
            double nPPTX = aLogic.X() / 1000.0;
            double nPPTY = aLogic.Y() / 1000.0;
            Fraction aZoom(1,1);
            rDoc.StyleSheetChanged( pStyle, false, pVDev, nPPTX, nPPTY, aZoom, aZoom );

            if (!rDoc.IsImportingXML())
            {
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
                pDocShell->SetDocumentModified();
            }
        }
        else
        {
            // #i22448# apply the default BoxInfoItem for page styles again
            // (same content as in ScStyleSheet::GetItemSet, to control the dialog)
            SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
            aBoxInfoItem.SetTable( false );
            aBoxInfoItem.SetDist( true );
            aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
            rSet.Put( aBoxInfoItem );

            pDocShell->PageStyleModified( aStyleName, true );
        }
    }
}

void SAL_CALL ScStyleObj::setPropertiesToDefault( const uno::Sequence<OUString>& aPropertyNames )
    throw (beans::UnknownPropertyException, uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount = aPropertyNames.getLength();
    if ( nCount )
    {
        const OUString* pNames = aPropertyNames.getConstArray();

        const SfxItemPropertyMap& rPropertyMap = pPropSet->getPropertyMap();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            const SfxItemPropertySimpleEntry*  pEntry = rPropertyMap.getByName( pNames[i] );
            SetOnePropertyValue( pNames[i], pEntry, nullptr );
        }
    }
}

uno::Sequence<uno::Any> SAL_CALL ScStyleObj::getPropertyDefaults(
                                const uno::Sequence<OUString>& aPropertyNames )
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                                uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    //! optimize

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
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return pPropSet->getPropertySetInfo();
}

void SAL_CALL ScStyleObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
          lang::IllegalArgumentException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    SetOnePropertyValue( aPropertyName, pEntry, &aValue );
}

void ScStyleObj::SetOnePropertyValue( const OUString& rPropertyName, const SfxItemPropertySimpleEntry* pEntry, const uno::Any* pValue )
                                throw(lang::IllegalArgumentException,
                                      uno::RuntimeException,
                                      std::exception)
{
    SfxStyleSheetBase* pStyle = GetStyle_Impl();
    if ( pStyle && pEntry )
    {
        //  cell styles cannot be modified if any sheet is protected
        if ( eFamily == SFX_STYLE_FAMILY_PARA && lcl_AnyTabProtected( pDocShell->GetDocument() ) )
            throw uno::RuntimeException();

        SfxItemSet& rSet = pStyle->GetItemSet();    // direkt im lebenden Style aendern...
        bool bDone = false;
        if ( eFamily == SFX_STYLE_FAMILY_PAGE )
        {
            if(pEntry->nWID == SC_WID_UNO_HEADERSET)
            {
                const SfxItemPropertySimpleEntry* pHeaderEntry = lcl_GetHeaderStyleMap()->getByName( rPropertyName );
                if ( pHeaderEntry ) // only item-wids in header/footer map
                {
                    SvxSetItem aNewHeader( static_cast<const SvxSetItem&>(rSet.Get(ATTR_PAGE_HEADERSET)) );
                    if (pValue)
                        pPropSet->setPropertyValue( *pHeaderEntry, *pValue, aNewHeader.GetItemSet() );
                    else
                        aNewHeader.GetItemSet().ClearItem( pHeaderEntry->nWID );
                    rSet.Put( aNewHeader );
                    bDone = true;
                }
            }
            else if(pEntry->nWID == SC_WID_UNO_FOOTERSET)
            {
                const SfxItemPropertySimpleEntry* pFooterEntry = lcl_GetFooterStyleMap()->getByName( rPropertyName );
                if ( pFooterEntry ) // only item-wids in header/footer map
                {
                    SvxSetItem aNewFooter( static_cast<const SvxSetItem&>(rSet.Get(ATTR_PAGE_FOOTERSET)) );
                    if (pValue)
                        pPropSet->setPropertyValue( *pFooterEntry, *pValue, aNewFooter.GetItemSet() );
                    else
                        aNewFooter.GetItemSet().ClearItem( pFooterEntry->nWID );
                    rSet.Put( aNewFooter );
                    bDone = true;
                }
            }
        }
        if (!bDone)
        {
            if ( pEntry )
            {
                if ( IsScItemWid( pEntry->nWID ) )
                {
                    if (pValue)
                    {
                        switch ( pEntry->nWID )     // fuer Item-Spezial-Behandlungen
                        {
                            case ATTR_VALUE_FORMAT:
                                {
                                    // language for number formats
                                    SvNumberFormatter* pFormatter =
                                            pDocShell->GetDocument().GetFormatTable();
                                    sal_uInt32 nOldFormat = static_cast<const SfxUInt32Item&>(
                                            rSet.Get( ATTR_VALUE_FORMAT )).GetValue();
                                    LanguageType eOldLang = static_cast<const SvxLanguageItem&>(
                                            rSet.Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                                    nOldFormat = pFormatter->
                                            GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );

                                    sal_uInt32 nNewFormat = 0;
                                    *pValue >>= nNewFormat;
                                    rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );

                                    const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewFormat );
                                    LanguageType eNewLang =
                                        pNewEntry ? pNewEntry->GetLanguage() : LANGUAGE_DONTKNOW;
                                    if ( eNewLang != eOldLang && eNewLang != LANGUAGE_DONTKNOW )
                                        rSet.Put( SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );

                                    //! keep default state of number format if only language changed?
                                }
                                break;
                            case ATTR_INDENT:
                                {
                                    sal_Int16 nVal = 0;
                                    *pValue >>= nVal;
                                    rSet.Put( SfxUInt16Item( pEntry->nWID, (sal_uInt16)HMMToTwips(nVal) ) );
                                }
                                break;
                            case ATTR_ROTATE_VALUE:
                                {
                                    sal_Int32 nRotVal = 0;
                                    if ( *pValue >>= nRotVal )
                                    {
                                        //  stored value is always between 0 and 360 deg.
                                        nRotVal %= 36000;
                                        if ( nRotVal < 0 )
                                            nRotVal += 36000;
                                        rSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, nRotVal ) );
                                    }
                                }
                                break;
                            case ATTR_STACKED:
                                {
                                    table::CellOrientation eOrient;
                                    if( *pValue >>= eOrient )
                                    {
                                        switch( eOrient )
                                        {
                                            case table::CellOrientation_STANDARD:
                                                rSet.Put( SfxBoolItem( ATTR_STACKED, false ) );
                                            break;
                                            case table::CellOrientation_TOPBOTTOM:
                                                rSet.Put( SfxBoolItem( ATTR_STACKED, false ) );
                                                rSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, 27000 ) );
                                            break;
                                            case table::CellOrientation_BOTTOMTOP:
                                                rSet.Put( SfxBoolItem( ATTR_STACKED, false ) );
                                                rSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, 9000 ) );
                                            break;
                                            case table::CellOrientation_STACKED:
                                                rSet.Put( SfxBoolItem( ATTR_STACKED, true ) );
                                            break;
                                            default:
                                            {
                                                // added to avoid warnings
                                            }
                                        }
                                    }
                                }
                                break;
                            case ATTR_PAGE_SCALE:
                            case ATTR_PAGE_SCALETOPAGES:
                                {
                                    rSet.ClearItem(ATTR_PAGE_SCALETOPAGES);
                                    rSet.ClearItem(ATTR_PAGE_SCALE);
                                    rSet.ClearItem(ATTR_PAGE_SCALETO);
                                    sal_Int16 nVal = 0;
                                    *pValue >>= nVal;
                                    rSet.Put( SfxUInt16Item( pEntry->nWID, nVal ) );
                                }
                                break;
                            case ATTR_PAGE_FIRSTPAGENO:
                                {
                                    sal_Int16 nVal = 0;
                                    *pValue >>= nVal;
                                    rSet.Put( SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, nVal ) );
                                }
                                break;
                            case ATTR_PAGE_CHARTS:
                            case ATTR_PAGE_OBJECTS:
                            case ATTR_PAGE_DRAWINGS:
                                {
                                    bool bBool = false;
                                    *pValue >>= bBool;
                                    //! sal_Bool-MID fuer ScViewObjectModeItem definieren?
                                    rSet.Put( ScViewObjectModeItem( pEntry->nWID,
                                        bBool ? VOBJ_MODE_SHOW : VOBJ_MODE_HIDE ) );
                                }
                                break;
                            case ATTR_PAGE_PAPERBIN:
                                {
                                    sal_uInt8 nTray = PAPERBIN_PRINTER_SETTINGS;
                                    bool bFound = false;

                                    OUString aName;
                                    if ( *pValue >>= aName )
                                    {
                                        if ( aName == SC_PAPERBIN_DEFAULTNAME )
                                            bFound = true;
                                        else
                                        {
                                            Printer* pPrinter = pDocShell->GetPrinter();
                                            if (pPrinter)
                                            {
                                                sal_uInt16 nCount = pPrinter->GetPaperBinCount();
                                                for (sal_uInt16 i=0; i<nCount; i++)
                                                    if ( aName == pPrinter->GetPaperBinName(i) )
                                                    {
                                                        nTray = (sal_uInt8) i;
                                                        bFound = true;
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
                            case ATTR_PAGE_SCALETO:
                                {
                                    sal_Int16 nPages = 0;
                                    if (*pValue >>= nPages)
                                    {
                                        ScPageScaleToItem aItem = static_cast<const ScPageScaleToItem&>(rSet.Get(ATTR_PAGE_SCALETO));
                                        if ( rPropertyName == SC_UNO_PAGE_SCALETOX )
                                            aItem.SetWidth(static_cast<sal_uInt16>(nPages));
                                        else
                                            aItem.SetHeight(static_cast<sal_uInt16>(nPages));
                                        rSet.Put( aItem );
                                        rSet.ClearItem(ATTR_PAGE_SCALETOPAGES);
                                        rSet.ClearItem(ATTR_PAGE_SCALE);
                                    }
                                }
                                break;
                            case ATTR_HIDDEN:
                                {
                                    bool bHidden = false;
                                    if ( *pValue >>= bHidden )
                                        pStyle->SetHidden( bHidden );
                                }
                                break;
                            default:
                                //  Default-Items mit falscher Slot-ID
                                //  funktionieren im SfxItemPropertySet3 nicht
                                //! Slot-IDs aendern...
                                if ( rSet.GetPool()->GetSlotId(pEntry->nWID) == pEntry->nWID &&
                                     rSet.GetItemState(pEntry->nWID, false) == SfxItemState::DEFAULT )
                                {
                                    rSet.Put( rSet.Get(pEntry->nWID) );
                                }
                                pPropSet->setPropertyValue( *pEntry, *pValue, rSet );
                        }
                    }
                    else
                    {
                        rSet.ClearItem( pEntry->nWID );
                        // language for number formats
                        if ( pEntry->nWID == ATTR_VALUE_FORMAT )
                            rSet.ClearItem( ATTR_LANGUAGE_FORMAT );

                        //! for ATTR_ROTATE_VALUE, also reset ATTR_ORIENTATION?
                    }
                }
                else if ( IsScUnoWid( pEntry->nWID ) )
                {
                    switch ( pEntry->nWID )
                    {
                        case SC_WID_UNO_TBLBORD:
                            {
                                if (pValue)
                                {
                                    table::TableBorder aBorder;
                                    if ( *pValue >>= aBorder )
                                    {
                                        SvxBoxItem aOuter( ATTR_BORDER );
                                        SvxBoxInfoItem aInner( ATTR_BORDER_INNER );
                                        ScHelperFunctions::FillBoxItems( aOuter, aInner, aBorder );
                                        rSet.Put( aOuter );
                                    }
                                }
                                else
                                {
                                    rSet.ClearItem( ATTR_BORDER );
                                }
                            }
                            break;
                        case SC_WID_UNO_TBLBORD2:
                            {
                                if (pValue)
                                {
                                    table::TableBorder2 aBorder2;
                                    if ( *pValue >>= aBorder2 )
                                    {
                                        SvxBoxItem aOuter( ATTR_BORDER );
                                        SvxBoxInfoItem aInner( ATTR_BORDER_INNER );
                                        ScHelperFunctions::FillBoxItems( aOuter, aInner, aBorder2 );
                                        rSet.Put( aOuter );
                                    }
                                }
                                else
                                {
                                    rSet.ClearItem( ATTR_BORDER );
                                }
                            }
                            break;
                    }
                }
            }
        }

        //! DocFunc-Funktion??
        //! Undo ?????????????

        ScDocument& rDoc = pDocShell->GetDocument();
        if ( eFamily == SFX_STYLE_FAMILY_PARA )
        {
            //  Zeilenhoehen anpassen...

            ScopedVclPtrInstance< VirtualDevice > pVDev;
            Point aLogic = pVDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
            double nPPTX = aLogic.X() / 1000.0;
            double nPPTY = aLogic.Y() / 1000.0;
            Fraction aZoom(1,1);
            rDoc.StyleSheetChanged( pStyle, false, pVDev, nPPTX, nPPTY, aZoom, aZoom );

            if (!rDoc.IsImportingXML())
            {
                pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
                pDocShell->SetDocumentModified();
            }
        }
        else
        {
            //! ModifyStyleSheet am Dokument (alte Werte merken)

            pDocShell->PageStyleModified( aStyleName, true );
        }
    }
}

uno::Any SAL_CALL ScStyleObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aAny;

    if ( aPropertyName == SC_UNONAME_DISPNAME )      // read-only
    {
        //  core always has the display name
        SfxStyleSheetBase* pStyle = GetStyle_Impl();
        if (pStyle)
            aAny <<= OUString( pStyle->GetName() );
    }
    else
    {
        const SfxItemPropertySimpleEntry* pResultEntry = nullptr;
        const SfxItemSet* pItemSet = GetStyleItemSet_Impl( aPropertyName, pResultEntry );

        if ( pItemSet && pResultEntry )
        {
            sal_uInt16 nWhich = pResultEntry->nWID;

            if ( IsScItemWid( nWhich ) )
            {
                switch ( nWhich )       // fuer Item-Spezial-Behandlungen
                {
                    case ATTR_VALUE_FORMAT:
                        if ( pDocShell )
                        {
                            sal_uInt32 nOldFormat = static_cast<const SfxUInt32Item&>(
                                    pItemSet->Get( ATTR_VALUE_FORMAT )).GetValue();
                            LanguageType eOldLang = static_cast<const SvxLanguageItem&>(
                                    pItemSet->Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                            nOldFormat = pDocShell->GetDocument().GetFormatTable()->
                                    GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );
                            aAny <<= nOldFormat;
                        }
                        break;
                    case ATTR_INDENT:
                        aAny <<= sal_Int16( TwipsToHMM(static_cast<const SfxUInt16Item&>(
                                        pItemSet->Get(nWhich)).GetValue()) );
                        break;
                    case ATTR_STACKED:
                        {
                            sal_Int32 nRot = static_cast<const SfxInt32Item&>(pItemSet->Get(ATTR_ROTATE_VALUE)).GetValue();
                            bool bStacked = static_cast<const SfxBoolItem&>(pItemSet->Get(nWhich)).GetValue();
                            SvxOrientationItem( nRot, bStacked, 0 ).QueryValue( aAny );
                        }
                        break;
                    case ATTR_PAGE_SCALE:
                    case ATTR_PAGE_SCALETOPAGES:
                    case ATTR_PAGE_FIRSTPAGENO:
                        aAny <<= sal_Int16( static_cast<const SfxUInt16Item&>(pItemSet->Get(nWhich)).GetValue() );
                        break;
                    case ATTR_PAGE_CHARTS:
                    case ATTR_PAGE_OBJECTS:
                    case ATTR_PAGE_DRAWINGS:
                        //! sal_Bool-MID fuer ScViewObjectModeItem definieren?
                        aAny <<= static_cast<const ScViewObjectModeItem&>(pItemSet->Get(nWhich)).GetValue() == VOBJ_MODE_SHOW;
                        break;
                    case ATTR_PAGE_PAPERBIN:
                        {
                            // property PrinterPaperTray is the name of the tray

                            sal_uInt8 nValue = static_cast<const SvxPaperBinItem&>(pItemSet->Get(nWhich)).GetValue();
                            OUString aName;
                            if ( nValue == PAPERBIN_PRINTER_SETTINGS )
                                aName = SC_PAPERBIN_DEFAULTNAME;
                            else
                            {
                                Printer* pPrinter = pDocShell->GetPrinter();
                                if (pPrinter)
                                    aName = pPrinter->GetPaperBinName( nValue );
                            }
                            aAny <<= aName;
                        }
                        break;
                    case ATTR_PAGE_SCALETO:
                        {
                            ScPageScaleToItem aItem(static_cast<const ScPageScaleToItem&>(pItemSet->Get(ATTR_PAGE_SCALETO)));
                            if ( aPropertyName == SC_UNO_PAGE_SCALETOX )
                                aAny = uno::makeAny(static_cast<sal_Int16>(aItem.GetWidth()));
                            else
                                aAny = uno::makeAny(static_cast<sal_Int16>(aItem.GetHeight()));
                        }
                        break;
                    case ATTR_HIDDEN:
                        {
                            bool bHidden = false;
                            SfxStyleSheetBase* pStyle = GetStyle_Impl();
                            if ( pStyle )
                                bHidden = pStyle->IsHidden();
                            aAny = uno::makeAny( bHidden );
                        }
                        break;
                    default:
                        //  Default-Items mit falscher Slot-ID
                        //  funktionieren im SfxItemPropertySet3 nicht
                        //! Slot-IDs aendern...
                        if ( pItemSet->GetPool()->GetSlotId(nWhich) == nWhich &&
                             pItemSet->GetItemState(nWhich, false) == SfxItemState::DEFAULT )
                        {
                            SfxItemSet aNoEmptySet( *pItemSet );
                            aNoEmptySet.Put( aNoEmptySet.Get( nWhich ) );
                            pPropSet->getPropertyValue( *pResultEntry, aNoEmptySet, aAny );
                        }
                        else
                            pPropSet->getPropertyValue( *pResultEntry, *pItemSet, aAny );
                }
            }
            else if ( IsScUnoWid( nWhich ) )
            {
                switch ( nWhich )
                {
                    case SC_WID_UNO_TBLBORD:
                    case SC_WID_UNO_TBLBORD2:
                        {
                            const SfxPoolItem* pItem = &pItemSet->Get( ATTR_BORDER );
                            if ( pItem )
                            {
                                SvxBoxItem aOuter( *( static_cast<const SvxBoxItem*>( pItem ) ) );
                                SvxBoxInfoItem aInner( ATTR_BORDER_INNER );
                                if (nWhich == SC_WID_UNO_TBLBORD2)
                                    ScHelperFunctions::AssignTableBorder2ToAny( aAny, aOuter, aInner, true);
                                else
                                    ScHelperFunctions::AssignTableBorderToAny( aAny, aOuter, aInner, true);
                            }
                        }
                        break;
                }
            }
        }
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScStyleObj )

// lang::XServiceInfo

OUString SAL_CALL ScStyleObj::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString("ScStyleObj" );
}

sal_Bool SAL_CALL ScStyleObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScStyleObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    bool bPage = ( eFamily == SFX_STYLE_FAMILY_PAGE );
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = SCSTYLE_SERVICE;
    pArray[1] = bPage ? OUString(SCPAGESTYLE_SERVICE)
                      : OUString(SCCELLSTYLE_SERVICE);
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

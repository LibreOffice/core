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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>
#include <o3tl/safeint.hxx>
#include <svx/svdpool.hxx>

#include <vcl/svapp.hxx>
#include <svx/algitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/srchitem.hxx>
#include <svl/sharedstringpool.hxx>
#include <svx/unomid.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unotext.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/bindings.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <float.h>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
#include <com/sun/star/beans/SetPropertyTolerantFailed.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/sheet/XConditionalFormats.hpp>

#include <autoform.hxx>
#include <cellvalue.hxx>
#include <cellmergeoption.hxx>
#include <cellsuno.hxx>
#include <cursuno.hxx>
#include <textuno.hxx>
#include <editsrc.hxx>
#include <notesuno.hxx>
#include <fielduno.hxx>
#include <docuno.hxx>
#include <datauno.hxx>
#include <dapiuno.hxx>
#include <chartuno.hxx>
#include <fmtuno.hxx>
#include <miscuno.hxx>
#include <convuno.hxx>
#include <srchuno.hxx>
#include <nameuno.hxx>
#include <targuno.hxx>
#include <tokenuno.hxx>
#include <eventuno.hxx>
#include <docsh.hxx>
#include <markdata.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <docfunc.hxx>
#include <dbdocfun.hxx>
#include <olinefun.hxx>
#include <hints.hxx>
#include <formulacell.hxx>
#include <undotab.hxx>
#include <undoblk.hxx>
#include <stlsheet.hxx>
#include <dbdata.hxx>
#include <attrib.hxx>
#include <chartarr.hxx>
#include <chartlis.hxx>
#include <drwlayer.hxx>
#include <printfun.hxx>
#include <prnsave.hxx>
#include <tablink.hxx>
#include <dociter.hxx>
#include <rangeutl.hxx>
#include <conditio.hxx>
#include <validat.hxx>
#include <sc.hrc>
#include <cellform.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <unonames.hxx>
#include <styleuno.hxx>
#include <rangeseq.hxx>
#include <unowids.hxx>
#include <paramisc.hxx>
#include <queryentry.hxx>
#include <formula/errorcodes.hxx>
#include <unoreflist.hxx>
#include <formula/grammar.hxx>
#include <editeng/escapementitem.hxx>
#include <stringutil.hxx>
#include <formulaiter.hxx>
#include <tokenarray.hxx>
#include <stylehelper.hxx>
#include <dputil.hxx>
#include <sortparam.hxx>
#include <condformatuno.hxx>
#include <TablePivotCharts.hxx>
#include <table.hxx>
#include <refundo.hxx>
#include <columnspanset.hxx>

#include <memory>

using namespace com::sun::star;

namespace {

class ScNamedEntry
{
    OUString  aName;
    ScRange   aRange;

public:
            ScNamedEntry(const OUString& rN, const ScRange& rR) :
                aName(rN), aRange(rR) {}

    const OUString& GetName() const     { return aName; }
    const ScRange&  GetRange() const    { return aRange; }
};

}

//  The names in the maps must be sorted according to strcmp!
//! Instead of Which-ID 0 use special IDs and do not compare via names!

//  Left/Right/Top/BottomBorder are mapped directly to the core items,
//  not collected/applied to the borders of a range -> ATTR_BORDER can be used directly

static const SfxItemPropertySet* lcl_GetCellsPropertySet()
{
    static const SfxItemPropertyMapEntry aCellsPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ABSNAME,  SC_WID_UNO_ABSNAME, cppu::UnoType<OUString>::get(),        0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ASIANVERT,ATTR_VERTICAL_ASIAN,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_BOTTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_BOTTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLBACK, ATTR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {u"" SC_UNONAME_CELLPRO,  ATTR_PROTECTION,    cppu::UnoType<util::CellProtection>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLSTYL, SC_WID_UNO_CELLSTYL,cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_CCOLOR,   ATTR_FONT_COLOR,    cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_COUTL,    ATTR_FONT_CONTOUR,  cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CCROSS,   ATTR_FONT_CROSSEDOUT,cppu::UnoType<bool>::get(),                 0, MID_CROSSED_OUT },
        {u"" SC_UNONAME_CEMPHAS,  ATTR_FONT_EMPHASISMARK,cppu::UnoType<sal_Int16>::get(),         0, MID_EMPHASIS },
        {u"" SC_UNONAME_CFONT,    ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFCHARS,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CJK_CFCHARS,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CTL_CFCHARS,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNONAME_CFFAMIL,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CJK_CFFAMIL,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CTL_CFFAMIL,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFNAME,   ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CJK_CFNAME,   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CTL_CFNAME,   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNONAME_CFPITCH,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CJK_CFPITCH,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CTL_CFPITCH,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNONAME_CFSTYLE,  ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CJK_CFSTYLE,  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CTL_CFSTYLE,  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNONAME_CHEIGHT,  ATTR_FONT_HEIGHT,   cppu::UnoType<float>::get(),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CJK_CHEIGHT,  ATTR_CJK_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CTL_CHEIGHT,  ATTR_CTL_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNONAME_CLOCAL,   ATTR_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(),         0, MID_LANG_LOCALE },
        {u"" SC_UNO_CJK_CLOCAL,   ATTR_CJK_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNO_CTL_CLOCAL,   ATTR_CTL_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNONAME_COVER,    ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_COVRLCOL, ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_COVRLHAS, ATTR_FONT_OVERLINE, cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CPOST,    ATTR_FONT_POSTURE,  cppu::UnoType<awt::FontSlant>::get(),       0, MID_POSTURE },
        {u"" SC_UNO_CJK_CPOST,    ATTR_CJK_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNO_CTL_CPOST,    ATTR_CTL_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNONAME_CRELIEF,  ATTR_FONT_RELIEF,   cppu::UnoType<sal_Int16>::get(),            0, MID_RELIEF },
        {u"" SC_UNONAME_CSHADD,   ATTR_FONT_SHADOWED, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CSTRIKE,  ATTR_FONT_CROSSEDOUT,cppu::UnoType<sal_Int16>::get(),           0, MID_CROSS_OUT },
        {u"" SC_UNONAME_CUNDER,   ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_CUNDLCOL, ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_CUNDLHAS, ATTR_FONT_UNDERLINE,cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CWEIGHT,  ATTR_FONT_WEIGHT,   cppu::UnoType<float>::get(),                0, MID_WEIGHT },
        {u"" SC_UNO_CJK_CWEIGHT,  ATTR_CJK_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNO_CTL_CWEIGHT,  ATTR_CTL_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNONAME_CWORDMOD, ATTR_FONT_WORDLINE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHCOLHDR, SC_WID_UNO_CHCOLHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHROWHDR, SC_WID_UNO_CHROWHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CONDFMT,  SC_WID_UNO_CONDFMT, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDLOC,  SC_WID_UNO_CONDLOC, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDXML,  SC_WID_UNO_CONDXML, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_DIAGONAL_BLTR, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_BLTR2, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR2, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLHJUS, ATTR_HOR_JUSTIFY,   cppu::UnoType<table::CellHoriJustify>::get(), 0, MID_HORJUST_HORJUST },
        {u"" SC_UNONAME_CELLHJUS_METHOD, ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_CELLTRAN, ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),                  0, MID_GRAPHIC_TRANSPARENT },
        {u"" SC_UNONAME_WRAP,     ATTR_LINEBREAK,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_LEFTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_LEFTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_NUMFMT,   ATTR_VALUE_FORMAT,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_NUMRULES, SC_WID_UNO_NUMRULES,cppu::UnoType<container::XIndexReplace>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLORI,  ATTR_STACKED,       cppu::UnoType<table::CellOrientation>::get(), 0, 0 },
        {u"" SC_UNONAME_PADJUST,  ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PBMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_PINDENT,  ATTR_INDENT,        cppu::UnoType<sal_Int16>::get(),            0, 0 }, //! CONVERT_TWIPS
        {u"" SC_UNONAME_PISCHDIST,ATTR_SCRIPTSPACE,   cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PISFORBID,ATTR_FORBIDDEN_RULES,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHANG,  ATTR_HANGPUNCTUATION,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHYPHEN,ATTR_HYPHENATE,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PLASTADJ, ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PLMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PRMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PTMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_RIGHTBORDER,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_RIGHTBORDER2,ATTR_BORDER,     ::cppu::UnoType<table::BorderLine2>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_ROTANG,   ATTR_ROTATE_VALUE,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_ROTREF,   ATTR_ROTATE_MODE,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_SHADOW,   ATTR_SHADOW,        cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_SHRINK_TO_FIT, ATTR_SHRINKTOFIT, cppu::UnoType<bool>::get(),               0, 0 },
        {u"" SC_UNONAME_TBLBORD,  SC_WID_UNO_TBLBORD, cppu::UnoType<table::TableBorder>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TBLBORD2,  SC_WID_UNO_TBLBORD2, cppu::UnoType<table::TableBorder2>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER,ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER2,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_USERDEF,  ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIDAT,  SC_WID_UNO_VALIDAT, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALILOC,  SC_WID_UNO_VALILOC, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIXML,  SC_WID_UNO_VALIXML, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS, ATTR_VER_JUSTIFY,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS_METHOD, ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_WRITING,  ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        {u"" SC_UNONAME_HYPERLINK,  ATTR_HYPERLINK, cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_FORMATID,  SC_WID_UNO_FORMATID, cppu::UnoType<sal_uInt64>::get(),        0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aCellsPropertySet( aCellsPropertyMap_Impl );
    return &aCellsPropertySet;
}

//  CellRange contains all entries from Cells, plus its own entries
//  with Which-ID 0 (those are needed only for getPropertySetInfo).

static const SfxItemPropertySet* lcl_GetRangePropertySet()
{
    static const SfxItemPropertyMapEntry aRangePropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ABSNAME,  SC_WID_UNO_ABSNAME, cppu::UnoType<OUString>::get(),        0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ASIANVERT,ATTR_VERTICAL_ASIAN,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_BOTTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_BOTTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLBACK, ATTR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {u"" SC_UNONAME_CELLPRO,  ATTR_PROTECTION,    cppu::UnoType<util::CellProtection>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLSTYL, SC_WID_UNO_CELLSTYL,cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_CCOLOR,   ATTR_FONT_COLOR,    cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_COUTL,    ATTR_FONT_CONTOUR,  cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CCROSS,   ATTR_FONT_CROSSEDOUT,cppu::UnoType<bool>::get(),                 0, MID_CROSSED_OUT },
        {u"" SC_UNONAME_CEMPHAS,  ATTR_FONT_EMPHASISMARK,cppu::UnoType<sal_Int16>::get(),         0, MID_EMPHASIS },
        {u"" SC_UNONAME_CFONT,    ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFCHARS,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CJK_CFCHARS,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CTL_CFCHARS,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNONAME_CFFAMIL,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CJK_CFFAMIL,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CTL_CFFAMIL,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFNAME,   ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CJK_CFNAME,   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CTL_CFNAME,   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNONAME_CFPITCH,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CJK_CFPITCH,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CTL_CFPITCH,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNONAME_CFSTYLE,  ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CJK_CFSTYLE,  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CTL_CFSTYLE,  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNONAME_CHEIGHT,  ATTR_FONT_HEIGHT,   cppu::UnoType<float>::get(),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CJK_CHEIGHT,  ATTR_CJK_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CTL_CHEIGHT,  ATTR_CTL_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNONAME_CLOCAL,   ATTR_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(),         0, MID_LANG_LOCALE },
        {u"" SC_UNO_CJK_CLOCAL,   ATTR_CJK_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNO_CTL_CLOCAL,   ATTR_CTL_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNONAME_COVER,    ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_COVRLCOL, ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_COVRLHAS, ATTR_FONT_OVERLINE, cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CPOST,    ATTR_FONT_POSTURE,  cppu::UnoType<awt::FontSlant>::get(),       0, MID_POSTURE },
        {u"" SC_UNO_CJK_CPOST,    ATTR_CJK_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNO_CTL_CPOST,    ATTR_CTL_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNONAME_CRELIEF,  ATTR_FONT_RELIEF,   cppu::UnoType<sal_Int16>::get(),            0, MID_RELIEF },
        {u"" SC_UNONAME_CSHADD,   ATTR_FONT_SHADOWED, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CSTRIKE,  ATTR_FONT_CROSSEDOUT,cppu::UnoType<sal_Int16>::get(),           0, MID_CROSS_OUT },
        {u"" SC_UNONAME_CUNDER,   ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_CUNDLCOL, ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_CUNDLHAS, ATTR_FONT_UNDERLINE,cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CWEIGHT,  ATTR_FONT_WEIGHT,   cppu::UnoType<float>::get(),                0, MID_WEIGHT },
        {u"" SC_UNO_CJK_CWEIGHT,  ATTR_CJK_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNO_CTL_CWEIGHT,  ATTR_CTL_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNONAME_CWORDMOD, ATTR_FONT_WORDLINE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHCOLHDR, SC_WID_UNO_CHCOLHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHROWHDR, SC_WID_UNO_CHROWHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CONDFMT,  SC_WID_UNO_CONDFMT, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDLOC,  SC_WID_UNO_CONDLOC, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDXML,  SC_WID_UNO_CONDXML, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_DIAGONAL_BLTR, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_BLTR2, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR2, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLHJUS, ATTR_HOR_JUSTIFY,   cppu::UnoType<table::CellHoriJustify>::get(),   0, MID_HORJUST_HORJUST },
        {u"" SC_UNONAME_CELLHJUS_METHOD, ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_CELLTRAN, ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),                  0, MID_GRAPHIC_TRANSPARENT },
        {u"" SC_UNONAME_WRAP,     ATTR_LINEBREAK,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_LEFTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_LEFTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_NUMFMT,   ATTR_VALUE_FORMAT,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_NUMRULES, SC_WID_UNO_NUMRULES,cppu::UnoType<container::XIndexReplace>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLORI,  ATTR_STACKED,       cppu::UnoType<table::CellOrientation>::get(), 0, 0 },
        {u"" SC_UNONAME_PADJUST,  ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PBMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_PINDENT,  ATTR_INDENT,        cppu::UnoType<sal_Int16>::get(),            0, 0 }, //! CONVERT_TWIPS
        {u"" SC_UNONAME_PISCHDIST,ATTR_SCRIPTSPACE,   cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PISFORBID,ATTR_FORBIDDEN_RULES,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHANG,  ATTR_HANGPUNCTUATION,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHYPHEN,ATTR_HYPHENATE,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PLASTADJ, ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PLMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PRMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PTMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_POS,      SC_WID_UNO_POS,     cppu::UnoType<awt::Point>::get(),           0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_RIGHTBORDER,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_RIGHTBORDER2,ATTR_BORDER,     ::cppu::UnoType<table::BorderLine2>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_ROTANG,   ATTR_ROTATE_VALUE,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_ROTREF,   ATTR_ROTATE_MODE,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_SHADOW,   ATTR_SHADOW,        cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_SHRINK_TO_FIT, ATTR_SHRINKTOFIT, cppu::UnoType<bool>::get(),               0, 0 },
        {u"" SC_UNONAME_SIZE,     SC_WID_UNO_SIZE,    cppu::UnoType<awt::Size>::get(),            0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_TBLBORD,  SC_WID_UNO_TBLBORD, cppu::UnoType<table::TableBorder>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TBLBORD2,  SC_WID_UNO_TBLBORD2, cppu::UnoType<table::TableBorder2>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER,ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER2,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_USERDEF,  ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIDAT,  SC_WID_UNO_VALIDAT, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALILOC,  SC_WID_UNO_VALILOC, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIXML,  SC_WID_UNO_VALIXML, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS, ATTR_VER_JUSTIFY,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS_METHOD, ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_WRITING,  ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        {u"" SC_UNONAME_FORMATID,  SC_WID_UNO_FORMATID, cppu::UnoType<sal_uInt64>::get(),        0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aRangePropertySet( aRangePropertyMap_Impl );
    return &aRangePropertySet;
}

//  Cell contains entries from CellRange, plus its own entries
//  with Which-ID 0 (those are needed only for getPropertySetInfo).

static const SfxItemPropertySet* lcl_GetCellPropertySet()
{
    static const SfxItemPropertyMapEntry aCellPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ABSNAME,  SC_WID_UNO_ABSNAME, cppu::UnoType<OUString>::get(),        0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ASIANVERT,ATTR_VERTICAL_ASIAN,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_BOTTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_BOTTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLBACK, ATTR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {u"" SC_UNONAME_CELLPRO,  ATTR_PROTECTION,    cppu::UnoType<util::CellProtection>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLSTYL, SC_WID_UNO_CELLSTYL,cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_CCOLOR,   ATTR_FONT_COLOR,    cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_COUTL,    ATTR_FONT_CONTOUR,  cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CCROSS,   ATTR_FONT_CROSSEDOUT,cppu::UnoType<bool>::get(),                 0, MID_CROSSED_OUT },
        {u"" SC_UNONAME_CEMPHAS,  ATTR_FONT_EMPHASISMARK,cppu::UnoType<sal_Int16>::get(),         0, MID_EMPHASIS },
        {u"" SC_UNONAME_CFONT,    ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFCHARS,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CJK_CFCHARS,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CTL_CFCHARS,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNONAME_CFFAMIL,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CJK_CFFAMIL,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CTL_CFFAMIL,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFNAME,   ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CJK_CFNAME,   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CTL_CFNAME,   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNONAME_CFPITCH,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CJK_CFPITCH,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CTL_CFPITCH,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNONAME_CFSTYLE,  ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CJK_CFSTYLE,  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CTL_CFSTYLE,  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNONAME_CHEIGHT,  ATTR_FONT_HEIGHT,   cppu::UnoType<float>::get(),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CJK_CHEIGHT,  ATTR_CJK_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CTL_CHEIGHT,  ATTR_CTL_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNONAME_CLOCAL,   ATTR_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(),         0, MID_LANG_LOCALE },
        {u"" SC_UNO_CJK_CLOCAL,   ATTR_CJK_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNO_CTL_CLOCAL,   ATTR_CTL_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNONAME_COVER,    ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_COVRLCOL, ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_COVRLHAS, ATTR_FONT_OVERLINE, cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CPOST,    ATTR_FONT_POSTURE,  cppu::UnoType<awt::FontSlant>::get(),       0, MID_POSTURE },
        {u"" SC_UNO_CJK_CPOST,    ATTR_CJK_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNO_CTL_CPOST,    ATTR_CTL_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNONAME_CRELIEF,  ATTR_FONT_RELIEF,   cppu::UnoType<sal_Int16>::get(),            0, MID_RELIEF },
        {u"" SC_UNONAME_CSHADD,   ATTR_FONT_SHADOWED, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CSTRIKE,  ATTR_FONT_CROSSEDOUT,cppu::UnoType<sal_Int16>::get(),           0, MID_CROSS_OUT },
        {u"" SC_UNONAME_CUNDER,   ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_CUNDLCOL, ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_CUNDLHAS, ATTR_FONT_UNDERLINE,cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CWEIGHT,  ATTR_FONT_WEIGHT,   cppu::UnoType<float>::get(),                0, MID_WEIGHT },
        {u"" SC_UNO_CJK_CWEIGHT,  ATTR_CJK_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNO_CTL_CWEIGHT,  ATTR_CTL_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNONAME_CWORDMOD, ATTR_FONT_WORDLINE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHCOLHDR, SC_WID_UNO_CHCOLHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHROWHDR, SC_WID_UNO_CHROWHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CONDFMT,  SC_WID_UNO_CONDFMT, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDLOC,  SC_WID_UNO_CONDLOC, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDXML,  SC_WID_UNO_CONDXML, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_DIAGONAL_BLTR, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_BLTR2, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR2, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_FORMLOC,  SC_WID_UNO_FORMLOC, cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_FORMRT,   SC_WID_UNO_FORMRT,  cppu::UnoType<table::CellContentType>::get(), 0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_CELLCONTENTTYPE,   SC_WID_UNO_CELLCONTENTTYPE,  cppu::UnoType<table::CellContentType>::get(), 0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_FORMRT2,  SC_WID_UNO_FORMRT2, cppu::UnoType<sal_Int32>::get(), 0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_CELLHJUS, ATTR_HOR_JUSTIFY,   cppu::UnoType<table::CellHoriJustify>::get(), 0, MID_HORJUST_HORJUST },
        {u"" SC_UNONAME_CELLHJUS_METHOD, ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_CELLTRAN, ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),                  0, MID_GRAPHIC_TRANSPARENT },
        {u"" SC_UNONAME_WRAP,     ATTR_LINEBREAK,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_LEFTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_LEFTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_NUMFMT,   ATTR_VALUE_FORMAT,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_NUMRULES, SC_WID_UNO_NUMRULES,cppu::UnoType<container::XIndexReplace>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLORI,  ATTR_STACKED,       cppu::UnoType<table::CellOrientation>::get(), 0, 0 },
        {u"" SC_UNONAME_PADJUST,  ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PBMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_PINDENT,  ATTR_INDENT,        cppu::UnoType<sal_Int16>::get(),            0, 0 }, //! CONVERT_TWIPS
        {u"" SC_UNONAME_PISCHDIST,ATTR_SCRIPTSPACE,   cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PISFORBID,ATTR_FORBIDDEN_RULES,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHANG,  ATTR_HANGPUNCTUATION,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHYPHEN,ATTR_HYPHENATE,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PLASTADJ, ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PLMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PRMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PTMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_POS,      SC_WID_UNO_POS,     cppu::UnoType<awt::Point>::get(),           0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_RIGHTBORDER,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_RIGHTBORDER2,ATTR_BORDER,     ::cppu::UnoType<table::BorderLine2>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_ROTANG,   ATTR_ROTATE_VALUE,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_ROTREF,   ATTR_ROTATE_MODE,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_SHADOW,   ATTR_SHADOW,        cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_SHRINK_TO_FIT, ATTR_SHRINKTOFIT, cppu::UnoType<bool>::get(),               0, 0 },
        {u"" SC_UNONAME_SIZE,     SC_WID_UNO_SIZE,    cppu::UnoType<awt::Size>::get(),            0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_TBLBORD,  SC_WID_UNO_TBLBORD, cppu::UnoType<table::TableBorder>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TBLBORD2,  SC_WID_UNO_TBLBORD2, cppu::UnoType<table::TableBorder2>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER,ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER2,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_USERDEF,  ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIDAT,  SC_WID_UNO_VALIDAT, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALILOC,  SC_WID_UNO_VALILOC, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIXML,  SC_WID_UNO_VALIXML, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS, ATTR_VER_JUSTIFY,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS_METHOD, ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_WRITING,  ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        {u"" UNO_NAME_EDIT_CHAR_ESCAPEMENT,   EE_CHAR_ESCAPEMENT, cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_HYPERLINK,  ATTR_HYPERLINK, cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_FORMATID,  SC_WID_UNO_FORMATID, cppu::UnoType<sal_uInt64>::get(),        0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aCellPropertySet( aCellPropertyMap_Impl );
    return &aCellPropertySet;
}

//  Column and Row contain all entries from CellRange, plus its own entries
//  with Which-ID 0 (those are needed only for getPropertySetInfo).

static const SfxItemPropertySet* lcl_GetColumnPropertySet()
{
    static const SfxItemPropertyMapEntry aColumnPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ABSNAME,  SC_WID_UNO_ABSNAME, cppu::UnoType<OUString>::get(),        0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ASIANVERT,ATTR_VERTICAL_ASIAN,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_BOTTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_BOTTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLBACK, ATTR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {u"" SC_UNONAME_CELLPRO,  ATTR_PROTECTION,    cppu::UnoType<util::CellProtection>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLSTYL, SC_WID_UNO_CELLSTYL,cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_CCOLOR,   ATTR_FONT_COLOR,    cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_COUTL,    ATTR_FONT_CONTOUR,  cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CCROSS,   ATTR_FONT_CROSSEDOUT,cppu::UnoType<bool>::get(),                 0, MID_CROSSED_OUT },
        {u"" SC_UNONAME_CEMPHAS,  ATTR_FONT_EMPHASISMARK,cppu::UnoType<sal_Int16>::get(),         0, MID_EMPHASIS },
        {u"" SC_UNONAME_CFONT,    ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFCHARS,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CJK_CFCHARS,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CTL_CFCHARS,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNONAME_CFFAMIL,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CJK_CFFAMIL,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CTL_CFFAMIL,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFNAME,   ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CJK_CFNAME,   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CTL_CFNAME,   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNONAME_CFPITCH,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CJK_CFPITCH,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CTL_CFPITCH,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNONAME_CFSTYLE,  ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CJK_CFSTYLE,  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CTL_CFSTYLE,  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNONAME_CHEIGHT,  ATTR_FONT_HEIGHT,   cppu::UnoType<float>::get(),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CJK_CHEIGHT,  ATTR_CJK_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CTL_CHEIGHT,  ATTR_CTL_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNONAME_CLOCAL,   ATTR_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(),         0, MID_LANG_LOCALE },
        {u"" SC_UNO_CJK_CLOCAL,   ATTR_CJK_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNO_CTL_CLOCAL,   ATTR_CTL_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNONAME_COVER,    ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_COVRLCOL, ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_COVRLHAS, ATTR_FONT_OVERLINE, cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CPOST,    ATTR_FONT_POSTURE,  cppu::UnoType<awt::FontSlant>::get(),       0, MID_POSTURE },
        {u"" SC_UNO_CJK_CPOST,    ATTR_CJK_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNO_CTL_CPOST,    ATTR_CTL_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNONAME_CRELIEF,  ATTR_FONT_RELIEF,   cppu::UnoType<sal_Int16>::get(),            0, MID_RELIEF },
        {u"" SC_UNONAME_CSHADD,   ATTR_FONT_SHADOWED, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CSTRIKE,  ATTR_FONT_CROSSEDOUT,cppu::UnoType<sal_Int16>::get(),           0, MID_CROSS_OUT },
        {u"" SC_UNONAME_CUNDER,   ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_CUNDLCOL, ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_CUNDLHAS, ATTR_FONT_UNDERLINE,cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CWEIGHT,  ATTR_FONT_WEIGHT,   cppu::UnoType<float>::get(),                0, MID_WEIGHT },
        {u"" SC_UNO_CJK_CWEIGHT,  ATTR_CJK_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNO_CTL_CWEIGHT,  ATTR_CTL_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNONAME_CWORDMOD, ATTR_FONT_WORDLINE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHCOLHDR, SC_WID_UNO_CHCOLHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHROWHDR, SC_WID_UNO_CHROWHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CONDFMT,  SC_WID_UNO_CONDFMT, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDLOC,  SC_WID_UNO_CONDLOC, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDXML,  SC_WID_UNO_CONDXML, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_DIAGONAL_BLTR, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_BLTR2, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR2, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLHJUS, ATTR_HOR_JUSTIFY,   cppu::UnoType<table::CellHoriJustify>::get(), 0, MID_HORJUST_HORJUST },
        {u"" SC_UNONAME_CELLHJUS_METHOD, ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_CELLTRAN, ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),                  0, MID_GRAPHIC_TRANSPARENT },
        {u"" SC_UNONAME_MANPAGE,  SC_WID_UNO_MANPAGE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_NEWPAGE,  SC_WID_UNO_NEWPAGE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_WRAP,     ATTR_LINEBREAK,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CELLVIS,  SC_WID_UNO_CELLVIS, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_LEFTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_LEFTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_NUMFMT,   ATTR_VALUE_FORMAT,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_NUMRULES, SC_WID_UNO_NUMRULES,cppu::UnoType<container::XIndexReplace>::get(), 0, 0 },
        {u"" SC_UNONAME_OWIDTH,   SC_WID_UNO_OWIDTH,  cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CELLORI,  ATTR_STACKED,       cppu::UnoType<table::CellOrientation>::get(), 0, 0 },
        {u"" SC_UNONAME_PADJUST,  ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PBMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_PINDENT,  ATTR_INDENT,        cppu::UnoType<sal_Int16>::get(),            0, 0 }, //! CONVERT_TWIPS
        {u"" SC_UNONAME_PISCHDIST,ATTR_SCRIPTSPACE,   cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PISFORBID,ATTR_FORBIDDEN_RULES,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHANG,  ATTR_HANGPUNCTUATION,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHYPHEN,ATTR_HYPHENATE,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PLASTADJ, ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PLMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PRMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PTMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_POS,      SC_WID_UNO_POS,     cppu::UnoType<awt::Point>::get(),           0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_RIGHTBORDER,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_RIGHTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_ROTANG,   ATTR_ROTATE_VALUE,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_ROTREF,   ATTR_ROTATE_MODE,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_SHADOW,   ATTR_SHADOW,        cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_SHRINK_TO_FIT, ATTR_SHRINKTOFIT, cppu::UnoType<bool>::get(),               0, 0 },
        {u"" SC_UNONAME_SIZE,     SC_WID_UNO_SIZE,    cppu::UnoType<awt::Size>::get(),            0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_TBLBORD,  SC_WID_UNO_TBLBORD, cppu::UnoType<table::TableBorder>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TBLBORD2,  SC_WID_UNO_TBLBORD2, cppu::UnoType<table::TableBorder2>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER,ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER2,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_USERDEF,  ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIDAT,  SC_WID_UNO_VALIDAT, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALILOC,  SC_WID_UNO_VALILOC, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIXML,  SC_WID_UNO_VALIXML, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS, ATTR_VER_JUSTIFY,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS_METHOD, ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_CELLWID,  SC_WID_UNO_CELLWID, cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_WRITING,  ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aColumnPropertySet( aColumnPropertyMap_Impl );
    return &aColumnPropertySet;
}

static const SfxItemPropertySet* lcl_GetRowPropertySet()
{
    static const SfxItemPropertyMapEntry aRowPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ABSNAME,  SC_WID_UNO_ABSNAME, cppu::UnoType<OUString>::get(),        0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ASIANVERT,ATTR_VERTICAL_ASIAN,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_BOTTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_BOTTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLBACK, ATTR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {u"" SC_UNONAME_CELLPRO,  ATTR_PROTECTION,    cppu::UnoType<util::CellProtection>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLSTYL, SC_WID_UNO_CELLSTYL,cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_CCOLOR,   ATTR_FONT_COLOR,    cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_COUTL,    ATTR_FONT_CONTOUR,  cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CCROSS,   ATTR_FONT_CROSSEDOUT,cppu::UnoType<bool>::get(),                 0, MID_CROSSED_OUT },
        {u"" SC_UNONAME_CEMPHAS,  ATTR_FONT_EMPHASISMARK,cppu::UnoType<sal_Int16>::get(),         0, MID_EMPHASIS },
        {u"" SC_UNONAME_CFONT,    ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFCHARS,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CJK_CFCHARS,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CTL_CFCHARS,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNONAME_CFFAMIL,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CJK_CFFAMIL,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CTL_CFFAMIL,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFNAME,   ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CJK_CFNAME,   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CTL_CFNAME,   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNONAME_CFPITCH,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CJK_CFPITCH,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CTL_CFPITCH,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNONAME_CFSTYLE,  ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CJK_CFSTYLE,  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CTL_CFSTYLE,  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNONAME_CHEIGHT,  ATTR_FONT_HEIGHT,   cppu::UnoType<float>::get(),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CJK_CHEIGHT,  ATTR_CJK_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CTL_CHEIGHT,  ATTR_CTL_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNONAME_CLOCAL,   ATTR_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(),         0, MID_LANG_LOCALE },
        {u"" SC_UNO_CJK_CLOCAL,   ATTR_CJK_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNO_CTL_CLOCAL,   ATTR_CTL_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNONAME_COVER,    ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_COVRLCOL, ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_COVRLHAS, ATTR_FONT_OVERLINE, cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CPOST,    ATTR_FONT_POSTURE,  cppu::UnoType<awt::FontSlant>::get(),       0, MID_POSTURE },
        {u"" SC_UNO_CJK_CPOST,    ATTR_CJK_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNO_CTL_CPOST,    ATTR_CTL_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNONAME_CRELIEF,  ATTR_FONT_RELIEF,   cppu::UnoType<sal_Int16>::get(),            0, MID_RELIEF },
        {u"" SC_UNONAME_CSHADD,   ATTR_FONT_SHADOWED, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CSTRIKE,  ATTR_FONT_CROSSEDOUT,cppu::UnoType<sal_Int16>::get(),           0, MID_CROSS_OUT },
        {u"" SC_UNONAME_CUNDER,   ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_CUNDLCOL, ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_CUNDLHAS, ATTR_FONT_UNDERLINE,cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CWEIGHT,  ATTR_FONT_WEIGHT,   cppu::UnoType<float>::get(),                0, MID_WEIGHT },
        {u"" SC_UNO_CJK_CWEIGHT,  ATTR_CJK_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNO_CTL_CWEIGHT,  ATTR_CTL_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNONAME_CWORDMOD, ATTR_FONT_WORDLINE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHCOLHDR, SC_WID_UNO_CHCOLHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHROWHDR, SC_WID_UNO_CHROWHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CONDFMT,  SC_WID_UNO_CONDFMT, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDLOC,  SC_WID_UNO_CONDLOC, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDXML,  SC_WID_UNO_CONDXML, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_DIAGONAL_BLTR, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_BLTR2, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR2, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLHGT,  SC_WID_UNO_CELLHGT, cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_CELLHJUS, ATTR_HOR_JUSTIFY,   cppu::UnoType<table::CellHoriJustify>::get(), 0, MID_HORJUST_HORJUST },
        {u"" SC_UNONAME_CELLHJUS_METHOD, ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_CELLTRAN, ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),                  0, MID_GRAPHIC_TRANSPARENT },
        {u"" SC_UNONAME_CELLFILT, SC_WID_UNO_CELLFILT,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_MANPAGE,  SC_WID_UNO_MANPAGE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_NEWPAGE,  SC_WID_UNO_NEWPAGE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_WRAP,     ATTR_LINEBREAK,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CELLVIS,  SC_WID_UNO_CELLVIS, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_LEFTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_LEFTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_NUMFMT,   ATTR_VALUE_FORMAT,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_NUMRULES, SC_WID_UNO_NUMRULES,cppu::UnoType<container::XIndexReplace>::get(), 0, 0 },
        {u"" SC_UNONAME_OHEIGHT,  SC_WID_UNO_OHEIGHT, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CELLORI,  ATTR_STACKED,       cppu::UnoType<table::CellOrientation>::get(), 0, 0 },
        {u"" SC_UNONAME_PADJUST,  ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PBMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_PINDENT,  ATTR_INDENT,        cppu::UnoType<sal_Int16>::get(),            0, 0 }, //! CONVERT_TWIPS
        {u"" SC_UNONAME_PISCHDIST,ATTR_SCRIPTSPACE,   cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PISFORBID,ATTR_FORBIDDEN_RULES,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHANG,  ATTR_HANGPUNCTUATION,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHYPHEN,ATTR_HYPHENATE,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PLASTADJ, ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PLMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PRMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PTMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_POS,      SC_WID_UNO_POS,     cppu::UnoType<awt::Point>::get(),           0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_RIGHTBORDER,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_RIGHTBORDER2,ATTR_BORDER,     ::cppu::UnoType<table::BorderLine2>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_ROTANG,   ATTR_ROTATE_VALUE,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_ROTREF,   ATTR_ROTATE_MODE,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_SHADOW,   ATTR_SHADOW,        cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_SHRINK_TO_FIT, ATTR_SHRINKTOFIT, cppu::UnoType<bool>::get(),               0, 0 },
        {u"" SC_UNONAME_SIZE,     SC_WID_UNO_SIZE,    cppu::UnoType<awt::Size>::get(),            0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_TBLBORD,  SC_WID_UNO_TBLBORD, cppu::UnoType<table::TableBorder>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TBLBORD2,  SC_WID_UNO_TBLBORD2, cppu::UnoType<table::TableBorder2>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER,ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER2,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_USERDEF,  ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIDAT,  SC_WID_UNO_VALIDAT, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALILOC,  SC_WID_UNO_VALILOC, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIXML,  SC_WID_UNO_VALIXML, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS, ATTR_VER_JUSTIFY,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS_METHOD, ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_WRITING,  ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aRowPropertySet( aRowPropertyMap_Impl );
    return &aRowPropertySet;
}

static const SfxItemPropertySet* lcl_GetSheetPropertySet()
{
    static const SfxItemPropertyMapEntry aSheetPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_ABSNAME,  SC_WID_UNO_ABSNAME, cppu::UnoType<OUString>::get(),        0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_ASIANVERT,ATTR_VERTICAL_ASIAN,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_AUTOPRINT,SC_WID_UNO_AUTOPRINT,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_BORDCOL,  SC_WID_UNO_BORDCOL, cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_BOTTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_BOTTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLBACK, ATTR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),            0, MID_BACK_COLOR },
        {u"" SC_UNONAME_CELLPRO,  ATTR_PROTECTION,    cppu::UnoType<util::CellProtection>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLSTYL, SC_WID_UNO_CELLSTYL,cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_CCOLOR,   ATTR_FONT_COLOR,    cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_COUTL,    ATTR_FONT_CONTOUR,  cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CCROSS,   ATTR_FONT_CROSSEDOUT,cppu::UnoType<bool>::get(),                 0, MID_CROSSED_OUT },
        {u"" SC_UNONAME_CEMPHAS,  ATTR_FONT_EMPHASISMARK,cppu::UnoType<sal_Int16>::get(),         0, MID_EMPHASIS },
        {u"" SC_UNONAME_CFONT,    ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFCHARS,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CJK_CFCHARS,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNO_CTL_CFCHARS,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_CHAR_SET },
        {u"" SC_UNONAME_CFFAMIL,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CJK_CFFAMIL,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNO_CTL_CFFAMIL,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_FAMILY },
        {u"" SC_UNONAME_CFNAME,   ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CJK_CFNAME,   ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNO_CTL_CFNAME,   ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_FAMILY_NAME },
        {u"" SC_UNONAME_CFPITCH,  ATTR_FONT,          cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CJK_CFPITCH,  ATTR_CJK_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNO_CTL_CFPITCH,  ATTR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),            0, MID_FONT_PITCH },
        {u"" SC_UNONAME_CFSTYLE,  ATTR_FONT,          cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CJK_CFSTYLE,  ATTR_CJK_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNO_CTL_CFSTYLE,  ATTR_CTL_FONT,      cppu::UnoType<OUString>::get(),        0, MID_FONT_STYLE_NAME },
        {u"" SC_UNONAME_CHEIGHT,  ATTR_FONT_HEIGHT,   cppu::UnoType<float>::get(),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CJK_CHEIGHT,  ATTR_CJK_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNO_CTL_CHEIGHT,  ATTR_CTL_FONT_HEIGHT,cppu::UnoType<float>::get(),               0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {u"" SC_UNONAME_CLOCAL,   ATTR_FONT_LANGUAGE, cppu::UnoType<lang::Locale>::get(),         0, MID_LANG_LOCALE },
        {u"" SC_UNO_CJK_CLOCAL,   ATTR_CJK_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNO_CTL_CLOCAL,   ATTR_CTL_FONT_LANGUAGE,cppu::UnoType<lang::Locale>::get(),          0, MID_LANG_LOCALE },
        {u"" SC_UNONAME_COVER,    ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_COVRLCOL, ATTR_FONT_OVERLINE, cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_COVRLHAS, ATTR_FONT_OVERLINE, cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CPOST,    ATTR_FONT_POSTURE,  cppu::UnoType<awt::FontSlant>::get(),       0, MID_POSTURE },
        {u"" SC_UNO_CJK_CPOST,    ATTR_CJK_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNO_CTL_CPOST,    ATTR_CTL_FONT_POSTURE,cppu::UnoType<awt::FontSlant>::get(),     0, MID_POSTURE },
        {u"" SC_UNONAME_CRELIEF,  ATTR_FONT_RELIEF,   cppu::UnoType<sal_Int16>::get(),            0, MID_RELIEF },
        {u"" SC_UNONAME_CSHADD,   ATTR_FONT_SHADOWED, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CSTRIKE,  ATTR_FONT_CROSSEDOUT,cppu::UnoType<sal_Int16>::get(),           0, MID_CROSS_OUT },
        {u"" SC_UNONAME_CUNDER,   ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int16>::get(),            0, MID_TL_STYLE },
        {u"" SC_UNONAME_CUNDLCOL, ATTR_FONT_UNDERLINE,cppu::UnoType<sal_Int32>::get(),            0, MID_TL_COLOR },
        {u"" SC_UNONAME_CUNDLHAS, ATTR_FONT_UNDERLINE,cppu::UnoType<bool>::get(),                  0, MID_TL_HASCOLOR },
        {u"" SC_UNONAME_CWEIGHT,  ATTR_FONT_WEIGHT,   cppu::UnoType<float>::get(),                0, MID_WEIGHT },
        {u"" SC_UNO_CJK_CWEIGHT,  ATTR_CJK_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNO_CTL_CWEIGHT,  ATTR_CTL_FONT_WEIGHT,cppu::UnoType<float>::get(),               0, MID_WEIGHT },
        {u"" SC_UNONAME_CWORDMOD, ATTR_FONT_WORDLINE, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHCOLHDR, SC_WID_UNO_CHCOLHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CHROWHDR, SC_WID_UNO_CHROWHDR,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CONDFMT,  SC_WID_UNO_CONDFMT, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDLOC,  SC_WID_UNO_CONDLOC, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_CONDXML,  SC_WID_UNO_CONDXML, cppu::UnoType<sheet::XSheetConditionalEntries>::get(), 0, 0 },
        {u"" SC_UNONAME_COPYBACK, SC_WID_UNO_COPYBACK,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_COPYFORM, SC_WID_UNO_COPYFORM,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_COPYSTYL, SC_WID_UNO_COPYSTYL,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_DIAGONAL_BLTR, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_BLTR2, ATTR_BORDER_BLTR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_DIAGONAL_TLBR2, ATTR_BORDER_TLBR, ::cppu::UnoType<table::BorderLine2>::get(), 0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_CELLHJUS, ATTR_HOR_JUSTIFY,   cppu::UnoType<table::CellHoriJustify>::get(), 0, MID_HORJUST_HORJUST },
        {u"" SC_UNONAME_CELLHJUS_METHOD, ATTR_HOR_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_ISACTIVE, SC_WID_UNO_ISACTIVE,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CELLTRAN, ATTR_BACKGROUND,    cppu::UnoType<bool>::get(),                  0, MID_GRAPHIC_TRANSPARENT },
        {u"" SC_UNONAME_WRAP,     ATTR_LINEBREAK,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_CELLVIS,  SC_WID_UNO_CELLVIS, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_LEFTBORDER,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_LEFTBORDER2,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine2>::get(), 0, LEFT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNO_LINKDISPBIT,  SC_WID_UNO_LINKDISPBIT,cppu::UnoType<awt::XBitmap>::get(), 0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNO_LINKDISPNAME, SC_WID_UNO_LINKDISPNAME,cppu::UnoType<OUString>::get(),    0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_NUMFMT,   ATTR_VALUE_FORMAT,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_NUMRULES, SC_WID_UNO_NUMRULES,cppu::UnoType<container::XIndexReplace>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLORI,  ATTR_STACKED,       cppu::UnoType<table::CellOrientation>::get(), 0, 0 },
        {u"" SC_UNONAME_PAGESTL,  SC_WID_UNO_PAGESTL, cppu::UnoType<OUString>::get(),        0, 0 },
        {u"" SC_UNONAME_PADJUST,  ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PBMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_PINDENT,  ATTR_INDENT,        cppu::UnoType<sal_Int16>::get(),            0, 0 }, //! CONVERT_TWIPS
        {u"" SC_UNONAME_PISCHDIST,ATTR_SCRIPTSPACE,   cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PISFORBID,ATTR_FORBIDDEN_RULES,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHANG,  ATTR_HANGPUNCTUATION,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PISHYPHEN,ATTR_HYPHENATE,     cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_PLASTADJ, ATTR_HOR_JUSTIFY,   ::cppu::UnoType<sal_Int16>::get(),    0, MID_HORJUST_ADJUST },
        {u"" SC_UNONAME_PLMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PRMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {u"" SC_UNONAME_PTMARGIN, ATTR_MARGIN,        cppu::UnoType<sal_Int32>::get(),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {u"" SC_UNONAME_POS,      SC_WID_UNO_POS,     cppu::UnoType<awt::Point>::get(),           0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_PRINTBORD,SC_WID_UNO_PRINTBORD,cppu::UnoType<bool>::get(),                 0, 0 },
        {u"" SC_UNONAME_PROTECT,  SC_WID_UNO_PROTECT, cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_RIGHTBORDER,ATTR_BORDER,      ::cppu::UnoType<table::BorderLine>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_RIGHTBORDER2,ATTR_BORDER,     ::cppu::UnoType<table::BorderLine2>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_ROTANG,   ATTR_ROTATE_VALUE,  cppu::UnoType<sal_Int32>::get(),            0, 0 },
        {u"" SC_UNONAME_ROTREF,   ATTR_ROTATE_MODE,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_SHADOW,   ATTR_SHADOW,        cppu::UnoType<table::ShadowFormat>::get(),  0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_SHOWBORD, SC_WID_UNO_SHOWBORD,cppu::UnoType<bool>::get(),                  0, 0 },
        {u"" SC_UNONAME_SHRINK_TO_FIT, ATTR_SHRINKTOFIT, cppu::UnoType<bool>::get(),               0, 0 },
        {u"" SC_UNONAME_SIZE,     SC_WID_UNO_SIZE,    cppu::UnoType<awt::Size>::get(),            0 | beans::PropertyAttribute::READONLY, 0 },
        {u"" SC_UNONAME_TBLBORD,  SC_WID_UNO_TBLBORD, cppu::UnoType<table::TableBorder>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TBLBORD2,  SC_WID_UNO_TBLBORD2, cppu::UnoType<table::TableBorder2>::get(),   0, 0 | CONVERT_TWIPS },
        {u"" SC_UNONAME_TABLAYOUT,SC_WID_UNO_TABLAYOUT,cppu::UnoType<sal_Int16>::get(),           0, 0 },
        {u"" SC_UNONAME_CONDFORMAT, SC_WID_UNO_CONDFORMAT, cppu::UnoType<sheet::XConditionalFormats>::get(), 0, 0},
        {u"" SC_UNONAME_TOPBORDER,ATTR_BORDER,        ::cppu::UnoType<table::BorderLine>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_TOPBORDER2,ATTR_BORDER,       ::cppu::UnoType<table::BorderLine2>::get(), 0, TOP_BORDER | CONVERT_TWIPS },
        {u"" SC_UNONAME_USERDEF,  ATTR_USERDEF,       cppu::UnoType<container::XNameContainer>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIDAT,  SC_WID_UNO_VALIDAT, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALILOC,  SC_WID_UNO_VALILOC, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_VALIXML,  SC_WID_UNO_VALIXML, cppu::UnoType<beans::XPropertySet>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS, ATTR_VER_JUSTIFY,   cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNONAME_CELLVJUS_METHOD, ATTR_VER_JUSTIFY_METHOD, ::cppu::UnoType<sal_Int32>::get(),   0, 0 },
        {u"" SC_UNONAME_WRITING,  ATTR_WRITINGDIR,    cppu::UnoType<sal_Int16>::get(),            0, 0 },
        {u"" SC_UNONAME_TABCOLOR, SC_WID_UNO_TABCOLOR, cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {u"" SC_UNO_CODENAME,        SC_WID_UNO_CODENAME, cppu::UnoType<OUString>::get(),    0, 0},
        {u"" SC_UNO_NAMEDRANGES, SC_WID_UNO_NAMES, cppu::UnoType<sheet::XNamedRanges>::get(), 0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aSheetPropertySet( aSheetPropertyMap_Impl );
    return &aSheetPropertySet;
}

static const SfxItemPropertyMapEntry* lcl_GetEditPropertyMap()
{
    static const SfxItemPropertyMapEntry aEditPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        SVX_UNOEDIT_NUMBERING_PROPERTIE,    // for completeness of service ParagraphProperties
        {u"" SC_UNONAME_TEXTUSER, EE_CHAR_XMLATTRIBS, cppu::UnoType<container::XNameContainer>::get(), 0, 0},
        {u"" SC_UNONAME_USERDEF,  EE_PARA_XMLATTRIBS, cppu::UnoType<container::XNameContainer>::get(), 0, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    return aEditPropertyMap_Impl;
}
static const SvxItemPropertySet* lcl_GetEditPropertySet()
{
    static SvxItemPropertySet aEditPropertySet( lcl_GetEditPropertyMap(), SdrObject::GetGlobalDrawObjectItemPool() );
    return &aEditPropertySet;
}

using sc::HMMToTwips;
using sc::TwipsToHMM;

#define SCCHARPROPERTIES_SERVICE    "com.sun.star.style.CharacterProperties"
#define SCPARAPROPERTIES_SERVICE    "com.sun.star.style.ParagraphProperties"
#define SCCELLPROPERTIES_SERVICE    "com.sun.star.table.CellProperties"
#define SCCELLRANGE_SERVICE         "com.sun.star.table.CellRange"
#define SCCELL_SERVICE              "com.sun.star.table.Cell"
#define SCSHEETCELLRANGES_SERVICE   "com.sun.star.sheet.SheetCellRanges"
#define SCSHEETCELLRANGE_SERVICE    "com.sun.star.sheet.SheetCellRange"
#define SCSPREADSHEET_SERVICE       "com.sun.star.sheet.Spreadsheet"
#define SCSHEETCELL_SERVICE         "com.sun.star.sheet.SheetCell"

SC_SIMPLE_SERVICE_INFO( ScCellFormatsEnumeration, "ScCellFormatsEnumeration", "com.sun.star.sheet.CellFormatRangesEnumeration" )
SC_SIMPLE_SERVICE_INFO( ScCellFormatsObj, "ScCellFormatsObj", "com.sun.star.sheet.CellFormatRanges" )
SC_SIMPLE_SERVICE_INFO( ScUniqueCellFormatsEnumeration, "ScUniqueCellFormatsEnumeration", "com.sun.star.sheet.UniqueCellFormatRangesEnumeration" )
SC_SIMPLE_SERVICE_INFO( ScUniqueCellFormatsObj, "ScUniqueCellFormatsObj", "com.sun.star.sheet.UniqueCellFormatRanges" )
SC_SIMPLE_SERVICE_INFO( ScCellRangesBase, "ScCellRangesBase", "stardiv.unknown" )
SC_SIMPLE_SERVICE_INFO( ScCellsEnumeration, "ScCellsEnumeration", "com.sun.star.sheet.CellsEnumeration" )
SC_SIMPLE_SERVICE_INFO( ScCellsObj, "ScCellsObj", "com.sun.star.sheet.Cells" )
SC_SIMPLE_SERVICE_INFO( ScTableColumnObj, "ScTableColumnObj", "com.sun.star.table.TableColumn" )
SC_SIMPLE_SERVICE_INFO( ScTableRowObj, "ScTableRowObj", "com.sun.star.table.TableRow" )

//! move ScLinkListener into another file !!!

ScLinkListener::~ScLinkListener()
{
}

void ScLinkListener::Notify( const SfxHint& rHint )
{
    aLink.Call( rHint );
}

static void lcl_CopyProperties( beans::XPropertySet& rDest, beans::XPropertySet& rSource )
{
    uno::Reference<beans::XPropertySetInfo> xInfo(rSource.getPropertySetInfo());
    if (xInfo.is())
    {
        const uno::Sequence<beans::Property> aSeq(xInfo->getProperties());
        for (const beans::Property& rProp : aSeq)
        {
            OUString aName(rProp.Name);
            rDest.setPropertyValue( aName, rSource.getPropertyValue( aName ) );
        }
    }
}

static SCTAB lcl_FirstTab( const ScRangeList& rRanges )
{
    if (rRanges.empty())
        throw std::out_of_range("empty range");
    const ScRange & rFirst = rRanges[0];
    return rFirst.aStart.Tab();
}

static bool lcl_WholeSheet( const ScDocument& rDoc, const ScRangeList& rRanges )
{
    if ( rRanges.size() == 1 )
    {
        const ScRange & rRange = rRanges[0];
        if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == rDoc.MaxCol() &&
             rRange.aStart.Row() == 0 && rRange.aEnd.Row() == rDoc.MaxRow() )
            return true;
    }
    return false;
}

namespace {
template<typename BorderLineType>
const ::editeng::SvxBorderLine* lcl_getBorderLine(
        ::editeng::SvxBorderLine& rLine, const BorderLineType& rStruct )
{
    // Convert from 1/100mm to Twips.
    if (!SvxBoxItem::LineToSvxLine( rStruct, rLine, true))
        return nullptr;

    if ( rLine.GetOutWidth() || rLine.GetInWidth() || rLine.GetDistance() )
        return &rLine;
    else
        return nullptr;
}
}

const ::editeng::SvxBorderLine* ScHelperFunctions::GetBorderLine(
        ::editeng::SvxBorderLine& rLine, const table::BorderLine& rStruct )
{
    return lcl_getBorderLine( rLine, rStruct);
}

const ::editeng::SvxBorderLine* ScHelperFunctions::GetBorderLine(
        ::editeng::SvxBorderLine& rLine, const table::BorderLine2& rStruct )
{
    return lcl_getBorderLine( rLine, rStruct);
}

namespace {
template<typename TableBorderType>
void lcl_fillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const TableBorderType& rBorder )
{
    ::editeng::SvxBorderLine aLine;
    rOuter.SetAllDistances(static_cast<sal_uInt16>(HMMToTwips(rBorder.Distance)));
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.TopLine ),         SvxBoxItemLine::TOP );
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.BottomLine ),      SvxBoxItemLine::BOTTOM );
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.LeftLine ),        SvxBoxItemLine::LEFT );
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.RightLine ),       SvxBoxItemLine::RIGHT );
    rInner.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.HorizontalLine ),  SvxBoxInfoItemLine::HORI );
    rInner.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.VerticalLine ),    SvxBoxInfoItemLine::VERT );
    rInner.SetValid( SvxBoxInfoItemValidFlags::TOP,      rBorder.IsTopLineValid );
    rInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,   rBorder.IsBottomLineValid );
    rInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,     rBorder.IsLeftLineValid );
    rInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,    rBorder.IsRightLineValid );
    rInner.SetValid( SvxBoxInfoItemValidFlags::HORI,     rBorder.IsHorizontalLineValid );
    rInner.SetValid( SvxBoxInfoItemValidFlags::VERT,     rBorder.IsVerticalLineValid );
    rInner.SetValid( SvxBoxInfoItemValidFlags::DISTANCE, rBorder.IsDistanceValid );
    rInner.SetTable( true );
}
}

void ScHelperFunctions::FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const table::TableBorder& rBorder )
{
    lcl_fillBoxItems( rOuter, rInner, rBorder);
}

void ScHelperFunctions::FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const table::TableBorder2& rBorder )
{
    lcl_fillBoxItems( rOuter, rInner, rBorder);
}

void ScHelperFunctions::FillBorderLine( table::BorderLine& rStruct, const ::editeng::SvxBorderLine* pLine )
{
    // Convert from Twips to 1/100mm.
    table::BorderLine2 aStruct( SvxBoxItem::SvxLineToLine( pLine, true));
    rStruct = aStruct;
}

void ScHelperFunctions::FillBorderLine( table::BorderLine2& rStruct, const ::editeng::SvxBorderLine* pLine )
{
    rStruct = SvxBoxItem::SvxLineToLine( pLine, true);
}

namespace {
template<typename TableBorderItem>
void lcl_fillTableBorder( TableBorderItem& rBorder, const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner,
        bool bInvalidateHorVerDist )
{
    ScHelperFunctions::FillBorderLine( rBorder.TopLine,         rOuter.GetTop() );
    ScHelperFunctions::FillBorderLine( rBorder.BottomLine,      rOuter.GetBottom() );
    ScHelperFunctions::FillBorderLine( rBorder.LeftLine,        rOuter.GetLeft() );
    ScHelperFunctions::FillBorderLine( rBorder.RightLine,       rOuter.GetRight() );
    ScHelperFunctions::FillBorderLine( rBorder.HorizontalLine,  rInner.GetHori() );
    ScHelperFunctions::FillBorderLine( rBorder.VerticalLine,    rInner.GetVert() );

    rBorder.Distance                = rOuter.GetSmallestDistance();
    rBorder.IsTopLineValid          = rInner.IsValid(SvxBoxInfoItemValidFlags::TOP);
    rBorder.IsBottomLineValid       = rInner.IsValid(SvxBoxInfoItemValidFlags::BOTTOM);
    rBorder.IsLeftLineValid         = rInner.IsValid(SvxBoxInfoItemValidFlags::LEFT);
    rBorder.IsRightLineValid        = rInner.IsValid(SvxBoxInfoItemValidFlags::RIGHT);
    rBorder.IsHorizontalLineValid   = !bInvalidateHorVerDist && rInner.IsValid(SvxBoxInfoItemValidFlags::HORI);
    rBorder.IsVerticalLineValid     = !bInvalidateHorVerDist && rInner.IsValid(SvxBoxInfoItemValidFlags::VERT);
    rBorder.IsDistanceValid         = !bInvalidateHorVerDist && rInner.IsValid(SvxBoxInfoItemValidFlags::DISTANCE);
}
}

void ScHelperFunctions::AssignTableBorderToAny( uno::Any& rAny,
        const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner, bool bInvalidateHorVerDist )
{
    table::TableBorder aBorder;
    lcl_fillTableBorder( aBorder, rOuter, rInner, bInvalidateHorVerDist);
    rAny <<= aBorder;
}

void ScHelperFunctions::AssignTableBorder2ToAny( uno::Any& rAny,
        const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner, bool bInvalidateHorVerDist )
{
    table::TableBorder2 aBorder;
    lcl_fillTableBorder( aBorder, rOuter, rInner, bInvalidateHorVerDist);
    rAny <<= aBorder;
}

//! move lcl_ApplyBorder to docfunc !

void ScHelperFunctions::ApplyBorder( ScDocShell* pDocShell, const ScRangeList& rRanges,
                        const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());
    ScDocumentUniquePtr pUndoDoc;
    if (bUndo)
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
    size_t nCount = rRanges.size();
    for (size_t i = 0; i < nCount; ++i)
    {
        ScRange const & rRange = rRanges[ i ];
        SCTAB nTab = rRange.aStart.Tab();

        if (bUndo)
        {
            if ( i==0 )
                pUndoDoc->InitUndo( rDoc, nTab, nTab );
            else
                pUndoDoc->AddUndoTab( nTab, nTab );
            rDoc.CopyToDocument(rRange, InsertDeleteFlags::ATTRIB, false, *pUndoDoc);
        }

        ScMarkData aMark(rDoc.GetSheetLimits());
        aMark.SetMarkArea( rRange );
        aMark.SelectTable( nTab, true );

        rDoc.ApplySelectionFrame(aMark, rOuter, &rInner);
        // don't need RowHeight if there is only a border
    }

    if (bUndo)
    {
        pDocShell->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoBorder>( pDocShell, rRanges, std::move(pUndoDoc), rOuter, rInner ) );
    }

    for (size_t i = 0; i < nCount; ++i )
        pDocShell->PostPaint( rRanges[ i ], PaintPartFlags::Grid, SC_PF_LINES | SC_PF_TESTMERGE );

    pDocShell->SetDocumentModified();
}

//! move lcl_PutDataArray to docfunc?
//! merge loop with ScFunctionAccess::callFunction

static bool lcl_PutDataArray( ScDocShell& rDocShell, const ScRange& rRange,
                        const uno::Sequence< uno::Sequence<uno::Any> >& aData )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    bool bUndo(rDoc.IsUndoEnabled());

    if ( !rDoc.IsBlockEditable( nTab, nStartCol,nStartRow, nEndCol,nEndRow ) )
    {
        //! error message
        return false;
    }

    sal_Int32 nCols = 0;
    sal_Int32 nRows = aData.getLength();
    if ( nRows )
        nCols = aData[0].getLength();

    if ( nCols != nEndCol-nStartCol+1 || nRows != nEndRow-nStartRow+1 )
    {
        //! error message?
        return false;
    }

    ScDocumentUniquePtr pUndoDoc;
    if ( bUndo )
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( rDoc, nTab, nTab );
        rDoc.CopyToDocument(rRange, InsertDeleteFlags::CONTENTS|InsertDeleteFlags::NOCAPTIONS, false, *pUndoDoc);
    }

    rDoc.DeleteAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, InsertDeleteFlags::CONTENTS );

    bool bError = false;
    SCROW nDocRow = nStartRow;
    for (const uno::Sequence<uno::Any>& rColSeq : aData)
    {
        if ( rColSeq.getLength() == nCols )
        {
            SCCOL nDocCol = nStartCol;
            for (const uno::Any& rElement : rColSeq)
            {
                ScAddress aPos(nDocCol, nDocRow, nTab);

                switch( rElement.getValueTypeClass() )
                {
                    case uno::TypeClass_VOID:
                    {
                        // void = "no value"
                        rDoc.SetError( nDocCol, nDocRow, nTab, FormulaError::NotAvailable );
                    }
                    break;

                    //  #87871# accept integer types because Basic passes a floating point
                    //  variable as byte, short or long if it's an integer number.
                    case uno::TypeClass_BYTE:
                    case uno::TypeClass_SHORT:
                    case uno::TypeClass_UNSIGNED_SHORT:
                    case uno::TypeClass_LONG:
                    case uno::TypeClass_UNSIGNED_LONG:
                    case uno::TypeClass_FLOAT:
                    case uno::TypeClass_DOUBLE:
                    {
                        double fVal(0.0);
                        rElement >>= fVal;
                        rDoc.SetValue(aPos, fVal);
                    }
                    break;

                    case uno::TypeClass_STRING:
                    {
                        OUString aUStr;
                        rElement >>= aUStr;
                        if ( !aUStr.isEmpty() )
                        {
                            ScSetStringParam aParam;
                            aParam.setTextInput();
                            rDoc.SetString(aPos, aUStr, &aParam);
                        }
                    }
                    break;

                    // accept Sequence<FormulaToken> for formula cells
                    case uno::TypeClass_SEQUENCE:
                    {
                        uno::Sequence< sheet::FormulaToken > aTokens;
                        if ( rElement >>= aTokens )
                        {
                            ScTokenArray aTokenArray(rDoc);
                            ScTokenConversion::ConvertToTokenArray( rDoc, aTokenArray, aTokens );
                            rDoc.SetFormula(aPos, aTokenArray);
                        }
                        else
                            bError = true;
                    }
                    break;

                    default:
                        bError = true;      // invalid type
                }
                ++nDocCol;
            }
        }
        else
            bError = true;                          // wrong size

        ++nDocRow;
    }

    bool bHeight = rDocShell.AdjustRowHeight( nStartRow, nEndRow, nTab );

    if ( pUndoDoc )
    {
        ScMarkData aDestMark(rDoc.GetSheetLimits());
        aDestMark.SelectOneTable( nTab );
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoPaste>(
                &rDocShell, ScRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab),
                aDestMark, std::move(pUndoDoc), nullptr, InsertDeleteFlags::CONTENTS, nullptr, false));
    }

    if (!bHeight)
        rDocShell.PostPaint( rRange, PaintPartFlags::Grid );      // AdjustRowHeight may have painted already

    rDocShell.SetDocumentModified();

    return !bError;
}

static bool lcl_PutFormulaArray( ScDocShell& rDocShell, const ScRange& rRange,
        const uno::Sequence< uno::Sequence<OUString> >& aData,
        const formula::FormulaGrammar::Grammar eGrammar )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    bool bUndo(rDoc.IsUndoEnabled());

    if ( !rDoc.IsBlockEditable( nTab, nStartCol,nStartRow, nEndCol,nEndRow ) )
    {
        //! error message
        return false;
    }

    sal_Int32 nCols = 0;
    sal_Int32 nRows = aData.getLength();
    if ( nRows )
        nCols = aData[0].getLength();

    if ( nCols != nEndCol-nStartCol+1 || nRows != nEndRow-nStartRow+1 )
    {
        //! error message?
        return false;
    }

    ScDocumentUniquePtr pUndoDoc;
    if ( bUndo )
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( rDoc, nTab, nTab );
        rDoc.CopyToDocument(rRange, InsertDeleteFlags::CONTENTS, false, *pUndoDoc);
    }

    rDoc.DeleteAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, InsertDeleteFlags::CONTENTS );

    bool bError = false;
    SCROW nDocRow = nStartRow;
    for (const uno::Sequence<OUString>& rColSeq : aData)
    {
        if ( rColSeq.getLength() == nCols )
        {
            SCCOL nDocCol = nStartCol;
            for (const OUString& aText : rColSeq)
            {
                ScAddress aPos( nDocCol, nDocRow, nTab );

                ScInputStringType aRes =
                    ScStringUtil::parseInputString(
                        *rDoc.GetFormatTable(), aText, LANGUAGE_ENGLISH_US);
                switch (aRes.meType)
                {
                    case ScInputStringType::Formula:
                        rDoc.SetFormula(aPos, aRes.maText, eGrammar);
                    break;
                    case ScInputStringType::Number:
                        rDoc.SetValue(aPos, aRes.mfValue);
                    break;
                    case ScInputStringType::Text:
                        rDoc.SetTextCell(aPos, aRes.maText);
                    break;
                    default:
                        ;
                }

                ++nDocCol;
            }
        }
        else
            bError = true;                          // wrong size

        ++nDocRow;
    }

    bool bHeight = rDocShell.AdjustRowHeight( nStartRow, nEndRow, nTab );

    if ( pUndoDoc )
    {
        ScMarkData aDestMark(rDoc.GetSheetLimits());
        aDestMark.SelectOneTable( nTab );
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoPaste>( &rDocShell,
                ScRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab), aDestMark,
                std::move(pUndoDoc), nullptr, InsertDeleteFlags::CONTENTS, nullptr, false));
    }

    if (!bHeight)
        rDocShell.PostPaint( rRange, PaintPartFlags::Grid );      // AdjustRowHeight may have painted already

    rDocShell.SetDocumentModified();

    return !bError;
}

//  used in ScCellRangeObj::getFormulaArray and ScCellObj::GetInputString_Impl
static OUString lcl_GetInputString( ScDocument& rDoc, const ScAddress& rPos, bool bEnglish )
{
    ScRefCellValue aCell(rDoc, rPos);
    if (aCell.isEmpty())
        return EMPTY_OUSTRING;

    OUString aVal;

    CellType eType = aCell.meType;
    if (eType == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pForm = aCell.mpFormula;
        pForm->GetFormula( aVal, formula::FormulaGrammar::mapAPItoGrammar( bEnglish, false));
        return aVal;
    }

    SvNumberFormatter* pFormatter = bEnglish ? ScGlobal::GetEnglishFormatter() :
                                                rDoc.GetFormatTable();
    // Since the English formatter was constructed with
    // LANGUAGE_ENGLISH_US the "General" format has index key 0,
    // we don't have to query.
    sal_uInt32 nNumFmt = bEnglish ? 0 : rDoc.GetNumberFormat(rPos);

    if (eType == CELLTYPE_EDIT)
    {
        //  GetString on EditCell turns breaks into spaces,
        //  but we need the breaks here
        const EditTextObject* pData = aCell.mpEditText;
        if (pData)
        {
            EditEngine& rEngine = rDoc.GetEditEngine();
            rEngine.SetText(*pData);
            aVal = rEngine.GetText();
        }
    }
    else
        ScCellFormat::GetInputString(aCell, nNumFmt, aVal, *pFormatter, rDoc);

    //  if applicable, prepend ' like in ScTabViewShell::UpdateInputHandler
    if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
    {
        double fDummy;
        OUString aTempString = aVal;
        bool bIsNumberFormat(pFormatter->IsNumberFormat(aTempString, nNumFmt, fDummy));
        if ( bIsNumberFormat )
            aTempString = "'" + aTempString;
        else if ( aTempString.startsWith("'") )
        {
            //  if the string starts with a "'", add another one because setFormula
            //  strips one (like text input, except for "text" number formats)
            if ( bEnglish || ( pFormatter->GetType(nNumFmt) != SvNumFormatType::TEXT ) )
                aTempString = "'" + aTempString;
        }
        aVal = aTempString;
    }
    return aVal;
}

ScCellRangesBase::ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR) :
    pPropSet(lcl_GetCellsPropertySet()),
    pDocShell( pDocSh ),
    nObjectId( 0 ),
    bChartColAsHdr( false ),
    bChartRowAsHdr( false ),
    bCursorOnly( false ),
    bGotDataChangedHint( false ),
    aValueListeners( 0 )
{
    // this is a hack to get m_wThis initialized; ideally there would be
    // factory functions doing this but there are so many subclasses of this...
    osl_atomic_increment(&m_refCount);
    {
        m_wThis = uno::Reference<uno::XInterface>(
                    static_cast<cppu::OWeakObject*>(this));
    }
    osl_atomic_decrement(&m_refCount);

    ScRange aCellRange(rR);
    aCellRange.PutInOrder();
    aRanges.push_back( aCellRange );

    if (pDocShell)  // Null if created with createInstance
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        rDoc.AddUnoObject(*this);
        nObjectId = rDoc.GetNewUnoId();
    }
}

ScCellRangesBase::ScCellRangesBase(ScDocShell* pDocSh, const ScRangeList& rR) :
    pPropSet(lcl_GetCellsPropertySet()),
    pDocShell( pDocSh ),
    aRanges( rR ),
    nObjectId( 0 ),
    bChartColAsHdr( false ),
    bChartRowAsHdr( false ),
    bCursorOnly( false ),
    bGotDataChangedHint( false ),
    aValueListeners( 0 )
{
    // this is a hack to get m_wThis initialized; ideally there would be
    // factory functions doing this but there are so many subclasses of this...
    osl_atomic_increment(&m_refCount);
    {
        m_wThis = uno::Reference<uno::XInterface>(
                    static_cast<cppu::OWeakObject*>(this));
    }
    osl_atomic_decrement(&m_refCount);

    if (pDocShell)  // Null if created with createInstance
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        rDoc.AddUnoObject(*this);
        nObjectId = rDoc.GetNewUnoId();
    }
}

ScCellRangesBase::~ScCellRangesBase()
{
    SolarMutexGuard g;

    //  call RemoveUnoObject first, so no notification can happen
    //  during ForgetCurrentAttrs

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);

    ForgetCurrentAttrs();
    ForgetMarkData();

    pValueListener.reset();

    //! unregister XChartDataChangeEventListener ??
    //! (ChartCollection will then hold this object as well!)
}

void ScCellRangesBase::ForgetCurrentAttrs()
{
    pCurrentFlat.reset();
    pCurrentDeep.reset();
    pCurrentDataSet.reset();
    pNoDfltCurrentDataSet.reset();
    pCurrentDataSet = nullptr;
    pNoDfltCurrentDataSet = nullptr;

    // #i62483# pMarkData can remain unchanged, is deleted only if the range changes (RefChanged)
}

void ScCellRangesBase::ForgetMarkData()
{
    pMarkData.reset();
}

const ScPatternAttr* ScCellRangesBase::GetCurrentAttrsFlat()
{
    //  get and cache direct cell attributes for this object's range

    if ( !pCurrentFlat && pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        pCurrentFlat = rDoc.CreateSelectionPattern( *GetMarkData(), false );
    }
    return pCurrentFlat.get();
}

const ScPatternAttr* ScCellRangesBase::GetCurrentAttrsDeep()
{
    //  get and cache cell attributes (incl. styles) for this object's range

    if ( !pCurrentDeep && pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        pCurrentDeep = rDoc.CreateSelectionPattern( *GetMarkData() );
    }
    return pCurrentDeep.get();
}

SfxItemSet* ScCellRangesBase::GetCurrentDataSet(bool bNoDflt)
{
    if(!pCurrentDataSet)
    {
        const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
        if ( pPattern )
        {
            //  replace Dontcare with Default,  so that we always have a reflection
            pCurrentDataSet.reset( new SfxItemSet( pPattern->GetItemSet() ) );
            pNoDfltCurrentDataSet.reset( new SfxItemSet( pPattern->GetItemSet() ) );
            pCurrentDataSet->ClearInvalidItems();
        }
    }
    return bNoDflt ? pNoDfltCurrentDataSet.get() : pCurrentDataSet.get();
}

const ScMarkData* ScCellRangesBase::GetMarkData()
{
    if (!pMarkData)
    {
        pMarkData.reset( new ScMarkData(GetDocument()->GetSheetLimits(), aRanges) );
    }
    return pMarkData.get();
}

void ScCellRangesBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        if (SfxHintId::Dying == rHint.GetId())
        {   // if the document dies, must reset to avoid crash in dtor!
            ForgetCurrentAttrs();
            pDocShell = nullptr;
        }
        return;
    }
    if ( auto pRefHint = dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        std::unique_ptr<ScRangeList> pUndoRanges;
        if ( rDoc.HasUnoRefUndo() )
            pUndoRanges.reset(new ScRangeList( aRanges ));

        if ( aRanges.UpdateReference( pRefHint->GetMode(), &rDoc, pRefHint->GetRange(),
                                    pRefHint->GetDx(), pRefHint->GetDy(), pRefHint->GetDz() ) )
        {
            if (  pRefHint->GetMode() == URM_INSDEL
               && aRanges.size() == 1
               && comphelper::getUnoTunnelImplementation<ScTableSheetObj>(xThis)
               )
            {
                // #101755#; the range size of a sheet does not change
                ScRange & rR = aRanges.front();
                rR.aStart.SetCol(0);
                rR.aStart.SetRow(0);
                rR.aEnd.SetCol(rDoc.MaxCol());
                rR.aEnd.SetRow(rDoc.MaxRow());
            }
            RefChanged();

            // any change of the range address is broadcast to value (modify) listeners
            if ( !aValueListeners.empty() )
                bGotDataChangedHint = true;

            if ( pUndoRanges )
                rDoc.AddUnoRefChange( nObjectId, *pUndoRanges );
        }
    }
    else if ( auto pUndoHint = dynamic_cast<const ScUnoRefUndoHint*>(&rHint) )
    {
        if ( pUndoHint->GetObjectId() == nObjectId )
        {
            // restore ranges from hint

            aRanges = pUndoHint->GetRanges();

            RefChanged();
            if ( !aValueListeners.empty() )
                bGotDataChangedHint = true;     // need to broadcast the undo, too
        }
    }
    else
    {
        const SfxHintId nId = rHint.GetId();
        if ( nId == SfxHintId::Dying )
        {
            ForgetCurrentAttrs();
            pDocShell = nullptr;           // invalid

            if ( !aValueListeners.empty() )
            {
                //  dispose listeners

                lang::EventObject aEvent;
                aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
                for (uno::Reference<util::XModifyListener> & xValueListener : aValueListeners)
                    xValueListener->disposing( aEvent );

                aValueListeners.clear();

                //  The listeners can't have the last ref to this, as it's still held
                //  by the DocShell.
            }
        }
        else if ( nId == SfxHintId::DataChanged )
        {
            // document content changed -> forget cached attributes
            ForgetCurrentAttrs();

            if ( bGotDataChangedHint && pDocShell )
            {
                //  This object was notified of content changes, so one call
                //  for each listener is generated now.
                //  The calls can't be executed directly because the document's
                //  UNO broadcaster list must not be modified.
                //  Instead, add to the document's list of listener calls,
                //  which will be executed directly after the broadcast of
                //  SfxHintId::DataChanged.

                lang::EventObject aEvent;
                aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));

                // the EventObject holds a Ref to this object until after the listener calls

                ScDocument& rDoc = pDocShell->GetDocument();
                for (const uno::Reference<util::XModifyListener> & xValueListener : aValueListeners)
                    rDoc.AddUnoListenerCall( xValueListener, aEvent );

                bGotDataChangedHint = false;
            }
        }
        else if ( nId == SfxHintId::ScCalcAll )
        {
            // broadcast from DoHardRecalc - set bGotDataChangedHint
            // (SfxHintId::DataChanged follows separately)

            if ( !aValueListeners.empty() )
                bGotDataChangedHint = true;
        }
    }
}

void ScCellRangesBase::RefChanged()
{
    //! adjust XChartDataChangeEventListener

    if ( pValueListener && !aValueListeners.empty() )
    {
        pValueListener->EndListeningAll();

        ScDocument& rDoc = pDocShell->GetDocument();
        for ( size_t i = 0, nCount = aRanges.size(); i < nCount; ++i )
            rDoc.StartListeningArea( aRanges[ i ], false, pValueListener.get() );
    }

    ForgetCurrentAttrs();
    ForgetMarkData();
}

ScDocument* ScCellRangesBase::GetDocument() const
{
    if (pDocShell)
        return &pDocShell->GetDocument();
    else
        return nullptr;
}

void ScCellRangesBase::InitInsertRange(ScDocShell* pDocSh, const ScRange& rR)
{
    if ( pDocShell || !pDocSh )
        return;

    pDocShell = pDocSh;

    ScRange aCellRange(rR);
    aCellRange.PutInOrder();
    aRanges.RemoveAll();
    aRanges.push_back( aCellRange );

    pDocShell->GetDocument().AddUnoObject(*this);

    RefChanged();   // adjust range in range object
}

void ScCellRangesBase::AddRange(const ScRange& rRange, const bool bMergeRanges)
{
    if (bMergeRanges)
        aRanges.Join(rRange);
    else
        aRanges.push_back(rRange);
    RefChanged();
}

void ScCellRangesBase::SetNewRange(const ScRange& rNew)
{
    ScRange aCellRange(rNew);
    aCellRange.PutInOrder();

    aRanges.RemoveAll();
    aRanges.push_back( aCellRange );
    RefChanged();
}

void ScCellRangesBase::SetNewRanges(const ScRangeList& rNew)
{
    aRanges = rNew;
    RefChanged();
}

void ScCellRangesBase::SetCursorOnly( bool bSet )
{
    //  set for a selection object that is created from the cursor position
    //  without anything selected (may contain several sheets)

    bCursorOnly = bSet;
}

void ScCellRangesBase::PaintGridRanges_Impl( )
{
    for (size_t i = 0, nCount = aRanges.size(); i < nCount; ++i)
        pDocShell->PostPaint( aRanges[ i ], PaintPartFlags::Grid );
}

// XSheetOperation

double SAL_CALL ScCellRangesBase::computeFunction( sheet::GeneralFunction nFunction )
{
    SolarMutexGuard aGuard;
    ScMarkData aMark(*GetMarkData());
    aMark.MarkToSimple();
    if (!aMark.IsMarked())
        aMark.SetMarkNegative(true);    // so we can enter dummy position

    ScAddress aDummy;                   // if not marked, ignored if it is negative
    double fVal;
    ScSubTotalFunc eFunc = ScDPUtil::toSubTotalFunc(static_cast<ScGeneralFunction>(nFunction));
    ScDocument& rDoc = pDocShell->GetDocument();
    if ( !rDoc.GetSelectionFunction( eFunc, aDummy, aMark, fVal ) )
    {
        throw uno::RuntimeException();      //! own exception?
    }

    return fVal;
}

void SAL_CALL ScCellRangesBase::clearContents( sal_Int32 nContentFlags )
{
    SolarMutexGuard aGuard;
    if ( !aRanges.empty() )
    {
        // only for clearContents: EDITATTR is only used if no contents are deleted
        InsertDeleteFlags nDelFlags = static_cast<InsertDeleteFlags>(nContentFlags) & InsertDeleteFlags::ALL;
        if ( ( nDelFlags & InsertDeleteFlags::EDITATTR ) && ( nDelFlags & InsertDeleteFlags::CONTENTS ) == InsertDeleteFlags::NONE )
            nDelFlags |= InsertDeleteFlags::EDITATTR;

        pDocShell->GetDocFunc().DeleteContents( *GetMarkData(), nDelFlags, true, true );
    }
    // otherwise nothing to do
}

// XPropertyState

const SfxItemPropertyMap& ScCellRangesBase::GetItemPropertyMap()
{
    return pPropSet->getPropertyMap();
}

static void lcl_GetPropertyWhich( const SfxItemPropertySimpleEntry* pEntry,
                                                sal_uInt16& rItemWhich )
{
    //  Which-ID of the affected items also when the item can't handle
    //  the property by itself
    if ( !pEntry )
        return;

    if ( IsScItemWid( pEntry->nWID ) )
        rItemWhich = pEntry->nWID;
    else
        switch ( pEntry->nWID )
        {
            case SC_WID_UNO_TBLBORD:
            case SC_WID_UNO_TBLBORD2:
                rItemWhich = ATTR_BORDER;
                break;
            case SC_WID_UNO_CONDFMT:
            case SC_WID_UNO_CONDLOC:
            case SC_WID_UNO_CONDXML:
                rItemWhich = ATTR_CONDITIONAL;
                break;
            case SC_WID_UNO_VALIDAT:
            case SC_WID_UNO_VALILOC:
            case SC_WID_UNO_VALIXML:
                rItemWhich = ATTR_VALIDDATA;
                break;
        }

}

beans::PropertyState ScCellRangesBase::GetOnePropertyState( sal_uInt16 nItemWhich, const SfxItemPropertySimpleEntry* pEntry )
{
    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    if ( nItemWhich )                   // item wid (from map or special case)
    {
        //  For items that contain several properties (like background),
        //  "ambiguous" is returned too often here

        //  for PropertyState, don't look at styles
        const ScPatternAttr* pPattern = GetCurrentAttrsFlat();
        if ( pPattern )
        {
            SfxItemState eState = pPattern->GetItemSet().GetItemState( nItemWhich, false );

            if ( nItemWhich == ATTR_VALUE_FORMAT && eState == SfxItemState::DEFAULT )
                eState = pPattern->GetItemSet().GetItemState( ATTR_LANGUAGE_FORMAT, false );

            if ( eState == SfxItemState::SET )
                eRet = beans::PropertyState_DIRECT_VALUE;
            else if ( eState == SfxItemState::DEFAULT )
                eRet = beans::PropertyState_DEFAULT_VALUE;
            else if ( eState == SfxItemState::DONTCARE )
                eRet = beans::PropertyState_AMBIGUOUS_VALUE;
            else
            {
                OSL_FAIL("unknown ItemState");
            }
        }
    }
    else if ( pEntry )
    {
        if ( pEntry->nWID == SC_WID_UNO_CHCOLHDR || pEntry->nWID == SC_WID_UNO_CHROWHDR || pEntry->nWID == SC_WID_UNO_ABSNAME )
            eRet = beans::PropertyState_DIRECT_VALUE;
        else if ( pEntry->nWID == SC_WID_UNO_CELLSTYL )
        {
            //  a style is always set, there's no default state
            const ScStyleSheet* pStyle = pDocShell->GetDocument().GetSelectionStyle(*GetMarkData());
            if (pStyle)
                eRet = beans::PropertyState_DIRECT_VALUE;
            else
                eRet = beans::PropertyState_AMBIGUOUS_VALUE;
        }
        else if ( pEntry->nWID == SC_WID_UNO_NUMRULES )
            eRet = beans::PropertyState_DEFAULT_VALUE;      // numbering rules are always default
    }
    return eRet;
}

beans::PropertyState SAL_CALL ScCellRangesBase::getPropertyState( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    if ( aRanges.empty() )
        throw uno::RuntimeException();

    const SfxItemPropertyMap& rMap = GetItemPropertyMap();     // from derived class
    sal_uInt16 nItemWhich = 0;
    const SfxItemPropertySimpleEntry* pEntry  = rMap.getByName( aPropertyName );
    lcl_GetPropertyWhich( pEntry, nItemWhich );
    return GetOnePropertyState( nItemWhich, pEntry );
}

uno::Sequence<beans::PropertyState> SAL_CALL ScCellRangesBase::getPropertyStates(
                                const uno::Sequence<OUString>& aPropertyNames )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class

    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    std::transform(aPropertyNames.begin(), aPropertyNames.end(), aRet.begin(),
        [this, &rPropertyMap](const auto& rName) -> beans::PropertyState {
            sal_uInt16 nItemWhich = 0;
            const SfxItemPropertySimpleEntry* pEntry  = rPropertyMap.getByName( rName );
            lcl_GetPropertyWhich( pEntry, nItemWhich );
            return GetOnePropertyState(nItemWhich, pEntry);
        });
    return aRet;
}

void SAL_CALL ScCellRangesBase::setPropertyToDefault( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    if ( !pDocShell )
        return;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    sal_uInt16 nItemWhich = 0;
    const SfxItemPropertySimpleEntry* pEntry  = rPropertyMap.getByName( aPropertyName );
    lcl_GetPropertyWhich( pEntry, nItemWhich );
    if ( nItemWhich )               // item wid (from map or special case)
    {
        if ( !aRanges.empty() )     // empty = nothing to do
        {
            //! for items that have multiple properties (e.g. background)
            //! too much will be reset
            //! for ATTR_ROTATE_VALUE, reset ATTR_ORIENTATION as well?

            sal_uInt16 aWIDs[3];
            aWIDs[0] = nItemWhich;
            if ( nItemWhich == ATTR_VALUE_FORMAT )
            {
                aWIDs[1] = ATTR_LANGUAGE_FORMAT; // language for number formats
                aWIDs[2] = 0;
            }
            else
                aWIDs[1] = 0;
            pDocShell->GetDocFunc().ClearItems( *GetMarkData(), aWIDs, true );
        }
    }
    else if ( pEntry )
    {
        if ( pEntry->nWID == SC_WID_UNO_CHCOLHDR )
            bChartColAsHdr = false;
        else if ( pEntry->nWID == SC_WID_UNO_CHROWHDR )
            bChartRowAsHdr = false;
        else if ( pEntry->nWID == SC_WID_UNO_CELLSTYL )
        {
            OUString aStyleName( ScResId( STR_STYLENAME_STANDARD ) );
            pDocShell->GetDocFunc().ApplyStyle( *GetMarkData(), aStyleName, true );
        }
    }
}

uno::Any SAL_CALL ScCellRangesBase::getPropertyDefault( const OUString& aPropertyName )
{
    //! bundle with getPropertyValue

    SolarMutexGuard aGuard;
    uno::Any aAny;

    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
        const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
        if ( pEntry )
        {
            if ( IsScItemWid( pEntry->nWID ) )
            {
                const ScPatternAttr* pPattern = rDoc.GetDefPattern();
                if ( pPattern )
                {
                    const SfxItemSet& rSet = pPattern->GetItemSet();

                    switch ( pEntry->nWID )     // for item-specific handling
                    {
                        case ATTR_VALUE_FORMAT:
                            //  default has no language set
                            aAny <<= static_cast<sal_Int32>( static_cast<const SfxUInt32Item&>(rSet.Get(pEntry->nWID)).GetValue() );
                            break;
                        case ATTR_INDENT:
                            aAny <<= static_cast<sal_Int16>( TwipsToHMM(static_cast<const ScIndentItem&>(
                                            rSet.Get(pEntry->nWID)).GetValue()) );
                            break;
                        default:
                            pPropSet->getPropertyValue(aPropertyName, rSet, aAny);
                    }
                }
            }
            else
                switch ( pEntry->nWID )
                {
                    case SC_WID_UNO_CHCOLHDR:
                    case SC_WID_UNO_CHROWHDR:
                        aAny <<= false;
                        break;
                    case SC_WID_UNO_CELLSTYL:
                        aAny <<= ScStyleNameConversion::DisplayToProgrammaticName(
                                    ScResId(STR_STYLENAME_STANDARD), SfxStyleFamily::Para );
                        break;
                    case SC_WID_UNO_TBLBORD:
                    case SC_WID_UNO_TBLBORD2:
                        {
                            const ScPatternAttr* pPattern = rDoc.GetDefPattern();
                            if ( pPattern )
                            {
                                if (pEntry->nWID == SC_WID_UNO_TBLBORD2)
                                    ScHelperFunctions::AssignTableBorder2ToAny( aAny,
                                            pPattern->GetItem(ATTR_BORDER),
                                            pPattern->GetItem(ATTR_BORDER_INNER) );
                                else
                                    ScHelperFunctions::AssignTableBorderToAny( aAny,
                                            pPattern->GetItem(ATTR_BORDER),
                                            pPattern->GetItem(ATTR_BORDER_INNER) );
                            }
                        }
                        break;
                    case SC_WID_UNO_CONDFMT:
                    case SC_WID_UNO_CONDLOC:
                    case SC_WID_UNO_CONDXML:
                        {
                            bool bEnglish = ( pEntry->nWID != SC_WID_UNO_CONDLOC );
                            bool bXML = ( pEntry->nWID == SC_WID_UNO_CONDXML );
                            formula::FormulaGrammar::Grammar eGrammar = (bXML ?
                                    rDoc.GetStorageGrammar() :
                                   formula::FormulaGrammar::mapAPItoGrammar( bEnglish, bXML));

                            aAny <<= uno::Reference<sheet::XSheetConditionalEntries>(
                                    new ScTableConditionalFormat( &rDoc, 0, aRanges[0].aStart.Tab(), eGrammar ));
                        }
                        break;
                    case SC_WID_UNO_VALIDAT:
                    case SC_WID_UNO_VALILOC:
                    case SC_WID_UNO_VALIXML:
                        {
                            bool bEnglish = ( pEntry->nWID != SC_WID_UNO_VALILOC );
                            bool bXML = ( pEntry->nWID == SC_WID_UNO_VALIXML );
                            formula::FormulaGrammar::Grammar eGrammar = (bXML ?
                                    rDoc.GetStorageGrammar() :
                                   formula::FormulaGrammar::mapAPItoGrammar( bEnglish, bXML));

                            aAny <<= uno::Reference<beans::XPropertySet>(
                                    new ScTableValidationObj( rDoc, 0, eGrammar ));
                        }
                        break;
                    case SC_WID_UNO_NUMRULES:
                        {
                            aAny <<= ScStyleObj::CreateEmptyNumberingRules();
                        }
                        break;
                }
        }
    }

    return aAny;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellRangesBase::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( pPropSet->getPropertyMap() ));
    return aRef;
}

static void lcl_SetCellProperty( const SfxItemPropertySimpleEntry& rEntry, const uno::Any& rValue,
                            ScPatternAttr& rPattern, const ScDocument &rDoc,
                            sal_uInt16& rFirstItemId, sal_uInt16& rSecondItemId )
{
    rFirstItemId = rEntry.nWID;
    rSecondItemId = 0;

    SfxItemSet& rSet = rPattern.GetItemSet();
    switch ( rEntry.nWID )
    {
        case ATTR_VALUE_FORMAT:
            {
                // language for number formats
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                sal_uLong nOldFormat = rSet.Get( ATTR_VALUE_FORMAT ).GetValue();
                LanguageType eOldLang = rSet.Get( ATTR_LANGUAGE_FORMAT ).GetLanguage();
                nOldFormat = pFormatter->GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );

                sal_Int32 nIntVal = 0;
                if ( !(rValue >>= nIntVal) )
                    throw lang::IllegalArgumentException();

                sal_uLong nNewFormat = static_cast<sal_uLong>(nIntVal);
                rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );

                const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewFormat );
                LanguageType eNewLang =
                    pNewEntry ? pNewEntry->GetLanguage() : LANGUAGE_DONTKNOW;
                if ( eNewLang != eOldLang && eNewLang != LANGUAGE_DONTKNOW )
                {
                    rSet.Put( SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );

                    // if only language is changed,
                    // don't touch number format attribute
                    sal_uLong nNewMod = nNewFormat % SV_COUNTRY_LANGUAGE_OFFSET;
                    if ( nNewMod == ( nOldFormat % SV_COUNTRY_LANGUAGE_OFFSET ) &&
                         nNewMod <= SV_MAX_COUNT_STANDARD_FORMATS )
                    {
                        rFirstItemId = 0;       // don't use ATTR_VALUE_FORMAT value
                    }

                    rSecondItemId = ATTR_LANGUAGE_FORMAT;
                }

            }
            break;
        case ATTR_INDENT:
            {
                sal_Int16 nIntVal = 0;
                if ( !(rValue >>= nIntVal) )
                    throw lang::IllegalArgumentException();

                rSet.Put( ScIndentItem( static_cast<sal_uInt16>(HMMToTwips(nIntVal)) ) );

            }
            break;
        case ATTR_ROTATE_VALUE:
            {
                sal_Int32 nRotVal = 0;
                if ( !(rValue >>= nRotVal) )
                    throw lang::IllegalArgumentException();

                //  stored value is always between 0 and 360 deg.
                nRotVal %= 36000;
                if ( nRotVal < 0 )
                    nRotVal += 36000;

                rSet.Put( ScRotateValueItem( nRotVal ) );

            }
            break;
        case ATTR_STACKED:
            {
                table::CellOrientation eOrient;
                if( rValue >>= eOrient )
                {
                    switch( eOrient )
                    {
                        case table::CellOrientation_STANDARD:
                            rSet.Put( ScVerticalStackCell( false ) );
                        break;
                        case table::CellOrientation_TOPBOTTOM:
                            rSet.Put( ScVerticalStackCell( false ) );
                            rSet.Put( ScRotateValueItem( 27000 ) );
                            rSecondItemId = ATTR_ROTATE_VALUE;
                        break;
                        case table::CellOrientation_BOTTOMTOP:
                            rSet.Put( ScVerticalStackCell( false ) );
                            rSet.Put( ScRotateValueItem( 9000 ) );
                            rSecondItemId = ATTR_ROTATE_VALUE;
                        break;
                        case table::CellOrientation_STACKED:
                            rSet.Put( ScVerticalStackCell( true ) );
                        break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
            }
            break;
        default:
            {
                lcl_GetCellsPropertySet()->setPropertyValue(rEntry, rValue, rSet);
            }
    }
}

void SAL_CALL ScCellRangesBase::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if ( !pDocShell || aRanges.empty() )
        throw uno::RuntimeException();

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException(aPropertyName);

    SetOnePropertyValue( pEntry, aValue );
}

void ScCellRangesBase::SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, const uno::Any& aValue )
{
    if ( !pEntry )
        return;

    if ( IsScItemWid( pEntry->nWID ) )
    {
        if ( !aRanges.empty() )     // empty = nothing to do
        {
            ScDocument& rDoc = pDocShell->GetDocument();

            //  For parts of compound items with multiple properties (e.g. background)
            //  the old item has to be first fetched from the document.
            //! But we can't recognize this case here
            //! -> an extra flag in PropertyMap entry, or something like that???
            //! fetch the item directly from its position in the range?
            //  ClearInvalidItems, so that in any case we have an item with the correct type

            ScPatternAttr aPattern( *GetCurrentAttrsDeep() );
            SfxItemSet& rSet = aPattern.GetItemSet();
            rSet.ClearInvalidItems();

            sal_uInt16 nFirstItem, nSecondItem;
            lcl_SetCellProperty( *pEntry, aValue, aPattern, rDoc, nFirstItem, nSecondItem );

            for (sal_uInt16 nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; nWhich++)
                if ( nWhich != nFirstItem && nWhich != nSecondItem )
                    rSet.ClearItem(nWhich);

            pDocShell->GetDocFunc().ApplyAttributes( *GetMarkData(), aPattern, true );
        }
    }
    else        // implemented here
        switch ( pEntry->nWID )
        {
            case EE_CHAR_ESCAPEMENT:    // Specifically for xlsx import
            {
                sal_Int32 nValue = 0;
                aValue >>= nValue;
                if (nValue)
                {
                    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
                    {
                        ScRange const & rRange = aRanges[i];

                        /* TODO: Iterate through the range */
                        ScAddress aAddr = rRange.aStart;
                        ScDocument& rDoc = pDocShell->GetDocument();
                        ScRefCellValue aCell(rDoc, aAddr);

                        OUString aStr = aCell.getString(&rDoc);
                        EditEngine aEngine( rDoc.GetEnginePool() );
                        aEngine.SetEditTextObjectPool(rDoc.GetEditPool());

                        /* EE_CHAR_ESCAPEMENT seems to be set on the cell _only_ when
                         * there are no other attribs for the cell.
                         * So, it is safe to overwrite the complete attribute set.
                         * If there is a need - getting CellType and processing
                         * the attributes could be considered.
                         */
                        SfxItemSet aAttr = aEngine.GetEmptyItemSet();
                        aEngine.SetText(aStr);
                        if( nValue < 0 )    // Subscript
                            aAttr.Put( SvxEscapementItem( SvxEscapement::Subscript, EE_CHAR_ESCAPEMENT ) );
                        else                // Superscript
                            aAttr.Put( SvxEscapementItem( SvxEscapement::Superscript, EE_CHAR_ESCAPEMENT ) );
                        aEngine.QuickSetAttribs(aAttr, ESelection(0, 0, 0, aStr.getLength()));

                        // The cell will own the text object instance.
                        rDoc.SetEditText(aRanges[0].aStart, aEngine.CreateTextObject());
                    }
                }
            }
            break;
            case SC_WID_UNO_CHCOLHDR:
                // chart header flags are set for this object, not stored with document
                bChartColAsHdr = ScUnoHelpFunctions::GetBoolFromAny( aValue );
                break;
            case SC_WID_UNO_CHROWHDR:
                bChartRowAsHdr = ScUnoHelpFunctions::GetBoolFromAny( aValue );
                break;
            case SC_WID_UNO_CELLSTYL:
                {
                    OUString aStrVal;
                    aValue >>= aStrVal;
                    OUString aString(ScStyleNameConversion::ProgrammaticToDisplayName(
                                                        aStrVal, SfxStyleFamily::Para ));
                    pDocShell->GetDocFunc().ApplyStyle( *GetMarkData(), aString, true );
                }
                break;
            case SC_WID_UNO_TBLBORD:
                {
                    table::TableBorder aBorder;
                    if ( !aRanges.empty() && ( aValue >>= aBorder ) )   // empty = nothing to do
                    {
                        SvxBoxItem aOuter(ATTR_BORDER);
                        SvxBoxInfoItem aInner(ATTR_BORDER_INNER);
                        ScHelperFunctions::FillBoxItems( aOuter, aInner, aBorder );

                        ScHelperFunctions::ApplyBorder( pDocShell, aRanges, aOuter, aInner );   //! docfunc
                    }
                }
                break;
            case SC_WID_UNO_TBLBORD2:
                {
                    table::TableBorder2 aBorder2;
                    if ( !aRanges.empty() && ( aValue >>= aBorder2 ) )   // empty = nothing to do
                    {
                        SvxBoxItem aOuter(ATTR_BORDER);
                        SvxBoxInfoItem aInner(ATTR_BORDER_INNER);
                        ScHelperFunctions::FillBoxItems( aOuter, aInner, aBorder2 );

                        ScHelperFunctions::ApplyBorder( pDocShell, aRanges, aOuter, aInner );   //! docfunc
                    }
                }
                break;
            case SC_WID_UNO_CONDFMT:
            case SC_WID_UNO_CONDLOC:
            case SC_WID_UNO_CONDXML:
                {
                    uno::Reference<sheet::XSheetConditionalEntries> xInterface(aValue, uno::UNO_QUERY);
                    if ( !aRanges.empty() && xInterface.is() )  // empty = nothing to do
                    {
                        ScTableConditionalFormat* pFormat =
                                comphelper::getUnoTunnelImplementation<ScTableConditionalFormat>( xInterface );
                        if (pFormat)
                        {
                            ScDocument& rDoc = pDocShell->GetDocument();
                            bool bEnglish = ( pEntry->nWID != SC_WID_UNO_CONDLOC );
                            bool bXML = ( pEntry->nWID == SC_WID_UNO_CONDXML );
                            formula::FormulaGrammar::Grammar eGrammar = (bXML ?
                                   formula::FormulaGrammar::GRAM_UNSPECIFIED :
                                   formula::FormulaGrammar::mapAPItoGrammar( bEnglish, bXML));

                            SCTAB nTab = aRanges.front().aStart.Tab();
                            // To remove conditional formats for all cells in aRanges we need to:
                            // Remove conditional format data from cells' attributes
                            rDoc.RemoveCondFormatData( aRanges, nTab,  0 );
                            // And also remove ranges from conditional formats list
                            for (size_t i = 0; i < aRanges.size(); ++i)
                            {
                                rDoc.GetCondFormList( aRanges[i].aStart.Tab() )->DeleteArea(
                                    aRanges[i].aStart.Col(), aRanges[i].aStart.Row(),
                                    aRanges[i].aEnd.Col(), aRanges[i].aEnd.Row() );
                            }

                            // Then we can apply new conditional format if there is one
                            if (pFormat->getCount())
                            {
                                auto pNew = std::make_unique<ScConditionalFormat>( 0, &rDoc );    // Index will be set on inserting
                                pFormat->FillFormat( *pNew, rDoc, eGrammar );
                                pNew->SetRange( aRanges );
                                pDocShell->GetDocFunc().ReplaceConditionalFormat( 0, std::move(pNew), nTab, aRanges );
                            }

                            // and repaint
                            for (size_t i = 0; i < aRanges.size(); ++i)
                                pDocShell->PostPaint(aRanges[i], PaintPartFlags::Grid);
                            pDocShell->SetDocumentModified();
                        }
                    }
                }
                break;
            case SC_WID_UNO_VALIDAT:
            case SC_WID_UNO_VALILOC:
            case SC_WID_UNO_VALIXML:
                {
                    uno::Reference<beans::XPropertySet> xInterface(aValue, uno::UNO_QUERY);
                    if ( !aRanges.empty() && xInterface.is() )  // empty = nothing to do
                    {
                        ScTableValidationObj* pValidObj =
                                comphelper::getUnoTunnelImplementation<ScTableValidationObj>( xInterface );
                        if (pValidObj)
                        {
                            ScDocument& rDoc = pDocShell->GetDocument();
                            bool bEnglish = ( pEntry->nWID != SC_WID_UNO_VALILOC );
                            bool bXML = ( pEntry->nWID == SC_WID_UNO_VALIXML );
                            formula::FormulaGrammar::Grammar eGrammar = (bXML ?
                                   formula::FormulaGrammar::GRAM_UNSPECIFIED :
                                   formula::FormulaGrammar::mapAPItoGrammar( bEnglish, bXML));

                            std::unique_ptr<ScValidationData> pNewData(
                                    pValidObj->CreateValidationData( rDoc, eGrammar ));
                            sal_uLong nIndex = rDoc.AddValidationEntry( *pNewData );
                            pNewData.reset();

                            ScPatternAttr aPattern( rDoc.GetPool() );
                            aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nIndex ) );
                            pDocShell->GetDocFunc().ApplyAttributes( *GetMarkData(), aPattern, true );
                        }
                    }
                }
                break;
            // SC_WID_UNO_NUMRULES is ignored...
        }
}

uno::Any SAL_CALL ScCellRangesBase::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    if ( !pDocShell || aRanges.empty() )
        throw uno::RuntimeException();

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException(aPropertyName);

    uno::Any aAny;
    GetOnePropertyValue( pEntry, aAny );
    return aAny;
}

void ScCellRangesBase::GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, uno::Any& rAny )
{
    if ( !pEntry )
        return;

    if ( IsScItemWid( pEntry->nWID ) )
    {
        SfxItemSet* pDataSet = GetCurrentDataSet();
        if ( pDataSet )
        {
            switch ( pEntry->nWID )     // for special handling of items
            {
                case ATTR_VALUE_FORMAT:
                    {
                        ScDocument& rDoc = pDocShell->GetDocument();

                        sal_uLong nOldFormat =
                                pDataSet->Get( ATTR_VALUE_FORMAT ).GetValue();
                        LanguageType eOldLang =
                                pDataSet->Get( ATTR_LANGUAGE_FORMAT ).GetLanguage();
                        nOldFormat = rDoc.GetFormatTable()->
                                GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );
                        rAny <<= static_cast<sal_Int32>(nOldFormat);
                    }
                    break;
                case ATTR_INDENT:
                    rAny <<= static_cast<sal_Int16>( TwipsToHMM(static_cast<const ScIndentItem&>(
                                    pDataSet->Get(pEntry->nWID)).GetValue()) );
                    break;
                case ATTR_STACKED:
                    {
                        sal_Int32 nRot = pDataSet->Get(ATTR_ROTATE_VALUE).GetValue();
                        bool bStacked = static_cast<const ScVerticalStackCell&>(pDataSet->Get(pEntry->nWID)).GetValue();
                        SvxOrientationItem( nRot, bStacked, 0 ).QueryValue( rAny );
                    }
                    break;
                default:
                    pPropSet->getPropertyValue(*pEntry, *pDataSet, rAny);
            }
        }
    }
    else        // implemented here
        switch ( pEntry->nWID )
        {
            case SC_WID_UNO_CHCOLHDR:
                rAny <<= bChartColAsHdr;
                break;
            case SC_WID_UNO_CHROWHDR:
                rAny <<= bChartRowAsHdr;
                break;
            case SC_WID_UNO_CELLSTYL:
                {
                    OUString aStyleName;
                    const ScStyleSheet* pStyle = pDocShell->GetDocument().GetSelectionStyle(*GetMarkData());
                    if (pStyle)
                        aStyleName = pStyle->GetName();
                    rAny <<= ScStyleNameConversion::DisplayToProgrammaticName(
                                                            aStyleName, SfxStyleFamily::Para );
                }
                break;
            case SC_WID_UNO_TBLBORD:
            case SC_WID_UNO_TBLBORD2:
                {
                    //! loop through all ranges
                    if ( !aRanges.empty() )
                    {
                        const ScRange & rFirst = aRanges[ 0 ];
                        SvxBoxItem aOuter(ATTR_BORDER);
                        SvxBoxInfoItem aInner(ATTR_BORDER_INNER);

                        ScDocument& rDoc = pDocShell->GetDocument();
                        ScMarkData aMark(rDoc.GetSheetLimits());
                        aMark.SetMarkArea( rFirst );
                        aMark.SelectTable( rFirst.aStart.Tab(), true );
                        rDoc.GetSelectionFrame( aMark, aOuter, aInner );

                        if (pEntry->nWID == SC_WID_UNO_TBLBORD2)
                            ScHelperFunctions::AssignTableBorder2ToAny( rAny, aOuter, aInner);
                        else
                            ScHelperFunctions::AssignTableBorderToAny( rAny, aOuter, aInner);
                    }
                }
                break;
            case SC_WID_UNO_CONDFMT:
            case SC_WID_UNO_CONDLOC:
            case SC_WID_UNO_CONDXML:
                {
                    const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
                    if ( pPattern )
                    {
                        ScDocument& rDoc = pDocShell->GetDocument();
                        bool bEnglish = ( pEntry->nWID != SC_WID_UNO_CONDLOC );
                        bool bXML = ( pEntry->nWID == SC_WID_UNO_CONDXML );
                        formula::FormulaGrammar::Grammar eGrammar = (bXML ?
                                rDoc.GetStorageGrammar() :
                               formula::FormulaGrammar::mapAPItoGrammar( bEnglish, bXML));
                        const ScCondFormatIndexes& rIndex =
                                pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData();
                        sal_uLong nIndex = 0;
                        if(!rIndex.empty())
                            nIndex = rIndex[0];
                        rAny <<= uno::Reference<sheet::XSheetConditionalEntries>(
                                new ScTableConditionalFormat( &rDoc, nIndex, aRanges.front().aStart.Tab(), eGrammar ));
                    }
                }
                break;
            case SC_WID_UNO_VALIDAT:
            case SC_WID_UNO_VALILOC:
            case SC_WID_UNO_VALIXML:
                {
                    const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
                    if ( pPattern )
                    {
                        ScDocument& rDoc = pDocShell->GetDocument();
                        bool bEnglish = ( pEntry->nWID != SC_WID_UNO_VALILOC );
                        bool bXML = ( pEntry->nWID == SC_WID_UNO_VALIXML );
                        formula::FormulaGrammar::Grammar eGrammar = (bXML ?
                                rDoc.GetStorageGrammar() :
                               formula::FormulaGrammar::mapAPItoGrammar( bEnglish, bXML));
                        sal_uLong nIndex =
                                pPattern->GetItem(ATTR_VALIDDATA).GetValue();
                        rAny <<= uno::Reference<beans::XPropertySet>(
                                new ScTableValidationObj( rDoc, nIndex, eGrammar ));
                    }
                }
                break;
            case SC_WID_UNO_NUMRULES:
                {
                    // always return empty numbering rules object
                    rAny <<= ScStyleObj::CreateEmptyNumberingRules();
                }
                break;
            case SC_WID_UNO_ABSNAME:
                {
                    OUString sRet;
                    aRanges.Format(sRet, ScRefFlags::RANGE_ABS_3D, pDocShell->GetDocument());
                    rAny <<= sRet;
                }
            break;
            case SC_WID_UNO_FORMATID:
                {
                    const ScPatternAttr* pPattern = GetCurrentAttrsFlat();
                    rAny <<= pPattern->GetKey();
                }
            break;
        }
}

void SAL_CALL ScCellRangesBase::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SolarMutexGuard aGuard;
    if ( aRanges.empty() )
        throw uno::RuntimeException();

    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellRangesBase::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SolarMutexGuard aGuard;
    if ( aRanges.empty() )
        throw uno::RuntimeException();

    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellRangesBase::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellRangesBase::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    OSL_FAIL("not implemented");
}

// XMultiPropertySet

void SAL_CALL ScCellRangesBase::setPropertyValues( const uno::Sequence< OUString >& aPropertyNames,
                                    const uno::Sequence< uno::Any >& aValues )
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount(aPropertyNames.getLength());
    sal_Int32 nValues(aValues.getLength());
    if (nCount != nValues)
        throw lang::IllegalArgumentException();

    if ( !(pDocShell && nCount) )
        return;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();      // from derived class
    const OUString* pNames = aPropertyNames.getConstArray();
    const uno::Any* pValues = aValues.getConstArray();

    std::unique_ptr<const SfxItemPropertySimpleEntry*[]> pEntryArray(new const SfxItemPropertySimpleEntry*[nCount]);

    sal_Int32 i;
    for(i = 0; i < nCount; i++)
    {
        // first loop: find all properties in map, but handle only CellStyle
        // (CellStyle must be set before any other cell properties)

        const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( pNames[i] );
        pEntryArray[i] = pEntry;
        if (pEntry)
        {
            if ( pEntry->nWID == SC_WID_UNO_CELLSTYL )
            {
                try
                {
                    SetOnePropertyValue( pEntry, pValues[i] );
                }
                catch ( lang::IllegalArgumentException& )
                {
                    OSL_FAIL("exception when setting cell style");     // not supposed to happen
                }
            }
        }
    }

    ScDocument& rDoc = pDocShell->GetDocument();
    std::unique_ptr<ScPatternAttr> pOldPattern;
    std::unique_ptr<ScPatternAttr> pNewPattern;

    for(i = 0; i < nCount; i++)
    {
        // second loop: handle other properties

        const SfxItemPropertySimpleEntry* pEntry = pEntryArray[i];
        if ( pEntry )
        {
            if ( IsScItemWid( pEntry->nWID ) )  // can be handled by SfxItemPropertySet
            {
                if ( !pOldPattern )
                {
                    pOldPattern.reset(new ScPatternAttr( *GetCurrentAttrsDeep() ));
                    pOldPattern->GetItemSet().ClearInvalidItems();
                    pNewPattern.reset(new ScPatternAttr( rDoc.GetPool() ));
                }

                //  collect items in pNewPattern, apply with one call after the loop

                sal_uInt16 nFirstItem, nSecondItem;
                lcl_SetCellProperty( *pEntry, pValues[i], *pOldPattern, rDoc, nFirstItem, nSecondItem );

                //  put only affected items into new set
                if ( nFirstItem )
                    pNewPattern->GetItemSet().Put( pOldPattern->GetItemSet().Get( nFirstItem ) );
                if ( nSecondItem )
                    pNewPattern->GetItemSet().Put( pOldPattern->GetItemSet().Get( nSecondItem ) );
            }
            else if ( pEntry->nWID != SC_WID_UNO_CELLSTYL )   // CellStyle is handled above
            {
                //  call virtual method to set a single property
                SetOnePropertyValue( pEntry, pValues[i] );
            }
        }
    }

    if ( pNewPattern && !aRanges.empty() )
        pDocShell->GetDocFunc().ApplyAttributes( *GetMarkData(), *pNewPattern, true );
}

uno::Sequence<uno::Any> SAL_CALL ScCellRangesBase::getPropertyValues(
                                const uno::Sequence< OUString >& aPropertyNames )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class

    uno::Sequence<uno::Any> aRet(aPropertyNames.getLength());
    uno::Any* pProperties = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyNames[i] );
        GetOnePropertyValue( pEntry, pProperties[i] );
    }
    return aRet;
}

void SAL_CALL ScCellRangesBase::addPropertiesChangeListener( const uno::Sequence< OUString >& /* aPropertyNames */,
                                    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellRangesBase::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellRangesBase::firePropertiesChangeEvent( const uno::Sequence< OUString >& /* aPropertyNames */,
                                    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

IMPL_LINK( ScCellRangesBase, ValueListenerHdl, const SfxHint&, rHint, void )
{
    if ( pDocShell && (rHint.GetId() == SfxHintId::ScDataChanged))
    {
        //  This may be called several times for a single change, if several formulas
        //  in the range are notified. So only a flag is set that is checked when
        //  SfxHintId::DataChanged is received.

        bGotDataChangedHint = true;
    }
}

// XTolerantMultiPropertySet
uno::Sequence< beans::SetPropertyTolerantFailed > SAL_CALL ScCellRangesBase::setPropertyValuesTolerant( const uno::Sequence< OUString >& aPropertyNames,
                                    const uno::Sequence< uno::Any >& aValues )
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount(aPropertyNames.getLength());
    sal_Int32 nValues(aValues.getLength());
    if (nCount != nValues)
        throw lang::IllegalArgumentException();

    if ( pDocShell && nCount )
    {
        uno::Sequence < beans::SetPropertyTolerantFailed > aReturns(nCount);
        beans::SetPropertyTolerantFailed* pReturns = aReturns.getArray();

        const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class
        const OUString* pNames = aPropertyNames.getConstArray();
        const uno::Any* pValues = aValues.getConstArray();

        std::unique_ptr<const SfxItemPropertySimpleEntry*[]> pMapArray(new const SfxItemPropertySimpleEntry*[nCount]);

        sal_Int32 i;
        for(i = 0; i < nCount; i++)
        {
            // first loop: find all properties in map, but handle only CellStyle
            // (CellStyle must be set before any other cell properties)

            const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( pNames[i] );
            pMapArray[i] = pEntry;
            if (pEntry)
            {
                if ( pEntry->nWID == SC_WID_UNO_CELLSTYL )
                {
                    try
                    {
                        SetOnePropertyValue( pEntry, pValues[i] );
                    }
                    catch ( lang::IllegalArgumentException& )
                    {
                        OSL_FAIL("exception when setting cell style");     // not supposed to happen
                    }
                }
            }
        }

        ScDocument& rDoc = pDocShell->GetDocument();
        std::unique_ptr<ScPatternAttr> pOldPattern;
        std::unique_ptr<ScPatternAttr> pNewPattern;

        sal_Int32 nFailed(0);
        for(i = 0; i < nCount; i++)
        {
            // second loop: handle other properties

            const SfxItemPropertySimpleEntry* pEntry = pMapArray[i];
            if ( pEntry && ((pEntry->nFlags & beans::PropertyAttribute::READONLY) == 0))
            {
                if ( IsScItemWid( pEntry->nWID ) )  // can be handled by SfxItemPropertySet
                {
                    if ( !pOldPattern )
                    {
                        pOldPattern.reset(new ScPatternAttr( *GetCurrentAttrsDeep() ));
                        pOldPattern->GetItemSet().ClearInvalidItems();
                        pNewPattern.reset(new ScPatternAttr( rDoc.GetPool() ));
                    }

                    //  collect items in pNewPattern, apply with one call after the loop
                    try
                    {
                        sal_uInt16 nFirstItem, nSecondItem;
                        lcl_SetCellProperty( *pEntry, pValues[i], *pOldPattern, rDoc, nFirstItem, nSecondItem );

                        //  put only affected items into new set
                        if ( nFirstItem )
                            pNewPattern->GetItemSet().Put( pOldPattern->GetItemSet().Get( nFirstItem ) );
                        if ( nSecondItem )
                            pNewPattern->GetItemSet().Put( pOldPattern->GetItemSet().Get( nSecondItem ) );
                    }
                    catch ( lang::IllegalArgumentException& )
                    {
                        pReturns[nFailed].Name = pNames[i];
                        pReturns[nFailed++].Result = beans::TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
                    }
                }
                else if ( pEntry->nWID != SC_WID_UNO_CELLSTYL )   // CellStyle is handled above
                {
                    //  call virtual method to set a single property
                    try
                    {
                        SetOnePropertyValue( pEntry, pValues[i] );
                    }
                    catch ( lang::IllegalArgumentException& )
                    {
                        pReturns[nFailed].Name = pNames[i];
                        pReturns[nFailed++].Result = beans::TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
                    }
                }
            }
            else
            {
                pReturns[nFailed].Name = pNames[i];
                if (pEntry)
                    pReturns[nFailed++].Result = beans::TolerantPropertySetResultType::PROPERTY_VETO;
                else
                    pReturns[nFailed++].Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            }
        }

        if ( pNewPattern && !aRanges.empty() )
            pDocShell->GetDocFunc().ApplyAttributes( *GetMarkData(), *pNewPattern, true );

        aReturns.realloc(nFailed);

        return aReturns;
    }
    return uno::Sequence < beans::SetPropertyTolerantFailed >();
}

uno::Sequence< beans::GetPropertyTolerantResult > SAL_CALL ScCellRangesBase::getPropertyValuesTolerant( const uno::Sequence< OUString >& aPropertyNames )
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount(aPropertyNames.getLength());
    uno::Sequence < beans::GetPropertyTolerantResult > aReturns(nCount);
    beans::GetPropertyTolerantResult* pReturns = aReturns.getArray();

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class

    for(sal_Int32 i = 0; i < nCount; i++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyNames[i] );
        if (!pEntry)
        {
            pReturns[i].Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        else
        {
            sal_uInt16 nItemWhich = 0;
            lcl_GetPropertyWhich( pEntry, nItemWhich );
            pReturns[i].State = GetOnePropertyState( nItemWhich, pEntry );
            GetOnePropertyValue( pEntry, pReturns[i].Value );
            pReturns[i].Result = beans::TolerantPropertySetResultType::SUCCESS;
        }
    }
    return aReturns;
}

uno::Sequence< beans::GetDirectPropertyTolerantResult > SAL_CALL ScCellRangesBase::getDirectPropertyValuesTolerant( const uno::Sequence< OUString >& aPropertyNames )
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount(aPropertyNames.getLength());
    uno::Sequence < beans::GetDirectPropertyTolerantResult > aReturns(nCount);
    beans::GetDirectPropertyTolerantResult* pReturns = aReturns.getArray();

    const SfxItemPropertyMap& rPropertyMap = GetItemPropertyMap();     // from derived class

    sal_Int32 j = 0;
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyNames[i] );
        if (!pEntry)
        {
            pReturns[i].Result = beans::TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        else
        {
            sal_uInt16 nItemWhich = 0;
            lcl_GetPropertyWhich( pEntry, nItemWhich );
            pReturns[j].State = GetOnePropertyState( nItemWhich, pEntry );
            if (pReturns[j].State == beans::PropertyState_DIRECT_VALUE)
            {
                GetOnePropertyValue( pEntry, pReturns[j].Value );
                pReturns[j].Result = beans::TolerantPropertySetResultType::SUCCESS;
                pReturns[j].Name = aPropertyNames[i];
                ++j;
            }
        }
    }
    if (j < nCount)
        aReturns.realloc(j);
    return aReturns;
}

// XIndent

void SAL_CALL ScCellRangesBase::decrementIndent()
{
    SolarMutexGuard aGuard;
    if ( pDocShell && !aRanges.empty() )
    {
        //#97041#; put only MultiMarked ScMarkData in ChangeIndent
        ScMarkData aMarkData(*GetMarkData());
        aMarkData.MarkToMulti();
        pDocShell->GetDocFunc().ChangeIndent( aMarkData, false, true );
    }
}

void SAL_CALL ScCellRangesBase::incrementIndent()
{
    SolarMutexGuard aGuard;
    if ( pDocShell && !aRanges.empty() )
    {
        //#97041#; put only MultiMarked ScMarkData in ChangeIndent
        ScMarkData aMarkData(*GetMarkData());
        aMarkData.MarkToMulti();
        pDocShell->GetDocFunc().ChangeIndent( aMarkData, true, true );
    }
}

// XChartData

std::unique_ptr<ScMemChart> ScCellRangesBase::CreateMemChart_Impl() const
{
    if ( pDocShell && !aRanges.empty() )
    {
        ScRangeListRef xChartRanges;
        if ( aRanges.size() == 1 )
        {
            //  set useful table limit (only occupied data area)
            //  (only here - Listeners are registered for the whole area)
            //! check immediately if a ScTableSheetObj?

            const ScDocument & rDoc = pDocShell->GetDocument();
            const ScRange & rRange = aRanges[0];
            if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == rDoc.MaxCol() &&
                 rRange.aStart.Row() == 0 && rRange.aEnd.Row() == rDoc.MaxRow() )
            {
                SCTAB nTab = rRange.aStart.Tab();

                SCCOL nStartX;
                SCROW nStartY; // Get start
                if (!pDocShell->GetDocument().GetDataStart( nTab, nStartX, nStartY ))
                {
                    nStartX = 0;
                    nStartY = 0;
                }

                SCCOL nEndX;
                SCROW nEndY; // Get end
                if (!pDocShell->GetDocument().GetTableArea( nTab, nEndX, nEndY ))
                {
                    nEndX = 0;
                    nEndY = 0;
                }

                xChartRanges = new ScRangeList( ScRange( nStartX, nStartY, nTab, nEndX, nEndY, nTab ) );
            }
        }
        if (!xChartRanges.is())         //  otherwise take Ranges directly
            xChartRanges = new ScRangeList(aRanges);
        ScChartArray aArr( pDocShell->GetDocument(), xChartRanges );

        // RowAsHdr = ColHeaders and vice versa
        aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );

        return aArr.CreateMemChart();
    }
    return nullptr;
}

uno::Sequence< uno::Sequence<double> > SAL_CALL ScCellRangesBase::getData()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<ScMemChart> pMemChart(CreateMemChart_Impl());
    if ( pMemChart )
    {
        sal_Int32 nColCount = pMemChart->GetColCount();
        sal_Int32 nRowCount = static_cast<sal_Int32>(pMemChart->GetRowCount());

        uno::Sequence< uno::Sequence<double> > aRowSeq( nRowCount );
        uno::Sequence<double>* pRowAry = aRowSeq.getArray();
        for (sal_Int32 nRow = 0; nRow < nRowCount; nRow++)
        {
            uno::Sequence<double> aColSeq( nColCount );
            double* pColAry = aColSeq.getArray();
            for (sal_Int32 nCol = 0; nCol < nColCount; nCol++)
                pColAry[nCol] = pMemChart->GetData( nCol, nRow );

            pRowAry[nRow] = aColSeq;
        }

        return aRowSeq;
    }

    return uno::Sequence< uno::Sequence<double> >(0);
}

ScRangeListRef ScCellRangesBase::GetLimitedChartRanges_Impl( sal_Int32 nDataColumns, sal_Int32 nDataRows ) const
{
    if ( aRanges.size() == 1 )
    {
        const ScDocument & rDoc = pDocShell->GetDocument();
        const ScRange & rRange = aRanges[0];
        if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == rDoc.MaxCol() &&
             rRange.aStart.Row() == 0 && rRange.aEnd.Row() == rDoc.MaxRow() )
        {
            //  if aRanges is a complete sheet, limit to given size

            SCTAB nTab = rRange.aStart.Tab();

            sal_Int32 nEndColumn = nDataColumns - 1 + ( bChartColAsHdr ? 1 : 0 );
            if ( nEndColumn < 0 )
                nEndColumn = 0;
            if ( nEndColumn > rDoc.MaxCol() )
                nEndColumn = rDoc.MaxCol();

            sal_Int32 nEndRow = nDataRows - 1 + ( bChartRowAsHdr ? 1 : 0 );
            if ( nEndRow < 0 )
                nEndRow = 0;
            if ( nEndRow > rDoc.MaxRow() )
                nEndRow = rDoc.MaxRow();

            ScRangeListRef xChartRanges = new ScRangeList( ScRange( 0, 0, nTab, static_cast<SCCOL>(nEndColumn), static_cast<SCROW>(nEndRow), nTab ) );
            return xChartRanges;
        }
    }

    return new ScRangeList(aRanges);        // as-is
}

void SAL_CALL ScCellRangesBase::setData( const uno::Sequence< uno::Sequence<double> >& aData )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    sal_Int32 nRowCount = aData.getLength();
    sal_Int32 nColCount = nRowCount ? aData[0].getLength() : 0;
    ScRangeListRef xChartRanges = GetLimitedChartRanges_Impl( nColCount, nRowCount );
    if ( pDocShell && xChartRanges.is() )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScChartArray aArr( rDoc, xChartRanges );
        aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );      // RowAsHdr = ColHeaders
        const ScChartPositionMap* pPosMap = aArr.GetPositionMap();
        if (pPosMap)
        {
            if ( pPosMap->GetColCount() == static_cast<SCCOL>(nColCount) &&
                 pPosMap->GetRowCount() == static_cast<SCROW>(nRowCount) )
            {
                for (sal_Int32 nRow=0; nRow<nRowCount; nRow++)
                {
                    const uno::Sequence<double>& rRowSeq = aData[nRow];
                    const double* pArray = rRowSeq.getConstArray();
                    nColCount = rRowSeq.getLength();
                    for (sal_Int32 nCol=0; nCol<nColCount; nCol++)
                    {
                        const ScAddress* pPos = pPosMap->GetPosition(
                                sal::static_int_cast<SCCOL>(nCol),
                                sal::static_int_cast<SCROW>(nRow) );
                        if (pPos)
                        {
                            double fVal = pArray[nCol];
                            if ( fVal == DBL_MIN )
                                rDoc.SetEmptyCell(*pPos);
                            else
                                rDoc.SetValue(*pPos, pArray[nCol]);
                        }
                    }
                }

                //! undo
                PaintGridRanges_Impl();
                pDocShell->SetDocumentModified();
                ForceChartListener_Impl();          // call listeners for this object synchronously
                bDone = true;
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();
}

uno::Sequence<OUString> SAL_CALL ScCellRangesBase::getRowDescriptions()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<ScMemChart> pMemChart(CreateMemChart_Impl());
    if ( pMemChart )
    {
        sal_Int32 nRowCount = static_cast<sal_Int32>(pMemChart->GetRowCount());
        uno::Sequence<OUString> aSeq( nRowCount );
        OUString* pAry = aSeq.getArray();
        for (sal_Int32 nRow = 0; nRow < nRowCount; nRow++)
            pAry[nRow] = pMemChart->GetRowText(nRow);

        return aSeq;
    }
    return uno::Sequence<OUString>(0);
}

void SAL_CALL ScCellRangesBase::setRowDescriptions(
                        const uno::Sequence<OUString>& aRowDescriptions )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if ( bChartColAsHdr )
    {
        sal_Int32 nRowCount = aRowDescriptions.getLength();
        ScRangeListRef xChartRanges = GetLimitedChartRanges_Impl( 1, nRowCount );
        if ( pDocShell && xChartRanges.is() )
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            ScChartArray aArr( rDoc, xChartRanges );
            aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );      // RowAsHdr = ColHeaders
            const ScChartPositionMap* pPosMap = aArr.GetPositionMap();
            if (pPosMap)
            {
                if ( pPosMap->GetRowCount() == static_cast<SCROW>(nRowCount) )
                {
                    const OUString* pArray = aRowDescriptions.getConstArray();
                    for (sal_Int32 nRow=0; nRow<nRowCount; nRow++)
                    {
                        const ScAddress* pPos = pPosMap->GetRowHeaderPosition(
                                static_cast<SCSIZE>(nRow) );
                        if (pPos)
                        {
                            const OUString& aStr = pArray[nRow];
                            if (aStr.isEmpty())
                                rDoc.SetEmptyCell(*pPos);
                            else
                            {
                                ScSetStringParam aParam;
                                aParam.setTextInput();
                                rDoc.SetString(*pPos, aStr, &aParam);
                            }
                        }
                    }

                    //! undo
                    PaintGridRanges_Impl();
                    pDocShell->SetDocumentModified();
                    ForceChartListener_Impl();          // call listeners for this object synchronously
                    bDone = true;
                }
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();
}

uno::Sequence<OUString> SAL_CALL ScCellRangesBase::getColumnDescriptions()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<ScMemChart> pMemChart(CreateMemChart_Impl());
    if ( pMemChart )
    {
        sal_Int32 nColCount = pMemChart->GetColCount();
        uno::Sequence<OUString> aSeq( nColCount );
        OUString* pAry = aSeq.getArray();
        for (sal_Int32 nCol = 0; nCol < nColCount; nCol++)
            pAry[nCol] = pMemChart->GetColText(nCol);

        return aSeq;
    }
    return uno::Sequence<OUString>(0);
}

void SAL_CALL ScCellRangesBase::setColumnDescriptions(
    const uno::Sequence<OUString>& aColumnDescriptions )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if ( bChartRowAsHdr )
    {
        sal_Int32 nColCount = aColumnDescriptions.getLength();
        ScRangeListRef xChartRanges = GetLimitedChartRanges_Impl( nColCount, 1 );
        if ( pDocShell && xChartRanges.is() )
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            ScChartArray aArr( rDoc, xChartRanges );
            aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );      // RowAsHdr = ColHeaders
            const ScChartPositionMap* pPosMap = aArr.GetPositionMap();
            if (pPosMap)
            {
                if ( pPosMap->GetColCount() == static_cast<SCCOL>(nColCount) )
                {
                    const OUString* pArray = aColumnDescriptions.getConstArray();
                    for (sal_Int32 nCol=0; nCol<nColCount; nCol++)
                    {
                        const ScAddress* pPos = pPosMap->GetColHeaderPosition(
                            sal::static_int_cast<SCCOL>(nCol) );
                        if (pPos)
                        {
                            const OUString& aStr = pArray[nCol];
                            if (aStr.isEmpty())
                                rDoc.SetEmptyCell(*pPos);
                            else
                            {
                                ScSetStringParam aParam;
                                aParam.setTextInput();
                                rDoc.SetString(*pPos, aStr, &aParam);
                            }
                        }
                    }

                    //! undo
                    PaintGridRanges_Impl();
                    pDocShell->SetDocumentModified();
                    ForceChartListener_Impl();          // call listeners for this object synchronously
                    bDone = true;
                }
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();
}

void ScCellRangesBase::ForceChartListener_Impl()
{
    //  call Update immediately so the caller to setData etc. can
    //  recognize the listener call

    if (!pDocShell)
        return;

    ScChartListenerCollection* pColl = pDocShell->GetDocument().GetChartListenerCollection();
    if (!pColl)
        return;

    ScChartListenerCollection::ListenersType& rListeners = pColl->getListeners();
    for (auto const& it : rListeners)
    {
        ScChartListener *const p = it.second.get();
        assert(p);
        if (p->GetUnoSource() == static_cast<chart::XChartData*>(this) && p->IsDirty())
            p->Update();
    }
}

void SAL_CALL ScCellRangesBase::addChartDataChangeEventListener( const uno::Reference<
                                    chart::XChartDataChangeEventListener >& aListener )
{
    SolarMutexGuard aGuard;
    if ( !pDocShell || aRanges.empty() )
        return;

    //! test for duplicates ?

    ScDocument& rDoc = pDocShell->GetDocument();
    ScRangeListRef aRangesRef( new ScRangeList(aRanges) );
    ScChartListenerCollection* pColl = rDoc.GetChartListenerCollection();
    OUString aName = pColl->getUniqueName("__Uno");
    if (aName.isEmpty())
        // failed to create unique name.
        return;

    ScChartListener* pListener = new ScChartListener( aName, rDoc, aRangesRef );
    pListener->SetUno( aListener, this );
    pColl->insert( pListener );
    pListener->StartListeningTo();
}

void SAL_CALL ScCellRangesBase::removeChartDataChangeEventListener( const uno::Reference<
                                    chart::XChartDataChangeEventListener >& aListener )
{
    SolarMutexGuard aGuard;
    if ( pDocShell && !aRanges.empty() )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScChartListenerCollection* pColl = rDoc.GetChartListenerCollection();
        pColl->FreeUno( aListener, this );
    }
}

double SAL_CALL ScCellRangesBase::getNotANumber()
{
    //  use DBL_MIN in ScChartArray, because Chart wants it so
    return DBL_MIN;
}

sal_Bool SAL_CALL ScCellRangesBase::isNotANumber( double nNumber )
{
    //  use DBL_MIN in ScChartArray, because Chart wants it so
    return (nNumber == DBL_MIN);
}

// XModifyBroadcaster

void SAL_CALL ScCellRangesBase::addModifyListener(const uno::Reference<util::XModifyListener>& aListener)
{
    SolarMutexGuard aGuard;
    if ( aRanges.empty() )
        throw uno::RuntimeException();

    aValueListeners.emplace_back( aListener );

    if ( aValueListeners.size() == 1 )
    {
        if (!pValueListener)
            pValueListener.reset( new ScLinkListener( LINK( this, ScCellRangesBase, ValueListenerHdl ) ) );

        ScDocument& rDoc = pDocShell->GetDocument();
        for ( size_t i = 0, nCount = aRanges.size(); i < nCount; i++)
            rDoc.StartListeningArea( aRanges[ i ], false, pValueListener.get() );

        acquire();  // don't lose this object (one ref for all listeners)
    }
}

void SAL_CALL ScCellRangesBase::removeModifyListener( const uno::Reference<util::XModifyListener>& aListener )
{

    SolarMutexGuard aGuard;
    if ( aRanges.empty() )
        throw uno::RuntimeException();

    rtl::Reference<ScCellRangesBase> xSelfHold(this); // in case the listeners have the last ref

    sal_uInt16 nCount = aValueListeners.size();
    for ( sal_uInt16 n=nCount; n--; )
    {
        uno::Reference<util::XModifyListener>& rObj = aValueListeners[n];
        if ( rObj == aListener )
        {
            aValueListeners.erase( aValueListeners.begin() + n );

            if ( aValueListeners.empty() )
            {
                if (pValueListener)
                    pValueListener->EndListeningAll();

                release();      // release the ref for the listeners
            }

            break;
        }
    }
}

// XCellRangesQuery

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryVisibleCells()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        //! Separate for all tables, if markings separated per table
        SCTAB nTab = lcl_FirstTab(aRanges);

        ScMarkData aMarkData(*GetMarkData());

        ScDocument& rDoc = pDocShell->GetDocument();
        SCCOL nCol = 0, nLastCol;
        while (nCol <= rDoc.MaxCol())
        {
            if (rDoc.ColHidden(nCol, nTab, nullptr, &nLastCol))
                // hidden columns.  Deselect them.
                aMarkData.SetMultiMarkArea(ScRange(nCol, 0, nTab, nLastCol, rDoc.MaxRow(), nTab), false);

            nCol = nLastCol + 1;
        }

        SCROW nRow = 0, nLastRow;
        while (nRow <= rDoc.MaxRow())
        {
            if (rDoc.RowHidden(nRow, nTab, nullptr, &nLastRow))
                // These rows are hidden.  Deselect them.
                aMarkData.SetMultiMarkArea(ScRange(0, nRow, nTab, rDoc.MaxCol(), nLastRow, nTab), false);

            nRow = nLastRow + 1;
        }

        ScRangeList aNewRanges;
        aMarkData.FillRangeListWithMarks( &aNewRanges, false );
        return new ScCellRangesObj( pDocShell, aNewRanges );
    }

    return nullptr;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryEmptyCells()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        ScMarkData aMarkData(*GetMarkData());

        //  mark occupied cells
        for (size_t i = 0, nCount = aRanges.size(); i < nCount; ++i)
        {
            ScRange const & rRange = aRanges[ i ];

            ScCellIterator aIter(rDoc, rRange);
            for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
            {
                //  notes count as non-empty
                if (!aIter.isEmpty())
                    aMarkData.SetMultiMarkArea(aIter.GetPos(), false);
            }
        }

        ScRangeList aNewRanges;
        //  IsMultiMarked is not enough (will not be reset during deselecting)
        //if (aMarkData.HasAnyMultiMarks()) // #i20044# should be set for all empty range
        aMarkData.FillRangeListWithMarks( &aNewRanges, false );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges can be empty
    }

    return nullptr;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryContentCells(
    sal_Int16 nContentFlags )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        ScMarkData aMarkData(rDoc.GetSheetLimits());

        //  select matching cells
        for ( size_t i = 0, nCount = aRanges.size(); i < nCount; ++i )
        {
            ScRange const & rRange = aRanges[ i ];

            ScCellIterator aIter(rDoc, rRange);
            for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
            {
                bool bAdd = false;
                switch (aIter.getType())
                {
                    case CELLTYPE_STRING:
                        if ( nContentFlags & sheet::CellFlags::STRING )
                            bAdd = true;
                        break;
                    case CELLTYPE_EDIT:
                        if ( (nContentFlags & sheet::CellFlags::STRING) || (nContentFlags & sheet::CellFlags::FORMATTED) )
                            bAdd = true;
                        break;
                    case CELLTYPE_FORMULA:
                        if ( nContentFlags & sheet::CellFlags::FORMULA )
                            bAdd = true;
                        break;
                    case CELLTYPE_VALUE:
                        if ( (nContentFlags & (sheet::CellFlags::VALUE|sheet::CellFlags::DATETIME))
                                == (sheet::CellFlags::VALUE|sheet::CellFlags::DATETIME) )
                            bAdd = true;
                        else
                        {
                            //  date/time identification

                            sal_uLong nIndex = static_cast<sal_uLong>(rDoc.GetAttr(
                                        aIter.GetPos(), ATTR_VALUE_FORMAT)->GetValue());
                            SvNumFormatType nTyp = rDoc.GetFormatTable()->GetType(nIndex);
                            if ((nTyp == SvNumFormatType::DATE) || (nTyp == SvNumFormatType::TIME) ||
                                    (nTyp == SvNumFormatType::DATETIME))
                            {
                                if ( nContentFlags & sheet::CellFlags::DATETIME )
                                    bAdd = true;
                            }
                            else
                            {
                                if ( nContentFlags & sheet::CellFlags::VALUE )
                                    bAdd = true;
                            }
                        }
                        break;
                    default:
                        {
                            // added to avoid warnings
                        }
                }

                if (bAdd)
                    aMarkData.SetMultiMarkArea(aIter.GetPos());
            }
        }

        if (nContentFlags & sheet::CellFlags::ANNOTATION)
        {
            std::vector<sc::NoteEntry> aNotes;
            rDoc.GetNotesInRange(aRanges, aNotes);

            for (const auto& i : aNotes)
            {
                aMarkData.SetMultiMarkArea(i.maPos);
            }
        }

        ScRangeList aNewRanges;
        if (aMarkData.IsMultiMarked())
            aMarkData.FillRangeListWithMarks( &aNewRanges, false );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges can be empty
    }

    return nullptr;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryFormulaCells(
    sal_Int32 nResultFlags )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        ScMarkData aMarkData(rDoc.GetSheetLimits());

        //  select matching cells
        for ( size_t i = 0, nCount = aRanges.size(); i < nCount; ++i )
        {
            ScRange const & rRange = aRanges[ i ];

            ScCellIterator aIter(rDoc, rRange);
            for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
            {
                if (aIter.getType() == CELLTYPE_FORMULA)
                {
                    ScFormulaCell* pFCell = aIter.getFormulaCell();
                    bool bAdd = false;
                    if (pFCell->GetErrCode() != FormulaError::NONE)
                    {
                        if ( nResultFlags & sheet::FormulaResult::ERROR )
                            bAdd = true;
                    }
                    else if (pFCell->IsValue())
                    {
                        if ( nResultFlags & sheet::FormulaResult::VALUE )
                            bAdd = true;
                    }
                    else    // String
                    {
                        if ( nResultFlags & sheet::FormulaResult::STRING )
                            bAdd = true;
                    }

                    if (bAdd)
                        aMarkData.SetMultiMarkArea(aIter.GetPos());
                }
            }
        }

        ScRangeList aNewRanges;
        if (aMarkData.IsMultiMarked())
            aMarkData.FillRangeListWithMarks( &aNewRanges, false );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges can be empty
    }

    return nullptr;
}

uno::Reference<sheet::XSheetCellRanges> ScCellRangesBase::QueryDifferences_Impl(
                        const table::CellAddress& aCompare, bool bColumnDiff)
{
    if (pDocShell)
    {
        size_t nRangeCount = aRanges.size();
        size_t i;
        ScDocument& rDoc = pDocShell->GetDocument();
        ScMarkData aMarkData(rDoc.GetSheetLimits());

        SCCOLROW nCmpPos = bColumnDiff ? static_cast<SCCOLROW>(aCompare.Row) : static_cast<SCCOLROW>(aCompare.Column);

        //  first select everything, where at all something is in the comparison column
        //  (in the second step the selection is cancelled for equal cells)

        SCTAB nTab = lcl_FirstTab(aRanges); //! for all tables, if markings per table
        ScRange aCmpRange, aCellRange;
        if (bColumnDiff)
            aCmpRange = ScRange( 0,nCmpPos,nTab, rDoc.MaxCol(),nCmpPos,nTab );
        else
            aCmpRange = ScRange( static_cast<SCCOL>(nCmpPos),0,nTab, static_cast<SCCOL>(nCmpPos),rDoc.MaxRow(),nTab );
        ScCellIterator aCmpIter(rDoc, aCmpRange);
        for (bool bHasCell = aCmpIter.first(); bHasCell; bHasCell = aCmpIter.next())
        {
            SCCOLROW nCellPos = bColumnDiff ? static_cast<SCCOLROW>(aCmpIter.GetPos().Col()) : static_cast<SCCOLROW>(aCmpIter.GetPos().Row());
            if (bColumnDiff)
                aCellRange = ScRange( static_cast<SCCOL>(nCellPos),0,nTab,
                        static_cast<SCCOL>(nCellPos),rDoc.MaxRow(),nTab );
            else
                aCellRange = ScRange( 0,nCellPos,nTab, rDoc.MaxCol(),nCellPos,nTab );

            for (i=0; i<nRangeCount; i++)
            {
                ScRange aRange( aRanges[ i ] );
                if ( aRange.Intersects( aCellRange ) )
                {
                    if (bColumnDiff)
                    {
                        aRange.aStart.SetCol(static_cast<SCCOL>(nCellPos));
                        aRange.aEnd.SetCol(static_cast<SCCOL>(nCellPos));
                    }
                    else
                    {
                        aRange.aStart.SetRow(nCellPos);
                        aRange.aEnd.SetRow(nCellPos);
                    }
                    aMarkData.SetMultiMarkArea( aRange );
                }
            }
        }

        //  compare all not empty cells with the comparison column and accordingly
        //  select or cancel

        ScAddress aCmpAddr;
        for (i=0; i<nRangeCount; i++)
        {
            ScRange const & rRange = aRanges[ i ];

            ScCellIterator aIter( rDoc, rRange );
            for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
            {
                if (bColumnDiff)
                    aCmpAddr = ScAddress( aIter.GetPos().Col(), nCmpPos, aIter.GetPos().Tab() );
                else
                    aCmpAddr = ScAddress( static_cast<SCCOL>(nCmpPos), aIter.GetPos().Row(), aIter.GetPos().Tab() );

                ScRange aOneRange(aIter.GetPos());
                if (!aIter.equalsWithoutFormat(aCmpAddr))
                    aMarkData.SetMultiMarkArea( aOneRange );
                else
                    aMarkData.SetMultiMarkArea( aOneRange, false );     // deselect
            }
        }

        ScRangeList aNewRanges;
        if (aMarkData.IsMultiMarked())
            aMarkData.FillRangeListWithMarks( &aNewRanges, false );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges can be empty
    }
    return nullptr;
}

uno::Reference<sheet::XSheetCellRanges > SAL_CALL ScCellRangesBase::queryColumnDifferences(
    const table::CellAddress& aCompare )
{
    SolarMutexGuard aGuard;
    return QueryDifferences_Impl( aCompare, true );
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryRowDifferences(
    const table::CellAddress& aCompare )
{
    SolarMutexGuard aGuard;
    return QueryDifferences_Impl( aCompare, false );
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryIntersection(
                            const table::CellRangeAddress& aRange )
{
    SolarMutexGuard aGuard;
    ScRange aMask( static_cast<SCCOL>(aRange.StartColumn), static_cast<SCROW>(aRange.StartRow), aRange.Sheet,
                   static_cast<SCCOL>(aRange.EndColumn),   static_cast<SCROW>(aRange.EndRow),   aRange.Sheet );

    ScRangeList aNew;
    for ( size_t i = 0, nCount = aRanges.size(); i < nCount; ++i )
    {
        ScRange aTemp( aRanges[ i ] );
        if ( aTemp.Intersects( aMask ) )
            aNew.Join( ScRange( std::max( aTemp.aStart.Col(), aMask.aStart.Col() ),
                                std::max( aTemp.aStart.Row(), aMask.aStart.Row() ),
                                std::max( aTemp.aStart.Tab(), aMask.aStart.Tab() ),
                                std::min( aTemp.aEnd.Col(), aMask.aEnd.Col() ),
                                std::min( aTemp.aEnd.Row(), aMask.aEnd.Row() ),
                                std::min( aTemp.aEnd.Tab(), aMask.aEnd.Tab() ) ) );
    }

    return new ScCellRangesObj( pDocShell, aNew );  // can be empty
}

// XFormulaQuery

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryPrecedents(
    sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        ScRangeList aNewRanges(aRanges);
        bool bFound;
        do
        {
            bFound = false;

            //  aMarkData uses aNewRanges, not aRanges, so GetMarkData can't be used
            ScMarkData aMarkData(rDoc.GetSheetLimits());
            aMarkData.MarkFromRangeList( aNewRanges, false );
            aMarkData.MarkToMulti();        // needed for IsAllMarked

            for (size_t nR = 0, nCount = aNewRanges.size(); nR<nCount; ++nR)
            {
                ScRange const & rRange = aNewRanges[ nR];
                ScCellIterator aIter(rDoc, rRange);
                for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
                {
                    if (aIter.getType() != CELLTYPE_FORMULA)
                        continue;

                    ScDetectiveRefIter aRefIter(rDoc, aIter.getFormulaCell());
                    ScRange aRefRange;
                    while ( aRefIter.GetNextRef( aRefRange) )
                    {
                        if ( bRecursive && !bFound && !aMarkData.IsAllMarked( aRefRange ) )
                            bFound = true;
                        aMarkData.SetMultiMarkArea(aRefRange);
                    }
                }
            }

            aMarkData.FillRangeListWithMarks( &aNewRanges, true );
        }
        while ( bRecursive && bFound );

        return new ScCellRangesObj( pDocShell, aNewRanges );
    }

    return nullptr;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryDependents(
    sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        ScRangeList aNewRanges(aRanges);
        bool bFound;
        do
        {
            bFound = false;

            //  aMarkData uses aNewRanges, not aRanges, so GetMarkData can't be used
            ScMarkData aMarkData(rDoc.GetSheetLimits());
            aMarkData.MarkFromRangeList( aNewRanges, false );
            aMarkData.MarkToMulti();        // needed for IsAllMarked

            SCTAB nTab = lcl_FirstTab(aNewRanges);              //! all tables

            ScCellIterator aCellIter( rDoc, ScRange(0, 0, nTab, rDoc.MaxCol(), rDoc.MaxRow(), nTab) );
            for (bool bHasCell = aCellIter.first(); bHasCell; bHasCell = aCellIter.next())
            {
                if (aCellIter.getType() != CELLTYPE_FORMULA)
                    continue;

                bool bMark = false;
                ScDetectiveRefIter aIter(rDoc, aCellIter.getFormulaCell());
                ScRange aRefRange;
                while ( aIter.GetNextRef( aRefRange) && !bMark )
                {
                    size_t nRangesCount = aNewRanges.size();
                    for (size_t nR = 0; nR < nRangesCount; ++nR)
                    {
                        ScRange const & rRange = aNewRanges[ nR ];
                        if (rRange.Intersects(aRefRange))
                        {
                            bMark = true;                   // depending on part of Range
                            break;
                        }
                    }
                }
                if (bMark)
                {
                    ScRange aCellRange(aCellIter.GetPos());
                    if ( bRecursive && !bFound && !aMarkData.IsAllMarked( aCellRange ) )
                        bFound = true;
                    aMarkData.SetMultiMarkArea(aCellRange);
                }
            }

            aMarkData.FillRangeListWithMarks( &aNewRanges, true );
        }
        while ( bRecursive && bFound );

        return new ScCellRangesObj( pDocShell, aNewRanges );
    }

    return nullptr;
}

// XSearchable

uno::Reference<util::XSearchDescriptor> SAL_CALL ScCellRangesBase::createSearchDescriptor()
{
    SolarMutexGuard aGuard;
    return new ScCellSearchObj;
}

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangesBase::findAll(
                        const uno::Reference<util::XSearchDescriptor>& xDesc )
{
    SolarMutexGuard aGuard;
    //  should we return Null if nothing is found(?)
    uno::Reference<container::XIndexAccess> xRet;
    if ( pDocShell && xDesc.is() )
    {
        ScCellSearchObj* pSearch = comphelper::getUnoTunnelImplementation<ScCellSearchObj>( xDesc );
        if (pSearch)
        {
            SvxSearchItem* pSearchItem = pSearch->GetSearchItem();
            if (pSearchItem)
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                pSearchItem->SetCommand( SvxSearchCmd::FIND_ALL );
                //  always only within this object
                pSearchItem->SetSelection( !lcl_WholeSheet(rDoc, aRanges) );

                ScMarkData aMark(*GetMarkData());

                OUString aDummyUndo;
                ScRangeList aMatchedRanges;
                SCCOL nCol = 0;
                SCROW nRow = 0;
                SCTAB nTab = 0;
                bool bFound = rDoc.SearchAndReplace(
                    *pSearchItem, nCol, nRow, nTab, aMark, aMatchedRanges, aDummyUndo);
                if (bFound)
                {
                    //  on findAll always CellRanges no matter how much has been found
                    xRet.set(new ScCellRangesObj( pDocShell, aMatchedRanges ));
                }
            }
        }
    }
    return xRet;
}

uno::Reference<uno::XInterface> ScCellRangesBase::Find_Impl(
                                    const uno::Reference<util::XSearchDescriptor>& xDesc,
                                    const ScAddress* pLastPos )
{
    uno::Reference<uno::XInterface> xRet;
    if ( pDocShell && xDesc.is() )
    {
        ScCellSearchObj* pSearch = comphelper::getUnoTunnelImplementation<ScCellSearchObj>( xDesc );
        if (pSearch)
        {
            SvxSearchItem* pSearchItem = pSearch->GetSearchItem();
            if (pSearchItem)
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                pSearchItem->SetCommand( SvxSearchCmd::FIND );
                //  only always in this object
                pSearchItem->SetSelection( !lcl_WholeSheet(rDoc, aRanges) );

                ScMarkData aMark(*GetMarkData());

                SCCOL nCol;
                SCROW nRow;
                SCTAB nTab;
                if (pLastPos)
                    pLastPos->GetVars( nCol, nRow, nTab );
                else
                {
                    nTab = lcl_FirstTab(aRanges);   //! multiple sheets?
                    rDoc.GetSearchAndReplaceStart( *pSearchItem, nCol, nRow );
                }

                OUString aDummyUndo;
                ScRangeList aMatchedRanges;
                bool bFound = rDoc.SearchAndReplace(
                    *pSearchItem, nCol, nRow, nTab, aMark, aMatchedRanges, aDummyUndo);
                if (bFound)
                {
                    ScAddress aFoundPos( nCol, nRow, nTab );
                    xRet.set(static_cast<cppu::OWeakObject*>(new ScCellObj( pDocShell, aFoundPos )));
                }
            }
        }
    }
    return xRet;
}

uno::Reference<uno::XInterface> SAL_CALL ScCellRangesBase::findFirst(
                        const uno::Reference<util::XSearchDescriptor>& xDesc )
{
    SolarMutexGuard aGuard;
    return Find_Impl( xDesc, nullptr );
}

uno::Reference<uno::XInterface> SAL_CALL ScCellRangesBase::findNext(
                        const uno::Reference<uno::XInterface>& xStartAt,
                        const uno::Reference<util::XSearchDescriptor >& xDesc )
{
    SolarMutexGuard aGuard;
    if ( xStartAt.is() )
    {
        ScCellRangesBase* pRangesImp = comphelper::getUnoTunnelImplementation<ScCellRangesBase>( xStartAt );
        if ( pRangesImp && pRangesImp->GetDocShell() == pDocShell )
        {
            const ScRangeList& rStartRanges = pRangesImp->GetRangeList();
            if ( rStartRanges.size() == 1 )
            {
                ScAddress aStartPos = rStartRanges[ 0 ].aStart;
                return Find_Impl( xDesc, &aStartPos );
            }
        }
    }
    return nullptr;
}

// XReplaceable

uno::Reference<util::XReplaceDescriptor> SAL_CALL ScCellRangesBase::createReplaceDescriptor()
{
    SolarMutexGuard aGuard;
    return new ScCellSearchObj;
}

sal_Int32 SAL_CALL ScCellRangesBase::replaceAll( const uno::Reference<util::XSearchDescriptor>& xDesc )
{
    SolarMutexGuard aGuard;
    sal_Int32 nReplaced = 0;
    if ( pDocShell && xDesc.is() )
    {
        ScCellSearchObj* pSearch = comphelper::getUnoTunnelImplementation<ScCellSearchObj>( xDesc );
        if (pSearch)
        {
            SvxSearchItem* pSearchItem = pSearch->GetSearchItem();
            if (pSearchItem)
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                bool bUndo(rDoc.IsUndoEnabled());
                pSearchItem->SetCommand( SvxSearchCmd::REPLACE_ALL );
                //  only always in this object
                pSearchItem->SetSelection( !lcl_WholeSheet(rDoc, aRanges) );

                ScMarkData aMark(*GetMarkData());

                SCTAB nTabCount = rDoc.GetTableCount();
                bool bProtected = !pDocShell->IsEditable();
                for (const auto& rTab : aMark)
                {
                    if (rTab >= nTabCount)
                        break;
                    if ( rDoc.IsTabProtected(rTab) )
                        bProtected = true;
                }
                if (bProtected)
                {
                    //! Exception, or what?
                }
                else
                {
                    SCTAB nTab = aMark.GetFirstSelected();      // do not use if SearchAndReplace
                    SCCOL nCol = 0;
                    SCROW nRow = 0;

                    OUString aUndoStr;
                    ScDocumentUniquePtr pUndoDoc;
                    if (bUndo)
                    {
                        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
                        pUndoDoc->InitUndo( rDoc, nTab, nTab );
                    }
                    for (const auto& rTab : aMark)
                    {
                        if (rTab >= nTabCount)
                            break;
                        if (rTab != nTab && bUndo)
                            pUndoDoc->AddUndoTab( rTab, rTab );
                    }
                    std::unique_ptr<ScMarkData> pUndoMark;
                    if (bUndo)
                        pUndoMark.reset(new ScMarkData(aMark));

                    bool bFound = false;
                    if (bUndo)
                    {
                        ScRangeList aMatchedRanges;
                        bFound = rDoc.SearchAndReplace(
                            *pSearchItem, nCol, nRow, nTab, aMark, aMatchedRanges, aUndoStr, pUndoDoc.get() );
                    }
                    if (bFound)
                    {
                        nReplaced = pUndoDoc->GetCellCount();

                        pDocShell->GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoReplace>( pDocShell, *pUndoMark, nCol, nRow, nTab,
                                                        aUndoStr, std::move(pUndoDoc), pSearchItem ) );

                        pDocShell->PostPaintGridAll();
                        pDocShell->SetDocumentModified();
                    }
                }
            }
        }
    }
    return nReplaced;
}

// XUnoTunnel

UNO3_GETIMPLEMENTATION_IMPL(ScCellRangesBase);

typedef std::vector<ScNamedEntry> ScNamedEntryArr_Impl;

struct ScCellRangesObj::Impl
{
    ScNamedEntryArr_Impl m_aNamedEntries;
};

ScCellRangesObj::ScCellRangesObj(ScDocShell* pDocSh, const ScRangeList& rR)
    : ScCellRangesBase(pDocSh, rR)
    , m_pImpl(new Impl)
{
}

ScCellRangesObj::~ScCellRangesObj()
{
}

void ScCellRangesObj::RefChanged()
{
    ScCellRangesBase::RefChanged();
}

uno::Any SAL_CALL ScCellRangesObj::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( sheet::XSheetCellRangeContainer )
    SC_QUERYINTERFACE( sheet::XSheetCellRanges )
    SC_QUERYINTERFACE( container::XIndexAccess )
    SC_QUERY_MULTIPLE( container::XElementAccess, container::XIndexAccess )
    SC_QUERYINTERFACE( container::XEnumerationAccess )
    SC_QUERYINTERFACE( container::XNameContainer )
    SC_QUERYINTERFACE( container::XNameReplace )
    SC_QUERYINTERFACE( container::XNameAccess )

    return ScCellRangesBase::queryInterface( rType );
}

void SAL_CALL ScCellRangesObj::acquire() throw()
{
    ScCellRangesBase::acquire();
}

void SAL_CALL ScCellRangesObj::release() throw()
{
    ScCellRangesBase::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellRangesObj::getTypes()
{
    static const uno::Sequence<uno::Type> aTypes = comphelper::concatSequences(
        ScCellRangesBase::getTypes(),
        uno::Sequence<uno::Type>
        {
            cppu::UnoType<sheet::XSheetCellRangeContainer>::get(),
            cppu::UnoType<container::XNameContainer>::get(),
            cppu::UnoType<container::XEnumerationAccess>::get()
        } );
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellRangesObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XCellRanges

ScCellRangeObj* ScCellRangesObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    if ( pDocSh && nIndex >= 0 && nIndex < sal::static_int_cast<sal_Int32>(rRanges.size()) )
    {
        ScRange const & rRange = rRanges[ nIndex ];
        if ( rRange.aStart == rRange.aEnd )
            return new ScCellObj( pDocSh, rRange.aStart );
        else
            return new ScCellRangeObj( pDocSh, rRange );
    }

    return nullptr;        // no DocShell or wrong index
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScCellRangesObj::getRangeAddresses()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    size_t nCount = rRanges.size();
    if ( pDocSh && nCount )
    {
        table::CellRangeAddress aRangeAddress;
        uno::Sequence<table::CellRangeAddress> aSeq(nCount);
        table::CellRangeAddress* pAry = aSeq.getArray();
        for ( size_t i=0; i < nCount; i++)
        {
            ScUnoConversion::FillApiRange( aRangeAddress, rRanges[ i ] );
            pAry[i] = aRangeAddress;
        }
        return aSeq;
    }

    return uno::Sequence<table::CellRangeAddress>(0);   // can be empty
}

uno::Reference<container::XEnumerationAccess> SAL_CALL ScCellRangesObj::getCells()
{
    SolarMutexGuard aGuard;

    //  getCells with empty range list is possible (no exception),
    //  the resulting enumeration just has no elements
    //  (same behaviour as a valid range with no cells)
    //  This is handled in ScCellsEnumeration ctor.

    const ScRangeList& rRanges = GetRangeList();
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScCellsObj( pDocSh, rRanges );
    return nullptr;
}

OUString SAL_CALL ScCellRangesObj::getRangeAddressesAsString()
{
    SolarMutexGuard aGuard;
    OUString aString;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    if (pDocSh)
        rRanges.Format( aString, ScRefFlags::VALID | ScRefFlags::TAB_3D, pDocSh->GetDocument() );
    return aString;
}

// XSheetCellRangeContainer

void SAL_CALL ScCellRangesObj::addRangeAddress( const table::CellRangeAddress& rRange,
                                    sal_Bool bMergeRanges )
{
    SolarMutexGuard aGuard;
    ScRange aRange(static_cast<SCCOL>(rRange.StartColumn),
            static_cast<SCROW>(rRange.StartRow),
            static_cast<SCTAB>(rRange.Sheet),
            static_cast<SCCOL>(rRange.EndColumn),
            static_cast<SCROW>(rRange.EndRow),
            static_cast<SCTAB>(rRange.Sheet));
    AddRange(aRange, bMergeRanges);
}

static void lcl_RemoveNamedEntry( ScNamedEntryArr_Impl& rNamedEntries, const ScRange& rRange )
{
    sal_uInt16 nCount = rNamedEntries.size();
    for ( sal_uInt16 n=nCount; n--; )
        if ( rNamedEntries[n].GetRange() == rRange )
            rNamedEntries.erase( rNamedEntries.begin() + n );
}

void SAL_CALL ScCellRangesObj::removeRangeAddress( const table::CellRangeAddress& rRange )
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();

    ScRangeList aSheetRanges;
    ScRangeList aNotSheetRanges;
    for (size_t i = 0; i < rRanges.size(); ++i)
    {
        if (rRanges[ i].aStart.Tab() == rRange.Sheet)
        {
            aSheetRanges.push_back( rRanges[ i ] );
        }
        else
        {
            aNotSheetRanges.push_back( rRanges[ i ] );
        }
    }
    ScMarkData aMarkData(GetDocument()->GetSheetLimits());
    aMarkData.MarkFromRangeList( aSheetRanges, false );
    ScRange aRange(static_cast<SCCOL>(rRange.StartColumn),
                static_cast<SCROW>(rRange.StartRow),
                static_cast<SCTAB>(rRange.Sheet),
                static_cast<SCCOL>(rRange.EndColumn),
                static_cast<SCROW>(rRange.EndRow),
                static_cast<SCTAB>(rRange.Sheet));
    if (aMarkData.GetTableSelect( aRange.aStart.Tab() ))
    {
        aMarkData.MarkToMulti();
        if (!aMarkData.IsAllMarked( aRange ) )
            throw container::NoSuchElementException();

        aMarkData.SetMultiMarkArea( aRange, false );
        lcl_RemoveNamedEntry(m_pImpl->m_aNamedEntries, aRange);

    }
    SetNewRanges(aNotSheetRanges);
    ScRangeList aNew;
    aMarkData.FillRangeListWithMarks( &aNew, false );
    for ( size_t j = 0; j < aNew.size(); ++j)
    {
        AddRange(aNew[ j ], false);
    }
}

void SAL_CALL ScCellRangesObj::addRangeAddresses( const uno::Sequence<table::CellRangeAddress >& rRanges,
                                    sal_Bool bMergeRanges )
{
    SolarMutexGuard aGuard;
    for (const table::CellRangeAddress& rRange : rRanges)
    {
        ScRange aRange(static_cast<SCCOL>(rRange.StartColumn),
                static_cast<SCROW>(rRange.StartRow),
                static_cast<SCTAB>(rRange.Sheet),
                static_cast<SCCOL>(rRange.EndColumn),
                static_cast<SCROW>(rRange.EndRow),
                static_cast<SCTAB>(rRange.Sheet));
        AddRange(aRange, bMergeRanges);
    }
}

void SAL_CALL ScCellRangesObj::removeRangeAddresses( const uno::Sequence<table::CellRangeAddress >& rRangeSeq )
{
    // use sometimes a better/faster implementation
    for (const table::CellRangeAddress& rRange : rRangeSeq)
    {
        removeRangeAddress(rRange);
    }
}

// XNameContainer

static void lcl_RemoveNamedEntry( ScNamedEntryArr_Impl& rNamedEntries, const OUString& rName )
{
    sal_uInt16 nCount = rNamedEntries.size();
    for ( sal_uInt16 n=nCount; n--; )
        if ( rNamedEntries[n].GetName() == rName )
            rNamedEntries.erase( rNamedEntries.begin() + n );
}

void SAL_CALL ScCellRangesObj::insertByName( const OUString& aName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    bool bDone = false;

    //! Type of aElement can be some specific interface instead of XInterface

    uno::Reference<uno::XInterface> xInterface(aElement, uno::UNO_QUERY);
    if ( pDocSh && xInterface.is() )
    {
        ScCellRangesBase* pRangesImp = comphelper::getUnoTunnelImplementation<ScCellRangesBase>( xInterface );
        if ( pRangesImp && pRangesImp->GetDocShell() == pDocSh )
        {
            //  if explicit name is given and already existing, throw exception

            if ( !aName.isEmpty() )
            {
                size_t nNamedCount = m_pImpl->m_aNamedEntries.size();
                for (size_t n = 0; n < nNamedCount; n++)
                {
                    if (m_pImpl->m_aNamedEntries[n].GetName() == aName)
                        throw container::ElementExistException();
                }
            }

            ScRangeList aNew(GetRangeList());
            const ScRangeList& rAddRanges = pRangesImp->GetRangeList();
            size_t nAddCount = rAddRanges.size();
            for ( size_t i = 0; i < nAddCount; i++ )
                aNew.Join( rAddRanges[ i ] );
            SetNewRanges(aNew);
            bDone = true;

            if ( !aName.isEmpty() && nAddCount == 1 )
            {
                //  if a name is given, also insert into list of named entries
                //  (only possible for a single range)
                //  name is not in m_pImpl->m_aNamedEntries (tested above)
                m_pImpl->m_aNamedEntries.emplace_back( aName, rAddRanges[ 0 ] );
            }
        }
    }

    if (!bDone)
    {
        //  invalid element - double names are handled above
        throw lang::IllegalArgumentException();
    }
}

static bool lcl_FindRangeByName( const ScRangeList& rRanges, ScDocShell* pDocSh,
                            const OUString& rName, size_t& rIndex )
{
    if (pDocSh)
    {
        OUString aRangeStr;
        ScDocument& rDoc = pDocSh->GetDocument();
        for ( size_t i = 0, nCount = rRanges.size(); i < nCount; i++ )
        {
            aRangeStr = rRanges[ i ].Format(rDoc, ScRefFlags::VALID | ScRefFlags::TAB_3D);
            if ( aRangeStr == rName )
            {
                rIndex = i;
                return true;
            }
        }
    }
    return false;
}

static bool lcl_FindRangeOrEntry( const ScNamedEntryArr_Impl& rNamedEntries,
                            const ScRangeList& rRanges, ScDocShell* pDocSh,
                            const OUString& rName, ScRange& rFound )
{
    //  exact range in list?

    size_t nIndex = 0;
    if ( lcl_FindRangeByName( rRanges, pDocSh, rName, nIndex ) )
    {
        rFound = rRanges[ nIndex ];
        return true;
    }

    //  range contained in selection? (sheet must be specified)

    ScRange aCellRange;
    ScRefFlags nParse = aCellRange.ParseAny( rName, pDocSh->GetDocument() );
    if ( (nParse & ( ScRefFlags::VALID | ScRefFlags::TAB_3D ))
               == ( ScRefFlags::VALID | ScRefFlags::TAB_3D ))
    {
        ScMarkData aMarkData(pDocSh->GetDocument().GetSheetLimits());
        aMarkData.MarkFromRangeList( rRanges, false );
        aMarkData.MarkToMulti();        // needed for IsAllMarked
        if ( aMarkData.IsAllMarked( aCellRange ) )
        {
            rFound = aCellRange;
            return true;
        }
    }

    //  named entry in this object?

    for (const auto & rNamedEntry : rNamedEntries)
        if ( rNamedEntry.GetName() == rName )
        {
            //  test if named entry is contained in rRanges

            const ScRange& rComp = rNamedEntry.GetRange();
            ScMarkData aMarkData(pDocSh->GetDocument().GetSheetLimits());
            aMarkData.MarkFromRangeList( rRanges, false );
            aMarkData.MarkToMulti();        // needed for IsAllMarked
            if ( aMarkData.IsAllMarked( rComp ) )
            {
                rFound = rComp;
                return true;
            }
        }

    return false;       // not found
}

void SAL_CALL ScCellRangesObj::removeByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    size_t nIndex = 0;
    if ( lcl_FindRangeByName( rRanges, pDocSh, aName, nIndex ) )
    {
        // skip a single range
        ScRangeList aNew;
        for ( size_t i = 0, nCount = rRanges.size(); i < nCount; i++ )
            if (i != nIndex)
                aNew.push_back( rRanges[ i ] );
        SetNewRanges(aNew);
        bDone = true;
    }
    else if (pDocSh)
    {
        //  deselect any ranges (parsed or named entry)
        ScRangeList aDiff;
        bool bValid = ( aDiff.Parse( aName, pDocSh->GetDocument() ) & ScRefFlags::VALID )
                                                                       == ScRefFlags::VALID;
        if (!bValid)
        {
            sal_uInt16 nCount = m_pImpl->m_aNamedEntries.size();
            for (sal_uInt16 n=0; n<nCount && !bValid; n++)
                if (m_pImpl->m_aNamedEntries[n].GetName() == aName)
                {
                    aDiff.RemoveAll();
                    aDiff.push_back(m_pImpl->m_aNamedEntries[n].GetRange());
                    bValid = true;
                }
        }
        if ( bValid )
        {
            ScMarkData aMarkData(GetDocument()->GetSheetLimits());
            aMarkData.MarkFromRangeList( rRanges, false );

            for ( size_t i = 0, nDiffCount = aDiff.size(); i < nDiffCount; i++ )
            {
                ScRange const & rDiffRange = aDiff[ i ];
                if (aMarkData.GetTableSelect( rDiffRange.aStart.Tab() ))
                    aMarkData.SetMultiMarkArea( rDiffRange, false );
            }

            ScRangeList aNew;
            aMarkData.FillRangeListWithMarks( &aNew, false );
            SetNewRanges(aNew);

            bDone = true;       //! error if range was not selected before?
        }
    }

    if (!m_pImpl->m_aNamedEntries.empty())
        lcl_RemoveNamedEntry(m_pImpl->m_aNamedEntries, aName);

    if (!bDone)
        throw container::NoSuchElementException();      // not found
}

// XNameReplace

void SAL_CALL ScCellRangesObj::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;
    //! combine?
    removeByName( aName );
    insertByName( aName, aElement );
}

// XNameAccess

uno::Any SAL_CALL ScCellRangesObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    ScRange aRange;
    if (!lcl_FindRangeOrEntry(m_pImpl->m_aNamedEntries, rRanges,
                pDocSh, aName, aRange))
        throw container::NoSuchElementException();

    uno::Reference<table::XCellRange> xRange;
    if ( aRange.aStart == aRange.aEnd )
        xRange.set(new ScCellObj( pDocSh, aRange.aStart ));
    else
        xRange.set(new ScCellRangeObj( pDocSh, aRange ));
    aRet <<= xRange;

    return aRet;
}

static bool lcl_FindEntryName( const ScNamedEntryArr_Impl& rNamedEntries,
                        const ScRange& rRange, OUString& rName )
{
    sal_uInt16 nCount = rNamedEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
        if (rNamedEntries[i].GetRange() == rRange)
        {
            rName = rNamedEntries[i].GetName();
            return true;
        }
    return false;
}

uno::Sequence<OUString> SAL_CALL ScCellRangesObj::getElementNames()
{
    SolarMutexGuard aGuard;

    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    if (pDocSh)
    {
        OUString aRangeStr;
        ScDocument& rDoc = pDocSh->GetDocument();
        size_t nCount = rRanges.size();

        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();
        for (size_t i=0; i < nCount; i++)
        {
            //  use given name if for exactly this range, otherwise just format
            ScRange const & rRange = rRanges[ i ];
            if (m_pImpl->m_aNamedEntries.empty() ||
                !lcl_FindEntryName(m_pImpl->m_aNamedEntries, rRange, aRangeStr))
            {
                aRangeStr = rRange.Format(rDoc, ScRefFlags::VALID | ScRefFlags::TAB_3D);
            }
            pAry[i] = aRangeStr;
        }
        return aSeq;
    }
    return uno::Sequence<OUString>(0);
}

sal_Bool SAL_CALL ScCellRangesObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    ScRange aRange;
    return lcl_FindRangeOrEntry(m_pImpl->m_aNamedEntries, rRanges, pDocSh,
                aName, aRange);
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScCellRangesObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.SheetCellRangesEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScCellRangesObj::getCount()
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    return rRanges.size();
}

uno::Any SAL_CALL ScCellRangesObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xRange(GetObjectByIndex_Impl(nIndex));
    if (!xRange.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xRange);

}

uno::Type SAL_CALL ScCellRangesObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XCellRange>::get();
}

sal_Bool SAL_CALL ScCellRangesObj::hasElements()
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    return !rRanges.empty();
}

// XServiceInfo
OUString SAL_CALL ScCellRangesObj::getImplementationName()
{
    return "ScCellRangesObj";
}

sal_Bool SAL_CALL ScCellRangesObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScCellRangesObj::getSupportedServiceNames()
{
    return {SCSHEETCELLRANGES_SERVICE,
            SCCELLPROPERTIES_SERVICE,
            SCCHARPROPERTIES_SERVICE,
            SCPARAPROPERTIES_SERVICE};
}

uno::Reference<table::XCellRange> ScCellRangeObj::CreateRangeFromDoc( const ScDocument& rDoc, const ScRange& rR )
{
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    if ( auto pDocShell = dynamic_cast<ScDocShell*>( pObjSh) )
        return new ScCellRangeObj( pDocShell, rR );
    return nullptr;
}

ScCellRangeObj::ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR) :
    ScCellRangesBase( pDocSh, rR ),
    pRangePropSet( lcl_GetRangePropertySet() ),
    aRange( rR )
{
    aRange.PutInOrder();       // beginning / end correct
}

ScCellRangeObj::~ScCellRangeObj()
{
}

void ScCellRangeObj::RefChanged()
{
    ScCellRangesBase::RefChanged();

    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE(rRanges.size() == 1, "What ranges ?!?!");
    if ( !rRanges.empty() )
    {
        const ScRange & rFirst = rRanges[0];
        aRange = rFirst;
        aRange.PutInOrder();
    }
}

uno::Any SAL_CALL ScCellRangeObj::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( sheet::XCellRangeAddressable )
    SC_QUERYINTERFACE( table::XCellRange )
    SC_QUERYINTERFACE( sheet::XSheetCellRange )
    SC_QUERYINTERFACE( sheet::XArrayFormulaRange )
    SC_QUERYINTERFACE( sheet::XArrayFormulaTokens )
    SC_QUERYINTERFACE( sheet::XCellRangeData )
    SC_QUERYINTERFACE( sheet::XCellRangeFormula )
    SC_QUERYINTERFACE( sheet::XMultipleOperation )
    SC_QUERYINTERFACE( util::XMergeable )
    SC_QUERYINTERFACE( sheet::XCellSeries )
    SC_QUERYINTERFACE( table::XAutoFormattable )
    SC_QUERYINTERFACE( util::XSortable )
    SC_QUERYINTERFACE( sheet::XSheetFilterableEx )
    SC_QUERYINTERFACE( sheet::XSheetFilterable )
    SC_QUERYINTERFACE( sheet::XSubTotalCalculatable )
    SC_QUERYINTERFACE( table::XColumnRowRange )
    SC_QUERYINTERFACE( util::XImportable )
    SC_QUERYINTERFACE( sheet::XCellFormatRangesSupplier )
    SC_QUERYINTERFACE( sheet::XUniqueCellFormatRangesSupplier )

    return ScCellRangesBase::queryInterface( rType );
}

void SAL_CALL ScCellRangeObj::acquire() throw()
{
    ScCellRangesBase::acquire();
}

void SAL_CALL ScCellRangeObj::release() throw()
{
    ScCellRangesBase::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellRangeObj::getTypes()
{
    static const uno::Sequence<uno::Type> aTypes = comphelper::concatSequences(
        ScCellRangesBase::getTypes(),
        uno::Sequence<uno::Type>
        {
            cppu::UnoType<sheet::XCellRangeAddressable>::get(),
            cppu::UnoType<sheet::XSheetCellRange>::get(),
            cppu::UnoType<sheet::XArrayFormulaRange>::get(),
            cppu::UnoType<sheet::XArrayFormulaTokens>::get(),
            cppu::UnoType<sheet::XCellRangeData>::get(),
            cppu::UnoType<sheet::XCellRangeFormula>::get(),
            cppu::UnoType<sheet::XMultipleOperation>::get(),
            cppu::UnoType<util::XMergeable>::get(),
            cppu::UnoType<sheet::XCellSeries>::get(),
            cppu::UnoType<table::XAutoFormattable>::get(),
            cppu::UnoType<util::XSortable>::get(),
            cppu::UnoType<sheet::XSheetFilterableEx>::get(),
            cppu::UnoType<sheet::XSubTotalCalculatable>::get(),
            cppu::UnoType<table::XColumnRowRange>::get(),
            cppu::UnoType<util::XImportable>::get(),
            cppu::UnoType<sheet::XCellFormatRangesSupplier>::get(),
            cppu::UnoType<sheet::XUniqueCellFormatRangesSupplier>::get()
        } );
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellRangeObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XCellRange

//  ColumnCount / RowCount vanished
//! are used in Writer for tables ???

uno::Reference<table::XCell> ScCellRangeObj::GetCellByPosition_Impl(
                                        sal_Int32 nColumn, sal_Int32 nRow )
{
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        throw uno::RuntimeException();

    if ( nColumn >= 0 && nRow >= 0 )
    {
        sal_Int32 nPosX = aRange.aStart.Col() + nColumn;
        sal_Int32 nPosY = aRange.aStart.Row() + nRow;

        if ( nPosX <= aRange.aEnd.Col() && nPosY <= aRange.aEnd.Row() )
        {
            ScAddress aNew( static_cast<SCCOL>(nPosX), static_cast<SCROW>(nPosY), aRange.aStart.Tab() );
            return new ScCellObj( pDocSh, aNew );
        }
    }

    throw lang::IndexOutOfBoundsException();
}

uno::Reference<table::XCell> SAL_CALL ScCellRangeObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
{
    SolarMutexGuard aGuard;

    return GetCellByPosition_Impl(nColumn, nRow);
}

uno::Reference<table::XCellRange> SAL_CALL ScCellRangeObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
{
    SolarMutexGuard aGuard;

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        throw uno::RuntimeException();

    if ( nLeft >= 0 && nTop >= 0 && nRight >= 0 && nBottom >= 0 )
    {
        sal_Int32 nStartX = aRange.aStart.Col() + nLeft;
        sal_Int32 nStartY = aRange.aStart.Row() + nTop;
        sal_Int32 nEndX = aRange.aStart.Col() + nRight;
        sal_Int32 nEndY = aRange.aStart.Row() + nBottom;

        if ( nStartX <= nEndX && nEndX <= aRange.aEnd.Col() &&
             nStartY <= nEndY && nEndY <= aRange.aEnd.Row() )
        {
            ScRange aNew( static_cast<SCCOL>(nStartX), static_cast<SCROW>(nStartY), aRange.aStart.Tab(),
                          static_cast<SCCOL>(nEndX), static_cast<SCROW>(nEndY), aRange.aEnd.Tab() );
            return new ScCellRangeObj( pDocSh, aNew );
        }
    }

    throw lang::IndexOutOfBoundsException();
}

uno::Reference<table::XCellRange> SAL_CALL ScCellRangeObj::getCellRangeByName(
                        const OUString& aName )
{
    return getCellRangeByName( aName, ScAddress::detailsOOOa1 );
}

uno::Reference<table::XCellRange>  ScCellRangeObj::getCellRangeByName(
                        const OUString& aName, const ScAddress::Details& rDetails  )
{
    //  name refers to the whole document (with the range's table as default),
    //  valid only if the range is within this range

    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = aRange.aStart.Tab();

        ScRange aCellRange;
        bool bFound = false;
        ScRefFlags nParse = aCellRange.ParseAny( aName, rDoc, rDetails );
        if ( nParse & ScRefFlags::VALID )
        {
            if ( !(nParse & ScRefFlags::TAB_3D) )   // no sheet specified -> this sheet
            {
                aCellRange.aStart.SetTab(nTab);
                aCellRange.aEnd.SetTab(nTab);
            }
            bFound = true;
        }
        else
        {
            if ( ScRangeUtil::MakeRangeFromName( aName, rDoc, nTab, aCellRange ) ||
                 ScRangeUtil::MakeRangeFromName( aName, rDoc, nTab, aCellRange, RUTL_DBASE ) )
                bFound = true;
        }

        if (bFound)         // valid only if within this object's range
        {
            if (!aRange.In(aCellRange))
                bFound = false;
        }

        if (bFound)
        {
            if ( aCellRange.aStart == aCellRange.aEnd )
                return new ScCellObj( pDocSh, aCellRange.aStart );
            else
                return new ScCellRangeObj( pDocSh, aCellRange );
        }
    }

    throw uno::RuntimeException();
}

// XColumnRowRange

uno::Reference<table::XTableColumns> SAL_CALL ScCellRangeObj::getColumns()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScTableColumnsObj( pDocSh, aRange.aStart.Tab(),
                                        aRange.aStart.Col(), aRange.aEnd.Col() );

    OSL_FAIL("Document invalid");
    return nullptr;
}

uno::Reference<table::XTableRows> SAL_CALL ScCellRangeObj::getRows()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScTableRowsObj( pDocSh, aRange.aStart.Tab(),
                                    aRange.aStart.Row(), aRange.aEnd.Row() );

    OSL_FAIL("Document invalid");
    return nullptr;
}

// XAddressableCellRange

table::CellRangeAddress SAL_CALL ScCellRangeObj::getRangeAddress()
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScUnoConversion::FillApiRange( aRet, aRange );
    return aRet;
}

// XSheetCellRange

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScCellRangeObj::getSpreadsheet()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScTableSheetObj( pDocSh, aRange.aStart.Tab() );

    OSL_FAIL("Document invalid");
    return nullptr;
}

// XArrayFormulaRange

OUString SAL_CALL ScCellRangeObj::getArrayFormula()
{
    SolarMutexGuard aGuard;

    //  Matrix formula if clearly part of a matrix (so when start and end of
    //  the block belong to the same matrix) else empty string.

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return EMPTY_OUSTRING;

    OUString aFormula;

    ScDocument& rDoc = pDocSh->GetDocument();
    ScRefCellValue aCell1(rDoc, aRange.aStart);
    ScRefCellValue aCell2(rDoc, aRange.aEnd);
    if (aCell1.meType == CELLTYPE_FORMULA && aCell2.meType == CELLTYPE_FORMULA)
    {
        const ScFormulaCell* pFCell1 = aCell1.mpFormula;
        const ScFormulaCell* pFCell2 = aCell2.mpFormula;
        ScAddress aStart1;
        ScAddress aStart2;
        if (pFCell1->GetMatrixOrigin(rDoc, aStart1) && pFCell2->GetMatrixOrigin(rDoc, aStart2))
        {
            if (aStart1 == aStart2)               // both the same matrix
                pFCell1->GetFormula(aFormula);    // it doesn't matter from which cell
        }
    }
    return aFormula;
}

void ScCellRangeObj::SetArrayFormula_Impl(const OUString& rFormula,
    const formula::FormulaGrammar::Grammar eGrammar)
{
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;

    if ( !rFormula.isEmpty() )
    {
        if ( comphelper::getUnoTunnelImplementation<ScTableSheetObj>( static_cast<cppu::OWeakObject*>(this) ) )
        {
            //  don't set array formula for sheet object
            throw uno::RuntimeException();
        }

        pDocSh->GetDocFunc().EnterMatrix( aRange, nullptr, nullptr, rFormula, true, true, OUString()/*rFormulaNmsp*/, eGrammar );
    }
    else
    {
        //  empty string -> erase array formula
        ScMarkData aMark(GetDocument()->GetSheetLimits());
        aMark.SetMarkArea( aRange );
        aMark.SelectTable( aRange.aStart.Tab(), true );
        pDocSh->GetDocFunc().DeleteContents( aMark, InsertDeleteFlags::CONTENTS, true, true );
    }
}

void SAL_CALL ScCellRangeObj::setArrayFormula( const OUString& aFormula )
{
    SolarMutexGuard aGuard;
    // GRAM_API for API compatibility.
    SetArrayFormula_Impl( aFormula, formula::FormulaGrammar::GRAM_API);
}

// XArrayFormulaTokens
uno::Sequence<sheet::FormulaToken> SAL_CALL ScCellRangeObj::getArrayTokens()
{
    SolarMutexGuard aGuard;

    // same cell logic as in getArrayFormula

    uno::Sequence<sheet::FormulaToken> aSequence;
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return aSequence;

    ScDocument& rDoc = pDocSh->GetDocument();
    ScRefCellValue aCell1(rDoc, aRange.aStart);
    ScRefCellValue aCell2(rDoc, aRange.aEnd);
    if (aCell1.meType == CELLTYPE_FORMULA && aCell2.meType == CELLTYPE_FORMULA)
    {
        const ScFormulaCell* pFCell1 = aCell1.mpFormula;
        const ScFormulaCell* pFCell2 = aCell2.mpFormula;
        ScAddress aStart1;
        ScAddress aStart2;
        if (pFCell1->GetMatrixOrigin(rDoc, aStart1) && pFCell2->GetMatrixOrigin(rDoc, aStart2))
        {
            if (aStart1 == aStart2)
            {
                const ScTokenArray* pTokenArray = pFCell1->GetCode();
                if (pTokenArray)
                    ScTokenConversion::ConvertToTokenSequence(rDoc, aSequence, *pTokenArray);
            }
        }
    }

    return aSequence;
}

void SAL_CALL ScCellRangeObj::setArrayTokens( const uno::Sequence<sheet::FormulaToken>& rTokens )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    if ( rTokens.hasElements() )
    {
        if ( comphelper::getUnoTunnelImplementation<ScTableSheetObj>( static_cast<cppu::OWeakObject*>(this) ) )
        {
            throw uno::RuntimeException();
        }

        ScDocument& rDoc = pDocSh->GetDocument();
        ScTokenArray aTokenArray(rDoc);
        (void)ScTokenConversion::ConvertToTokenArray( rDoc, aTokenArray, rTokens );

        // Actually GRAM_API is a don't-care here because of the token
        // array being set, it fits with other API compatibility grammars
        // though.
        pDocSh->GetDocFunc().EnterMatrix( aRange, nullptr, &aTokenArray, EMPTY_OUSTRING, true, true, EMPTY_OUSTRING, formula::FormulaGrammar::GRAM_API );
    }
    else
    {
        //  empty sequence -> erase array formula
        ScMarkData aMark(pDocSh->GetDocument().GetSheetLimits());
        aMark.SetMarkArea( aRange );
        aMark.SelectTable( aRange.aStart.Tab(), true );
        pDocSh->GetDocFunc().DeleteContents( aMark, InsertDeleteFlags::CONTENTS, true, true );
    }
}

// XCellRangeData

uno::Sequence< uno::Sequence<uno::Any> > SAL_CALL ScCellRangeObj::getDataArray()
{
    SolarMutexGuard aGuard;

    if ( comphelper::getUnoTunnelImplementation<ScTableSheetObj>( static_cast<cppu::OWeakObject*>(this) ) )
    {
        //  don't create a data array for the sheet
        throw uno::RuntimeException();
    }

    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        uno::Any aAny;
        // bAllowNV = TRUE: errors as void
        if ( ScRangeToSequence::FillMixedArray( aAny, pDocSh->GetDocument(), aRange, true ) )
        {
            uno::Sequence< uno::Sequence<uno::Any> > aSeq;
            if ( aAny >>= aSeq )
                return aSeq;            // success
        }
    }

    throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScCellRangeObj::setDataArray(
                        const uno::Sequence< uno::Sequence<uno::Any> >& aArray )
{
    SolarMutexGuard aGuard;

    bool bDone = false;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        //! move lcl_PutDataArray to docfunc?
        bDone = lcl_PutDataArray( *pDocSh, aRange, aArray );
    }

    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// XCellRangeFormula

uno::Sequence< uno::Sequence<OUString> > SAL_CALL ScCellRangeObj::getFormulaArray()
{
    SolarMutexGuard aGuard;

    if ( comphelper::getUnoTunnelImplementation<ScTableSheetObj>( static_cast<cppu::OWeakObject*>(this) ) )
    {
        //  don't create a data array for the sheet
        throw uno::RuntimeException();
    }

    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        SCCOL nStartCol = aRange.aStart.Col();
        SCROW nStartRow = aRange.aStart.Row();
        SCCOL nEndCol = aRange.aEnd.Col();
        SCROW nEndRow = aRange.aEnd.Row();
        SCCOL nColCount = nEndCol + 1 - nStartCol;
        SCROW nRowCount = nEndRow + 1 - nStartRow;
        SCTAB nTab = aRange.aStart.Tab();

        uno::Sequence< uno::Sequence<OUString> > aRowSeq( nRowCount );
        uno::Sequence<OUString>* pRowAry = aRowSeq.getArray();
        for (SCROW nRowIndex = 0; nRowIndex < nRowCount; nRowIndex++)
        {
            uno::Sequence<OUString> aColSeq( nColCount );
            OUString* pColAry = aColSeq.getArray();
            for (SCCOL nColIndex = 0; nColIndex < nColCount; nColIndex++)
                pColAry[nColIndex] = lcl_GetInputString( pDocSh->GetDocument(),
                                    ScAddress( nStartCol+nColIndex, nStartRow+nRowIndex, nTab ), true );

            pRowAry[nRowIndex] = aColSeq;
        }

        return aRowSeq;
    }

    throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScCellRangeObj::setFormulaArray(
                        const uno::Sequence< uno::Sequence<OUString> >& aArray )
{
    SolarMutexGuard aGuard;

    bool bDone = false;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        ScExternalRefManager::ApiGuard aExtRefGuard(pDocSh->GetDocument());

        // GRAM_API for API compatibility.
        bDone = lcl_PutFormulaArray( *pDocSh, aRange, aArray, formula::FormulaGrammar::GRAM_API );
    }

    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// XMultipleOperation

void SAL_CALL ScCellRangeObj::setTableOperation( const table::CellRangeAddress& aFormulaRange,
                                        sheet::TableOperationMode nMode,
                                        const table::CellAddress& aColumnCell,
                                        const table::CellAddress& aRowCell )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;

    bool bError = false;
    ScTabOpParam aParam;
    aParam.aRefFormulaCell = ScRefAddress( static_cast<SCCOL>(aFormulaRange.StartColumn),
                                          static_cast<SCROW>(aFormulaRange.StartRow), aFormulaRange.Sheet );
    aParam.aRefFormulaEnd  = ScRefAddress( static_cast<SCCOL>(aFormulaRange.EndColumn),
                                          static_cast<SCROW>(aFormulaRange.EndRow), aFormulaRange.Sheet );
    aParam.aRefRowCell     = ScRefAddress( static_cast<SCCOL>(aRowCell.Column),
                                          static_cast<SCROW>(aRowCell.Row), aRowCell.Sheet );
    aParam.aRefColCell     = ScRefAddress( static_cast<SCCOL>(aColumnCell.Column),
                                          static_cast<SCROW>(aColumnCell.Row), aColumnCell.Sheet );

    switch (nMode)
    {
        case sheet::TableOperationMode_COLUMN:
            aParam.meMode = ScTabOpParam::Column;
            break;
        case sheet::TableOperationMode_ROW:
            aParam.meMode = ScTabOpParam::Row;
            break;
        case sheet::TableOperationMode_BOTH:
            aParam.meMode = ScTabOpParam::Both;
            break;
        default:
            bError = true;
    }

    if (!bError)
        pDocSh->GetDocFunc().TabOp( aRange, nullptr, aParam, true, true );
}

// XMergeable

void SAL_CALL ScCellRangeObj::merge( sal_Bool bMerge )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScCellMergeOption aMergeOption(
        aRange.aStart.Col(), aRange.aStart.Row(),
        aRange.aEnd.Col(), aRange.aEnd.Row(), false);
    aMergeOption.maTabs.insert(aRange.aStart.Tab());
    if ( bMerge )
        pDocSh->GetDocFunc().MergeCells( aMergeOption, false, true, true );
    else
        pDocSh->GetDocFunc().UnmergeCells( aMergeOption, true, nullptr );

    //! Catch error?
}

sal_Bool SAL_CALL ScCellRangeObj::getIsMerged()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    return pDocSh && pDocSh->GetDocument().HasAttrib( aRange, HasAttrFlags::Merged );
}

// XCellSeries

void SAL_CALL ScCellRangeObj::fillSeries( sheet::FillDirection nFillDirection,
                        sheet::FillMode nFillMode, sheet::FillDateMode nFillDateMode,
                        double fStep, double fEndValue )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    bool bError = false;

    FillDir eDir = FILL_TO_BOTTOM;
    switch (nFillDirection)
    {
        case sheet::FillDirection_TO_BOTTOM:
            eDir = FILL_TO_BOTTOM;
            break;
        case sheet::FillDirection_TO_RIGHT:
            eDir = FILL_TO_RIGHT;
            break;
        case sheet::FillDirection_TO_TOP:
            eDir = FILL_TO_TOP;
            break;
        case sheet::FillDirection_TO_LEFT:
            eDir = FILL_TO_LEFT;
            break;
        default:
            bError = true;
    }

    FillCmd eCmd = FILL_SIMPLE;
    switch ( nFillMode )
    {
        case sheet::FillMode_SIMPLE:
            eCmd = FILL_SIMPLE;
            break;
        case sheet::FillMode_LINEAR:
            eCmd = FILL_LINEAR;
            break;
        case sheet::FillMode_GROWTH:
            eCmd = FILL_GROWTH;
            break;
        case sheet::FillMode_DATE:
            eCmd = FILL_DATE;
            break;
        case sheet::FillMode_AUTO:
            eCmd = FILL_AUTO;
            break;
        default:
            bError = true;
    }

    FillDateCmd eDateCmd = FILL_DAY;
    switch ( nFillDateMode )
    {
        case sheet::FillDateMode_FILL_DATE_DAY:
            eDateCmd = FILL_DAY;
            break;
        case sheet::FillDateMode_FILL_DATE_WEEKDAY:
            eDateCmd = FILL_WEEKDAY;
            break;
        case sheet::FillDateMode_FILL_DATE_MONTH:
            eDateCmd = FILL_MONTH;
            break;
        case sheet::FillDateMode_FILL_DATE_YEAR:
            eDateCmd = FILL_YEAR;
            break;
        default:
            bError = true;
    }

    if (!bError)
        pDocSh->GetDocFunc().FillSeries( aRange, nullptr, eDir, eCmd, eDateCmd,
                                            MAXDOUBLE, fStep, fEndValue, true );
}

void SAL_CALL ScCellRangeObj::fillAuto( sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !(pDocSh && nSourceCount) )
        return;

    ScRange aSourceRange(aRange);
    SCCOLROW nCount = 0;                   // "Dest-Count"
    FillDir eDir = FILL_TO_BOTTOM;
    bool bError = false;
    switch (nFillDirection)
    {
        case sheet::FillDirection_TO_BOTTOM:
            aSourceRange.aEnd.SetRow( static_cast<SCROW>( aSourceRange.aStart.Row() + nSourceCount - 1 ) );
            nCount = aRange.aEnd.Row() - aSourceRange.aEnd.Row();
            eDir = FILL_TO_BOTTOM;
            break;
        case sheet::FillDirection_TO_RIGHT:
            aSourceRange.aEnd.SetCol( static_cast<SCCOL>( aSourceRange.aStart.Col() + nSourceCount - 1 ) );
            nCount = aRange.aEnd.Col() - aSourceRange.aEnd.Col();
            eDir = FILL_TO_RIGHT;
            break;
        case sheet::FillDirection_TO_TOP:
            aSourceRange.aStart.SetRow( static_cast<SCROW>( aSourceRange.aEnd.Row() - nSourceCount + 1 ) );
            nCount = aSourceRange.aStart.Row() - aRange.aStart.Row();
            eDir = FILL_TO_TOP;
            break;
        case sheet::FillDirection_TO_LEFT:
            aSourceRange.aStart.SetCol( static_cast<SCCOL>( aSourceRange.aEnd.Col() - nSourceCount + 1 ) );
            nCount = aSourceRange.aStart.Col() - aRange.aStart.Col();
            eDir = FILL_TO_LEFT;
            break;
        default:
            bError = true;
    }
    const ScDocument& rDoc = pDocSh->GetDocument();
    if (nCount < 0 || nCount > rDoc.MaxRow())      // overflow
        bError = true;

    if (!bError)
        pDocSh->GetDocFunc().FillAuto( aSourceRange, nullptr, eDir, nCount, true );
}

// XAutoFormattable

void SAL_CALL ScCellRangeObj::autoFormat( const OUString& aName )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScAutoFormat* pAutoFormat = ScGlobal::GetOrCreateAutoFormat();
        ScAutoFormat::const_iterator it = pAutoFormat->find(aName);
        if (it == pAutoFormat->end())
            throw lang::IllegalArgumentException();

        ScAutoFormat::const_iterator itBeg = pAutoFormat->begin();
        size_t nIndex = std::distance(itBeg, it);
        pDocSh->GetDocFunc().AutoFormat(aRange, nullptr, nIndex, true);

    }
}

// XSortable

uno::Sequence<beans::PropertyValue> SAL_CALL ScCellRangeObj::createSortDescriptor()
{
    SolarMutexGuard aGuard;
    ScSortParam aParam;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        // create DB-Area only during execution; API always the exact area
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, ScGetDBSelection::ForceMark );
        if (pData)
        {
            pData->GetSortParam(aParam);

            //  SortDescriptor contains the counted fields inside the area
            ScRange aDBRange;
            pData->GetArea(aDBRange);
            SCCOLROW nFieldStart = aParam.bByRow ?
                static_cast<SCCOLROW>(aDBRange.aStart.Col()) :
                static_cast<SCCOLROW>(aDBRange.aStart.Row());
            for (sal_uInt16 i=0; i<aParam.GetSortKeyCount(); i++)
                if ( aParam.maKeyState[i].bDoSort && aParam.maKeyState[i].nField >= nFieldStart )
                    aParam.maKeyState[i].nField -= nFieldStart;
        }
    }

    uno::Sequence<beans::PropertyValue> aSeq( ScSortDescriptor::GetPropertyCount() );
    ScSortDescriptor::FillProperties( aSeq, aParam );
    return aSeq;
}

void SAL_CALL ScCellRangeObj::sort( const uno::Sequence<beans::PropertyValue>& aDescriptor )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;

    sal_uInt16 i;
    ScSortParam aParam;
    ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_MAKE, ScGetDBSelection::ForceMark ); // if needed create area
    if (pData)
    {
        //  get old settings if not everything is set anew
        pData->GetSortParam(aParam);
        SCCOLROW nOldStart = aParam.bByRow ?
            static_cast<SCCOLROW>(aRange.aStart.Col()) :
            static_cast<SCCOLROW>(aRange.aStart.Row());
        for (i=0; i<aParam.GetSortKeyCount(); i++)
            if ( aParam.maKeyState[i].bDoSort && aParam.maKeyState[i].nField >= nOldStart )
                aParam.maKeyState[i].nField -= nOldStart;
    }

    ScSortDescriptor::FillSortParam( aParam, aDescriptor );

    //  SortDescriptor contains the counted fields inside the area
    //  ByRow can be changed during execution of FillSortParam
    SCCOLROW nFieldStart = aParam.bByRow ?
        static_cast<SCCOLROW>(aRange.aStart.Col()) :
        static_cast<SCCOLROW>(aRange.aStart.Row());
    SCCOLROW nFieldEnd = aParam.bByRow ?
        static_cast<SCCOLROW>(aRange.aEnd.Col()) :
        static_cast<SCCOLROW>(aRange.aEnd.Row());
    for (i=0; i<aParam.GetSortKeyCount(); i++)
    {
        aParam.maKeyState[i].nField += nFieldStart;
        // tdf#103632 - sanity check poorly behaved macros.
        if (aParam.maKeyState[i].nField > nFieldEnd)
            aParam.maKeyState[i].nField = nFieldEnd;
    }

    SCTAB nTab = aRange.aStart.Tab();
    aParam.nCol1 = aRange.aStart.Col();
    aParam.nRow1 = aRange.aStart.Row();
    aParam.nCol2 = aRange.aEnd.Col();
    aParam.nRow2 = aRange.aEnd.Row();

    pDocSh->GetDBData( aRange, SC_DB_MAKE, ScGetDBSelection::ForceMark );       // if needed create area

    ScDBDocFunc aFunc(*pDocSh); // area must be created
    (void)aFunc.Sort( nTab, aParam, true, true, true );
}

// XFilterable

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScCellRangeObj::createFilterDescriptor(
                                sal_Bool bEmpty )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    ScFilterDescriptor* pNew = new ScFilterDescriptor(pDocSh);
    if ( !bEmpty && pDocSh )
    {
        // create DB-Area only during execution; API always the exact area
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, ScGetDBSelection::ForceMark );
        if (pData)
        {
            ScQueryParam aParam;
            pData->GetQueryParam(aParam);
            //  FilterDescriptor contains the counted fields inside the area
            ScRange aDBRange;
            pData->GetArea(aDBRange);
            SCCOLROW nFieldStart = aParam.bByRow ?
                static_cast<SCCOLROW>(aDBRange.aStart.Col()) :
                static_cast<SCCOLROW>(aDBRange.aStart.Row());
            SCSIZE nCount = aParam.GetEntryCount();
            for (SCSIZE i=0; i<nCount; i++)
            {
                ScQueryEntry& rEntry = aParam.GetEntry(i);
                if (rEntry.bDoQuery && rEntry.nField >= nFieldStart)
                    rEntry.nField -= nFieldStart;
            }
            pNew->SetParam(aParam);
        }
    }
    return pNew;
}

void SAL_CALL ScCellRangeObj::filter( const uno::Reference<sheet::XSheetFilterDescriptor>& xDescriptor )
{
    SolarMutexGuard aGuard;

    if (!xDescriptor.is()) return;

    //  This could be theoretically an unknown object, so only use the
    //  public XSheetFilterDescriptor interface to copy the data into a
    //  ScFilterDescriptor object:
    //! if it already a ScFilterDescriptor is, direct via getImplementation?

    ScDocShell* pDocSh = GetDocShell();
    rtl::Reference<ScFilterDescriptor> xImpl(new ScFilterDescriptor(pDocSh));
    uno::Reference< sheet::XSheetFilterDescriptor2 > xDescriptor2( xDescriptor, uno::UNO_QUERY );
    if ( xDescriptor2.is() )
    {
        xImpl->setFilterFields2( xDescriptor2->getFilterFields2() );
    }
    else
    {
        xImpl->setFilterFields( xDescriptor->getFilterFields() );
    }
    //  the rest are now properties...

    uno::Reference<beans::XPropertySet> xPropSet( xDescriptor, uno::UNO_QUERY );
    if (xPropSet.is())
        lcl_CopyProperties(*xImpl, *xPropSet);

    if (!pDocSh)
        return;

    ScQueryParam aParam = xImpl->GetParam();
    //  FilterDescriptor contains the counted fields inside the area
    SCCOLROW nFieldStart = aParam.bByRow ?
        static_cast<SCCOLROW>(aRange.aStart.Col()) :
        static_cast<SCCOLROW>(aRange.aStart.Row());
    SCSIZE nCount = aParam.GetEntryCount();
    svl::SharedStringPool& rPool = pDocSh->GetDocument().GetSharedStringPool();
    for (SCSIZE i=0; i<nCount; i++)
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        if (rEntry.bDoQuery)
        {
            rEntry.nField += nFieldStart;
            //  dialog always shows the string -> must match the value
            ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
            rItems.resize(1);
            ScQueryEntry::Item& rItem = rItems.front();
            if (rItem.meType != ScQueryEntry::ByString)
            {
                OUString aStr;
                pDocSh->GetDocument().GetFormatTable()->GetInputLineString(rItem.mfVal, 0, aStr);
                rItem.maString = rPool.intern(aStr);
            }
        }
    }

    SCTAB nTab = aRange.aStart.Tab();
    aParam.nCol1 = aRange.aStart.Col();
    aParam.nRow1 = aRange.aStart.Row();
    aParam.nCol2 = aRange.aEnd.Col();
    aParam.nRow2 = aRange.aEnd.Row();

    pDocSh->GetDBData( aRange, SC_DB_MAKE, ScGetDBSelection::ForceMark );   // if needed create area

    //! keep source range in filter descriptor
    //! if created by createFilterDescriptorByObject ???

    ScDBDocFunc aFunc(*pDocSh);
    aFunc.Query( nTab, aParam, nullptr, true, true );  // area must be created
}

//! get/setAutoFilter as properties!!!

// XAdvancedFilterSource

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScCellRangeObj::createFilterDescriptorByObject(
                        const uno::Reference<sheet::XSheetFilterable>& xObject )
{
    SolarMutexGuard aGuard;

    //  this here is not the area, which will be filtered, instead the area
    //  with the query

    uno::Reference<sheet::XCellRangeAddressable> xAddr( xObject, uno::UNO_QUERY );

    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh || !xAddr.is() )
    {
        OSL_FAIL("no document or no area");
        return nullptr;
    }

    //! check if xObject is in the same document

    std::unique_ptr<ScFilterDescriptor> pNew(new ScFilterDescriptor(pDocSh));  //! instead from object?

    ScQueryParam aParam = pNew->GetParam();
    aParam.bHasHeader = true;

    table::CellRangeAddress aDataAddress(xAddr->getRangeAddress());
    aParam.nCol1 = static_cast<SCCOL>(aDataAddress.StartColumn);
    aParam.nRow1 = static_cast<SCROW>(aDataAddress.StartRow);
    aParam.nCol2 = static_cast<SCCOL>(aDataAddress.EndColumn);
    aParam.nRow2 = static_cast<SCROW>(aDataAddress.EndRow);
    aParam.nTab  = aDataAddress.Sheet;

    ScDocument& rDoc = pDocSh->GetDocument();
    if (!rDoc.CreateQueryParam(aRange, aParam))
        return nullptr;

    //  FilterDescriptor contains the counted fields inside the area
    SCCOLROW nFieldStart = aParam.bByRow ?
        static_cast<SCCOLROW>(aDataAddress.StartColumn) :
        static_cast<SCCOLROW>(aDataAddress.StartRow);
    SCSIZE nCount = aParam.GetEntryCount();
    for (SCSIZE i=0; i<nCount; i++)
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        if (rEntry.bDoQuery && rEntry.nField >= nFieldStart)
            rEntry.nField -= nFieldStart;
    }

    pNew->SetParam( aParam );
    return pNew.release();
}

// XSubTotalSource

uno::Reference<sheet::XSubTotalDescriptor> SAL_CALL ScCellRangeObj::createSubTotalDescriptor(
                                sal_Bool bEmpty )
{
    SolarMutexGuard aGuard;
    ScSubTotalDescriptor* pNew = new ScSubTotalDescriptor;
    ScDocShell* pDocSh = GetDocShell();
    if ( !bEmpty && pDocSh )
    {
        // create DB-Area only during execution; API always the exact area
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, ScGetDBSelection::ForceMark );
        if (pData)
        {
            ScSubTotalParam aParam;
            pData->GetSubTotalParam(aParam);
            //  SubTotalDescriptor contains the counted fields inside the area
            ScRange aDBRange;
            pData->GetArea(aDBRange);
            SCCOL nFieldStart = aDBRange.aStart.Col();
            for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
            {
                if ( aParam.bGroupActive[i] )
                {
                    if ( aParam.nField[i] >= nFieldStart )
                        aParam.nField[i] = sal::static_int_cast<SCCOL>( aParam.nField[i] - nFieldStart );
                    for (SCCOL j=0; j<aParam.nSubTotals[i]; j++)
                        if ( aParam.pSubTotals[i][j] >= nFieldStart )
                            aParam.pSubTotals[i][j] = sal::static_int_cast<SCCOL>( aParam.pSubTotals[i][j] - nFieldStart );
                }
            }
            pNew->SetParam(aParam);
        }
    }
    return pNew;
}

void SAL_CALL ScCellRangeObj::applySubTotals(
    const uno::Reference<sheet::XSubTotalDescriptor>& xDescriptor,
    sal_Bool bReplace)
{
    SolarMutexGuard aGuard;

    if (!xDescriptor.is()) return;

    ScDocShell* pDocSh = GetDocShell();
    ScSubTotalDescriptorBase* pImp =
        comphelper::getUnoTunnelImplementation<ScSubTotalDescriptorBase>( xDescriptor );

    if (!(pDocSh && pImp))
        return;

    ScSubTotalParam aParam;
    pImp->GetData(aParam);      // virtual method of base class

    //  SubTotalDescriptor contains the counted fields inside the area
    SCCOL nFieldStart = aRange.aStart.Col();
    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        if ( aParam.bGroupActive[i] )
        {
            aParam.nField[i] = sal::static_int_cast<SCCOL>( aParam.nField[i] + nFieldStart );
            for (SCCOL j=0; j<aParam.nSubTotals[i]; j++)
                aParam.pSubTotals[i][j] = sal::static_int_cast<SCCOL>( aParam.pSubTotals[i][j] + nFieldStart );
        }
    }

    aParam.bReplace = bReplace;

    SCTAB nTab = aRange.aStart.Tab();
    aParam.nCol1 = aRange.aStart.Col();
    aParam.nRow1 = aRange.aStart.Row();
    aParam.nCol2 = aRange.aEnd.Col();
    aParam.nRow2 = aRange.aEnd.Row();

    pDocSh->GetDBData( aRange, SC_DB_MAKE, ScGetDBSelection::ForceMark );   // if needed create area

    ScDBDocFunc aFunc(*pDocSh);
    aFunc.DoSubTotals( nTab, aParam, true, true );    // area must be created
}

void SAL_CALL ScCellRangeObj::removeSubTotals()
{
    SolarMutexGuard aGuard;

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;

    ScSubTotalParam aParam;
    ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, ScGetDBSelection::ForceMark );
    if (pData)
        pData->GetSubTotalParam(aParam);    // also keep field entries during remove

    aParam.bRemoveOnly = true;

    SCTAB nTab = aRange.aStart.Tab();
    aParam.nCol1 = aRange.aStart.Col();
    aParam.nRow1 = aRange.aStart.Row();
    aParam.nCol2 = aRange.aEnd.Col();
    aParam.nRow2 = aRange.aEnd.Row();

    pDocSh->GetDBData( aRange, SC_DB_MAKE, ScGetDBSelection::ForceMark );   // if needed create area

    ScDBDocFunc aFunc(*pDocSh);
    aFunc.DoSubTotals( nTab, aParam, true, true );    // are must be created
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScCellRangeObj::createImportDescriptor( sal_Bool bEmpty )
{
    SolarMutexGuard aGuard;
    ScImportParam aParam;
    ScDocShell* pDocSh = GetDocShell();
    if ( !bEmpty && pDocSh )
    {
        // create DB-Area only during execution; API always the exact area
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, ScGetDBSelection::ForceMark );
        if (pData)
            pData->GetImportParam(aParam);
    }

    uno::Sequence<beans::PropertyValue> aSeq( ScImportDescriptor::GetPropertyCount() );
    ScImportDescriptor::FillProperties( aSeq, aParam );
    return aSeq;
}

void SAL_CALL ScCellRangeObj::doImport( const uno::Sequence<beans::PropertyValue>& aDescriptor )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;

    ScImportParam aParam;
    ScImportDescriptor::FillImportParam( aParam, aDescriptor );

    SCTAB nTab = aRange.aStart.Tab();
    aParam.nCol1 = aRange.aStart.Col();
    aParam.nRow1 = aRange.aStart.Row();
    aParam.nCol2 = aRange.aEnd.Col();
    aParam.nRow2 = aRange.aEnd.Row();

    //! TODO: could we get passed a valid result set by any means?

    pDocSh->GetDBData( aRange, SC_DB_MAKE, ScGetDBSelection::ForceMark );       // if needed create area

    ScDBDocFunc aFunc(*pDocSh);                         // are must be created
    aFunc.DoImport( nTab, aParam, nullptr );         //! Api-Flag as parameter
}

// XCellFormatRangesSupplier

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangeObj::getCellFormatRanges()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScCellFormatsObj( pDocSh, aRange );
    return nullptr;
}

// XUniqueCellFormatRangesSupplier

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangeObj::getUniqueCellFormatRanges()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScUniqueCellFormatsObj( pDocSh, aRange );
    return nullptr;
}

// XPropertySet extended for Range-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellRangeObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( pRangePropSet->getPropertyMap() ));
    return aRef;
}

void ScCellRangeObj::SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, const uno::Any& aValue )
{
    //  Range has only Position and Size in addition to ScCellRangesBase, both are ReadOnly
    //  -> nothing to do here

    ScCellRangesBase::SetOnePropertyValue( pEntry, aValue );
}

void ScCellRangeObj::GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, uno::Any& rAny )
{
    if ( !pEntry )
        return;

    if ( pEntry->nWID == SC_WID_UNO_POS )
    {
        ScDocShell* pDocSh = GetDocShell();
        if (pDocSh)
        {
            //  GetMMRect converts using HMM_PER_TWIPS, like the DrawingLayer
            tools::Rectangle aMMRect(pDocSh->GetDocument().GetMMRect(
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() ));
            awt::Point aPos( aMMRect.Left(), aMMRect.Top() );
            rAny <<= aPos;
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_SIZE )
    {
        ScDocShell* pDocSh = GetDocShell();
        if (pDocSh)
        {
            //  GetMMRect converts using HMM_PER_TWIPS, like the DrawingLayer
            tools::Rectangle aMMRect = pDocSh->GetDocument().GetMMRect(
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() );
            Size aSize(aMMRect.GetSize());
            awt::Size aAwtSize( aSize.Width(), aSize.Height() );
            rAny <<= aAwtSize;
        }
    }
    else
        ScCellRangesBase::GetOnePropertyValue( pEntry, rAny );
}

const SfxItemPropertyMap& ScCellRangeObj::GetItemPropertyMap()
{
    return pRangePropSet->getPropertyMap();
}

// XServiceInfo

OUString SAL_CALL ScCellRangeObj::getImplementationName()
{
    return "ScCellRangeObj";
}

sal_Bool SAL_CALL ScCellRangeObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScCellRangeObj::getSupportedServiceNames()
{
    return {SCSHEETCELLRANGE_SERVICE,
            SCCELLRANGE_SERVICE,
            SCCELLPROPERTIES_SERVICE,
            SCCHARPROPERTIES_SERVICE,
            SCPARAPROPERTIES_SERVICE};
}

const SvxItemPropertySet* ScCellObj::GetEditPropertySet()
{
    return lcl_GetEditPropertySet();
}

const SfxItemPropertyMap& ScCellObj::GetCellPropertyMap()
{
    return lcl_GetCellPropertySet()->getPropertyMap();
}

ScCellObj::ScCellObj(ScDocShell* pDocSh, const ScAddress& rP) :
    ScCellRangeObj( pDocSh, ScRange(rP,rP) ),
    pCellPropSet( lcl_GetCellPropertySet() ),
    aCellPos( rP ),
    nActionLockCount( 0 )
{
    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

SvxUnoText& ScCellObj::GetUnoText()
{
    if (!mxUnoText.is())
    {
        mxUnoText.set(new ScCellTextObj(GetDocShell(), aCellPos));
        if (nActionLockCount)
        {
            ScCellEditSource* pEditSource =
                static_cast<ScCellEditSource*> (mxUnoText->GetEditSource());
            if (pEditSource)
                pEditSource->SetDoUpdateData(false);
        }
    }
    return *mxUnoText;
}

ScCellObj::~ScCellObj()
{
}

void ScCellObj::RefChanged()
{
    ScCellRangeObj::RefChanged();

    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE(rRanges.size() == 1, "What ranges ?!?!");
    if ( !rRanges.empty() )
    {
        aCellPos = rRanges[ 0 ].aStart;
    }
}

uno::Any SAL_CALL ScCellObj::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( table::XCell )
    SC_QUERYINTERFACE( table::XCell2 )
    SC_QUERYINTERFACE( sheet::XFormulaTokens )
    SC_QUERYINTERFACE( sheet::XCellAddressable )
    SC_QUERYINTERFACE( text::XText )
    SC_QUERYINTERFACE( text::XSimpleText )
    SC_QUERYINTERFACE( text::XTextRange )
    SC_QUERYINTERFACE( container::XEnumerationAccess )
    SC_QUERYINTERFACE( container::XElementAccess )
    SC_QUERYINTERFACE( sheet::XSheetAnnotationAnchor )
    SC_QUERYINTERFACE( text::XTextFieldsSupplier )
    SC_QUERYINTERFACE( document::XActionLockable )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScCellObj::acquire() throw()
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScCellObj::release() throw()
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellObj::getTypes()
{
    static const uno::Sequence<uno::Type> aTypes = comphelper::concatSequences(
        ScCellRangeObj::getTypes(),
        uno::Sequence<uno::Type>
        {
            cppu::UnoType<table::XCell>::get(),
            cppu::UnoType<sheet::XCellAddressable>::get(),
            cppu::UnoType<text::XText>::get(),
            cppu::UnoType<container::XEnumerationAccess>::get(),
            cppu::UnoType<sheet::XSheetAnnotationAnchor>::get(),
            cppu::UnoType<text::XTextFieldsSupplier>::get(),
            cppu::UnoType<document::XActionLockable>::get(),
            cppu::UnoType<sheet::XFormulaTokens>::get(),
            cppu::UnoType<table::XCell2>::get()
        } );
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// helper methods

OUString ScCellObj::GetInputString_Impl(bool bEnglish) const      // for getFormula / FormulaLocal
{
    if (GetDocShell())
        return lcl_GetInputString( GetDocShell()->GetDocument(), aCellPos, bEnglish );
    return OUString();
}

OUString ScCellObj::GetOutputString_Impl() const
{
    ScDocShell* pDocSh = GetDocShell();
    OUString aVal;
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        ScRefCellValue aCell(rDoc, aCellPos);

        aVal = ScCellFormat::GetOutputString(rDoc, aCellPos, aCell);
    }
    return aVal;
}

void ScCellObj::SetString_Impl(const OUString& rString, bool bInterpret, bool bEnglish)
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        // GRAM_API for API compatibility.
        (void)pDocSh->GetDocFunc().SetCellText(
            aCellPos, rString, bInterpret, bEnglish, true, formula::FormulaGrammar::GRAM_API );
    }
}

double ScCellObj::GetValue_Impl() const
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocument().GetValue( aCellPos );

    return 0.0;
}

void ScCellObj::SetValue_Impl(double fValue)
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocFunc().SetValueCell(aCellPos, fValue, false);
}

// only for XML import

void ScCellObj::InputEnglishString( const OUString& rText )
{
    // This is like a mixture of setFormula and property FormulaLocal:
    // The cell's number format is checked for "text", a new cell format may be set,
    // but all parsing is in English.

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    sal_uInt32 nOldFormat = rDoc.GetNumberFormat( aCellPos );
    if (pFormatter->GetType(nOldFormat) == SvNumFormatType::TEXT)
    {
        SetString_Impl(rText, false, false);      // text cell
        return;
    }

    ScDocFunc &rFunc = pDocSh->GetDocFunc();

    ScInputStringType aRes =
        ScStringUtil::parseInputString(*pFormatter, rText, LANGUAGE_ENGLISH_US);

    if (aRes.meType != ScInputStringType::Unknown)
    {
        if ((nOldFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 && aRes.mnFormatType != SvNumFormatType::ALL)
        {
            // apply a format for the recognized type and the old format's language
            sal_uInt32 nNewFormat = ScGlobal::GetStandardFormat(*pFormatter, nOldFormat, aRes.mnFormatType);
            if (nNewFormat != nOldFormat)
            {
                ScPatternAttr aPattern( rDoc.GetPool() );
                aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
                // ATTR_LANGUAGE_FORMAT remains unchanged
                rFunc.ApplyAttributes( *GetMarkData(), aPattern, true );
            }
        }
    }
    switch (aRes.meType)
    {
        case ScInputStringType::Formula:
            rFunc.SetFormulaCell(
                aCellPos,
                new ScFormulaCell(rDoc, aCellPos, aRes.maText, formula::FormulaGrammar::GRAM_API),
                false);
        break;
        case ScInputStringType::Number:
            rFunc.SetValueCell(aCellPos, aRes.mfValue, false);
        break;
        case ScInputStringType::Text:
            rFunc.SetStringOrEditCell(aCellPos, aRes.maText, false);
        break;
        default:
            SetString_Impl(rText, false, false); // probably empty string
    }
}

//  XText

uno::Reference<text::XTextCursor> SAL_CALL ScCellObj::createTextCursor()
{
    SolarMutexGuard aGuard;
    return new ScCellTextCursor( *this );
}

uno::Reference<text::XTextCursor> SAL_CALL ScCellObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
{
    SolarMutexGuard aGuard;
    SvxUnoTextCursor* pCursor = new ScCellTextCursor( *this );
    uno::Reference<text::XTextCursor> xCursor(pCursor);

    SvxUnoTextRangeBase* pRange = comphelper::getUnoTunnelImplementation<SvxUnoTextRangeBase>( aTextPosition );
    if(pRange)
        pCursor->SetSelection( pRange->GetSelection() );
    else
    {
        ScCellTextCursor* pOther = comphelper::getUnoTunnelImplementation<ScCellTextCursor>( aTextPosition );
        if(!pOther)
            throw uno::RuntimeException();

        pCursor->SetSelection( pOther->GetSelection() );

    }

    return xCursor;
}

OUString SAL_CALL ScCellObj::getString()
{
    SolarMutexGuard aGuard;
    return GetOutputString_Impl();
}

void SAL_CALL ScCellObj::setString( const OUString& aText )
{
    SolarMutexGuard aGuard;
    SetString_Impl(aText, false, false);  // always text

    // don't create pUnoText here if not there
    if (mxUnoText.is())
        mxUnoText->SetSelection(ESelection( 0,0, 0,aText.getLength() ));
}

void SAL_CALL ScCellObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
{
    // special handling for ScCellTextCursor is no longer needed,
    // SvxUnoText::insertString checks for SvxUnoTextRangeBase instead of SvxUnoTextRange

    SolarMutexGuard aGuard;
    GetUnoText().insertString(xRange, aString, bAbsorb);
}

void SAL_CALL ScCellObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                                sal_Int16 nControlCharacter, sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;
    GetUnoText().insertControlCharacter(xRange, nControlCharacter, bAbsorb);
}

void SAL_CALL ScCellObj::insertTextContent( const uno::Reference<text::XTextRange >& xRange,
                                                const uno::Reference<text::XTextContent >& xContent,
                                                sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && xContent.is() )
    {
        ScEditFieldObj* pCellField = comphelper::getUnoTunnelImplementation<ScEditFieldObj>(xContent);
        SvxUnoTextRangeBase* pTextRange = comphelper::getUnoTunnelImplementation<ScCellTextCursor>( xRange );

        if ( pCellField && !pCellField->IsInserted() && pTextRange )
        {
            SvxEditSource* pEditSource = pTextRange->GetEditSource();
            ESelection aSelection(pTextRange->GetSelection());

            if (!bAbsorb)
            {
                //  do not replace -> append
                aSelection.Adjust();
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos  = aSelection.nEndPos;
            }

            if (pCellField->GetFieldType() == text::textfield::Type::TABLE)
                pCellField->setPropertyValue(SC_UNONAME_TABLEPOS, uno::makeAny<sal_Int32>(aCellPos.Tab()));

            SvxFieldItem aItem = pCellField->CreateFieldItem();
            SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
            pForwarder->QuickInsertField( aItem, aSelection );
            pEditSource->UpdateData();

            //  new selection: a digit
            aSelection.Adjust();
            aSelection.nEndPara = aSelection.nStartPara;
            aSelection.nEndPos = aSelection.nStartPos + 1;
            uno::Reference<text::XTextRange> xParent(this);
            pCellField->InitDoc(
                xParent, std::make_unique<ScCellEditSource>(pDocSh, aCellPos), aSelection);

            //  for bAbsorb=FALSE, the new selection must be behind the inserted content
            //  (the xml filter relies on this)
            if (!bAbsorb)
                aSelection.nStartPos = aSelection.nEndPos;

            pTextRange->SetSelection( aSelection );

            return;
        }
    }
    GetUnoText().insertTextContent(xRange, xContent, bAbsorb);
}

void SAL_CALL ScCellObj::removeTextContent( const uno::Reference<text::XTextContent>& xContent )
{
    SolarMutexGuard aGuard;
    if ( xContent.is() )
    {
        ScEditFieldObj* pCellField = comphelper::getUnoTunnelImplementation<ScEditFieldObj>(xContent);
        if ( pCellField && pCellField->IsInserted() )
        {
            //! Check if field is in this cell
            pCellField->DeleteField();
            return;
        }
    }
    GetUnoText().removeTextContent(xContent);
}

uno::Reference<text::XText> SAL_CALL ScCellObj::getText()
{
    return this;
}

uno::Reference<text::XTextRange> SAL_CALL ScCellObj::getStart()
{
    SolarMutexGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScCellObj::getEnd()
{
    SolarMutexGuard aGuard;
    return GetUnoText().getEnd();
}

uno::Reference<container::XEnumeration> SAL_CALL ScCellObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return GetUnoText().createEnumeration();
}

uno::Type SAL_CALL ScCellObj::getElementType()
{
    SolarMutexGuard aGuard;
    return GetUnoText().getElementType();
}

sal_Bool SAL_CALL ScCellObj::hasElements()
{
    SolarMutexGuard aGuard;
    return GetUnoText().hasElements();
}

//  XCell

OUString SAL_CALL ScCellObj::getFormula()
{
    SolarMutexGuard aGuard;
    return GetInputString_Impl( true /* English */ );
}

void SAL_CALL ScCellObj::setFormula( const OUString& aFormula )
{
    SolarMutexGuard aGuard;
    SetString_Impl(aFormula, true, true); // Interpret as English
}

double SAL_CALL ScCellObj::getValue()
{
    SolarMutexGuard aGuard;
    return GetValue_Impl();
}

void SAL_CALL ScCellObj::setValue( double nValue )
{
    SolarMutexGuard aGuard;
    SetValue_Impl(nValue);
}

void SAL_CALL ScCellObj::setFormulaString( const OUString& aFormula)
{
    SolarMutexGuard aGuard;
    ScDocShell *pDocSh = GetDocShell();
    if( pDocSh )
    {
        ScFormulaCell* pCell = new ScFormulaCell( pDocSh->GetDocument(), aCellPos );
        pCell->SetHybridFormula( aFormula, formula::FormulaGrammar::GRAM_NATIVE );
        pDocSh->GetDocFunc().SetFormulaCell(aCellPos, pCell, false);
    }
}
void SAL_CALL ScCellObj::setFormulaResult( double nValue )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        ScRefCellValue aCell(pDocSh->GetDocument(), aCellPos);
        if (aCell.meType == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pCell = aCell.mpFormula;
            pCell->SetHybridDouble( nValue );
            pCell->ResetDirty();
            pCell->SetChanged(false);
        }
    }
}

table::CellContentType SAL_CALL ScCellObj::getType()
{
    SolarMutexGuard aGuard;
    table::CellContentType eRet = table::CellContentType_EMPTY;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        CellType eCalcType = pDocSh->GetDocument().GetCellType( aCellPos );
        switch (eCalcType)
        {
            case CELLTYPE_VALUE:
                eRet = table::CellContentType_VALUE;
                break;
            case CELLTYPE_STRING:
            case CELLTYPE_EDIT:
                eRet = table::CellContentType_TEXT;
                break;
            case CELLTYPE_FORMULA:
                eRet = table::CellContentType_FORMULA;
                break;
            default:
                eRet = table::CellContentType_EMPTY;
        }
    }
    else
    {
        OSL_FAIL("no DocShell");     //! Exception or so?
    }

    return eRet;
}

sal_Int32 ScCellObj::GetResultType_Impl() const
{
    SolarMutexGuard aGuard;
    sal_Int32 eRet = sheet::FormulaResult::STRING;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        if (pDocSh->GetDocument().GetCellType(aCellPos) == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = pDocSh->GetDocument().GetFormulaCell(aCellPos);
            if (!pFCell)
            {
                // should throw instead of default string?
            }
            else if (pFCell->GetErrCode() != FormulaError::NONE )
            {
                eRet = sheet::FormulaResult::ERROR;
            }
            else if (pFCell->IsValue())
            {
                eRet = sheet::FormulaResult::VALUE;
            }
            else
            {
                eRet = sheet::FormulaResult::STRING;
            }
        }
    }
    else
    {
        OSL_FAIL("no DocShell");
    }

    return eRet;
}

table::CellContentType ScCellObj::GetContentType_Impl()
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScRefCellValue aCell(pDocSh->GetDocument(), aCellPos);
        if (aCell.meType == CELLTYPE_FORMULA)
        {
            bool bValue = aCell.mpFormula->IsValue();
            return bValue ? table::CellContentType_VALUE : table::CellContentType_TEXT;
        }
    }
    return getType();
}

sal_Int32 SAL_CALL ScCellObj::getError()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
    {
        OSL_FAIL("no DocShell");     //! Exception or so?
        return 0;
    }

    FormulaError nError = FormulaError::NONE;
    ScRefCellValue aCell(pDocSh->GetDocument(), aCellPos);
    if (aCell.meType == CELLTYPE_FORMULA)
        nError = aCell.mpFormula->GetErrCode();

    return static_cast<sal_Int32>(nError);
}

// XFormulaTokens

uno::Sequence<sheet::FormulaToken> SAL_CALL ScCellObj::getTokens()
{
    SolarMutexGuard aGuard;
    uno::Sequence<sheet::FormulaToken> aSequence;
    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return aSequence;

    ScDocument& rDoc = pDocSh->GetDocument();
    ScRefCellValue aCell(rDoc, aCellPos);
    if (aCell.meType == CELLTYPE_FORMULA)
    {
        ScTokenArray* pTokenArray = aCell.mpFormula->GetCode();
        if (pTokenArray)
            ScTokenConversion::ConvertToTokenSequence(rDoc, aSequence, *pTokenArray);
    }
    return aSequence;
}

void SAL_CALL ScCellObj::setTokens( const uno::Sequence<sheet::FormulaToken>& rTokens )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        ScTokenArray aTokenArray(rDoc);
        (void)ScTokenConversion::ConvertToTokenArray( rDoc, aTokenArray, rTokens );

        ScFormulaCell* pNewCell = new ScFormulaCell(rDoc, aCellPos, aTokenArray);
        (void)pDocSh->GetDocFunc().SetFormulaCell(aCellPos, pNewCell, false);
    }
}

// XCellAddressable

table::CellAddress SAL_CALL ScCellObj::getCellAddress()
{
    SolarMutexGuard aGuard;
    table::CellAddress aAdr;
    aAdr.Sheet  = aCellPos.Tab();
    aAdr.Column = aCellPos.Col();
    aAdr.Row    = aCellPos.Row();
    return aAdr;
}

// XSheetAnnotationAnchor

uno::Reference<sheet::XSheetAnnotation> SAL_CALL ScCellObj::getAnnotation()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScAnnotationObj( pDocSh, aCellPos );

    OSL_FAIL("getAnnotation without DocShell");
    return nullptr;
}

// XFieldTypesSupplier

uno::Reference<container::XEnumerationAccess> SAL_CALL ScCellObj::getTextFields()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        uno::Reference<text::XTextRange> xContent(this);
        return new ScCellFieldsObj(xContent, pDocSh, aCellPos);
    }

    return nullptr;
}

uno::Reference<container::XNameAccess> SAL_CALL ScCellObj::getTextFieldMasters()
{
    //  there is no such thing in Calc (?)
    return nullptr;
}

// XPropertySet extended for Cell-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( pCellPropSet->getPropertyMap() ));
    return aRef;
}

void ScCellObj::SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, const uno::Any& aValue )
{
    if ( !pEntry )
        return;

    if ( pEntry->nWID == SC_WID_UNO_FORMLOC )
    {
        OUString aStrVal;
        aValue >>= aStrVal;
        SetString_Impl(aStrVal, true, false);   // interpret locally
    }
    else if ( pEntry->nWID == SC_WID_UNO_FORMRT || pEntry->nWID == SC_WID_UNO_FORMRT2
              || pEntry->nWID == SC_WID_UNO_CELLCONTENTTYPE )
    {
        //  Read-Only
        //! Exception or so...
    }
    else
        ScCellRangeObj::SetOnePropertyValue( pEntry, aValue );
}

void ScCellObj::GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, uno::Any& rAny )
{
    if ( !pEntry )
        return;

    if ( pEntry->nWID == SC_WID_UNO_FORMLOC )
    {
        // sal_False = local
        rAny <<= GetInputString_Impl(false);
    }
    else if ( pEntry->nWID == SC_WID_UNO_FORMRT2 )
    {
        sal_Int32 eType = GetResultType_Impl();
        rAny <<= eType;
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLCONTENTTYPE || pEntry->nWID == SC_WID_UNO_FORMRT )
    {
        table::CellContentType eType = GetContentType_Impl();
        rAny <<= eType;
    }
    else
        ScCellRangeObj::GetOnePropertyValue(pEntry, rAny);
}

const SfxItemPropertyMap& ScCellObj::GetItemPropertyMap()
{
    return pCellPropSet->getPropertyMap();
}

// XServiceInfo

OUString SAL_CALL ScCellObj::getImplementationName()
{
    return "ScCellObj";
}

sal_Bool SAL_CALL ScCellObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScCellObj::getSupportedServiceNames()
{
    return {SCSHEETCELL_SERVICE,
            SCCELL_SERVICE,
            SCCELLPROPERTIES_SERVICE,
            SCCHARPROPERTIES_SERVICE,
            SCPARAPROPERTIES_SERVICE,
            SCSHEETCELLRANGE_SERVICE,
            SCCELLRANGE_SERVICE};
}

// XActionLockable

sal_Bool SAL_CALL ScCellObj::isActionLocked()
{
    SolarMutexGuard aGuard;
    return nActionLockCount != 0;
}

void SAL_CALL ScCellObj::addActionLock()
{
    SolarMutexGuard aGuard;
    if (!nActionLockCount)
    {
        if (mxUnoText.is())
        {
            ScCellEditSource* pEditSource =
                static_cast<ScCellEditSource*> (mxUnoText->GetEditSource());
            if (pEditSource)
                pEditSource->SetDoUpdateData(false);
        }
    }
    nActionLockCount++;
}

void SAL_CALL ScCellObj::removeActionLock()
{
    SolarMutexGuard aGuard;
    if (nActionLockCount <= 0)
        return;

    nActionLockCount--;
    if (nActionLockCount)
        return;

    if (mxUnoText.is())
    {
        ScCellEditSource* pEditSource =
            static_cast<ScCellEditSource*> (mxUnoText->GetEditSource());
        if (pEditSource)
        {
            pEditSource->SetDoUpdateData(true);
            if (pEditSource->IsDirty())
                pEditSource->UpdateData();
        }
    }
}

void SAL_CALL ScCellObj::setActionLocks( sal_Int16 nLock )
{
    SolarMutexGuard aGuard;
    if (mxUnoText.is())
    {
        ScCellEditSource* pEditSource =
            static_cast<ScCellEditSource*> (mxUnoText->GetEditSource());
        if (pEditSource)
        {
            pEditSource->SetDoUpdateData(nLock == 0);
            if ((nActionLockCount > 0) && (nLock == 0) && pEditSource->IsDirty())
                pEditSource->UpdateData();
        }
    }
    nActionLockCount = nLock;
}

sal_Int16 SAL_CALL ScCellObj::resetActionLocks()
{
    SolarMutexGuard aGuard;
    sal_uInt16 nRet(nActionLockCount);
    if (mxUnoText.is())
    {
        ScCellEditSource* pEditSource =
            static_cast<ScCellEditSource*> (mxUnoText->GetEditSource());
        if (pEditSource)
        {
            pEditSource->SetDoUpdateData(true);
            if (pEditSource->IsDirty())
                pEditSource->UpdateData();
        }
    }
    nActionLockCount = 0;
    return nRet;
}

static ScRange MaxDocRange(ScDocShell* pDocSh, SCTAB nTab)
{
    const SCCOL nMaxcol = pDocSh ? pDocSh->GetDocument().MaxCol() : MAXCOL;
    const SCROW nMaxRow = pDocSh ? pDocSh->GetDocument().MaxRow() : MAXROW;
    return ScRange(0, 0, nTab, nMaxcol, nMaxRow, nTab);
}

ScTableSheetObj::ScTableSheetObj( ScDocShell* pDocSh, SCTAB nTab ) :
    ScCellRangeObj( pDocSh, MaxDocRange(pDocSh, nTab) ),
    pSheetPropSet(lcl_GetSheetPropertySet())
{
}

ScTableSheetObj::~ScTableSheetObj()
{
}

void ScTableSheetObj::InitInsertSheet(ScDocShell* pDocSh, SCTAB nTab)
{
    ScDocument& rDoc = pDocSh->GetDocument();
    InitInsertRange( pDocSh, ScRange(0,0,nTab, rDoc.MaxCol(),rDoc.MaxRow(),nTab) );
}

uno::Any SAL_CALL ScTableSheetObj::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( sheet::XSpreadsheet )
    SC_QUERYINTERFACE( container::XNamed )
    SC_QUERYINTERFACE( sheet::XSheetPageBreak )
    SC_QUERYINTERFACE( sheet::XCellRangeMovement )
    SC_QUERYINTERFACE( table::XTableChartsSupplier )
    SC_QUERYINTERFACE( sheet::XDataPilotTablesSupplier )
    SC_QUERYINTERFACE( sheet::XScenariosSupplier )
    SC_QUERYINTERFACE( sheet::XSheetAnnotationsSupplier )
    SC_QUERYINTERFACE( drawing::XDrawPageSupplier )
    SC_QUERYINTERFACE( sheet::XPrintAreas )
    SC_QUERYINTERFACE( sheet::XSheetAuditing )
    SC_QUERYINTERFACE( sheet::XSheetOutline )
    SC_QUERYINTERFACE( util::XProtectable )
    SC_QUERYINTERFACE( sheet::XScenario )
    SC_QUERYINTERFACE( sheet::XScenarioEnhanced )
    SC_QUERYINTERFACE( sheet::XSheetLinkable )
    SC_QUERYINTERFACE( sheet::XExternalSheetName )
    SC_QUERYINTERFACE( document::XEventsSupplier )
    SC_QUERYINTERFACE( table::XTablePivotChartsSupplier )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScTableSheetObj::acquire() throw()
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScTableSheetObj::release() throw()
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScTableSheetObj::getTypes()
{
    static const uno::Sequence<uno::Type> aTypes = comphelper::concatSequences(
        ScCellRangeObj::getTypes(),
        uno::Sequence<uno::Type>
        {
            cppu::UnoType<sheet::XSpreadsheet>::get(),
            cppu::UnoType<container::XNamed>::get(),
            cppu::UnoType<sheet::XSheetPageBreak>::get(),
            cppu::UnoType<sheet::XCellRangeMovement>::get(),
            cppu::UnoType<table::XTableChartsSupplier>::get(),
            cppu::UnoType<sheet::XDataPilotTablesSupplier>::get(),
            cppu::UnoType<sheet::XScenariosSupplier>::get(),
            cppu::UnoType<sheet::XSheetAnnotationsSupplier>::get(),
            cppu::UnoType<drawing::XDrawPageSupplier>::get(),
            cppu::UnoType<sheet::XPrintAreas>::get(),
            cppu::UnoType<sheet::XSheetAuditing>::get(),
            cppu::UnoType<sheet::XSheetOutline>::get(),
            cppu::UnoType<util::XProtectable>::get(),
            cppu::UnoType<sheet::XScenario>::get(),
            cppu::UnoType<sheet::XScenarioEnhanced>::get(),
            cppu::UnoType<sheet::XSheetLinkable>::get(),
            cppu::UnoType<sheet::XExternalSheetName>::get(),
            cppu::UnoType<document::XEventsSupplier>::get(),
            cppu::UnoType<table::XTablePivotChartsSupplier>::get()
        } );
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScTableSheetObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

//  Helper functions

SCTAB ScTableSheetObj::GetTab_Impl() const
{
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE(rRanges.size() == 1, "What ranges ?!?!");
    if ( !rRanges.empty() )
    {
        return rRanges[ 0 ].aStart.Tab();
    }
    return 0;
}

// former XSheet

uno::Reference<table::XTableCharts> SAL_CALL ScTableSheetObj::getCharts()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScChartsObj( pDocSh, GetTab_Impl() );

    OSL_FAIL("no document");
    return nullptr;
}

uno::Reference<table::XTablePivotCharts> SAL_CALL ScTableSheetObj::getPivotCharts()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new sc::TablePivotCharts(pDocSh, GetTab_Impl());

    OSL_FAIL("no document");
    return nullptr;
}

uno::Reference<sheet::XDataPilotTables> SAL_CALL ScTableSheetObj::getDataPilotTables()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScDataPilotTablesObj( pDocSh, GetTab_Impl() );

    OSL_FAIL("no document");
    return nullptr;
}

uno::Reference<sheet::XScenarios> SAL_CALL ScTableSheetObj::getScenarios()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();

    if ( pDocSh )
        return new ScScenariosObj( pDocSh, GetTab_Impl() );

    OSL_FAIL("no document");
    return nullptr;
}

uno::Reference<sheet::XSheetAnnotations> SAL_CALL ScTableSheetObj::getAnnotations()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();

    if ( pDocSh )
        return new ScAnnotationsObj( pDocSh, GetTab_Impl() );

    OSL_FAIL("no document");
    return nullptr;
}

uno::Reference<table::XCellRange> SAL_CALL ScTableSheetObj::getCellRangeByName(
                        const OUString& rRange )
{
    SolarMutexGuard aGuard;
    return ScCellRangeObj::getCellRangeByName( rRange );
}

uno::Reference<sheet::XSheetCellCursor> SAL_CALL ScTableSheetObj::createCursor()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        //! single cell or whole table??????
        const ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();
        return new ScCellCursorObj( pDocSh, ScRange( 0,0,nTab, rDoc.MaxCol(),rDoc.MaxRow(),nTab ) );
    }
    return nullptr;
}

uno::Reference<sheet::XSheetCellCursor> SAL_CALL ScTableSheetObj::createCursorByRange(
                        const uno::Reference<sheet::XSheetCellRange>& xCellRange )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && xCellRange.is() )
    {
        ScCellRangesBase* pRangesImp = comphelper::getUnoTunnelImplementation<ScCellRangesBase>( xCellRange );
        if (pRangesImp)
        {
            const ScRangeList& rRanges = pRangesImp->GetRangeList();
            OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
            return new ScCellCursorObj( pDocSh, rRanges[ 0 ] );
        }
    }
    return nullptr;
}

// XSheetCellRange

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScTableSheetObj::getSpreadsheet()
{
    return this;        //!???
}

// XCellRange

uno::Reference<table::XCell> SAL_CALL ScTableSheetObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
{
    SolarMutexGuard aGuard;
    return ScCellRangeObj::GetCellByPosition_Impl(nColumn, nRow);
}

uno::Reference<table::XCellRange> SAL_CALL ScTableSheetObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
{
    SolarMutexGuard aGuard;
    return ScCellRangeObj::getCellRangeByPosition(nLeft,nTop,nRight,nBottom);
}

uno::Sequence<sheet::TablePageBreakData> SAL_CALL ScTableSheetObj::getColumnPageBreaks()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();

        Size aSize(rDoc.GetPageSize( nTab ));
        if (aSize.Width() && aSize.Height())        // effective size already set?
            rDoc.UpdatePageBreaks( nTab );
        else
        {
            //  update breaks like in ScDocShell::PageStyleModified:
            ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab );
            aPrintFunc.UpdatePages();
        }

        SCCOL nCount = 0;
        for (SCCOL nCol : rDoc.GetColumnsRange(nTab, 0, rDoc.MaxCol()))
            if (rDoc.HasColBreak(nCol, nTab) != ScBreakType::NONE)
                ++nCount;

        sheet::TablePageBreakData aData;
        uno::Sequence<sheet::TablePageBreakData> aSeq(nCount);
        sheet::TablePageBreakData* pAry = aSeq.getArray();
        sal_uInt16 nPos = 0;
        for (SCCOL nCol : rDoc.GetColumnsRange(nTab, 0, rDoc.MaxCol()))
        {
            ScBreakType nBreak = rDoc.HasColBreak(nCol, nTab);
            if (nBreak != ScBreakType::NONE)
            {
                aData.Position    = nCol;
                aData.ManualBreak = bool(nBreak & ScBreakType::Manual);
                pAry[nPos] = aData;
                ++nPos;
            }
        }
        return aSeq;
    }
    return uno::Sequence<sheet::TablePageBreakData>(0);
}

uno::Sequence<sheet::TablePageBreakData> SAL_CALL ScTableSheetObj::getRowPageBreaks()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();

        Size aSize(rDoc.GetPageSize( nTab ));
        if (aSize.Width() && aSize.Height())        // effective size already set?
            rDoc.UpdatePageBreaks( nTab );
        else
        {
            //  update breaks like in ScDocShell::PageStyleModified:
            ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab );
            aPrintFunc.UpdatePages();
        }
        return rDoc.GetRowBreakData(nTab);
    }
    return uno::Sequence<sheet::TablePageBreakData>(0);
}

void SAL_CALL ScTableSheetObj::removeAllManualPageBreaks()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    //! DocFunc function, also for ScViewFunc::RemoveManualBreaks

    ScDocument& rDoc = pDocSh->GetDocument();
    bool bUndo (rDoc.IsUndoEnabled());
    SCTAB nTab = GetTab_Impl();

    if (bUndo)
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( rDoc, nTab, nTab, true, true );
        rDoc.CopyToDocument(0,0,nTab, rDoc.MaxCol(),rDoc.MaxRow(),nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
        pDocSh->GetUndoManager()->AddUndoAction(
                                std::make_unique<ScUndoRemoveBreaks>( pDocSh, nTab, std::move(pUndoDoc) ) );
    }

    rDoc.RemoveManualBreaks(nTab);
    rDoc.UpdatePageBreaks(nTab);

    //? UpdatePageBreakData( sal_True );
    pDocSh->SetDocumentModified();
    pDocSh->PostPaint(ScRange(0, 0, nTab, rDoc.MaxCol(), rDoc.MaxRow(), nTab), PaintPartFlags::Grid);
}

// XNamed

OUString SAL_CALL ScTableSheetObj::getName()
{
    SolarMutexGuard aGuard;
    OUString aName;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocument().GetName( GetTab_Impl(), aName );
    return aName;
}

void SAL_CALL ScTableSheetObj::setName( const OUString& aNewName )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        pDocSh->GetDocFunc().RenameTable( GetTab_Impl(), aNewName, true, true );
    }
}

// XDrawPageSupplier

uno::Reference<drawing::XDrawPage> SAL_CALL ScTableSheetObj::getDrawPage()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDrawLayer* pDrawLayer = pDocSh->MakeDrawLayer();
        OSL_ENSURE(pDrawLayer,"Cannot create Draw-Layer");

        SCTAB nTab = GetTab_Impl();
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Draw-Page not found");
        if (pPage)
            return uno::Reference<drawing::XDrawPage> (pPage->getUnoPage(), uno::UNO_QUERY);

        //  The DrawPage object will register itself as a Listener at SdrModel
        //  and should receive all action from there
    }
    return nullptr;
}

// XCellMovement

void SAL_CALL ScTableSheetObj::insertCells( const table::CellRangeAddress& rRangeAddress,
                                sheet::CellInsertMode nMode )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    bool bDo = true;
    InsCellCmd eCmd = INS_NONE;
    switch (nMode)
    {
        case sheet::CellInsertMode_NONE:    bDo = false;                break;
        case sheet::CellInsertMode_DOWN:    eCmd = INS_CELLSDOWN;       break;
        case sheet::CellInsertMode_RIGHT:   eCmd = INS_CELLSRIGHT;      break;
        case sheet::CellInsertMode_ROWS:    eCmd = INS_INSROWS_BEFORE;  break;
        case sheet::CellInsertMode_COLUMNS: eCmd = INS_INSCOLS_BEFORE;  break;
        default:
            OSL_FAIL("insertCells: wrong mode");
            bDo = false;
    }

    if (bDo)
    {
        OSL_ENSURE( rRangeAddress.Sheet == GetTab_Impl(), "wrong table in CellRangeAddress" );
        ScRange aScRange;
        ScUnoConversion::FillScRange( aScRange, rRangeAddress );
        (void)pDocSh->GetDocFunc().InsertCells( aScRange, nullptr, eCmd, true, true );
    }
}

void SAL_CALL ScTableSheetObj::removeRange( const table::CellRangeAddress& rRangeAddress,
                                sheet::CellDeleteMode nMode )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    bool bDo = true;
    DelCellCmd eCmd = DelCellCmd::NONE;
    switch (nMode)
    {
        case sheet::CellDeleteMode_NONE:     bDo = false;           break;
        case sheet::CellDeleteMode_UP:       eCmd = DelCellCmd::CellsUp;    break;
        case sheet::CellDeleteMode_LEFT:     eCmd = DelCellCmd::CellsLeft;  break;
        case sheet::CellDeleteMode_ROWS:     eCmd = DelCellCmd::Rows;    break;
        case sheet::CellDeleteMode_COLUMNS:  eCmd = DelCellCmd::Cols;    break;
        default:
            OSL_FAIL("deleteCells: wrong mode");
            bDo = false;
    }

    if (bDo)
    {
        OSL_ENSURE( rRangeAddress.Sheet == GetTab_Impl(), "wrong table in CellRangeAddress" );
        ScRange aScRange;
        ScUnoConversion::FillScRange( aScRange, rRangeAddress );
        (void)pDocSh->GetDocFunc().DeleteCells( aScRange, nullptr, eCmd, true );
    }
}

void SAL_CALL ScTableSheetObj::moveRange( const table::CellAddress& aDestination,
                                        const table::CellRangeAddress& aSource )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        OSL_ENSURE( aSource.Sheet == GetTab_Impl(), "wrong table in CellRangeAddress" );
        ScRange aSourceRange;
        ScUnoConversion::FillScRange( aSourceRange, aSource );
        ScAddress aDestPos( static_cast<SCCOL>(aDestination.Column), static_cast<SCROW>(aDestination.Row), aDestination.Sheet );
        (void)pDocSh->GetDocFunc().MoveBlock( aSourceRange, aDestPos, true, true, true, true );
    }
}

void SAL_CALL ScTableSheetObj::copyRange( const table::CellAddress& aDestination,
                                        const table::CellRangeAddress& aSource )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        OSL_ENSURE( aSource.Sheet == GetTab_Impl(), "wrong table in CellRangeAddress" );
        ScRange aSourceRange;
        ScUnoConversion::FillScRange( aSourceRange, aSource );
        ScAddress aDestPos( static_cast<SCCOL>(aDestination.Column), static_cast<SCROW>(aDestination.Row), aDestination.Sheet );
        (void)pDocSh->GetDocFunc().MoveBlock( aSourceRange, aDestPos, false, true, true, true );
    }
}

// XPrintAreas

void ScTableSheetObj::PrintAreaUndo_Impl( std::unique_ptr<ScPrintRangeSaver> pOldRanges )
{
    //  page break and undo
    ScDocShell* pDocSh = GetDocShell();

    if(!pDocSh)
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    const bool bUndo(rDoc.IsUndoEnabled());
    const SCTAB nTab(GetTab_Impl());

    if(bUndo)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoPrintRange>(
                pDocSh,
                nTab,
                std::move(pOldRanges),
                rDoc.CreatePrintRangeSaver())); // create new ranges
    }

    ScPrintFunc(pDocSh, pDocSh->GetPrinter(), nTab).UpdatePages();
    SfxBindings* pBindings = pDocSh->GetViewBindings();

    if(pBindings)
    {
        pBindings->Invalidate(SID_DELETE_PRINTAREA);
    }

    pDocSh->SetDocumentModified();
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScTableSheetObj::getPrintAreas()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();
        sal_uInt16 nCount = rDoc.GetPrintRangeCount( nTab );

        table::CellRangeAddress aRangeAddress;
        uno::Sequence<table::CellRangeAddress> aSeq(nCount);
        table::CellRangeAddress* pAry = aSeq.getArray();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            const ScRange* pRange = rDoc.GetPrintRange( nTab, i );
            OSL_ENSURE(pRange,"where is the printing area");
            if (pRange)
            {
                ScUnoConversion::FillApiRange( aRangeAddress, *pRange );
                aRangeAddress.Sheet = nTab; // core does not care about sheet index
                pAry[i] = aRangeAddress;
            }
        }
        return aSeq;
    }
    return uno::Sequence<table::CellRangeAddress>();
}

void SAL_CALL ScTableSheetObj::setPrintAreas(
                    const uno::Sequence<table::CellRangeAddress>& aPrintAreas )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    std::unique_ptr<ScPrintRangeSaver> pOldRanges;
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    if ( rDoc.IsUndoEnabled() )
        pOldRanges = rDoc.CreatePrintRangeSaver();

    sal_uInt16 nCount = static_cast<sal_uInt16>(aPrintAreas.getLength());
    rDoc.ClearPrintRanges( nTab );
    if (nCount)
    {
        ScRange aPrintRange;
        for (const table::CellRangeAddress& rPrintArea : aPrintAreas)
        {
            ScUnoConversion::FillScRange( aPrintRange, rPrintArea );
            rDoc.AddPrintRange( nTab, aPrintRange );
        }
    }

    if ( rDoc.IsUndoEnabled() )
        PrintAreaUndo_Impl( std::move(pOldRanges) );   // Undo, Page Breaks, Modified etc.
}

sal_Bool SAL_CALL ScTableSheetObj::getPrintTitleColumns()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();
        return ( rDoc.GetRepeatColRange(nTab) != nullptr );
    }
    return false;
}

void SAL_CALL ScTableSheetObj::setPrintTitleColumns( sal_Bool bPrintTitleColumns )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    std::unique_ptr<ScPrintRangeSaver> pOldRanges = rDoc.CreatePrintRangeSaver();

    if ( bPrintTitleColumns )
    {
        if ( !rDoc.GetRepeatColRange( nTab ) )         // do not change existing area
        {
            rDoc.SetRepeatColRange( nTab, std::unique_ptr<ScRange>(new ScRange( 0, 0, nTab, 0, 0, nTab )) );     // enable
        }
    }
    else
        rDoc.SetRepeatColRange( nTab, nullptr );          // disable

    PrintAreaUndo_Impl( std::move(pOldRanges) );   // undo, page break, modified etc.

    //! save last set area during switch off and recreate during switch on ???
}

table::CellRangeAddress SAL_CALL ScTableSheetObj::getTitleColumns()
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();
        const ScRange* pRange = rDoc.GetRepeatColRange(nTab);
        if (pRange)
        {
            ScUnoConversion::FillApiRange( aRet, *pRange );
            aRet.Sheet = nTab; // core does not care about sheet index
        }
    }
    return aRet;
}

void SAL_CALL ScTableSheetObj::setTitleColumns( const table::CellRangeAddress& aTitleColumns )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    std::unique_ptr<ScPrintRangeSaver> pOldRanges = rDoc.CreatePrintRangeSaver();

    std::unique_ptr<ScRange> pNew(new ScRange);
    ScUnoConversion::FillScRange( *pNew, aTitleColumns );
    rDoc.SetRepeatColRange( nTab, std::move(pNew) );     // also always enable

    PrintAreaUndo_Impl( std::move(pOldRanges) );           // undo, page breaks, modified etc.
}

sal_Bool SAL_CALL ScTableSheetObj::getPrintTitleRows()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();
        return ( rDoc.GetRepeatRowRange(nTab) != nullptr );
    }
    return false;
}

void SAL_CALL ScTableSheetObj::setPrintTitleRows( sal_Bool bPrintTitleRows )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    std::unique_ptr<ScPrintRangeSaver> pOldRanges = rDoc.CreatePrintRangeSaver();

    if ( bPrintTitleRows )
    {
        if ( !rDoc.GetRepeatRowRange( nTab ) )         // do not change existing area
        {
            std::unique_ptr<ScRange> pNew( new ScRange(0, 0, nTab, 0, 0, nTab) );
            rDoc.SetRepeatRowRange( nTab, std::move(pNew) );     // enable
        }
    }
    else
        rDoc.SetRepeatRowRange( nTab, nullptr );          // disable

    PrintAreaUndo_Impl( std::move(pOldRanges) );   // undo, page breaks, modified etc.

    //! save last set area during switch off and recreate during switch on ???
}

table::CellRangeAddress SAL_CALL ScTableSheetObj::getTitleRows()
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();
        const ScRange* pRange = rDoc.GetRepeatRowRange(nTab);
        if (pRange)
        {
            ScUnoConversion::FillApiRange( aRet, *pRange );
            aRet.Sheet = nTab; // core does not care about sheet index
        }
    }
    return aRet;
}

void SAL_CALL ScTableSheetObj::setTitleRows( const table::CellRangeAddress& aTitleRows )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    std::unique_ptr<ScPrintRangeSaver> pOldRanges = rDoc.CreatePrintRangeSaver();

    std::unique_ptr<ScRange> pNew(new ScRange);
    ScUnoConversion::FillScRange( *pNew, aTitleRows );
    rDoc.SetRepeatRowRange( nTab, std::move(pNew) );     // also always enable

    PrintAreaUndo_Impl( std::move(pOldRanges) );           // Undo, page breaks, modified etc.
}

// XSheetLinkable

sheet::SheetLinkMode SAL_CALL ScTableSheetObj::getLinkMode()
{
    SolarMutexGuard aGuard;
    sheet::SheetLinkMode eRet = sheet::SheetLinkMode_NONE;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScLinkMode nMode = pDocSh->GetDocument().GetLinkMode( GetTab_Impl() );
        if ( nMode == ScLinkMode::NORMAL )
            eRet = sheet::SheetLinkMode_NORMAL;
        else if ( nMode == ScLinkMode::VALUE )
            eRet = sheet::SheetLinkMode_VALUE;
    }
    return eRet;
}

void SAL_CALL ScTableSheetObj::setLinkMode( sheet::SheetLinkMode nLinkMode )
{
    SolarMutexGuard aGuard;

    //! search for filter and options in old link

    OUString aUrl(getLinkUrl());
    OUString aSheet(getLinkSheetName());

    link( aUrl, aSheet, "", "", nLinkMode );
}

OUString SAL_CALL ScTableSheetObj::getLinkUrl()
{
    SolarMutexGuard aGuard;
    OUString aFile;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        aFile = pDocSh->GetDocument().GetLinkDoc( GetTab_Impl() );
    return aFile;
}

void SAL_CALL ScTableSheetObj::setLinkUrl( const OUString& aLinkUrl )
{
    SolarMutexGuard aGuard;

    //! search for filter and options in old link

    sheet::SheetLinkMode eMode = getLinkMode();
    OUString aSheet(getLinkSheetName());

    link( aLinkUrl, aSheet, "", "", eMode );
}

OUString SAL_CALL ScTableSheetObj::getLinkSheetName()
{
    SolarMutexGuard aGuard;
    OUString aSheet;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        aSheet = pDocSh->GetDocument().GetLinkTab( GetTab_Impl() );
    return aSheet;
}

void SAL_CALL ScTableSheetObj::setLinkSheetName( const OUString& aLinkSheetName )
{
    SolarMutexGuard aGuard;

    //! search for filter and options in old link

    sheet::SheetLinkMode eMode = getLinkMode();
    OUString aUrl(getLinkUrl());

    link( aUrl, aLinkSheetName, "", "", eMode );
}

void SAL_CALL ScTableSheetObj::link( const OUString& aUrl, const OUString& aSheetName,
                        const OUString& aFilterName, const OUString& aFilterOptions,
                        sheet::SheetLinkMode nMode )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    OUString aFileString = aUrl;
    OUString aFilterString = aFilterName;
    OUString aOptString = aFilterOptions;

    aFileString = ScGlobal::GetAbsDocName( aFileString, pDocSh );
    if (aFilterString.isEmpty())
        ScDocumentLoader::GetFilterName( aFileString, aFilterString, aOptString, true, false );

    //  remove application prefix from filter name here, so the filter options
    //  aren't reset when the filter name is changed in ScTableLink::DataChanged
    ScDocumentLoader::RemoveAppPrefix( aFilterString );

    ScLinkMode nLinkMode = ScLinkMode::NONE;
    if ( nMode == sheet::SheetLinkMode_NORMAL )
        nLinkMode = ScLinkMode::NORMAL;
    else if ( nMode == sheet::SheetLinkMode_VALUE )
        nLinkMode = ScLinkMode::VALUE;

    rDoc.SetLink( nTab, nLinkMode, aFileString, aFilterString, aOptString, aSheetName, 0/*nRefresh*/ );

    pDocSh->UpdateLinks();                  // if needed add or delete link
    SfxBindings* pBindings = pDocSh->GetViewBindings();
    if (pBindings)
        pBindings->Invalidate(SID_LINKS);

    //! undo of link data on the table

    if ( !(nLinkMode != ScLinkMode::NONE && rDoc.IsExecuteLinkEnabled()) )        // update link
        return;

    //  Always update link also if already exists
    //! update only on the affected table???

    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();
    sal_uInt16 nCount = pLinkManager->GetLinks().size();
    for ( sal_uInt16 i=0; i<nCount; i++ )
    {
        ::sfx2::SvBaseLink* pBase = pLinkManager->GetLinks()[i].get();
        if (auto pTabLink = dynamic_cast<ScTableLink*>( pBase))
        {
            if ( aFileString == pTabLink->GetFileName() )
                pTabLink->Update();                         // include Paint&Undo

            //! The file name should only exists once (?)
        }
    }

    //! notify ScSheetLinkObj objects!!!
}

// XSheetAuditing

sal_Bool SAL_CALL ScTableSheetObj::hideDependents( const table::CellAddress& aPosition )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCTAB nTab = GetTab_Impl();
        OSL_ENSURE( aPosition.Sheet == nTab, "wrong table in CellAddress" );
        ScAddress aPos( static_cast<SCCOL>(aPosition.Column), static_cast<SCROW>(aPosition.Row), nTab );
        return pDocSh->GetDocFunc().DetectiveDelSucc( aPos );
    }
    return false;
}

sal_Bool SAL_CALL ScTableSheetObj::hidePrecedents( const table::CellAddress& aPosition )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCTAB nTab = GetTab_Impl();
        OSL_ENSURE( aPosition.Sheet == nTab, "wrong table in CellAddress" );
        ScAddress aPos( static_cast<SCCOL>(aPosition.Column), static_cast<SCROW>(aPosition.Row), nTab );
        return pDocSh->GetDocFunc().DetectiveDelPred( aPos );
    }
    return false;
}

sal_Bool SAL_CALL ScTableSheetObj::showDependents( const table::CellAddress& aPosition )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCTAB nTab = GetTab_Impl();
        OSL_ENSURE( aPosition.Sheet == nTab, "wrong table in CellAddress" );
        ScAddress aPos( static_cast<SCCOL>(aPosition.Column), static_cast<SCROW>(aPosition.Row), nTab );
        return pDocSh->GetDocFunc().DetectiveAddSucc( aPos );
    }
    return false;
}

sal_Bool SAL_CALL ScTableSheetObj::showPrecedents( const table::CellAddress& aPosition )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCTAB nTab = GetTab_Impl();
        OSL_ENSURE( aPosition.Sheet == nTab, "wrong table in CellAddress" );
        ScAddress aPos( static_cast<SCCOL>(aPosition.Column), static_cast<SCROW>(aPosition.Row), nTab );
        return pDocSh->GetDocFunc().DetectiveAddPred( aPos );
    }
    return false;
}

sal_Bool SAL_CALL ScTableSheetObj::showErrors( const table::CellAddress& aPosition )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCTAB nTab = GetTab_Impl();
        OSL_ENSURE( aPosition.Sheet == nTab, "wrong table in CellAddress" );
        ScAddress aPos( static_cast<SCCOL>(aPosition.Column), static_cast<SCROW>(aPosition.Row), nTab );
        return pDocSh->GetDocFunc().DetectiveAddError( aPos );
    }
    return false;
}

sal_Bool SAL_CALL ScTableSheetObj::showInvalid()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocFunc().DetectiveMarkInvalid( GetTab_Impl() );
    return false;
}

void SAL_CALL ScTableSheetObj::clearArrows()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocFunc().DetectiveDelAll( GetTab_Impl() );
}

// XSheetOutline

void SAL_CALL ScTableSheetObj::group( const table::CellRangeAddress& rGroupRange,
                                        table::TableOrientation nOrientation )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        bool bColumns = ( nOrientation == table::TableOrientation_COLUMNS );
        ScRange aGroupRange;
        ScUnoConversion::FillScRange( aGroupRange, rGroupRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.MakeOutline( aGroupRange, bColumns, true, true );
    }
}

void SAL_CALL ScTableSheetObj::ungroup( const table::CellRangeAddress& rGroupRange,
                                        table::TableOrientation nOrientation )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        bool bColumns = ( nOrientation == table::TableOrientation_COLUMNS );
        ScRange aGroupRange;
        ScUnoConversion::FillScRange( aGroupRange, rGroupRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.RemoveOutline( aGroupRange, bColumns, true, true );
    }
}

void SAL_CALL ScTableSheetObj::autoOutline( const table::CellRangeAddress& rCellRange )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScRange aFormulaRange;
        ScUnoConversion::FillScRange( aFormulaRange, rCellRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.AutoOutline( aFormulaRange, true );
    }
}

void SAL_CALL ScTableSheetObj::clearOutline()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCTAB nTab = GetTab_Impl();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.RemoveAllOutlines( nTab, true );
    }
}

void SAL_CALL ScTableSheetObj::hideDetail( const table::CellRangeAddress& rCellRange )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScRange aMarkRange;
        ScUnoConversion::FillScRange( aMarkRange, rCellRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.HideMarkedOutlines( aMarkRange, true );
    }
}

void SAL_CALL ScTableSheetObj::showDetail( const table::CellRangeAddress& rCellRange )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScRange aMarkRange;
        ScUnoConversion::FillScRange( aMarkRange, rCellRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.ShowMarkedOutlines( aMarkRange, true );
    }
}

void SAL_CALL ScTableSheetObj::showLevel( sal_Int16 nLevel, table::TableOrientation nOrientation )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        bool bColumns = ( nOrientation == table::TableOrientation_COLUMNS );
        SCTAB nTab = GetTab_Impl();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.SelectLevel( nTab, bColumns, nLevel, true, true );
    }
}

// XProtectable

void SAL_CALL ScTableSheetObj::protect( const OUString& aPassword )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    // #i108245# if already protected, don't change anything
    if ( pDocSh && !pDocSh->GetDocument().IsTabProtected( GetTab_Impl() ) )
    {
        pDocSh->GetDocFunc().Protect( GetTab_Impl(), aPassword );
    }
}

void SAL_CALL ScTableSheetObj::unprotect( const OUString& aPassword )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        bool bDone = pDocSh->GetDocFunc().Unprotect( GetTab_Impl(), aPassword, true );
        if (!bDone)
            throw lang::IllegalArgumentException();
    }
}

sal_Bool SAL_CALL ScTableSheetObj::isProtected()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocument().IsTabProtected( GetTab_Impl() );

    OSL_FAIL("no DocShell");     //! Exception or so?
    return false;
}

// XScenario

sal_Bool SAL_CALL ScTableSheetObj::getIsScenario()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocument().IsScenario( GetTab_Impl() );

    return false;
}

OUString SAL_CALL ScTableSheetObj::getScenarioComment()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        OUString aComment;
        Color  aColor;
        ScScenarioFlags nFlags;
        pDocSh->GetDocument().GetScenarioData( GetTab_Impl(), aComment, aColor, nFlags );
        return aComment;
    }
    return OUString();
}

void SAL_CALL ScTableSheetObj::setScenarioComment( const OUString& aScenarioComment )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    OUString aName;
    OUString aComment;
    Color  aColor;
    ScScenarioFlags nFlags;
    rDoc.GetName( nTab, aName );
    rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );

    aComment = aScenarioComment;

    pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
}

void SAL_CALL ScTableSheetObj::addRanges( const uno::Sequence<table::CellRangeAddress>& rScenRanges )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    if (!rDoc.IsScenario(nTab))
        return;

    ScMarkData aMarkData(rDoc.GetSheetLimits());
    aMarkData.SelectTable( nTab, true );

    for (const table::CellRangeAddress& rRange : rScenRanges)
    {
        OSL_ENSURE( rRange.Sheet == nTab, "addRanges with wrong Tab" );
        ScRange aOneRange( static_cast<SCCOL>(rRange.StartColumn), static_cast<SCROW>(rRange.StartRow), nTab,
                           static_cast<SCCOL>(rRange.EndColumn),   static_cast<SCROW>(rRange.EndRow),   nTab );

        aMarkData.SetMultiMarkArea( aOneRange );
    }

    //  Scenario ranges are tagged with attribute
    ScPatternAttr aPattern( rDoc.GetPool() );
    aPattern.GetItemSet().Put( ScMergeFlagAttr( ScMF::Scenario ) );
    aPattern.GetItemSet().Put( ScProtectionAttr( true ) );
    pDocSh->GetDocFunc().ApplyAttributes( aMarkData, aPattern, true );
}

void SAL_CALL ScTableSheetObj::apply()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( !pDocSh )
        return;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();
    OUString aName;
    rDoc.GetName( nTab, aName );       // scenario name

    SCTAB nDestTab = nTab;
    while ( nDestTab > 0 && rDoc.IsScenario(nDestTab) )
        --nDestTab;

    if ( !rDoc.IsScenario(nDestTab) )
        pDocSh->UseScenario( nDestTab, aName );

    //! otherwise error or so
}

// XScenarioEnhanced

uno::Sequence< table::CellRangeAddress > SAL_CALL ScTableSheetObj::getRanges(  )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetTab_Impl();
        const ScRangeList* pRangeList = rDoc.GetScenarioRanges(nTab);
        if (pRangeList)
        {
            size_t nCount = pRangeList->size();
            uno::Sequence< table::CellRangeAddress > aRetRanges( nCount );
            table::CellRangeAddress* pAry = aRetRanges.getArray();
            for( size_t nIndex = 0; nIndex < nCount; nIndex++ )
            {
                const ScRange & rRange = (*pRangeList)[nIndex];
                pAry->StartColumn = rRange.aStart.Col();
                pAry->StartRow = rRange.aStart.Row();
                pAry->EndColumn = rRange.aEnd.Col();
                pAry->EndRow = rRange.aEnd.Row();
                pAry->Sheet = rRange.aStart.Tab();
                ++pAry;
            }
            return aRetRanges;
        }
    }
    return uno::Sequence< table::CellRangeAddress > ();
}

// XExternalSheetName

void ScTableSheetObj::setExternalName( const OUString& aUrl, const OUString& aSheetName )
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        const SCTAB nTab = GetTab_Impl();
        const OUString aAbsDocName( ScGlobal::GetAbsDocName( aUrl, pDocSh ) );
        const OUString aDocTabName( ScGlobal::GetDocTabName( aAbsDocName, aSheetName ) );
        if ( !rDoc.RenameTab( nTab, aDocTabName, true /*bExternalDocument*/ ) )
        {
            throw container::ElementExistException( OUString(), *this );
        }
    }
}

// XEventsSupplier

uno::Reference<container::XNameReplace> SAL_CALL ScTableSheetObj::getEvents()
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScSheetEventsObj( pDocSh, GetTab_Impl() );

    return nullptr;
}

// XPropertySet extended for Sheet-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableSheetObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( pSheetPropSet->getPropertyMap() ));
    return aRef;
}

void ScTableSheetObj::SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, const uno::Any& aValue )
{
    if ( !pEntry )
        return;

    if ( IsScItemWid( pEntry->nWID ) )
    {
        //  for Item WIDs, call ScCellRangesBase directly
        ScCellRangesBase::SetOnePropertyValue(pEntry, aValue);
        return;
    }

    //  own properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;                                                 //! Exception or so?
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();

    if ( pEntry->nWID == SC_WID_UNO_PAGESTL )
    {
        OUString aStrVal;
        aValue >>= aStrVal;
        OUString aNewStr(ScStyleNameConversion::ProgrammaticToDisplayName(
                                            aStrVal, SfxStyleFamily::Page ));

        //! Undo? (also if SID_STYLE_APPLY on View)

        if ( rDoc.GetPageStyle( nTab ) != aNewStr )
        {
            rDoc.SetPageStyle( nTab, aNewStr );
            if (!rDoc.IsImportingXML())
            {
                ScPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab ).UpdatePages();

                SfxBindings* pBindings = pDocSh->GetViewBindings();
                if (pBindings)
                {
                    pBindings->Invalidate( SID_STYLE_FAMILY4 );
                    pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                    pBindings->Invalidate( FID_RESET_PRINTZOOM );
                    pBindings->Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
                    pBindings->Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
                }
            }
            pDocSh->SetDocumentModified();
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLVIS )
    {
        bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        rFunc.SetTableVisible( nTab, bVis, true );
    }
    else if ( pEntry->nWID == SC_WID_UNO_ISACTIVE )
    {
        if (rDoc.IsScenario(nTab))
            rDoc.SetActiveScenario( nTab, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    }
    else if ( pEntry->nWID == SC_WID_UNO_BORDCOL )
    {
        if (rDoc.IsScenario(nTab))
        {
            sal_Int32 nNewColor = 0;
            if (aValue >>= nNewColor)
            {
                OUString aName;
                OUString aComment;
                Color  aColor;
                ScScenarioFlags nFlags;
                rDoc.GetName( nTab, aName );
                rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );

                aColor = Color(static_cast<sal_uInt32>(nNewColor));

                pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
            }
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_PROTECT )
    {
        if (rDoc.IsScenario(nTab))
        {
            OUString aName;
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nFlags;
            rDoc.GetName( nTab, aName );
            rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );
            bool bModify(false);

            if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            {
                if (!(nFlags & ScScenarioFlags::Protected))
                {
                    nFlags |= ScScenarioFlags::Protected;
                    bModify = true;
                }
            }
            else
            {
                if (nFlags & ScScenarioFlags::Protected)
                {
                    nFlags &= ~ScScenarioFlags::Protected;
                    bModify = true;
                }
            }

            if (bModify)
                pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_SHOWBORD )
    {
        if (rDoc.IsScenario(nTab))
        {
            OUString aName;
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nFlags;
            rDoc.GetName( nTab, aName );
            rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );
            bool bModify(false);

            if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            {
                if (!(nFlags & ScScenarioFlags::ShowFrame))
                {
                    nFlags |= ScScenarioFlags::ShowFrame;
                    bModify = true;
                }
            }
            else
            {
                if (nFlags & ScScenarioFlags::ShowFrame)
                {
                    nFlags &= ~ScScenarioFlags::ShowFrame;
                    bModify = true;
                }
            }

            if (bModify)
                pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_PRINTBORD )
    {
        if (rDoc.IsScenario(nTab))
        {
            OUString aName;
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nFlags;
            rDoc.GetName( nTab, aName );
            rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );
            bool bModify(false);

            if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            {
                if (!(nFlags & ScScenarioFlags::PrintFrame))
                {
                    nFlags |= ScScenarioFlags::PrintFrame;
                    bModify = true;
                }
            }
            else
            {
                if (nFlags & ScScenarioFlags::PrintFrame)
                {
                    nFlags &= ~ScScenarioFlags::PrintFrame;
                    bModify = true;
                }
            }

            if (bModify)
                pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_COPYBACK )
    {
        if (rDoc.IsScenario(nTab))
        {
            OUString aName;
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nFlags;
            rDoc.GetName( nTab, aName );
            rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );
            bool bModify(false);

            if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            {
                if (!(nFlags & ScScenarioFlags::TwoWay))
                {
                    nFlags |= ScScenarioFlags::TwoWay;
                    bModify = true;
                }
            }
            else
            {
                if (nFlags & ScScenarioFlags::TwoWay)
                {
                    nFlags &= ~ScScenarioFlags::TwoWay;
                    bModify = true;
                }
            }

            if (bModify)
                pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_COPYSTYL )
    {
        if (rDoc.IsScenario(nTab))
        {
            OUString aName;
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nFlags;
            rDoc.GetName( nTab, aName );
            rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );
            bool bModify(false);

            if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            {
                if (!(nFlags & ScScenarioFlags::Attrib))
                {
                    nFlags |= ScScenarioFlags::Attrib;
                    bModify = true;
                }
            }
            else
            {
                if (nFlags & ScScenarioFlags::Attrib)
                {
                    nFlags &= ~ScScenarioFlags::Attrib;
                    bModify = true;
                }
            }

            if (bModify)
                pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_COPYFORM )
    {
        if (rDoc.IsScenario(nTab))
        {
            OUString aName;
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nFlags;
            rDoc.GetName( nTab, aName );
            rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );
            bool bModify(false);

            if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            {
                if (nFlags & ScScenarioFlags::Value)
                {
                    nFlags &= ~ScScenarioFlags::Value;
                    bModify = true;
                }
            }
            else
            {
                if (!(nFlags & ScScenarioFlags::Value))
                {
                    nFlags |= ScScenarioFlags::Value;
                    bModify = true;
                }
            }

            if (bModify)
                pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_TABLAYOUT )
    {
        sal_Int16 nValue = 0;
        if (aValue >>= nValue)
        {
            if (nValue == css::text::WritingMode2::RL_TB)
                rFunc.SetLayoutRTL(nTab, true);
            else
                rFunc.SetLayoutRTL(nTab, false);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_AUTOPRINT )
    {
        bool bAutoPrint = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bAutoPrint)
            rDoc.SetPrintEntireSheet( nTab ); // clears all print ranges
        else
        {
            if (rDoc.IsPrintEntireSheet( nTab ))
                rDoc.ClearPrintRanges( nTab ); // if this flag is true, there are no PrintRanges, so Clear clears only the flag.
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_TABCOLOR )
    {
        Color aColor = COL_AUTO;
        if ( aValue >>= aColor )
        {
            if ( rDoc.GetTabBgColor( nTab ) != aColor )
                rFunc.SetTabBgColor( nTab, aColor, true, true );
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_CODENAME )
    {
        OUString aCodeName;
        if (aValue >>= aCodeName)
        {
            pDocSh->GetDocument().SetCodeName( GetTab_Impl(), aCodeName );
        }
    }
    else if (pEntry->nWID == SC_WID_UNO_CONDFORMAT)
    {
        uno::Reference<sheet::XConditionalFormats> xCondFormat;
        if (aValue >>= xCondFormat)
        {
            // how to set the format correctly
        }
    }
    else
        ScCellRangeObj::SetOnePropertyValue(pEntry, aValue);        // base class, no Item WID
}

void ScTableSheetObj::GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                            uno::Any& rAny )
{
    if ( !pEntry )
        return;

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        throw uno::RuntimeException();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetTab_Impl();

    if ( pEntry->nWID == SC_WID_UNO_NAMES )
    {
        rAny <<= uno::Reference<sheet::XNamedRanges>(new ScLocalNamedRangesObj(pDocSh, this));
    }
    else if ( pEntry->nWID == SC_WID_UNO_PAGESTL )
    {
        rAny <<= ScStyleNameConversion::DisplayToProgrammaticName(
                            rDoc.GetPageStyle( nTab ), SfxStyleFamily::Page );
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLVIS )
    {
        bool bVis = rDoc.IsVisible( nTab );
        rAny <<= bVis;
    }
    else if ( pEntry->nWID == SC_WID_UNO_LINKDISPBIT )
    {
        //  no target bitmaps for individual entries (would be all equal)
        // ScLinkTargetTypeObj::SetLinkTargetBitmap( aAny, SC_LINKTARGETTYPE_SHEET );
    }
    else if ( pEntry->nWID == SC_WID_UNO_LINKDISPNAME )
    {
        //  LinkDisplayName for hyperlink dialog
        rAny <<= getName();     // sheet name
    }
    else if ( pEntry->nWID == SC_WID_UNO_ISACTIVE )
    {
        if (rDoc.IsScenario(nTab))
            rAny <<= rDoc.IsActiveScenario( nTab );
    }
    else if ( pEntry->nWID == SC_WID_UNO_BORDCOL )
    {
        if (rDoc.IsScenario(nTab))
        {
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nFlags;
            rDoc.GetScenarioData( nTab, aComment, aColor, nFlags );

            rAny <<= aColor;
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_PROTECT )
    {
        if (rDoc.IsScenario(nTab))
        {
            ScScenarioFlags nFlags;
            rDoc.GetScenarioFlags(nTab, nFlags);

            rAny <<= ((nFlags & ScScenarioFlags::Protected) != ScScenarioFlags::NONE);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_SHOWBORD )
    {
        if (rDoc.IsScenario(nTab))
        {
            ScScenarioFlags nFlags;
            rDoc.GetScenarioFlags(nTab, nFlags);

            rAny <<= ((nFlags & ScScenarioFlags::ShowFrame) != ScScenarioFlags::NONE);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_PRINTBORD )
    {
        if (rDoc.IsScenario(nTab))
        {
            ScScenarioFlags nFlags;
            rDoc.GetScenarioFlags(nTab, nFlags);

            rAny <<= ((nFlags & ScScenarioFlags::PrintFrame) != ScScenarioFlags::NONE);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_COPYBACK )
    {
        if (rDoc.IsScenario(nTab))
        {
            ScScenarioFlags nFlags;
            rDoc.GetScenarioFlags(nTab, nFlags);

            rAny <<= ((nFlags & ScScenarioFlags::TwoWay) != ScScenarioFlags::NONE);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_COPYSTYL )
    {
        if (rDoc.IsScenario(nTab))
        {
            ScScenarioFlags nFlags;
            rDoc.GetScenarioFlags(nTab, nFlags);

            rAny <<= ((nFlags & ScScenarioFlags::Attrib) != ScScenarioFlags::NONE);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_COPYFORM )
    {
        if (rDoc.IsScenario(nTab))
        {
            ScScenarioFlags nFlags;
            rDoc.GetScenarioFlags(nTab, nFlags);

            rAny <<= !(nFlags & ScScenarioFlags::Value);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_TABLAYOUT )
    {
        if (rDoc.IsLayoutRTL(nTab))
            rAny <<= sal_Int16(css::text::WritingMode2::RL_TB);
        else
            rAny <<= sal_Int16(css::text::WritingMode2::LR_TB);
    }
    else if ( pEntry->nWID == SC_WID_UNO_AUTOPRINT )
    {
        bool bAutoPrint = rDoc.IsPrintEntireSheet( nTab );
        rAny <<= bAutoPrint;
    }
    else if ( pEntry->nWID == SC_WID_UNO_TABCOLOR )
    {
        rAny <<= rDoc.GetTabBgColor(nTab);
    }
    else if ( pEntry->nWID == SC_WID_UNO_CODENAME )
    {
        OUString aCodeName;
        pDocSh->GetDocument().GetCodeName(GetTab_Impl(), aCodeName);
        rAny <<= aCodeName;
    }
    else if (pEntry->nWID == SC_WID_UNO_CONDFORMAT)
    {
        rAny <<= uno::Reference<sheet::XConditionalFormats>(new ScCondFormatsObj(pDocSh, nTab));
    }
    else
        ScCellRangeObj::GetOnePropertyValue(pEntry, rAny);
}

const SfxItemPropertyMap& ScTableSheetObj::GetItemPropertyMap()
{
    return pSheetPropSet->getPropertyMap();
}

// XServiceInfo

OUString SAL_CALL ScTableSheetObj::getImplementationName()
{
    return "ScTableSheetObj";
}

sal_Bool SAL_CALL ScTableSheetObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScTableSheetObj::getSupportedServiceNames()
{
    return {SCSPREADSHEET_SERVICE,
            SCSHEETCELLRANGE_SERVICE,
            SCCELLRANGE_SERVICE,
            SCCELLPROPERTIES_SERVICE,
            SCCHARPROPERTIES_SERVICE,
            SCPARAPROPERTIES_SERVICE,
            SCLINKTARGET_SERVICE};
}

// XUnoTunnel

UNO3_GETIMPLEMENTATION2_IMPL(ScTableSheetObj, ScCellRangeObj);

ScTableColumnObj::ScTableColumnObj( ScDocShell* pDocSh, SCCOL nCol, SCTAB nTab ) :
    ScCellRangeObj( pDocSh, ScRange(nCol,0,nTab, nCol, pDocSh->GetDocument().MaxRow(),nTab) ),
    pColPropSet(lcl_GetColumnPropertySet())
{
}

ScTableColumnObj::~ScTableColumnObj()
{
}

uno::Any SAL_CALL ScTableColumnObj::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( container::XNamed )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScTableColumnObj::acquire() throw()
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScTableColumnObj::release() throw()
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScTableColumnObj::getTypes()
{
    return comphelper::concatSequences(
        ScCellRangeObj::getTypes(),
        uno::Sequence<uno::Type> { cppu::UnoType<container::XNamed>::get() } );
}

uno::Sequence<sal_Int8> SAL_CALL ScTableColumnObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XNamed

OUString SAL_CALL ScTableColumnObj::getName()
{
    SolarMutexGuard aGuard;

    const ScRange& rRange = GetRange();
    OSL_ENSURE(rRange.aStart.Col() == rRange.aEnd.Col(), "too many columns");
    SCCOL nCol = rRange.aStart.Col();

    return ScColToAlpha( nCol );        // from global.hxx
}

void SAL_CALL ScTableColumnObj::setName( const OUString& /* aNewName */ )
{
    SolarMutexGuard aGuard;
    throw uno::RuntimeException();      // read-only
}

// XPropertySet extended for Column-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableColumnObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( pColPropSet->getPropertyMap() ));
    return aRef;
}

void ScTableColumnObj::SetOnePropertyValue(const SfxItemPropertySimpleEntry* pEntry, const uno::Any& aValue)
{
    if ( !pEntry )
        return;

    if ( IsScItemWid( pEntry->nWID ) )
    {
        //  for Item WIDs, call ScCellRangesBase directly
        ScCellRangesBase::SetOnePropertyValue(pEntry, aValue);
        return;
    }

    //  own properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;                                                 //! Exception or so?
    const ScRange& rRange = GetRange();
    OSL_ENSURE(rRange.aStart.Col() == rRange.aEnd.Col(), "Too many columns");
    SCCOL nCol = rRange.aStart.Col();
    SCTAB nTab = rRange.aStart.Tab();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();

    std::vector<sc::ColRowSpan> aColArr(1, sc::ColRowSpan(nCol,nCol));

    if ( pEntry->nWID == SC_WID_UNO_CELLWID )
    {
        sal_Int32 nNewWidth = 0;
        if ( aValue >>= nNewWidth )
        {
            //  property is 1/100mm, column width is twips
            nNewWidth = HMMToTwips(nNewWidth);
            rFunc.SetWidthOrHeight(
                true, aColArr, nTab, SC_SIZE_ORIGINAL, static_cast<sal_uInt16>(nNewWidth), true, true);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLVIS )
    {
        bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight(true, aColArr, nTab, eMode, 0, true, true);
        //  SC_SIZE_DIRECT with size 0 will hide
    }
    else if ( pEntry->nWID == SC_WID_UNO_OWIDTH )
    {
        bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bOpt)
            rFunc.SetWidthOrHeight(
                true, aColArr, nTab, SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH, true, true);
        // sal_False on columns currently without effect
    }
    else if ( pEntry->nWID == SC_WID_UNO_NEWPAGE || pEntry->nWID == SC_WID_UNO_MANPAGE )
    {
        bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bSet)
            rFunc.InsertPageBreak( true, rRange.aStart, true, true );
        else
            rFunc.RemovePageBreak( true, rRange.aStart, true, true );
    }
    else
        ScCellRangeObj::SetOnePropertyValue(pEntry, aValue);        // base class, no Item WID
}

void ScTableColumnObj::GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, uno::Any& rAny )
{
    if ( !pEntry )
        return;

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocSh->GetDocument();
    const ScRange& rRange = GetRange();
    OSL_ENSURE(rRange.aStart.Col() == rRange.aEnd.Col(), "too many columns");
    SCCOL nCol = rRange.aStart.Col();
    SCTAB nTab = rRange.aStart.Tab();

    if ( pEntry->nWID == SC_WID_UNO_CELLWID )
    {
        // for hidden column, return original height
        sal_uInt16 nWidth = rDoc.GetOriginalWidth( nCol, nTab );
        //  property is 1/100mm, column width is twips
        nWidth = static_cast<sal_uInt16>(TwipsToHMM(nWidth));
        rAny <<= static_cast<sal_Int32>(nWidth);
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLVIS )
    {
        bool bHidden = rDoc.ColHidden(nCol, nTab);
        rAny <<= !bHidden;
    }
    else if ( pEntry->nWID == SC_WID_UNO_OWIDTH )
    {
        //! at the moment always set ??!?!
        bool bOpt = !(rDoc.GetColFlags( nCol, nTab ) & CRFlags::ManualSize);
        rAny <<= bOpt;
    }
    else if ( pEntry->nWID == SC_WID_UNO_NEWPAGE )
    {
        ScBreakType nBreak = rDoc.HasColBreak(nCol, nTab);
        rAny <<= nBreak != ScBreakType::NONE;
    }
    else if ( pEntry->nWID == SC_WID_UNO_MANPAGE )
    {
        ScBreakType nBreak = rDoc.HasColBreak(nCol, nTab);
        rAny <<= bool(nBreak & ScBreakType::Manual);
    }
    else
        ScCellRangeObj::GetOnePropertyValue(pEntry, rAny);
}

const SfxItemPropertyMap& ScTableColumnObj::GetItemPropertyMap()
{
    return pColPropSet->getPropertyMap();
}

ScTableRowObj::ScTableRowObj(ScDocShell* pDocSh, SCROW nRow, SCTAB nTab) :
    ScCellRangeObj( pDocSh, ScRange(0,nRow,nTab, pDocSh->GetDocument().MaxCol(),nRow,nTab) ),
    pRowPropSet(lcl_GetRowPropertySet())
{
}

ScTableRowObj::~ScTableRowObj()
{
}

// XPropertySet extended for Row-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableRowObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( pRowPropSet->getPropertyMap() ));
    return aRef;
}

void ScTableRowObj::SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, const uno::Any& aValue )
{
    if ( !pEntry )
        return;

    if ( IsScItemWid( pEntry->nWID ) )
    {
        //  for Item WIDs, call ScCellRangesBase directly
        ScCellRangesBase::SetOnePropertyValue(pEntry, aValue);
        return;
    }

    //  own properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;                                                 //! Exception or so?
    ScDocument& rDoc = pDocSh->GetDocument();
    const ScRange& rRange = GetRange();
    OSL_ENSURE(rRange.aStart.Row() == rRange.aEnd.Row(), "too many rows");
    SCROW nRow = rRange.aStart.Row();
    SCTAB nTab = rRange.aStart.Tab();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();

    std::vector<sc::ColRowSpan> aRowArr(1, sc::ColRowSpan(nRow,nRow));

    if ( pEntry->nWID == SC_WID_UNO_CELLHGT )
    {
        sal_Int32 nNewHeight = 0;
        if ( aValue >>= nNewHeight )
        {
            //  property is 1/100mm, row height is twips
            nNewHeight = HMMToTwips(nNewHeight);
            rFunc.SetWidthOrHeight(
                false, aRowArr, nTab, SC_SIZE_ORIGINAL, static_cast<sal_uInt16>(nNewHeight), true, true);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLVIS )
    {
        bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight(false, aRowArr, nTab, eMode, 0, true, true);
        //  SC_SIZE_DIRECT with size zero will hide
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLFILT )
    {
        bool bFil = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        //  SC_SIZE_DIRECT with size zero will hide
        rDoc.SetRowFiltered(nRow, nRow, nTab, bFil);
    }
    else if ( pEntry->nWID == SC_WID_UNO_OHEIGHT )
    {
        bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bOpt)
            rFunc.SetWidthOrHeight(false, aRowArr, nTab, SC_SIZE_OPTIMAL, 0, true, true);
        else
        {
            //  set current height again manually
            sal_uInt16 nHeight = rDoc.GetOriginalHeight( nRow, nTab );
            rFunc.SetWidthOrHeight(false, aRowArr, nTab, SC_SIZE_ORIGINAL, nHeight, true, true);
        }
    }
    else if ( pEntry->nWID == SC_WID_UNO_NEWPAGE || pEntry->nWID == SC_WID_UNO_MANPAGE )
    {
        bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bSet)
            rFunc.InsertPageBreak( false, rRange.aStart, true, true );
        else
            rFunc.RemovePageBreak( false, rRange.aStart, true, true );
    }
    else
        ScCellRangeObj::SetOnePropertyValue(pEntry, aValue);        // base class, no Item WID
}

void ScTableRowObj::GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry, uno::Any& rAny )
{
    if ( !pEntry )
        return;

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        throw uno::RuntimeException();
    ScDocument& rDoc = pDocSh->GetDocument();
    const ScRange& rRange = GetRange();
    OSL_ENSURE(rRange.aStart.Row() == rRange.aEnd.Row(), "too many rows");
    SCROW nRow = rRange.aStart.Row();
    SCTAB nTab = rRange.aStart.Tab();

    if ( pEntry->nWID == SC_WID_UNO_CELLHGT )
    {
        // for hidden row, return original height
        sal_uInt16 nHeight = rDoc.GetOriginalHeight( nRow, nTab );
        //  property is 1/100mm, row height is twips
        nHeight = static_cast<sal_uInt16>(TwipsToHMM(nHeight));
        rAny <<= static_cast<sal_Int32>(nHeight);
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLVIS )
    {
        bool bHidden = rDoc.RowHidden(nRow, nTab);
        rAny <<= !bHidden;
    }
    else if ( pEntry->nWID == SC_WID_UNO_CELLFILT )
    {
        bool bVis = rDoc.RowFiltered(nRow, nTab);
        rAny <<= bVis;
    }
    else if ( pEntry->nWID == SC_WID_UNO_OHEIGHT )
    {
        bool bOpt = !(rDoc.GetRowFlags( nRow, nTab ) & CRFlags::ManualSize);
        rAny <<= bOpt;
    }
    else if ( pEntry->nWID == SC_WID_UNO_NEWPAGE )
    {
        ScBreakType nBreak = rDoc.HasRowBreak(nRow, nTab);
        rAny <<= (nBreak != ScBreakType::NONE);
    }
    else if ( pEntry->nWID == SC_WID_UNO_MANPAGE )
    {
        bool bBreak(rDoc.HasRowBreak(nRow, nTab) & ScBreakType::Manual);
        rAny <<= bBreak;
    }
    else
        ScCellRangeObj::GetOnePropertyValue(pEntry, rAny);
}

const SfxItemPropertyMap& ScTableRowObj::GetItemPropertyMap()
{
    return pRowPropSet->getPropertyMap();
}

ScCellsObj::ScCellsObj(ScDocShell* pDocSh, const ScRangeList& rR) :
    pDocShell( pDocSh ),
    aRanges( rR )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScCellsObj::~ScCellsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScCellsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( auto pRefHint = dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        aRanges.UpdateReference( pRefHint->GetMode(), &pDocShell->GetDocument(), pRefHint->GetRange(),
                                        pRefHint->GetDx(), pRefHint->GetDy(), pRefHint->GetDz() );
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScCellsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScCellsEnumeration( pDocShell, aRanges );
    return nullptr;
}

uno::Type SAL_CALL ScCellsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XCell>::get();
}

sal_Bool SAL_CALL ScCellsObj::hasElements()
{
    SolarMutexGuard aGuard;
    bool bHas = false;
    if ( pDocShell )
    {
        //! faster if test ourself?

        uno::Reference<container::XEnumeration> xEnum(new ScCellsEnumeration( pDocShell, aRanges ));
        bHas = xEnum->hasMoreElements();
    }
    return bHas;
}

ScCellsEnumeration::ScCellsEnumeration(ScDocShell* pDocSh, const ScRangeList& rR) :
    pDocShell( pDocSh ),
    aRanges( rR ),
    bAtEnd( false )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.AddUnoObject(*this);

    if ( aRanges.empty() )
        bAtEnd = true;
    else
    {
        SCTAB nTab = aRanges[ 0 ].aStart.Tab();
        aPos = ScAddress(0,0,nTab);
        CheckPos_Impl();                    // set aPos on first matching cell
    }
}

void ScCellsEnumeration::CheckPos_Impl()
{
    if (!pDocShell)
        return;

    bool bFound = false;
    ScDocument& rDoc = pDocShell->GetDocument();
    ScRefCellValue aCell(rDoc, aPos);
    if (!aCell.isEmpty())
    {
        if (!pMark)
        {
            pMark.reset( new ScMarkData(rDoc.GetSheetLimits()) );
            pMark->MarkFromRangeList(aRanges, false);
            pMark->MarkToMulti();   // needed for GetNextMarkedCell
        }
        bFound = pMark->IsCellMarked(aPos.Col(), aPos.Row());
    }
    if (!bFound)
        Advance_Impl();
}

ScCellsEnumeration::~ScCellsEnumeration()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
    pMark.reset();
}

void ScCellsEnumeration::Advance_Impl()
{
    OSL_ENSURE(!bAtEnd,"too much Advance_Impl");
    if (!pMark)
    {
        pMark.reset( new ScMarkData(pDocShell->GetDocument().GetSheetLimits()) );
        pMark->MarkFromRangeList( aRanges, false );
        pMark->MarkToMulti();   // needed for GetNextMarkedCell
    }

    SCCOL nCol = aPos.Col();
    SCROW nRow = aPos.Row();
    SCTAB nTab = aPos.Tab();
    bool bFound = pDocShell->GetDocument().GetNextMarkedCell( nCol, nRow, nTab, *pMark );
    if (bFound)
        aPos.Set( nCol, nRow, nTab );
    else
        bAtEnd = true;      // nothing will follow
}

void ScCellsEnumeration::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const ScUpdateRefHint* pRefHint = dynamic_cast<const ScUpdateRefHint*>(&rHint);
    if ( pRefHint )
    {
        if (pDocShell)
        {
            aRanges.UpdateReference( pRefHint->GetMode(), &pDocShell->GetDocument(), pRefHint->GetRange(),
                                     pRefHint->GetDx(), pRefHint->GetDy(), pRefHint->GetDz() );

            pMark.reset();       // recreate from moved area

            if (!bAtEnd)        // adjust aPos
            {
                ScRangeList aNew { ScRange(aPos) };
                aNew.UpdateReference( pRefHint->GetMode(), &pDocShell->GetDocument(), pRefHint->GetRange(),
                                      pRefHint->GetDx(), pRefHint->GetDy(), pRefHint->GetDz() );
                if (aNew.size()==1)
                {
                    aPos = aNew[ 0 ].aStart;
                    CheckPos_Impl();
                }
            }
        }
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;
    }
}

// XEnumeration

sal_Bool SAL_CALL ScCellsEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;
    return !bAtEnd;
}

uno::Any SAL_CALL ScCellsEnumeration::nextElement()
{
    SolarMutexGuard aGuard;
    if (pDocShell && !bAtEnd)
    {
        // interface must match ScCellsObj::getElementType

        ScAddress aTempPos(aPos);
        Advance_Impl();
        return uno::makeAny(uno::Reference<table::XCell>(new ScCellObj( pDocShell, aTempPos )));
    }

    throw container::NoSuchElementException();      // no more elements
}

ScCellFormatsObj::ScCellFormatsObj(ScDocShell* pDocSh, const ScRange& rRange) :
    pDocShell( pDocSh ),
    aTotalRange( rRange )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.AddUnoObject(*this);

    OSL_ENSURE( aTotalRange.aStart.Tab() == aTotalRange.aEnd.Tab(), "different tables" );
}

ScCellFormatsObj::~ScCellFormatsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScCellFormatsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! aTotalRange...
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;
    }
}

ScCellRangeObj* ScCellFormatsObj::GetObjectByIndex_Impl(tools::Long nIndex) const
{
    //! access the AttrArrays directly !!!!

    ScCellRangeObj* pRet = nullptr;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        tools::Long nPos = 0;
        ScAttrRectIterator aIter( rDoc, aTotalRange.aStart.Tab(),
                                    aTotalRange.aStart.Col(), aTotalRange.aStart.Row(),
                                    aTotalRange.aEnd.Col(), aTotalRange.aEnd.Row() );
        SCCOL nCol1, nCol2;
        SCROW nRow1, nRow2;
        while ( aIter.GetNext( nCol1, nCol2, nRow1, nRow2 ) )
        {
            if ( nPos == nIndex )
            {
                SCTAB nTab = aTotalRange.aStart.Tab();
                ScRange aNext( nCol1, nRow1, nTab, nCol2, nRow2, nTab );

                if ( aNext.aStart == aNext.aEnd )
                    pRet = new ScCellObj( pDocShell, aNext.aStart );
                else
                    pRet = new ScCellRangeObj( pDocShell, aNext );
            }
            ++nPos;
        }
    }
    return pRet;
}

// XIndexAccess

sal_Int32 SAL_CALL ScCellFormatsObj::getCount()
{
    SolarMutexGuard aGuard;

    //! access the AttrArrays directly !!!!

    tools::Long nCount = 0;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScAttrRectIterator aIter( rDoc, aTotalRange.aStart.Tab(),
                                    aTotalRange.aStart.Col(), aTotalRange.aStart.Row(),
                                    aTotalRange.aEnd.Col(), aTotalRange.aEnd.Row() );
        SCCOL nCol1, nCol2;
        SCROW nRow1, nRow2;
        while ( aIter.GetNext( nCol1, nCol2, nRow1, nRow2 ) )
            ++nCount;
    }
    return nCount;
}

uno::Any SAL_CALL ScCellFormatsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;

    uno::Reference<table::XCellRange> xRange(GetObjectByIndex_Impl(nIndex));
    if (!xRange.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xRange);

}

uno::Type SAL_CALL ScCellFormatsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XCellRange>::get();
}

sal_Bool SAL_CALL ScCellFormatsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );     //! always greater then zero ??
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScCellFormatsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScCellFormatsEnumeration( pDocShell, aTotalRange );
    return nullptr;
}

ScCellFormatsEnumeration::ScCellFormatsEnumeration(ScDocShell* pDocSh, const ScRange& rRange) :
    pDocShell( pDocSh ),
    nTab( rRange.aStart.Tab() ),
    bAtEnd( false ),
    bDirty( false )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.AddUnoObject(*this);

    OSL_ENSURE( rRange.aStart.Tab() == rRange.aEnd.Tab(),
                "CellFormatsEnumeration: different tables" );

    pIter.reset( new ScAttrRectIterator( rDoc, nTab,
                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row() ) );
    Advance_Impl();
}

ScCellFormatsEnumeration::~ScCellFormatsEnumeration()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScCellFormatsEnumeration::Advance_Impl()
{
    OSL_ENSURE(!bAtEnd,"too many Advance_Impl");

    if ( pIter )
    {
        if ( bDirty )
        {
            pIter->DataChanged();   // new search for AttrArray-Index
            bDirty = false;
        }

        SCCOL nCol1, nCol2;
        SCROW nRow1, nRow2;
        if ( pIter->GetNext( nCol1, nCol2, nRow1, nRow2 ) )
            aNext = ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
        else
            bAtEnd = true;
    }
    else
        bAtEnd = true;          // document vanished or so
}

ScCellRangeObj* ScCellFormatsEnumeration::NextObject_Impl()
{
    ScCellRangeObj* pRet = nullptr;
    if (pDocShell && !bAtEnd)
    {
        if ( aNext.aStart == aNext.aEnd )
            pRet = new ScCellObj( pDocShell, aNext.aStart );
        else
            pRet = new ScCellRangeObj( pDocShell, aNext );
        Advance_Impl();
    }
    return pRet;
}

void ScCellFormatsEnumeration::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! and now???
    }
    else
    {
        const SfxHintId nId = rHint.GetId();
        if ( nId == SfxHintId::Dying )
        {
            pDocShell = nullptr;
            pIter.reset();
        }
        else if ( nId == SfxHintId::DataChanged )
        {
            bDirty = true;          // AttrArray-Index possibly invalid
        }
    }
}

// XEnumeration

sal_Bool SAL_CALL ScCellFormatsEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;
    return !bAtEnd;
}

uno::Any SAL_CALL ScCellFormatsEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if ( bAtEnd || !pDocShell )
        throw container::NoSuchElementException();      // no more elements

    // interface must match ScCellFormatsObj::getElementType

    return uno::makeAny(uno::Reference<table::XCellRange> (NextObject_Impl()));
}

ScUniqueCellFormatsObj::~ScUniqueCellFormatsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScUniqueCellFormatsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! aTotalRange...
    }
    else
    {
        if ( rHint.GetId() == SfxHintId::Dying )
            pDocShell = nullptr;
    }
}

//  Fill the list of formats from the document

namespace {

// hash code to access the range lists by ScPatternAttr pointer
struct ScPatternHashCode
{
    size_t operator()( const ScPatternAttr* pPattern ) const
    {
        return reinterpret_cast<size_t>(pPattern);
    }
};

}

// Hash map to find a range by its start row
typedef std::unordered_map< SCROW, ScRange > ScRowRangeHashMap;

namespace {

// Hash map entry.
// The Join method depends on the column-wise order of ScAttrRectIterator
class ScUniqueFormatsEntry
{
    enum EntryState { STATE_EMPTY, STATE_SINGLE, STATE_COMPLEX };

    EntryState          eState;
    ScRange             aSingleRange;
    ScRowRangeHashMap   aJoinedRanges;      // "active" ranges to be merged
    std::vector<ScRange> aCompletedRanges;   // ranges that will no longer be touched
    ScRangeListRef      aReturnRanges;      // result as ScRangeList for further use

public:
                        ScUniqueFormatsEntry() : eState( STATE_EMPTY ) {}

    void                Join( const ScRange& rNewRange );
    const ScRangeList&  GetRanges();
    void                Clear() { aReturnRanges.clear(); }  // aJoinedRanges and aCompletedRanges are cleared in GetRanges
};

}

void ScUniqueFormatsEntry::Join( const ScRange& rNewRange )
{
    // Special-case handling for single range

    if ( eState == STATE_EMPTY )
    {
        aSingleRange = rNewRange;
        eState = STATE_SINGLE;
        return;
    }
    if ( eState == STATE_SINGLE )
    {
        if ( aSingleRange.aStart.Row() == rNewRange.aStart.Row() &&
             aSingleRange.aEnd.Row() == rNewRange.aEnd.Row() &&
             aSingleRange.aEnd.Col() + 1 == rNewRange.aStart.Col() )
        {
            aSingleRange.aEnd.SetCol( rNewRange.aEnd.Col() );
            return;     // still a single range
        }

        SCROW nSingleRow = aSingleRange.aStart.Row();
        aJoinedRanges.emplace( nSingleRow, aSingleRange );
        eState = STATE_COMPLEX;
        // continue normally
    }

    // This is called in the order of ScAttrRectIterator results.
    // rNewRange can only be joined with an existing entry if it's the same rows, starting in the next column.
    // If the old entry for the start row extends to a different end row, or ends in a different column, it
    // can be moved to aCompletedRanges because it can't be joined with following iterator results.
    // Everything happens within one sheet, so Tab can be ignored.

    SCROW nStartRow = rNewRange.aStart.Row();
    ScRowRangeHashMap::iterator aIter( aJoinedRanges.find( nStartRow ) );       // find the active entry for the start row
    if ( aIter != aJoinedRanges.end() )
    {
        ScRange& rOldRange = aIter->second;
        if ( rOldRange.aEnd.Row() == rNewRange.aEnd.Row() &&
             rOldRange.aEnd.Col() + 1 == rNewRange.aStart.Col() )
        {
            // extend existing range
            rOldRange.aEnd.SetCol( rNewRange.aEnd.Col() );
        }
        else
        {
            // move old range to aCompletedRanges, keep rNewRange for joining
            aCompletedRanges.push_back( rOldRange );
            rOldRange = rNewRange;  // replace in hash map
        }
    }
    else
    {
        // keep rNewRange for joining
        aJoinedRanges.emplace( nStartRow, rNewRange );
    }
}

const ScRangeList& ScUniqueFormatsEntry::GetRanges()
{
    if ( eState == STATE_SINGLE )
    {
        aReturnRanges = new ScRangeList( aSingleRange );
        return *aReturnRanges;
    }

    // move remaining entries from aJoinedRanges to aCompletedRanges

    for ( const auto& rEntry : aJoinedRanges )
        aCompletedRanges.push_back( rEntry.second );
    aJoinedRanges.clear();

    // sort all ranges for a predictable API result

    std::sort( aCompletedRanges.begin(), aCompletedRanges.end() );

    // fill and return ScRangeList

    aReturnRanges = new ScRangeList;
    for ( const auto& rCompletedRange : aCompletedRanges )
        aReturnRanges->push_back( rCompletedRange );
    aCompletedRanges.clear();

    return *aReturnRanges;
}

typedef std::unordered_map< const ScPatternAttr*, ScUniqueFormatsEntry, ScPatternHashCode > ScUniqueFormatsHashMap;

namespace {

// function object to sort the range lists by start of first range
struct ScUniqueFormatsOrder
{
    bool operator()( const ScRangeList& rList1, const ScRangeList& rList2 ) const
    {
        // all range lists have at least one entry
        OSL_ENSURE( !rList1.empty() && !rList2.empty(), "ScUniqueFormatsOrder: empty list" );

        // compare start positions using ScAddress comparison operator
        return ( rList1[ 0 ].aStart < rList2[ 0 ].aStart );
    }
};

}

ScUniqueCellFormatsObj::ScUniqueCellFormatsObj(ScDocShell* pDocSh, const ScRange& rTotalRange) :
    pDocShell( pDocSh ),
    aRangeLists()
{
    pDocShell->GetDocument().AddUnoObject(*this);

    OSL_ENSURE( rTotalRange.aStart.Tab() == rTotalRange.aEnd.Tab(), "different tables" );

    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTab = rTotalRange.aStart.Tab();
    ScAttrRectIterator aIter( rDoc, nTab,
                                rTotalRange.aStart.Col(), rTotalRange.aStart.Row(),
                                rTotalRange.aEnd.Col(), rTotalRange.aEnd.Row() );
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;

    // Collect the ranges for each format in a hash map, to avoid nested loops

    ScUniqueFormatsHashMap aHashMap;
    while (aIter.GetNext( nCol1, nCol2, nRow1, nRow2 ) )
    {
        ScRange aRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
        const ScPatternAttr* pPattern = rDoc.GetPattern(nCol1, nRow1, nTab);
        aHashMap[pPattern].Join( aRange );
    }

    // Fill the vector aRangeLists with the range lists from the hash map

    aRangeLists.reserve( aHashMap.size() );
    for ( auto& rMapEntry : aHashMap )
    {
        ScUniqueFormatsEntry& rEntry = rMapEntry.second;
        const ScRangeList& rRanges = rEntry.GetRanges();
        aRangeLists.push_back( rRanges );       // copy ScRangeList
        rEntry.Clear();                         // free memory, don't hold both copies of all ranges
    }

    // Sort the vector by first range's start position, to avoid random shuffling
    // due to using the ScPatterAttr pointers

    ::std::sort( aRangeLists.begin(), aRangeLists.end(), ScUniqueFormatsOrder() );
}


// XIndexAccess

sal_Int32 SAL_CALL ScUniqueCellFormatsObj::getCount()
{
    SolarMutexGuard aGuard;

    return aRangeLists.size();
}

uno::Any SAL_CALL ScUniqueCellFormatsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;

    if(o3tl::make_unsigned(nIndex) >= aRangeLists.size())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(uno::Reference<sheet::XSheetCellRangeContainer>(new ScCellRangesObj(pDocShell, aRangeLists[nIndex])));

}

uno::Type SAL_CALL ScUniqueCellFormatsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XSheetCellRangeContainer>::get();
}

sal_Bool SAL_CALL ScUniqueCellFormatsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( !aRangeLists.empty() );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScUniqueCellFormatsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScUniqueCellFormatsEnumeration( pDocShell, aRangeLists );
    return nullptr;
}

ScUniqueCellFormatsEnumeration::ScUniqueCellFormatsEnumeration(ScDocShell* pDocSh, const std::vector<ScRangeList>& rRangeLists) :
    aRangeLists(rRangeLists),
    pDocShell( pDocSh ),
    nCurrentPosition(0)
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScUniqueCellFormatsEnumeration::~ScUniqueCellFormatsEnumeration()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScUniqueCellFormatsEnumeration::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! and now ???
    }
    else
    {
        if ( rHint.GetId() == SfxHintId::Dying )
            pDocShell = nullptr;
    }
}

// XEnumeration

sal_Bool SAL_CALL ScUniqueCellFormatsEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;
    return o3tl::make_unsigned(nCurrentPosition) < aRangeLists.size();
}

uno::Any SAL_CALL ScUniqueCellFormatsEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if ( !hasMoreElements() || !pDocShell )
        throw container::NoSuchElementException();      // no more elements

    // interface type must match ScCellFormatsObj::getElementType

    return uno::makeAny(uno::Reference<sheet::XSheetCellRangeContainer>(new ScCellRangesObj(pDocShell, aRangeLists[nCurrentPosition++])));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

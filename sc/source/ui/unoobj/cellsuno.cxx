/*************************************************************************
 *
 *  $RCSfile: cellsuno.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-09 10:59:10 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/editeng.hxx>
#include <svx/flditem.hxx>
#include <svx/fmdpage.hxx>
#include <svx/langitem.hxx>
#include <svx/linkmgr.hxx>
#include <svx/srchitem.hxx>
#include <svx/unomid.hxx>
#include <svx/unoprnms.hxx>
#include <svx/unotext.hxx>
#include <sfx2/bindings.hxx>
#include <sch/schdll.hxx>   // SchMemChart
#include <sch/memchrt.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <rtl/uuid.h>
#include <float.h>              // DBL_MIN

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include "autoform.hxx"
#include "cellsuno.hxx"
#include "cursuno.hxx"
#include "textuno.hxx"
#include "editsrc.hxx"
#include "notesuno.hxx"
#include "fielduno.hxx"
#include "docuno.hxx"       // ScTableColumnsObj etc
#include "datauno.hxx"
#include "dapiuno.hxx"
#include "chartuno.hxx"
#include "fmtuno.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "srchuno.hxx"
#include "targuno.hxx"
#include "docsh.hxx"
#include "markdata.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "docfunc.hxx"
#include "dbdocfun.hxx"
#include "olinefun.hxx"
#include "hints.hxx"
#include "cell.hxx"
#include "undocell.hxx"
#include "undotab.hxx"
#include "undoblk.hxx"      // fuer lcl_ApplyBorder - nach docfunc verschieben!
#include "stlsheet.hxx"
#include "dbcolect.hxx"
#include "attrib.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "drwlayer.hxx"
#include "printfun.hxx"
#include "prnsave.hxx"
#include "tablink.hxx"
#include "dociter.hxx"
#include "rangeutl.hxx"
#include "conditio.hxx"
#include "validat.hxx"
#include "sc.hrc"
#include "brdcst.hxx"
#include "unoguard.hxx"
#include "cellform.hxx"
#include "globstr.hrc"
#include "unonames.hxx"
#include "styleuno.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//! ScLinkListener in anderes File verschieben !!!

class ScLinkListener : public SfxListener
{
    Link    aLink;
public:
                    ScLinkListener(const Link& rL) : aLink(rL) {}
    virtual         ~ScLinkListener();
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


class ScNamedEntry
{
    String  aName;
    ScRange aRange;

public:
            ScNamedEntry(const String& rN, const ScRange& rR) :
                aName(rN), aRange(rR) {}

    const String&   GetName() const     { return aName; }
    const ScRange&  GetRange() const    { return aRange; }
};


//------------------------------------------------------------------------

//  Die Namen in den Maps muessen (nach strcmp) sortiert sein!
//! statt Which-ID 0 special IDs verwenden, und nicht ueber Namen vergleichen !!!!!!!!!

//  Left/Right/Top/BottomBorder are mapped directly to the core items,
//  not collected/applied to the borders of a range -> ATTR_BORDER can be used directly

const SfxItemPropertyMap* lcl_GetCellsPropertyMap()
{
    static SfxItemPropertyMap aCellsPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    &getCppuType((sal_Int32*)0),            0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),  ATTR_PROTECTION,    &getCppuType((util::CellProtection*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL), 0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),                 0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,          &getCppuType((sal_Int16*)0),            0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &getCppuType((rtl::OUString*)0),        0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   &getCppuType((float*)0),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, &getCppuType((lang::Locale*)0),         0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  &getCppuType((awt::FontSlant*)0),       0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),            0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   &getCppuType((float*)0),                0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   &getCppuType((table::CellHoriJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    &getBooleanCppuType(),                  0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),   ATTR_VALUE_FORMAT,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_ORIENTATION,   &getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),  ATTR_INDENT,        &getCppuType((sal_Int16*)0),            0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,      &::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),   ATTR_SHADOW,        &getCppuType((table::ShadowFormat*)0),  0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),  0,                  &getCppuType((table::TableBorder*)0),   0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,        &::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {0,0,0,0}
    };
    return aCellsPropertyMap_Impl;
}

//  CellRange enthaelt alle Eintraege von Cells, zusaetzlich eigene Eintraege
//  mit Which-ID 0 (werden nur fuer getPropertySetInfo benoetigt).

const SfxItemPropertyMap* lcl_GetRangePropertyMap()
{
    static SfxItemPropertyMap aRangePropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    &getCppuType((sal_Int32*)0),            0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),  ATTR_PROTECTION,    &getCppuType((util::CellProtection*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL), 0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),                 0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,          &getCppuType((sal_Int16*)0),            0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &getCppuType((rtl::OUString*)0),        0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   &getCppuType((float*)0),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, &getCppuType((lang::Locale*)0),         0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  &getCppuType((awt::FontSlant*)0),       0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),            0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   &getCppuType((float*)0),                0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   &getCppuType((table::CellHoriJustify*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    &getBooleanCppuType(),                  0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),   ATTR_VALUE_FORMAT,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_ORIENTATION,   &getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),  ATTR_INDENT,        &getCppuType((sal_Int16*)0),            0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),      0,                  &getCppuType((awt::Point*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,      &::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),   ATTR_SHADOW,        &getCppuType((table::ShadowFormat*)0),  0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),     0,                  &getCppuType((awt::Size*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),  0,                  &getCppuType((table::TableBorder*)0),   0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,        &::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {0,0,0,0}
    };
    return aRangePropertyMap_Impl;
}

//  Cell enthaelt alle Eintraege von CellRange, zusaetzlich eigene Eintraege
//  mit Which-ID 0 (werden nur fuer getPropertySetInfo benoetigt).

const SfxItemPropertyMap* lcl_GetCellPropertyMap()
{
    static SfxItemPropertyMap aCellPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    &getCppuType((sal_Int32*)0),            0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),  ATTR_PROTECTION,    &getCppuType((util::CellProtection*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL), 0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),                 0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,          &getCppuType((sal_Int16*)0),            0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &getCppuType((rtl::OUString*)0),        0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   &getCppuType((float*)0),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, &getCppuType((lang::Locale*)0),         0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  &getCppuType((awt::FontSlant*)0),       0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),            0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   &getCppuType((float*)0),                0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FORMLOC),  0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FORMRT),   0,                  &getCppuType((table::CellContentType*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   &getCppuType((table::CellHoriJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    &getBooleanCppuType(),                  0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),   ATTR_VALUE_FORMAT,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_ORIENTATION,   &getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),  ATTR_INDENT,        &getCppuType((sal_Int16*)0),            0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),      0,                  &getCppuType((awt::Point*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,      &::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),   ATTR_SHADOW,        &getCppuType((table::ShadowFormat*)0),  0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),     0,                  &getCppuType((awt::Size*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),  0,                  &getCppuType((table::TableBorder*)0),   0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,        &::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {0,0,0,0}
    };
    return aCellPropertyMap_Impl;
}

//  Column und Row enthalten alle Eintraege von CellRange, zusaetzlich eigene Eintraege
//  mit Which-ID 0 (werden nur fuer getPropertySetInfo benoetigt).

const SfxItemPropertyMap* lcl_GetColumnPropertyMap()
{
    static SfxItemPropertyMap aColumnPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    &getCppuType((sal_Int32*)0),            0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),  ATTR_PROTECTION,    &getCppuType((util::CellProtection*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL), 0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),                 0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,          &getCppuType((sal_Int16*)0),            0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &getCppuType((rtl::OUString*)0),        0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   &getCppuType((float*)0),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, &getCppuType((lang::Locale*)0),         0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  &getCppuType((awt::FontSlant*)0),       0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),            0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   &getCppuType((float*)0),                0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   &getCppuType((table::CellHoriJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    &getBooleanCppuType(),                  0, MID_GRAPHIC_TRANSPARENT },
//      {MAP_CHAR_LEN(SC_UNONAME_CELLFILT), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_MANPAGE),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NEWPAGE),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVIS),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),   ATTR_VALUE_FORMAT,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_OWIDTH),   0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_ORIENTATION,   &getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),  ATTR_INDENT,        &getCppuType((sal_Int16*)0),            0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),      0,                  &getCppuType((awt::Point*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,      &::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),   ATTR_SHADOW,        &getCppuType((table::ShadowFormat*)0),  0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),     0,                  &getCppuType((awt::Size*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),  0,                  &getCppuType((table::TableBorder*)0),   0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,        &::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLWID),  0,                  &getCppuType((sal_Int32*)0),            0, 0 },
        {0,0,0,0}
    };
    return aColumnPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetRowPropertyMap()
{
    static SfxItemPropertyMap aRowPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    &getCppuType((sal_Int32*)0),            0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),  ATTR_PROTECTION,    &getCppuType((util::CellProtection*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL), 0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),                 0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,          &getCppuType((sal_Int16*)0),            0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &getCppuType((rtl::OUString*)0),        0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   &getCppuType((float*)0),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, &getCppuType((lang::Locale*)0),         0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  &getCppuType((awt::FontSlant*)0),       0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),            0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   &getCppuType((float*)0),                0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHGT),  0,                  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   &getCppuType((table::CellHoriJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    &getBooleanCppuType(),                  0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_CELLFILT), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_MANPAGE),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NEWPAGE),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVIS),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),   ATTR_VALUE_FORMAT,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_OHEIGHT),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_ORIENTATION,   &getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),  ATTR_INDENT,        &getCppuType((sal_Int16*)0),            0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),      0,                  &getCppuType((awt::Point*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,      &::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),   ATTR_SHADOW,        &getCppuType((table::ShadowFormat*)0),  0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),     0,                  &getCppuType((awt::Size*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),  0,                  &getCppuType((table::TableBorder*)0),   0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,        &::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {0,0,0,0}
    };
    return aRowPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetSheetPropertyMap()
{
    static SfxItemPropertyMap aSheetPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND,    &getCppuType((sal_Int32*)0),            0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),  ATTR_PROTECTION,    &getCppuType((util::CellProtection*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL), 0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),   ATTR_FONT_COLOR,    &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),    ATTR_FONT_CONTOUR,  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),   ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),                 0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),    ATTR_FONT,          &getCppuType((sal_Int16*)0),            0, MID_FONT_FAMILY },
        {MAP_CHAR_LEN(SC_UNONAME_CFNAME),   ATTR_FONT,          &getCppuType((rtl::OUString*)0),        0, MID_FONT_FAMILY_NAME },
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),  ATTR_FONT_HEIGHT,   &getCppuType((float*)0),                0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),   ATTR_FONT_LANGUAGE, &getCppuType((lang::Locale*)0),         0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),    ATTR_FONT_POSTURE,  &getCppuType((awt::FontSlant*)0),       0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),   ATTR_FONT_SHADOWED, &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),   ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),            0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),  ATTR_FONT_WEIGHT,   &getCppuType((float*)0),                0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR), 0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),  0,                  &getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS), ATTR_HOR_JUSTIFY,   &getCppuType((table::CellHoriJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND,    &getBooleanCppuType(),                  0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),     ATTR_LINEBREAK,     &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVIS),  0,                  &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,       &::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPBIT),  0,                  &getCppuType((uno::Reference<awt::XBitmap>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPNAME), 0,                  &getCppuType((rtl::OUString*)0),        0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),   ATTR_VALUE_FORMAT,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),  ATTR_ORIENTATION,   &getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PAGESTL),  0,                  &getCppuType((rtl::OUString*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),  ATTR_INDENT,        &getCppuType((sal_Int16*)0),            0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN), ATTR_MARGIN,        &getCppuType((sal_Int32*)0),            0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),      0,                  &getCppuType((awt::Point*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,      &::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),   ATTR_ROTATE_VALUE,  &getCppuType((sal_Int32*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),   ATTR_ROTATE_MODE,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),   ATTR_SHADOW,        &getCppuType((table::ShadowFormat*)0),  0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),     0,                  &getCppuType((awt::Size*)0),            0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),  0,                  &getCppuType((table::TableBorder*)0),   0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,        &::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),  0,                  &getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS), ATTR_VER_JUSTIFY,   &getCppuType((table::CellVertJustify*)0), 0, 0 },
        {0,0,0,0}
    };
    return aSheetPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetEditPropertyMap()
{
    static SfxItemPropertyMap aEditPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {0,0,0,0}
    };
    return aEditPropertyMap_Impl;
}


//------------------------------------------------------------------------

//! diese Funktionen in einen allgemeinen Header verschieben
inline long TwipsToHMM(long nTwips) { return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)   { return (nHMM * 72 + 63) / 127; }

//------------------------------------------------------------------------

#define SCCHARPROPERTIES_SERVICE    "com.sun.star.style.CharacterProperties"
#define SCCELLPROPERTIES_SERVICE    "com.sun.star.table.CellProperties"
#define SCCELLRANGE_SERVICE         "com.sun.star.table.CellRange"
#define SCCELL_SERVICE              "com.sun.star.table.Cell"
#define SCSHEETCELLRANGES_SERVICE   "com.sun.star.sheet.SheetCellRanges"
#define SCSHEETCELLRANGE_SERVICE    "com.sun.star.sheet.SheetCellRange"
#define SCSPREADSHEET_SERVICE       "com.sun.star.sheet.Spreadsheet"
#define SCSHEETCELL_SERVICE         "com.sun.star.sheet.SheetCell"

SC_SIMPLE_SERVICE_INFO( ScCellFormatsEnumeration, "ScCellFormatsEnumeration", "com.sun.star.sheet.CellFormatRangesEnumeration" )
SC_SIMPLE_SERVICE_INFO( ScCellFormatsObj, "ScCellFormatsObj", "com.sun.star.sheet.CellFormatRanges" )
SC_SIMPLE_SERVICE_INFO( ScCellRangesBase, "ScCellRangesBase", "stardiv.unknown" )
SC_SIMPLE_SERVICE_INFO( ScCellsEnumeration, "ScCellsEnumeration", "com.sun.star.sheet.CellsEnumeration" )
SC_SIMPLE_SERVICE_INFO( ScCellsObj, "ScCellsObj", "com.sun.star.sheet.Cells" )
SC_SIMPLE_SERVICE_INFO( ScTableColumnObj, "ScTableColumnObj", "com.sun.star.table.TableColumn" )
SC_SIMPLE_SERVICE_INFO( ScTableRowObj, "ScTableRowObj", "com.sun.star.table.TableRow" )

//------------------------------------------------------------------------

SV_IMPL_PTRARR( XPropertyChangeListenerArr_Impl, XPropertyChangeListenerPtr );
SV_IMPL_PTRARR( ScNamedEntryArr_Impl, ScNamedEntryPtr );

//------------------------------------------------------------------------

#ifdef DBG_UTIL
#define TEST_PROPERTY_MAPS
#endif

#ifdef TEST_PROPERTY_MAPS

void lcl_TestMap( const SfxItemPropertyMap* pMap )
{
    while ( pMap->pName )
    {
        const SfxItemPropertyMap* pNext = pMap + 1;
        if ( pNext->pName )
        {
            int nDiff = strcmp( pMap->pName, pNext->pName );
            if ( nDiff >= 0 )
            {
                ByteString aErr("Reihenfolge: ");
                aErr += pMap->pName;
                aErr += '/';
                aErr += pNext->pName;
                DBG_ERROR( aErr.GetBuffer() );
            }
        }
        pMap = pNext;
    }
}

struct ScPropertyTester
{
    ScPropertyTester();
};

ScPropertyTester::ScPropertyTester()
{
    lcl_TestMap( lcl_GetCellsPropertyMap() );
    lcl_TestMap( lcl_GetRangePropertyMap() );
    lcl_TestMap( lcl_GetCellPropertyMap() );
    lcl_TestMap( lcl_GetColumnPropertyMap() );
    lcl_TestMap( lcl_GetRowPropertyMap() );
    lcl_TestMap( lcl_GetSheetPropertyMap() );
}

ScPropertyTester aPropertyTester;

#endif

//------------------------------------------------------------------------

//! ScLinkListener in anderes File verschieben !!!

ScLinkListener::~ScLinkListener()
{
}

void ScLinkListener::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    aLink.Call( (SfxHint*)&rHint );
}

//------------------------------------------------------------------------

void lcl_CopyProperties( beans::XPropertySet& rDest, beans::XPropertySet& rSource )
{
    uno::Reference<beans::XPropertySetInfo> xInfo = rSource.getPropertySetInfo();
    if (xInfo.is())
    {
        uno::Sequence<beans::Property> aSeq = xInfo->getProperties();
        const beans::Property* pAry = aSeq.getConstArray();
        ULONG nCount = aSeq.getLength();
        for (ULONG i=0; i<nCount; i++)
        {
            rtl::OUString aName = pAry[i].Name;
            rDest.setPropertyValue( aName, rSource.getPropertyValue( aName ) );
        }
    }
}

USHORT lcl_FirstTab( const ScRangeList& rRanges )
{
    DBG_ASSERT(rRanges.Count() >= 1, "was fuer Ranges ?!?!");
    const ScRange* pFirst = rRanges.GetObject(0);
    if (pFirst)
        return pFirst->aStart.Tab();

    return 0;   // soll nicht sein
}

BOOL lcl_WholeSheet( const ScRangeList& rRanges )
{
    if ( rRanges.Count() == 1 )
    {
        ScRange* pRange = rRanges.GetObject(0);
        if ( pRange && pRange->aStart.Col() == 0 && pRange->aEnd.Col() == MAXCOL &&
                       pRange->aStart.Row() == 0 && pRange->aEnd.Row() == MAXROW )
            return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------

ScSubTotalFunc lcl_SummaryToSubTotal( sheet::GeneralFunction eSummary )
{
    ScSubTotalFunc eSubTotal;
    switch (eSummary)
    {
        case sheet::GeneralFunction_SUM:
            eSubTotal = SUBTOTAL_FUNC_SUM;
            break;
        case sheet::GeneralFunction_COUNT:
            eSubTotal = SUBTOTAL_FUNC_CNT2;
            break;
        case sheet::GeneralFunction_AVERAGE:
            eSubTotal = SUBTOTAL_FUNC_AVE;
            break;
        case sheet::GeneralFunction_MAX:
            eSubTotal = SUBTOTAL_FUNC_MAX;
            break;
        case sheet::GeneralFunction_MIN:
            eSubTotal = SUBTOTAL_FUNC_MIN;
            break;
        case sheet::GeneralFunction_PRODUCT:
            eSubTotal = SUBTOTAL_FUNC_PROD;
            break;
        case sheet::GeneralFunction_COUNTNUMS:
            eSubTotal = SUBTOTAL_FUNC_CNT;
            break;
        case sheet::GeneralFunction_STDEV:
            eSubTotal = SUBTOTAL_FUNC_STD;
            break;
        case sheet::GeneralFunction_STDEVP:
            eSubTotal = SUBTOTAL_FUNC_STDP;
            break;
        case sheet::GeneralFunction_VAR:
            eSubTotal = SUBTOTAL_FUNC_VAR;
            break;
        case sheet::GeneralFunction_VARP:
            eSubTotal = SUBTOTAL_FUNC_VARP;
            break;

        case sheet::GeneralFunction_NONE:
        case sheet::GeneralFunction_AUTO:
        default:
            eSubTotal = SUBTOTAL_FUNC_NONE;
            break;
    }
    return eSubTotal;
}

//------------------------------------------------------------------------

const SvxBorderLine* lcl_GetBorderLine( SvxBorderLine& rLine, const table::BorderLine& rStruct )
{
    //  Calc braucht Twips, im Uno-Struct sind 1/100mm

    rLine.SetOutWidth( HMMToTwips( rStruct.OuterLineWidth ) );
    rLine.SetInWidth(  HMMToTwips( rStruct.InnerLineWidth ) );
    rLine.SetDistance( HMMToTwips( rStruct.LineDistance ) );
    rLine.SetColor( ColorData( rStruct.Color ) );

    if ( rLine.GetOutWidth() || rLine.GetInWidth() || rLine.GetDistance() )
        return &rLine;
    else
        return NULL;
}

void lcl_FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const table::TableBorder& rBorder )
{
    SvxBorderLine aLine;
    rOuter.SetDistance( HMMToTwips( rBorder.Distance ) );
    rOuter.SetLine( lcl_GetBorderLine( aLine, rBorder.TopLine ),        BOX_LINE_TOP );
    rOuter.SetLine( lcl_GetBorderLine( aLine, rBorder.BottomLine ),     BOX_LINE_BOTTOM );
    rOuter.SetLine( lcl_GetBorderLine( aLine, rBorder.LeftLine ),       BOX_LINE_LEFT );
    rOuter.SetLine( lcl_GetBorderLine( aLine, rBorder.RightLine ),      BOX_LINE_RIGHT );
    rInner.SetLine( lcl_GetBorderLine( aLine, rBorder.HorizontalLine ), BOXINFO_LINE_HORI );
    rInner.SetLine( lcl_GetBorderLine( aLine, rBorder.VerticalLine ),   BOXINFO_LINE_VERT );
    rInner.SetValid( VALID_TOP,      rBorder.IsTopLineValid );
    rInner.SetValid( VALID_BOTTOM,   rBorder.IsBottomLineValid );
    rInner.SetValid( VALID_LEFT,     rBorder.IsLeftLineValid );
    rInner.SetValid( VALID_RIGHT,    rBorder.IsRightLineValid );
    rInner.SetValid( VALID_HORI,     rBorder.IsHorizontalLineValid );
    rInner.SetValid( VALID_VERT,     rBorder.IsVerticalLineValid );
    rInner.SetValid( VALID_DISTANCE, rBorder.IsDistanceValid );
    rInner.SetTable( TRUE );
}

void lcl_FillBorderLine( table::BorderLine& rStruct, const SvxBorderLine* pLine )
{
    if (pLine)
    {
        rStruct.Color          = pLine->GetColor().GetColor();
        rStruct.InnerLineWidth = TwipsToHMM( pLine->GetInWidth() );
        rStruct.OuterLineWidth = TwipsToHMM( pLine->GetOutWidth() );
        rStruct.LineDistance   = TwipsToHMM( pLine->GetDistance() );
    }
    else
        rStruct.Color = rStruct.InnerLineWidth =
            rStruct.OuterLineWidth = rStruct.LineDistance = 0;
}

void lcl_FillTableBorder( table::TableBorder& rBorder,
                            const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner )
{
    lcl_FillBorderLine( rBorder.TopLine,        rOuter.GetTop() );
    lcl_FillBorderLine( rBorder.BottomLine,     rOuter.GetBottom() );
    lcl_FillBorderLine( rBorder.LeftLine,       rOuter.GetLeft() );
    lcl_FillBorderLine( rBorder.RightLine,      rOuter.GetRight() );
    lcl_FillBorderLine( rBorder.HorizontalLine, rInner.GetHori() );
    lcl_FillBorderLine( rBorder.VerticalLine,   rInner.GetVert() );

    rBorder.Distance                = rOuter.GetDistance();
    rBorder.IsTopLineValid          = rInner.IsValid(VALID_TOP);
    rBorder.IsBottomLineValid       = rInner.IsValid(VALID_BOTTOM);
    rBorder.IsLeftLineValid         = rInner.IsValid(VALID_LEFT);
    rBorder.IsRightLineValid        = rInner.IsValid(VALID_RIGHT);
    rBorder.IsHorizontalLineValid   = rInner.IsValid(VALID_HORI);
    rBorder.IsVerticalLineValid     = rInner.IsValid(VALID_VERT);
    rBorder.IsDistanceValid         = rInner.IsValid(VALID_DISTANCE);
}

//------------------------------------------------------------------------

//! lcl_ApplyBorder nach docfunc verschieben!

void lcl_ApplyBorder( ScDocShell* pDocShell, const ScRangeList& rRanges,
                        const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
    ULONG nCount = rRanges.Count();
    ULONG i;
    for (i=0; i<nCount; i++)
    {
        ScRange aRange = *rRanges.GetObject(i);
        USHORT nTab = aRange.aStart.Tab();

        if ( i==0 )
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
        else
            pUndoDoc->AddUndoTab( nTab, nTab );
        pDoc->CopyToDocument( aRange, IDF_ATTRIB, FALSE, pUndoDoc );

        ScMarkData aMark;
        aMark.SetMarkArea( aRange );
        aMark.SelectTable( nTab, TRUE );

        pDoc->ApplySelectionFrame( aMark, &rOuter, &rInner );
        // RowHeight bei Umrandung alleine nicht noetig
    }

    pDocShell->GetUndoManager()->AddUndoAction(
            new ScUndoBorder( pDocShell, rRanges, pUndoDoc, rOuter, rInner ) );

    for (i=0; i<nCount; i++)
        pDocShell->PostPaint( *rRanges.GetObject(i), PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    pDocShell->SetDocumentModified();
}

//------------------------------------------------------------------------

// Default-ctor fuer SMART_REFLECTION Krempel
ScCellRangesBase::ScCellRangesBase() :
    pDocShell( NULL ),
    aPropSet(lcl_GetCellsPropertyMap()),
    bChartColAsHdr( FALSE ),
    bChartRowAsHdr( FALSE ),
    pCurrentFlat( NULL ),
    pCurrentDeep( NULL ),
    pValueListener( NULL ),
    bValueChangePosted( FALSE )
{
}

ScCellRangesBase::ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR) :
    pDocShell( pDocSh ),
    aPropSet(lcl_GetCellsPropertyMap()),
    bChartColAsHdr( FALSE ),
    bChartRowAsHdr( FALSE ),
    pCurrentFlat( NULL ),
    pCurrentDeep( NULL ),
    pValueListener( NULL ),
    bValueChangePosted( FALSE )
{
    ScRange aCellRange = rR;
    aCellRange.Justify();
    aRanges.Append( aCellRange );

    if (pDocShell)  // Null, wenn per createInstance erzeugt...
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellRangesBase::ScCellRangesBase(ScDocShell* pDocSh, const ScRangeList& rR) :
    pDocShell( pDocSh ),
    aRanges( rR ),
    aPropSet(lcl_GetCellsPropertyMap()),
    bChartColAsHdr( FALSE ),
    bChartRowAsHdr( FALSE ),
    pCurrentFlat( NULL ),
    pCurrentDeep( NULL ),
    pValueListener( NULL ),
    bValueChangePosted( FALSE )
{
    if (pDocShell)  // Null, wenn per createInstance erzeugt...
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellRangesBase::~ScCellRangesBase()
{
    ForgetCurrentAttrs();

    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pValueListener;

    //! XChartDataChangeEventListener abmelden ??
    //! (ChartCollection haelt dann auch dieses Objekt fest!)
}

void ScCellRangesBase::ForgetCurrentAttrs()
{
    delete pCurrentFlat;
    delete pCurrentDeep;
    pCurrentFlat = NULL;
    pCurrentDeep = NULL;
}

const ScPatternAttr* ScCellRangesBase::GetCurrentAttrsFlat()
{
    //  get and cache direct cell attributes for this object's range

    if ( !pCurrentFlat && pDocShell )
    {
        ScMarkData aMark;
        aMark.MarkFromRangeList( aRanges, FALSE );
        ScDocument* pDoc = pDocShell->GetDocument();
        pCurrentFlat = pDoc->CreateSelectionPattern( aMark, FALSE );
    }
    return pCurrentFlat;
}

const ScPatternAttr* ScCellRangesBase::GetCurrentAttrsDeep()
{
    //  get and cache cell attributes (incl. styles) for this object's range

    if ( !pCurrentDeep && pDocShell )
    {
        ScMarkData aMark;
        aMark.MarkFromRangeList( aRanges, FALSE );
        ScDocument* pDoc = pDocShell->GetDocument();
        pCurrentDeep = pDoc->CreateSelectionPattern( aMark, TRUE );
    }
    return pCurrentDeep;
}

void ScCellRangesBase::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;
        if ( aRanges.UpdateReference( rRef.GetMode(), pDocShell->GetDocument(), rRef.GetRange(),
                                        rRef.GetDx(), rRef.GetDy(), rRef.GetDz() ) )
            RefChanged();
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            ForgetCurrentAttrs();
            pDocShell = NULL;           // invalid
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            // document content changed -> forget cached attributes
            ForgetCurrentAttrs();
        }
    }
}

void ScCellRangesBase::RefChanged()
{
    //! XChartDataChangeEventListener anpassen

    //! Test !!!

    if ( pValueListener && aValueListeners.Count() != 0 )
    {
        pValueListener->EndListeningAll();

        ScDocument* pDoc = pDocShell->GetDocument();
        ULONG nCount = aRanges.Count();
        for (ULONG i=0; i<nCount; i++)
            pDoc->StartListeningArea( *aRanges.GetObject(i), pValueListener );
    }
    //! Test !!!
}

ScDocument* ScCellRangesBase::GetDocument() const
{
    if (pDocShell)
        return pDocShell->GetDocument();
    else
        return NULL;
}

void ScCellRangesBase::InitInsertRange(ScDocShell* pDocSh, const ScRange& rR)
{
    if ( !pDocShell && pDocSh )
    {
        pDocShell = pDocSh;

        ScRange aCellRange = rR;
        aCellRange.Justify();
        aRanges.RemoveAll();
        aRanges.Append( aCellRange );

        pDocShell->GetDocument()->AddUnoObject(*this);

        RefChanged();   // Range im Range-Objekt anpassen
    }
}

void ScCellRangesBase::SetNewRange(const ScRange& rNew)
{
    ScRange aCellRange = rNew;
    aCellRange.Justify();

    aRanges.RemoveAll();
    aRanges.Append( aCellRange );
    RefChanged();
}

void ScCellRangesBase::SetNewRanges(const ScRangeList& rNew)
{
    aRanges = rNew;
    RefChanged();
}

uno::Any SAL_CALL ScCellRangesBase::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( beans::XPropertyState )
    SC_QUERYINTERFACE( sheet::XSheetOperation )
    SC_QUERYINTERFACE( chart::XChartDataArray )
    SC_QUERYINTERFACE( chart::XChartData )
    SC_QUERYINTERFACE( util::XIndent )
    SC_QUERYINTERFACE( sheet::XCellRangesQuery )
    SC_QUERYINTERFACE( sheet::XFormulaQuery )
    SC_QUERYINTERFACE( util::XReplaceable )
    SC_QUERYINTERFACE( util::XSearchable )
    SC_QUERYINTERFACE( lang::XServiceInfo )
    SC_QUERYINTERFACE( lang::XUnoTunnel )
    SC_QUERYINTERFACE( lang::XTypeProvider )

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL ScCellRangesBase::acquire() throw(uno::RuntimeException)
{
    OWeakObject::acquire();
}

void SAL_CALL ScCellRangesBase::release() throw(uno::RuntimeException)
{
    OWeakObject::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellRangesBase::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc(11);
        uno::Type* pPtr = aTypes.getArray();
        pPtr[0] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[1] = getCppuType((const uno::Reference<beans::XPropertyState>*)0);
        pPtr[2] = getCppuType((const uno::Reference<sheet::XSheetOperation>*)0);
        pPtr[3] = getCppuType((const uno::Reference<chart::XChartDataArray>*)0);
        pPtr[4] = getCppuType((const uno::Reference<util::XIndent>*)0);
        pPtr[5] = getCppuType((const uno::Reference<sheet::XCellRangesQuery>*)0);
        pPtr[6] = getCppuType((const uno::Reference<sheet::XFormulaQuery>*)0);
        pPtr[7] = getCppuType((const uno::Reference<util::XReplaceable>*)0);
        pPtr[8] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
        pPtr[9] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[10]= getCppuType((const uno::Reference<lang::XTypeProvider>*)0);
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellRangesBase::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// ---

void ScCellRangesBase::PaintRanges_Impl( USHORT nPart )
{
    ULONG nCount = aRanges.Count();
    for (ULONG i=0; i<nCount; i++)
        pDocShell->PostPaint( *aRanges.GetObject(i), nPart );
}

// XSheetOperation

double SAL_CALL ScCellRangesBase::computeFunction( sheet::GeneralFunction nFunction )
                                                throw(uno::Exception, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScMarkData aMark;
    aMark.MarkFromRangeList( aRanges, FALSE );
    aMark.MarkToSimple();
    if (!aMark.IsMarked())
        aMark.SetMarkNegative(TRUE);    // um Dummy Position angeben zu koennen

    ScAddress aDummy;                   // wenn nicht Marked, ignoriert wegen Negative
    double fVal;
    ScSubTotalFunc eFunc = lcl_SummaryToSubTotal( nFunction );
    ScDocument* pDoc = pDocShell->GetDocument();
    if ( !pDoc->GetSelectionFunction( eFunc, aDummy, aMark, fVal ) )
    {
        throw uno::RuntimeException();      //! own exception?
    }

    return fVal;
}

void SAL_CALL ScCellRangesBase::clearContents( sal_Int32 nContentFlags ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() )
    {
        ScMarkData aMark;
        aMark.MarkFromRangeList( aRanges, FALSE );

        // only for clearContents: EDITATTR is only used if no contents are deleted
        USHORT nDelFlags = nContentFlags & IDF_ALL;
        if ( ( nContentFlags & IDF_EDITATTR ) && ( nContentFlags & IDF_CONTENTS ) == 0 )
            nDelFlags |= IDF_EDITATTR;

        ScDocFunc aFunc(*pDocShell);
        aFunc.DeleteContents( aMark, nDelFlags, TRUE, TRUE );
    }
    // sonst ist nichts zu tun
}

// XPropertyState

USHORT lcl_GetPropertyWhich( const String& rName )
{
    //  Which-ID des betroffenen Items, auch wenn das Item die Property
    //  nicht alleine behandeln kann

    USHORT nWhich = 0;
    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), rName );
    if ( pMap && pMap->nWID )
        nWhich = pMap->nWID;
    else if ( rName.EqualsAscii( SC_UNONAME_TBLBORD ) )
        nWhich = ATTR_BORDER;
    else if ( rName.EqualsAscii( SC_UNONAME_CONDFMT ) || rName.EqualsAscii( SC_UNONAME_CONDLOC ) )
        nWhich = ATTR_CONDITIONAL;
    else if ( rName.EqualsAscii( SC_UNONAME_VALIDAT ) || rName.EqualsAscii( SC_UNONAME_VALILOC ) )
        nWhich = ATTR_VALIDDATA;
    return nWhich;
}

beans::PropertyState SAL_CALL ScCellRangesBase::getPropertyState( const rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() == 0 )
        throw uno::RuntimeException();

    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    if ( pDocShell )
    {
        String aNameString = aPropertyName;
        USHORT nWhich = lcl_GetPropertyWhich( aNameString );

        if ( nWhich )           // irgendwie zustandegekommene Which-ID
        {
            //! Bei Items, die mehrere Properties enthalten (z.B. Hintergrund)
            //! wird hier evtl. zu oft "Ambiguous" zurueckgegeben

            //  for PropertyState, don't look at styles
            const ScPatternAttr* pPattern = GetCurrentAttrsFlat();
            if ( pPattern )
            {
                SfxItemState eState = pPattern->GetItemSet().GetItemState( nWhich, FALSE );

                //  if no rotate value is set, look at orientation
                //! also for a fixed value of 0 (in case orientation is ambiguous)?
                if ( nWhich == ATTR_ROTATE_VALUE && eState == SFX_ITEM_DEFAULT )
                    eState = pPattern->GetItemSet().GetItemState( ATTR_ORIENTATION, FALSE );

                if ( eState == SFX_ITEM_SET )
                    eRet = beans::PropertyState_DIRECT_VALUE;
                else if ( eState == SFX_ITEM_DEFAULT )
                    eRet = beans::PropertyState_DEFAULT_VALUE;
                else if ( eState == SFX_ITEM_DONTCARE )
                    eRet = beans::PropertyState_AMBIGUOUS_VALUE;
                else
                    DBG_ERROR("unbekannter ItemState");
            }
        }
        else if ( aNameString.EqualsAscii( SC_UNONAME_CHCOLHDR ) || aNameString.EqualsAscii( SC_UNONAME_CHROWHDR ) )
            eRet = beans::PropertyState_DIRECT_VALUE;
        else if ( aNameString.EqualsAscii( SC_UNONAME_CELLSTYL ) )
        {
            //  a style is always set, there's no default state
            ScMarkData aMark;
            aMark.MarkFromRangeList( aRanges, FALSE );
            const ScStyleSheet* pStyle = pDocShell->GetDocument()->GetSelectionStyle(aMark);
            if (pStyle)
                eRet = beans::PropertyState_DIRECT_VALUE;
            else
                eRet = beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }

    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScCellRangesBase::getPropertyStates(
                                const uno::Sequence<rtl::OUString>& aPropertyNames )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    //  duemmliche Default-Implementierung: alles einzeln per getPropertyState holen
    //! sollte optimiert werden!

    ScUnoGuard aGuard;
    const rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(INT32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScCellRangesBase::setPropertyToDefault( const rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell )
    {
        String aNameString = aPropertyName;
        USHORT nWhich = lcl_GetPropertyWhich( aNameString );

        if ( nWhich )           // irgendwie zustandegekommene Which-ID
        {
            if ( aRanges.Count() )      // leer = nichts zu tun
            {
                ScMarkData aMark;
                aMark.MarkFromRangeList( aRanges, FALSE );
                ScDocument* pDoc = pDocShell->GetDocument();
                ScDocFunc aFunc(*pDocShell);

                //! Bei Items, die mehrere Properties enthalten (z.B. Hintergrund)
                //! wird hier zuviel zurueckgesetzt

                //! for ATTR_ROTATE_VALUE, also reset ATTR_ORIENTATION?

                USHORT aWIDs[3];
                aWIDs[0] = nWhich;
                if ( nWhich == ATTR_VALUE_FORMAT )
                {
                    aWIDs[1] = ATTR_LANGUAGE_FORMAT;    // #67847# language for number formats
                    aWIDs[2] = 0;
                }
                else
                    aWIDs[1] = 0;
                aFunc.ClearItems( aMark, aWIDs, TRUE );
            }
        }
        else if ( aNameString.EqualsAscii( SC_UNONAME_CHCOLHDR ) )
            bChartColAsHdr = FALSE;
        else if ( aNameString.EqualsAscii( SC_UNONAME_CHROWHDR ) )
            bChartRowAsHdr = FALSE;
        else if ( aNameString.EqualsAscii( SC_UNONAME_CELLSTYL ) )
        {
            ScMarkData aMark;
            aMark.MarkFromRangeList( aRanges, FALSE );
            ScDocFunc aFunc(*pDocShell);
            aFunc.ApplyStyle( aMark, ScGlobal::GetRscString(STR_STYLENAME_STANDARD), TRUE, TRUE );
        }
    }
}

uno::Any SAL_CALL ScCellRangesBase::getPropertyDefault( const rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    //! mit getPropertyValue zusammenfassen

    ScUnoGuard aGuard;
    uno::Any aAny;

    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        String aNameString = aPropertyName;
        const SfxItemPropertyMap* pMap =
                SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
        if ( pMap && pMap->nWID )
        {
            const ScPatternAttr* pPattern = pDoc->GetDefPattern();
            if ( pPattern )
            {
                const SfxItemSet& rSet = pPattern->GetItemSet();

                switch ( pMap->nWID )       // fuer Item-Spezial-Behandlungen
                {
                    case ATTR_VALUE_FORMAT:
                        //  default has no language set
                        aAny <<= (sal_Int32)( ((const SfxUInt32Item&)rSet.Get(pMap->nWID)).GetValue() );
                        break;
                    case ATTR_INDENT:
                        aAny <<= (sal_Int16)( TwipsToHMM(((const SfxUInt16Item&)
                                        rSet.Get(pMap->nWID)).GetValue()) );
                        break;
                    default:
                        aAny = aPropSet.getPropertyValue(aNameString, rSet);
                }
            }
        }
        else        // hier direkt implementierte
        {
            if ( aNameString.EqualsAscii( SC_UNONAME_CHCOLHDR ) || aNameString.EqualsAscii( SC_UNONAME_CHROWHDR ) )
                ScUnoHelpFunctions::SetBoolInAny( aAny, FALSE );
            else if ( aNameString.EqualsAscii( SC_UNONAME_CELLSTYL ) )
                aAny <<= rtl::OUString( ScStyleNameConversion::DisplayToProgrammaticName(
                            ScGlobal::GetRscString(STR_STYLENAME_STANDARD), SFX_STYLE_FAMILY_PARA ) );
            else if ( aNameString.EqualsAscii( SC_UNONAME_TBLBORD ) )
            {
                const ScPatternAttr* pPattern = pDoc->GetDefPattern();
                if ( pPattern )
                {
                    table::TableBorder aBorder;
                    lcl_FillTableBorder( aBorder,
                            (const SvxBoxItem&)pPattern->GetItem(ATTR_BORDER),
                            (const SvxBoxInfoItem&)pPattern->GetItem(ATTR_BORDER_INNER) );
                    aAny <<= aBorder;
                }
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_CONDFMT ) || aNameString.EqualsAscii( SC_UNONAME_CONDLOC ) )
            {
                BOOL bEnglish = ( aNameString.EqualsAscii( SC_UNONAME_CONDFMT ) );

                uno::Reference<sheet::XSheetConditionalEntries> xObj =
                        new ScTableConditionalFormat( pDoc, 0, bEnglish );
                aAny <<= xObj;
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_VALIDAT ) || aNameString.EqualsAscii( SC_UNONAME_VALILOC ) )
            {
                BOOL bEnglish = ( aNameString.EqualsAscii( SC_UNONAME_VALIDAT ) );

                uno::Reference<beans::XPropertySet> xObj =
                        new ScTableValidationObj( pDoc, 0, bEnglish );
                aAny <<= xObj;
            }
        }
    }

    return aAny;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellRangesBase::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScCellRangesBase::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell )
    {
        String aNameString = aPropertyName;
        const SfxItemPropertyMap* pMap =
                SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
        if ( pMap && pMap->nWID )
        {
            if ( aRanges.Count() )      // leer = nichts zu tun
            {
                ScMarkData aMark;
                aMark.MarkFromRangeList( aRanges, FALSE );
                ScDocument* pDoc = pDocShell->GetDocument();
                ScDocFunc aFunc(*pDocShell);

                //  Fuer Teile von zusammengesetzten Items mit mehreren Properties (z.B. Hintergrund)
                //  muss vorher das alte Item aus dem Dokument geholt werden
                //! Das kann hier aber nicht erkannt werden
                //! -> eigenes Flag im PropertyMap-Eintrag, oder was ???
                //! Item direkt von einzelner Position im Bereich holen?
                //  ClearInvalidItems, damit auf jeden Fall ein Item vom richtigen Typ da ist

                ScPatternAttr aPattern( *GetCurrentAttrsDeep() );
                SfxItemSet& rSet = aPattern.GetItemSet();
                rSet.ClearInvalidItems();

                USHORT nDontClear = 0;

                switch ( pMap->nWID )       // fuer Item-Spezial-Behandlungen
                {
                    case ATTR_VALUE_FORMAT:
                        {
                            // #67847# language for number formats
                            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                            ULONG nOldFormat = ((const SfxUInt32Item&)
                                    rSet.Get( ATTR_VALUE_FORMAT )).GetValue();
                            LanguageType eOldLang = ((const SvxLanguageItem&)
                                    rSet.Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                            nOldFormat = pFormatter->
                                    GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );

                            sal_Int32 nIntVal;
                            if ( aValue >>= nIntVal )
                            {
                                ULONG nNewFormat = (ULONG)nIntVal;
                                rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );

                                const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewFormat );
                                LanguageType eNewLang =
                                    pNewEntry ? pNewEntry->GetLanguage() : LANGUAGE_DONTKNOW;
                                if ( eNewLang != eOldLang && eNewLang != LANGUAGE_DONTKNOW )
                                {
                                    rSet.Put( SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );

                                    // #40606# if only language is changed,
                                    // don't touch number format attribute
                                    ULONG nNewMod = nNewFormat % SV_COUNTRY_LANGUAGE_OFFSET;
                                    if ( nNewMod == ( nOldFormat % SV_COUNTRY_LANGUAGE_OFFSET ) &&
                                         nNewMod <= SV_MAX_ANZ_STANDARD_FORMATE )
                                        rSet.ClearItem( ATTR_VALUE_FORMAT );

                                    nDontClear = ATTR_LANGUAGE_FORMAT;
                                }
                            }
                        }
                        break;
                    case ATTR_INDENT:
                        {
                            sal_Int16 nIntVal;
                            if ( aValue >>= nIntVal )
                                rSet.Put( SfxUInt16Item( pMap->nWID, HMMToTwips(nIntVal) ) );
                        }
                        break;
                    case ATTR_ROTATE_VALUE:
                        {
                            sal_Int32 nRotVal;
                            if ( aValue >>= nRotVal )
                            {
                                //  stored value is always between 0 and 360 deg.
                                nRotVal %= 36000;
                                if ( nRotVal < 0 )
                                    nRotVal += 36000;

                                //  always set rotation value and orientation

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
                                {
                                    rSet.Put( SvxOrientationItem( eOrient, ATTR_ORIENTATION ) );
                                    nDontClear = ATTR_ORIENTATION;
                                }
                            }
                        }
                        break;
                    default:
                        aPropSet.setPropertyValue(aNameString, aValue, rSet);
                }

                //  alle anderen Items muessen aber wieder raus...
                //! oder neues Pattern, nur das betroffene Item putten?
                for (USHORT nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; nWhich++)
                    if ( nWhich != pMap->nWID && nWhich != nDontClear )
                        rSet.ClearItem(nWhich);

                aFunc.ApplyAttributes( aMark, aPattern, TRUE, TRUE );
            }
        }
        else        // hier direkt implementierte
        {
            //  Chart-Header Flags werden nur an diesem Objekt gesetzt,
            //  nicht im Dokument gespeichert

            if ( aNameString.EqualsAscii( SC_UNONAME_CHCOLHDR ) )
                bChartColAsHdr = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            else if ( aNameString.EqualsAscii( SC_UNONAME_CHROWHDR ) )
                bChartRowAsHdr = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            else if ( aNameString.EqualsAscii( SC_UNONAME_CELLSTYL ) )
            {
                rtl::OUString aStrVal;
                aValue >>= aStrVal;
                String aString = ScStyleNameConversion::ProgrammaticToDisplayName(
                                                    aStrVal, SFX_STYLE_FAMILY_PARA );
                ScMarkData aMark;
                aMark.MarkFromRangeList( aRanges, FALSE );
                ScDocFunc aFunc(*pDocShell);
                aFunc.ApplyStyle( aMark, aString, TRUE, TRUE );
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_TBLBORD ) )
            {
                table::TableBorder aBorder;
                if ( aRanges.Count() && ( aValue >>= aBorder ) )    // leer = nichts zu tun
                {
                    SvxBoxItem aOuter(ATTR_BORDER);
                    SvxBoxInfoItem aInner(ATTR_BORDER_INNER);
                    lcl_FillBoxItems( aOuter, aInner, aBorder );

                    lcl_ApplyBorder( pDocShell, aRanges, aOuter, aInner );  //! docfunc
                }
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_CONDFMT ) || aNameString.EqualsAscii( SC_UNONAME_CONDLOC ) )
            {
                BOOL bEnglish = ( aNameString.EqualsAscii( SC_UNONAME_CONDFMT ) );

                uno::Reference<sheet::XSheetConditionalEntries> xInterface;
                if ( aRanges.Count() && ( aValue >>= xInterface ) ) // leer = nichts zu tun
                {
                    ScTableConditionalFormat* pFormat =
                            ScTableConditionalFormat::getImplementation( xInterface );
                    if (pFormat)
                    {
                        ScDocument* pDoc = pDocShell->GetDocument();

                        ScConditionalFormat aNew( 0, pDoc );    // Index wird beim Einfuegen gesetzt
                        pFormat->FillFormat( aNew, pDoc, bEnglish );
                        ULONG nIndex = pDoc->AddCondFormat( aNew );

                        ScMarkData aMark;
                        aMark.MarkFromRangeList( aRanges, FALSE );
                        ScDocFunc aFunc(*pDocShell);

                        ScPatternAttr aPattern( pDoc->GetPool() );
                        aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_CONDITIONAL, nIndex ) );
                        aFunc.ApplyAttributes( aMark, aPattern, TRUE, TRUE );
                    }
                }
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_VALIDAT ) || aNameString.EqualsAscii( SC_UNONAME_VALILOC ) )
            {
                BOOL bEnglish = ( aNameString.EqualsAscii( SC_UNONAME_VALIDAT ) );

                uno::Reference<beans::XPropertySet> xInterface;
                if ( aRanges.Count() && ( aValue >>= xInterface ) ) // leer = nichts zu tun
                {
                    ScTableValidationObj* pValidObj =
                            ScTableValidationObj::getImplementation( xInterface );
                    if (pValidObj)
                    {
                        ScDocument* pDoc = pDocShell->GetDocument();

                        ScValidationData* pNewData =
                                pValidObj->CreateValidationData( pDoc, bEnglish );
                        ULONG nIndex = pDoc->AddValidationEntry( *pNewData );
                        delete pNewData;

                        ScMarkData aMark;
                        aMark.MarkFromRangeList( aRanges, FALSE );
                        ScDocFunc aFunc(*pDocShell);

                        ScPatternAttr aPattern( pDoc->GetPool() );
                        aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nIndex ) );
                        aFunc.ApplyAttributes( aMark, aPattern, TRUE, TRUE );
                    }
                }
            }
        }
    }
}

uno::Any SAL_CALL ScCellRangesBase::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() == 0 )
        throw uno::RuntimeException();

    uno::Any aAny;
    if ( pDocShell )
    {
        String aNameString = aPropertyName;
        const SfxItemPropertyMap* pMap =
                SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
        if ( pMap && pMap->nWID )
        {
            const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
            if ( pPattern )
            {
                //  Dontcare durch Default ersetzen, damit man immer eine Reflection hat
                SfxItemSet aDataSet( pPattern->GetItemSet() );
                aDataSet.ClearInvalidItems();

                switch ( pMap->nWID )       // fuer Item-Spezial-Behandlungen
                {
                    case ATTR_VALUE_FORMAT:
                        {
                            ScDocument* pDoc = pDocShell->GetDocument();
                            ULONG nOldFormat = ((const SfxUInt32Item&)
                                    aDataSet.Get( ATTR_VALUE_FORMAT )).GetValue();
                            LanguageType eOldLang = ((const SvxLanguageItem&)
                                    aDataSet.Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                            nOldFormat = pDoc->GetFormatTable()->
                                    GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );
                            aAny <<= (sal_Int32)( nOldFormat );
                        }
                        break;
                    case ATTR_INDENT:
                        aAny <<= (sal_Int16)( TwipsToHMM(((const SfxUInt16Item&)
                                        aDataSet.Get(pMap->nWID)).GetValue()) );
                        break;
                    case ATTR_ROTATE_VALUE:
                        {
                            //  if value is 0 and orientation topbottom or bottomtop,
                            //  adjust value

                            sal_Int32 nRotVal = ((const SfxInt32Item&)
                                        aDataSet.Get(ATTR_ROTATE_VALUE)).GetValue();
                            if ( nRotVal == 0 )
                            {
                                SvxCellOrientation eOrient = (SvxCellOrientation)
                                        ((const SvxOrientationItem&)aDataSet.Get(ATTR_ORIENTATION)).
                                            GetValue();
                                if ( eOrient == SVX_ORIENTATION_BOTTOMTOP )
                                    nRotVal = 9000;
                                else if ( eOrient == SVX_ORIENTATION_TOPBOTTOM )
                                    nRotVal = 27000;
                            }
                            aAny <<= nRotVal;
                        }
                        break;
                    default:
                        aAny = aPropSet.getPropertyValue(aNameString, aDataSet);
                }
            }
        }
        else        // hier direkt implementierte
        {
            if ( aNameString.EqualsAscii( SC_UNONAME_CHCOLHDR ) )
                ScUnoHelpFunctions::SetBoolInAny( aAny, bChartColAsHdr );
            else if ( aNameString.EqualsAscii( SC_UNONAME_CHROWHDR ) )
                ScUnoHelpFunctions::SetBoolInAny( aAny, bChartRowAsHdr );
            else if ( aNameString.EqualsAscii( SC_UNONAME_CELLSTYL ) )
            {
                ScMarkData aMark;
                aMark.MarkFromRangeList( aRanges, FALSE );
                String aStyleName;
                const ScStyleSheet* pStyle = pDocShell->GetDocument()->GetSelectionStyle(aMark);
                if (pStyle)
                    aStyleName = pStyle->GetName();
                aAny <<= rtl::OUString( ScStyleNameConversion::DisplayToProgrammaticName(
                                                        aStyleName, SFX_STYLE_FAMILY_PARA ) );
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_TBLBORD ) )
            {
                //! alle Ranges durchgehen
                const ScRange* pFirst = aRanges.GetObject(0);
                if (pFirst)
                {
                    SvxBoxItem aOuter(ATTR_BORDER);
                    SvxBoxInfoItem aInner(ATTR_BORDER_INNER);

                    ScDocument* pDoc = pDocShell->GetDocument();
                    ScMarkData aMark;
                    aMark.SetMarkArea( *pFirst );
                    aMark.SelectTable( pFirst->aStart.Tab(), TRUE );
                    pDoc->GetSelectionFrame( aMark, aOuter, aInner );

                    table::TableBorder aBorder;
                    lcl_FillTableBorder( aBorder, aOuter, aInner );
                    aAny <<= aBorder;
                }
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_CONDFMT ) || aNameString.EqualsAscii( SC_UNONAME_CONDLOC ) )
            {
                BOOL bEnglish = ( aNameString.EqualsAscii( SC_UNONAME_CONDFMT ) );

                const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
                if ( pPattern )
                {
                    ScDocument* pDoc = pDocShell->GetDocument();
                    ULONG nIndex = ((const SfxUInt32Item&)
                            pPattern->GetItem(ATTR_CONDITIONAL)).GetValue();
                    uno::Reference<sheet::XSheetConditionalEntries> xObj =
                            new ScTableConditionalFormat( pDoc, nIndex, bEnglish );
                    aAny <<= xObj;
                }
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_VALIDAT ) || aNameString.EqualsAscii( SC_UNONAME_VALILOC ) )
            {
                BOOL bEnglish = ( aNameString.EqualsAscii( SC_UNONAME_VALIDAT ) );

                const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
                if ( pPattern )
                {
                    ScDocument* pDoc = pDocShell->GetDocument();
                    ULONG nIndex = ((const SfxUInt32Item&)
                            pPattern->GetItem(ATTR_VALIDDATA)).GetValue();
                    uno::Reference<beans::XPropertySet> xObj =
                            new ScTableValidationObj( pDoc, nIndex, bEnglish );
                    aAny <<= xObj;
                }
            }
        }
    }

    return aAny;
}

void SAL_CALL ScCellRangesBase::addPropertyChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() == 0 )
        throw uno::RuntimeException();

    //! Test !!!!!
    if (String(aPropertyName).EqualsAscii( "Test" ))
    {
        uno::Reference<beans::XPropertyChangeListener> *pObj =
                new uno::Reference<beans::XPropertyChangeListener>( aListener );
        aValueListeners.Insert( pObj, aValueListeners.Count() );

        if ( aValueListeners.Count() == 1 )
        {
            if (!pValueListener)
                pValueListener = new ScLinkListener( LINK( this, ScCellRangesBase, ValueListenerHdl ) );

            ScDocument* pDoc = pDocShell->GetDocument();
            ULONG nCount = aRanges.Count();
            for (ULONG i=0; i<nCount; i++)
                pDoc->StartListeningArea( *aRanges.GetObject(i), pValueListener );

            acquire();  // nicht verlieren (eine Ref fuer alle Listener)
        }

        return;
    }
    //! Test !!!!!

    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::removePropertyChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() == 0 )
        throw uno::RuntimeException();

    //! Test !!!!!
    if (String(aPropertyName).EqualsAscii( "Test" ))
    {
        acquire();      // falls fuer Listener die letzte Ref existiert - wird unten freigegeben

        USHORT nCount = aValueListeners.Count();
        for ( USHORT n=nCount; n--; )
        {
            uno::Reference<beans::XPropertyChangeListener> *pObj = aValueListeners[n];
            if ( *pObj == aListener )
            {
                aValueListeners.DeleteAndDestroy( n );

                if ( aValueListeners.Count() == 0 )
                {
                    if (pValueListener)
                        pValueListener->EndListeningAll();

                    release();      // Listener-Ref freigeben
                }

                break;
            }
        }

        release();      // damit kann dieses Objekt geloescht werden
        return;
    }
    //! Test !!!!!

    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::addVetoableChangeListener( const rtl::OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::removeVetoableChangeListener( const rtl::OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

//! Test !!!

IMPL_LINK( ScCellRangesBase, ValueListenerHdl, SfxHint*, pHint )
{
    if ( pDocShell && pHint && pHint->ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint*)pHint)->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING) )
    {
        //  nicht doppelt notifien, wenn sich mehrere Formeln im Bereich aendern...

        if ( aValueListeners.Count() && !bValueChangePosted )
        {
            //  Die Listener koennen nur asynchron benachrichtigt werden, weil im
            //  Formel-Broadcast auf keinen Fall Reschedule gerufen werden darf

            beans::PropertyChangeEvent* pEvent = new beans::PropertyChangeEvent;
            pEvent->Source         = (cppu::OWeakObject*)this;
            pEvent->PropertyName   = rtl::OUString::createFromAscii( "Test" );
            pEvent->Further        = FALSE;
            pEvent->PropertyHandle = -1;
            pEvent->OldValue       = uno::Any();
            pEvent->NewValue       = uno::Any();

            // Die Ref im Event-Objekt haelt dieses Objekt fest
            // Das Event-Objekt wird im Link-Handler geloescht

            bValueChangePosted = TRUE;
            Application::PostUserEvent( LINK( this, ScCellRangesBase, ValueChanged ), pEvent );
        }
    }
    return 0;
}

//  ValueChanged wird asynchron gerufen

IMPL_LINK( ScCellRangesBase, ValueChanged, beans::PropertyChangeEvent*, pEvent )
{
    if ( pEvent )
    {
        if ( pDocShell )
            for ( USHORT n=0; n<aValueListeners.Count(); n++ )
                (*aValueListeners[n])->propertyChange( *pEvent );

        bValueChangePosted = FALSE;

        delete pEvent;  // damit kann auch dieses Objekt geloescht werden
    }
    return 0;
}

//! Test !!!!!

// XIndent

void SAL_CALL ScCellRangesBase::decrementIndent() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell && aRanges.Count() )     // leer = nichts zu tun
    {
        ScMarkData aMark;
        aMark.MarkFromRangeList( aRanges, FALSE );
        ScDocFunc aFunc(*pDocShell);
        aFunc.ChangeIndent( aMark, FALSE, TRUE );
    }
}

void SAL_CALL ScCellRangesBase::incrementIndent() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell && aRanges.Count() )     // leer = nichts zu tun
    {
        ScMarkData aMark;
        aMark.MarkFromRangeList( aRanges, FALSE );
        ScDocFunc aFunc(*pDocShell);
        aFunc.ChangeIndent( aMark, TRUE, TRUE );
    }
}

// XChartData

SchMemChart* ScCellRangesBase::CreateMemChart_Impl() const
{
    if ( pDocShell && aRanges.Count() )
    {
        ScRangeListRef xChartRanges;
        if ( aRanges.Count() == 1 )
        {
            //  ganze Tabelle sinnvoll begrenzen (auf belegten Datenbereich)
            //  (nur hier, Listener werden auf den ganzen Bereich angemeldet)
            //! direkt testen, ob es ein ScTableSheetObj ist?

            ScRange* pRange = aRanges.GetObject(0);
            if ( pRange->aStart.Col() == 0 && pRange->aEnd.Col() == MAXCOL &&
                 pRange->aStart.Row() == 0 && pRange->aEnd.Row() == MAXROW )
            {
                USHORT nTab = pRange->aStart.Tab();

                USHORT nStartX, nStartY;        // Anfang holen
                if (!pDocShell->GetDocument()->GetDataStart( nTab, nStartX, nStartY ))
                    nStartX = nStartY = 0;

                USHORT nEndX, nEndY;            // Ende holen
                if (!pDocShell->GetDocument()->GetTableArea( nTab, nEndX, nEndY ))
                    nEndX = nEndY = 0;

                xChartRanges = new ScRangeList;
                xChartRanges->Append( ScRange( nStartX, nStartY, nTab, nEndX, nEndY, nTab ) );
            }
        }
        if (!xChartRanges.Is())         //  sonst Ranges direkt uebernehmen
            xChartRanges = new ScRangeList(aRanges);
        ScChartArray aArr( pDocShell->GetDocument(), xChartRanges, String() );

        // RowAsHdr = ColHeaders und umgekehrt
        aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );

        return aArr.CreateMemChart();
    }
    return NULL;
}

uno::Sequence< uno::Sequence<double> > SAL_CALL ScCellRangesBase::getData()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SchMemChart* pMemChart = CreateMemChart_Impl();
    if ( pMemChart )
    {
        short nColCount = pMemChart->GetColCount();
        short nRowCount = pMemChart->GetRowCount();

        uno::Sequence< uno::Sequence<double> > aRowSeq( nRowCount );
        uno::Sequence<double>* pRowAry = aRowSeq.getArray();
        for (short nRow = 0; nRow < nRowCount; nRow++)
        {
            uno::Sequence<double> aColSeq( nColCount );
            double* pColAry = aColSeq.getArray();
            for (short nCol = 0; nCol < nColCount; nCol++)
                pColAry[nCol] = pMemChart->GetData( nCol, nRow );

            pRowAry[nRow] = aColSeq;
        }

        delete pMemChart;
        return aRowSeq;
    }

    return uno::Sequence< uno::Sequence<double> >(0);
}

void SAL_CALL ScCellRangesBase::setData( const uno::Sequence< uno::Sequence<double> >& aData )
                                                throw(uno::RuntimeException)
{
    //! ist das ernst gemeint?
    DBG_ERROR("not implemented");
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellRangesBase::getRowDescriptions()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SchMemChart* pMemChart = CreateMemChart_Impl();
    if ( pMemChart )
    {
        short nRowCount = pMemChart->GetRowCount();
        uno::Sequence<rtl::OUString> aSeq( nRowCount );
        rtl::OUString* pAry = aSeq.getArray();
        for (short nRow = 0; nRow < nRowCount; nRow++)
            pAry[nRow] = pMemChart->GetRowText(nRow);

        delete pMemChart;
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>(0);
}

void SAL_CALL ScCellRangesBase::setRowDescriptions(
                        const uno::Sequence<rtl::OUString>& aRowDescriptions )
                                                throw(uno::RuntimeException)
{
    //! ist das ernst gemeint?
    DBG_ERROR("not implemented");
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellRangesBase::getColumnDescriptions()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SchMemChart* pMemChart = CreateMemChart_Impl();
    if ( pMemChart )
    {
        short nColCount = pMemChart->GetColCount();
        uno::Sequence<rtl::OUString> aSeq( nColCount );
        rtl::OUString* pAry = aSeq.getArray();
        for (short nCol = 0; nCol < nColCount; nCol++)
            pAry[nCol] = pMemChart->GetColText(nCol);

        delete pMemChart;
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>(0);
}

void SAL_CALL ScCellRangesBase::setColumnDescriptions(
                        const uno::Sequence<rtl::OUString>& aColumnDescriptions )
                                                throw(uno::RuntimeException)
{
    //! ist das ernst gemeint?
    DBG_ERROR("not implemented");
}

String lcl_UniqueName( StrCollection& rColl, const String& rPrefix )
{
    long nNumber = 1;
    USHORT nCollCount = rColl.GetCount();
    while (TRUE)
    {
        String aName = rPrefix;
        aName += String::CreateFromInt32( nNumber );
        BOOL bFound = FALSE;
        for (USHORT i=0; i<nCollCount; i++)
            if ( rColl[i]->GetString() == aName )
            {
                bFound = TRUE;
                break;
            }
        if (!bFound)
            return aName;
        ++nNumber;
    }
}

void SAL_CALL ScCellRangesBase::addChartDataChangeEventListener( const uno::Reference<
                                    chart::XChartDataChangeEventListener >& aListener )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell && aRanges.Count() )
    {
        //! auf doppelte testen?

        ScDocument* pDoc = pDocShell->GetDocument();
        ScRangeListRef aRangesRef( new ScRangeList(aRanges) );
        ScChartListenerCollection* pColl = pDoc->GetChartListenerCollection();
        String aName = lcl_UniqueName( *pColl,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("__Uno")) );
        ScChartListener* pListener = new ScChartListener( aName, pDoc, aRangesRef );
        pListener->SetUno( aListener, this );
        pColl->Insert( pListener );
        pListener->StartListeningTo();
    }
}

void SAL_CALL ScCellRangesBase::removeChartDataChangeEventListener( const uno::Reference<
                                    chart::XChartDataChangeEventListener >& aListener )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell && aRanges.Count() )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScChartListenerCollection* pColl = pDoc->GetChartListenerCollection();
        pColl->FreeUno( aListener, this );
    }
}

double SAL_CALL ScCellRangesBase::getNotANumber() throw(::com::sun::star::uno::RuntimeException)
{
    //  im ScChartArray wird DBL_MIN verwendet, weil das Chart es so will
    return DBL_MIN;
}

sal_Bool SAL_CALL ScCellRangesBase::isNotANumber( double nNumber ) throw(uno::RuntimeException)
{
    //  im ScChartArray wird DBL_MIN verwendet, weil das Chart es so will
    return (nNumber == DBL_MIN);
}

// XCellRangesQuery

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryVisibleCells()
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        //! fuer alle Tabellen getrennt, wenn Markierungen pro Tabelle getrennt sind!
        USHORT nTab = lcl_FirstTab(aRanges);

        ScMarkData aMarkData;
        aMarkData.MarkFromRangeList( aRanges, FALSE );

        ScDocument* pDoc = pDocShell->GetDocument();
        for (USHORT nCol=0; nCol<=MAXCOL; nCol++)
            if (pDoc->GetColFlags(nCol,nTab) & CR_HIDDEN)
                aMarkData.SetMultiMarkArea( ScRange( nCol,0,nTab, nCol,MAXROW,nTab ), FALSE );

        //! nur bis zur letzten selektierten Zeile testen?
        for (USHORT nRow=0; nRow<=MAXROW; nRow++)
            if (pDoc->GetRowFlags(nRow,nTab) & CR_HIDDEN)
            {
                USHORT nHiddenCount = pDoc->GetHiddenRowCount( nRow, nTab );
                DBG_ASSERT(nHiddenCount, "huch?");
                USHORT nLast = nRow + nHiddenCount - 1;
                aMarkData.SetMultiMarkArea( ScRange( 0,nRow,nTab, MAXCOL,nLast,nTab ), FALSE );
                nRow = nLast;   // +1 wird hinterher addiert
            }

        ScRangeList aNewRanges;
        aMarkData.FillRangeListWithMarks( &aNewRanges, FALSE );
        return new ScCellRangesObj( pDocShell, aNewRanges );
    }

    return NULL;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryEmptyCells()
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        ScMarkData aMarkData;
        aMarkData.MarkFromRangeList( aRanges, FALSE );

        //  belegte Zellen wegmarkieren
        ULONG nCount = aRanges.Count();
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange = *aRanges.GetObject(i);

            ScCellIterator aIter( pDoc, aRange );
            ScBaseCell* pCell = aIter.GetFirst();
            while (pCell)
            {
                //  Notizen zaehlen als nicht-leer
                if ( pCell->GetCellType() != CELLTYPE_NOTE || pCell->GetNotePtr() )
                    aMarkData.SetMultiMarkArea(
                            ScRange( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() ),
                            FALSE );

                pCell = aIter.GetNext();
            }
        }

        ScRangeList aNewRanges;
        //  IsMultiMarked reicht hier nicht (wird beim deselektieren nicht zurueckgesetzt)
        if (aMarkData.HasAnyMultiMarks())
            aMarkData.FillRangeListWithMarks( &aNewRanges, FALSE );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges kann leer sein
    }

    return NULL;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryContentCells(
                                                    sal_Int16 nContentFlags )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        ScMarkData aMarkData;

        //  passende Zellen selektieren
        ULONG nCount = aRanges.Count();
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange = *aRanges.GetObject(i);

            ScCellIterator aIter( pDoc, aRange );
            ScBaseCell* pCell = aIter.GetFirst();
            while (pCell)
            {
                BOOL bAdd = FALSE;
                if ( pCell->GetNotePtr() && ( nContentFlags & sheet::CellFlags::ANNOTATION ) )
                    bAdd = TRUE;
                else
                    switch ( pCell->GetCellType() )
                    {
                        case CELLTYPE_STRING:
                        case CELLTYPE_EDIT:
                            if ( nContentFlags & sheet::CellFlags::STRING )
                                bAdd = TRUE;
                            break;
                        case CELLTYPE_FORMULA:
                            if ( nContentFlags & sheet::CellFlags::FORMULA )
                                bAdd = TRUE;
                            break;
                        case CELLTYPE_VALUE:
                            if ( (nContentFlags & (sheet::CellFlags::VALUE|sheet::CellFlags::DATETIME))
                                    == (sheet::CellFlags::VALUE|sheet::CellFlags::DATETIME) )
                                bAdd = TRUE;
                            else
                            {
                                //  Date/Time Erkennung

                                ULONG nIndex = (ULONG)((SfxUInt32Item*)pDoc->GetAttr(
                                        aIter.GetCol(), aIter.GetRow(), aIter.GetTab(),
                                        ATTR_VALUE_FORMAT ))->GetValue();
                                short nTyp = pDoc->GetFormatTable()->GetType(nIndex);
                                if ((nTyp == NUMBERFORMAT_DATE) || (nTyp == NUMBERFORMAT_TIME) ||
                                    (nTyp == NUMBERFORMAT_DATETIME))
                                {
                                    if ( nContentFlags & sheet::CellFlags::DATETIME )
                                        bAdd = TRUE;
                                }
                                else
                                {
                                    if ( nContentFlags & sheet::CellFlags::VALUE )
                                        bAdd = TRUE;
                                }
                            }
                            break;
                    }

                if (bAdd)
                    aMarkData.SetMultiMarkArea(
                            ScRange( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() ),
                            TRUE );

                pCell = aIter.GetNext();
            }
        }

        ScRangeList aNewRanges;
        if (aMarkData.IsMultiMarked())
            aMarkData.FillRangeListWithMarks( &aNewRanges, FALSE );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges kann leer sein
    }

    return NULL;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryFormulaCells(
                                                    sal_Int32 nResultFlags )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        ScMarkData aMarkData;

        //  passende Zellen selektieren
        ULONG nCount = aRanges.Count();
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange = *aRanges.GetObject(i);

            ScCellIterator aIter( pDoc, aRange );
            ScBaseCell* pCell = aIter.GetFirst();
            while (pCell)
            {
                if (pCell->GetCellType() == CELLTYPE_FORMULA)
                {
                    ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                    BOOL bAdd = FALSE;
                    if (pFCell->GetErrCode())
                    {
                        if ( nResultFlags & sheet::FormulaResult::ERROR )
                            bAdd = TRUE;
                    }
                    else if (pFCell->IsValue())
                    {
                        if ( nResultFlags & sheet::FormulaResult::VALUE )
                            bAdd = TRUE;
                    }
                    else    // String
                    {
                        if ( nResultFlags & sheet::FormulaResult::STRING )
                            bAdd = TRUE;
                    }

                    if (bAdd)
                        aMarkData.SetMultiMarkArea(
                                ScRange( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() ),
                                TRUE );
                }

                pCell = aIter.GetNext();
            }
        }

        ScRangeList aNewRanges;
        if (aMarkData.IsMultiMarked())
            aMarkData.FillRangeListWithMarks( &aNewRanges, FALSE );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges kann leer sein
    }

    return NULL;
}

uno::Reference<sheet::XSheetCellRanges> ScCellRangesBase::QueryDifferences_Impl(
                        const table::CellAddress& aCompare, BOOL bColumnDiff)
{
    if (pDocShell)
    {
        ULONG nRangeCount = aRanges.Count();
        ULONG i;
        ScDocument* pDoc = pDocShell->GetDocument();
        ScMarkData aMarkData;

        USHORT nCmpPos = bColumnDiff ? aCompare.Row : aCompare.Column;

        //  zuerst alles selektieren, wo ueberhaupt etwas in der Vergleichsspalte steht
        //  (fuer gleiche Zellen wird die Selektion im zweiten Schritt aufgehoben)

        USHORT nTab = lcl_FirstTab(aRanges);    //! fuer alle Tabellen, wenn Markierungen pro Tabelle!
        ScRange aCmpRange, aCellRange;
        if (bColumnDiff)
            aCmpRange = ScRange( 0,nCmpPos,nTab, MAXCOL,nCmpPos,nTab );
        else
            aCmpRange = ScRange( nCmpPos,0,nTab, nCmpPos,MAXROW,nTab );
        ScCellIterator aCmpIter( pDoc, aCmpRange );
        ScBaseCell* pCmpCell = aCmpIter.GetFirst();
        while (pCmpCell)
        {
            if (pCmpCell->GetCellType() != CELLTYPE_NOTE)
            {
                USHORT nCellPos = bColumnDiff ? aCmpIter.GetCol() : aCmpIter.GetRow();
                if (bColumnDiff)
                    aCellRange = ScRange( nCellPos,0,nTab, nCellPos,MAXROW,nTab );
                else
                    aCellRange = ScRange( 0,nCellPos,nTab, MAXCOL,nCellPos,nTab );

                for (i=0; i<nRangeCount; i++)
                {
                    ScRange aRange = *aRanges.GetObject(i);
                    if ( aRange.Intersects( aCellRange ) )
                    {
                        if (bColumnDiff)
                        {
                            aRange.aStart.SetCol(nCellPos);
                            aRange.aEnd.SetCol(nCellPos);
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
            pCmpCell = aCmpIter.GetNext();
        }

        //  alle nichtleeren Zellen mit der Vergleichsspalte vergleichen und entsprechend
        //  selektieren oder aufheben

        ScAddress aCmpAddr;
        for (i=0; i<nRangeCount; i++)
        {
            ScRange aRange = *aRanges.GetObject(i);

            ScCellIterator aIter( pDoc, aRange );
            ScBaseCell* pCell = aIter.GetFirst();
            while (pCell)
            {
                if (bColumnDiff)
                    aCmpAddr = ScAddress( aIter.GetCol(), nCmpPos, aIter.GetTab() );
                else
                    aCmpAddr = ScAddress( nCmpPos, aIter.GetRow(), aIter.GetTab() );
                const ScBaseCell* pCmpCell = pDoc->GetCell( aCmpAddr );

                ScRange aCellRange( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() );
                if ( !ScBaseCell::CellEqual( pCell, pCmpCell ) )
                    aMarkData.SetMultiMarkArea( aCellRange );
                else
                    aMarkData.SetMultiMarkArea( aCellRange, FALSE );    // aufheben

                pCell = aIter.GetNext();
            }
        }

        ScRangeList aNewRanges;
        if (aMarkData.IsMultiMarked())
            aMarkData.FillRangeListWithMarks( &aNewRanges, FALSE );

        return new ScCellRangesObj( pDocShell, aNewRanges );    // aNewRanges kann leer sein
    }
    return NULL;
}

uno::Reference<sheet::XSheetCellRanges > SAL_CALL ScCellRangesBase::queryColumnDifferences(
                            const table::CellAddress& aCompare ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return QueryDifferences_Impl( aCompare, TRUE );
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryRowDifferences(
                            const table::CellAddress& aCompare ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return QueryDifferences_Impl( aCompare, FALSE );
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryIntersection(
                            const table::CellRangeAddress& aRange ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScRange aMask( aRange.StartColumn, aRange.StartRow, aRange.Sheet,
                   aRange.EndColumn,   aRange.EndRow,   aRange.Sheet );

    ScRangeList aNew;
    ULONG nCount = aRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScRange aTemp = *aRanges.GetObject(i);
        if ( aTemp.Intersects( aMask ) )
            aNew.Join( ScRange( Max( aTemp.aStart.Col(), aMask.aStart.Col() ),
                                Max( aTemp.aStart.Row(), aMask.aStart.Row() ),
                                Max( aTemp.aStart.Tab(), aMask.aStart.Tab() ),
                                Min( aTemp.aEnd.Col(), aMask.aEnd.Col() ),
                                Min( aTemp.aEnd.Row(), aMask.aEnd.Row() ),
                                Min( aTemp.aEnd.Tab(), aMask.aEnd.Tab() ) ) );
    }

    return new ScCellRangesObj( pDocShell, aNew );  // kann leer sein
}

// XFormulaQuery

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryPrecedents(
                                sal_Bool bRecursive ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        ScRangeList aNewRanges = aRanges;
        BOOL bFound;
        do
        {
            bFound = FALSE;

            ScMarkData aMarkData;
            aMarkData.MarkFromRangeList( aNewRanges, FALSE );

            ULONG nCount = aNewRanges.Count();
            for (USHORT nR=0; nR<nCount; nR++)
            {
                ScRange aRange = *aNewRanges.GetObject(nR);
                ScCellIterator aIter( pDoc, aRange );
                ScBaseCell* pCell = aIter.GetFirst();
                while (pCell)
                {
                    if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                    {
                        ScFormulaCell* pFCell = (ScFormulaCell*) pCell;

                        ScDetectiveRefIter aIter( pFCell );
                        ScTripel aRefStart;
                        ScTripel aRefEnd;
                        while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
                        {
                            ScRange aRefRange( aRefStart, aRefEnd );
                            if ( bRecursive && !bFound && !aMarkData.IsAllMarked( aRefRange ) )
                                bFound = TRUE;
                            aMarkData.SetMultiMarkArea( aRefRange, TRUE );
                        }
                    }
                    pCell = aIter.GetNext();
                }
            }

            aMarkData.FillRangeListWithMarks( &aNewRanges, TRUE );
        }
        while ( bRecursive && bFound );

        return new ScCellRangesObj( pDocShell, aNewRanges );
    }

    return NULL;
}

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryDependents(
                                sal_Bool bRecursive ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        ScRangeList aNewRanges = aRanges;
        BOOL bFound;
        do
        {
            bFound = FALSE;
            ULONG nRangesCount = aNewRanges.Count();

            ScMarkData aMarkData;
            aMarkData.MarkFromRangeList( aNewRanges, FALSE );

            USHORT nTab = lcl_FirstTab(aNewRanges);                 //! alle Tabellen

            ScCellIterator aCellIter( pDoc, 0,0, nTab, MAXCOL,MAXROW, nTab );
            ScBaseCell* pCell = aCellIter.GetFirst();
            while (pCell)
            {
                if (pCell->GetCellType() == CELLTYPE_FORMULA)
                {
                    BOOL bMark = FALSE;
                    ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
                    ScTripel aRefStart;
                    ScTripel aRefEnd;
                    while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
                    {
                        ScRange aRefRange( aRefStart, aRefEnd );
                        for (USHORT nR=0; nR<nRangesCount; nR++)
                        {
                            ScRange aRange = *aNewRanges.GetObject(nR);
                            if (aRange.Intersects(aRefRange))
                                bMark = TRUE;                   // von Teil des Ranges abhaengig
                        }
                    }
                    if (bMark)
                    {
                        ScRange aCellRange( aCellIter.GetCol(),
                                            aCellIter.GetRow(),
                                            aCellIter.GetTab() );
                        if ( bRecursive && !bFound && !aMarkData.IsAllMarked( aCellRange ) )
                            bFound = TRUE;
                        aMarkData.SetMultiMarkArea( aCellRange, TRUE );
                    }
                }
                pCell = aCellIter.GetNext();
            }

            aMarkData.FillRangeListWithMarks( &aNewRanges, TRUE );
        }
        while ( bRecursive && bFound );

        return new ScCellRangesObj( pDocShell, aNewRanges );
    }

    return NULL;
}

// XSearchable

uno::Reference<util::XSearchDescriptor> SAL_CALL ScCellRangesBase::createSearchDescriptor()
                                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScCellSearchObj;
}

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangesBase::findAll(
                        const uno::Reference<util::XSearchDescriptor>& xDesc )
                                                    throw(uno::RuntimeException)
{
    //  Wenn nichts gefunden wird, soll Null zurueckgegeben werden (?)
    uno::Reference<container::XIndexAccess> xRet;
    if ( pDocShell && xDesc.is() )
    {
        ScCellSearchObj* pSearch = ScCellSearchObj::getImplementation( xDesc );
        if (pSearch)
        {
            SvxSearchItem* pSearchItem = pSearch->GetSearchItem();
            if (pSearchItem)
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                pSearchItem->SetCommand( SVX_SEARCHCMD_FIND_ALL );
                //  immer nur innerhalb dieses Objekts
                pSearchItem->SetSelection( !lcl_WholeSheet(aRanges) );

                ScMarkData aMark;
                aMark.MarkFromRangeList( aRanges, FALSE );

                String aDummyUndo;
                USHORT nCol = 0, nRow = 0, nTab = 0;
                BOOL bFound = pDoc->SearchAndReplace( *pSearchItem, nCol, nRow, nTab,
                                                        aMark, aDummyUndo, NULL );
                if (bFound)
                {
                    ScRangeList aNewRanges;
                    aMark.FillRangeListWithMarks( &aNewRanges, TRUE );
                    //  bei findAll immer CellRanges, egal wieviel gefunden wurde
                    xRet = new ScCellRangesObj( pDocShell, aNewRanges );
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
        ScCellSearchObj* pSearch = ScCellSearchObj::getImplementation( xDesc );
        if (pSearch)
        {
            SvxSearchItem* pSearchItem = pSearch->GetSearchItem();
            if (pSearchItem)
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                pSearchItem->SetCommand( SVX_SEARCHCMD_FIND );
                //  immer nur innerhalb dieses Objekts
                pSearchItem->SetSelection( !lcl_WholeSheet(aRanges) );

                ScMarkData aMark;
                aMark.MarkFromRangeList( aRanges, FALSE );

                USHORT nCol, nRow, nTab;
                if (pLastPos)
                    pLastPos->GetVars( nCol, nRow, nTab );
                else
                {
                    nTab = lcl_FirstTab(aRanges);   //! mehrere Tabellen?
                    ScDocument::GetSearchAndReplaceStart( *pSearchItem, nCol, nRow );
                }

                String aDummyUndo;
                BOOL bFound = pDoc->SearchAndReplace( *pSearchItem, nCol, nRow, nTab,
                                                        aMark, aDummyUndo, NULL );
                if (bFound)
                {
                    ScAddress aFoundPos( nCol, nRow, nTab );
                    xRet = (cppu::OWeakObject*) new ScCellObj( pDocShell, aFoundPos );
                }
            }
        }
    }
    return xRet;
}

uno::Reference<uno::XInterface> SAL_CALL ScCellRangesBase::findFirst(
                        const uno::Reference<util::XSearchDescriptor>& xDesc )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return Find_Impl( xDesc, NULL );
}

uno::Reference<uno::XInterface> SAL_CALL ScCellRangesBase::findNext(
                        const uno::Reference<uno::XInterface>& xStartAt,
                        const uno::Reference<util::XSearchDescriptor >& xDesc )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( xStartAt.is() )
    {
        ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( xStartAt );
        if ( pRangesImp && pRangesImp->GetDocShell() == pDocShell )
        {
            const ScRangeList& rStartRanges = pRangesImp->GetRangeList();
            if ( rStartRanges.Count() == 1 )
            {
                ScAddress aStartPos = rStartRanges.GetObject(0)->aStart;
                return Find_Impl( xDesc, &aStartPos );
            }
        }
    }
    return NULL;
}

// XReplaceable

uno::Reference<util::XReplaceDescriptor> SAL_CALL ScCellRangesBase::createReplaceDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScCellSearchObj;
}

sal_Int32 SAL_CALL ScCellRangesBase::replaceAll( const uno::Reference<util::XSearchDescriptor>& xDesc )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    INT32 nReplaced = 0;
    if ( pDocShell && xDesc.is() )
    {
        ScCellSearchObj* pSearch = ScCellSearchObj::getImplementation( xDesc );
        if (pSearch)
        {
            SvxSearchItem* pSearchItem = pSearch->GetSearchItem();
            if (pSearchItem)
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                pSearchItem->SetCommand( SVX_SEARCHCMD_REPLACE_ALL );
                //  immer nur innerhalb dieses Objekts
                pSearchItem->SetSelection( !lcl_WholeSheet(aRanges) );

                ScMarkData aMark;
                aMark.MarkFromRangeList( aRanges, FALSE );
                USHORT i;

                USHORT nTabCount = pDoc->GetTableCount();
                BOOL bProtected = !pDocShell->IsEditable();
                for (i=0; i<nTabCount; i++)
                    if ( aMark.GetTableSelect(i) && pDoc->IsTabProtected(i) )
                        bProtected = TRUE;
                if (bProtected)
                {
                    //! Exception, oder was?
                }
                else
                {
                    USHORT nTab = aMark.GetFirstSelected();     // bei SearchAndReplace nicht benutzt
                    USHORT nCol = 0, nRow = 0;

                    String aUndoStr;
                    ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                    pUndoDoc->InitUndo( pDoc, nTab, nTab );
                    for (i=0; i<nTabCount; i++)
                        if ( aMark.GetTableSelect(i) && i != nTab )
                            pUndoDoc->AddUndoTab( i, i );
                    ScMarkData* pUndoMark = new ScMarkData(aMark);

                    BOOL bFound = pDoc->SearchAndReplace( *pSearchItem, nCol, nRow, nTab,
                                                            aMark, aUndoStr, pUndoDoc );
                    if (bFound)
                    {
                        nReplaced = pUndoDoc->GetCellCount();

                        pDocShell->GetUndoManager()->AddUndoAction(
                            new ScUndoReplace( pDocShell, *pUndoMark, nCol, nRow, nTab,
                                                        aUndoStr, pUndoDoc, pSearchItem ) );

                        pDocShell->PostPaintGridAll();
                        pDocShell->SetDocumentModified();
                    }
                    else
                    {
                        delete pUndoDoc;
                        delete pUndoMark;
                        // nReplaced bleibt 0
                    }
                }
            }
        }
    }
    return nReplaced;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScCellRangesBase::getSomething(
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
const uno::Sequence<sal_Int8>& ScCellRangesBase::getUnoTunnelId()
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
ScCellRangesBase* ScCellRangesBase::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScCellRangesBase* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScCellRangesBase*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScCellRangesObj::ScCellRangesObj(ScDocShell* pDocSh, const ScRangeList& rR) :
    ScCellRangesBase( pDocSh, rR )
{
}

ScCellRangesObj::~ScCellRangesObj()
{
    List                    aNamedEntries;
}

void ScCellRangesObj::RefChanged()
{
    ScCellRangesBase::RefChanged();

    //  nix weiter...
}

uno::Any SAL_CALL ScCellRangesObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XSheetCellRanges )
    SC_QUERYINTERFACE( container::XIndexAccess )
    SC_QUERY_MULTIPLE( container::XElementAccess, container::XIndexAccess )
    SC_QUERYINTERFACE( container::XEnumerationAccess )
    SC_QUERYINTERFACE( container::XNameContainer )
    SC_QUERYINTERFACE( container::XNameReplace )
    SC_QUERYINTERFACE( container::XNameAccess )

    return ScCellRangesBase::queryInterface( rType );
}

void SAL_CALL ScCellRangesObj::acquire() throw(uno::RuntimeException)
{
    ScCellRangesBase::acquire();
}

void SAL_CALL ScCellRangesObj::release() throw(uno::RuntimeException)
{
    ScCellRangesBase::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellRangesObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangesBase::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 3 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XSheetCellRanges>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<container::XNameContainer>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<container::XEnumerationAccess>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellRangesObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XCellRanges

ScCellRangeObj* ScCellRangesObj::GetObjectByIndex_Impl(USHORT nIndex) const
{
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    if ( pDocSh && nIndex < rRanges.Count() )
    {
        ScRange aRange = *rRanges.GetObject(nIndex);
        if ( aRange.aStart == aRange.aEnd )
            return new ScCellObj( pDocSh, aRange.aStart );
        else
            return new ScCellRangeObj( pDocSh, aRange );
    }

    return NULL;        // keine DocShell oder falscher Index
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScCellRangesObj::getRangeAddresses()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    ULONG nCount = rRanges.Count();
    if ( pDocSh && nCount )
    {
        table::CellRangeAddress aRangeAddress;
        uno::Sequence<table::CellRangeAddress> aSeq(nCount);
        table::CellRangeAddress* pAry = aSeq.getArray();
        for (USHORT i=0; i<nCount; i++)
        {
            ScUnoConversion::FillAddress( aRangeAddress, *rRanges.GetObject(i) );
            pAry[i] = aRangeAddress;
        }
        return aSeq;
    }

    return uno::Sequence<table::CellRangeAddress>(0);   // leer ist moeglich
}

uno::Reference<container::XEnumerationAccess> SAL_CALL ScCellRangesObj::getCells()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  getCells with empty range list is possible (no exception),
    //  the resulting enumeration just has no elements
    //  (same behaviour as a valid range with no cells)
    //  This is handled in ScCellsEnumeration ctor.

    const ScRangeList& rRanges = GetRangeList();
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScCellsObj( pDocSh, rRanges );
    return NULL;
}

rtl::OUString SAL_CALL ScCellRangesObj::getRangeAddressesAsString()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    if (pDocSh)
        rRanges.Format( aString, SCA_VALID | SCA_TAB_3D, pDocSh->GetDocument() );
    return aString;
}

// XNameContainer

void lcl_RemoveNamedEntry( ScNamedEntryArr_Impl& rNamedEntries, const String& rName )
{
    USHORT nCount = rNamedEntries.Count();
    for ( USHORT n=nCount; n--; )
        if ( rNamedEntries[n]->GetName() == rName )
            rNamedEntries.DeleteAndDestroy( n );
}

void SAL_CALL ScCellRangesObj::insertByName( const rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    BOOL bDone = FALSE;

    //! Type of aElement can be some specific interface instead of XInterface

    uno::Reference<uno::XInterface> xInterface;
    if ( pDocSh && ( aElement >>= xInterface ) )
    {
        ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( xInterface );
        if ( pRangesImp && pRangesImp->GetDocShell() == pDocSh )
        {
            ScRangeList aNew = GetRangeList();
            const ScRangeList& rAddRanges = pRangesImp->GetRangeList();
            ULONG nAddCount = rAddRanges.Count();
            for (ULONG i=0; i<nAddCount; i++)
                aNew.Join( *rAddRanges.GetObject(i) );
            SetNewRanges(aNew);
            bDone = TRUE;

            if ( aName.getLength() && nAddCount == 1 )
            {
                //  if a name is given, also insert into list of named entries
                //  (only possible for a single range)

                String aNamStr = aName;

                //  remove old entry with this name
                lcl_RemoveNamedEntry( aNamedEntries, aNamStr );

                ScNamedEntry* pEntry = new ScNamedEntry( aNamStr, *rAddRanges.GetObject(0) );
                aNamedEntries.Insert( pEntry, aNamedEntries.Count() );
            }
        }
    }

    if (!bDone)
    {
        //  invalid element - double names are handled above
        throw lang::IllegalArgumentException();
    }
}

BOOL lcl_FindRangeByName( const ScRangeList& rRanges, ScDocShell* pDocSh,
                            const String& rName, ULONG& rIndex )
{
    if (pDocSh)
    {
        String aRangeStr;
        ScDocument* pDoc = pDocSh->GetDocument();
        ULONG nCount = rRanges.Count();
        for (ULONG i=0; i<nCount; i++)
        {
            rRanges.GetObject(i)->Format( aRangeStr, SCA_VALID | SCA_TAB_3D, pDoc );
            if ( aRangeStr == rName )
            {
                rIndex = i;
                return TRUE;
            }
        }
    }
    return FALSE;   // nicht gefunden
}

BOOL lcl_FindRangeOrEntry( const ScNamedEntryArr_Impl& rNamedEntries,
                            const ScRangeList& rRanges, ScDocShell* pDocSh,
                            const String& rName, ScRange& rFound )
{
    ULONG nIndex = 0;
    if ( lcl_FindRangeByName( rRanges, pDocSh, rName, nIndex ) )
    {
        rFound = *rRanges.GetObject(nIndex);
        return TRUE;
    }

    if ( rNamedEntries.Count() )
    {
        for ( USHORT n=0; n<rNamedEntries.Count(); n++ )
            if ( rNamedEntries[n]->GetName() == rName )
            {
                //  test if named entry is contained in rRanges

                const ScRange& rComp = rNamedEntries[n]->GetRange();
                ScMarkData aMarkData;
                aMarkData.MarkFromRangeList( rRanges, FALSE );
                if ( aMarkData.IsAllMarked( rComp ) )
                {
                    rFound = rComp;
                    return TRUE;
                }
            }
    }

    return FALSE;       // not found
}

void SAL_CALL ScCellRangesObj::removeByName( const rtl::OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    String aNameStr = aName;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    ULONG nIndex = 0;
    if ( lcl_FindRangeByName( rRanges, pDocSh, aNameStr, nIndex ) )
    {
        //  einzelnen Range weglassen
        ScRangeList aNew;
        ULONG nCount = rRanges.Count();
        for (ULONG i=0; i<nCount; i++)
            if (i != nIndex)
                aNew.Append( *rRanges.GetObject(i) );
        SetNewRanges(aNew);
        bDone = TRUE;
    }
    else if (pDocSh)
    {
        //  deselect any ranges (parsed or named entry)
        ScRangeList aDiff;
        BOOL bValid = ( aDiff.Parse( aNameStr, pDocSh->GetDocument() ) & SCA_VALID ) != 0;
        if ( !bValid && aNamedEntries.Count() )
        {
            USHORT nCount = aNamedEntries.Count();
            for (USHORT n=0; n<nCount && !bValid; n++)
                if (aNamedEntries[n]->GetName() == aNameStr)
                {
                    aDiff.RemoveAll();
                    aDiff.Append( aNamedEntries[n]->GetRange() );
                    bValid = TRUE;
                }
        }
        if ( bValid )
        {
            ScMarkData aMarkData;
            aMarkData.MarkFromRangeList( rRanges, FALSE );

            ULONG nDiffCount = aDiff.Count();
            for (ULONG i=0; i<nDiffCount; i++)
            {
                ScRange* pDiffRange = aDiff.GetObject(i);
                if (aMarkData.GetTableSelect( pDiffRange->aStart.Tab() ))
                    aMarkData.SetMultiMarkArea( *pDiffRange, FALSE );
            }

            ScRangeList aNew;
            aMarkData.FillRangeListWithMarks( &aNew, FALSE );
            SetNewRanges(aNew);

            bDone = TRUE;       //! error if range was not selected before?
        }
    }

    if (aNamedEntries.Count())
        lcl_RemoveNamedEntry( aNamedEntries, aNameStr );    //  remove named entry

    if (!bDone)
        throw container::NoSuchElementException();      // not found
}

// XNameReplace

void SAL_CALL ScCellRangesObj::replaceByName( const rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //! zusammenfassen?
    removeByName( aName );
    insertByName( aName, aElement );
}

// XNameAccess

uno::Any SAL_CALL ScCellRangesObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;

    String aNameStr = aName;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    ScRange aRange;
    if ( lcl_FindRangeOrEntry( aNamedEntries, rRanges, pDocSh, aNameStr, aRange ) )
    {
        uno::Reference<table::XCellRange> xRange;
        if ( aRange.aStart == aRange.aEnd )
            xRange = new ScCellObj( pDocSh, aRange.aStart );
        else
            xRange = new ScCellRangeObj( pDocSh, aRange );
        aRet <<= xRange;
    }
    else
        throw container::NoSuchElementException();
    return aRet;
}

BOOL lcl_FindEntryName( const ScNamedEntryArr_Impl& rNamedEntries,
                        const ScRange& rRange, String& rName )
{
    USHORT nCount = rNamedEntries.Count();
    for (USHORT i=0; i<nCount; i++)
        if (rNamedEntries[i]->GetRange() == rRange)
        {
            rName = rNamedEntries[i]->GetName();
            return TRUE;
        }
    return FALSE;
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellRangesObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    if (pDocSh)
    {
        String aRangeStr;
        ScDocument* pDoc = pDocSh->GetDocument();
        ULONG nCount = rRanges.Count();

        uno::Sequence<rtl::OUString> aSeq(nCount);
        rtl::OUString* pAry = aSeq.getArray();
        for (ULONG i=0; i<nCount; i++)
        {
            //  use given name if for exactly this range, otherwise just format
            ScRange aRange = *rRanges.GetObject(i);
            if ( !aNamedEntries.Count() || !lcl_FindEntryName( aNamedEntries, aRange, aRangeStr ) )
                aRange.Format( aRangeStr, SCA_VALID | SCA_TAB_3D, pDoc );
            pAry[i] = aRangeStr;
        }
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScCellRangesObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameStr = aName;
    ScDocShell* pDocSh = GetDocShell();
    const ScRangeList& rRanges = GetRangeList();
    ScRange aRange;
    return lcl_FindRangeOrEntry( aNamedEntries, rRanges, pDocSh, aNameStr, aRange );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScCellRangesObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScCellRangesObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    return rRanges.Count();
}

uno::Any SAL_CALL ScCellRangesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<table::XCellRange> xRange = GetObjectByIndex_Impl((USHORT)nIndex);
    uno::Any aAny;
    if (xRange.is())
        aAny <<= xRange;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScCellRangesObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<table::XCellRange>*)0);
}

sal_Bool SAL_CALL ScCellRangesObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    return rRanges.Count() != 0;
}

// XServiceInfo

rtl::OUString SAL_CALL ScCellRangesObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScCellRangesObj" );
}

sal_Bool SAL_CALL ScCellRangesObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr(rServiceName);
    return aServiceStr.EqualsAscii( SCSHEETCELLRANGES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellRangesObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(3);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCSHEETCELLRANGES_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[2] = rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    return aRet;
}

//------------------------------------------------------------------------

// static
uno::Reference<table::XCellRange> ScCellRangeObj::CreateRangeFromDoc( ScDocument* pDoc, const ScRange& rR )
{
    SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
    if ( pObjSh && pObjSh->ISA(ScDocShell) )
        return new ScCellRangeObj( (ScDocShell*) pObjSh, rR );
    return NULL;
}

//------------------------------------------------------------------------

ScCellRangeObj::ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR) :
    ScCellRangesBase( pDocSh, rR ),
    aRangePropSet( lcl_GetRangePropertyMap() ),
    aRange( rR )
{
    aRange.Justify();       // Anfang / Ende richtig
}

ScCellRangeObj::~ScCellRangeObj()
{
}

void ScCellRangeObj::RefChanged()
{
    ScCellRangesBase::RefChanged();

    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT(rRanges.Count() == 1, "was fuer Ranges ?!?!");
    const ScRange* pFirst = rRanges.GetObject(0);
    if (pFirst)
    {
        aRange = *pFirst;
        aRange.Justify();
    }
}

uno::Any SAL_CALL ScCellRangeObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XCellRangeAddressable )
    SC_QUERYINTERFACE( table::XCellRange )
    SC_QUERYINTERFACE( sheet::XSheetCellRange )
    SC_QUERYINTERFACE( sheet::XArrayFormulaRange )
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

    return ScCellRangesBase::queryInterface( rType );
}

void SAL_CALL ScCellRangeObj::acquire() throw(uno::RuntimeException)
{
    ScCellRangesBase::acquire();
}

void SAL_CALL ScCellRangeObj::release() throw(uno::RuntimeException)
{
    ScCellRangesBase::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellRangeObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangesBase::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 13 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XCellRangeAddressable>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<sheet::XSheetCellRange>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<sheet::XArrayFormulaRange>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<sheet::XMultipleOperation>*)0);
        pPtr[nParentLen + 4] = getCppuType((const uno::Reference<util::XMergeable>*)0);
        pPtr[nParentLen + 5] = getCppuType((const uno::Reference<sheet::XCellSeries>*)0);
        pPtr[nParentLen + 6] = getCppuType((const uno::Reference<table::XAutoFormattable>*)0);
        pPtr[nParentLen + 7] = getCppuType((const uno::Reference<util::XSortable>*)0);
        pPtr[nParentLen + 8] = getCppuType((const uno::Reference<sheet::XSheetFilterableEx>*)0);
        pPtr[nParentLen + 9] = getCppuType((const uno::Reference<sheet::XSubTotalCalculatable>*)0);
        pPtr[nParentLen +10] = getCppuType((const uno::Reference<table::XColumnRowRange>*)0);
        pPtr[nParentLen +11] = getCppuType((const uno::Reference<util::XImportable>*)0);
        pPtr[nParentLen +12] = getCppuType((const uno::Reference<sheet::XCellFormatRangesSupplier>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellRangeObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XCellRange

//  ColumnCount / RowCount sind weggefallen
//! werden im Writer fuer Tabellen noch gebraucht ???

uno::Reference<table::XCell> SAL_CALL ScCellRangeObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( nColumn >= 0 && nRow >= 0 )
    {
        ScDocShell* pDocSh = GetDocShell();
        sal_Int32 nPosX = aRange.aStart.Col() + nColumn;
        sal_Int32 nPosY = aRange.aStart.Row() + nRow;

        if ( pDocSh && nPosX <= aRange.aEnd.Col() && nPosY <= aRange.aEnd.Row() )
        {
            ScAddress aNew( nPosX, nPosY, aRange.aStart.Tab() );
            return new ScCellObj( pDocSh, aNew );
        }
    }

    throw uno::RuntimeException();
    return NULL;
}

uno::Reference<table::XCellRange> SAL_CALL ScCellRangeObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( nLeft >= 0 && nTop >= 0 && nRight >= 0 && nBottom >= 0 )
    {
        ScDocShell* pDocSh = GetDocShell();
        sal_Int32 nStartX = aRange.aStart.Col() + nLeft;
        sal_Int32 nStartY = aRange.aStart.Row() + nTop;
        sal_Int32 nEndX = aRange.aStart.Col() + nRight;
        sal_Int32 nEndY = aRange.aStart.Row() + nBottom;

        if ( pDocSh && nStartX <= nEndX && nEndX <= aRange.aEnd.Col() &&
                       nStartY <= nEndY && nEndY <= aRange.aEnd.Row() )
        {
            ScRange aNew( nStartX, nStartY, aRange.aStart.Tab(), nEndX, nEndY, aRange.aEnd.Tab() );
            return new ScCellRangeObj( pDocSh, aNew );
        }
    }

    throw uno::RuntimeException();
    return NULL;
}

uno::Reference<table::XCellRange> SAL_CALL ScCellRangeObj::getCellRangeByName(
                        const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    //  name refers to the whole document (with the range's table as default),
    //  valid only if the range is within this range

    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = aRange.aStart.Tab();

        ScRange aCellRange;
        BOOL bFound = FALSE;
        String aString = aName;
        USHORT nParse = aCellRange.ParseAny( aString, pDoc );
        if ( nParse & SCA_VALID )
        {
            if ( !(nParse & SCA_TAB_3D) )   // keine Tabelle angegeben -> auf dieser Tabelle
            {
                aCellRange.aStart.SetTab(nTab);
                aCellRange.aEnd.SetTab(nTab);
            }
            bFound = TRUE;
        }
        else
        {
            ScRangeUtil aRangeUtil;
            if ( aRangeUtil.MakeRangeFromName( aString, pDoc, nTab, aCellRange, RUTL_NAMES ) ||
                 aRangeUtil.MakeRangeFromName( aString, pDoc, nTab, aCellRange, RUTL_DBASE ) )
                bFound = TRUE;
        }

        if (bFound)         // valid only if within this object's range
        {
            if (!aRange.In(aCellRange))
                bFound = FALSE;
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
    return NULL;
}

// XColumnRowRange

uno::Reference<table::XTableColumns> SAL_CALL ScCellRangeObj::getColumns() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScTableColumnsObj( pDocSh, aRange.aStart.Tab(),
                                        aRange.aStart.Col(), aRange.aEnd.Col() );

    DBG_ERROR("Dokument ungueltig");
    return NULL;
}

uno::Reference<table::XTableRows> SAL_CALL ScCellRangeObj::getRows() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScTableRowsObj( pDocSh, aRange.aStart.Tab(),
                                    aRange.aStart.Row(), aRange.aEnd.Row() );

    DBG_ERROR("Dokument ungueltig");
    return NULL;
}

// XAddressableCellRange

table::CellRangeAddress SAL_CALL ScCellRangeObj::getRangeAddress() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aRet;
    ScUnoConversion::FillAddress( aRet, aRange );
    return aRet;
}

// XSheetCellRange

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScCellRangeObj::getSpreadsheet()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScTableSheetObj( pDocSh, aRange.aStart.Tab() );

    DBG_ERROR("Dokument ungueltig");
    return NULL;
}

// XFormulaArray

rtl::OUString SAL_CALL ScCellRangeObj::getArrayFormula() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  Matrix-Formel, wenn eindeutig Teil einer Matrix,
    //  also wenn Anfang und Ende des Blocks zur selben Matrix gehoeren.
    //  Sonst Leerstring.

    String aFormula;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        const ScBaseCell* pCell1 = pDoc->GetCell( aRange.aStart );
        const ScBaseCell* pCell2 = pDoc->GetCell( aRange.aEnd );
        if ( pCell1 && pCell2 && pCell1->GetCellType() == CELLTYPE_FORMULA &&
                                 pCell2->GetCellType() == CELLTYPE_FORMULA )
        {
            const ScFormulaCell* pFCell1 = (const ScFormulaCell*)pCell1;
            const ScFormulaCell* pFCell2 = (const ScFormulaCell*)pCell2;
            ScAddress aStart1;
            ScAddress aStart2;
            if ( pFCell1->GetMatrixOrigin( aStart1 ) && pFCell2->GetMatrixOrigin( aStart2 ) )
            {
                if ( aStart1 == aStart2 )               // beides dieselbe Matrix
                    pFCell1->GetFormula( aFormula );    // egal, von welcher Zelle
            }
        }
    }
    return aFormula;
}

void SAL_CALL ScCellRangeObj::setArrayFormula( const rtl::OUString& aFormula )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        String aString = aFormula;
        ScDocFunc aFunc(*pDocSh);
        if ( aString.Len() )
        {
            if ( ScTableSheetObj::getImplementation( (cppu::OWeakObject*)this ) )
            {
                //  #74681# don't set array formula for sheet object
                throw uno::RuntimeException();
            }

            aFunc.EnterMatrix( aRange, NULL, aString, TRUE );
        }
        else
        {
            //  empty string -> erase array formula
            ScMarkData aMark;
            aMark.SetMarkArea( aRange );
            aMark.SelectTable( aRange.aStart.Tab(), TRUE );
            aFunc.DeleteContents( aMark, IDF_CONTENTS, TRUE, TRUE );
        }
    }
}

// XMultipleOperation

void SAL_CALL ScCellRangeObj::setTableOperation( const table::CellRangeAddress& aFormulaRange,
                                        sheet::TableOperationMode nMode,
                                        const table::CellAddress& aColumnCell,
                                        const table::CellAddress& aRowCell )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        BOOL bError = FALSE;
        ScTabOpParam aParam;
        aParam.aRefFormulaCell = ScRefTripel( aFormulaRange.StartColumn,
                                              aFormulaRange.StartRow, aFormulaRange.Sheet,
                                              FALSE, FALSE, FALSE );
        aParam.aRefFormulaEnd  = ScRefTripel( aFormulaRange.EndColumn,
                                              aFormulaRange.EndRow, aFormulaRange.Sheet,
                                              FALSE, FALSE, FALSE );
        aParam.aRefRowCell     = ScRefTripel( aRowCell.Column,
                                              aRowCell.Row, aRowCell.Sheet,
                                              FALSE, FALSE, FALSE );
        aParam.aRefColCell     = ScRefTripel( aColumnCell.Column,
                                              aColumnCell.Row, aColumnCell.Sheet,
                                              FALSE, FALSE, FALSE );
        switch (nMode)
        {
            case sheet::TableOperationMode_COLUMN:
                aParam.nMode = 0;
                break;
            case sheet::TableOperationMode_ROW:
                aParam.nMode = 1;
                break;
            case sheet::TableOperationMode_BOTH:
                aParam.nMode = 2;
                break;
            default:
                bError = TRUE;
        }

        if (!bError)
        {
            ScDocFunc aFunc(*pDocSh);
            aFunc.TabOp( aRange, NULL, aParam, TRUE, TRUE );
        }
    }
}

// XMergeable

void SAL_CALL ScCellRangeObj::merge( sal_Bool bMerge ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocFunc aFunc(*pDocSh);
        if ( bMerge )
            aFunc.MergeCells( aRange, FALSE, TRUE, TRUE );
        else
            aFunc.UnmergeCells( aRange, TRUE, TRUE );

        //! Fehler abfangen?
    }
}

sal_Bool SAL_CALL ScCellRangeObj::getIsMerged() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    return pDocSh && pDocSh->GetDocument()->HasAttrib( aRange, HASATTR_MERGED );
}

// XCellSeries

void SAL_CALL ScCellRangeObj::fillSeries( sheet::FillDirection nFillDirection,
                        sheet::FillMode nFillMode, sheet::FillDateMode nFillDateMode,
                        double fStep, double fEndValue ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        BOOL bError = FALSE;

        FillDir eDir;
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
                bError = TRUE;
        }

        FillCmd eCmd;
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
                bError = TRUE;
        }

        FillDateCmd eDateCmd;
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
                bError = TRUE;
        }

        if (!bError)
        {
            ScDocFunc aFunc(*pDocSh);
            aFunc.FillSeries( aRange, NULL, eDir, eCmd, eDateCmd,
                                MAXDOUBLE, fStep, fEndValue, TRUE, TRUE );
        }
    }
}

void SAL_CALL ScCellRangeObj::fillAuto( sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && nSourceCount )
    {
        ScRange aSourceRange = aRange;
        USHORT nCount;                      // "Dest-Count"
        FillDir eDir;
        BOOL bError = FALSE;
        switch (nFillDirection)
        {
            case sheet::FillDirection_TO_BOTTOM:
                aSourceRange.aEnd.SetRow( aSourceRange.aStart.Row() + nSourceCount - 1 );
                nCount = aRange.aEnd.Row() - aSourceRange.aEnd.Row();
                eDir = FILL_TO_BOTTOM;
                break;
            case sheet::FillDirection_TO_RIGHT:
                aSourceRange.aEnd.SetCol( aSourceRange.aStart.Col() + nSourceCount - 1 );
                nCount = aRange.aEnd.Col() - aSourceRange.aEnd.Col();
                eDir = FILL_TO_RIGHT;
                break;
            case sheet::FillDirection_TO_TOP:
                aSourceRange.aStart.SetRow( aSourceRange.aEnd.Row() - nSourceCount + 1 );
                nCount = aSourceRange.aStart.Row() - aRange.aStart.Row();
                eDir = FILL_TO_TOP;
                break;
            case sheet::FillDirection_TO_LEFT:
                aSourceRange.aStart.SetCol( aSourceRange.aEnd.Col() - nSourceCount + 1 );
                nCount = aSourceRange.aStart.Col() - aRange.aStart.Col();
                eDir = FILL_TO_LEFT;
                break;
            default:
                bError = TRUE;
        }
        if (nCount > MAXROW)        // Ueberlauf
            bError = TRUE;

        if (!bError)
        {
            ScDocFunc aFunc(*pDocSh);
            aFunc.FillAuto( aSourceRange, NULL, eDir, nCount, TRUE, TRUE );
        }
    }
}

// XAutoFormattable

void SAL_CALL ScCellRangeObj::autoFormat( const rtl::OUString& aName )
                    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAutoFormat* pAutoFormat = ScGlobal::GetAutoFormat();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && pAutoFormat )
    {
        String aNameString = aName;
        USHORT nCount = pAutoFormat->GetCount();
        USHORT nIndex;
        String aCompare;
        for (nIndex=0; nIndex<nCount; nIndex++)
        {
            (*pAutoFormat)[nIndex]->GetName(aCompare);
            if ( aCompare == aNameString )                      //! Case-insensitiv ???
                break;
        }
        if (nIndex<nCount)
        {
            ScDocFunc aFunc(*pDocSh);
            aFunc.AutoFormat( aRange, NULL, nIndex, TRUE, TRUE );
        }
    }
}

// XSortable

uno::Sequence<beans::PropertyValue> SAL_CALL ScCellRangeObj::createSortDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSortParam aParam;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        // DB-Bereich anlegen erst beim Ausfuehren, per API immer genau den Bereich
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, TRUE );
        if (pData)
        {
            pData->GetSortParam(aParam);

            //  im SortDescriptor sind die Fields innerhalb des Bereichs gezaehlt
            ScRange aDBRange;
            pData->GetArea(aDBRange);
            USHORT nFieldStart = aParam.bByRow ? aDBRange.aStart.Col() : aDBRange.aStart.Row();
            for (USHORT i=0; i<MAXSORT; i++)
                if ( aParam.bDoSort[i] && aParam.nField[i] >= nFieldStart )
                    aParam.nField[i] -= nFieldStart;
        }
    }

    uno::Sequence<beans::PropertyValue> aSeq( ScSortDescriptor::GetPropertyCount() );
    ScSortDescriptor::FillProperties( aSeq, aParam );
    return aSeq;
}

void SAL_CALL ScCellRangeObj::sort( const uno::Sequence<beans::PropertyValue>& aDescriptor )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        USHORT i;
        ScSortParam aParam;
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );    // ggf. Bereich anlegen
        if (pData)
        {
            //  alten Einstellungen holen, falls nicht alles neu gesetzt wird
            pData->GetSortParam(aParam);
            USHORT nOldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();
            for (i=0; i<MAXSORT; i++)
                if ( aParam.bDoSort[i] && aParam.nField[i] >= nOldStart )
                    aParam.nField[i] -= nOldStart;
        }

        ScSortDescriptor::FillSortParam( aParam, aDescriptor );

        //  im SortDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        //  ByRow kann bei FillSortParam umgesetzt worden sein
        USHORT nFieldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();
        for (i=0; i<MAXSORT; i++)
            aParam.nField[i] += nFieldStart;

        USHORT nTab = aRange.aStart.Tab();
        aParam.nCol1 = aRange.aStart.Col();
        aParam.nRow1 = aRange.aStart.Row();
        aParam.nCol2 = aRange.aEnd.Col();
        aParam.nRow2 = aRange.aEnd.Row();

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );      // ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);                         // Bereich muss angelegt sein
        aFunc.Sort( nTab, aParam, TRUE, TRUE, TRUE );
    }
}

// XFilterable

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScCellRangeObj::createFilterDescriptor(
                                sal_Bool bEmpty ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScFilterDescriptor* pNew = new ScFilterDescriptor;
    ScDocShell* pDocSh = GetDocShell();
    if ( !bEmpty && pDocSh )
    {
        // DB-Bereich anlegen erst beim Ausfuehren, per API immer genau den Bereich
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, TRUE );
        if (pData)
        {
            ScQueryParam aParam;
            pData->GetQueryParam(aParam);
            //  im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
            ScRange aDBRange;
            pData->GetArea(aDBRange);
            USHORT nFieldStart = aParam.bByRow ? aDBRange.aStart.Col() : aDBRange.aStart.Row();
            USHORT nCount = aParam.GetEntryCount();
            for (USHORT i=0; i<nCount; i++)
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
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  das koennte theoretisch ein fremdes Objekt sein, also nur das
    //  oeffentliche XSheetFilterDescriptor Interface benutzen, um
    //  die Daten in ein ScFilterDescriptor Objekt zu kopieren:
    //! wenn es schon ein ScFilterDescriptor ist, direkt per getImplementation?

    ScFilterDescriptor aImpl;
    aImpl.setFilterFields( xDescriptor->getFilterFields() );
    //  Rest sind jetzt Properties...

    uno::Reference<beans::XPropertySet> xPropSet( xDescriptor, uno::UNO_QUERY );
    if (xPropSet.is())
        lcl_CopyProperties( aImpl, *(beans::XPropertySet*)xPropSet.get() );

    //
    //  ausfuehren...
    //

    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        ScQueryParam aParam = aImpl.GetParam();
        //  im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        USHORT nFieldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();
        USHORT nCount = aParam.GetEntryCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScQueryEntry& rEntry = aParam.GetEntry(i);
            if (rEntry.bDoQuery)
            {
                rEntry.nField += nFieldStart;
                //  Im Dialog wird immer der String angezeigt -> muss zum Wert passen
                if ( !rEntry.bQueryByString )
                    pDocSh->GetDocument()->GetFormatTable()->
                        GetInputLineString( rEntry.nVal, 0, *rEntry.pStr );
            }
        }

        USHORT nTab = aRange.aStart.Tab();
        aParam.nCol1 = aRange.aStart.Col();
        aParam.nRow1 = aRange.aStart.Row();
        aParam.nCol2 = aRange.aEnd.Col();
        aParam.nRow2 = aRange.aEnd.Row();

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );  // ggf. Bereich anlegen

        //! keep source range in filter descriptor
        //! if created by createFilterDescriptorByObject ???

        ScDBDocFunc aFunc(*pDocSh);
        aFunc.Query( nTab, aParam, NULL, TRUE, TRUE );  // Bereich muss angelegt sein
    }
}

//! get/setAutoFilter als Properties!!!

// XAdvancedFilterSource

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScCellRangeObj::createFilterDescriptorByObject(
                        const uno::Reference<sheet::XSheetFilterable>& xObject )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  this ist hier nicht der Bereich, der gefiltert wird, sondern der
    //  Bereich mit der Abfrage...

    uno::Reference<sheet::XCellRangeAddressable> xAddr( xObject, uno::UNO_QUERY );

    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && xAddr.is() )
    {
        //! Test, ob xObject im selben Dokument ist

        ScFilterDescriptor* pNew = new ScFilterDescriptor;  //! stattdessen vom Objekt?
        //XSheetFilterDescriptorRef xNew = xObject->createFilterDescriptor(TRUE);

        ScQueryParam aParam = pNew->GetParam();
        aParam.bHasHeader = TRUE;

        table::CellRangeAddress aDataAddress = xAddr->getRangeAddress();
        aParam.nCol1 = aDataAddress.StartColumn;
        aParam.nRow1 = aDataAddress.StartRow;
        aParam.nCol2 = aDataAddress.EndColumn;
        aParam.nRow2 = aDataAddress.EndRow;
        aParam.nTab  = aDataAddress.Sheet;

        ScDocument* pDoc = pDocSh->GetDocument();
        BOOL bOk = pDoc->CreateQueryParam(
                            aRange.aStart.Col(), aRange.aStart.Row(),
                            aRange.aEnd.Col(), aRange.aEnd.Row(),
                            aRange.aStart.Tab(), aParam );
        if ( bOk )
        {
            //  im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
            USHORT nFieldStart = aParam.bByRow ? aDataAddress.StartColumn : aDataAddress.StartRow;
            USHORT nCount = aParam.GetEntryCount();
            for (USHORT i=0; i<nCount; i++)
            {
                ScQueryEntry& rEntry = aParam.GetEntry(i);
                if (rEntry.bDoQuery && rEntry.nField >= nFieldStart)
                    rEntry.nField -= nFieldStart;
            }

            pNew->SetParam( aParam );
            return pNew;
        }
        else
        {
            delete pNew;
            return NULL;        // ungueltig -> null
        }
    }

    DBG_ERROR("kein Dokument oder kein Bereich");
    return NULL;
}

// XSubTotalSource

uno::Reference<sheet::XSubTotalDescriptor> SAL_CALL ScCellRangeObj::createSubTotalDescriptor(
                                sal_Bool bEmpty ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScSubTotalDescriptor* pNew = new ScSubTotalDescriptor;
    ScDocShell* pDocSh = GetDocShell();
    if ( !bEmpty && pDocSh )
    {
        // DB-Bereich anlegen erst beim Ausfuehren, per API immer genau den Bereich
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, TRUE );
        if (pData)
        {
            ScSubTotalParam aParam;
            pData->GetSubTotalParam(aParam);
            //  im SubTotalDescriptor sind die Fields innerhalb des Bereichs gezaehlt
            ScRange aDBRange;
            pData->GetArea(aDBRange);
            USHORT nFieldStart = aDBRange.aStart.Col();
            for (USHORT i=0; i<MAXSUBTOTAL; i++)
            {
                if ( aParam.bGroupActive[i] )
                {
                    if ( aParam.nField[i] >= nFieldStart )
                        aParam.nField[i] -= nFieldStart;
                    for (USHORT j=0; j<aParam.nSubTotals[i]; j++)
                        if ( aParam.pSubTotals[i][j] >= nFieldStart )
                            aParam.pSubTotals[i][j] -= nFieldStart;
                }
            }
            pNew->SetParam(aParam);
        }
    }
    return pNew;
}

void SAL_CALL ScCellRangeObj::applySubTotals(
                const uno::Reference<sheet::XSubTotalDescriptor>& xDescriptor,
                sal_Bool bReplace ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (!xDescriptor.is()) return;

    ScDocShell* pDocSh = GetDocShell();
    ScSubTotalDescriptorBase* pImp =
        ScSubTotalDescriptorBase::getImplementation( xDescriptor );

    if (pDocSh && pImp)
    {
        ScSubTotalParam aParam;
        pImp->GetData(aParam);      // virtuelle Methode der Basisklasse

        //  im SubTotalDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        USHORT nFieldStart = aRange.aStart.Col();
        for (USHORT i=0; i<MAXSUBTOTAL; i++)
        {
            if ( aParam.bGroupActive[i] )
            {
                aParam.nField[i] += nFieldStart;
                for (USHORT j=0; j<aParam.nSubTotals[i]; j++)
                    aParam.pSubTotals[i][j] += nFieldStart;
            }
        }

        aParam.bReplace = bReplace;

        USHORT nTab = aRange.aStart.Tab();
        aParam.nCol1 = aRange.aStart.Col();
        aParam.nRow1 = aRange.aStart.Row();
        aParam.nCol2 = aRange.aEnd.Col();
        aParam.nRow2 = aRange.aEnd.Row();

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );  // ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);
        aFunc.DoSubTotals( nTab, aParam, NULL, TRUE, TRUE );    // Bereich muss angelegt sein
    }
}

void SAL_CALL ScCellRangeObj::removeSubTotals() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        ScSubTotalParam aParam;
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, TRUE );
        if (pData)
            pData->GetSubTotalParam(aParam);    // auch bei Remove die Feld-Eintraege behalten

        aParam.bRemoveOnly = TRUE;

        USHORT nTab = aRange.aStart.Tab();
        aParam.nCol1 = aRange.aStart.Col();
        aParam.nRow1 = aRange.aStart.Row();
        aParam.nCol2 = aRange.aEnd.Col();
        aParam.nRow2 = aRange.aEnd.Row();

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );  // ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);
        aFunc.DoSubTotals( nTab, aParam, NULL, TRUE, TRUE );    // Bereich muss angelegt sein
    }
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScCellRangeObj::createImportDescriptor( sal_Bool bEmpty )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScImportParam aParam;
    ScDocShell* pDocSh = GetDocShell();
    if ( !bEmpty && pDocSh )
    {
        // DB-Bereich anlegen erst beim Ausfuehren, per API immer genau den Bereich
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, TRUE );
        if (pData)
            pData->GetImportParam(aParam);
    }

    uno::Sequence<beans::PropertyValue> aSeq( ScImportDescriptor::GetPropertyCount() );
    ScImportDescriptor::FillProperties( aSeq, aParam );
    return aSeq;
}

void SAL_CALL ScCellRangeObj::doImport( const uno::Sequence<beans::PropertyValue>& aDescriptor )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        ScImportParam aParam;
        ScImportDescriptor::FillImportParam( aParam, aDescriptor );

        USHORT nTab = aRange.aStart.Tab();
        aParam.nCol1 = aRange.aStart.Col();
        aParam.nRow1 = aRange.aStart.Row();
        aParam.nCol2 = aRange.aEnd.Col();
        aParam.nRow2 = aRange.aEnd.Row();

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );      // ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);                         // Bereich muss angelegt sein
        aFunc.DoImport( nTab, aParam, NULL, TRUE, FALSE );  //! Api-Flag als Parameter
    }
}

// XCellFormatsSupplier

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangeObj::getCellFormatRanges()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScCellFormatsObj( pDocSh, aRange );
    return NULL;
}

// XPropertySet erweitert fuer Range-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellRangeObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aRangePropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScCellRangeObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  Range hat nur Position und Size zusaetzlich zu ScCellRangesBase, beide sind ReadOnly
    //  -> hier muss nichts passieren

    ScCellRangesBase::setPropertyValue(aPropertyName, aValue);
}

uno::Any SAL_CALL ScCellRangeObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  eigene Properties

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_POS ) )
    {
        ScDocShell* pDocSh = GetDocShell();
        if (pDocSh)
        {
            //  GetMMRect rechnet per HMM_PER_TWIPS, also passend zum DrawingLayer
            Rectangle aMMRect = pDocSh->GetDocument()->GetMMRect(
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() );
            awt::Point aPos( aMMRect.Left(), aMMRect.Top() );
            aAny <<= aPos;
        }
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_SIZE ) )
    {
        ScDocShell* pDocSh = GetDocShell();
        if (pDocSh)
        {
            //  GetMMRect rechnet per HMM_PER_TWIPS, also passend zum DrawingLayer
            Rectangle aMMRect = pDocSh->GetDocument()->GetMMRect(
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() );
            Size aSize = aMMRect.GetSize();
            awt::Size aAwtSize( aSize.Width(), aSize.Height() );
            aAny <<= aAwtSize;
        }
    }
    else
        aAny = ScCellRangesBase::getPropertyValue(aPropertyName);       // geerbte mit oder ohne WID

    return aAny;
}

// XServiceInfo

rtl::OUString SAL_CALL ScCellRangeObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScCellRangeObj" );
}

sal_Bool SAL_CALL ScCellRangeObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCSHEETCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellRangeObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(4);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCSHEETCELLRANGE_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCCELLRANGE_SERVICE );
    pArray[2] = rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[3] = rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    return aRet;
}

//------------------------------------------------------------------------

const SfxItemPropertyMap* ScCellObj::GetEditPropertyMap()       // static
{
    return lcl_GetEditPropertyMap();
}

ScCellObj::ScCellObj(ScDocShell* pDocSh, const ScAddress& rP) :
    ScCellRangeObj( pDocSh, ScRange(rP,rP) ),
    aCellPropSet( lcl_GetCellPropertyMap() ),
    aCellPos( rP ),
    pUnoText( NULL )
{
    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

SvxUnoText& ScCellObj::GetUnoText()
{
    if (!pUnoText)
    {
        ScCellEditSource aEditSource( GetDocShell(), aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetEditPropertyMap(),
                                    uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
    return *pUnoText;
}

ScCellObj::~ScCellObj()
{
    if (pUnoText)
        pUnoText->release();
}

void ScCellObj::RefChanged()
{
    ScCellRangeObj::RefChanged();

    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT(rRanges.Count() == 1, "was fuer Ranges ?!?!");
    const ScRange* pFirst = rRanges.GetObject(0);
    if (pFirst)
        aCellPos = pFirst->aStart;
}

uno::Any SAL_CALL ScCellObj::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( table::XCell )
    SC_QUERYINTERFACE( sheet::XCellAddressable )
    SC_QUERYINTERFACE( text::XText )
    SC_QUERYINTERFACE( text::XSimpleText )
    SC_QUERYINTERFACE( text::XTextRange )
    SC_QUERYINTERFACE( container::XEnumerationAccess )
    SC_QUERYINTERFACE( container::XElementAccess )
    SC_QUERYINTERFACE( sheet::XSheetAnnotationAnchor )
    SC_QUERYINTERFACE( text::XTextFieldsSupplier )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScCellObj::acquire() throw(uno::RuntimeException)
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScCellObj::release() throw(uno::RuntimeException)
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangeObj::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 6 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<table::XCell>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<sheet::XCellAddressable>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<text::XText>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<container::XEnumerationAccess>*)0);
        pPtr[nParentLen + 4] = getCppuType((const uno::Reference<sheet::XSheetAnnotationAnchor>*)0);
        pPtr[nParentLen + 5] = getCppuType((const uno::Reference<text::XTextFieldsSupplier>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellObj::getImplementationId() throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

//  Hilfsfunktionen

String ScCellObj::GetInputString_Impl(BOOL bEnglish) const      // fuer getFormula / FormulaLocal
{
    ScDocShell* pDocSh = GetDocShell();
    String aVal;
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        ScBaseCell* pCell = pDoc->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
        {
            CellType eType = pCell->GetCellType();
            if ( eType == CELLTYPE_FORMULA )
            {
                ScFormulaCell* pForm = (ScFormulaCell*)pCell;
                if (bEnglish)
                    pForm->GetEnglishFormula( aVal );
                else
                    pForm->GetFormula( aVal );
            }
            else
            {
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                ULONG nNumFmt = bEnglish ?
                        pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US) :
                        pDoc->GetNumberFormat( aCellPos );

                if ( eType == CELLTYPE_EDIT )
                {
                    //  GetString an der EditCell macht Leerzeichen aus Umbruechen,
                    //  hier werden die Umbrueche aber gebraucht
                    const EditTextObject* pData = ((ScEditCell*)pCell)->GetData();
                    if (pData)
                    {
                        EditEngine& rEngine = pDoc->GetEditEngine();
                        rEngine.SetText( *pData );
                        aVal = rEngine.GetText( LINEEND_LF );
                    }
                }
                else
                    ScCellFormat::GetInputString( pCell, nNumFmt, aVal, *pFormatter );

                //  ggf. ein ' davorhaengen wie in ScTabViewShell::UpdateInputHandler
                if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
                {
                    double fDummy;
                    if ( pFormatter->IsNumberFormat(aVal, nNumFmt, fDummy) )
                        aVal.Insert('\'',0);
                }
            }
        }
    }
    return aVal;
}

String ScCellObj::GetOutputString_Impl() const
{
    ScDocShell* pDocSh = GetDocShell();
    String aVal;
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        ScBaseCell* pCell = pDoc->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
        {
            if ( pCell->GetCellType() == CELLTYPE_EDIT )
            {
                //  GetString an der EditCell macht Leerzeichen aus Umbruechen,
                //  hier werden die Umbrueche aber gebraucht
                const EditTextObject* pData = ((ScEditCell*)pCell)->GetData();
                if (pData)
                {
                    EditEngine& rEngine = pDoc->GetEditEngine();
                    rEngine.SetText( *pData );
                    aVal = rEngine.GetText( LINEEND_LF );
                }
                //  Edit-Zellen auch nicht per NumberFormatter formatieren
                //  (passend zur Ausgabe)
            }
            else
            {
                //  wie in GetString am Dokument (column)
                Color* pColor;
                ULONG nNumFmt = pDoc->GetNumberFormat( aCellPos );
                ScCellFormat::GetString( pCell, nNumFmt, aVal, &pColor, *pDoc->GetFormatTable() );
            }
        }
    }
    return aVal;
}

void ScCellObj::SetString_Impl(const String& rString, BOOL bInterpret, BOOL bEnglish)
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocFunc aFunc(*pDocSh);
        BOOL bOk = aFunc.SetCellText( aCellPos, rString, bInterpret, bEnglish, TRUE );
    }
}

double ScCellObj::GetValue_Impl() const
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocument()->GetValue( aCellPos );

    return 0.0;
}

void ScCellObj::SetValue_Impl(double fValue)
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocFunc aFunc(*pDocSh);
        BOOL bOk = aFunc.PutCell( aCellPos, new ScValueCell(fValue), TRUE );
    }
}

// only for XML import

void ScCellObj::SetFormulaResultString( const ::rtl::OUString& rResult )
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScBaseCell* pCell = pDocSh->GetDocument()->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->SetString( rResult );
    }
}

void ScCellObj::SetFormulaResultDouble( double fResult )
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScBaseCell* pCell = pDocSh->GetDocument()->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->SetDouble( fResult );
    }
}

//  XText

uno::Reference<text::XTextCursor> SAL_CALL ScCellObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScCellTextCursor( GetUnoText() );
}

uno::Reference<text::XTextCursor> SAL_CALL ScCellObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SvxUnoTextCursor* pCursor = new ScCellTextCursor( GetUnoText() );
    uno::Reference<text::XTextCursor> xCursor = pCursor;

    SvxUnoTextRangeBase* pRange = SvxUnoTextRangeBase::getImplementation( aTextPosition );
    if(pRange)
        pCursor->SetSelection( pRange->GetSelection() );
    else
    {
        ScCellTextCursor* pOther = ScCellTextCursor::getImplementation( aTextPosition );
        if(pOther)
            pCursor->SetSelection( pOther->GetSelection() );
        else
            throw uno::RuntimeException();
    }

    return xCursor;
}

rtl::OUString SAL_CALL ScCellObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetOutputString_Impl();
}

void SAL_CALL ScCellObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aText;
    SetString_Impl(aString, FALSE, FALSE);  // immer Text

    // don't create pUnoText here if not there
    if (pUnoText)
        pUnoText->SetSelection(ESelection( 0,0, 0,aString.Len() ));
}

void SAL_CALL ScCellObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                        const rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    // der SvxUnoText kennt den ScCellTextCursor nicht, darum muss der Fall hier abgefangen werden

    if ( xRange.is() )
    {
        ScCellTextCursor* pCursor = ScCellTextCursor::getImplementation( xRange );
        if ( pCursor )
        {
            //  setString am Cursor statt selber QuickInsertText und UpdateData,
            //  damit die Selektion am Cursor angepasst wird.
            //! Eigentlich muessten alle Cursor-Objekte dieses Textes angepasst werden!

            if (!bAbsorb)                   // nicht ersetzen -> hinten anhaengen
                pCursor->CollapseToEnd();

            pCursor->setString( aString );
            return;
        }
    }

    GetUnoText().insertString(xRange, aString, bAbsorb);
}

void SAL_CALL ScCellObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                                sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().insertControlCharacter(xRange, nControlCharacter, bAbsorb);
}

void SAL_CALL ScCellObj::insertTextContent( const uno::Reference<text::XTextRange >& xRange,
                                                const uno::Reference<text::XTextContent >& xContent,
                                                sal_Bool bAbsorb )
                                    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && xContent.is() )
    {
        ScCellFieldObj* pCellField = ScCellFieldObj::getImplementation( xContent );
        SvxUnoTextRangeBase* pTextRange = ScCellTextCursor::getImplementation( xRange );

#if 0
        if (!pTextRange)
            pTextRange = SvxUnoTextRangeBase::getImplementation( xRange );

        //! bei SvxUnoTextRange testen, ob in passendem Objekt !!!
#endif

        if ( pCellField && !pCellField->IsInserted() && pTextRange )
        {
            SvxEditSource* pEditSource = pTextRange->GetEditSource();
            ESelection aSelection = pTextRange->GetSelection();

            if (!bAbsorb)
            {
                //  nicht ersetzen -> hinten anhaengen
                aSelection.Adjust();
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos  = aSelection.nEndPos;
            }

            SvxFieldItem aItem = pCellField->CreateFieldItem();

            SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
            pForwarder->QuickInsertField( aItem, aSelection );
            pEditSource->UpdateData();

            //  neue Selektion: ein Zeichen
            aSelection.Adjust();
            aSelection.nEndPara = aSelection.nStartPara;
            aSelection.nEndPos = aSelection.nStartPos + 1;
            pCellField->InitDoc( pDocSh, aCellPos, aSelection );

            pTextRange->SetSelection( aSelection );

            return;
        }
    }
    GetUnoText().insertTextContent(xRange, xContent, bAbsorb);
}

void SAL_CALL ScCellObj::removeTextContent( const uno::Reference<text::XTextContent>& xContent )
                                throw(container::NoSuchElementException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( xContent.is() )
    {
        ScCellFieldObj* pCellField = ScCellFieldObj::getImplementation( xContent );
        if ( pCellField && pCellField->IsInserted() )
        {
            //! Testen, ob das Feld in dieser Zelle ist
            pCellField->DeleteField();
            return;
        }
    }
    GetUnoText().removeTextContent(xContent);
}

uno::Reference<text::XText> SAL_CALL ScCellObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScCellObj::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScCellObj::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getEnd();
}

uno::Reference<container::XEnumeration> SAL_CALL ScCellObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().createEnumeration();
}

uno::Type SAL_CALL ScCellObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getElementType();
}

sal_Bool SAL_CALL ScCellObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().hasElements();
}

//  XCell

rtl::OUString SAL_CALL ScCellObj::getFormula() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  TRUE = englisch
    return GetInputString_Impl(TRUE);
}

void SAL_CALL ScCellObj::setFormula( const rtl::OUString& aFormula ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aFormula;
    SetString_Impl(aString, TRUE, TRUE);    // englisch interpretieren
}

double SAL_CALL ScCellObj::getValue() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetValue_Impl();
}

void SAL_CALL ScCellObj::setValue( double nValue ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SetValue_Impl(nValue);
}

table::CellContentType SAL_CALL ScCellObj::getType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellContentType eRet = table::CellContentType_EMPTY;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        CellType eCalcType = pDocSh->GetDocument()->GetCellType( aCellPos );
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
        DBG_ERROR("keine DocShell");        //! Exception oder so?

    return eRet;
}

table::CellContentType ScCellObj::GetResultType_Impl()
{
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScBaseCell* pCell = pDocSh->GetDocument()->GetCell(aCellPos);
        if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
        {
            BOOL bValue = ((ScFormulaCell*)pCell)->IsValue();
            return bValue ? table::CellContentType_VALUE : table::CellContentType_TEXT;
        }
    }
    return getType();   // wenn keine Formel
}

sal_Int32 SAL_CALL ScCellObj::getError() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nError = 0;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        ScBaseCell* pCell = pDocSh->GetDocument()->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
            nError = ((ScFormulaCell*)pCell)->GetErrCode();
        // sonst bleibt's bei 0
    }
    else
        DBG_ERROR("keine DocShell");        //! Exception oder so?

    return nError;
}

// XCellAddressable

table::CellAddress SAL_CALL ScCellObj::getCellAddress() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellAddress aAdr;
    aAdr.Sheet  = aCellPos.Tab();
    aAdr.Column = aCellPos.Col();
    aAdr.Row    = aCellPos.Row();
    return aAdr;
}

// XSheetAnnotationAnchor

uno::Reference<sheet::XSheetAnnotation> SAL_CALL ScCellObj::getAnnotation()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScAnnotationObj( pDocSh, aCellPos );

    DBG_ERROR("getAnnotation ohne DocShell");
    return NULL;
}

// XFieldTypesSupplier

uno::Reference<container::XEnumerationAccess> SAL_CALL ScCellObj::getTextFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScCellFieldsObj( pDocSh, aCellPos );

    return NULL;
}

uno::Reference<container::XNameAccess> SAL_CALL ScCellObj::getTextFieldMasters()
                                                throw(uno::RuntimeException)
{
    //  sowas gibts nicht im Calc (?)
    return NULL;
}

// XPropertySet erweitert fuer Zell-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aCellPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScCellObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu setzen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
    {
        ScCellRangesBase::setPropertyValue(aPropertyName, aValue);
        return;
    }

    //  eigene Properties

    if ( aNameString.EqualsAscii( SC_UNONAME_FORMLOC ) )
    {
        rtl::OUString aStrVal;
        aValue >>= aStrVal;
        String aString = aStrVal;
        SetString_Impl(aString, TRUE, FALSE);   // lokal interpretieren
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_FORMRT ) )
    {
        //  Read-Only
        //! Exception oder so...
    }
    else
        ScCellRangeObj::setPropertyValue(aPropertyName, aValue);        // geerbte ohne WID
}

uno::Any SAL_CALL ScCellObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu bekommen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
        return ScCellRangesBase::getPropertyValue(aPropertyName);

    //  eigene Properties

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_FORMLOC ) )
    {
        // FALSE = lokal
        String aFormStr = GetInputString_Impl(FALSE);
        aAny <<= rtl::OUString( aFormStr );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_FORMRT ) )
    {
        table::CellContentType eType = GetResultType_Impl();
        aAny <<= eType;
    }
    else
        aAny = ScCellRangeObj::getPropertyValue(aPropertyName);     // geerbte ohne WID

    return aAny;
}

// XServiceInfo

rtl::OUString SAL_CALL ScCellObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScCellObj" );
}

sal_Bool SAL_CALL ScCellObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    //  CellRange/SheetCellRange are not in SheetCell service description,
    //  but ScCellObj is used instead of ScCellRangeObj in CellRanges collections,
    //  so it must support them

    String aServiceStr(rServiceName);
    return aServiceStr.EqualsAscii( SCSHEETCELL_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELL_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCSHEETCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLRANGE_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(6);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCSHEETCELL_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCCELL_SERVICE );
    pArray[2] = rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[3] = rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    pArray[4] = rtl::OUString::createFromAscii( SCSHEETCELLRANGE_SERVICE );
    pArray[5] = rtl::OUString::createFromAscii( SCCELLRANGE_SERVICE );
    return aRet;
}

//------------------------------------------------------------------------

ScTableSheetObj::ScTableSheetObj( ScDocShell* pDocSh, USHORT nTab ) :
    ScCellRangeObj( pDocSh, ScRange(0,0,nTab, MAXCOL,MAXROW,nTab) ),
    aSheetPropSet(lcl_GetSheetPropertyMap())
{
}

ScTableSheetObj::~ScTableSheetObj()
{
}

void ScTableSheetObj::InitInsertSheet(ScDocShell* pDocSh, USHORT nTab)
{
    InitInsertRange( pDocSh, ScRange(0,0,nTab, MAXCOL,MAXROW,nTab) );
}

uno::Any SAL_CALL ScTableSheetObj::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
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
    SC_QUERYINTERFACE( sheet::XSheetLinkable )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScTableSheetObj::acquire() throw(uno::RuntimeException)
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScTableSheetObj::release() throw(uno::RuntimeException)
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScTableSheetObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangeObj::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 15 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XSpreadsheet>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<container::XNamed>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<sheet::XSheetPageBreak>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<sheet::XCellRangeMovement>*)0);
        pPtr[nParentLen + 4] = getCppuType((const uno::Reference<table::XTableChartsSupplier>*)0);
        pPtr[nParentLen + 5] = getCppuType((const uno::Reference<sheet::XDataPilotTablesSupplier>*)0);
        pPtr[nParentLen + 6] = getCppuType((const uno::Reference<sheet::XScenariosSupplier>*)0);
        pPtr[nParentLen + 7] = getCppuType((const uno::Reference<sheet::XSheetAnnotationsSupplier>*)0);
        pPtr[nParentLen + 8] = getCppuType((const uno::Reference<drawing::XDrawPageSupplier>*)0);
        pPtr[nParentLen + 9] = getCppuType((const uno::Reference<sheet::XPrintAreas>*)0);
        pPtr[nParentLen +10] = getCppuType((const uno::Reference<sheet::XSheetAuditing>*)0);
        pPtr[nParentLen +11] = getCppuType((const uno::Reference<sheet::XSheetOutline>*)0);
        pPtr[nParentLen +12] = getCppuType((const uno::Reference<util::XProtectable>*)0);
        pPtr[nParentLen +13] = getCppuType((const uno::Reference<sheet::XScenario>*)0);
        pPtr[nParentLen +14] = getCppuType((const uno::Reference<sheet::XSheetLinkable>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScTableSheetObj::getImplementationId() throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

//  Hilfsfunktionen

USHORT ScTableSheetObj::GetTab_Impl() const
{
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT(rRanges.Count() == 1, "was fuer Ranges ?!?!");
    const ScRange* pFirst = rRanges.GetObject(0);
    if (pFirst)
        return pFirst->aStart.Tab();

    return 0;   // soll nicht sein
}

// former XSheet

uno::Reference<table::XTableCharts> SAL_CALL ScTableSheetObj::getCharts() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScChartsObj( pDocSh, GetTab_Impl() );

    DBG_ERROR("kein Dokument");
    return NULL;
}

uno::Reference<sheet::XDataPilotTables> SAL_CALL ScTableSheetObj::getDataPilotTables()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScDataPilotTablesObj( pDocSh, GetTab_Impl() );

    DBG_ERROR("kein Dokument");
    return NULL;
}

uno::Reference<sheet::XScenarios> SAL_CALL ScTableSheetObj::getScenarios() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();

    if ( pDocSh )
        return new ScScenariosObj( pDocSh, GetTab_Impl() );

    DBG_ERROR("kein Dokument");
    return NULL;
}

uno::Reference<sheet::XSheetAnnotations> SAL_CALL ScTableSheetObj::getAnnotations()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();

    if ( pDocSh )
        return new ScAnnotationsObj( pDocSh, GetTab_Impl() );

    DBG_ERROR("kein Dokument");
    return NULL;
}

uno::Reference<table::XCellRange> SAL_CALL ScTableSheetObj::getCellRangeByName(
                        const rtl::OUString& aRange ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::getCellRangeByName( aRange );
}

uno::Reference<sheet::XSheetCellCursor> SAL_CALL ScTableSheetObj::createCursor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        //! einzelne Zelle oder ganze Tabelle???????
        USHORT nTab = GetTab_Impl();
        return new ScCellCursorObj( pDocSh, ScRange( 0,0,nTab, MAXCOL,MAXROW,nTab ) );
    }
    return NULL;
}

uno::Reference<sheet::XSheetCellCursor> SAL_CALL ScTableSheetObj::createCursorByRange(
                        const uno::Reference<sheet::XSheetCellRange>& aRange )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && aRange.is() )
    {
        ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( aRange );
        if (pRangesImp)
        {
            const ScRangeList& rRanges = pRangesImp->GetRangeList();
            DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
            return new ScCellCursorObj( pDocSh, *rRanges.GetObject(0) );
        }
    }
    return NULL;
}

// XSheetCellRange

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScTableSheetObj::getSpreadsheet()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return this;        //!???
}

// XCellRange

uno::Reference<table::XCell> SAL_CALL ScTableSheetObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::getCellByPosition(nColumn, nRow);
}

uno::Reference<table::XCellRange> SAL_CALL ScTableSheetObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::getCellRangeByPosition(nLeft,nTop,nRight,nBottom);
}

uno::Sequence<sheet::TablePageBreakData> SAL_CALL ScTableSheetObj::getColumnPageBreaks()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        Size aSize = pDoc->GetPageSize( nTab );
        if (aSize.Width() && aSize.Height())        // effektive Groesse schon gesetzt?
            pDoc->UpdatePageBreaks( nTab );
        else
        {
            //  Umbrueche updaten wie in ScDocShell::PageStyleModified:
            ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab );
            aPrintFunc.UpdatePages();
        }

        USHORT nCount = 0;
        USHORT nCol;
        for (nCol=0; nCol<=MAXCOL; nCol++)
            if (pDoc->GetColFlags( nCol, nTab ) & ( CR_PAGEBREAK | CR_MANUALBREAK ))
                ++nCount;

        sheet::TablePageBreakData aData;
        uno::Sequence<sheet::TablePageBreakData> aSeq(nCount);
        sheet::TablePageBreakData* pAry = aSeq.getArray();
        USHORT nPos = 0;
        for (nCol=0; nCol<=MAXCOL; nCol++)
        {
            BYTE nFlags = pDoc->GetColFlags( nCol, nTab );
            if (nFlags & ( CR_PAGEBREAK | CR_MANUALBREAK ))
            {
                aData.Position    = nCol;
                aData.ManualBreak = ( nFlags & CR_MANUALBREAK ) != 0;
                pAry[nPos] = aData;
                ++nPos;
            }
        }
        return aSeq;
    }
    return uno::Sequence<sheet::TablePageBreakData>(0);
}

uno::Sequence<sheet::TablePageBreakData> SAL_CALL ScTableSheetObj::getRowPageBreaks()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        Size aSize = pDoc->GetPageSize( nTab );
        if (aSize.Width() && aSize.Height())        // effektive Groesse schon gesetzt?
            pDoc->UpdatePageBreaks( nTab );
        else
        {
            //  Umbrueche updaten wie in ScDocShell::PageStyleModified:
            ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab );
            aPrintFunc.UpdatePages();
        }

        USHORT nCount = 0;
        USHORT nRow;
        for (nRow=0; nRow<=MAXROW; nRow++)
            if (pDoc->GetRowFlags( nRow, nTab ) & ( CR_PAGEBREAK | CR_MANUALBREAK ))
                ++nCount;

        sheet::TablePageBreakData aData;
        uno::Sequence<sheet::TablePageBreakData> aSeq(nCount);
        sheet::TablePageBreakData* pAry = aSeq.getArray();
        USHORT nPos = 0;
        for (nRow=0; nRow<=MAXROW; nRow++)
        {
            BYTE nFlags = pDoc->GetRowFlags( nRow, nTab );
            if (nFlags & ( CR_PAGEBREAK | CR_MANUALBREAK ))
            {
                aData.Position    = nRow;
                aData.ManualBreak = ( nFlags & CR_MANUALBREAK ) != 0;
                pAry[nPos] = aData;
                ++nPos;
            }
        }
        return aSeq;
    }
    return uno::Sequence<sheet::TablePageBreakData>(0);
}

void SAL_CALL ScTableSheetObj::removeAllManualPageBreaks() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        //! docfunc Funktion, auch fuer ScViewFunc::RemoveManualBreaks

        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
        pDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, FALSE, pUndoDoc );
        pDocSh->GetUndoManager()->AddUndoAction(
                                    new ScUndoRemoveBreaks( pDocSh, nTab, pUndoDoc ) );

        pDoc->RemoveManualBreaks(nTab);
        pDoc->UpdatePageBreaks(nTab);

        //? UpdatePageBreakData( TRUE );
        pDocSh->SetDocumentModified();
        pDocSh->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
    }
}

// XNamed

rtl::OUString SAL_CALL ScTableSheetObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aName;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocument()->GetName( GetTab_Impl(), aName );
    return aName;
}

void SAL_CALL ScTableSheetObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        String aString = aNewName;
        ScDocFunc aFunc( *pDocSh );
        aFunc.RenameTable( GetTab_Impl(), aString, TRUE, TRUE );
    }
}

// XDrawPageSupplier

uno::Reference<drawing::XDrawPage> SAL_CALL ScTableSheetObj::getDrawPage()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDrawLayer* pDrawLayer = pDocSh->MakeDrawLayer();
        DBG_ASSERT(pDrawLayer,"kann Draw-Layer nicht anlegen");

        USHORT nTab = GetTab_Impl();
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Draw-Page nicht gefunden");
        if (pPage)
            return new SvxFmDrawPage( pPage );

        //  Das DrawPage-Objekt meldet sich als Listener am SdrModel an
        //  und sollte von dort alle Aktionen mitbekommen
    }
    return NULL;
}

// XCellMovement

void SAL_CALL ScTableSheetObj::insertCells( const table::CellRangeAddress& aRange,
                                sheet::CellInsertMode nMode ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        BOOL bDo = TRUE;
        InsCellCmd eCmd;
        switch (nMode)
        {
            case sheet::CellInsertMode_NONE:    bDo = FALSE;            break;
            case sheet::CellInsertMode_DOWN:    eCmd = INS_CELLSDOWN;   break;
            case sheet::CellInsertMode_RIGHT:   eCmd = INS_CELLSRIGHT;  break;
            case sheet::CellInsertMode_ROWS:    eCmd = INS_INSROWS;     break;
            case sheet::CellInsertMode_COLUMNS: eCmd = INS_INSCOLS;     break;
            default:
                DBG_ERROR("insertCells: falscher Mode");
                bDo = FALSE;
        }

        if (bDo)
        {
            DBG_ASSERT( aRange.Sheet == GetTab_Impl(), "falsche Tabelle in CellRangeAddress" );
            ScRange aScRange;
            ScUnoConversion::FillRange( aScRange, aRange );
            ScDocFunc aFunc(*pDocSh);
            aFunc.InsertCells( aScRange, eCmd, TRUE, TRUE );
        }
    }
}

void SAL_CALL ScTableSheetObj::removeRange( const table::CellRangeAddress& aRange,
                                sheet::CellDeleteMode nMode ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        BOOL bDo = TRUE;
        DelCellCmd eCmd;
        switch (nMode)
        {
            case sheet::CellDeleteMode_NONE:     bDo = FALSE;           break;
            case sheet::CellDeleteMode_UP:       eCmd = DEL_CELLSUP;    break;
            case sheet::CellDeleteMode_LEFT:     eCmd = DEL_CELLSLEFT;  break;
            case sheet::CellDeleteMode_ROWS:     eCmd = DEL_DELROWS;    break;
            case sheet::CellDeleteMode_COLUMNS:  eCmd = DEL_DELCOLS;    break;
            default:
                DBG_ERROR("deleteCells: falscher Mode");
                bDo = FALSE;
        }

        if (bDo)
        {
            DBG_ASSERT( aRange.Sheet == GetTab_Impl(), "falsche Tabelle in CellRangeAddress" );
            ScRange aScRange;
            ScUnoConversion::FillRange( aScRange, aRange );
            ScDocFunc aFunc(*pDocSh);
            aFunc.DeleteCells( aScRange, eCmd, TRUE, TRUE );
        }
    }
}

void SAL_CALL ScTableSheetObj::moveRange( const table::CellAddress& aDestination,
                                        const table::CellRangeAddress& aSource )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        DBG_ASSERT( aSource.Sheet == GetTab_Impl(), "falsche Tabelle in CellRangeAddress" );
        ScRange aRange;
        ScUnoConversion::FillRange( aRange, aSource );
        ScAddress aDestPos( aDestination.Column, aDestination.Row, aDestination.Sheet );
        ScDocFunc aFunc(*pDocSh);
        aFunc.MoveBlock( aRange, aDestPos, TRUE, TRUE, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::copyRange( const table::CellAddress& aDestination,
                                        const table::CellRangeAddress& aSource )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        DBG_ASSERT( aSource.Sheet == GetTab_Impl(), "falsche Tabelle in CellRangeAddress" );
        ScRange aRange;
        ScUnoConversion::FillRange( aRange, aSource );
        ScAddress aDestPos( aDestination.Column, aDestination.Row, aDestination.Sheet );
        ScDocFunc aFunc(*pDocSh);
        aFunc.MoveBlock( aRange, aDestPos, FALSE, TRUE, TRUE, TRUE );
    }
}

// XPrintAreas

void ScTableSheetObj::PrintAreaUndo_Impl( ScPrintRangeSaver* pOldRanges )
{
    //  Umbrueche und Undo

    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScPrintRangeSaver* pNewRanges = pDoc->CreatePrintRangeSaver();
        pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoPrintRange( pDocSh, nTab, pOldRanges, pNewRanges ) );

        ScPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab ).UpdatePages();

        SfxBindings* pBindings = pDocSh->GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DELETE_PRINTAREA );

        pDocSh->SetDocumentModified();
    }
    else
        delete pOldRanges;
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScTableSheetObj::getPrintAreas()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();
        USHORT nCount = pDoc->GetPrintRangeCount( nTab );

        table::CellRangeAddress aRangeAddress;
        uno::Sequence<table::CellRangeAddress> aSeq(nCount);
        table::CellRangeAddress* pAry = aSeq.getArray();
        for (USHORT i=0; i<nCount; i++)
        {
            const ScRange* pRange = pDoc->GetPrintRange( nTab, i );
            DBG_ASSERT(pRange,"wo ist der Druckbereich");
            if (pRange)
            {
                ScUnoConversion::FillAddress( aRangeAddress, *pRange );
                pAry[i] = aRangeAddress;
            }
        }
        return aSeq;
    }
    return uno::Sequence<table::CellRangeAddress>();
}

void SAL_CALL ScTableSheetObj::setPrintAreas(
                    const uno::Sequence<table::CellRangeAddress>& aPrintAreas )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScPrintRangeSaver* pOldRanges = pDoc->CreatePrintRangeSaver();

        USHORT nCount = (USHORT) aPrintAreas.getLength();
        pDoc->SetPrintRangeCount( nTab, nCount );
        if (nCount)
        {
            ScRange aRange;
            const table::CellRangeAddress* pAry = aPrintAreas.getConstArray();
            for (USHORT i=0; i<nCount; i++)
            {
                ScUnoConversion::FillRange( aRange, pAry[i] );
                pDoc->SetPrintRange( nTab, i, aRange );
            }
        }

        PrintAreaUndo_Impl( pOldRanges );   // Undo, Umbrueche, Modified etc.
    }
}

sal_Bool SAL_CALL ScTableSheetObj::getPrintTitleColumns() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();
        return ( pDoc->GetRepeatColRange(nTab) != NULL );
    }
    return FALSE;
}

void SAL_CALL ScTableSheetObj::setPrintTitleColumns( sal_Bool bPrintTitleColumns )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScPrintRangeSaver* pOldRanges = pDoc->CreatePrintRangeSaver();

        if ( bPrintTitleColumns )
        {
            if ( !pDoc->GetRepeatColRange( nTab ) )         // keinen bestehenden Bereich veraendern
            {
                ScRange aNew( 0, 0, nTab, 0, 0, nTab );     // Default
                pDoc->SetRepeatColRange( nTab, &aNew );     // einschalten
            }
        }
        else
            pDoc->SetRepeatColRange( nTab, NULL );          // abschalten

        PrintAreaUndo_Impl( pOldRanges );   // Undo, Umbrueche, Modified etc.

        //! zuletzt gesetzten Bereich beim Abschalten merken und beim Einschalten wiederherstellen ???
    }
}

table::CellRangeAddress SAL_CALL ScTableSheetObj::getTitleColumns() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aRet;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();
        const ScRange* pRange = pDoc->GetRepeatColRange(nTab);
        if (pRange)
            ScUnoConversion::FillAddress( aRet, *pRange );
    }
    return aRet;
}

void SAL_CALL ScTableSheetObj::setTitleColumns( const table::CellRangeAddress& aTitleColumns )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScPrintRangeSaver* pOldRanges = pDoc->CreatePrintRangeSaver();

        ScRange aNew;
        ScUnoConversion::FillRange( aNew, aTitleColumns );
        pDoc->SetRepeatColRange( nTab, &aNew );     // immer auch einschalten

        PrintAreaUndo_Impl( pOldRanges );           // Undo, Umbrueche, Modified etc.
    }
}

sal_Bool SAL_CALL ScTableSheetObj::getPrintTitleRows() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();
        return ( pDoc->GetRepeatRowRange(nTab) != NULL );
    }
    return FALSE;
}

void SAL_CALL ScTableSheetObj::setPrintTitleRows( sal_Bool bPrintTitleRows )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScPrintRangeSaver* pOldRanges = pDoc->CreatePrintRangeSaver();

        if ( bPrintTitleRows )
        {
            if ( !pDoc->GetRepeatRowRange( nTab ) )         // keinen bestehenden Bereich veraendern
            {
                ScRange aNew( 0, 0, nTab, 0, 0, nTab );     // Default
                pDoc->SetRepeatRowRange( nTab, &aNew );     // einschalten
            }
        }
        else
            pDoc->SetRepeatRowRange( nTab, NULL );          // abschalten

        PrintAreaUndo_Impl( pOldRanges );   // Undo, Umbrueche, Modified etc.

        //! zuletzt gesetzten Bereich beim Abschalten merken und beim Einschalten wiederherstellen ???
    }
}

table::CellRangeAddress SAL_CALL ScTableSheetObj::getTitleRows() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aRet;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();
        const ScRange* pRange = pDoc->GetRepeatRowRange(nTab);
        if (pRange)
            ScUnoConversion::FillAddress( aRet, *pRange );
    }
    return aRet;
}

void SAL_CALL ScTableSheetObj::setTitleRows( const table::CellRangeAddress& aTitleRows )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScPrintRangeSaver* pOldRanges = pDoc->CreatePrintRangeSaver();

        ScRange aNew;
        ScUnoConversion::FillRange( aNew, aTitleRows );
        pDoc->SetRepeatRowRange( nTab, &aNew );     // immer auch einschalten

        PrintAreaUndo_Impl( pOldRanges );           // Undo, Umbrueche, Modified etc.
    }
}

// XSheetLinkable

sheet::SheetLinkMode SAL_CALL ScTableSheetObj::getLinkMode() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sheet::SheetLinkMode eRet = sheet::SheetLinkMode_NONE;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        BYTE nMode = pDocSh->GetDocument()->GetLinkMode( GetTab_Impl() );
        if ( nMode == SC_LINK_NORMAL )
            eRet = sheet::SheetLinkMode_NORMAL;
        else if ( nMode == SC_LINK_VALUE )
            eRet = sheet::SheetLinkMode_VALUE;
    }
    return eRet;
}

void SAL_CALL ScTableSheetObj::setLinkMode( sheet::SheetLinkMode nLinkMode )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! Filter und Options aus altem Link suchen

    rtl::OUString aUrl = getLinkUrl();
    rtl::OUString aSheet = getLinkSheetName();

    rtl::OUString aEmpty;
    link( aUrl, aSheet, aEmpty, aEmpty, nLinkMode );
}

rtl::OUString SAL_CALL ScTableSheetObj::getLinkUrl() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aFile;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        aFile = pDocSh->GetDocument()->GetLinkDoc( GetTab_Impl() );
    return aFile;
}

void SAL_CALL ScTableSheetObj::setLinkUrl( const rtl::OUString& aLinkUrl )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! Filter und Options aus altem Link suchen

    sheet::SheetLinkMode eMode = getLinkMode();
    rtl::OUString aSheet = getLinkSheetName();

    rtl::OUString aEmpty;
    link( aLinkUrl, aSheet, aEmpty, aEmpty, eMode );
}

rtl::OUString SAL_CALL ScTableSheetObj::getLinkSheetName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aSheet;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        aSheet = pDocSh->GetDocument()->GetLinkTab( GetTab_Impl() );
    return aSheet;
}

void SAL_CALL ScTableSheetObj::setLinkSheetName( const rtl::OUString& aLinkSheetName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! Filter und Options aus altem Link suchen

    sheet::SheetLinkMode eMode = getLinkMode();
    rtl::OUString aUrl = getLinkUrl();

    rtl::OUString aEmpty;
    link( aUrl, aLinkSheetName, aEmpty, aEmpty, eMode );
}

void SAL_CALL ScTableSheetObj::link( const rtl::OUString& aUrl, const rtl::OUString& aSheetName,
                        const rtl::OUString& aFilterName, const rtl::OUString& aFilterOptions,
                        sheet::SheetLinkMode nMode ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        String aFileString   = aUrl;
        String aFilterString = aFilterName;
        String aOptString    = aFilterOptions;
        String aSheetString  = aSheetName;

        aFileString = ScGlobal::GetAbsDocName( aFileString, pDocSh );
        if ( !aFilterString.Len() )
            ScDocumentLoader::GetFilterName( aFileString, aFilterString, aOptString );

        BYTE nLinkMode = SC_LINK_NONE;
        if ( nMode == sheet::SheetLinkMode_NORMAL )
            nLinkMode = SC_LINK_NORMAL;
        else if ( nMode == sheet::SheetLinkMode_VALUE )
            nLinkMode = SC_LINK_VALUE;

        pDoc->SetLink( nTab, nLinkMode, aFileString, aFilterString, aOptString, aSheetString );

        pDocSh->UpdateLinks();                  // ggf. Link eintragen oder loeschen
        SfxBindings* pBindings = pDocSh->GetViewBindings();
        if (pBindings)
            pBindings->Invalidate(SID_LINKS);

        //! Undo fuer Link-Daten an der Table

        if ( nLinkMode != SC_LINK_NONE )        // Link updaten
        {
            //  Update immer, auch wenn der Link schon da war
            //! Update nur fuer die betroffene Tabelle???

            SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
            USHORT nCount = pLinkManager->GetLinks().Count();
            for ( USHORT i=0; i<nCount; i++ )
            {
                SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
                if (pBase->ISA(ScTableLink))
                {
                    ScTableLink* pTabLink = (ScTableLink*)pBase;
                    if ( pTabLink->GetFileName() == aFileString )
                        pTabLink->Update();                         // inkl. Paint&Undo

                    //! Der Dateiname sollte nur einmal vorkommen (?)
                }
            }
        }

        //! Notify fuer ScSheetLinkObj Objekte!!!
    }
}

// XSheetAuditing

sal_Bool SAL_CALL ScTableSheetObj::hideDependents( const table::CellAddress& aPosition )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nTab = GetTab_Impl();
        DBG_ASSERT( aPosition.Sheet == nTab, "falsche Tabelle in CellAddress" );
        ScAddress aPos( aPosition.Column, aPosition.Row, nTab );
        ScDocFunc aFunc(*pDocSh);
        return aFunc.DetectiveDelSucc( aPos );
    }
    return FALSE;
}

sal_Bool SAL_CALL ScTableSheetObj::hidePrecedents( const table::CellAddress& aPosition )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nTab = GetTab_Impl();
        DBG_ASSERT( aPosition.Sheet == nTab, "falsche Tabelle in CellAddress" );
        ScAddress aPos( aPosition.Column, aPosition.Row, nTab );
        ScDocFunc aFunc(*pDocSh);
        return aFunc.DetectiveDelPred( aPos );
    }
    return FALSE;
}

sal_Bool SAL_CALL ScTableSheetObj::showDependents( const table::CellAddress& aPosition )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nTab = GetTab_Impl();
        DBG_ASSERT( aPosition.Sheet == nTab, "falsche Tabelle in CellAddress" );
        ScAddress aPos( aPosition.Column, aPosition.Row, nTab );
        ScDocFunc aFunc(*pDocSh);
        return aFunc.DetectiveAddSucc( aPos );
    }
    return FALSE;
}

sal_Bool SAL_CALL ScTableSheetObj::showPrecedents( const table::CellAddress& aPosition )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nTab = GetTab_Impl();
        DBG_ASSERT( aPosition.Sheet == nTab, "falsche Tabelle in CellAddress" );
        ScAddress aPos( aPosition.Column, aPosition.Row, nTab );
        ScDocFunc aFunc(*pDocSh);
        return aFunc.DetectiveAddPred( aPos );
    }
    return FALSE;
}

sal_Bool SAL_CALL ScTableSheetObj::showErrors( const table::CellAddress& aPosition )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nTab = GetTab_Impl();
        DBG_ASSERT( aPosition.Sheet == nTab, "falsche Tabelle in CellAddress" );
        ScAddress aPos( aPosition.Column, aPosition.Row, nTab );
        ScDocFunc aFunc(*pDocSh);
        return aFunc.DetectiveAddError( aPos );
    }
    return FALSE;
}

sal_Bool SAL_CALL ScTableSheetObj::showInvalid() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocFunc aFunc(*pDocSh);
        return aFunc.DetectiveMarkInvalid( GetTab_Impl() );
    }
    return FALSE;
}

void SAL_CALL ScTableSheetObj::clearArrows() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocFunc aFunc(*pDocSh);
        aFunc.DetectiveDelAll( GetTab_Impl() );
    }
}

// XSheetOutline

void SAL_CALL ScTableSheetObj::group( const table::CellRangeAddress& aRange,
                                        table::TableOrientation nOrientation )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        BOOL bColumns = ( nOrientation == table::TableOrientation_COLUMNS );
        ScRange aGroupRange;
        ScUnoConversion::FillRange( aGroupRange, aRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.MakeOutline( aGroupRange, bColumns, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::ungroup( const table::CellRangeAddress& aRange,
                                        table::TableOrientation nOrientation )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        BOOL bColumns = ( nOrientation == table::TableOrientation_COLUMNS );
        ScRange aGroupRange;
        ScUnoConversion::FillRange( aGroupRange, aRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.RemoveOutline( aGroupRange, bColumns, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::autoOutline( const table::CellRangeAddress& aRange )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScRange aFormulaRange;
        ScUnoConversion::FillRange( aFormulaRange, aRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.AutoOutline( aFormulaRange, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::clearOutline() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nTab = GetTab_Impl();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.RemoveAllOutlines( nTab, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::hideDetail( const table::CellRangeAddress& aRange )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScRange aMarkRange;
        ScUnoConversion::FillRange( aMarkRange, aRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.HideMarkedOutlines( aMarkRange, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::showDetail( const table::CellRangeAddress& aRange )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScRange aMarkRange;
        ScUnoConversion::FillRange( aMarkRange, aRange );
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.ShowMarkedOutlines( aMarkRange, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::showLevel( sal_Int16 nLevel, table::TableOrientation nOrientation )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        BOOL bColumns = ( nOrientation == table::TableOrientation_COLUMNS );
        USHORT nTab = GetTab_Impl();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.SelectLevel( nTab, bColumns, nLevel, TRUE, TRUE, TRUE );
    }
}

// XProtectable

void SAL_CALL ScTableSheetObj::protect( const rtl::OUString& aPassword )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        String aString = aPassword;
        ScDocFunc aFunc(*pDocSh);
        aFunc.Protect( GetTab_Impl(), aString, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::unprotect( const rtl::OUString& aPassword )
                            throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        String aString = aPassword;
        ScDocFunc aFunc(*pDocSh);
        aFunc.Unprotect( GetTab_Impl(), aString, TRUE );

        //! Rueckgabewert auswerten, Exception oder so
    }
}

sal_Bool SAL_CALL ScTableSheetObj::isProtected() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocument()->IsTabProtected( GetTab_Impl() );

    DBG_ERROR("keine DocShell");        //! Exception oder so?
    return FALSE;
}

// XScenario

sal_Bool SAL_CALL ScTableSheetObj::getIsScenario() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocument()->IsScenario( GetTab_Impl() );

    return FALSE;
}

rtl::OUString SAL_CALL ScTableSheetObj::getScenarioComment() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        String aComment;
        Color  aColor;
        USHORT nFlags;
        pDocSh->GetDocument()->GetScenarioData( GetTab_Impl(), aComment, aColor, nFlags );
        return aComment;
    }
    return rtl::OUString();
}

void SAL_CALL ScTableSheetObj::setScenarioComment( const rtl::OUString& aScenarioComment )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        String aName;
        String aComment;
        Color  aColor;
        USHORT nFlags;
        pDoc->GetName( nTab, aName );
        pDoc->GetScenarioData( nTab, aComment, aColor, nFlags );

        aComment = String( aScenarioComment );

        pDocSh->ModifyScenario( nTab, aName, aComment, aColor, nFlags );
    }
}

void SAL_CALL ScTableSheetObj::addRanges( const uno::Sequence<table::CellRangeAddress>& aRanges )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        ScMarkData aMarkData;
        aMarkData.SelectTable( nTab, TRUE );

        USHORT nRangeCount = (USHORT)aRanges.getLength();
        if (nRangeCount)
        {
            const table::CellRangeAddress* pAry = aRanges.getConstArray();
            for (USHORT i=0; i<nRangeCount; i++)
            {
                DBG_ASSERT( pAry[i].Sheet == nTab, "addRanges mit falscher Tab" );
                ScRange aRange( pAry[i].StartColumn, pAry[i].StartRow, nTab,
                                pAry[i].EndColumn,   pAry[i].EndRow,   nTab );

                aMarkData.SetMultiMarkArea( aRange );
            }
        }

        //  Szenario-Ranges sind durch Attribut gekennzeichnet
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( ScMergeFlagAttr( SC_MF_SCENARIO ) );
        aPattern.GetItemSet().Put( ScProtectionAttr( TRUE ) );
        ScDocFunc aFunc(*pDocSh);
        aFunc.ApplyAttributes( aMarkData, aPattern, TRUE, TRUE );
    }
}

void SAL_CALL ScTableSheetObj::apply() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();
        String aName;
        pDoc->GetName( nTab, aName );       // Name dieses Szenarios

        USHORT nDestTab = nTab;
        while ( nDestTab > 0 && pDoc->IsScenario(nDestTab) )
            --nDestTab;

        if ( !pDoc->IsScenario(nDestTab) )
            pDocSh->UseScenario( nDestTab, aName );

        //! sonst Fehler oder so
    }
}

// XPropertySet erweitert fuer Sheet-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableSheetObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aSheetPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScTableSheetObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu setzen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
    {
        ScCellRangesBase::setPropertyValue(aPropertyName, aValue);
        return;
    }

    //  eigene Properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;                                                 //! Exception oder so?
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nTab = GetTab_Impl();
    ScDocFunc aFunc(*pDocSh);

    if ( aNameString.EqualsAscii( SC_UNONAME_PAGESTL ) )
    {
        rtl::OUString aStrVal;
        aValue >>= aStrVal;
        String aNewStr = ScStyleNameConversion::ProgrammaticToDisplayName(
                                            aStrVal, SFX_STYLE_FAMILY_PAGE );

        //! Undo? (auch bei SID_STYLE_APPLY an der View)

        if ( pDoc->GetPageStyle( nTab ) != aNewStr )
        {
            pDoc->SetPageStyle( nTab, aNewStr );
            ScPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab ).UpdatePages();
            pDocSh->SetDocumentModified();

            SfxBindings* pBindings = pDocSh->GetViewBindings();
            if (pBindings)
            {
                pBindings->Invalidate( SID_STYLE_FAMILY4 );
                pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                pBindings->Invalidate( FID_RESET_PRINTZOOM );
            }
        }
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
    {
        BOOL bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        aFunc.SetTableVisible( nTab, bVis, TRUE );
    }
    else
        ScCellRangeObj::setPropertyValue(aPropertyName, aValue);        // geerbte ohne WID
}

uno::Any SAL_CALL ScTableSheetObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu bekommen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
        return ScCellRangesBase::getPropertyValue(aPropertyName);

    //  eigene Properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return uno::Any();                          //! Exception oder so?
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nTab = GetTab_Impl();

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_PAGESTL ) )
    {
        String aStyle = ScStyleNameConversion::DisplayToProgrammaticName(
                            pDoc->GetPageStyle( nTab ), SFX_STYLE_FAMILY_PAGE );
        aAny <<= rtl::OUString( aStyle );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
    {
        BOOL bVis = pDoc->IsVisible( nTab );
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString.EqualsAscii( SC_UNO_LINKDISPBIT ) )
    {
        //  no target bitmaps for individual entries (would be all equal)
        // ScLinkTargetTypeObj::SetLinkTargetBitmap( aAny, SC_LINKTARGETTYPE_SHEET );
    }
    else if ( aNameString.EqualsAscii( SC_UNO_LINKDISPNAME ) )
    {
        //  LinkDisplayName for hyperlink dialog
        aAny <<= getName();     // sheet name
    }
    else
        aAny = ScCellRangeObj::getPropertyValue(aPropertyName);     // geerbte ohne WID

    return aAny;
}

// XServiceInfo

rtl::OUString SAL_CALL ScTableSheetObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScTableSheetObj" );
}

sal_Bool SAL_CALL ScTableSheetObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCSPREADSHEET_SERVICE ) ||
           aServiceStr.EqualsAscii( SCSHEETCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCLINKTARGET_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScTableSheetObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(6);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCSPREADSHEET_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCSHEETCELLRANGE_SERVICE );
    pArray[2] = rtl::OUString::createFromAscii( SCCELLRANGE_SERVICE );
    pArray[3] = rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[4] = rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    pArray[5] = rtl::OUString::createFromAscii( SCLINKTARGET_SERVICE );
    return aRet;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScTableSheetObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }

    return ScCellRangeObj::getSomething( rId );
}

// static
const uno::Sequence<sal_Int8>& ScTableSheetObj::getUnoTunnelId()
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
ScTableSheetObj* ScTableSheetObj::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScTableSheetObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScTableSheetObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScTableColumnObj::ScTableColumnObj( ScDocShell* pDocSh, USHORT nCol, USHORT nTab ) :
    ScCellRangeObj( pDocSh, ScRange(nCol,0,nTab, nCol,MAXROW,nTab) ),
    aColPropSet(lcl_GetColumnPropertyMap())
{
}

ScTableColumnObj::~ScTableColumnObj()
{
}

uno::Any SAL_CALL ScTableColumnObj::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( container::XNamed )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScTableColumnObj::acquire() throw(uno::RuntimeException)
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScTableColumnObj::release() throw(uno::RuntimeException)
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScTableColumnObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangeObj::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 1 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<container::XNamed>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScTableColumnObj::getImplementationId() throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XNamed

rtl::OUString SAL_CALL ScTableColumnObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    const ScRange& rRange = GetRange();
    DBG_ASSERT(rRange.aStart.Col() == rRange.aEnd.Col(), "too many columns");
    USHORT nCol = rRange.aStart.Col();

    return ColToAlpha( nCol );      // from global.hxx
}

void SAL_CALL ScTableColumnObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    throw uno::RuntimeException();      // read-only
}

// XPropertySet erweitert fuer Spalten-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableColumnObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aColPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScTableColumnObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu setzen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
    {
        ScCellRangesBase::setPropertyValue(aPropertyName, aValue);
        return;
    }

    //  eigene Properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;                                                 //! Exception oder so?
    ScDocument* pDoc = pDocSh->GetDocument();
    const ScRange& rRange = GetRange();
    DBG_ASSERT(rRange.aStart.Col() == rRange.aEnd.Col(), "zuviele Spalten");
    USHORT nCol = rRange.aStart.Col();
    USHORT nTab = rRange.aStart.Tab();
    ScDocFunc aFunc(*pDocSh);

    USHORT nColArr[2];
    nColArr[0] = nColArr[1] = nCol;

    if ( aNameString.EqualsAscii( SC_UNONAME_CELLWID ) )
    {
        sal_Int32 nNewWidth;
        if ( aValue >>= nNewWidth )
        {
            //  property is 1/100mm, column width is twips
            nNewWidth = HMMToTwips(nNewWidth);
            aFunc.SetWidthOrHeight( TRUE, 1, nColArr, nTab, SC_SIZE_DIRECT,
                                    (USHORT)nNewWidth, TRUE, TRUE );
        }
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
    {
        BOOL bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        aFunc.SetWidthOrHeight( TRUE, 1, nColArr, nTab, eMode, 0, TRUE, TRUE );
        //  SC_SIZE_DIRECT mit Groesse 0 blendet aus
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_OWIDTH ) )
    {
        BOOL bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bOpt)
            aFunc.SetWidthOrHeight( TRUE, 1, nColArr, nTab,
                                    SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH, TRUE, TRUE );
        // FALSE bei Spalten momentan ohne Auswirkung
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE ) || aNameString.EqualsAscii( SC_UNONAME_MANPAGE ) )
    {
        BOOL bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bSet)
            aFunc.InsertPageBreak( TRUE, rRange.aStart, TRUE, TRUE, TRUE );
        else
            aFunc.RemovePageBreak( TRUE, rRange.aStart, TRUE, TRUE, TRUE );
    }
    else
        ScCellRangeObj::setPropertyValue(aPropertyName, aValue);        // geerbte ohne WID
}

uno::Any SAL_CALL ScTableColumnObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu bekommen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
        return ScCellRangesBase::getPropertyValue(aPropertyName);

    //  eigene Properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return uno::Any();                          //! Exception oder so?
    ScDocument* pDoc = pDocSh->GetDocument();
    const ScRange& rRange = GetRange();
    DBG_ASSERT(rRange.aStart.Col() == rRange.aEnd.Col(), "zuviele Spalten");
    USHORT nCol = rRange.aStart.Col();
    USHORT nTab = rRange.aStart.Tab();

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_CELLWID ) )
    {
        USHORT nWidth = pDoc->GetColWidth( nCol, nTab );
        //  Property ist 1/100mm, Spaltenbreite in Twips
        nWidth = (USHORT) TwipsToHMM(nWidth);
        aAny <<= (sal_Int32)( nWidth );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
    {
        BOOL bVis = !(pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_OWIDTH ) )
    {
        //! momentan immer gesetzt ??!?!
        BOOL bOpt = !(pDoc->GetColFlags( nCol, nTab ) & CR_MANUALSIZE);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE ) )
    {
        BOOL bBreak = ( 0 != (pDoc->GetColFlags( nCol, nTab ) & (CR_PAGEBREAK|CR_MANUALBREAK)) );
        ScUnoHelpFunctions::SetBoolInAny( aAny, bBreak );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_MANPAGE ) )
    {
        BOOL bBreak = ( 0 != (pDoc->GetColFlags( nCol, nTab ) & (CR_MANUALBREAK)) );
        ScUnoHelpFunctions::SetBoolInAny( aAny, bBreak );
    }
    else
        aAny = ScCellRangeObj::getPropertyValue(aPropertyName);     // geerbte ohne WID

    return aAny;
}

//------------------------------------------------------------------------

ScTableRowObj::ScTableRowObj(ScDocShell* pDocSh, USHORT nRow, USHORT nTab) :
    ScCellRangeObj( pDocSh, ScRange(0,nRow,nTab, MAXCOL,nRow,nTab) ),
    aRowPropSet(lcl_GetRowPropertyMap())
{
}

ScTableRowObj::~ScTableRowObj()
{
}

// XPropertySet erweitert fuer Zeilen-Properties

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableRowObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aRowPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScTableRowObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu setzen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
    {
        ScCellRangesBase::setPropertyValue(aPropertyName, aValue);
        return;
    }

    //  eigene Properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return;                                                 //! Exception oder so?
    ScDocument* pDoc = pDocSh->GetDocument();
    const ScRange& rRange = GetRange();
    DBG_ASSERT(rRange.aStart.Row() == rRange.aEnd.Row(), "zuviele Zeilen");
    USHORT nRow = rRange.aStart.Row();
    USHORT nTab = rRange.aStart.Tab();
    ScDocFunc aFunc(*pDocSh);

    USHORT nRowArr[2];
    nRowArr[0] = nRowArr[1] = nRow;

    if ( aNameString.EqualsAscii( SC_UNONAME_CELLHGT ) )
    {
        sal_Int32 nNewHeight;
        if ( aValue >>= nNewHeight )
        {
            //  property is 1/100mm, row height is twips
            nNewHeight = HMMToTwips(nNewHeight);
            aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, SC_SIZE_DIRECT,
                                    (USHORT)nNewHeight, TRUE, TRUE );
        }
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
    {
        BOOL bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, eMode, 0, TRUE, TRUE );
        //  SC_SIZE_DIRECT mit Groesse 0 blendet aus
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLFILT ) )
    {
        BOOL bFil = ScUnoHelpFunctions::GetBoolFromAny( aValue );
//      ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
//      aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, eMode, 0, TRUE, TRUE );
        //  SC_SIZE_DIRECT mit Groesse 0 blendet aus
        Byte nFlags = pDoc->GetRowFlags(nRow, nTab);
        if (bFil)
            nFlags |= CR_FILTERED;
        else
            nFlags &= ~CR_FILTERED;
        pDoc->SetRowFlags(nRow, nTab, nFlags);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_OHEIGHT ) )
    {
        BOOL bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bOpt)
            aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, SC_SIZE_OPTIMAL, 0, TRUE, TRUE );
        else
        {
            //  alte Hoehe nochmal manuell setzen
            USHORT nHeight = pDoc->GetRowHeight( nRow, nTab );
            aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, SC_SIZE_DIRECT, nHeight, TRUE, TRUE );
        }
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE) || aNameString.EqualsAscii( SC_UNONAME_MANPAGE) )
    {
        BOOL bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bSet)
            aFunc.InsertPageBreak( FALSE, rRange.aStart, TRUE, TRUE, TRUE );
        else
            aFunc.RemovePageBreak( FALSE, rRange.aStart, TRUE, TRUE, TRUE );
    }
    else
        ScCellRangeObj::setPropertyValue(aPropertyName, aValue);        // geerbte ohne WID
}

uno::Any SAL_CALL ScTableRowObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    //  Abkuerzung, um Attributs-Properties relativ schnell zu bekommen

    const SfxItemPropertyMap* pMap =
            SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aNameString );
    if ( pMap && pMap->nWID )
        return ScCellRangesBase::getPropertyValue(aPropertyName);

    //  eigene Properties

    ScDocShell* pDocSh = GetDocShell();
    if (!pDocSh)
        return uno::Any();          //! Exception oder so?
    ScDocument* pDoc = pDocSh->GetDocument();
    const ScRange& rRange = GetRange();
    DBG_ASSERT(rRange.aStart.Row() == rRange.aEnd.Row(), "zuviele Zeilen");
    USHORT nRow = rRange.aStart.Row();
    USHORT nTab = rRange.aStart.Tab();

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_CELLHGT ) )
    {
        USHORT nHeight = pDoc->GetRowHeight( nRow, nTab );
        //  Property ist 1/100mm, Zeilenhoehe in Twips
        nHeight = (USHORT) TwipsToHMM(nHeight);
        aAny <<= (sal_Int32)( nHeight );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
    {
        BOOL bVis = !(pDoc->GetRowFlags( nRow, nTab ) & CR_HIDDEN);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLFILT ) )
    {
        BOOL bVis = ((pDoc->GetRowFlags( nRow, nTab ) & CR_FILTERED) != 0);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_OHEIGHT ) )
    {
        BOOL bOpt = !(pDoc->GetRowFlags( nRow, nTab ) & CR_MANUALSIZE);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE ) )
    {
        BOOL bBreak = ( 0 != (pDoc->GetRowFlags( nRow, nTab ) & (CR_PAGEBREAK|CR_MANUALBREAK)) );
        ScUnoHelpFunctions::SetBoolInAny( aAny, bBreak );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_MANPAGE ) )
    {
        BOOL bBreak = ( 0 != (pDoc->GetRowFlags( nRow, nTab ) & (CR_MANUALBREAK)) );
        ScUnoHelpFunctions::SetBoolInAny( aAny, bBreak );
    }
    else
        aAny = ScCellRangeObj::getPropertyValue(aPropertyName);     // geerbte ohne WID

    return aAny;
}

//------------------------------------------------------------------------

ScCellsObj::ScCellsObj(ScDocShell* pDocSh, const ScRangeList& rR) :
    pDocShell( pDocSh ),
    aRanges( rR )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellsObj::~ScCellsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScCellsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;
        aRanges.UpdateReference( rRef.GetMode(), pDocShell->GetDocument(), rRef.GetRange(),
                                        rRef.GetDx(), rRef.GetDy(), rRef.GetDz() );
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScCellsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
        return new ScCellsEnumeration( pDocShell, aRanges );
    return NULL;
}

uno::Type SAL_CALL ScCellsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<table::XCell>*)0);
}

sal_Bool SAL_CALL ScCellsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bHas = FALSE;
    if ( pDocShell )
    {
        //! schneller selber testen?

        uno::Reference<container::XEnumeration> xEnum = new ScCellsEnumeration( pDocShell, aRanges );
        bHas = xEnum->hasMoreElements();
    }
    return bHas;
}

//------------------------------------------------------------------------

ScCellsEnumeration::ScCellsEnumeration(ScDocShell* pDocSh, const ScRangeList& rR) :
    pDocShell( pDocSh ),
    aRanges( rR ),
    bAtEnd( FALSE ),
    pMark( NULL )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->AddUnoObject(*this);

    if ( aRanges.Count() == 0 )
        bAtEnd = TRUE;
    else
    {
        USHORT nTab = 0;
        const ScRange* pFirst = aRanges.GetObject(0);
        if (pFirst)
            nTab = pFirst->aStart.Tab();
        aPos = ScAddress(0,0,nTab);
        CheckPos_Impl();                    // aPos auf erste passende Zelle setzen
    }
}

void ScCellsEnumeration::CheckPos_Impl()
{
    if (pDocShell)
    {
        BOOL bFound = FALSE;
        ScDocument* pDoc = pDocShell->GetDocument();
        ScBaseCell* pCell = pDoc->GetCell(aPos);
        if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
        {
            if (!pMark)
            {
                pMark = new ScMarkData;
                pMark->MarkFromRangeList( aRanges, FALSE );
                pMark->MarkToMulti();   // needed for GetNextMarkedCell
            }
            bFound = pMark->IsCellMarked( aPos.Col(), aPos.Row() );
        }
        if (!bFound)
            Advance_Impl();
    }
}

ScCellsEnumeration::~ScCellsEnumeration()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
    delete pMark;
}

void ScCellsEnumeration::Advance_Impl()
{
    DBG_ASSERT(!bAtEnd,"zuviel Advance_Impl");
    if (!pMark)
    {
        pMark = new ScMarkData;
        pMark->MarkFromRangeList( aRanges, FALSE );
        pMark->MarkToMulti();   // needed for GetNextMarkedCell
    }

    USHORT nCol = aPos.Col();
    USHORT nRow = aPos.Row();
    USHORT nTab = aPos.Tab();
    BOOL bFound = pDocShell->GetDocument()->GetNextMarkedCell( nCol, nRow, nTab, *pMark );
    if (bFound)
        aPos.Set( nCol, nRow, nTab );
    else
        bAtEnd = TRUE;      // kommt nix mehr
}

void ScCellsEnumeration::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        if (pDocShell)
        {
            const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;
            aRanges.UpdateReference( rRef.GetMode(), pDocShell->GetDocument(), rRef.GetRange(),
                                            rRef.GetDx(), rRef.GetDy(), rRef.GetDz() );

            delete pMark;       // aus verschobenen Bereichen neu erzeugen
            pMark = NULL;

            if (!bAtEnd)        // aPos anpassen
            {
                ScRangeList aNew;
                aNew.Append(ScRange(aPos));
                aNew.UpdateReference( rRef.GetMode(), pDocShell->GetDocument(), rRef.GetRange(),
                                        rRef.GetDx(), rRef.GetDy(), rRef.GetDz() );
                if (aNew.Count()==1)
                {
                    aPos = aNew.GetObject(0)->aStart;
                    CheckPos_Impl();
                }
            }
        }
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XEnumeration

sal_Bool SAL_CALL ScCellsEnumeration::hasMoreElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return !bAtEnd;
}

uno::Any SAL_CALL ScCellsEnumeration::nextElement() throw(container::NoSuchElementException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell && !bAtEnd)
    {
        // Interface-Typ muss zu ScCellsObj::getElementType passen

        uno::Any aAny;
        uno::Reference<table::XCell> xCell = new ScCellObj( pDocShell, aPos );
        aAny <<= xCell;
        Advance_Impl();
        return aAny;
    }

    throw container::NoSuchElementException();      // no more elements
    return uno::Any();
}

//------------------------------------------------------------------------

ScCellFormatsObj::ScCellFormatsObj(ScDocShell* pDocSh, const ScRange& rRange) :
    pDocShell( pDocSh ),
    aTotalRange( rRange )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->AddUnoObject(*this);

    DBG_ASSERT( aTotalRange.aStart.Tab() == aTotalRange.aEnd.Tab(), "unterschiedliche Tabellen" );
}

ScCellFormatsObj::~ScCellFormatsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScCellFormatsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        //! aTotalRange...
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

ScCellRangeObj* ScCellFormatsObj::GetObjectByIndex_Impl(long nIndex) const
{
    //! direkt auf die AttrArrays zugreifen !!!!

    ScCellRangeObj* pRet = NULL;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        long nPos = 0;
        ScAttrRectIterator aIter( pDoc, aTotalRange.aStart.Tab(),
                                    aTotalRange.aStart.Col(), aTotalRange.aStart.Row(),
                                    aTotalRange.aEnd.Col(), aTotalRange.aEnd.Row() );
        USHORT nCol1, nCol2, nRow1, nRow2;
        while ( aIter.GetNext( nCol1, nCol2, nRow1, nRow2 ) )
        {
            if ( nPos == nIndex )
            {
                USHORT nTab = aTotalRange.aStart.Tab();
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

sal_Int32 SAL_CALL ScCellFormatsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! direkt auf die AttrArrays zugreifen !!!!

    long nCount = 0;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScAttrRectIterator aIter( pDoc, aTotalRange.aStart.Tab(),
                                    aTotalRange.aStart.Col(), aTotalRange.aStart.Row(),
                                    aTotalRange.aEnd.Col(), aTotalRange.aEnd.Row() );
        USHORT nCol1, nCol2, nRow1, nRow2;
        while ( aIter.GetNext( nCol1, nCol2, nRow1, nRow2 ) )
            ++nCount;
    }
    return nCount;
}

uno::Any SAL_CALL ScCellFormatsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<table::XCellRange> xRange = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xRange.is())
        aAny <<= xRange;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScCellFormatsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<table::XCellRange>*)0);
}

sal_Bool SAL_CALL ScCellFormatsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );     //! immer groesser 0 ??
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScCellFormatsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
        return new ScCellFormatsEnumeration( pDocShell, aTotalRange );
    return NULL;
}

//------------------------------------------------------------------------

ScCellFormatsEnumeration::ScCellFormatsEnumeration(ScDocShell* pDocSh, const ScRange& rRange) :
    pDocShell( pDocSh ),
    nTab( rRange.aStart.Tab() ),
    pIter( NULL ),
    bAtEnd( FALSE ),
    bDirty( FALSE )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->AddUnoObject(*this);

    DBG_ASSERT( rRange.aStart.Tab() == rRange.aEnd.Tab(),
                "CellFormatsEnumeration: unterschiedliche Tabellen" );

    pIter = new ScAttrRectIterator( pDoc, nTab,
                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row() );
    Advance_Impl();
}

ScCellFormatsEnumeration::~ScCellFormatsEnumeration()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
    delete pIter;
}

void ScCellFormatsEnumeration::Advance_Impl()
{
    DBG_ASSERT(!bAtEnd,"zuviel Advance_Impl");

    if ( pIter )
    {
        if ( bDirty )
        {
            pIter->DataChanged();   // AttrArray-Index neu suchen
            bDirty = FALSE;
        }

        USHORT nCol1, nCol2, nRow1, nRow2;
        if ( pIter->GetNext( nCol1, nCol2, nRow1, nRow2 ) )
            aNext = ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
        else
            bAtEnd = TRUE;      // kommt nix mehr
    }
    else
        bAtEnd = TRUE;          // Dok weggekommen oder so
}

ScCellRangeObj* ScCellFormatsEnumeration::NextObject_Impl()
{
    ScCellRangeObj* pRet = NULL;
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

void ScCellFormatsEnumeration::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        //! und nun ???
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;                       // ungueltig geworden
            delete pIter;
            pIter = NULL;
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            bDirty = TRUE;          // AttrArray-Index evtl. ungueltig geworden
        }
    }
}

// XEnumeration

sal_Bool SAL_CALL ScCellFormatsEnumeration::hasMoreElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return !bAtEnd;
}

uno::Any SAL_CALL ScCellFormatsEnumeration::nextElement() throw(container::NoSuchElementException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( bAtEnd || !pDocShell )
        throw container::NoSuchElementException();      // no more elements

    // Interface-Typ muss zu ScCellFormatsObj::getElementType passen

    uno::Any aAny;
    uno::Reference<table::XCellRange> xRange = NextObject_Impl();
    aAny <<= xRange;
    return aAny;
}




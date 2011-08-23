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

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "scitems.hxx"
#include <bf_svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD



#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

#ifndef _SVX_ITEMDATA_HXX
#include <bf_svx/itemdata.hxx>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#include <vcl/svapp.hxx>
#include <bf_svx/flditem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/linkmgr.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_sch/memchrt.hxx>
#include <bf_svtools/zformat.hxx>
#include <rtl/uuid.h>
#include <float.h>				// DBL_MIN

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>

#include "autoform.hxx"
#include "cursuno.hxx"
#include "textuno.hxx"
#include "editsrc.hxx"
#include "notesuno.hxx"
#include "fielduno.hxx"
#include "docuno.hxx"		// ScTableColumnsObj etc
#include "datauno.hxx"
#include "dapiuno.hxx"
#include "chartuno.hxx"
#include "fmtuno.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "targuno.hxx"
#include "docsh.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "docfunc.hxx"
#include "dbdocfun.hxx"
#include "olinefun.hxx"
#include "hints.hxx"
#include "cell.hxx"
#include "undotab.hxx"
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
#include "validat.hxx"
#include "bf_sc.hrc"
#include "unoguard.hxx"
#include "cellform.hxx"
#include "globstr.hrc"
#include "unonames.hxx"
#include "styleuno.hxx"
#include "rangeseq.hxx"
#include "unowids.hxx"

#ifndef __SGI_STL_LIST
#include <list>
#endif

namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

//!	ScLinkListener in anderes File verschieben !!!

class ScLinkListener : public SfxListener
{
    Link	aLink;
public:
                    ScLinkListener(const Link& rL) : aLink(rL) {}
    virtual			~ScLinkListener();
    virtual void	Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


class ScNamedEntry
{
    String	aName;
    ScRange	aRange;

public:
            ScNamedEntry(const String& rN, const ScRange& rR) :
                aName(rN), aRange(rR) {}

    const String&	GetName() const		{ return aName; }
    const ScRange&	GetRange() const	{ return aRange; }
};


//------------------------------------------------------------------------

//	Die Namen in den Maps muessen (nach strcmp) sortiert sein!
//!	statt Which-ID 0 special IDs verwenden, und nicht ueber Namen vergleichen !!!!!!!!!

//	Left/Right/Top/BottomBorder are mapped directly to the core items,
//	not collected/applied to the borders of a range -> ATTR_BORDER can be used directly

const SfxItemPropertyMap* lcl_GetCellsPropertyMap()
{
    static SfxItemPropertyMap aCellsPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK),	ATTR_BACKGROUND,	&getCppuType((sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),	ATTR_PROTECTION,	&getCppuType((util::CellProtection*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL),	SC_WID_UNO_CELLSTYL,&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),	ATTR_FONT_COLOR,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),	ATTR_FONT_CONTOUR,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),	ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),					0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CEMPHAS),	ATTR_FONT_EMPHASISMARK,&getCppuType((sal_Int16*)0),			0, MID_EMPHASIS },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
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
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),	ATTR_FONT_HEIGHT,	&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),	ATTR_CJK_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),	ATTR_CTL_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),	ATTR_FONT_POSTURE,	&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),	ATTR_CJK_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),	ATTR_CTL_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CRELIEF),	ATTR_FONT_RELIEF,	&getCppuType((sal_Int16*)0),			0, MID_RELIEF },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),	ATTR_FONT_SHADOWED,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CSTRIKE),	ATTR_FONT_CROSSEDOUT,&getCppuType((sal_Int16*)0),			0, MID_CROSS_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),			0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLCOL),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int32*)0),			0, MID_UL_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLHAS),	ATTR_FONT_UNDERLINE,&getBooleanCppuType(),					0, MID_UL_HASCOLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),	ATTR_FONT_WEIGHT,	&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),	ATTR_CJK_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),	ATTR_CTL_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CWORDMOD),	ATTR_FONT_WORDLINE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR),	SC_WID_UNO_CHCOLHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR),	SC_WID_UNO_CHROWHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),	SC_WID_UNO_CONDFMT,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),	SC_WID_UNO_CONDLOC,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDXML),	SC_WID_UNO_CONDXML,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS),	ATTR_HOR_JUSTIFY,	&getCppuType((table::CellHoriJustify*)0), 0, MID_HORJUST_HORJUST },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN),	ATTR_BACKGROUND,	&getBooleanCppuType(),					0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),		ATTR_LINEBREAK,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),	ATTR_VALUE_FORMAT,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMRULES),	SC_WID_UNO_NUMRULES,&getCppuType((const uno::Reference<container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),	ATTR_ORIENTATION,	&getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PADJUST),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),	ATTR_INDENT,		&getCppuType((sal_Int16*)0),			0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHANG),	ATTR_HANGPUNCTUATION,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PLASTADJ),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),	ATTR_ROTATE_VALUE,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),	ATTR_ROTATE_MODE,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),	ATTR_SHADOW,		&getCppuType((table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),	SC_WID_UNO_TBLBORD,	&getCppuType((table::TableBorder*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),	SC_WID_UNO_VALIDAT,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),	SC_WID_UNO_VALILOC,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIXML),	SC_WID_UNO_VALIXML,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS),	ATTR_VER_JUSTIFY,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),	ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
        {0,0,0,0}
    };
    return aCellsPropertyMap_Impl;
}

//	CellRange enthaelt alle Eintraege von Cells, zusaetzlich eigene Eintraege
//	mit Which-ID 0 (werden nur fuer getPropertySetInfo benoetigt).

const SfxItemPropertyMap* lcl_GetRangePropertyMap()
{
    static SfxItemPropertyMap aRangePropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK),	ATTR_BACKGROUND,	&getCppuType((sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),	ATTR_PROTECTION,	&getCppuType((util::CellProtection*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL),	SC_WID_UNO_CELLSTYL,&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),	ATTR_FONT_COLOR,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),	ATTR_FONT_CONTOUR,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),	ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),					0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CEMPHAS),	ATTR_FONT_EMPHASISMARK,&getCppuType((sal_Int16*)0),			0, MID_EMPHASIS },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
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
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),	ATTR_FONT_HEIGHT,	&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),	ATTR_CJK_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),	ATTR_CTL_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),	ATTR_FONT_POSTURE,	&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),	ATTR_CJK_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),	ATTR_CTL_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CRELIEF),	ATTR_FONT_RELIEF,	&getCppuType((sal_Int16*)0),			0, MID_RELIEF },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),	ATTR_FONT_SHADOWED,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CSTRIKE),	ATTR_FONT_CROSSEDOUT,&getCppuType((sal_Int16*)0),			0, MID_CROSS_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),			0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLCOL),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int32*)0),			0, MID_UL_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLHAS),	ATTR_FONT_UNDERLINE,&getBooleanCppuType(),					0, MID_UL_HASCOLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),	ATTR_FONT_WEIGHT,	&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),	ATTR_CJK_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),	ATTR_CTL_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CWORDMOD),	ATTR_FONT_WORDLINE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR),	SC_WID_UNO_CHCOLHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR),	SC_WID_UNO_CHROWHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),	SC_WID_UNO_CONDFMT,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),	SC_WID_UNO_CONDLOC,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDXML),	SC_WID_UNO_CONDXML,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS),	ATTR_HOR_JUSTIFY,	&getCppuType((table::CellHoriJustify*)0),	0, MID_HORJUST_HORJUST },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN),	ATTR_BACKGROUND,	&getBooleanCppuType(),					0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),		ATTR_LINEBREAK,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),	ATTR_VALUE_FORMAT,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMRULES),	SC_WID_UNO_NUMRULES,&getCppuType((const uno::Reference<container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),	ATTR_ORIENTATION,	&getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PADJUST),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),	ATTR_INDENT,		&getCppuType((sal_Int16*)0),			0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHANG),	ATTR_HANGPUNCTUATION,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PLASTADJ),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),		SC_WID_UNO_POS,		&getCppuType((awt::Point*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),	ATTR_ROTATE_VALUE,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),	ATTR_ROTATE_MODE,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),	ATTR_SHADOW,		&getCppuType((table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),		SC_WID_UNO_SIZE,	&getCppuType((awt::Size*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),	SC_WID_UNO_TBLBORD,	&getCppuType((table::TableBorder*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),	SC_WID_UNO_VALIDAT,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),	SC_WID_UNO_VALILOC,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIXML),	SC_WID_UNO_VALIXML,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS),	ATTR_VER_JUSTIFY,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),	ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
        {0,0,0,0}
    };
    return aRangePropertyMap_Impl;
}

//	Cell enthaelt alle Eintraege von CellRange, zusaetzlich eigene Eintraege
//	mit Which-ID 0 (werden nur fuer getPropertySetInfo benoetigt).

const SfxItemPropertyMap* lcl_GetCellPropertyMap()
{
    static SfxItemPropertyMap aCellPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK),	ATTR_BACKGROUND,	&getCppuType((sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),	ATTR_PROTECTION,	&getCppuType((util::CellProtection*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL),	SC_WID_UNO_CELLSTYL,&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),	ATTR_FONT_COLOR,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),	ATTR_FONT_CONTOUR,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),	ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),					0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CEMPHAS),	ATTR_FONT_EMPHASISMARK,&getCppuType((sal_Int16*)0),			0, MID_EMPHASIS },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
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
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),	ATTR_FONT_HEIGHT,	&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),	ATTR_CJK_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),	ATTR_CTL_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),	ATTR_FONT_POSTURE,	&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),	ATTR_CJK_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),	ATTR_CTL_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CRELIEF),	ATTR_FONT_RELIEF,	&getCppuType((sal_Int16*)0),			0, MID_RELIEF },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),	ATTR_FONT_SHADOWED,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CSTRIKE),	ATTR_FONT_CROSSEDOUT,&getCppuType((sal_Int16*)0),			0, MID_CROSS_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),			0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLCOL),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int32*)0),			0, MID_UL_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLHAS),	ATTR_FONT_UNDERLINE,&getBooleanCppuType(),					0, MID_UL_HASCOLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),	ATTR_FONT_WEIGHT,	&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),	ATTR_CJK_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),	ATTR_CTL_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CWORDMOD),	ATTR_FONT_WORDLINE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR),	SC_WID_UNO_CHCOLHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR),	SC_WID_UNO_CHROWHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),	SC_WID_UNO_CONDFMT,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),	SC_WID_UNO_CONDLOC,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDXML),	SC_WID_UNO_CONDXML,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FORMLOC),	SC_WID_UNO_FORMLOC,	&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FORMRT),	SC_WID_UNO_FORMRT,	&getCppuType((table::CellContentType*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS),	ATTR_HOR_JUSTIFY,	&getCppuType((table::CellHoriJustify*)0), 0, MID_HORJUST_HORJUST },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN),	ATTR_BACKGROUND,	&getBooleanCppuType(),					0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),		ATTR_LINEBREAK,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),	ATTR_VALUE_FORMAT,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMRULES),	SC_WID_UNO_NUMRULES,&getCppuType((const uno::Reference<container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),	ATTR_ORIENTATION,	&getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PADJUST),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),	ATTR_INDENT,		&getCppuType((sal_Int16*)0),			0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHANG),	ATTR_HANGPUNCTUATION,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PLASTADJ),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),		SC_WID_UNO_POS,		&getCppuType((awt::Point*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),	ATTR_ROTATE_VALUE,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),	ATTR_ROTATE_MODE,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),	ATTR_SHADOW,		&getCppuType((table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),		SC_WID_UNO_SIZE,	&getCppuType((awt::Size*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),	SC_WID_UNO_TBLBORD,	&getCppuType((table::TableBorder*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),	SC_WID_UNO_VALIDAT,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),	SC_WID_UNO_VALILOC,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIXML),	SC_WID_UNO_VALIXML,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS),	ATTR_VER_JUSTIFY,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),	ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
        {0,0,0,0}
    };
    return aCellPropertyMap_Impl;
}

//	Column und Row enthalten alle Eintraege von CellRange, zusaetzlich eigene Eintraege
//	mit Which-ID 0 (werden nur fuer getPropertySetInfo benoetigt).

const SfxItemPropertyMap* lcl_GetColumnPropertyMap()
{
    static SfxItemPropertyMap aColumnPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK),	ATTR_BACKGROUND,	&getCppuType((sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),	ATTR_PROTECTION,	&getCppuType((util::CellProtection*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL),	SC_WID_UNO_CELLSTYL,&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),	ATTR_FONT_COLOR,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),	ATTR_FONT_CONTOUR,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),	ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),					0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CEMPHAS),	ATTR_FONT_EMPHASISMARK,&getCppuType((sal_Int16*)0),			0, MID_EMPHASIS },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
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
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),	ATTR_FONT_HEIGHT,	&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),	ATTR_CJK_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),	ATTR_CTL_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),	ATTR_FONT_POSTURE,	&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),	ATTR_CJK_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),	ATTR_CTL_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CRELIEF),	ATTR_FONT_RELIEF,	&getCppuType((sal_Int16*)0),			0, MID_RELIEF },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),	ATTR_FONT_SHADOWED,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CSTRIKE),	ATTR_FONT_CROSSEDOUT,&getCppuType((sal_Int16*)0),			0, MID_CROSS_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),			0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLCOL),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int32*)0),			0, MID_UL_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLHAS),	ATTR_FONT_UNDERLINE,&getBooleanCppuType(),					0, MID_UL_HASCOLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),	ATTR_FONT_WEIGHT,	&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),	ATTR_CJK_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),	ATTR_CTL_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CWORDMOD),	ATTR_FONT_WORDLINE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR),	SC_WID_UNO_CHCOLHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR),	SC_WID_UNO_CHROWHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),	SC_WID_UNO_CONDFMT,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),	SC_WID_UNO_CONDLOC,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDXML),	SC_WID_UNO_CONDXML,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS),	ATTR_HOR_JUSTIFY,	&getCppuType((table::CellHoriJustify*)0), 0, MID_HORJUST_HORJUST },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN),	ATTR_BACKGROUND,	&getBooleanCppuType(),					0, MID_GRAPHIC_TRANSPARENT },
//		{MAP_CHAR_LEN(SC_UNONAME_CELLFILT),	SC_WID_UNO_CELLFILT,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_MANPAGE),	SC_WID_UNO_MANPAGE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NEWPAGE),	SC_WID_UNO_NEWPAGE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),		ATTR_LINEBREAK,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVIS),	SC_WID_UNO_CELLVIS,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),	ATTR_VALUE_FORMAT,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMRULES),	SC_WID_UNO_NUMRULES,&getCppuType((const uno::Reference<container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_OWIDTH),	SC_WID_UNO_OWIDTH,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),	ATTR_ORIENTATION,	&getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PADJUST),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),	ATTR_INDENT,		&getCppuType((sal_Int16*)0),			0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHANG),	ATTR_HANGPUNCTUATION,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PLASTADJ),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),		SC_WID_UNO_POS,		&getCppuType((awt::Point*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),	ATTR_ROTATE_VALUE,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),	ATTR_ROTATE_MODE,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),	ATTR_SHADOW,		&getCppuType((table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),		SC_WID_UNO_SIZE,	&getCppuType((awt::Size*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),	SC_WID_UNO_TBLBORD,	&getCppuType((table::TableBorder*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),	SC_WID_UNO_VALIDAT,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),	SC_WID_UNO_VALILOC,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIXML),	SC_WID_UNO_VALIXML,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS),	ATTR_VER_JUSTIFY,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLWID),	SC_WID_UNO_CELLWID,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),	ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
        {0,0,0,0}
    };
    return aColumnPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetRowPropertyMap()
{
    static SfxItemPropertyMap aRowPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK),	ATTR_BACKGROUND,	&getCppuType((sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),	ATTR_PROTECTION,	&getCppuType((util::CellProtection*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL),	SC_WID_UNO_CELLSTYL,&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),	ATTR_FONT_COLOR,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),	ATTR_FONT_CONTOUR,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),	ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),					0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CEMPHAS),	ATTR_FONT_EMPHASISMARK,&getCppuType((sal_Int16*)0),			0, MID_EMPHASIS },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
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
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),	ATTR_FONT_HEIGHT,	&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),	ATTR_CJK_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),	ATTR_CTL_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),	ATTR_FONT_POSTURE,	&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),	ATTR_CJK_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),	ATTR_CTL_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CRELIEF),	ATTR_FONT_RELIEF,	&getCppuType((sal_Int16*)0),			0, MID_RELIEF },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),	ATTR_FONT_SHADOWED,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CSTRIKE),	ATTR_FONT_CROSSEDOUT,&getCppuType((sal_Int16*)0),			0, MID_CROSS_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),			0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLCOL),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int32*)0),			0, MID_UL_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLHAS),	ATTR_FONT_UNDERLINE,&getBooleanCppuType(),					0, MID_UL_HASCOLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),	ATTR_FONT_WEIGHT,	&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),	ATTR_CJK_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),	ATTR_CTL_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CWORDMOD),	ATTR_FONT_WORDLINE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR),	SC_WID_UNO_CHCOLHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR),	SC_WID_UNO_CHROWHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),	SC_WID_UNO_CONDFMT,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),	SC_WID_UNO_CONDLOC,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDXML),	SC_WID_UNO_CONDXML,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHGT),	SC_WID_UNO_CELLHGT,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS),	ATTR_HOR_JUSTIFY,	&getCppuType((table::CellHoriJustify*)0), 0, MID_HORJUST_HORJUST },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN),	ATTR_BACKGROUND,	&getBooleanCppuType(),					0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_CELLFILT),	SC_WID_UNO_CELLFILT,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_MANPAGE),	SC_WID_UNO_MANPAGE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NEWPAGE),	SC_WID_UNO_NEWPAGE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),		ATTR_LINEBREAK,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVIS),	SC_WID_UNO_CELLVIS,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),	ATTR_VALUE_FORMAT,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMRULES),	SC_WID_UNO_NUMRULES,&getCppuType((const uno::Reference<container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_OHEIGHT),	SC_WID_UNO_OHEIGHT,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),	ATTR_ORIENTATION,	&getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PADJUST),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),	ATTR_INDENT,		&getCppuType((sal_Int16*)0),			0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHANG),	ATTR_HANGPUNCTUATION,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PLASTADJ),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),		SC_WID_UNO_POS,		&getCppuType((awt::Point*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),	ATTR_ROTATE_VALUE,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),	ATTR_ROTATE_MODE,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),	ATTR_SHADOW,		&getCppuType((table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),		SC_WID_UNO_SIZE,	&getCppuType((awt::Size*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),	SC_WID_UNO_TBLBORD,	&getCppuType((table::TableBorder*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),	SC_WID_UNO_VALIDAT,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),	SC_WID_UNO_VALILOC,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIXML),	SC_WID_UNO_VALIXML,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS),	ATTR_VER_JUSTIFY,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),	ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
        {0,0,0,0}
    };
    return aRowPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetSheetPropertyMap()
{
    static SfxItemPropertyMap aSheetPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ASIANVERT),ATTR_VERTICAL_ASIAN,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_BOTTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, BOTTOM_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK),	ATTR_BACKGROUND,	&getCppuType((sal_Int32*)0),			0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLPRO),	ATTR_PROTECTION,	&getCppuType((util::CellProtection*)0),	0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLSTYL),	SC_WID_UNO_CELLSTYL,&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCOLOR),	ATTR_FONT_COLOR,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_COUTL),	ATTR_FONT_CONTOUR,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CCROSS),	ATTR_FONT_CROSSEDOUT,&getBooleanCppuType(),					0, MID_CROSSED_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CEMPHAS),	ATTR_FONT_EMPHASISMARK,&getCppuType((sal_Int16*)0),			0, MID_EMPHASIS },
        {MAP_CHAR_LEN(SC_UNONAME_CFONT),	ATTR_FONT,			&getCppuType((sal_Int16*)0),			0, MID_FONT_FAMILY },
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
        {MAP_CHAR_LEN(SC_UNONAME_CHEIGHT),	ATTR_FONT_HEIGHT,	&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CJK_CHEIGHT),	ATTR_CJK_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_CTL_CHEIGHT),	ATTR_CTL_FONT_HEIGHT,&getCppuType((float*)0),				0, MID_FONTHEIGHT | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),	ATTR_FONT_LANGUAGE,	&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),	ATTR_CJK_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),	ATTR_CTL_FONT_LANGUAGE,&getCppuType((lang::Locale*)0),			0, MID_LANG_LOCALE },
        {MAP_CHAR_LEN(SC_UNONAME_CPOST),	ATTR_FONT_POSTURE,	&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CJK_CPOST),	ATTR_CJK_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNO_CTL_CPOST),	ATTR_CTL_FONT_POSTURE,&getCppuType((awt::FontSlant*)0),		0, MID_POSTURE },
        {MAP_CHAR_LEN(SC_UNONAME_CRELIEF),	ATTR_FONT_RELIEF,	&getCppuType((sal_Int16*)0),			0, MID_RELIEF },
        {MAP_CHAR_LEN(SC_UNONAME_CSHADD),	ATTR_FONT_SHADOWED,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CSTRIKE),	ATTR_FONT_CROSSEDOUT,&getCppuType((sal_Int16*)0),			0, MID_CROSS_OUT },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDER),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int16*)0),			0, MID_UNDERLINE },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLCOL),	ATTR_FONT_UNDERLINE,&getCppuType((sal_Int32*)0),			0, MID_UL_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CUNDLHAS),	ATTR_FONT_UNDERLINE,&getBooleanCppuType(),					0, MID_UL_HASCOLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CWEIGHT),	ATTR_FONT_WEIGHT,	&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CJK_CWEIGHT),	ATTR_CJK_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNO_CTL_CWEIGHT),	ATTR_CTL_FONT_WEIGHT,&getCppuType((float*)0),				0, MID_WEIGHT },
        {MAP_CHAR_LEN(SC_UNONAME_CWORDMOD),	ATTR_FONT_WORDLINE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHCOLHDR),	SC_WID_UNO_CHCOLHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CHROWHDR),	SC_WID_UNO_CHROWHDR,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDFMT),	SC_WID_UNO_CONDFMT,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDLOC),	SC_WID_UNO_CONDLOC,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CONDXML),	SC_WID_UNO_CONDXML,	&getCppuType((uno::Reference<sheet::XSheetConditionalEntries>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLHJUS),	ATTR_HOR_JUSTIFY,	&getCppuType((table::CellHoriJustify*)0), 0, MID_HORJUST_HORJUST },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN),	ATTR_BACKGROUND,	&getBooleanCppuType(),					0, MID_GRAPHIC_TRANSPARENT },
        {MAP_CHAR_LEN(SC_UNONAME_WRAP),		ATTR_LINEBREAK,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVIS),	SC_WID_UNO_CELLVIS,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LEFTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, LEFT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPBIT),	SC_WID_UNO_LINKDISPBIT,&getCppuType((uno::Reference<awt::XBitmap>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPNAME),	SC_WID_UNO_LINKDISPNAME,&getCppuType((::rtl::OUString*)0),	0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMFMT),	ATTR_VALUE_FORMAT,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_NUMRULES),	SC_WID_UNO_NUMRULES,&getCppuType((const uno::Reference<container::XIndexReplace>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLORI),	ATTR_ORIENTATION,	&getCppuType((table::CellOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PAGESTL),	SC_WID_UNO_PAGESTL,	&getCppuType((::rtl::OUString*)0),		0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PADJUST),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PBMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_LO_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PINDENT),	ATTR_INDENT,		&getCppuType((sal_Int16*)0),			0, 0 }, //! CONVERT_TWIPS
        {MAP_CHAR_LEN(SC_UNONAME_PISCHDIST),ATTR_SCRIPTSPACE,	&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISFORBID),ATTR_FORBIDDEN_RULES,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHANG),	ATTR_HANGPUNCTUATION,&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PISHYPHEN),ATTR_HYPHENATE,		&getBooleanCppuType(),					0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_PLASTADJ),	ATTR_HOR_JUSTIFY,	&::getCppuType((const sal_Int16*)0),	0, MID_HORJUST_ADJUST },
        {MAP_CHAR_LEN(SC_UNONAME_PLMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_L_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PRMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_R_MARGIN  | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_PTMARGIN),	ATTR_MARGIN,		&getCppuType((sal_Int32*)0),			0, MID_MARGIN_UP_MARGIN | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_POS),		SC_WID_UNO_POS,		&getCppuType((awt::Point*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_RIGHTBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, RIGHT_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_ROTANG),	ATTR_ROTATE_VALUE,	&getCppuType((sal_Int32*)0),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROTREF),	ATTR_ROTATE_MODE,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SHADOW),	ATTR_SHADOW,		&getCppuType((table::ShadowFormat*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_SIZE),		SC_WID_UNO_SIZE,	&getCppuType((awt::Size*)0),			0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TBLBORD),	SC_WID_UNO_TBLBORD,	&getCppuType((table::TableBorder*)0),	0, 0 | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_TOPBORDER),ATTR_BORDER,		&::getCppuType((const table::BorderLine*)0), 0, TOP_BORDER | CONVERT_TWIPS },
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	ATTR_USERDEF,		&getCppuType((uno::Reference<container::XNameContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIDAT),	SC_WID_UNO_VALIDAT,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALILOC),	SC_WID_UNO_VALILOC,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VALIXML),	SC_WID_UNO_VALIXML,	&getCppuType((uno::Reference<beans::XPropertySet>*)0), 0 | beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLVJUS),	ATTR_VER_JUSTIFY,	&getCppuType((table::CellVertJustify*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_WRITING),	ATTR_WRITINGDIR,	&getCppuType((sal_Int16*)0),			0, 0 },
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
        SVX_UNOEDIT_NUMBERING_PROPERTIE,	// for completeness of service ParagraphProperties
        {MAP_CHAR_LEN(SC_UNONAME_TEXTUSER),	EE_CHAR_XMLATTRIBS,	&getCppuType((const uno::Reference< container::XNameContainer >*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_USERDEF),	EE_PARA_XMLATTRIBS,	&getCppuType((const uno::Reference< container::XNameContainer >*)0), 0, 0},
        {0,0,0,0}
    };
    return aEditPropertyMap_Impl;
}


//------------------------------------------------------------------------

//!	diese Funktionen in einen allgemeinen Header verschieben
inline long TwipsToHMM(long nTwips)	{ return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)	{ return (nHMM * 72 + 63) / 127; }

//------------------------------------------------------------------------

#define SCCHARPROPERTIES_SERVICE	"com.sun.star.style.CharacterProperties"
#define SCPARAPROPERTIES_SERVICE	"com.sun.star.style.ParagraphProperties"
#define SCCELLPROPERTIES_SERVICE	"com.sun.star.table.CellProperties"
#define SCCELLRANGE_SERVICE			"com.sun.star.table.CellRange"
#define SCCELL_SERVICE				"com.sun.star.table.Cell"
#define SCSHEETCELLRANGES_SERVICE	"com.sun.star.sheet.SheetCellRanges"
#define SCSHEETCELLRANGE_SERVICE	"com.sun.star.sheet.SheetCellRange"
#define SCSPREADSHEET_SERVICE		"com.sun.star.sheet.Spreadsheet"
#define SCSHEETCELL_SERVICE			"com.sun.star.sheet.SheetCell"

SC_SIMPLE_SERVICE_INFO( ScCellFormatsEnumeration, "ScCellFormatsEnumeration", "com.sun.star.sheet.CellFormatRangesEnumeration" )
SC_SIMPLE_SERVICE_INFO( ScCellFormatsObj, "ScCellFormatsObj", "com.sun.star.sheet.CellFormatRanges" )
SC_SIMPLE_SERVICE_INFO( ScUniqueCellFormatsEnumeration, "ScUniqueCellFormatsEnumeration", "com.sun.star.sheet.UniqueCellFormatRangesEnumeration" )
SC_SIMPLE_SERVICE_INFO( ScUniqueCellFormatsObj, "ScUniqueCellFormatsObj", "com.sun.star.sheet.UniqueCellFormatRanges" )
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

//extern const SfxItemPropertyMap* lcl_GetCellStyleMap();
//extern const SfxItemPropertyMap* lcl_GetPageStyleMap();
//extern const SfxItemPropertyMap* lcl_GetHeaderStyleMap();
//extern const SfxItemPropertyMap* lcl_GetFooterStyleMap();

ScPropertyTester::ScPropertyTester()
{
    lcl_TestMap( lcl_GetCellsPropertyMap() );
    lcl_TestMap( lcl_GetRangePropertyMap() );
    lcl_TestMap( lcl_GetCellPropertyMap() );
    lcl_TestMap( lcl_GetColumnPropertyMap() );
    lcl_TestMap( lcl_GetRowPropertyMap() );
    lcl_TestMap( lcl_GetSheetPropertyMap() );

//	lcl_TestMap( lcl_GetCellStyleMap() );
//	lcl_TestMap( lcl_GetPageStyleMap() );
//	lcl_TestMap( lcl_GetHeaderStyleMap() );
//	lcl_TestMap( lcl_GetFooterStyleMap() );
}

ScPropertyTester aPropertyTester;

#endif

//------------------------------------------------------------------------

//!	ScLinkListener in anderes File verschieben !!!

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
            ::rtl::OUString aName = pAry[i].Name;
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

    return 0;	// soll nicht sein
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

const SvxBorderLine* ScHelperFunctions::GetBorderLine( SvxBorderLine& rLine, const table::BorderLine& rStruct )
{
    //	Calc braucht Twips, im Uno-Struct sind 1/100mm

    rLine.SetOutWidth( (USHORT)HMMToTwips( rStruct.OuterLineWidth ) );
    rLine.SetInWidth(  (USHORT)HMMToTwips( rStruct.InnerLineWidth ) );
    rLine.SetDistance( (USHORT)HMMToTwips( rStruct.LineDistance ) );
    rLine.SetColor( ColorData( rStruct.Color ) );

    if ( rLine.GetOutWidth() || rLine.GetInWidth() || rLine.GetDistance() )
        return &rLine;
    else
        return NULL;
}

void ScHelperFunctions::FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const table::TableBorder& rBorder )
{
    SvxBorderLine aLine;
    rOuter.SetDistance( (USHORT)HMMToTwips( rBorder.Distance ) );
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.TopLine ),		BOX_LINE_TOP );
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.BottomLine ),		BOX_LINE_BOTTOM );
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.LeftLine ),		BOX_LINE_LEFT );
    rOuter.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.RightLine ),		BOX_LINE_RIGHT );
    rInner.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.HorizontalLine ),	BOXINFO_LINE_HORI );
    rInner.SetLine( ScHelperFunctions::GetBorderLine( aLine, rBorder.VerticalLine ),	BOXINFO_LINE_VERT );
    rInner.SetValid( VALID_TOP,		 rBorder.IsTopLineValid );
    rInner.SetValid( VALID_BOTTOM,	 rBorder.IsBottomLineValid );
    rInner.SetValid( VALID_LEFT,	 rBorder.IsLeftLineValid );
    rInner.SetValid( VALID_RIGHT,	 rBorder.IsRightLineValid );
    rInner.SetValid( VALID_HORI,	 rBorder.IsHorizontalLineValid );
    rInner.SetValid( VALID_VERT,	 rBorder.IsVerticalLineValid );
    rInner.SetValid( VALID_DISTANCE, rBorder.IsDistanceValid );
    rInner.SetTable( TRUE );
}

void ScHelperFunctions::FillBorderLine( table::BorderLine& rStruct, const SvxBorderLine* pLine )
{
    if (pLine)
    {
        rStruct.Color		   = pLine->GetColor().GetColor();
        rStruct.InnerLineWidth = (sal_Int16)TwipsToHMM( pLine->GetInWidth() );
        rStruct.OuterLineWidth = (sal_Int16)TwipsToHMM( pLine->GetOutWidth() );
        rStruct.LineDistance   = (sal_Int16)TwipsToHMM( pLine->GetDistance() );
    }
    else
        rStruct.Color = rStruct.InnerLineWidth =
            rStruct.OuterLineWidth = rStruct.LineDistance = 0;
}

void ScHelperFunctions::FillTableBorder( table::TableBorder& rBorder,
                            const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner )
{
    ScHelperFunctions::FillBorderLine( rBorder.TopLine, 		rOuter.GetTop() );
    ScHelperFunctions::FillBorderLine( rBorder.BottomLine,		rOuter.GetBottom() );
    ScHelperFunctions::FillBorderLine( rBorder.LeftLine,		rOuter.GetLeft() );
    ScHelperFunctions::FillBorderLine( rBorder.RightLine,		rOuter.GetRight() );
    ScHelperFunctions::FillBorderLine( rBorder.HorizontalLine,	rInner.GetHori() );
    ScHelperFunctions::FillBorderLine( rBorder.VerticalLine,	rInner.GetVert() );

    rBorder.Distance 				= rOuter.GetDistance();
    rBorder.IsTopLineValid 			= rInner.IsValid(VALID_TOP);
    rBorder.IsBottomLineValid		= rInner.IsValid(VALID_BOTTOM);
    rBorder.IsLeftLineValid			= rInner.IsValid(VALID_LEFT);
    rBorder.IsRightLineValid		= rInner.IsValid(VALID_RIGHT);
    rBorder.IsHorizontalLineValid 	= rInner.IsValid(VALID_HORI);
    rBorder.IsVerticalLineValid		= rInner.IsValid(VALID_VERT);
    rBorder.IsDistanceValid 		= rInner.IsValid(VALID_DISTANCE);
}

//------------------------------------------------------------------------

//!	lcl_ApplyBorder nach docfunc verschieben!

void ScHelperFunctions::ApplyBorder( ScDocShell* pDocShell, const ScRangeList& rRanges,
                        const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ULONG nCount = rRanges.Count();
    ULONG i;
    for (i=0; i<nCount; i++)
    {
        ScRange aRange = *rRanges.GetObject(i);
        USHORT nTab = aRange.aStart.Tab();

        ScMarkData aMark;
        aMark.SetMarkArea( aRange );
        aMark.SelectTable( nTab, TRUE );

        pDoc->ApplySelectionFrame( aMark, &rOuter, &rInner );
        // RowHeight bei Umrandung alleine nicht noetig
    }

    for (i=0; i<nCount; i++)
        pDocShell->PostPaint( *rRanges.GetObject(i), PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    pDocShell->SetDocumentModified();
}

//! move lcl_PutDataArray to docfunc?
//!	merge loop with ScFunctionAccess::callFunction

BOOL lcl_PutDataArray( ScDocShell& rDocShell, const ScRange& rRange,
                        const uno::Sequence< uno::Sequence<uno::Any> >& aData )
{
//	BOOL bApi = TRUE;

    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nTab = rRange.aStart.Tab();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();

    if ( !pDoc->IsBlockEditable( nTab, nStartCol,nStartRow, nEndCol,nEndRow ) )
    {
        //!	error message
        return FALSE;
    }

    long nCols = 0;
    long nRows = aData.getLength();
    const uno::Sequence<uno::Any>* pArray = aData.getConstArray();
    if ( nRows )
        nCols = pArray[0].getLength();

    if ( nCols != nEndCol-nStartCol+1 || nRows != nEndRow-nStartRow+1 )
    {
        //!	error message?
        return FALSE;
    }

    pDoc->DeleteAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, IDF_CONTENTS );

    BOOL bError = FALSE;
    USHORT nDocRow = nStartRow;
    for (long nRow=0; nRow<nRows; nRow++)
    {
        const uno::Sequence<uno::Any>& rColSeq = pArray[nRow];
        if ( rColSeq.getLength() == nCols )
        {
            USHORT nDocCol = nStartCol;
            const uno::Any* pColArr = rColSeq.getConstArray();
            for (long nCol=0; nCol<nCols; nCol++)
            {
                const uno::Any& rElement = pColArr[nCol];
                uno::TypeClass eElemClass = rElement.getValueTypeClass();
                if ( eElemClass == uno::TypeClass_VOID )
                {
                    // void = "no value"
                    pDoc->SetError( nDocCol, nDocRow, nTab, NOVALUE );
                }
                else if ( eElemClass == uno::TypeClass_BYTE ||
                            eElemClass == uno::TypeClass_SHORT ||
                            eElemClass == uno::TypeClass_UNSIGNED_SHORT ||
                            eElemClass == uno::TypeClass_LONG ||
                            eElemClass == uno::TypeClass_UNSIGNED_LONG ||
                            eElemClass == uno::TypeClass_FLOAT ||
                            eElemClass == uno::TypeClass_DOUBLE )
                {
                    //	#87871# accept integer types because Basic passes a floating point
                    //	variable as byte, short or long if it's an integer number.
                    double fVal;
                    rElement >>= fVal;
                    pDoc->SetValue( nDocCol, nDocRow, nTab, fVal );
                }
                else if ( eElemClass == uno::TypeClass_STRING )
                {
                    ::rtl::OUString aUStr;
                    rElement >>= aUStr;
                    if ( aUStr.getLength() )
                        pDoc->PutCell( nDocCol, nDocRow, nTab, new ScStringCell( aUStr ) );
                }
                else
                    bError = TRUE;		// invalid type

                ++nDocCol;
            }
        }
        else
            bError = TRUE;							// wrong size

        ++nDocRow;
    }

    BOOL bHeight = rDocShell.AdjustRowHeight( nStartRow, nEndRow, nTab );

    if (!bHeight)
        rDocShell.PostPaint( rRange, PAINT_GRID );		// AdjustRowHeight may have painted already

    rDocShell.SetDocumentModified();

    return !bError;
}

BOOL lcl_PutFormulaArray( ScDocShell& rDocShell, const ScRange& rRange,
                        const uno::Sequence< uno::Sequence< ::rtl::OUString> >& aData )
{
//	BOOL bApi = TRUE;

    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nTab = rRange.aStart.Tab();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();

    if ( !pDoc->IsBlockEditable( nTab, nStartCol,nStartRow, nEndCol,nEndRow ) )
    {
        //!	error message
        return FALSE;
    }

    long nCols = 0;
    long nRows = aData.getLength();
    const uno::Sequence< ::rtl::OUString>* pArray = aData.getConstArray();
    if ( nRows )
        nCols = pArray[0].getLength();

    if ( nCols != nEndCol-nStartCol+1 || nRows != nEndRow-nStartRow+1 )
    {
        //!	error message?
        return FALSE;
    }

    pDoc->DeleteAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, IDF_CONTENTS );

    ScDocFunc aFunc( rDocShell );		// for InterpretEnglishString

    BOOL bError = FALSE;
    USHORT nDocRow = nStartRow;
    for (long nRow=0; nRow<nRows; nRow++)
    {
        const uno::Sequence< ::rtl::OUString>& rColSeq = pArray[nRow];
        if ( rColSeq.getLength() == nCols )
        {
            USHORT nDocCol = nStartCol;
            const ::rtl::OUString* pColArr = rColSeq.getConstArray();
            for (long nCol=0; nCol<nCols; nCol++)
            {
                String aText = pColArr[nCol];
                ScAddress aPos( nDocCol, nDocRow, nTab );
                ScBaseCell* pNewCell = aFunc.InterpretEnglishString( aPos, aText );
                pDoc->PutCell( aPos, pNewCell );

                ++nDocCol;
            }
        }
        else
            bError = TRUE;							// wrong size

        ++nDocRow;
    }

    BOOL bHeight = rDocShell.AdjustRowHeight( nStartRow, nEndRow, nTab );

    if (!bHeight)
        rDocShell.PostPaint( rRange, PAINT_GRID );		// AdjustRowHeight may have painted already

    rDocShell.SetDocumentModified();

    return !bError;
}

//	used in ScCellRangeObj::getFormulaArray and ScCellObj::GetInputString_Impl
String lcl_GetInputString( ScDocShell* pDocSh, const ScAddress& rPosition, BOOL bEnglish )
{
    String aVal;
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        ScBaseCell* pCell = pDoc->GetCell( rPosition );
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
                SvNumberFormatter* pFormatter = bEnglish ? ScGlobal::GetEnglishFormatter() :
                                                            pDoc->GetFormatTable();
                // Since the English formatter was constructed with
                // LANGUAGE_ENGLISH_US the "General" format has index key 0,
                // we don't have to query.
                sal_uInt32 nNumFmt = bEnglish ?
//						pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US) :
                        0 :
                        pDoc->GetNumberFormat( rPosition );

                if ( eType == CELLTYPE_EDIT )
                {
                    //	GetString an der EditCell macht Leerzeichen aus Umbruechen,
                    //	hier werden die Umbrueche aber gebraucht
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

                //	ggf. ein ' davorhaengen wie in ScTabViewShell::UpdateInputHandler
                if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
                {
                    double fDummy;
                    sal_Bool bIsNumberFormat(pFormatter->IsNumberFormat(aVal, nNumFmt, fDummy));
                    if ( bIsNumberFormat )
                        aVal.Insert('\'',0);
                    else if ( aVal.Len() && aVal.GetChar(0) == '\'' )
                    {
                        //	if the string starts with a "'", add another one because setFormula
                        //	strips one (like text input, except for "text" number formats)
                        if ( bEnglish || ( pFormatter->GetType(nNumFmt) != NUMBERFORMAT_TEXT ) )
                            aVal.Insert('\'',0);
                    }
                }
            }
        }
    }
    return aVal;
}

//------------------------------------------------------------------------

ScCellRangesBase::ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR) :
    pDocShell( pDocSh ),
    aPropSet(lcl_GetCellsPropertyMap()),
    bChartColAsHdr( FALSE ),
    bChartRowAsHdr( FALSE ),
    bCursorOnly( FALSE ),
    pCurrentFlat( NULL ),
    pCurrentDeep( NULL ),
    pCurrentDataSet( NULL ),
    pValueListener( NULL ),
    bValueChangePosted( FALSE ),
    pMarkData( NULL ),
    aValueListeners( 0 )
{
    ScRange aCellRange = rR;
    aCellRange.Justify();
    aRanges.Append( aCellRange );

    if (pDocShell)	// Null, wenn per createInstance erzeugt...
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellRangesBase::ScCellRangesBase(ScDocShell* pDocSh, const ScRangeList& rR) :
    pDocShell( pDocSh ),
    aRanges( rR ),
    aPropSet(lcl_GetCellsPropertyMap()),
    bChartColAsHdr( FALSE ),
    bChartRowAsHdr( FALSE ),
    bCursorOnly( FALSE ),
    pCurrentFlat( NULL ),
    pCurrentDeep( NULL ),
    pCurrentDataSet( NULL ),
    pValueListener( NULL ),
    bValueChangePosted( FALSE ),
    pMarkData( NULL ),
    aValueListeners( 0 )
{
    if (pDocShell)	// Null, wenn per createInstance erzeugt...
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellRangesBase::~ScCellRangesBase()
{
    //	#107294# call RemoveUnoObject first, so no notification can happen
    //	during ForgetCurrentAttrs

    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    ForgetCurrentAttrs();

    delete pValueListener;

    //!	XChartDataChangeEventListener abmelden ??
    //!	(ChartCollection haelt dann auch dieses Objekt fest!)
}

void ScCellRangesBase::ForgetCurrentAttrs()
{
    delete pCurrentFlat;
    delete pCurrentDeep;
    delete pCurrentDataSet;
    delete pMarkData;
    pCurrentFlat = NULL;
    pCurrentDeep = NULL;
    pCurrentDataSet = NULL;
    pMarkData = NULL;
}

const ScPatternAttr* ScCellRangesBase::GetCurrentAttrsFlat()
{
    //	get and cache direct cell attributes for this object's range

    if ( !pCurrentFlat && pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pCurrentFlat = pDoc->CreateSelectionPattern( *GetMarkData(), FALSE );
    }
    return pCurrentFlat;
}

const ScPatternAttr* ScCellRangesBase::GetCurrentAttrsDeep()
{
    //	get and cache cell attributes (incl. styles) for this object's range

    if ( !pCurrentDeep && pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pCurrentDeep = pDoc->CreateSelectionPattern( *GetMarkData(), TRUE );
    }
    return pCurrentDeep;
}

SfxItemSet* ScCellRangesBase::GetCurrentDataSet()
{
    if(!pCurrentDataSet)
    {
        const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
        if ( pPattern )
        {
            //	Dontcare durch Default ersetzen, damit man immer eine Reflection hat
            pCurrentDataSet = new SfxItemSet( pPattern->GetItemSet() );
            pCurrentDataSet->ClearInvalidItems();
        }
    }
    return pCurrentDataSet;
}

const ScMarkData* ScCellRangesBase::GetMarkData()
{
    if (!pMarkData)
    {
        pMarkData = new ScMarkData();
        pMarkData->MarkFromRangeList( aRanges, FALSE );
    }
    return pMarkData;
}

void ScCellRangesBase::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        if ( aRanges.UpdateReference( rRef.GetMode(), pDocShell->GetDocument(), rRef.GetRange(),
                                    rRef.GetDx(), rRef.GetDy(), rRef.GetDz() ) )
        {
            if (rRef.GetMode() == URM_INSDEL &&
                aRanges.Count() == 1 &&
                ScTableSheetObj::getImplementation( (cppu::OWeakObject*)this ))
            {
                // #101755#; the range size of a sheet does not change
                ScRange* pR = aRanges.First();
                if (pR)
                {
                    pR->aStart.SetCol(0);
                    pR->aStart.SetRow(0);
                    pR->aEnd.SetCol(MAXCOL);
                    pR->aEnd.SetRow(MAXROW);
                }
            }
            RefChanged();
        }
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            ForgetCurrentAttrs();
            pDocShell = NULL;			// invalid
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
    //!	XChartDataChangeEventListener anpassen

    //!	Test !!!

    if ( pValueListener && aValueListeners.Count() != 0 )
    {
        pValueListener->EndListeningAll();

        ScDocument* pDoc = pDocShell->GetDocument();
        ULONG nCount = aRanges.Count();
        for (ULONG i=0; i<nCount; i++)
            pDoc->StartListeningArea( *aRanges.GetObject(i), pValueListener );
    }
    //!	Test !!!

    ForgetCurrentAttrs();
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

        RefChanged();	// Range im Range-Objekt anpassen
    }
}

void ScCellRangesBase::AddRange(const ScRange& rRange, const sal_Bool bMergeRanges)
{
    if (bMergeRanges)
        aRanges.Join(rRange);
    else
        aRanges.Append(rRange);
    RefChanged();
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
    SC_QUERYINTERFACE( beans::XMultiPropertySet )
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

void SAL_CALL ScCellRangesBase::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ScCellRangesBase::release() throw()
{
    OWeakObject::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellRangesBase::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc(12);
        uno::Type* pPtr = aTypes.getArray();
        pPtr[0] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[1] = getCppuType((const uno::Reference<beans::XMultiPropertySet>*)0);
        pPtr[2] = getCppuType((const uno::Reference<beans::XPropertyState>*)0);
        pPtr[3] = getCppuType((const uno::Reference<sheet::XSheetOperation>*)0);
        pPtr[4] = getCppuType((const uno::Reference<chart::XChartDataArray>*)0);
        pPtr[5] = getCppuType((const uno::Reference<util::XIndent>*)0);
        pPtr[6] = getCppuType((const uno::Reference<sheet::XCellRangesQuery>*)0);
        pPtr[7] = getCppuType((const uno::Reference<sheet::XFormulaQuery>*)0);
        pPtr[8] = getCppuType((const uno::Reference<util::XReplaceable>*)0);
        pPtr[9] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
        pPtr[10] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[11]= getCppuType((const uno::Reference<lang::XTypeProvider>*)0);
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
    ScMarkData aMark(*GetMarkData());
    aMark.MarkToSimple();
    if (!aMark.IsMarked())
        aMark.SetMarkNegative(TRUE);	// um Dummy Position angeben zu koennen

    ScAddress aDummy;					// wenn nicht Marked, ignoriert wegen Negative
    double fVal;
    ScSubTotalFunc eFunc = lcl_SummaryToSubTotal( nFunction );
    ScDocument* pDoc = pDocShell->GetDocument();
    if ( !pDoc->GetSelectionFunction( eFunc, aDummy, aMark, fVal ) )
    {
        throw uno::RuntimeException();		//!	own exception?
    }

    return fVal;
}

void SAL_CALL ScCellRangesBase::clearContents( sal_Int32 nContentFlags ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() )
    {
        // only for clearContents: EDITATTR is only used if no contents are deleted
        USHORT nDelFlags = nContentFlags & IDF_ALL;
        if ( ( nContentFlags & IDF_EDITATTR ) && ( nContentFlags & IDF_CONTENTS ) == 0 )
            nDelFlags |= IDF_EDITATTR;

        ScDocFunc aFunc(*pDocShell);
        aFunc.DeleteContents( *GetMarkData(), nDelFlags, TRUE, TRUE );
    }
    // sonst ist nichts zu tun
}

// XPropertyState

const SfxItemPropertyMap* ScCellRangesBase::GetItemPropertyMap()
{
    return lcl_GetCellsPropertyMap();
}

const SfxItemPropertyMap* lcl_GetPropertyWhich( const SfxItemPropertyMap* pMap, const ::rtl::OUString& rName,
                                                USHORT& rItemWhich )
{
    //	Which-ID des betroffenen Items, auch wenn das Item die Property
    //	nicht alleine behandeln kann

    const SfxItemPropertyMap* pRetMap = SfxItemPropertyMap::GetByName( pMap, rName );
    if ( pRetMap )
    {
        if ( IsScItemWid( pRetMap->nWID ) )
            rItemWhich = pRetMap->nWID;
        else
            switch ( pRetMap->nWID )
            {
                case SC_WID_UNO_TBLBORD:
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

    return pRetMap;
}

beans::PropertyState ScCellRangesBase::GetOnePropertyState( USHORT nItemWhich, const SfxItemPropertyMap* pMap )
{
    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    if ( nItemWhich )					// item wid (from map or special case)
    {
        //	For items that contain several properties (like background),
        //	"ambiguous" is returned too often here

        //	for PropertyState, don't look at styles
        const ScPatternAttr* pPattern = GetCurrentAttrsFlat();
        if ( pPattern )
        {
            SfxItemState eState = pPattern->GetItemSet().GetItemState( nItemWhich, FALSE );

            //	if no rotate value is set, look at orientation
            //!	also for a fixed value of 0 (in case orientation is ambiguous)?
            if ( nItemWhich == ATTR_ROTATE_VALUE && eState == SFX_ITEM_DEFAULT )
                eState = pPattern->GetItemSet().GetItemState( ATTR_ORIENTATION, FALSE );

            if ( nItemWhich == ATTR_VALUE_FORMAT && eState == SFX_ITEM_DEFAULT )
                eState = pPattern->GetItemSet().GetItemState( ATTR_LANGUAGE_FORMAT, FALSE );

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
    else if ( pMap )
    {
        if ( pMap->nWID == SC_WID_UNO_CHCOLHDR || pMap->nWID == SC_WID_UNO_CHROWHDR )
            eRet = beans::PropertyState_DIRECT_VALUE;
        else if ( pMap->nWID == SC_WID_UNO_CELLSTYL )
        {
            //	a style is always set, there's no default state
            const ScStyleSheet* pStyle = pDocShell->GetDocument()->GetSelectionStyle(*GetMarkData());
            if (pStyle)
                eRet = beans::PropertyState_DIRECT_VALUE;
            else
                eRet = beans::PropertyState_AMBIGUOUS_VALUE;
        }
        else if ( pMap->nWID == SC_WID_UNO_NUMRULES )
            eRet = beans::PropertyState_DEFAULT_VALUE;		// numbering rules are always default
    }
    return eRet;
}

beans::PropertyState SAL_CALL ScCellRangesBase::getPropertyState( const ::rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() == 0 )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = GetItemPropertyMap();		// from derived class
    USHORT nItemWhich = 0;
    pMap = lcl_GetPropertyWhich( pMap, aPropertyName, nItemWhich );
    return GetOnePropertyState( nItemWhich, pMap );
}

uno::Sequence<beans::PropertyState> SAL_CALL ScCellRangesBase::getPropertyStates(
                                const uno::Sequence< ::rtl::OUString>& aPropertyNames )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    const SfxItemPropertyMap* pPropertyMap = GetItemPropertyMap();		// from derived class

    const ::rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const SfxItemPropertyMap* pMap = pPropertyMap;
    for(INT32 i = 0; i < aPropertyNames.getLength(); i++)
    {
        USHORT nItemWhich = 0;
        pMap = lcl_GetPropertyWhich(pMap, aPropertyNames[i], nItemWhich);
        pStates[i] = GetOnePropertyState(nItemWhich, pMap);
        if (!pMap)
            pMap = pPropertyMap;
        else
            pMap++;
    }
    return aRet;
}

void SAL_CALL ScCellRangesBase::setPropertyToDefault( const ::rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell )
    {
        const SfxItemPropertyMap* pMap = lcl_GetCellsPropertyMap();
        USHORT nItemWhich = 0;
        pMap = lcl_GetPropertyWhich(pMap, aPropertyName, nItemWhich);

        if ( nItemWhich )				// item wid (from map or special case)
        {
            if ( aRanges.Count() )		// leer = nichts zu tun
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                ScDocFunc aFunc(*pDocShell);

                //!	Bei Items, die mehrere Properties enthalten (z.B. Hintergrund)
                //!	wird hier zuviel zurueckgesetzt

                //!	for ATTR_ROTATE_VALUE, also reset ATTR_ORIENTATION?

                USHORT aWIDs[3];
                aWIDs[0] = nItemWhich;
                if ( nItemWhich == ATTR_VALUE_FORMAT )
                {
                    aWIDs[1] = ATTR_LANGUAGE_FORMAT;	// #67847# language for number formats
                    aWIDs[2] = 0;
                }
                else
                    aWIDs[1] = 0;
                aFunc.ClearItems( *GetMarkData(), aWIDs, TRUE );
            }
        }
        else if ( pMap )
        {
            if ( pMap->nWID == SC_WID_UNO_CHCOLHDR )
                bChartColAsHdr = FALSE;
            else if ( pMap->nWID == SC_WID_UNO_CHROWHDR )
                bChartRowAsHdr = FALSE;
            else if ( pMap->nWID == SC_WID_UNO_CELLSTYL )
            {
                ScDocFunc aFunc(*pDocShell);
                aFunc.ApplyStyle( *GetMarkData(), ScGlobal::GetRscString(STR_STYLENAME_STANDARD), TRUE, TRUE );
            }
        }
    }
}

uno::Any SAL_CALL ScCellRangesBase::getPropertyDefault( const ::rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    //!	mit getPropertyValue zusammenfassen

    ScUnoGuard aGuard;
    uno::Any aAny;

    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        const SfxItemPropertyMap* pMap =
                SfxItemPropertyMap::GetByName( lcl_GetCellsPropertyMap(), aPropertyName );
        if ( pMap )
        {
            if ( IsScItemWid( pMap->nWID ) )
            {
                const ScPatternAttr* pPattern = pDoc->GetDefPattern();
                if ( pPattern )
                {
                    const SfxItemSet& rSet = pPattern->GetItemSet();

                    switch ( pMap->nWID )		// fuer Item-Spezial-Behandlungen
                    {
                        case ATTR_VALUE_FORMAT:
                            //	default has no language set
                            aAny <<= (sal_Int32)( ((const SfxUInt32Item&)rSet.Get(pMap->nWID)).GetValue() );
                            break;
                        case ATTR_INDENT:
                            aAny <<= (sal_Int16)( TwipsToHMM(((const SfxUInt16Item&)
                                            rSet.Get(pMap->nWID)).GetValue()) );
                            break;
                        default:
                            aPropSet.getPropertyValue(aPropertyName, rSet, aAny);
                    }
                }
            }
            else
                switch ( pMap->nWID )
                {
                    case SC_WID_UNO_CHCOLHDR:
                    case SC_WID_UNO_CHROWHDR:
                        ScUnoHelpFunctions::SetBoolInAny( aAny, FALSE );
                        break;
                    case SC_WID_UNO_CELLSTYL:
                        aAny <<= ::rtl::OUString( ScStyleNameConversion::DisplayToProgrammaticName(
                                    ScGlobal::GetRscString(STR_STYLENAME_STANDARD), SFX_STYLE_FAMILY_PARA ) );
                        break;
                    case SC_WID_UNO_TBLBORD:
                        {
                            const ScPatternAttr* pPattern = pDoc->GetDefPattern();
                            if ( pPattern )
                            {
                                table::TableBorder aBorder;
                                ScHelperFunctions::FillTableBorder( aBorder,
                                        (const SvxBoxItem&)pPattern->GetItem(ATTR_BORDER),
                                        (const SvxBoxInfoItem&)pPattern->GetItem(ATTR_BORDER_INNER) );
                                aAny <<= aBorder;
                            }
                        }
                        break;
                    case SC_WID_UNO_CONDFMT:
                    case SC_WID_UNO_CONDLOC:
                    case SC_WID_UNO_CONDXML:
                        {
                            BOOL bEnglish = ( pMap->nWID != SC_WID_UNO_CONDLOC );
                            BOOL bXML = ( pMap->nWID == SC_WID_UNO_CONDXML );

                            uno::Reference<sheet::XSheetConditionalEntries> xObj =
                                    new ScTableConditionalFormat( pDoc, 0, bEnglish, bXML );
                            aAny <<= xObj;
                        }
                        break;
                    case SC_WID_UNO_VALIDAT:
                    case SC_WID_UNO_VALILOC:
                    case SC_WID_UNO_VALIXML:
                        {
                            BOOL bEnglish = ( pMap->nWID != SC_WID_UNO_VALILOC );
                            BOOL bXML = ( pMap->nWID == SC_WID_UNO_VALIXML );

                            uno::Reference<beans::XPropertySet> xObj =
                                    new ScTableValidationObj( pDoc, 0, bEnglish, bXML );
                            aAny <<= xObj;
                        }
                        break;
                    case SC_WID_UNO_NUMRULES:
                        {
                            uno::Reference<container::XIndexReplace> xObj = ScStyleObj::CreateEmptyNumberingRules();
                            aAny <<= xObj;
                        }
                        break;
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


void lcl_SetCellProperty( const SfxItemPropertyMap& rMap, const uno::Any& rValue,
                            ScPatternAttr& rPattern, ScDocument* pDoc,
                            USHORT& rFirstItemId, USHORT& rSecondItemId )
{
    rFirstItemId = rMap.nWID;
    rSecondItemId = 0;

    SfxItemSet& rSet = rPattern.GetItemSet();
    switch ( rMap.nWID )
    {
        case ATTR_VALUE_FORMAT:
            {
                // #67847# language for number formats
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                ULONG nOldFormat = ((const SfxUInt32Item&)rSet.Get( ATTR_VALUE_FORMAT )).GetValue();
                LanguageType eOldLang = ((const SvxLanguageItem&)rSet.Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                nOldFormat = pFormatter->GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );

                sal_Int32 nIntVal;
                if ( rValue >>= nIntVal )
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
                        {
                            rFirstItemId = 0;		// don't use ATTR_VALUE_FORMAT value
                        }

                        rSecondItemId = ATTR_LANGUAGE_FORMAT;
                    }
                }
            }
            break;
        case ATTR_INDENT:
            {
                sal_Int16 nIntVal;
                if ( rValue >>= nIntVal )
                    rSet.Put( SfxUInt16Item( rMap.nWID, (USHORT)HMMToTwips(nIntVal) ) );
            }
            break;
        case ATTR_ROTATE_VALUE:
            {
                sal_Int32 nRotVal;
                if ( rValue >>= nRotVal )
                {
                    //	stored value is always between 0 and 360 deg.
                    nRotVal %= 36000;
                    if ( nRotVal < 0 )
                        nRotVal += 36000;

                    //	always set rotation value and orientation

                    SvxCellOrientation eOrient = SVX_ORIENTATION_STANDARD;
                    BOOL bSetOrient = TRUE;

                    SvxCellOrientation eOld = (SvxCellOrientation)
                        ((const SvxOrientationItem&)rSet.Get(ATTR_ORIENTATION)).
                            GetValue();

                    // #103272#; don't overwrite stacked orientation
                    if ( eOld == SVX_ORIENTATION_STACKED )
                        bSetOrient = FALSE;
                    else if ( nRotVal == 9000 )
                        eOrient = SVX_ORIENTATION_BOTTOMTOP;
                    else if ( nRotVal == 27000 )
                        eOrient = SVX_ORIENTATION_TOPBOTTOM;

                    rSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, nRotVal ) );
                    if (bSetOrient)
                    {
                        rSet.Put( SvxOrientationItem( eOrient, ATTR_ORIENTATION ) );
                        rSecondItemId = ATTR_ORIENTATION;
                    }
                }
            }
            break;
        default:
            {
                SfxItemPropertySet aPropSet( lcl_GetCellsPropertyMap() );
                aPropSet.setPropertyValue(rMap, rValue, rSet);
            }
    }
}

void SAL_CALL ScCellRangesBase::setPropertyValue(
                        const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !pDocShell || aRanges.Count() == 0 )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = GetItemPropertyMap();		// from derived class
    pMap = SfxItemPropertyMap::GetByName( pMap, aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    SetOnePropertyValue( pMap, aValue );
}

void ScCellRangesBase::SetOnePropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& aValue )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( pMap )
    {
        if ( IsScItemWid( pMap->nWID ) )
        {
            if ( aRanges.Count() )		// leer = nichts zu tun
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                ScDocFunc aFunc(*pDocShell);

                //	Fuer Teile von zusammengesetzten Items mit mehreren Properties (z.B. Hintergrund)
                //	muss vorher das alte Item aus dem Dokument geholt werden
                //!	Das kann hier aber nicht erkannt werden
                //!	-> eigenes Flag im PropertyMap-Eintrag, oder was ???
                //!	Item direkt von einzelner Position im Bereich holen?
                //	ClearInvalidItems, damit auf jeden Fall ein Item vom richtigen Typ da ist

                ScPatternAttr aPattern( *GetCurrentAttrsDeep() );
                SfxItemSet& rSet = aPattern.GetItemSet();
                rSet.ClearInvalidItems();

                USHORT nFirstItem, nSecondItem;
                lcl_SetCellProperty( *pMap, aValue, aPattern, pDoc, nFirstItem, nSecondItem );

                for (USHORT nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; nWhich++)
                    if ( nWhich != nFirstItem && nWhich != nSecondItem )
                        rSet.ClearItem(nWhich);

                aFunc.ApplyAttributes( *GetMarkData(), aPattern, TRUE, TRUE );
            }
        }
        else		// implemented here
            switch ( pMap->nWID )
            {
                case SC_WID_UNO_CHCOLHDR:
                    // chart header flags are set for this object, not stored with document
                    bChartColAsHdr = ScUnoHelpFunctions::GetBoolFromAny( aValue );
                    break;
                case SC_WID_UNO_CHROWHDR:
                    bChartRowAsHdr = ScUnoHelpFunctions::GetBoolFromAny( aValue );
                    break;
                case SC_WID_UNO_CELLSTYL:
                    {
                        ::rtl::OUString aStrVal;
                        aValue >>= aStrVal;
                        String aString = ScStyleNameConversion::ProgrammaticToDisplayName(
                                                            aStrVal, SFX_STYLE_FAMILY_PARA );
                        ScDocFunc aFunc(*pDocShell);
                        aFunc.ApplyStyle( *GetMarkData(), aString, TRUE, TRUE );
                    }
                    break;
                case SC_WID_UNO_TBLBORD:
                    {
                        table::TableBorder aBorder;
                        if ( aRanges.Count() && ( aValue >>= aBorder ) )	// empty = nothing to do
                        {
                            SvxBoxItem aOuter(ATTR_BORDER);
                            SvxBoxInfoItem aInner(ATTR_BORDER_INNER);
                            ScHelperFunctions::FillBoxItems( aOuter, aInner, aBorder );

                            ScHelperFunctions::ApplyBorder( pDocShell, aRanges, aOuter, aInner );	//! docfunc
                        }
                    }
                    break;
                case SC_WID_UNO_CONDFMT:
                case SC_WID_UNO_CONDLOC:
                case SC_WID_UNO_CONDXML:
                    {
                        BOOL bEnglish = ( pMap->nWID != SC_WID_UNO_CONDLOC );
                        BOOL bXML = ( pMap->nWID == SC_WID_UNO_CONDXML );

                        uno::Reference<sheet::XSheetConditionalEntries> xInterface;
                        if ( aRanges.Count() && ( aValue >>= xInterface ) )	// leer = nichts zu tun
                        {
                            ScTableConditionalFormat* pFormat =
                                    ScTableConditionalFormat::getImplementation( xInterface );
                            if (pFormat)
                            {
                                ScDocument* pDoc = pDocShell->GetDocument();

                                ScConditionalFormat aNew( 0, pDoc );	// Index wird beim Einfuegen gesetzt
                                pFormat->FillFormat( aNew, pDoc, bEnglish, bXML );
                                ULONG nIndex = pDoc->AddCondFormat( aNew );

                                ScDocFunc aFunc(*pDocShell);

                                ScPatternAttr aPattern( pDoc->GetPool() );
                                aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_CONDITIONAL, nIndex ) );
                                aFunc.ApplyAttributes( *GetMarkData(), aPattern, TRUE, TRUE );
                            }
                        }
                    }
                    break;
                case SC_WID_UNO_VALIDAT:
                case SC_WID_UNO_VALILOC:
                case SC_WID_UNO_VALIXML:
                    {
                        BOOL bEnglish = ( pMap->nWID != SC_WID_UNO_VALILOC );
                        BOOL bXML = ( pMap->nWID == SC_WID_UNO_VALIXML );

                        uno::Reference<beans::XPropertySet> xInterface;
                        if ( aRanges.Count() && ( aValue >>= xInterface ) )	// leer = nichts zu tun
                        {
                            ScTableValidationObj* pValidObj =
                                    ScTableValidationObj::getImplementation( xInterface );
                            if (pValidObj)
                            {
                                ScDocument* pDoc = pDocShell->GetDocument();

                                ScValidationData* pNewData =
                                        pValidObj->CreateValidationData( pDoc, bEnglish, bXML );
                                ULONG nIndex = pDoc->AddValidationEntry( *pNewData );
                                delete pNewData;

                                ScDocFunc aFunc(*pDocShell);

                                ScPatternAttr aPattern( pDoc->GetPool() );
                                aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nIndex ) );
                                aFunc.ApplyAttributes( *GetMarkData(), aPattern, TRUE, TRUE );
                            }
                        }
                    }
                    break;
                // SC_WID_UNO_NUMRULES is ignored...
            }
    }
}

uno::Any SAL_CALL ScCellRangesBase::getPropertyValue( const ::rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !pDocShell || aRanges.Count() == 0 )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = GetItemPropertyMap();		// from derived class
    pMap = SfxItemPropertyMap::GetByName( pMap, aPropertyName );
    if ( !pMap )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    GetOnePropertyValue( pMap, aAny );
    return aAny;
}

void ScCellRangesBase::GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                uno::Any& rAny )
                                                throw(uno::RuntimeException)
{
    if ( pMap )
    {
        if ( IsScItemWid( pMap->nWID ) )
        {
            SfxItemSet* pDataSet = GetCurrentDataSet();
            if ( pDataSet )
            {
                switch ( pMap->nWID )		// fuer Item-Spezial-Behandlungen
                {
                    case ATTR_VALUE_FORMAT:
                        {
                            ScDocument* pDoc = pDocShell->GetDocument();
                            ULONG nOldFormat = ((const SfxUInt32Item&)
                                    pDataSet->Get( ATTR_VALUE_FORMAT )).GetValue();
                            LanguageType eOldLang = ((const SvxLanguageItem&)
                                    pDataSet->Get( ATTR_LANGUAGE_FORMAT )).GetLanguage();
                            nOldFormat = pDoc->GetFormatTable()->
                                    GetFormatForLanguageIfBuiltIn( nOldFormat, eOldLang );
                            rAny <<= (sal_Int32)( nOldFormat );
                        }
                        break;
                    case ATTR_INDENT:
                        rAny <<= (sal_Int16)( TwipsToHMM(((const SfxUInt16Item&)
                                        pDataSet->Get(pMap->nWID)).GetValue()) );
                        break;
                    case ATTR_ROTATE_VALUE:
                        {
                            //	if value is 0 and orientation topbottom or bottomtop,
                            //	adjust value

                            sal_Int32 nRotVal = ((const SfxInt32Item&)
                                        pDataSet->Get(ATTR_ROTATE_VALUE)).GetValue();
                            if ( nRotVal == 0 )
                            {
                                SvxCellOrientation eOrient = (SvxCellOrientation)
                                        ((const SvxOrientationItem&)pDataSet->Get(ATTR_ORIENTATION)).
                                            GetValue();
                                if ( eOrient == SVX_ORIENTATION_BOTTOMTOP )
                                    nRotVal = 9000;
                                else if ( eOrient == SVX_ORIENTATION_TOPBOTTOM )
                                    nRotVal = 27000;
                            }
                            rAny <<= nRotVal;
                        }
                        break;
                    default:
                        aPropSet.getPropertyValue(*pMap, *pDataSet, rAny);
                }
            }
        }
        else		// implemented here
            switch ( pMap->nWID )
            {
                case SC_WID_UNO_CHCOLHDR:
                    ScUnoHelpFunctions::SetBoolInAny( rAny, bChartColAsHdr );
                    break;
                case SC_WID_UNO_CHROWHDR:
                    ScUnoHelpFunctions::SetBoolInAny( rAny, bChartRowAsHdr );
                    break;
                case SC_WID_UNO_CELLSTYL:
                    {
                        String aStyleName;
                        const ScStyleSheet* pStyle = pDocShell->GetDocument()->GetSelectionStyle(*GetMarkData());
                        if (pStyle)
                            aStyleName = pStyle->GetName();
                        rAny <<= ::rtl::OUString( ScStyleNameConversion::DisplayToProgrammaticName(
                                                                aStyleName, SFX_STYLE_FAMILY_PARA ) );
                    }
                    break;
                case SC_WID_UNO_TBLBORD:
                    {
                        //!	loop throgh all ranges
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
                            ScHelperFunctions::FillTableBorder( aBorder, aOuter, aInner );
                            rAny <<= aBorder;
                        }
                    }
                    break;
                case SC_WID_UNO_CONDFMT:
                case SC_WID_UNO_CONDLOC:
                case SC_WID_UNO_CONDXML:
                    {
                        BOOL bEnglish = ( pMap->nWID != SC_WID_UNO_CONDLOC );
                        BOOL bXML = ( pMap->nWID == SC_WID_UNO_CONDXML );

                        const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
                        if ( pPattern )
                        {
                            ScDocument* pDoc = pDocShell->GetDocument();
                            ULONG nIndex = ((const SfxUInt32Item&)
                                    pPattern->GetItem(ATTR_CONDITIONAL)).GetValue();
                            uno::Reference<sheet::XSheetConditionalEntries> xObj =
                                    new ScTableConditionalFormat( pDoc, nIndex, bEnglish, bXML );
                            rAny <<= xObj;
                        }
                    }
                    break;
                case SC_WID_UNO_VALIDAT:
                case SC_WID_UNO_VALILOC:
                case SC_WID_UNO_VALIXML:
                    {
                        BOOL bEnglish = ( pMap->nWID != SC_WID_UNO_VALILOC );
                        BOOL bXML = ( pMap->nWID == SC_WID_UNO_VALIXML );

                        const ScPatternAttr* pPattern = GetCurrentAttrsDeep();
                        if ( pPattern )
                        {
                            ScDocument* pDoc = pDocShell->GetDocument();
                            ULONG nIndex = ((const SfxUInt32Item&)
                                    pPattern->GetItem(ATTR_VALIDDATA)).GetValue();
                            uno::Reference<beans::XPropertySet> xObj =
                                    new ScTableValidationObj( pDoc, nIndex, bEnglish, bXML );
                            rAny <<= xObj;
                        }
                    }
                    break;
                case SC_WID_UNO_NUMRULES:
                    {
                        // always return empty numbering rules object
                        uno::Reference<container::XIndexReplace> xObj = ScStyleObj::CreateEmptyNumberingRules();
                        rAny <<= xObj;
                    }
                    break;
            }
    }
}

void SAL_CALL ScCellRangesBase::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() == 0 )
        throw uno::RuntimeException();

    //!	Test !!!!!
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

            acquire();	// nicht verlieren (eine Ref fuer alle Listener)
        }

        return;
    }
    //!	Test !!!!!

    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( aRanges.Count() == 0 )
        throw uno::RuntimeException();

    //!	Test !!!!!
    if (String(aPropertyName).EqualsAscii( "Test" ))
    {
        acquire();		// falls fuer Listener die letzte Ref existiert - wird unten freigegeben

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

                    release();		// Listener-Ref freigeben
                }

                break;
            }
        }

        release();		// damit kann dieses Objekt geloescht werden
        return;
    }
    //!	Test !!!!!

    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::addVetoableChangeListener( const ::rtl::OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::removeVetoableChangeListener( const ::rtl::OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

// XMultiPropertySet

void SAL_CALL ScCellRangesBase::setPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const uno::Sequence< uno::Any >& aValues )
                                throw (beans::PropertyVetoException,
                                    lang::IllegalArgumentException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;

    const SfxItemPropertyMap* pPropertyMap = GetItemPropertyMap();		// from derived class

    sal_Int32 nCount(aPropertyNames.getLength());
    sal_Int32 nValues(aValues.getLength());
    if (nCount != nValues)
        throw lang::IllegalArgumentException();

    if ( pDocShell && nCount )
    {
        SfxItemPropertySet aPropSet( pPropertyMap );

        ScDocument* pDoc = pDocShell->GetDocument();
        ScPatternAttr* pOldPattern = NULL;
        ScPatternAttr* pNewPattern = NULL;

        const ::rtl::OUString* pNames = aPropertyNames.getConstArray();
        const uno::Any* pValues = aValues.getConstArray();
        const SfxItemPropertyMap* pMap = pPropertyMap;
        for(sal_Int32 i = 0; i < nCount; i++)
        {
            pMap = SfxItemPropertyMap::GetByName( pPropertyMap, pNames[i] );
            if ( pMap )
            {
                if ( IsScItemWid( pMap->nWID ) )	// can be handled by SfxItemPropertySet
                {
                    if ( !pOldPattern )
                    {
                        pOldPattern = new ScPatternAttr( *GetCurrentAttrsDeep() );
                        pOldPattern->GetItemSet().ClearInvalidItems();
                        pNewPattern = new ScPatternAttr( pDoc->GetPool() );
                    }

                    //	collect items in pNewPattern, apply with one call after the loop

                    USHORT nFirstItem, nSecondItem;
                    lcl_SetCellProperty( *pMap, pValues[i], *pOldPattern, pDoc, nFirstItem, nSecondItem );

                    //	put only affected items into new set
                    if ( nFirstItem )
                        pNewPattern->GetItemSet().Put( pOldPattern->GetItemSet().Get( nFirstItem ) );
                    if ( nSecondItem )
                        pNewPattern->GetItemSet().Put( pOldPattern->GetItemSet().Get( nSecondItem ) );
                }
                else
                {
                    //	call virtual method to set a single property
                    SetOnePropertyValue( pMap, pValues[i] );
                }

                pMap++;
            }
            else
                pMap = pPropertyMap;
        }

        if ( pNewPattern && aRanges.Count() )
        {
            ScDocFunc aFunc(*pDocShell);
            aFunc.ApplyAttributes( *GetMarkData(), *pNewPattern, TRUE, TRUE );
        }

        delete pNewPattern;
        delete pOldPattern;
    }
}

uno::Sequence<uno::Any> SAL_CALL ScCellRangesBase::getPropertyValues(
                                const uno::Sequence< ::rtl::OUString >& aPropertyNames )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    const SfxItemPropertyMap* pPropertyMap = GetItemPropertyMap();		// from derived class

    const ::rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<uno::Any> aRet(aPropertyNames.getLength());
    uno::Any* pProperties = aRet.getArray();
    const SfxItemPropertyMap* pMap = pPropertyMap;
    for(INT32 i = 0; i < aPropertyNames.getLength(); i++)
    {
        USHORT nItemWhich = 0;
        pMap = SfxItemPropertyMap::GetByName( pMap, aPropertyNames[i] );
        GetOnePropertyValue( pMap, pProperties[i] );
        if (!pMap)
            pMap = pPropertyMap;
        else
            pMap++;
    }
    return aRet;
}

void SAL_CALL ScCellRangesBase::addPropertiesChangeListener( const uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellRangesBase::firePropertiesChangeEvent( const uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

//!	Test !!!

IMPL_LINK( ScCellRangesBase, ValueListenerHdl, SfxHint*, pHint )
{
    if ( pDocShell && pHint && pHint->ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint*)pHint)->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING) )
    {
        //	nicht doppelt notifien, wenn sich mehrere Formeln im Bereich aendern...

        if ( aValueListeners.Count() && !bValueChangePosted )
        {
            //	Die Listener koennen nur asynchron benachrichtigt werden, weil im
            //	Formel-Broadcast auf keinen Fall Reschedule gerufen werden darf

            beans::PropertyChangeEvent* pEvent = new beans::PropertyChangeEvent;
            pEvent->Source		   = (cppu::OWeakObject*)this;
            pEvent->PropertyName   = ::rtl::OUString::createFromAscii( "Test" );
            pEvent->Further		   = FALSE;
            pEvent->PropertyHandle = -1;
            pEvent->OldValue	   = uno::Any();
            pEvent->NewValue	   = uno::Any();

            // Die Ref im Event-Objekt haelt dieses Objekt fest
            // Das Event-Objekt wird im Link-Handler geloescht

            bValueChangePosted = TRUE;
            Application::PostUserEvent( LINK( this, ScCellRangesBase, ValueChanged ), pEvent );
        }
    }
    return 0;
}

//	ValueChanged wird asynchron gerufen

IMPL_LINK( ScCellRangesBase, ValueChanged, beans::PropertyChangeEvent*, pEvent )
{
    if ( pEvent )
    {
        if ( pDocShell )
            for ( USHORT n=0; n<aValueListeners.Count(); n++ )
                (*aValueListeners[n])->propertyChange( *pEvent );

        bValueChangePosted = FALSE;

        delete pEvent;	// damit kann auch dieses Objekt geloescht werden
    }
    return 0;
}

//!	Test !!!!!

// XIndent

void SAL_CALL ScCellRangesBase::decrementIndent() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell && aRanges.Count() )		// leer = nichts zu tun
    {
        ScDocFunc aFunc(*pDocShell);
        //#97041#; put only MultiMarked ScMarkData in ChangeIndent
        ScMarkData aMarkData(*GetMarkData());
        aMarkData.MarkToMulti();
        aFunc.ChangeIndent( aMarkData, FALSE, TRUE );
    }
}

void SAL_CALL ScCellRangesBase::incrementIndent() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell && aRanges.Count() )		// leer = nichts zu tun
    {
        ScDocFunc aFunc(*pDocShell);
        //#97041#; put only MultiMarked ScMarkData in ChangeIndent
        ScMarkData aMarkData(*GetMarkData());
        aMarkData.MarkToMulti();
        aFunc.ChangeIndent( aMarkData, TRUE, TRUE );
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
            //	ganze Tabelle sinnvoll begrenzen (auf belegten Datenbereich)
            //	(nur hier, Listener werden auf den ganzen Bereich angemeldet)
            //!	direkt testen, ob es ein ScTableSheetObj ist?

            ScRange* pRange = aRanges.GetObject(0);
            if ( pRange->aStart.Col() == 0 && pRange->aEnd.Col() == MAXCOL &&
                 pRange->aStart.Row() == 0 && pRange->aEnd.Row() == MAXROW )
            {
                USHORT nTab = pRange->aStart.Tab();

                USHORT nStartX, nStartY;		// Anfang holen
                if (!pDocShell->GetDocument()->GetDataStart( nTab, nStartX, nStartY ))
                    nStartX = nStartY = 0;

                USHORT nEndX, nEndY;			// Ende holen
                if (!pDocShell->GetDocument()->GetTableArea( nTab, nEndX, nEndY ))
                    nEndX = nEndY = 0;

                xChartRanges = new ScRangeList;
                xChartRanges->Append( ScRange( nStartX, nStartY, nTab, nEndX, nEndY, nTab ) );
            }
        }
        if (!xChartRanges.Is())			//	sonst Ranges direkt uebernehmen
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

ScRangeListRef ScCellRangesBase::GetLimitedChartRanges_Impl( long nDataColumns, long nDataRows ) const
{
    if ( aRanges.Count() == 1 )
    {
        ScRange* pRange = aRanges.GetObject(0);
        if ( pRange->aStart.Col() == 0 && pRange->aEnd.Col() == MAXCOL &&
             pRange->aStart.Row() == 0 && pRange->aEnd.Row() == MAXROW )
        {
            //	if aRanges is a complete sheet, limit to given size

            USHORT nTab = pRange->aStart.Tab();

            long nEndColumn = nDataColumns - 1 + ( bChartColAsHdr ? 1 : 0 );
            if ( nEndColumn < 0 )
                nEndColumn = 0;
            if ( nEndColumn > MAXCOL )
                nEndColumn = MAXCOL;

            long nEndRow = nDataRows - 1 + ( bChartRowAsHdr ? 1 : 0 );
            if ( nEndRow < 0 )
                nEndRow = 0;
            if ( nEndRow > MAXROW )
                nEndRow = MAXROW;

            ScRangeListRef xChartRanges = new ScRangeList;
            xChartRanges->Append( ScRange( 0, 0, nTab, (USHORT)nEndColumn, (USHORT)nEndRow, nTab ) );
            return xChartRanges;
        }
    }

    return new ScRangeList(aRanges);		// as-is
}

void SAL_CALL ScCellRangesBase::setData( const uno::Sequence< uno::Sequence<double> >& aData )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    long nRowCount = aData.getLength();
    long nColCount = nRowCount ? aData[0].getLength() : 0;
    ScRangeListRef xChartRanges = GetLimitedChartRanges_Impl( nColCount, nRowCount );
    if ( pDocShell && xChartRanges.Is() )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScChartArray aArr( pDoc, xChartRanges, String() );
        aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );		// RowAsHdr = ColHeaders
        const ScChartPositionMap* pPosMap = aArr.GetPositionMap();
        if (pPosMap)
        {
            if ( pPosMap->GetColCount() == nColCount &&
                 pPosMap->GetRowCount() == nRowCount )
            {
                for (long nRow=0; nRow<nRowCount; nRow++)
                {
                    const uno::Sequence<double>& rRowSeq = aData[nRow];
                    const double* pArray = rRowSeq.getConstArray();
                    nColCount = rRowSeq.getLength();
                    for (long nCol=0; nCol<nColCount; nCol++)
                    {
                        const ScAddress* pPos = pPosMap->GetPosition( (USHORT)nCol, (USHORT)nRow );
                        if (pPos)
                        {
                            double fVal = pArray[nCol];
                            if ( fVal == DBL_MIN )
                                pDoc->PutCell( *pPos, NULL );		// empty cell
                            else
                                pDoc->SetValue( pPos->Col(), pPos->Row(), pPos->Tab(), pArray[nCol] );
                        }
                    }
                }

                //!	undo
                PaintRanges_Impl( PAINT_GRID );
                pDocShell->SetDocumentModified();
                ForceChartListener_Impl();			// call listeners for this object synchronously
                bDone = TRUE;
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScCellRangesBase::getRowDescriptions()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SchMemChart* pMemChart = CreateMemChart_Impl();
    if ( pMemChart )
    {
        short nRowCount = pMemChart->GetRowCount();
        uno::Sequence< ::rtl::OUString> aSeq( nRowCount );
        ::rtl::OUString* pAry = aSeq.getArray();
        for (short nRow = 0; nRow < nRowCount; nRow++)
            pAry[nRow] = pMemChart->GetRowText(nRow);

        delete pMemChart;
        return aSeq;
    }
    return uno::Sequence< ::rtl::OUString>(0);
}

void SAL_CALL ScCellRangesBase::setRowDescriptions(
                        const uno::Sequence< ::rtl::OUString>& aRowDescriptions )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    if ( bChartColAsHdr )
    {
        long nRowCount = aRowDescriptions.getLength();
        ScRangeListRef xChartRanges = GetLimitedChartRanges_Impl( 1, nRowCount );
        if ( pDocShell && xChartRanges.Is() )
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            ScChartArray aArr( pDoc, xChartRanges, String() );
            aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );		// RowAsHdr = ColHeaders
            const ScChartPositionMap* pPosMap = aArr.GetPositionMap();
            if (pPosMap)
            {
                if ( pPosMap->GetRowCount() == nRowCount )
                {
                    const ::rtl::OUString* pArray = aRowDescriptions.getConstArray();
                    for (long nRow=0; nRow<nRowCount; nRow++)
                    {
                        const ScAddress* pPos = pPosMap->GetRowHeaderPosition( (USHORT)nRow );
                        if (pPos)
                        {
                            String aStr = pArray[nRow];
                            if ( aStr.Len() )
                                pDoc->PutCell( *pPos, new ScStringCell( aStr ) );
                            else
                                pDoc->PutCell( *pPos, NULL );		// empty cell
                        }
                    }

                    //!	undo
                    PaintRanges_Impl( PAINT_GRID );
                    pDocShell->SetDocumentModified();
                    ForceChartListener_Impl();			// call listeners for this object synchronously
                    bDone = TRUE;
                }
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScCellRangesBase::getColumnDescriptions()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SchMemChart* pMemChart = CreateMemChart_Impl();
    if ( pMemChart )
    {
        short nColCount = pMemChart->GetColCount();
        uno::Sequence< ::rtl::OUString> aSeq( nColCount );
        ::rtl::OUString* pAry = aSeq.getArray();
        for (short nCol = 0; nCol < nColCount; nCol++)
            pAry[nCol] = pMemChart->GetColText(nCol);

        delete pMemChart;
        return aSeq;
    }
    return uno::Sequence< ::rtl::OUString>(0);
}

void SAL_CALL ScCellRangesBase::setColumnDescriptions(
                        const uno::Sequence< ::rtl::OUString>& aColumnDescriptions )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    if ( bChartRowAsHdr )
    {
        long nColCount = aColumnDescriptions.getLength();
        ScRangeListRef xChartRanges = GetLimitedChartRanges_Impl( nColCount, 1 );
        if ( pDocShell && xChartRanges.Is() )
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            ScChartArray aArr( pDoc, xChartRanges, String() );
            aArr.SetHeaders( bChartRowAsHdr, bChartColAsHdr );		// RowAsHdr = ColHeaders
            const ScChartPositionMap* pPosMap = aArr.GetPositionMap();
            if (pPosMap)
            {
                if ( pPosMap->GetColCount() == nColCount )
                {
                    const ::rtl::OUString* pArray = aColumnDescriptions.getConstArray();
                    for (long nCol=0; nCol<nColCount; nCol++)
                    {
                        const ScAddress* pPos = pPosMap->GetColHeaderPosition( (USHORT)nCol );
                        if (pPos)
                        {
                            String aStr = pArray[nCol];
                            if ( aStr.Len() )
                                pDoc->PutCell( *pPos, new ScStringCell( aStr ) );
                            else
                                pDoc->PutCell( *pPos, NULL );		// empty cell
                        }
                    }

                    //!	undo
                    PaintRanges_Impl( PAINT_GRID );
                    pDocShell->SetDocumentModified();
                    ForceChartListener_Impl();			// call listeners for this object synchronously
                    bDone = TRUE;
                }
            }
        }
    }

    if (!bDone)
        throw uno::RuntimeException();
}

void ScCellRangesBase::ForceChartListener_Impl()
{
    //	call Update immediately so the caller to setData etc. can
    //	regognize the listener call

    if ( pDocShell )
    {
        ScChartListenerCollection* pColl = pDocShell->GetDocument()->GetChartListenerCollection();
        if ( pColl )
        {
            USHORT nCollCount = pColl->GetCount();
            for ( USHORT nIndex = 0; nIndex < nCollCount; nIndex++ )
            {
                ScChartListener* pChartListener = (ScChartListener*)pColl->At(nIndex);
                if ( pChartListener &&
                        pChartListener->GetUnoSource() == static_cast<chart::XChartData*>(this) &&
                        pChartListener->IsDirty() )
                    pChartListener->Update();
            }
        }
    }
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
        //!	auf doppelte testen?

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

double SAL_CALL	ScCellRangesBase::getNotANumber() throw(::com::sun::star::uno::RuntimeException)
{
    //	im ScChartArray wird DBL_MIN verwendet, weil das Chart es so will
    return DBL_MIN;
}

sal_Bool SAL_CALL ScCellRangesBase::isNotANumber( double nNumber ) throw(uno::RuntimeException)
{
    //	im ScChartArray wird DBL_MIN verwendet, weil das Chart es so will
    return (nNumber == DBL_MIN);
}

// XCellRangesQuery

uno::Reference<sheet::XSheetCellRanges> SAL_CALL ScCellRangesBase::queryVisibleCells()
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        //!	fuer alle Tabellen getrennt, wenn Markierungen pro Tabelle getrennt sind!
        USHORT nTab = lcl_FirstTab(aRanges);

        ScMarkData aMarkData(*GetMarkData());

        ScDocument* pDoc = pDocShell->GetDocument();
        for (USHORT nCol=0; nCol<=MAXCOL; nCol++)
            if (pDoc->GetColFlags(nCol,nTab) & CR_HIDDEN)
                aMarkData.SetMultiMarkArea( ScRange( nCol,0,nTab, nCol,MAXROW,nTab ), FALSE );

        //!	nur bis zur letzten selektierten Zeile testen?
        for (USHORT nRow=0; nRow<=MAXROW; nRow++)
            if (pDoc->GetRowFlags(nRow,nTab) & CR_HIDDEN)
            {
                USHORT nHiddenCount = pDoc->GetHiddenRowCount( nRow, nTab );
                DBG_ASSERT(nHiddenCount, "huch?");
                USHORT nLast = nRow + nHiddenCount - 1;
                aMarkData.SetMultiMarkArea( ScRange( 0,nRow,nTab, MAXCOL,nLast,nTab ), FALSE );
                nRow = nLast;	// +1 wird hinterher addiert
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

        ScMarkData aMarkData(*GetMarkData());

        //	belegte Zellen wegmarkieren
        ULONG nCount = aRanges.Count();
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange = *aRanges.GetObject(i);

            ScCellIterator aIter( pDoc, aRange );
            ScBaseCell* pCell = aIter.GetFirst();
            while (pCell)
            {
                //	Notizen zaehlen als nicht-leer
                if ( pCell->GetCellType() != CELLTYPE_NOTE || pCell->GetNotePtr() )
                    aMarkData.SetMultiMarkArea(
                            ScRange( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() ),
                            FALSE );

                pCell = aIter.GetNext();
            }
        }

        ScRangeList aNewRanges;
        //	IsMultiMarked reicht hier nicht (wird beim deselektieren nicht zurueckgesetzt)
        if (aMarkData.HasAnyMultiMarks())
            aMarkData.FillRangeListWithMarks( &aNewRanges, FALSE );

        return new ScCellRangesObj( pDocShell, aNewRanges );	// aNewRanges kann leer sein
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

        //	passende Zellen selektieren
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
                                //	Date/Time Erkennung

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

        return new ScCellRangesObj( pDocShell, aNewRanges );	// aNewRanges kann leer sein
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

        //	passende Zellen selektieren
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
                    else	// String
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

        return new ScCellRangesObj( pDocShell, aNewRanges );	// aNewRanges kann leer sein
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

        USHORT nCmpPos = bColumnDiff ? (USHORT)aCompare.Row : (USHORT)aCompare.Column;

        //	zuerst alles selektieren, wo ueberhaupt etwas in der Vergleichsspalte steht
        //	(fuer gleiche Zellen wird die Selektion im zweiten Schritt aufgehoben)

        USHORT nTab = lcl_FirstTab(aRanges);	//!	fuer alle Tabellen, wenn Markierungen pro Tabelle!
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

        //	alle nichtleeren Zellen mit der Vergleichsspalte vergleichen und entsprechend
        //	selektieren oder aufheben

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
                    aMarkData.SetMultiMarkArea( aCellRange, FALSE );	// aufheben

                pCell = aIter.GetNext();
            }
        }

        ScRangeList aNewRanges;
        if (aMarkData.IsMultiMarked())
            aMarkData.FillRangeListWithMarks( &aNewRanges, FALSE );

        return new ScCellRangesObj( pDocShell, aNewRanges );	// aNewRanges kann leer sein
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
    ScRange aMask( (USHORT)aRange.StartColumn, (USHORT)aRange.StartRow, aRange.Sheet,
                   (USHORT)aRange.EndColumn,   (USHORT)aRange.EndRow,   aRange.Sheet );

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

    return new ScCellRangesObj( pDocShell, aNew );	// kann leer sein
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

            //	#97205# aMarkData uses aNewRanges, not aRanges, so GetMarkData can't be used
            ScMarkData aMarkData;
            aMarkData.MarkFromRangeList( aNewRanges, FALSE );
            aMarkData.MarkToMulti();		// needed for IsAllMarked

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

            //	#97205# aMarkData uses aNewRanges, not aRanges, so GetMarkData can't be used
            ScMarkData aMarkData;
            aMarkData.MarkFromRangeList( aNewRanges, FALSE );
            aMarkData.MarkToMulti();		// needed for IsAllMarked

            USHORT nTab = lcl_FirstTab(aNewRanges); 				//! alle Tabellen

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
                                bMark = TRUE;					// von Teil des Ranges abhaengig
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
    return uno::Reference<util::XSearchDescriptor>() ;
}

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangesBase::findAll(
                        const uno::Reference<util::XSearchDescriptor>& xDesc )
                                                    throw(uno::RuntimeException)
{
    //	Wenn nichts gefunden wird, soll Null zurueckgegeben werden (?)
    uno::Reference<container::XIndexAccess> xRet;
    return xRet;
}

uno::Reference<uno::XInterface> ScCellRangesBase::Find_Impl(
                                    const uno::Reference<util::XSearchDescriptor>& xDesc,
                                    const ScAddress* pLastPos )
{
    uno::Reference<uno::XInterface> xRet;
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
    return NULL;
}

// XReplaceable

uno::Reference<util::XReplaceDescriptor> SAL_CALL ScCellRangesBase::createReplaceDescriptor()
                                                throw(uno::RuntimeException)
{
    return uno::Reference<util::XReplaceDescriptor>() ;
}

sal_Int32 SAL_CALL ScCellRangesBase::replaceAll( const uno::Reference<util::XSearchDescriptor>& xDesc )
                                                throw(uno::RuntimeException)
{
    INT32 nReplaced = 0;
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
    List					aNamedEntries;
}

void ScCellRangesObj::RefChanged()
{
    ScCellRangesBase::RefChanged();

    //	nix weiter...
}

uno::Any SAL_CALL ScCellRangesObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
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
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XSheetCellRangeContainer>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<container::XNameContainer>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<container::XEnumerationAccess>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];				// parent types first
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

    return NULL;		// keine DocShell oder falscher Index
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
            ScUnoConversion::FillApiRange( aRangeAddress, *rRanges.GetObject(i) );
            pAry[i] = aRangeAddress;
        }
        return aSeq;
    }

    return uno::Sequence<table::CellRangeAddress>(0);	// leer ist moeglich
}

uno::Reference<container::XEnumerationAccess> SAL_CALL ScCellRangesObj::getCells()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //	getCells with empty range list is possible (no exception),
    //	the resulting enumeration just has no elements
    //	(same behaviour as a valid range with no cells)
    //	This is handled in ScCellsEnumeration ctor.

    const ScRangeList& rRanges = GetRangeList();
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
        return new ScCellsObj( pDocSh, rRanges );
    return NULL;
}

::rtl::OUString SAL_CALL ScCellRangesObj::getRangeAddressesAsString()
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

// XSheetCellRangeContainer

void SAL_CALL ScCellRangesObj::addRangeAddress( const table::CellRangeAddress& rRange,
                                    sal_Bool bMergeRanges )
                                    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScRange aRange(static_cast<sal_uInt16>(rRange.StartColumn),
            static_cast<sal_uInt16>(rRange.StartRow),
            static_cast<sal_uInt16>(rRange.Sheet),
            static_cast<sal_uInt16>(rRange.EndColumn),
            static_cast<sal_uInt16>(rRange.EndRow),
            static_cast<sal_uInt16>(rRange.Sheet));
    AddRange(aRange, bMergeRanges);
}

void lcl_RemoveNamedEntry( ScNamedEntryArr_Impl& rNamedEntries, const ScRange& rRange )
{
    USHORT nCount = rNamedEntries.Count();
    for ( USHORT n=nCount; n--; )
        if ( rNamedEntries[n]->GetRange() == rRange )
            rNamedEntries.DeleteAndDestroy( n );
}

void SAL_CALL ScCellRangesObj::removeRangeAddress( const table::CellRangeAddress& rRange )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();

    ScRangeList aSheetRanges;
    ScRangeList aNotSheetRanges;
    for (sal_uInt32 i = 0; i < rRanges.Count(); ++i)
    {
        if (rRanges.GetObject(i)->aStart.Tab() == rRange.Sheet)
        {
            aSheetRanges.Append(*rRanges.GetObject(i));
        }
        else
        {
            aNotSheetRanges.Append(*rRanges.GetObject(i));
        }
    }
    ScMarkData aMarkData;
    aMarkData.MarkFromRangeList( aSheetRanges, FALSE );
    ScRange aRange(static_cast<sal_uInt16>(rRange.StartColumn),
                static_cast<sal_uInt16>(rRange.StartRow),
                static_cast<sal_uInt16>(rRange.Sheet),
                static_cast<sal_uInt16>(rRange.EndColumn),
                static_cast<sal_uInt16>(rRange.EndRow),
                static_cast<sal_uInt16>(rRange.Sheet));
    if (aMarkData.GetTableSelect( aRange.aStart.Tab() ))
    {
        aMarkData.MarkToMulti();
        if (aMarkData.IsAllMarked( aRange ) )
        {
            aMarkData.SetMultiMarkArea( aRange, FALSE );
            lcl_RemoveNamedEntry(aNamedEntries, aRange);
        }
        else
            throw container::NoSuchElementException();
    }
    SetNewRanges(aNotSheetRanges);
    ScRangeList aNew;
    aMarkData.FillRangeListWithMarks( &aNew, FALSE );
    for (sal_uInt32 j = 0; j < aNew.Count(); ++j)
    {
        AddRange(*aNew.GetObject(j), sal_False);
    }
}

void SAL_CALL ScCellRangesObj::addRangeAddresses( const uno::Sequence<table::CellRangeAddress >& rRanges,
                                    sal_Bool bMergeRanges )
                                    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Int32 nCount(rRanges.getLength());
    if (nCount)
    {
        const table::CellRangeAddress* pRanges = rRanges.getConstArray();
        for (sal_Int32 i = 0; i < rRanges.getLength(); i++, pRanges++)
        {
            ScRange aRange(static_cast<sal_uInt16>(pRanges->StartColumn),
                    static_cast<sal_uInt16>(pRanges->StartRow),
                    static_cast<sal_uInt16>(pRanges->Sheet),
                    static_cast<sal_uInt16>(pRanges->EndColumn),
                    static_cast<sal_uInt16>(pRanges->EndRow),
                    static_cast<sal_uInt16>(pRanges->Sheet));
            AddRange(aRange, bMergeRanges);
        }
    }
}

void SAL_CALL ScCellRangesObj::removeRangeAddresses( const uno::Sequence<table::CellRangeAddress >& rRangeSeq )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::uno::RuntimeException)
{
    // with this implementation not needed
//	ScUnoGuard aGuard;


    // use sometimes a better/faster implementation
    sal_uInt32 nCount(rRangeSeq.getLength());
    if (nCount)
    {
        const table::CellRangeAddress* pRanges = rRangeSeq.getConstArray();
        for (sal_uInt32 i=0; i < nCount; ++i, ++pRanges)
        {
            removeRangeAddress(*pRanges);
        }
    }
}

// XNameContainer

void lcl_RemoveNamedEntry( ScNamedEntryArr_Impl& rNamedEntries, const String& rName )
{
    USHORT nCount = rNamedEntries.Count();
    for ( USHORT n=nCount; n--; )
        if ( rNamedEntries[n]->GetName() == rName )
            rNamedEntries.DeleteAndDestroy( n );
}

void SAL_CALL ScCellRangesObj::insertByName( const ::rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    BOOL bDone = FALSE;

    //!	Type of aElement can be some specific interface instead of XInterface

    uno::Reference<uno::XInterface> xInterface;
    if ( pDocSh && ( aElement >>= xInterface ) )
    {
        ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( xInterface );
        if ( pRangesImp && pRangesImp->GetDocShell() == pDocSh )
        {
            //	if explicit name is given and already existing, throw exception

            String aNamStr = aName;
            if ( aNamStr.Len() )
            {
                USHORT nNamedCount = aNamedEntries.Count();
                for (USHORT n=0; n<nNamedCount; n++)
                    if ( aNamedEntries[n]->GetName() == aNamStr )
                        throw container::ElementExistException();
            }

            ScRangeList aNew = GetRangeList();
            const ScRangeList& rAddRanges = pRangesImp->GetRangeList();
            ULONG nAddCount = rAddRanges.Count();
            for (ULONG i=0; i<nAddCount; i++)
                aNew.Join( *rAddRanges.GetObject(i) );
            SetNewRanges(aNew);
            bDone = TRUE;

            if ( aName.getLength() && nAddCount == 1 )
            {
                //	if a name is given, also insert into list of named entries
                //	(only possible for a single range)
                //	name is not in aNamedEntries (tested above)

                ScNamedEntry* pEntry = new ScNamedEntry( aNamStr, *rAddRanges.GetObject(0) );
                aNamedEntries.Insert( pEntry, aNamedEntries.Count() );
            }
        }
    }

    if (!bDone)
    {
        //	invalid element - double names are handled above
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
    return FALSE;	// nicht gefunden
}

BOOL lcl_FindRangeOrEntry( const ScNamedEntryArr_Impl& rNamedEntries,
                            const ScRangeList& rRanges, ScDocShell* pDocSh,
                            const String& rName, ScRange& rFound )
{
    //	exact range in list?

    ULONG nIndex = 0;
    if ( lcl_FindRangeByName( rRanges, pDocSh, rName, nIndex ) )
    {
        rFound = *rRanges.GetObject(nIndex);
        return TRUE;
    }

    //	range contained in selection? (sheet must be specified)

    ScRange aCellRange;
    USHORT nParse = aCellRange.ParseAny( rName, pDocSh->GetDocument() );
    if ( ( nParse & ( SCA_VALID | SCA_TAB_3D ) ) == ( SCA_VALID | SCA_TAB_3D ) )
    {
        ScMarkData aMarkData;
        aMarkData.MarkFromRangeList( rRanges, FALSE );
        aMarkData.MarkToMulti();		// needed for IsAllMarked
        if ( aMarkData.IsAllMarked( aCellRange ) )
        {
            rFound = aCellRange;
            return TRUE;
        }
    }

    //	named entry in this object?

    if ( rNamedEntries.Count() )
    {
        for ( USHORT n=0; n<rNamedEntries.Count(); n++ )
            if ( rNamedEntries[n]->GetName() == rName )
            {
                //	test if named entry is contained in rRanges

                const ScRange& rComp = rNamedEntries[n]->GetRange();
                ScMarkData aMarkData;
                aMarkData.MarkFromRangeList( rRanges, FALSE );
                aMarkData.MarkToMulti();		// needed for IsAllMarked
                if ( aMarkData.IsAllMarked( rComp ) )
                {
                    rFound = rComp;
                    return TRUE;
                }
            }
    }

    return FALSE;		// not found
}

void SAL_CALL ScCellRangesObj::removeByName( const ::rtl::OUString& aName )
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
        //	einzelnen Range weglassen
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
        //	deselect any ranges (parsed or named entry)
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

            bDone = TRUE;		//! error if range was not selected before?
        }
    }

    if (aNamedEntries.Count())
        lcl_RemoveNamedEntry( aNamedEntries, aNameStr );	//	remove named entry

    if (!bDone)
        throw container::NoSuchElementException();		// not found
}

// XNameReplace

void SAL_CALL ScCellRangesObj::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //!	zusammenfassen?
    removeByName( aName );
    insertByName( aName, aElement );
}

// XNameAccess

uno::Any SAL_CALL ScCellRangesObj::getByName( const ::rtl::OUString& aName )
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

uno::Sequence< ::rtl::OUString> SAL_CALL ScCellRangesObj::getElementNames()
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

        uno::Sequence< ::rtl::OUString> aSeq(nCount);
        ::rtl::OUString* pAry = aSeq.getArray();
        for (ULONG i=0; i<nCount; i++)
        {
            //	use given name if for exactly this range, otherwise just format
            ScRange aRange = *rRanges.GetObject(i);
            if ( !aNamedEntries.Count() || !lcl_FindEntryName( aNamedEntries, aRange, aRangeStr ) )
                aRange.Format( aRangeStr, SCA_VALID | SCA_TAB_3D, pDoc );
            pAry[i] = aRangeStr;
        }
        return aSeq;
    }
    return uno::Sequence< ::rtl::OUString>(0);
}

sal_Bool SAL_CALL ScCellRangesObj::hasByName( const ::rtl::OUString& aName )
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
    return new ScIndexEnumeration(this, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SheetCellRangesEnumeration")));
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

::rtl::OUString SAL_CALL ScCellRangesObj::getImplementationName() throw(uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "ScCellRangesObj" );
}

sal_Bool SAL_CALL ScCellRangesObj::supportsService( const ::rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr(rServiceName);
    return aServiceStr.EqualsAscii( SCSHEETCELLRANGES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCPARAPROPERTIES_SERVICE );
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScCellRangesObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aRet(4);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString::createFromAscii( SCSHEETCELLRANGES_SERVICE );
    pArray[1] = ::rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[2] = ::rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    pArray[3] = ::rtl::OUString::createFromAscii( SCPARAPROPERTIES_SERVICE );
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
    aRange.Justify();		// Anfang / Ende richtig
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

uno::Sequence<uno::Type> SAL_CALL ScCellRangeObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangesBase::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 16 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XCellRangeAddressable>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<sheet::XSheetCellRange>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<sheet::XArrayFormulaRange>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<sheet::XCellRangeData>*)0);
        pPtr[nParentLen + 4] = getCppuType((const uno::Reference<sheet::XCellRangeFormula>*)0);
        pPtr[nParentLen + 5] = getCppuType((const uno::Reference<sheet::XMultipleOperation>*)0);
        pPtr[nParentLen + 6] = getCppuType((const uno::Reference<util::XMergeable>*)0);
        pPtr[nParentLen + 7] = getCppuType((const uno::Reference<sheet::XCellSeries>*)0);
        pPtr[nParentLen + 8] = getCppuType((const uno::Reference<table::XAutoFormattable>*)0);
        pPtr[nParentLen + 9] = getCppuType((const uno::Reference<util::XSortable>*)0);
        pPtr[nParentLen +10] = getCppuType((const uno::Reference<sheet::XSheetFilterableEx>*)0);
        pPtr[nParentLen +11] = getCppuType((const uno::Reference<sheet::XSubTotalCalculatable>*)0);
        pPtr[nParentLen +12] = getCppuType((const uno::Reference<table::XColumnRowRange>*)0);
        pPtr[nParentLen +13] = getCppuType((const uno::Reference<util::XImportable>*)0);
        pPtr[nParentLen +14] = getCppuType((const uno::Reference<sheet::XCellFormatRangesSupplier>*)0);
        pPtr[nParentLen +15] = getCppuType((const uno::Reference<sheet::XUniqueCellFormatRangesSupplier>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];				// parent types first
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

//	ColumnCount / RowCount sind weggefallen
//!	werden im Writer fuer Tabellen noch gebraucht ???

uno::Reference<table::XCell> ScCellRangeObj::GetCellByPosition_Impl(
                                        sal_Int32 nColumn, sal_Int32 nRow )
                                throw(lang::IndexOutOfBoundsException, uno::RuntimeException)
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
            ScAddress aNew( (USHORT)nPosX, (USHORT)nPosY, aRange.aStart.Tab() );
            return new ScCellObj( pDocSh, aNew );
        }
    }

    throw lang::IndexOutOfBoundsException();
    return NULL;
}

uno::Reference<table::XCell> SAL_CALL ScCellRangeObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
                                throw(lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return GetCellByPosition_Impl(nColumn, nRow);
}

uno::Reference<table::XCellRange> SAL_CALL ScCellRangeObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

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
            ScRange aNew( (USHORT)nStartX, (USHORT)nStartY, aRange.aStart.Tab(),
                          (USHORT)nEndX, (USHORT)nEndY, aRange.aEnd.Tab() );
            return new ScCellRangeObj( pDocSh, aNew );
        }
    }

    throw lang::IndexOutOfBoundsException();
    return NULL;
}

uno::Reference<table::XCellRange> SAL_CALL ScCellRangeObj::getCellRangeByName(
                        const ::rtl::OUString& aName ) throw(uno::RuntimeException)
{
    //	name refers to the whole document (with the range's table as default),
    //	valid only if the range is within this range

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
            if ( !(nParse & SCA_TAB_3D) )	// keine Tabelle angegeben -> auf dieser Tabelle
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

        if (bFound)			// valid only if within this object's range
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
    ScUnoConversion::FillApiRange( aRet, aRange );
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

// XArrayFormulaRange

::rtl::OUString SAL_CALL ScCellRangeObj::getArrayFormula() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //	Matrix-Formel, wenn eindeutig Teil einer Matrix,
    //	also wenn Anfang und Ende des Blocks zur selben Matrix gehoeren.
    //	Sonst Leerstring.

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
                if ( aStart1 == aStart2 )				// beides dieselbe Matrix
                    pFCell1->GetFormula( aFormula );	// egal, von welcher Zelle
            }
        }
    }
    return aFormula;
}

void SAL_CALL ScCellRangeObj::setArrayFormula( const ::rtl::OUString& aFormula )
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
                //	#74681# don't set array formula for sheet object
                throw uno::RuntimeException();
            }

            aFunc.EnterMatrix( aRange, NULL, aString, TRUE, TRUE );
        }
        else
        {
            //	empty string -> erase array formula
            ScMarkData aMark;
            aMark.SetMarkArea( aRange );
            aMark.SelectTable( aRange.aStart.Tab(), TRUE );
            aFunc.DeleteContents( aMark, IDF_CONTENTS, TRUE, TRUE );
        }
    }
}

// XCellRangeData

uno::Sequence< uno::Sequence<uno::Any> > SAL_CALL ScCellRangeObj::getDataArray()
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( ScTableSheetObj::getImplementation( (cppu::OWeakObject*)this ) )
    {
        //	don't create a data array for the sheet
        throw uno::RuntimeException();
    }

    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        uno::Any aAny;
        // bAllowNV = TRUE: errors as void
        if ( ScRangeToSequence::FillMixedArray( aAny, pDocSh->GetDocument(), aRange, TRUE ) )
        {
            uno::Sequence< uno::Sequence<uno::Any> > aSeq;
            if ( aAny >>= aSeq )
                return aSeq;			// success
        }
    }

    throw uno::RuntimeException();		// no other exceptions specified
    return uno::Sequence< uno::Sequence<uno::Any> >(0);
}

void SAL_CALL ScCellRangeObj::setDataArray(
                        const uno::Sequence< uno::Sequence<uno::Any> >& aArray )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    BOOL bDone = FALSE;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        //! move lcl_PutDataArray to docfunc?
        bDone = lcl_PutDataArray( *pDocSh, aRange, aArray );
    }

    if (!bDone)
        throw uno::RuntimeException();		// no other exceptions specified
}

// XCellRangeFormula

uno::Sequence< uno::Sequence< ::rtl::OUString> > SAL_CALL ScCellRangeObj::getFormulaArray()
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( ScTableSheetObj::getImplementation( (cppu::OWeakObject*)this ) )
    {
        //	don't create a data array for the sheet
        throw uno::RuntimeException();
    }

    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        USHORT nStartCol = aRange.aStart.Col();
        USHORT nStartRow = aRange.aStart.Row();
        USHORT nEndCol = aRange.aEnd.Col();
        USHORT nEndRow = aRange.aEnd.Row();
        USHORT nColCount = nEndCol + 1 - nStartCol;
        USHORT nRowCount = nEndRow + 1 - nStartRow;
        USHORT nTab = aRange.aStart.Tab();

        uno::Sequence< uno::Sequence< ::rtl::OUString> > aRowSeq( nRowCount );
        uno::Sequence< ::rtl::OUString>* pRowAry = aRowSeq.getArray();
        for (USHORT nRowIndex = 0; nRowIndex < nRowCount; nRowIndex++)
        {
            uno::Sequence< ::rtl::OUString> aColSeq( nColCount );
            ::rtl::OUString* pColAry = aColSeq.getArray();
            for (USHORT nColIndex = 0; nColIndex < nColCount; nColIndex++)
                pColAry[nColIndex] = lcl_GetInputString( pDocSh,
                                    ScAddress( nStartCol+nColIndex, nStartRow+nRowIndex, nTab ), TRUE );

            pRowAry[nRowIndex] = aColSeq;
        }

        return aRowSeq;
    }

    throw uno::RuntimeException();		// no other exceptions specified
    return uno::Sequence< uno::Sequence< ::rtl::OUString> >(0);
}

void SAL_CALL ScCellRangeObj::setFormulaArray(
                        const uno::Sequence< uno::Sequence< ::rtl::OUString> >& aArray )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    BOOL bDone = FALSE;
    ScDocShell* pDocSh = GetDocShell();
    if (pDocSh)
    {
        bDone = lcl_PutFormulaArray( *pDocSh, aRange, aArray );
    }

    if (!bDone)
        throw uno::RuntimeException();		// no other exceptions specified
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
        aParam.aRefFormulaCell = ScRefTripel( (USHORT)aFormulaRange.StartColumn,
                                              (USHORT)aFormulaRange.StartRow, aFormulaRange.Sheet,
                                              FALSE, FALSE, FALSE );
        aParam.aRefFormulaEnd  = ScRefTripel( (USHORT)aFormulaRange.EndColumn,
                                              (USHORT)aFormulaRange.EndRow, aFormulaRange.Sheet,
                                              FALSE, FALSE, FALSE );
        aParam.aRefRowCell	   = ScRefTripel( (USHORT)aRowCell.Column,
                                              (USHORT)aRowCell.Row, aRowCell.Sheet,
                                              FALSE, FALSE, FALSE );
        aParam.aRefColCell	   = ScRefTripel( (USHORT)aColumnCell.Column,
                                              (USHORT)aColumnCell.Row, aColumnCell.Sheet,
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

        //!	Fehler abfangen?
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

        FillDir	eDir;
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

        FillDateCmd	eDateCmd;
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
        USHORT nCount = 0;						// "Dest-Count"
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
        if (nCount > MAXROW)		// Ueberlauf
            bError = TRUE;

        if (!bError)
        {
            ScDocFunc aFunc(*pDocSh);
            aFunc.FillAuto( aSourceRange, NULL, eDir, nCount, TRUE, TRUE );
        }
    }
}

// XAutoFormattable

void SAL_CALL ScCellRangeObj::autoFormat( const ::rtl::OUString& aName )
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
            if ( aCompare == aNameString )						//!	Case-insensitiv ???
                break;
        }
        if (nIndex<nCount)
        {
            ScDocFunc aFunc(*pDocSh);
            aFunc.AutoFormat( aRange, NULL, nIndex, TRUE, TRUE );
        }
        else
            throw lang::IllegalArgumentException();
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

            //	im SortDescriptor sind die Fields innerhalb des Bereichs gezaehlt
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
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );	// ggf. Bereich anlegen
        if (pData)
        {
            //	alten Einstellungen holen, falls nicht alles neu gesetzt wird
            pData->GetSortParam(aParam);
            USHORT nOldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();
            for (i=0; i<MAXSORT; i++)
                if ( aParam.bDoSort[i] && aParam.nField[i] >= nOldStart )
                    aParam.nField[i] -= nOldStart;
        }

        ScSortDescriptor::FillSortParam( aParam, aDescriptor );

        //	im SortDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        //	ByRow kann bei FillSortParam umgesetzt worden sein
        USHORT nFieldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();
        for (i=0; i<MAXSORT; i++)
            aParam.nField[i] += nFieldStart;

        USHORT nTab = aRange.aStart.Tab();
        aParam.nCol1 = aRange.aStart.Col();
        aParam.nRow1 = aRange.aStart.Row();
        aParam.nCol2 = aRange.aEnd.Col();
        aParam.nRow2 = aRange.aEnd.Row();

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );		// ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);							// Bereich muss angelegt sein
        aFunc.Sort( nTab, aParam, TRUE, TRUE, TRUE );
    }
}

// XFilterable

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScCellRangeObj::createFilterDescriptor(
                                sal_Bool bEmpty ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    ScFilterDescriptor* pNew = new ScFilterDescriptor(pDocSh);
    if ( !bEmpty && pDocSh )
    {
        // DB-Bereich anlegen erst beim Ausfuehren, per API immer genau den Bereich
        ScDBData* pData = pDocSh->GetDBData( aRange, SC_DB_OLD, TRUE );
        if (pData)
        {
            ScQueryParam aParam;
            pData->GetQueryParam(aParam);
            //	im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
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

    //	das koennte theoretisch ein fremdes Objekt sein, also nur das
    //	oeffentliche XSheetFilterDescriptor Interface benutzen, um
    //	die Daten in ein ScFilterDescriptor Objekt zu kopieren:
    //!	wenn es schon ein ScFilterDescriptor ist, direkt per getImplementation?

    ScDocShell* pDocSh = GetDocShell();
    ScFilterDescriptor aImpl(pDocSh);
    aImpl.setFilterFields( xDescriptor->getFilterFields() );
    //	Rest sind jetzt Properties...

    uno::Reference<beans::XPropertySet> xPropSet( xDescriptor, uno::UNO_QUERY );
    if (xPropSet.is())
        lcl_CopyProperties( aImpl, *(beans::XPropertySet*)xPropSet.get() );

    //
    //	ausfuehren...
    //

    if (pDocSh)
    {
        ScQueryParam aParam = aImpl.GetParam();
        //	im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
        USHORT nFieldStart = aParam.bByRow ? aRange.aStart.Col() : aRange.aStart.Row();
        USHORT nCount = aParam.GetEntryCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScQueryEntry& rEntry = aParam.GetEntry(i);
            if (rEntry.bDoQuery)
            {
                rEntry.nField += nFieldStart;
                //	Im Dialog wird immer der String angezeigt -> muss zum Wert passen
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

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );	// ggf. Bereich anlegen

        //!	keep source range in filter descriptor
        //!	if created by createFilterDescriptorByObject ???

        ScDBDocFunc aFunc(*pDocSh);
        aFunc.Query( nTab, aParam, NULL, TRUE, TRUE );	// Bereich muss angelegt sein
    }
}

//!	get/setAutoFilter als Properties!!!

// XAdvancedFilterSource

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScCellRangeObj::createFilterDescriptorByObject(
                        const uno::Reference<sheet::XSheetFilterable>& xObject )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //	this ist hier nicht der Bereich, der gefiltert wird, sondern der
    //	Bereich mit der Abfrage...

    uno::Reference<sheet::XCellRangeAddressable> xAddr( xObject, uno::UNO_QUERY );

    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh && xAddr.is() )
    {
        //!	Test, ob xObject im selben Dokument ist

        ScFilterDescriptor* pNew = new ScFilterDescriptor(pDocSh);	//! stattdessen vom Objekt?
        //XSheetFilterDescriptorRef xNew = xObject->createFilterDescriptor(TRUE);

        ScQueryParam aParam = pNew->GetParam();
        aParam.bHasHeader = TRUE;

        table::CellRangeAddress aDataAddress = xAddr->getRangeAddress();
        aParam.nCol1 = (USHORT)aDataAddress.StartColumn;
        aParam.nRow1 = (USHORT)aDataAddress.StartRow;
        aParam.nCol2 = (USHORT)aDataAddress.EndColumn;
        aParam.nRow2 = (USHORT)aDataAddress.EndRow;
        aParam.nTab  = aDataAddress.Sheet;

        ScDocument* pDoc = pDocSh->GetDocument();
        BOOL bOk = pDoc->CreateQueryParam(
                            aRange.aStart.Col(), aRange.aStart.Row(),
                            aRange.aEnd.Col(), aRange.aEnd.Row(),
                            aRange.aStart.Tab(), aParam );
        if ( bOk )
        {
            //	im FilterDescriptor sind die Fields innerhalb des Bereichs gezaehlt
            USHORT nFieldStart = aParam.bByRow ? (USHORT)aDataAddress.StartColumn : (USHORT)aDataAddress.StartRow;
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
            return NULL;		// ungueltig -> null
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
            //	im SubTotalDescriptor sind die Fields innerhalb des Bereichs gezaehlt
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
        pImp->GetData(aParam);		// virtuelle Methode der Basisklasse

        //	im SubTotalDescriptor sind die Fields innerhalb des Bereichs gezaehlt
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

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );	// ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);
        aFunc.DoSubTotals( nTab, aParam, NULL, TRUE, TRUE );	// Bereich muss angelegt sein
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
            pData->GetSubTotalParam(aParam);	// auch bei Remove die Feld-Eintraege behalten

        aParam.bRemoveOnly = TRUE;

        USHORT nTab = aRange.aStart.Tab();
        aParam.nCol1 = aRange.aStart.Col();
        aParam.nRow1 = aRange.aStart.Row();
        aParam.nCol2 = aRange.aEnd.Col();
        aParam.nRow2 = aRange.aEnd.Row();

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );	// ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);
        aFunc.DoSubTotals( nTab, aParam, NULL, TRUE, TRUE );	// Bereich muss angelegt sein
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

        //! TODO: could we get passed a valid result set by any means?
        uno::Reference< sdbc::XResultSet > xResultSet;

        pDocSh->GetDBData( aRange, SC_DB_MAKE, TRUE );		// ggf. Bereich anlegen

        ScDBDocFunc aFunc(*pDocSh);							// Bereich muss angelegt sein
        aFunc.DoImport( nTab, aParam, xResultSet, NULL, TRUE, FALSE );	//! Api-Flag als Parameter
    }
}

// XCellFormatRangesSupplier

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangeObj::getCellFormatRanges()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScCellFormatsObj( pDocSh, aRange );
    return NULL;
}

// XUniqueCellFormatRangesSupplier

uno::Reference<container::XIndexAccess> SAL_CALL ScCellRangeObj::getUniqueCellFormatRanges()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return new ScUniqueCellFormatsObj( pDocSh, aRange );
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

void ScCellRangeObj::SetOnePropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& aValue )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    //	Range has only Position and Size in addition to ScCellRangesBase, both are ReadOnly
    //	-> nothing to do here

    ScCellRangesBase::SetOnePropertyValue( pMap, aValue );
}

void ScCellRangeObj::GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                            uno::Any& rAny )
                                                throw(uno::RuntimeException)
{
    if ( pMap )
    {
        if ( pMap->nWID == SC_WID_UNO_POS )
        {
            ScDocShell* pDocSh = GetDocShell();
            if (pDocSh)
            {
                //	GetMMRect converts using HMM_PER_TWIPS, like the DrawingLayer
                Rectangle aMMRect = pDocSh->GetDocument()->GetMMRect(
                                        aRange.aStart.Col(), aRange.aStart.Row(),
                                        aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() );
                awt::Point aPos( aMMRect.Left(), aMMRect.Top() );
                rAny <<= aPos;
            }
        }
        else if ( pMap->nWID == SC_WID_UNO_SIZE )
        {
            ScDocShell* pDocSh = GetDocShell();
            if (pDocSh)
            {
                //	GetMMRect converts using HMM_PER_TWIPS, like the DrawingLayer
                Rectangle aMMRect = pDocSh->GetDocument()->GetMMRect(
                                        aRange.aStart.Col(), aRange.aStart.Row(),
                                        aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() );
                Size aSize = aMMRect.GetSize();
                awt::Size aAwtSize( aSize.Width(), aSize.Height() );
                rAny <<= aAwtSize;
            }
        }
        else
            ScCellRangesBase::GetOnePropertyValue( pMap, rAny );

    }
}

const SfxItemPropertyMap* ScCellRangeObj::GetItemPropertyMap()
{
    return lcl_GetRangePropertyMap();
}

// XServiceInfo

::rtl::OUString SAL_CALL ScCellRangeObj::getImplementationName() throw(uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "ScCellRangeObj" );
}

sal_Bool SAL_CALL ScCellRangeObj::supportsService( const ::rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCSHEETCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCPARAPROPERTIES_SERVICE );
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScCellRangeObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aRet(5);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString::createFromAscii( SCSHEETCELLRANGE_SERVICE );
    pArray[1] = ::rtl::OUString::createFromAscii( SCCELLRANGE_SERVICE );
    pArray[2] = ::rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[3] = ::rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    pArray[4] = ::rtl::OUString::createFromAscii( SCPARAPROPERTIES_SERVICE );
    return aRet;
}

//------------------------------------------------------------------------

const SfxItemPropertyMap* ScCellObj::GetEditPropertyMap()		// static
{
    return lcl_GetEditPropertyMap();
}

ScCellObj::ScCellObj(ScDocShell* pDocSh, const ScAddress& rP) :
    ScCellRangeObj( pDocSh, ScRange(rP,rP) ),
    aCellPropSet( lcl_GetCellPropertyMap() ),
    aCellPos( rP ),
    pUnoText( NULL ),
    nActionLockCount( 0 )
{
    //	pUnoText is allocated on demand (GetUnoText)
    //	can't be aggregated because getString/setString is handled here
}

SvxUnoText&	ScCellObj::GetUnoText()
{
    if (!pUnoText)
    {
        pUnoText = new ScCellTextObj( GetDocShell(), aCellPos );
        pUnoText->acquire();
        if (nActionLockCount)
        {
            ScSharedCellEditSource* pEditSource =
                static_cast<ScSharedCellEditSource*> (pUnoText->GetEditSource());
            if (pEditSource)
                pEditSource->SetDoUpdateData(sal_False);
        }
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

uno::Sequence<uno::Type> SAL_CALL ScCellObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangeObj::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 7 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<table::XCell>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<sheet::XCellAddressable>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<text::XText>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<container::XEnumerationAccess>*)0);
        pPtr[nParentLen + 4] = getCppuType((const uno::Reference<sheet::XSheetAnnotationAnchor>*)0);
        pPtr[nParentLen + 5] = getCppuType((const uno::Reference<text::XTextFieldsSupplier>*)0);
        pPtr[nParentLen + 6] = getCppuType((const uno::Reference<document::XActionLockable>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];				// parent types first
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

//	Hilfsfunktionen

String ScCellObj::GetInputString_Impl(BOOL bEnglish) const		// fuer getFormula / FormulaLocal
{
    return lcl_GetInputString( GetDocShell(), aCellPos, bEnglish );
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
                //	GetString an der EditCell macht Leerzeichen aus Umbruechen,
                //	hier werden die Umbrueche aber gebraucht
                const EditTextObject* pData = ((ScEditCell*)pCell)->GetData();
                if (pData)
                {
                    EditEngine& rEngine = pDoc->GetEditEngine();
                    rEngine.SetText( *pData );
                    aVal = rEngine.GetText( LINEEND_LF );
                }
                //	Edit-Zellen auch nicht per NumberFormatter formatieren
                //	(passend zur Ausgabe)
            }
            else
            {
                //	wie in GetString am Dokument (column)
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

//	XText

uno::Reference<text::XTextCursor> SAL_CALL ScCellObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScCellTextCursor( *this );
}

uno::Reference<text::XTextCursor> SAL_CALL ScCellObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SvxUnoTextCursor* pCursor = new ScCellTextCursor( *this );
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

::rtl::OUString SAL_CALL ScCellObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetOutputString_Impl();
}

void SAL_CALL ScCellObj::setString( const ::rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aText;
    SetString_Impl(aString, FALSE, FALSE);	// immer Text

    // don't create pUnoText here if not there
    if (pUnoText)
        pUnoText->SetSelection(ESelection( 0,0, 0,aString.Len() ));
}

void SAL_CALL ScCellObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(uno::RuntimeException)
{
    // special handling for ScCellTextCursor is no longer needed,
    // SvxUnoText::insertString checks for SvxUnoTextRangeBase instead of SvxUnoTextRange

    ScUnoGuard aGuard;
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

        //!	bei SvxUnoTextRange testen, ob in passendem Objekt !!!
#endif

        if ( pCellField && !pCellField->IsInserted() && pTextRange )
        {
            SvxEditSource* pEditSource = pTextRange->GetEditSource();
            ESelection aSelection = pTextRange->GetSelection();

            if (!bAbsorb)
            {
                //	nicht ersetzen -> hinten anhaengen
                aSelection.Adjust();
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos  = aSelection.nEndPos;
            }

            SvxFieldItem aItem = pCellField->CreateFieldItem();

            SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
            pForwarder->QuickInsertField( aItem, aSelection );
            pEditSource->UpdateData();

            //	neue Selektion: ein Zeichen
            aSelection.Adjust();
            aSelection.nEndPara = aSelection.nStartPara;
            aSelection.nEndPos = aSelection.nStartPos + 1;
            pCellField->InitDoc( pDocSh, aCellPos, aSelection );

            //	#91431# for bAbsorb=FALSE, the new selection must be behind the inserted content
            //	(the xml filter relies on this)
            if (!bAbsorb)
                aSelection.nStartPos = aSelection.nEndPos;

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
            //!	Testen, ob das Feld in dieser Zelle ist
            pCellField->DeleteField();
            return;
        }
    }
    GetUnoText().removeTextContent(xContent);
}

uno::Reference<text::XText> SAL_CALL ScCellObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return this;
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

//	XCell

::rtl::OUString SAL_CALL ScCellObj::getFormula() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //	TRUE = englisch
    return GetInputString_Impl(TRUE);
}

void SAL_CALL ScCellObj::setFormula( const ::rtl::OUString& aFormula ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aFormula;
    SetString_Impl(aString, TRUE, TRUE);	// englisch interpretieren
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
        DBG_ERROR("keine DocShell");		//! Exception oder so?

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
    return getType();	// wenn keine Formel
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
        DBG_ERROR("keine DocShell");		//! Exception oder so?

    return nError;
}

// XCellAddressable

table::CellAddress SAL_CALL ScCellObj::getCellAddress() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellAddress aAdr;
    aAdr.Sheet	= aCellPos.Tab();
    aAdr.Column	= aCellPos.Col();
    aAdr.Row	= aCellPos.Row();
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
    //	sowas gibts nicht im Calc (?)
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

void ScCellObj::SetOnePropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& aValue )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( pMap )
    {
        if ( pMap->nWID == SC_WID_UNO_FORMLOC )
        {
            ::rtl::OUString aStrVal;
            aValue >>= aStrVal;
            String aString = aStrVal;
            SetString_Impl(aString, TRUE, FALSE);	// lokal interpretieren
        }
        else if ( pMap->nWID == SC_WID_UNO_FORMRT )
        {
            //	Read-Only
            //!	Exception oder so...
        }
        else
            ScCellRangeObj::SetOnePropertyValue( pMap, aValue );
    }
}

void ScCellObj::GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                        uno::Any& rAny )
                                            throw(uno::RuntimeException)
{
    if ( pMap )
    {
        if ( pMap->nWID == SC_WID_UNO_FORMLOC )
        {
            // FALSE = lokal
            String aFormStr = GetInputString_Impl(FALSE);
            rAny <<= ::rtl::OUString( aFormStr );
        }
        else if ( pMap->nWID == SC_WID_UNO_FORMRT )
        {
            table::CellContentType eType = GetResultType_Impl();
            rAny <<= eType;
        }
        else
            ScCellRangeObj::GetOnePropertyValue(pMap, rAny);
    }
}

const SfxItemPropertyMap* ScCellObj::GetItemPropertyMap()
{
    return lcl_GetCellPropertyMap();
}

// XServiceInfo

::rtl::OUString SAL_CALL ScCellObj::getImplementationName() throw(uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "ScCellObj" );
}

sal_Bool SAL_CALL ScCellObj::supportsService( const ::rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    //	CellRange/SheetCellRange are not in SheetCell service description,
    //	but ScCellObj is used instead of ScCellRangeObj in CellRanges collections,
    //	so it must support them

    String aServiceStr(rServiceName);
    return aServiceStr.EqualsAscii( SCSHEETCELL_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELL_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCPARAPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCSHEETCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLRANGE_SERVICE );
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScCellObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aRet(7);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString::createFromAscii( SCSHEETCELL_SERVICE );
    pArray[1] = ::rtl::OUString::createFromAscii( SCCELL_SERVICE );
    pArray[2] = ::rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[3] = ::rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    pArray[4] = ::rtl::OUString::createFromAscii( SCPARAPROPERTIES_SERVICE );
    pArray[5] = ::rtl::OUString::createFromAscii( SCSHEETCELLRANGE_SERVICE );
    pArray[6] = ::rtl::OUString::createFromAscii( SCCELLRANGE_SERVICE );
    return aRet;
}

// XActionLockable

sal_Bool SAL_CALL ScCellObj::isActionLocked() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return nActionLockCount != 0;
}

void SAL_CALL ScCellObj::addActionLock() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!nActionLockCount)
    {
        if (pUnoText)
        {
            ScSharedCellEditSource* pEditSource =
                static_cast<ScSharedCellEditSource*> (pUnoText->GetEditSource());
            if (pEditSource)
                pEditSource->SetDoUpdateData(sal_False);
        }
    }
    nActionLockCount++;
}

void SAL_CALL ScCellObj::removeActionLock() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (nActionLockCount > 0)
    {
        nActionLockCount--;
        if (!nActionLockCount)
        {
            if (pUnoText)
            {
                ScSharedCellEditSource* pEditSource =
                    static_cast<ScSharedCellEditSource*> (pUnoText->GetEditSource());
                if (pEditSource)
                {
                    pEditSource->SetDoUpdateData(sal_True);
                    if (pEditSource->IsDirty())
                        pEditSource->UpdateData();
                }
            }
        }
    }
}

void SAL_CALL ScCellObj::setActionLocks( sal_Int16 nLock ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pUnoText)
    {
        ScSharedCellEditSource* pEditSource =
            static_cast<ScSharedCellEditSource*> (pUnoText->GetEditSource());
        if (pEditSource)
        {
            pEditSource->SetDoUpdateData(nLock == 0);
            if ((nActionLockCount > 0) && (nLock == 0) && pEditSource->IsDirty())
                pEditSource->UpdateData();
        }
    }
    nActionLockCount = nLock;
}

sal_Int16 SAL_CALL ScCellObj::resetActionLocks() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nRet(nActionLockCount);
    if (pUnoText)
    {
        ScSharedCellEditSource* pEditSource =
            static_cast<ScSharedCellEditSource*> (pUnoText->GetEditSource());
        if (pEditSource)
        {
            pEditSource->SetDoUpdateData(sal_True);
            if (pEditSource->IsDirty())
                pEditSource->UpdateData();
        }
    }
    nActionLockCount = 0;
    return nRet;
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

void SAL_CALL ScTableSheetObj::acquire() throw()
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScTableSheetObj::release() throw()
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
            pPtr[i] = pParentPtr[i];				// parent types first
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

//	Hilfsfunktionen

USHORT ScTableSheetObj::GetTab_Impl() const
{
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT(rRanges.Count() == 1, "was fuer Ranges ?!?!");
    const ScRange* pFirst = rRanges.GetObject(0);
    if (pFirst)
        return pFirst->aStart.Tab();

    return 0;	// soll nicht sein
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
                        const ::rtl::OUString& aRange ) throw(uno::RuntimeException)
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
        //!	einzelne Zelle oder ganze Tabelle???????
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
    return this;		//!???
}

// XCellRange

uno::Reference<table::XCell> SAL_CALL ScTableSheetObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
                                throw(lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::GetCellByPosition_Impl(nColumn, nRow);
}

uno::Reference<table::XCellRange> SAL_CALL ScTableSheetObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
                                throw(lang::IndexOutOfBoundsException, uno::RuntimeException)
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
        if (aSize.Width() && aSize.Height())		// effektive Groesse schon gesetzt?
            pDoc->UpdatePageBreaks( nTab );
        else
        {
            //	Umbrueche updaten wie in ScDocShell::PageStyleModified:
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
                aData.Position	  = nCol;
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
        if (aSize.Width() && aSize.Height())		// effektive Groesse schon gesetzt?
            pDoc->UpdatePageBreaks( nTab );
        else
        {
            //	Umbrueche updaten wie in ScDocShell::PageStyleModified:
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
                aData.Position	  = nRow;
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
        //!	docfunc Funktion, auch fuer ScViewFunc::RemoveManualBreaks

        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        pDoc->RemoveManualBreaks(nTab);
        pDoc->UpdatePageBreaks(nTab);

        //? UpdatePageBreakData( TRUE );
        pDocSh->SetDocumentModified();
        pDocSh->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
    }
}

// XNamed

::rtl::OUString SAL_CALL ScTableSheetObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aName;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocument()->GetName( GetTab_Impl(), aName );
    return aName;
}

void SAL_CALL ScTableSheetObj::setName( const ::rtl::OUString& aNewName )
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
            return uno::Reference<drawing::XDrawPage> (pPage->getUnoPage(), uno::UNO_QUERY);

        //	Das DrawPage-Objekt meldet sich als Listener am SdrModel an
        //	und sollte von dort alle Aktionen mitbekommen
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
            case sheet::CellInsertMode_NONE:	bDo = FALSE;			break;
            case sheet::CellInsertMode_DOWN:	eCmd = INS_CELLSDOWN;	break;
            case sheet::CellInsertMode_RIGHT:	eCmd = INS_CELLSRIGHT;	break;
            case sheet::CellInsertMode_ROWS:	eCmd = INS_INSROWS;		break;
            case sheet::CellInsertMode_COLUMNS: eCmd = INS_INSCOLS;		break;
            default:
                DBG_ERROR("insertCells: falscher Mode");
                bDo = FALSE;
        }

        if (bDo)
        {
            DBG_ASSERT( aRange.Sheet == GetTab_Impl(), "falsche Tabelle in CellRangeAddress" );
            ScRange aScRange;
            ScUnoConversion::FillScRange( aScRange, aRange );
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
            case sheet::CellDeleteMode_NONE:	 bDo = FALSE;			break;
            case sheet::CellDeleteMode_UP:		 eCmd = DEL_CELLSUP;	break;
            case sheet::CellDeleteMode_LEFT:	 eCmd = DEL_CELLSLEFT;	break;
            case sheet::CellDeleteMode_ROWS:	 eCmd = DEL_DELROWS;	break;
            case sheet::CellDeleteMode_COLUMNS:	 eCmd = DEL_DELCOLS;	break;
            default:
                DBG_ERROR("deleteCells: falscher Mode");
                bDo = FALSE;
        }

        if (bDo)
        {
            DBG_ASSERT( aRange.Sheet == GetTab_Impl(), "falsche Tabelle in CellRangeAddress" );
            ScRange aScRange;
            ScUnoConversion::FillScRange( aScRange, aRange );
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
        ScUnoConversion::FillScRange( aRange, aSource );
        ScAddress aDestPos( (USHORT)aDestination.Column, (USHORT)aDestination.Row, aDestination.Sheet );
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
        ScUnoConversion::FillScRange( aRange, aSource );
        ScAddress aDestPos( (USHORT)aDestination.Column, (USHORT)aDestination.Row, aDestination.Sheet );
        ScDocFunc aFunc(*pDocSh);
        aFunc.MoveBlock( aRange, aDestPos, FALSE, TRUE, TRUE, TRUE );
    }
}

// XPrintAreas

void ScTableSheetObj::PrintAreaUndo_Impl( ScPrintRangeSaver* pOldRanges )
{
    //	Umbrueche und Undo

    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        pDoc->CreatePrintRangeSaver();
        ScPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab ).UpdatePages();
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
                ScUnoConversion::FillApiRange( aRangeAddress, *pRange );
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
                ScUnoConversion::FillScRange( aRange, pAry[i] );
                pDoc->SetPrintRange( nTab, i, aRange );
            }
        }

        PrintAreaUndo_Impl( pOldRanges );	// Undo, Umbrueche, Modified etc.
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
            if ( !pDoc->GetRepeatColRange( nTab ) )			// keinen bestehenden Bereich veraendern
            {
                ScRange aNew( 0, 0, nTab, 0, 0, nTab );		// Default
                pDoc->SetRepeatColRange( nTab, &aNew );		// einschalten
            }
        }
        else
            pDoc->SetRepeatColRange( nTab, NULL );			// abschalten

        PrintAreaUndo_Impl( pOldRanges );	// Undo, Umbrueche, Modified etc.

        //!	zuletzt gesetzten Bereich beim Abschalten merken und beim Einschalten wiederherstellen ???
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
            ScUnoConversion::FillApiRange( aRet, *pRange );
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
        ScUnoConversion::FillScRange( aNew, aTitleColumns );
        pDoc->SetRepeatColRange( nTab, &aNew );		// immer auch einschalten

        PrintAreaUndo_Impl( pOldRanges );			// Undo, Umbrueche, Modified etc.
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
            if ( !pDoc->GetRepeatRowRange( nTab ) )			// keinen bestehenden Bereich veraendern
            {
                ScRange aNew( 0, 0, nTab, 0, 0, nTab );		// Default
                pDoc->SetRepeatRowRange( nTab, &aNew );		// einschalten
            }
        }
        else
            pDoc->SetRepeatRowRange( nTab, NULL );			// abschalten

        PrintAreaUndo_Impl( pOldRanges );	// Undo, Umbrueche, Modified etc.

        //!	zuletzt gesetzten Bereich beim Abschalten merken und beim Einschalten wiederherstellen ???
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
            ScUnoConversion::FillApiRange( aRet, *pRange );
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
        ScUnoConversion::FillScRange( aNew, aTitleRows );
        pDoc->SetRepeatRowRange( nTab, &aNew );		// immer auch einschalten

        PrintAreaUndo_Impl( pOldRanges );			// Undo, Umbrueche, Modified etc.
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

    //!	Filter und Options aus altem Link suchen

    ::rtl::OUString aUrl = getLinkUrl();
    ::rtl::OUString aSheet = getLinkSheetName();

    ::rtl::OUString aEmpty;
    link( aUrl, aSheet, aEmpty, aEmpty, nLinkMode );
}

::rtl::OUString SAL_CALL ScTableSheetObj::getLinkUrl() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aFile;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        aFile = pDocSh->GetDocument()->GetLinkDoc( GetTab_Impl() );
    return aFile;
}

void SAL_CALL ScTableSheetObj::setLinkUrl( const ::rtl::OUString& aLinkUrl )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	Filter und Options aus altem Link suchen

    sheet::SheetLinkMode eMode = getLinkMode();
    ::rtl::OUString aSheet = getLinkSheetName();

    ::rtl::OUString aEmpty;
    link( aLinkUrl, aSheet, aEmpty, aEmpty, eMode );
}

::rtl::OUString SAL_CALL ScTableSheetObj::getLinkSheetName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aSheet;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        aSheet = pDocSh->GetDocument()->GetLinkTab( GetTab_Impl() );
    return aSheet;
}

void SAL_CALL ScTableSheetObj::setLinkSheetName( const ::rtl::OUString& aLinkSheetName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	Filter und Options aus altem Link suchen

    sheet::SheetLinkMode eMode = getLinkMode();
    ::rtl::OUString aUrl = getLinkUrl();

    ::rtl::OUString aEmpty;
    link( aUrl, aLinkSheetName, aEmpty, aEmpty, eMode );
}

void SAL_CALL ScTableSheetObj::link( const ::rtl::OUString& aUrl, const ::rtl::OUString& aSheetName,
                        const ::rtl::OUString& aFilterName, const ::rtl::OUString& aFilterOptions,
                        sheet::SheetLinkMode nMode ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        String aFileString	 = aUrl;
        String aFilterString = aFilterName;
        String aOptString	 = aFilterOptions;
        String aSheetString	 = aSheetName;

        aFileString = ScGlobal::GetAbsDocName( aFileString, pDocSh );
        if ( !aFilterString.Len() )
            ScDocumentLoader::GetFilterName( aFileString, aFilterString, aOptString );

        //	remove application prefix from filter name here, so the filter options
        //	aren't reset when the filter name is changed in ScTableLink::DataChanged
        ScDocumentLoader::RemoveAppPrefix( aFilterString );

        BYTE nLinkMode = SC_LINK_NONE;
        if ( nMode == sheet::SheetLinkMode_NORMAL )
            nLinkMode = SC_LINK_NORMAL;
        else if ( nMode == sheet::SheetLinkMode_VALUE )
            nLinkMode = SC_LINK_VALUE;

        ULONG nRefresh = 0;
        pDoc->SetLink( nTab, nLinkMode, aFileString, aFilterString, aOptString, aSheetString, nRefresh );

        pDocSh->UpdateLinks();					// ggf. Link eintragen oder loeschen

        //!	Undo fuer Link-Daten an der Table

        if ( nLinkMode != SC_LINK_NONE )		// Link updaten
        {
            //	Update immer, auch wenn der Link schon da war
            //!	Update nur fuer die betroffene Tabelle???

            SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
            USHORT nCount = pLinkManager->GetLinks().Count();
            for ( USHORT i=0; i<nCount; i++ )
            {
                ::binfilter::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
                if (pBase->ISA(ScTableLink))
                {
                    ScTableLink* pTabLink = (ScTableLink*)pBase;
                    if ( pTabLink->GetFileName() == aFileString )
                        pTabLink->Update();							// inkl. Paint&Undo

                    //!	Der Dateiname sollte nur einmal vorkommen (?)
                }
            }
        }

        //!	Notify fuer ScSheetLinkObj Objekte!!!
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
        ScAddress aPos( (USHORT)aPosition.Column, (USHORT)aPosition.Row, nTab );
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
        ScAddress aPos( (USHORT)aPosition.Column, (USHORT)aPosition.Row, nTab );
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
        ScAddress aPos( (USHORT)aPosition.Column, (USHORT)aPosition.Row, nTab );
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
        ScAddress aPos( (USHORT)aPosition.Column, (USHORT)aPosition.Row, nTab );
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
        ScAddress aPos( (USHORT)aPosition.Column, (USHORT)aPosition.Row, nTab );
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
        ScUnoConversion::FillScRange( aGroupRange, aRange );
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
        ScUnoConversion::FillScRange( aGroupRange, aRange );
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
        ScUnoConversion::FillScRange( aFormulaRange, aRange );
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
        ScUnoConversion::FillScRange( aMarkRange, aRange );
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
        ScUnoConversion::FillScRange( aMarkRange, aRange );
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

void SAL_CALL ScTableSheetObj::protect( const ::rtl::OUString& aPassword )
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

void SAL_CALL ScTableSheetObj::unprotect( const ::rtl::OUString& aPassword )
                            throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        String aString = aPassword;
        ScDocFunc aFunc(*pDocSh);
        aFunc.Unprotect( GetTab_Impl(), aString, TRUE );

        //!	Rueckgabewert auswerten, Exception oder so
    }
}

sal_Bool SAL_CALL ScTableSheetObj::isProtected() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        return pDocSh->GetDocument()->IsTabProtected( GetTab_Impl() );

    DBG_ERROR("keine DocShell");		//! Exception oder so?
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

::rtl::OUString SAL_CALL ScTableSheetObj::getScenarioComment() throw(uno::RuntimeException)
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
    return ::rtl::OUString();
}

void SAL_CALL ScTableSheetObj::setScenarioComment( const ::rtl::OUString& aScenarioComment )
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
                ScRange aRange( (USHORT)pAry[i].StartColumn, (USHORT)pAry[i].StartRow, nTab,
                                (USHORT)pAry[i].EndColumn,   (USHORT)pAry[i].EndRow,   nTab );

                aMarkData.SetMultiMarkArea( aRange );
            }
        }

        //	Szenario-Ranges sind durch Attribut gekennzeichnet
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
        pDoc->GetName( nTab, aName );		// Name dieses Szenarios

        USHORT nDestTab = nTab;
        while ( nDestTab > 0 && pDoc->IsScenario(nDestTab) )
            --nDestTab;

        if ( !pDoc->IsScenario(nDestTab) )
            pDocSh->UseScenario( nDestTab, aName );

        //!	sonst Fehler oder so
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

void ScTableSheetObj::SetOnePropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& aValue )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( pMap )
    {
        if ( IsScItemWid( pMap->nWID ) )
        {
            //	for Item WIDs, call ScCellRangesBase directly
            ScCellRangesBase::SetOnePropertyValue(pMap, aValue);
            return;
        }

        //	own properties

        ScDocShell* pDocSh = GetDocShell();
        if (!pDocSh)
            return;													//!	Exception oder so?
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();
        ScDocFunc aFunc(*pDocSh);

        if ( pMap->nWID == SC_WID_UNO_PAGESTL )
        {
            ::rtl::OUString aStrVal;
            aValue >>= aStrVal;
            String aNewStr = ScStyleNameConversion::ProgrammaticToDisplayName(
                                                aStrVal, SFX_STYLE_FAMILY_PAGE );

            //!	Undo? (auch bei SID_STYLE_APPLY an der View)

            if ( pDoc->GetPageStyle( nTab ) != aNewStr )
            {
                pDoc->SetPageStyle( nTab, aNewStr );
                ScPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab ).UpdatePages();
                pDocSh->SetDocumentModified();

            }
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLVIS )
        {
            BOOL bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            aFunc.SetTableVisible( nTab, bVis, TRUE );
        }
        else
            ScCellRangeObj::SetOnePropertyValue(pMap, aValue);		// base class, no Item WID
    }
}

void ScTableSheetObj::GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                            uno::Any& rAny )
                                                throw(uno::RuntimeException)
{
    if ( pMap )
    {
        ScDocShell* pDocSh = GetDocShell();
        if (!pDocSh)
            throw uno::RuntimeException();
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetTab_Impl();

        if ( pMap->nWID == SC_WID_UNO_PAGESTL )
        {
            String aStyle = ScStyleNameConversion::DisplayToProgrammaticName(
                                pDoc->GetPageStyle( nTab ), SFX_STYLE_FAMILY_PAGE );
            rAny <<= ::rtl::OUString( aStyle );
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLVIS )
        {
            BOOL bVis = pDoc->IsVisible( nTab );
            ScUnoHelpFunctions::SetBoolInAny( rAny, bVis );
        }
        else if ( pMap->nWID == SC_WID_UNO_LINKDISPBIT )
        {
            //	no target bitmaps for individual entries (would be all equal)
            // ScLinkTargetTypeObj::SetLinkTargetBitmap( aAny, SC_LINKTARGETTYPE_SHEET );
        }
        else if ( pMap->nWID == SC_WID_UNO_LINKDISPNAME )
        {
            //	LinkDisplayName for hyperlink dialog
            rAny <<= getName();		// sheet name
        }
        else
            ScCellRangeObj::GetOnePropertyValue(pMap, rAny);
    }
}

const SfxItemPropertyMap* ScTableSheetObj::GetItemPropertyMap()
{
    return lcl_GetSheetPropertyMap();
}

// XServiceInfo

::rtl::OUString SAL_CALL ScTableSheetObj::getImplementationName() throw(uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "ScTableSheetObj" );
}

sal_Bool SAL_CALL ScTableSheetObj::supportsService( const ::rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCSPREADSHEET_SERVICE ) ||
           aServiceStr.EqualsAscii( SCSHEETCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLRANGE_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCHARPROPERTIES_SERVICE ) ||
           aServiceStr.EqualsAscii( SCPARAPROPERTIES_SERVICE );
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScTableSheetObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aRet(6);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString::createFromAscii( SCSPREADSHEET_SERVICE );
    pArray[1] = ::rtl::OUString::createFromAscii( SCSHEETCELLRANGE_SERVICE );
    pArray[2] = ::rtl::OUString::createFromAscii( SCCELLRANGE_SERVICE );
    pArray[3] = ::rtl::OUString::createFromAscii( SCCELLPROPERTIES_SERVICE );
    pArray[4] = ::rtl::OUString::createFromAscii( SCCHARPROPERTIES_SERVICE );
    pArray[5] = ::rtl::OUString::createFromAscii( SCPARAPROPERTIES_SERVICE );
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

void SAL_CALL ScTableColumnObj::acquire() throw()
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScTableColumnObj::release() throw()
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
            pPtr[i] = pParentPtr[i];				// parent types first
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

::rtl::OUString SAL_CALL ScTableColumnObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    const ScRange& rRange = GetRange();
    DBG_ASSERT(rRange.aStart.Col() == rRange.aEnd.Col(), "too many columns");
    USHORT nCol = rRange.aStart.Col();

    return ColToAlpha( nCol );		// from global.hxx
}

void SAL_CALL ScTableColumnObj::setName( const ::rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    throw uno::RuntimeException();		// read-only
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

void ScTableColumnObj::SetOnePropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& aValue )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( pMap )
    {
        if ( IsScItemWid( pMap->nWID ) )
        {
            //	for Item WIDs, call ScCellRangesBase directly
            ScCellRangesBase::SetOnePropertyValue(pMap, aValue);
            return;
        }

        //	own properties

        ScDocShell* pDocSh = GetDocShell();
        if (!pDocSh)
            return;													//!	Exception oder so?
        ScDocument* pDoc = pDocSh->GetDocument();
        const ScRange& rRange = GetRange();
        DBG_ASSERT(rRange.aStart.Col() == rRange.aEnd.Col(), "zuviele Spalten");
        USHORT nCol = rRange.aStart.Col();
        USHORT nTab = rRange.aStart.Tab();
        ScDocFunc aFunc(*pDocSh);

        USHORT nColArr[2];
        nColArr[0] = nColArr[1] = nCol;

        if ( pMap->nWID == SC_WID_UNO_CELLWID )
        {
            sal_Int32 nNewWidth;
            if ( aValue >>= nNewWidth )
            {
                //	property is 1/100mm, column width is twips
                nNewWidth = HMMToTwips(nNewWidth);
                aFunc.SetWidthOrHeight( TRUE, 1, nColArr, nTab, SC_SIZE_ORIGINAL,
                                        (USHORT)nNewWidth, TRUE, TRUE );
            }
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLVIS )
        {
            BOOL bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
            aFunc.SetWidthOrHeight( TRUE, 1, nColArr, nTab, eMode, 0, TRUE, TRUE );
            //	SC_SIZE_DIRECT mit Groesse 0 blendet aus
        }
        else if ( pMap->nWID == SC_WID_UNO_OWIDTH )
        {
            BOOL bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if (bOpt)
                aFunc.SetWidthOrHeight( TRUE, 1, nColArr, nTab,
                                        SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH, TRUE, TRUE );
            // FALSE bei Spalten momentan ohne Auswirkung
        }
        else if ( pMap->nWID == SC_WID_UNO_NEWPAGE || pMap->nWID == SC_WID_UNO_MANPAGE )
        {
            BOOL bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if (bSet)
                aFunc.InsertPageBreak( TRUE, rRange.aStart, TRUE, TRUE, TRUE );
            else
                aFunc.RemovePageBreak( TRUE, rRange.aStart, TRUE, TRUE, TRUE );
        }
        else
            ScCellRangeObj::SetOnePropertyValue(pMap, aValue);		// base class, no Item WID
    }
}

void ScTableColumnObj::GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                            uno::Any& rAny )
                                                throw(uno::RuntimeException)
{
    if ( pMap )
    {
        ScDocShell* pDocSh = GetDocShell();
        if (!pDocSh)
            throw uno::RuntimeException();

        ScDocument* pDoc = pDocSh->GetDocument();
        const ScRange& rRange = GetRange();
        DBG_ASSERT(rRange.aStart.Col() == rRange.aEnd.Col(), "zuviele Spalten");
        USHORT nCol = rRange.aStart.Col();
        USHORT nTab = rRange.aStart.Tab();

        if ( pMap->nWID == SC_WID_UNO_CELLWID )
        {
            // for hidden column, return original height
            USHORT nWidth = pDoc->GetOriginalWidth( nCol, nTab );
            //	property is 1/100mm, column width is twips
            nWidth = (USHORT) TwipsToHMM(nWidth);
            rAny <<= (sal_Int32)( nWidth );
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLVIS )
        {
            BOOL bVis = !(pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN);
            ScUnoHelpFunctions::SetBoolInAny( rAny, bVis );
        }
        else if ( pMap->nWID == SC_WID_UNO_OWIDTH )
        {
            //!	momentan immer gesetzt ??!?!
            BOOL bOpt = !(pDoc->GetColFlags( nCol, nTab ) & CR_MANUALSIZE);
            ScUnoHelpFunctions::SetBoolInAny( rAny, bOpt );
        }
        else if ( pMap->nWID == SC_WID_UNO_NEWPAGE )
        {
            BOOL bBreak = ( 0 != (pDoc->GetColFlags( nCol, nTab ) & (CR_PAGEBREAK|CR_MANUALBREAK)) );
            ScUnoHelpFunctions::SetBoolInAny( rAny, bBreak );
        }
        else if ( pMap->nWID == SC_WID_UNO_MANPAGE )
        {
            BOOL bBreak = ( 0 != (pDoc->GetColFlags( nCol, nTab ) & (CR_MANUALBREAK)) );
            ScUnoHelpFunctions::SetBoolInAny( rAny, bBreak );
        }
        else
            ScCellRangeObj::GetOnePropertyValue(pMap, rAny);
    }
}

const SfxItemPropertyMap* ScTableColumnObj::GetItemPropertyMap()
{
    return lcl_GetColumnPropertyMap();
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

void ScTableRowObj::SetOnePropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& aValue )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( pMap )
    {
        if ( IsScItemWid( pMap->nWID ) )
        {
            //	for Item WIDs, call ScCellRangesBase directly
            ScCellRangesBase::SetOnePropertyValue(pMap, aValue);
            return;
        }

        //	own properties

        ScDocShell* pDocSh = GetDocShell();
        if (!pDocSh)
            return;													//!	Exception oder so?
        ScDocument* pDoc = pDocSh->GetDocument();
        const ScRange& rRange = GetRange();
        DBG_ASSERT(rRange.aStart.Row() == rRange.aEnd.Row(), "zuviele Zeilen");
        USHORT nRow = rRange.aStart.Row();
        USHORT nTab = rRange.aStart.Tab();
        ScDocFunc aFunc(*pDocSh);

        USHORT nRowArr[2];
        nRowArr[0] = nRowArr[1] = nRow;

        if ( pMap->nWID == SC_WID_UNO_CELLHGT )
        {
            sal_Int32 nNewHeight;
            if ( aValue >>= nNewHeight )
            {
                //	property is 1/100mm, row height is twips
                nNewHeight = HMMToTwips(nNewHeight);
                aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, SC_SIZE_ORIGINAL,
                                        (USHORT)nNewHeight, TRUE, TRUE );
            }
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLVIS )
        {
            BOOL bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
            aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, eMode, 0, TRUE, TRUE );
            //	SC_SIZE_DIRECT mit Groesse 0 blendet aus
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLFILT )
        {
            BOOL bFil = ScUnoHelpFunctions::GetBoolFromAny( aValue );
//			ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
//			aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, eMode, 0, TRUE, TRUE );
            //	SC_SIZE_DIRECT mit Groesse 0 blendet aus
            BYTE nFlags = pDoc->GetRowFlags(nRow, nTab);
            if (bFil)
                nFlags |= CR_FILTERED;
            else
                nFlags &= ~CR_FILTERED;
            pDoc->SetRowFlags(nRow, nTab, nFlags);
        }
        else if ( pMap->nWID == SC_WID_UNO_OHEIGHT )
        {
            BOOL bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if (bOpt)
                aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, SC_SIZE_OPTIMAL, 0, TRUE, TRUE );
            else
            {
                //	set current height again manually
                USHORT nHeight = pDoc->GetOriginalHeight( nRow, nTab );
                aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, nTab, SC_SIZE_ORIGINAL, nHeight, TRUE, TRUE );
            }
        }
        else if ( pMap->nWID == SC_WID_UNO_NEWPAGE || pMap->nWID == SC_WID_UNO_MANPAGE )
        {
            BOOL bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if (bSet)
                aFunc.InsertPageBreak( FALSE, rRange.aStart, TRUE, TRUE, TRUE );
            else
                aFunc.RemovePageBreak( FALSE, rRange.aStart, TRUE, TRUE, TRUE );
        }
        else
            ScCellRangeObj::SetOnePropertyValue(pMap, aValue);		// base class, no Item WID
    }
}

void ScTableRowObj::GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                        uno::Any& rAny )
                                                throw(uno::RuntimeException)
{
    if ( pMap )
    {
        ScDocShell* pDocSh = GetDocShell();
        if (!pDocSh)
            throw uno::RuntimeException();
        ScDocument* pDoc = pDocSh->GetDocument();
        const ScRange& rRange = GetRange();
        DBG_ASSERT(rRange.aStart.Row() == rRange.aEnd.Row(), "zuviele Zeilen");
        USHORT nRow = rRange.aStart.Row();
        USHORT nTab = rRange.aStart.Tab();

        if ( pMap->nWID == SC_WID_UNO_CELLHGT )
        {
            // for hidden row, return original height
            USHORT nHeight = pDoc->GetOriginalHeight( nRow, nTab );
            //	property is 1/100mm, row height is twips
            nHeight = (USHORT) TwipsToHMM(nHeight);
            rAny <<= (sal_Int32)( nHeight );
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLVIS )
        {
            BOOL bVis = !(pDoc->GetRowFlags( nRow, nTab ) & CR_HIDDEN);
            ScUnoHelpFunctions::SetBoolInAny( rAny, bVis );
        }
        else if ( pMap->nWID == SC_WID_UNO_CELLFILT )
        {
            BOOL bVis = ((pDoc->GetRowFlags( nRow, nTab ) & CR_FILTERED) != 0);
            ScUnoHelpFunctions::SetBoolInAny( rAny, bVis );
        }
        else if ( pMap->nWID == SC_WID_UNO_OHEIGHT )
        {
            BOOL bOpt = !(pDoc->GetRowFlags( nRow, nTab ) & CR_MANUALSIZE);
            ScUnoHelpFunctions::SetBoolInAny( rAny, bOpt );
        }
        else if ( pMap->nWID == SC_WID_UNO_NEWPAGE )
        {
            BOOL bBreak = ( 0 != (pDoc->GetRowFlags( nRow, nTab ) & (CR_PAGEBREAK|CR_MANUALBREAK)) );
            ScUnoHelpFunctions::SetBoolInAny( rAny, bBreak );
        }
        else if ( pMap->nWID == SC_WID_UNO_MANPAGE )
        {
            BOOL bBreak = ( 0 != (pDoc->GetRowFlags( nRow, nTab ) & (CR_MANUALBREAK)) );
            ScUnoHelpFunctions::SetBoolInAny( rAny, bBreak );
        }
        else
            ScCellRangeObj::GetOnePropertyValue(pMap, rAny);
    }
}

const SfxItemPropertyMap* ScTableRowObj::GetItemPropertyMap()
{
    return lcl_GetRowPropertyMap();
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
        pDocShell = NULL;		// ungueltig geworden
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
        //!	schneller selber testen?

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
        CheckPos_Impl();					// aPos auf erste passende Zelle setzen
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
                pMark->MarkToMulti();	// needed for GetNextMarkedCell
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
        pMark->MarkToMulti();	// needed for GetNextMarkedCell
    }

    USHORT nCol = aPos.Col();
    USHORT nRow = aPos.Row();
    USHORT nTab = aPos.Tab();
    BOOL bFound = pDocShell->GetDocument()->GetNextMarkedCell( nCol, nRow, nTab, *pMark );
    if (bFound)
        aPos.Set( nCol, nRow, nTab );
    else
        bAtEnd = TRUE;		// kommt nix mehr
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

            delete pMark;		// aus verschobenen Bereichen neu erzeugen
            pMark = NULL;

            if (!bAtEnd)		// aPos anpassen
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
        pDocShell = NULL;		// ungueltig geworden
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

    throw container::NoSuchElementException();		// no more elements
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
        //!	aTotalRange...
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}

ScCellRangeObj* ScCellFormatsObj::GetObjectByIndex_Impl(long nIndex) const
{
    //!	direkt auf die AttrArrays zugreifen !!!!

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

    //!	direkt auf die AttrArrays zugreifen !!!!

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
    return ( getCount() != 0 );		//! immer groesser 0 ??
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
            pIter->DataChanged();	// AttrArray-Index neu suchen
            bDirty = FALSE;
        }

        USHORT nCol1, nCol2, nRow1, nRow2;
        if ( pIter->GetNext( nCol1, nCol2, nRow1, nRow2 ) )
            aNext = ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
        else
            bAtEnd = TRUE;		// kommt nix mehr
    }
    else
        bAtEnd = TRUE;			// Dok weggekommen oder so
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
        //!	und nun ???
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;						// ungueltig geworden
            delete pIter;
            pIter = NULL;
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            bDirty = TRUE;			// AttrArray-Index evtl. ungueltig geworden
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
        throw container::NoSuchElementException();		// no more elements

    // Interface-Typ muss zu ScCellFormatsObj::getElementType passen

    uno::Any aAny;
    uno::Reference<table::XCellRange> xRange = NextObject_Impl();
    aAny <<= xRange;
    return aAny;
}

//------------------------------------------------------------------------

ScUniqueCellFormatsObj::ScUniqueCellFormatsObj(ScDocShell* pDocSh, const ScRange& rRange) :
    pDocShell( pDocSh ),
    aTotalRange( rRange ),
    aRangeLists()
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    DBG_ASSERT( aTotalRange.aStart.Tab() == aTotalRange.aEnd.Tab(), "unterschiedliche Tabellen" );

    GetObjects_Impl();
}

ScUniqueCellFormatsObj::~ScUniqueCellFormatsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScUniqueCellFormatsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        //!	aTotalRange...
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
            pDocShell = NULL;						// ungueltig geworden
    }
}

void ScUniqueCellFormatsObj::GetObjects_Impl()
{
    //!	direkt auf die AttrArrays zugreifen !!!!

    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        long nPos = 0;
        USHORT nTab = aTotalRange.aStart.Tab();
        ScAttrRectIterator aIter( pDoc, nTab,
                                    aTotalRange.aStart.Col(), aTotalRange.aStart.Row(),
                                    aTotalRange.aEnd.Col(), aTotalRange.aEnd.Row() );
        USHORT nCol1, nCol2, nRow1, nRow2;
        std::list<ScRange> aList;
        ScRange aFirst;
        if (aIter.GetNext( nCol1, nCol2, nRow1, nRow2 ) )
        {
            aFirst = ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
            aRangeLists.push_back(ScRangeList());
            aRangeLists[0].Join(aFirst);
        }
        while ( aIter.GetNext( nCol1, nCol2, nRow1, nRow2 ) )
        {
            ScRange aNext( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
            if (pDoc->GetPattern(nCol1, nRow1, nTab) == pDoc->GetPattern(aFirst.aStart.Col(), aFirst.aStart.Row(), aFirst.aStart.Tab()) )
                aRangeLists[0].Join(aNext);
            else
                aList.push_back(aNext);
        }
        if (!aList.empty())
        {
            std::list<ScRange>::iterator aItr = aList.begin();
            aRangeLists.push_back(ScRangeList());
            sal_Int32 nIndex(1);
            aFirst = *aItr;
            aRangeLists[nIndex].Join(aFirst);
            aItr = aList.erase(aItr);
            while (!aList.empty())
            {
                if (pDoc->GetPattern(aItr->aStart.Col(), aItr->aStart.Row(), aItr->aStart.Tab()) ==
                    pDoc->GetPattern(aFirst.aStart.Col(), aFirst.aStart.Row(), aFirst.aStart.Tab()) )
                {
                    aRangeLists[nIndex].Join(*aItr);
                    aItr = aList.erase(aItr);
                }
                else
                    aItr++;
                if (aItr == aList.end() && !aList.empty())
                {
                    aItr = aList.begin();
                    aRangeLists.push_back(ScRangeList());
                    nIndex++;
                    aFirst = *aItr;
                    aRangeLists[nIndex].Join(aFirst);
                    aItr = aList.erase(aItr);
                }
            }
        }
    }
}

// XIndexAccess

sal_Int32 SAL_CALL ScUniqueCellFormatsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return aRangeLists.size();
}

uno::Any SAL_CALL ScUniqueCellFormatsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Any aAny;
    if(static_cast<sal_uInt32>(nIndex) < aRangeLists.size())
    {
        uno::Reference<sheet::XSheetCellRangeContainer> xCellRanges = new ScCellRangesObj(pDocShell, aRangeLists[nIndex]);
        aAny <<= xCellRanges;
    }
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScUniqueCellFormatsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XSheetCellRangeContainer>*)0);
}

sal_Bool SAL_CALL ScUniqueCellFormatsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( aRangeLists.size() != 0 );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScUniqueCellFormatsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
        return new ScUniqueCellFormatsEnumeration( pDocShell, aRangeLists );
    return NULL;
}

//------------------------------------------------------------------------

ScUniqueCellFormatsEnumeration::ScUniqueCellFormatsEnumeration(ScDocShell* pDocSh, const ScMyRangeLists& rRangeLists) :
    pDocShell( pDocSh ),
    aRangeLists(rRangeLists),
    nCurrentPosition(0)
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScUniqueCellFormatsEnumeration::~ScUniqueCellFormatsEnumeration()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScUniqueCellFormatsEnumeration::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        //!	und nun ???
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
            pDocShell = NULL;						// ungueltig geworden
    }
}

// XEnumeration

sal_Bool SAL_CALL ScUniqueCellFormatsEnumeration::hasMoreElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return static_cast<sal_uInt32>(nCurrentPosition + 1) < aRangeLists.size();
}

uno::Any SAL_CALL ScUniqueCellFormatsEnumeration::nextElement() throw(container::NoSuchElementException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !hasMoreElements() || !pDocShell )
        throw container::NoSuchElementException();		// no more elements

    // Interface-Typ muss zu ScCellFormatsObj::getElementType passen

    nCurrentPosition++;
    uno::Reference<sheet::XSheetCellRangeContainer> xCellRanges = new ScCellRangesObj(pDocShell, aRangeLists[nCurrentPosition]);
    uno::Any aAny;
    aAny <<= xCellRanges;
    return aAny;
}


}

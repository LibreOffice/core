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

#include "formmetadata.hxx"
#include "formstrings.hxx"
#include "formresid.hrc"
#include "propctrlr.hrc"
#include <svtools/localresaccess.hxx>
#include <tools/debug.hxx>
#include <comphelper/extract.hxx>
#include <sal/macros.h>
#include <algorithm>
#include <functional>

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;

    //========================================================================
    //= OPropertyInfoImpl
    //========================================================================
    struct OPropertyInfoImpl
    {
        String          sName;
        String          sTranslation;
        rtl::OString    sHelpId;
        sal_Int32       nId;
        sal_uInt16      nPos;
        sal_uInt32      nUIFlags;

        OPropertyInfoImpl(
                        const ::rtl::OUString&      rName,
                        sal_Int32                   _nId,
                        const String&               aTranslation,
                        sal_uInt16                  nPosId,
                        const rtl::OString&,
                        sal_uInt32                  _nUIFlags);
    };

    //------------------------------------------------------------------------
    OPropertyInfoImpl::OPropertyInfoImpl(const ::rtl::OUString& _rName, sal_Int32 _nId,
                                   const String& aString, sal_uInt16 nP, const rtl::OString& sHid, sal_uInt32 _nUIFlags)
       :sName(_rName)
       ,sTranslation(aString)
       ,sHelpId(sHid)
       ,nId(_nId)
       ,nPos(nP)
       ,nUIFlags(_nUIFlags)
    {
    }

    //------------------------------------------------------------------------
    // Compare PropertyInfo
    struct PropertyInfoLessByName : public ::std::binary_function< OPropertyInfoImpl, OPropertyInfoImpl, bool >
    {
        bool operator()( const OPropertyInfoImpl& _rLHS, const OPropertyInfoImpl& _rRHS )
        {
            return _rLHS.sName.CompareTo( _rRHS.sName ) == COMPARE_LESS;
        }
    };

    //========================================================================
    //= OPropertyInfoService
    //========================================================================
#define DEF_INFO( ident, uinameres, helpid, flags )   \
    OPropertyInfoImpl( PROPERTY_##ident, PROPERTY_ID_##ident, \
            String( PcrRes( RID_STR_##uinameres ) ), nPos++, HID_PROP_##helpid, flags )

#define DEF_INFO_1( ident, uinameres, helpid, flag1 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 )

#define DEF_INFO_2( ident, uinameres, helpid, flag1, flag2 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 )

#define DEF_INFO_3( ident, uinameres, helpid, flag1, flag2, flag3 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 )

#define DEF_INFO_4( ident, uinameres, helpid, flag1, flag2, flag3, flag4 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 | PROP_FLAG_##flag4 )

#define DEF_INFO_5( ident, uinameres, helpid, flag1, flag2, flag3, flag4, flag5 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 | PROP_FLAG_##flag4 | PROP_FLAG_##flag5 )

    sal_uInt16              OPropertyInfoService::s_nCount = 0;
    OPropertyInfoImpl*      OPropertyInfoService::s_pPropertyInfos = NULL;
    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo()
    {
        if ( s_pPropertyInfos )
            return s_pPropertyInfos;

        PcrClient aResourceAccess;
        // this ensures that we have our resource file loaded

        sal_uInt16 nPos = 1;

        static OPropertyInfoImpl aPropertyInfos[] =
        {
        /*
        DEF_INFO_?( propname and id,   resoure id,         help id,           flags ),
        */
        DEF_INFO_3( NAME,              NAME,               NAME,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( TITLE,             TITLE,              TITLE,             FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( LABEL,             LABEL,              LABEL,             FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( CONTROLLABEL,      LABELCONTROL,       CONTROLLABEL,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( WRITING_MODE,      WRITING_MODE,       WRITING_MODE,      FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( GROUP_NAME,        GROUP_NAME,         GROUP_NAME,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( TEXT,              TEXT,               TEXT,              DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( MAXTEXTLEN,        MAXTEXTLEN,         MAXTEXTLEN,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( EDITMASK,          EDITMASK,           EDITMASK,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LITERALMASK,       LITERALMASK,        LITERALMASK,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( STRICTFORMAT,      STRICTFORMAT,       STRICTFORMAT,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ENABLED,           ENABLED,            ENABLED,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ENABLE_VISIBLE,    ENABLE_VISIBLE,     ENABLE_VISIBLE,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( READONLY,          READONLY,           READONLY,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( PRINTABLE,         PRINTABLE,          PRINTABLE,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( STEP,              STEP,               STEP,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TABSTOP,           TABSTOP,            TABSTOP,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( TABINDEX,          TABINDEX,           TABINDEX,          FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( WHEEL_BEHAVIOR,    WHEEL_BEHAVIOR,     WHEEL_BEHAVIOR,    FORM_VISIBLE, ENUM, COMPOSEABLE ),

        DEF_INFO_2( BOUND_CELL,        BOUND_CELL,         BOUND_CELL,        FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_3( CELL_EXCHANGE_TYPE,CELL_EXCHANGE_TYPE, CELL_EXCHANGE_TYPE,FORM_VISIBLE, DATA_PROPERTY, ENUM ),
        DEF_INFO_2( LIST_CELL_RANGE,   LIST_CELL_RANGE,    LIST_CELL_RANGE,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_3( CONTROLSOURCE,     CONTROLSOURCE,      CONTROLSOURCE,     FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( EMPTY_IS_NULL,     EMPTY_IS_NULL,      EMPTY_IS_NULL,     FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( INPUT_REQUIRED,    INPUT_REQUIRED,     INPUT_REQUIRED,    FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( REFVALUE,          REFVALUE,           REFVALUE,          FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( UNCHECKEDREFVALUE, UNCHECKEDREFVALUE,  UNCHECKEDREFVALUE, FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( DATASOURCE,        DATASOURCE,         DATASOURCE,        FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_4( COMMANDTYPE,       CURSORSOURCETYPE,   CURSORSOURCETYPE,  FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_3( COMMAND,           CURSORSOURCE,       CURSORSOURCE,      FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( ESCAPE_PROCESSING, ESCAPE_PROCESSING,  ESCAPE_PROCESSING, FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( FILTER,            FILTER,             FILTER,            FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( SORT,              SORT_CRITERIA,      SORT_CRITERIA,     FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_2( MASTERFIELDS,      MASTERFIELDS,       MASTERFIELDS,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( DETAILFIELDS,      SLAVEFIELDS,        SLAVEFIELDS,       FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_3( ALLOWADDITIONS,    ALLOW_ADDITIONS,    ALLOW_ADDITIONS,   FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( ALLOWEDITS,        ALLOW_EDITS,        ALLOW_EDITS,       FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( ALLOWDELETIONS,    ALLOW_DELETIONS,    ALLOW_DELETIONS,   FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( INSERTONLY,        DATAENTRY,          DATAENTRY,         FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_4( NAVIGATION,        NAVIGATION,         NAVIGATION,        FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_4( CYCLE,             CYCLE,              CYCLE,             FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_3( FILTERPROPOSAL,    FILTERPROPOSAL,     FILTERPROPOSAL,    FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_4( LISTSOURCETYPE,    LISTSOURCETYPE,     LISTSOURCETYPE,    FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_3( LISTSOURCE,        LISTSOURCE,         LISTSOURCE,        FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( BOUNDCOLUMN,       BOUNDCOLUMN,        BOUNDCOLUMN,       FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),

        // <!----------------->
        // XML node binding
        DEF_INFO_2( LIST_BINDING,      LIST_BINDING,       LIST_BINDING,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XML_DATA_MODEL,    XML_DATA_MODEL,     XML_DATA_MODEL,    FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( BINDING_NAME,      BINDING_NAME,       BINDING_NAME,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( BIND_EXPRESSION,   BIND_EXPRESSION,    BIND_EXPRESSION,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_REQUIRED,      XSD_REQUIRED,       XSD_REQUIRED,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_RELEVANT,      XSD_RELEVANT,       XSD_RELEVANT,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_READONLY,      XSD_READONLY,       XSD_READONLY,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_CONSTRAINT,    XSD_CONSTRAINT,     XSD_CONSTRAINT,    FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_CALCULATION,   XSD_CALCULATION,    XSD_CALCULATION,   FORM_VISIBLE, DATA_PROPERTY ),

        // data type
        DEF_INFO_2( XSD_DATA_TYPE,     XSD_DATA_TYPE,      XSD_DATA_TYPE,     FORM_VISIBLE, DATA_PROPERTY ),
        // data types facets
        //  common
        DEF_INFO_3( XSD_WHITESPACES,   XSD_WHITESPACES,    XSD_WHITESPACES,   FORM_VISIBLE, DATA_PROPERTY, ENUM ),
        DEF_INFO_2( XSD_PATTERN,       XSD_PATTERN,        XSD_PATTERN,       FORM_VISIBLE, DATA_PROPERTY ),
        //  string
        DEF_INFO_2( XSD_LENGTH,        XSD_LENGTH,         XSD_LENGTH,        FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_LENGTH,    XSD_MIN_LENGTH,     XSD_MIN_LENGTH,    FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_LENGTH,    XSD_MAX_LENGTH,     XSD_MAX_LENGTH,    FORM_VISIBLE, DATA_PROPERTY ),
        //  decimal
        DEF_INFO_2( XSD_TOTAL_DIGITS,   XSD_TOTAL_DIGITS,   XSD_TOTAL_DIGITS,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_FRACTION_DIGITS,XSD_FRACTION_DIGITS,XSD_FRACTION_DIGITS,FORM_VISIBLE, DATA_PROPERTY ),
        //  int value types (year, month, day)
        DEF_INFO_2( XSD_MAX_INCLUSIVE_INT, XSD_MAX_INCLUSIVE, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_INT, XSD_MAX_EXCLUSIVE, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_INT, XSD_MIN_INCLUSIVE, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_INT, XSD_MIN_EXCLUSIVE, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        //  double value types (double, float, decimal)
        DEF_INFO_2( XSD_MAX_INCLUSIVE_DOUBLE, XSD_MAX_INCLUSIVE, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_DOUBLE, XSD_MAX_EXCLUSIVE, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_DOUBLE, XSD_MIN_INCLUSIVE, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_DOUBLE, XSD_MIN_EXCLUSIVE, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // date value type
        DEF_INFO_2( XSD_MAX_INCLUSIVE_DATE, XSD_MAX_INCLUSIVE, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_DATE, XSD_MAX_EXCLUSIVE, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_DATE, XSD_MIN_INCLUSIVE, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_DATE, XSD_MIN_EXCLUSIVE, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // time value type
        DEF_INFO_2( XSD_MAX_INCLUSIVE_TIME, XSD_MAX_INCLUSIVE, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_TIME, XSD_MAX_EXCLUSIVE, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_TIME, XSD_MIN_INCLUSIVE, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_TIME, XSD_MIN_EXCLUSIVE, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // dateTime value type
        DEF_INFO_2( XSD_MAX_INCLUSIVE_DATE_TIME, XSD_MAX_INCLUSIVE, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_DATE_TIME, XSD_MAX_EXCLUSIVE, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_DATE_TIME, XSD_MIN_INCLUSIVE, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_DATE_TIME, XSD_MIN_EXCLUSIVE, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // <!----------------->

        DEF_INFO_2( HIDDEN_VALUE,      VALUE,              HIDDEN_VALUE,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( VALUE,             VALUE,              VALUE,             DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VALUEMIN,          VALUEMIN,           VALUEMIN,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VALUEMAX,          VALUEMAX,           VALUEMAX,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VALUESTEP,         VALUESTEP,          VALUESTEP,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_VALUE,     DEFAULTVALUE,       DEFAULT_LONG_VALUE,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DECIMAL_ACCURACY,  DECIMAL_ACCURACY,   DECIMAL_ACCURACY,  FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SHOWTHOUSANDSEP,   SHOWTHOUSANDSEP,    SHOWTHOUSANDSEP,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( CURRENCYSYMBOL,    CURRENCYSYMBOL,     CURRENCYSYMBOL,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( CURRSYM_POSITION,  CURRSYM_POSITION,   CURRSYM_POSITION,  FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( DATE,              DATE,               DATE,              DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DATEMIN,           DATEMIN,            DATEMIN,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DATEMAX,           DATEMAX,            DATEMAX,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( DATEFORMAT,        DATEFORMAT,         DATEFORMAT,        FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_DATE,      DEFAULTDATE,        DEFAULT_DATE,      FORM_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( TIME,              TIME,               TIME,              DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TIMEMIN,           TIMEMIN,            TIMEMIN,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TIMEMAX,           TIMEMAX,            TIMEMAX,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( TIMEFORMAT,        TIMEFORMAT,         TIMEFORMAT,        FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_TIME,      DEFAULTTIME,        DEFAULT_TIME,      FORM_VISIBLE, COMPOSEABLE ),

        DEF_INFO_1( EFFECTIVE_VALUE,   VALUE,              VALUE,             DIALOG_VISIBLE ),
        DEF_INFO_3( EFFECTIVE_MIN,     VALUEMIN,           EFFECTIVEMIN,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( EFFECTIVE_MAX,     VALUEMAX,           EFFECTIVEMAX,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( EFFECTIVE_DEFAULT, DEFAULTVALUE,       EFFECTIVEDEFAULT,  FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( FORMATKEY,         FORMATKEY,          FORMATKEY,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( PROGRESSVALUE,     PROGRESSVALUE,      PROGRESSVALUE,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( PROGRESSVALUE_MIN, PROGRESSVALUE_MIN,  PROGRESSVALUE_MIN, FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( PROGRESSVALUE_MAX, PROGRESSVALUE_MAX,  PROGRESSVALUE_MAX, FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( SCROLLVALUE,       SCROLLVALUE,        SCROLLVALUE,       DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SCROLLVALUE_MIN,   SCROLLVALUE_MIN,    SCROLLVALUE_MIN,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SCROLLVALUE_MAX,   SCROLLVALUE_MAX,    SCROLLVALUE_MAX,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SCROLL_WIDTH,      SCROLL_WIDTH,       SCROLL_WIDTH,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SCROLL_HEIGHT,     SCROLL_HEIGHT,      SCROLL_HEIGHT,     DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SCROLL_TOP,        SCROLL_TOP,         SCROLL_TOP,        DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SCROLL_LEFT,       SCROLL_LEFT,        SCROLL_LEFT,       DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_SCROLLVALUE,DEFAULT_SCROLLVALUE,DEFAULT_SCROLLVALUE,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LINEINCREMENT,     LINEINCREMENT,      LINEINCREMENT,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( BLOCKINCREMENT,    BLOCKINCREMENT,     BLOCKINCREMENT,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( SPINVALUE,        VALUE,               SPINVALUE,         DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SPINVALUE_MIN,    VALUEMIN,            SPINVALUE_MIN,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SPINVALUE_MAX,    VALUEMAX,            SPINVALUE_MAX,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_SPINVALUE,DEFAULTVALUE,        DEFAULT_SPINVALUE, FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SPININCREMENT,    VALUESTEP,           SPININCREMENT,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( SPIN,              SPIN,               SPIN,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( REPEAT,            REPEAT,             REPEAT,            FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( REPEAT_DELAY,      REPEAT_DELAY,       REPEAT_DELAY,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VISIBLESIZE,       VISIBLESIZE,        VISIBLESIZE,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( ORIENTATION,       ORIENTATION,        ORIENTATION,       FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( FOCUSONCLICK,      FOCUSONCLICK,       FOCUSONCLICK,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TOGGLE,            TOGGLE,             TOGGLE,            FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DEFAULT_STATE,     DEFAULT_STATE,      DEFAULT_STATE,     FORM_VISIBLE, ENUM, COMPOSEABLE ),

        DEF_INFO_3( TEXT_ANCHOR_TYPE,  ANCHOR_TYPE,        ANCHOR_TYPE,       FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( SHEET_ANCHOR_TYPE, ANCHOR_TYPE,        ANCHOR_TYPE,       FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( POSITIONX,         POSITIONX,          POSITIONX,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( POSITIONY,         POSITIONY,          POSITIONY,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( WIDTH,             WIDTH,              WIDTH,             FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HEIGHT,            HEIGHT,             HEIGHT,            FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_1( LISTINDEX,         LISTINDEX,          LISTINDEX,         FORM_VISIBLE ),
        DEF_INFO_3( STRINGITEMLIST,    STRINGITEMLIST,     STRINGITEMLIST,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_TEXT,      DEFAULTTEXT,        DEFAULTVALUE,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( FONT,              FONT,               FONT,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( VISUALEFFECT,      VISUALEFFECT,       VISUALEFFECT,      FORM_VISIBLE, DIALOG_VISIBLE, ENUM_ONE, COMPOSEABLE ),
        DEF_INFO_4( ALIGN,             ALIGN,              ALIGN,             FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( VERTICAL_ALIGN,    VERTICAL_ALIGN,     VERTICAL_ALIGN,    FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( ROWHEIGHT,         ROWHEIGHT,          ROWHEIGHT,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( BACKGROUNDCOLOR,   BACKGROUNDCOLOR,    BACKGROUNDCOLOR,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SYMBOLCOLOR,       SYMBOLCOLOR,        SYMBOLCOLOR,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( FILLCOLOR,         FILLCOLOR,          FILLCOLOR,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LINECOLOR,         LINECOLOR,          LINECOLOR,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( BORDER,            BORDER,             BORDER,            FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( BORDERCOLOR,       BORDERCOLOR,        BORDERCOLOR,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ICONSIZE,          ICONSIZE,           ICONSIZE,          FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( SHOW_POSITION,     SHOW_POSITION,      SHOW_POSITION,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOW_NAVIGATION,   SHOW_NAVIGATION,    SHOW_NAVIGATION,   FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOW_RECORDACTIONS,SHOW_RECORDACTIONS, SHOW_RECORDACTIONS,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOW_FILTERSORT,   SHOW_FILTERSORT,    SHOW_FILTERSORT,   FORM_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( DROPDOWN,          DROPDOWN,           DROPDOWN,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LINECOUNT,         LINECOUNT,          LINECOUNT,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( AUTOCOMPLETE,      AUTOCOMPLETE,       AUTOCOMPLETE,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( MULTILINE,         MULTILINE,          MULTILINE,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( WORDBREAK,         WORDBREAK,          WORDBREAK,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TEXTTYPE,          TEXTTYPE,           TEXTTYPE,          FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( LINEEND_FORMAT,    LINEEND_FORMAT,     LINEEND_FORMAT,    FORM_VISIBLE, ENUM_ONE, COMPOSEABLE ),
        DEF_INFO_3( MULTISELECTION,    MULTISELECTION,     MULTISELECTION,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( SHOW_SCROLLBARS,   SHOW_SCROLLBARS,    SHOW_SCROLLBARS,   FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( HSCROLL,           HSCROLL,            HSCROLL,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VSCROLL,           VSCROLL,            VSCROLL,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( BUTTONTYPE,        BUTTONTYPE,         BUTTONTYPE,        FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( XFORMS_BUTTONTYPE, BUTTONTYPE,         BUTTONTYPE,        FORM_VISIBLE, ENUM ),
        DEF_INFO_1( SUBMISSION_ID,     SUBMISSION_ID,      SUBMISSION_ID,     FORM_VISIBLE ),
        DEF_INFO_2( PUSHBUTTONTYPE,    PUSHBUTTONTYPE,     PUSHBUTTONTYPE,    DIALOG_VISIBLE, ENUM ),
        DEF_INFO_2( TARGET_URL,        TARGET_URL,         TARGET_URL,        FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_1( TARGET_FRAME,      TARGET_FRAME,       TARGET_FRAME,      FORM_VISIBLE ),
        DEF_INFO_2( SUBMIT_ACTION,     SUBMIT_ACTION,      SUBMIT_ACTION,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SUBMIT_TARGET,     SUBMIT_TARGET,      SUBMIT_TARGET,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SUBMIT_ENCODING,   SUBMIT_ENCODING,    SUBMIT_ENCODING,   FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( SUBMIT_METHOD,     SUBMIT_METHOD,      SUBMIT_METHOD,     FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( STATE,             STATE,              STATE,             DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( DEFAULTBUTTON,     DEFAULT_BUTTON,     DEFAULT_BUTTON,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( IMAGE_URL,         IMAGE_URL,          IMAGE_URL,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( IMAGEPOSITION,     IMAGEPOSITION,      IMAGEPOSITION,     FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( SCALEIMAGE,        SCALEIMAGE,         SCALEIMAGE,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( SCALE_MODE,        SCALEIMAGE,         SCALEIMAGE,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE, ENUM ),
        DEF_INFO_2( DEFAULT_SELECT_SEQ,DEFAULT_SELECT_SEQ, DEFAULT_SELECT_SEQ,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SELECTEDITEMS,     SELECTEDITEMS,      SELECTEDITEMS,     DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ECHO_CHAR,         ECHO_CHAR,          ECHO_CHAR,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HIDEINACTIVESELECTION, HIDEINACTIVESELECTION, HIDEINACTIVESELECTION, FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TRISTATE,          TRISTATE,           TRISTATE,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( HASNAVIGATION,     NAVIGATION,         NAVIGATIONBAR,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( RECORDMARKER,      RECORDMARKER,       RECORDMARKER,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TAG,               TAG,                TAG,               FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HELPTEXT,          HELPTEXT,           HELPTEXT,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HELPURL,           HELPURL,            HELPURL,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SELECTION_TYPE,    SELECTION_TYPE,     SELECTION_TYPE,                  DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( ROOT_DISPLAYED,    ROOT_DISPLAYED,     ROOT_DISPLAYED,                  DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOWS_HANDLES,     SHOWS_HANDLES,      SHOWS_HANDLES,                   DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOWS_ROOT_HANDLES, SHOWS_ROOT_HANDLES, SHOWS_ROOT_HANDLES,             DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( EDITABLE,          EDITABLE,           EDITABLE,                        DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( INVOKES_STOP_NOT_EDITING, INVOKES_STOP_NOT_EDITING, INVOKES_STOP_NOT_EDITING, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DECORATION,        DECORATION,         DECORATION,                      DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( NOLABEL,           NOLABEL,            NOLABEL,                         DIALOG_VISIBLE, COMPOSEABLE )
        };

        s_pPropertyInfos = aPropertyInfos;
        s_nCount = SAL_N_ELEMENTS(aPropertyInfos);

        // sort
        ::std::sort( s_pPropertyInfos, s_pPropertyInfos + s_nCount, PropertyInfoLessByName() );

#if OSL_DEBUG_LEVEL > 0
        for ( const OPropertyInfoImpl* pCheck = s_pPropertyInfos; pCheck != s_pPropertyInfos + s_nCount - 1; ++pCheck )
        {
            OSL_ENSURE( pCheck->sName != ( pCheck + 1 )->sName, "OPropertyInfoService::getPropertyInfo: duplicate entry in the table!" );
        }
#endif

        return s_pPropertyInfos;
    }

    //------------------------------------------------------------------------
    sal_Int32 OPropertyInfoService::getPropertyId(const String& _rName) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_rName);
        return pInfo ? pInfo->nId : -1;
    }

    //------------------------------------------------------------------------
    String OPropertyInfoService::getPropertyName( sal_Int32 _nPropId )
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nPropId);
        return pInfo ? pInfo->sName : String();
    }

    //------------------------------------------------------------------------
    String OPropertyInfoService::getPropertyTranslation(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->sTranslation : String();
    }

    //------------------------------------------------------------------------
    rtl::OString OPropertyInfoService::getPropertyHelpId(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->sHelpId : rtl::OString();
    }

    //------------------------------------------------------------------------
    sal_Int16 OPropertyInfoService::getPropertyPos(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nPos : 0xFFFF;
    }

    //------------------------------------------------------------------------
    sal_uInt32 OPropertyInfoService::getPropertyUIFlags(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nUIFlags : 0;
    }

    //------------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > OPropertyInfoService::getPropertyEnumRepresentations(sal_Int32 _nId) const
    {
        OSL_ENSURE( ( ( getPropertyUIFlags( _nId ) & PROP_FLAG_ENUM ) != 0 ) || ( _nId == PROPERTY_ID_TARGET_FRAME ),
            "OPropertyInfoService::getPropertyEnumRepresentations: this is no enum property!" );

        sal_Int16 nStringItemsResId = 0;
        switch ( _nId )
        {
            case PROPERTY_ID_IMAGEPOSITION:
                nStringItemsResId = RID_RSC_ENUM_IMAGE_POSITION;
                break;
            case PROPERTY_ID_BORDER:
                nStringItemsResId = RID_RSC_ENUM_BORDER_TYPE;
                break;
            case PROPERTY_ID_ICONSIZE:
                nStringItemsResId = RID_RSC_ENUM_ICONSIZE_TYPE;
                break;
            case PROPERTY_ID_COMMANDTYPE:
                nStringItemsResId = RID_RSC_ENUM_COMMAND_TYPE;
                break;
            case PROPERTY_ID_LISTSOURCETYPE:
                nStringItemsResId = RID_RSC_ENUM_LISTSOURCE_TYPE;
                break;
            case PROPERTY_ID_ALIGN:
                nStringItemsResId = RID_RSC_ENUM_ALIGNMENT;
                break;
            case PROPERTY_ID_VERTICAL_ALIGN:
                nStringItemsResId = RID_RSC_ENUM_VERTICAL_ALIGN;
                break;
            case PROPERTY_ID_BUTTONTYPE:
                nStringItemsResId = RID_RSC_ENUM_BUTTONTYPE;
                break;
            case PROPERTY_ID_PUSHBUTTONTYPE:
                nStringItemsResId = RID_RSC_ENUM_PUSHBUTTONTYPE;
                break;
            case PROPERTY_ID_SUBMIT_METHOD:
                nStringItemsResId = RID_RSC_ENUM_SUBMIT_METHOD;
                break;
            case PROPERTY_ID_SUBMIT_ENCODING:
                nStringItemsResId = RID_RSC_ENUM_SUBMIT_ENCODING;
                break;
            case PROPERTY_ID_DATEFORMAT:
                nStringItemsResId = RID_RSC_ENUM_DATEFORMAT_LIST;
                break;
            case PROPERTY_ID_TIMEFORMAT:
                nStringItemsResId = RID_RSC_ENUM_TIMEFORMAT_LIST;
                break;
            case PROPERTY_ID_DEFAULT_STATE:
            case PROPERTY_ID_STATE:
                nStringItemsResId = RID_RSC_ENUM_CHECKED;
                break;
            case PROPERTY_ID_CYCLE:
                nStringItemsResId = RID_RSC_ENUM_CYCLE;
                break;
            case PROPERTY_ID_NAVIGATION:
                nStringItemsResId = RID_RSC_ENUM_NAVIGATION;
                break;
            case PROPERTY_ID_TARGET_FRAME:
                nStringItemsResId = RID_RSC_ENUM_SUBMIT_TARGET;
                break;
            case PROPERTY_ID_ORIENTATION:
                nStringItemsResId = RID_RSC_ENUM_ORIENTATION;
                break;
            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                nStringItemsResId = RID_RSC_ENUM_CELL_EXCHANGE_TYPE;
                break;
            case PROPERTY_ID_SHOW_SCROLLBARS:
                nStringItemsResId = RID_RSC_ENUM_SCROLLBARS;
                break;
            case PROPERTY_ID_VISUALEFFECT:
                nStringItemsResId = RID_RSC_ENUM_VISUALEFFECT;
                break;
            case PROPERTY_ID_TEXTTYPE:
                nStringItemsResId = RID_RSC_ENUM_TEXTTYPE;
                break;
            case PROPERTY_ID_LINEEND_FORMAT:
                nStringItemsResId = RID_RSC_ENUM_LINEEND_FORMAT;
                break;
            case PROPERTY_ID_XSD_WHITESPACES:
                nStringItemsResId = RID_RSC_ENUM_WHITESPACE_HANDLING;
                break;
            case PROPERTY_ID_SELECTION_TYPE:
                nStringItemsResId = RID_RSC_ENUM_SELECTION_TYPE;
                break;
            case PROPERTY_ID_SCALE_MODE:
                nStringItemsResId = RID_RSC_ENUM_SCALE_MODE;
                break;
            case PROPERTY_ID_WRITING_MODE:
                nStringItemsResId = RID_RSC_ENUM_WRITING_MODE;
                break;
            case PROPERTY_ID_WHEEL_BEHAVIOR:
                nStringItemsResId = RID_RSC_ENUM_WHEEL_BEHAVIOR;
                break;
            case PROPERTY_ID_TEXT_ANCHOR_TYPE:
                nStringItemsResId = RID_RSC_ENUM_TEXT_ANCHOR_TYPE;
                break;
            case PROPERTY_ID_SHEET_ANCHOR_TYPE:
                nStringItemsResId = RID_RSC_ENUM_SHEET_ANCHOR_TYPE;
                break;
            default:
                OSL_FAIL( "OPropertyInfoService::getPropertyEnumRepresentations: unknown enum property!" );
                break;
        }

        ::std::vector< ::rtl::OUString > aReturn;

        if ( nStringItemsResId )
        {
            PcrRes aResId( nStringItemsResId );
            ::svt::OLocalResourceAccess aEnumStrings( aResId, RSC_RESOURCE );

            sal_Int16 i = 1;
            PcrRes aLocalId( i );
            while ( aEnumStrings.IsAvailableRes( aLocalId.SetRT( RSC_STRING ) ) )
            {
                aReturn.push_back( String( aLocalId ) );
                aLocalId = PcrRes( ++i );
            }
        }

        return aReturn;
    }

    //------------------------------------------------------------------------
    sal_Bool OPropertyInfoService::isComposeable( const ::rtl::OUString& _rPropertyName ) const
    {
        sal_Int32 nId = getPropertyId( _rPropertyName );
        if ( nId == -1 )
            return sal_False;

        sal_uInt32 nFlags = getPropertyUIFlags( nId );
        return ( nFlags & PROP_FLAG_COMPOSEABLE ) != 0;
    }

    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(const String& _rName)
    {
        // Initialization
        if(!s_pPropertyInfos)
            getPropertyInfo();
        OPropertyInfoImpl  aSearch(_rName, 0L, String(), 0, "", 0);

        const OPropertyInfoImpl* pInfo = ::std::lower_bound(
            s_pPropertyInfos, s_pPropertyInfos + s_nCount, aSearch, PropertyInfoLessByName() );

        if ( pInfo == s_pPropertyInfos + s_nCount )
            return NULL;

        if ( pInfo->sName != _rName )
            return NULL;

        return pInfo;
    }


    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(sal_Int32 _nId)
    {
        // Initialization
        if(!s_pPropertyInfos)
            getPropertyInfo();

        // TODO: a real structure which allows quick access by name as well as by id
        for (sal_uInt16 i = 0; i < s_nCount; i++)
            if (s_pPropertyInfos[i].nId == _nId)
                return &s_pPropertyInfos[i];

        return NULL;
    }

    //====================================================================
    //= DefaultEnumRepresentation
    //====================================================================
    DBG_NAME( DefaultEnumRepresentation )
    //--------------------------------------------------------------------
    DefaultEnumRepresentation::DefaultEnumRepresentation( const IPropertyInfoService& _rInfo, const Type& _rType, sal_Int32 _nPropertyId )
        :m_refCount( 0 )
        ,m_rMetaData( _rInfo )
        ,m_aType( _rType )
        ,m_nPropertyId( _nPropertyId )
    {
        DBG_CTOR( DefaultEnumRepresentation, NULL );
    }

    //--------------------------------------------------------------------
    DefaultEnumRepresentation::~DefaultEnumRepresentation()
    {
        DBG_DTOR( DefaultEnumRepresentation, NULL );
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL DefaultEnumRepresentation::getDescriptions() const
    {
        return m_rMetaData.getPropertyEnumRepresentations( m_nPropertyId );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DefaultEnumRepresentation::getValueFromDescription( const ::rtl::OUString& _rDescription, Any& _out_rValue ) const
    {
        sal_uInt32  nPropertyUIFlags = m_rMetaData.getPropertyUIFlags( m_nPropertyId );
        ::std::vector< ::rtl::OUString > aEnumStrings = m_rMetaData.getPropertyEnumRepresentations( m_nPropertyId );
        ::std::vector< ::rtl::OUString >::const_iterator pos = ::std::find( aEnumStrings.begin(), aEnumStrings.end(), _rDescription );
        if ( pos != aEnumStrings.end() )
        {
            sal_Int32 nPos = pos - aEnumStrings.begin();
            if ( ( nPropertyUIFlags & PROP_FLAG_ENUM_ONE ) == PROP_FLAG_ENUM_ONE )
                // enum value starting with 1
                ++nPos;

            switch ( m_aType.getTypeClass() )
            {
                case TypeClass_ENUM:
                    _out_rValue = ::cppu::int2enum( nPos, m_aType );
                    break;

                case TypeClass_SHORT:
                    _out_rValue <<= (sal_Int16)nPos;
                    break;

                case TypeClass_UNSIGNED_SHORT:
                    _out_rValue <<= (sal_uInt16)nPos;
                    break;

                case TypeClass_UNSIGNED_LONG:
                    _out_rValue <<= (sal_uInt32)nPos;
                    break;

                default:
                    _out_rValue <<= (sal_Int32)nPos;
                    break;
            }
        }
        else
        {
            OSL_FAIL( "DefaultEnumRepresentation::getValueFromDescription: could not translate the enum string!" );
            _out_rValue.clear();
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DefaultEnumRepresentation::getDescriptionForValue( const Any& _rEnumValue ) const
    {
        ::rtl::OUString sReturn;
        sal_Int32 nIntValue = -1;
        OSL_VERIFY( ::cppu::enum2int( nIntValue, _rEnumValue ) );

        sal_uInt32 nUIFlags = m_rMetaData.getPropertyUIFlags( m_nPropertyId );
        if ( ( nUIFlags & PROP_FLAG_ENUM_ONE ) == PROP_FLAG_ENUM_ONE )
            // enum value starting with 1
            --nIntValue;

        ::std::vector< ::rtl::OUString > aEnumStrings = m_rMetaData.getPropertyEnumRepresentations( m_nPropertyId );
        if ( ( nIntValue >= 0 ) && ( nIntValue < (sal_Int32)aEnumStrings.size() ) )
        {
            sReturn = aEnumStrings[ nIntValue ];
        }
        else
        {
            OSL_FAIL( "DefaultEnumRepresentation::getDescriptionForValue: could not translate an enum value" );
        }
        return sReturn;
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL DefaultEnumRepresentation::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL DefaultEnumRepresentation::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
           delete this;
           return 0;
        }
        return m_refCount;
    }

//............................................................................
} // namespace pcr
//............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

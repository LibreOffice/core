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
        OString    sHelpId;
        sal_Int32       nId;
        sal_uInt16      nPos;
        sal_uInt32      nUIFlags;

        OPropertyInfoImpl(
                        const OUString&      rName,
                        sal_Int32                   _nId,
                        const String&               aTranslation,
                        sal_uInt16                  nPosId,
                        const OString&,
                        sal_uInt32                  _nUIFlags);
    };

    //------------------------------------------------------------------------
    OPropertyInfoImpl::OPropertyInfoImpl(const OUString& _rName, sal_Int32 _nId,
                                   const String& aString, sal_uInt16 nP, const OString& sHid, sal_uInt32 _nUIFlags)
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
#define DEF_INFO( ident, uinameres, pos, helpid, flags )       \
    OPropertyInfoImpl( PROPERTY_##ident, PROPERTY_ID_##ident, \
            PcrRes( RID_STR_##uinameres ).toString(), pos, HID_PROP_##helpid, flags )

#define DEF_INFO_1( ident, uinameres, pos, helpid, flag1 )   \
    DEF_INFO( ident, uinameres, pos, helpid, PROP_FLAG_##flag1 )

#define DEF_INFO_2( ident, uinameres, pos, helpid, flag1, flag2 )        \
    DEF_INFO( ident, uinameres, pos, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 )

#define DEF_INFO_3( ident, uinameres, pos, helpid, flag1, flag2, flag3 ) \
    DEF_INFO( ident, uinameres, pos, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 )

#define DEF_INFO_4( ident, uinameres, pos, helpid, flag1, flag2, flag3, flag4 ) \
    DEF_INFO( ident, uinameres, pos, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 | PROP_FLAG_##flag4 )

    sal_uInt16              OPropertyInfoService::s_nCount = 0;
    OPropertyInfoImpl*      OPropertyInfoService::s_pPropertyInfos = NULL;
    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo()
    {
        if ( s_pPropertyInfos )
            return s_pPropertyInfos;

        PcrClient aResourceAccess;
        // this ensures that we have our resource file loaded

        static OPropertyInfoImpl aPropertyInfos[] =
        {
        /*
        DEF_INFO_?( propname and id,   resoure id,         pos, help id,           flags ),
        */
        DEF_INFO_3( NAME,              NAME,                 0, NAME,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( TITLE,             TITLE,                1, TITLE,             FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( LABEL,             LABEL,                2, LABEL,             FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( CONTROLLABEL,      LABELCONTROL,         3, CONTROLLABEL,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( WRITING_MODE,      WRITING_MODE,         4, WRITING_MODE,      FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( GROUP_NAME,        GROUP_NAME,           5, GROUP_NAME,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( TEXT,              TEXT,                 6, TEXT,              DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( MAXTEXTLEN,        MAXTEXTLEN,           7, MAXTEXTLEN,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( EDITMASK,          EDITMASK,             8, EDITMASK,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LITERALMASK,       LITERALMASK,          9, LITERALMASK,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( STRICTFORMAT,      STRICTFORMAT,        10, STRICTFORMAT,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ENABLED,           ENABLED,             11, ENABLED,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ENABLE_VISIBLE,    ENABLE_VISIBLE,      12, ENABLE_VISIBLE,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( READONLY,          READONLY,            13, READONLY,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( PRINTABLE,         PRINTABLE,           14, PRINTABLE,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( STEP,              STEP,                15, STEP,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( WHEEL_BEHAVIOR,    WHEEL_BEHAVIOR,      16, WHEEL_BEHAVIOR,    FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( TABSTOP,           TABSTOP,             17, TABSTOP,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( TABINDEX,          TABINDEX,            18, TABINDEX,          FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_2( BOUND_CELL,        BOUND_CELL,          19, BOUND_CELL,        FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_3( CELL_EXCHANGE_TYPE,CELL_EXCHANGE_TYPE,  20, CELL_EXCHANGE_TYPE,FORM_VISIBLE, DATA_PROPERTY, ENUM ),
        DEF_INFO_2( LIST_CELL_RANGE,   LIST_CELL_RANGE,     21, LIST_CELL_RANGE,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_3( CONTROLSOURCE,     CONTROLSOURCE,       22, CONTROLSOURCE,     FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( EMPTY_IS_NULL,     EMPTY_IS_NULL,       23, EMPTY_IS_NULL,     FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( INPUT_REQUIRED,    INPUT_REQUIRED,      24, INPUT_REQUIRED,    FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( REFVALUE,          REFVALUE,            25, REFVALUE,          FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( UNCHECKEDREFVALUE, UNCHECKEDREFVALUE,   26, UNCHECKEDREFVALUE, FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( DATASOURCE,        DATASOURCE,          27, DATASOURCE,        FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_4( COMMANDTYPE,       CURSORSOURCETYPE,    28, CURSORSOURCETYPE,  FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_3( COMMAND,           CURSORSOURCE,        29, CURSORSOURCE,      FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( ESCAPE_PROCESSING, ESCAPE_PROCESSING,   30, ESCAPE_PROCESSING, FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( FILTER,            FILTER,              31, FILTER,            FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( SORT,              SORT_CRITERIA,       32, SORT_CRITERIA,     FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_2( MASTERFIELDS,      MASTERFIELDS,        33, MASTERFIELDS,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( DETAILFIELDS,      SLAVEFIELDS,         34, SLAVEFIELDS,       FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_3( ALLOWADDITIONS,    ALLOW_ADDITIONS,     35, ALLOW_ADDITIONS,   FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( ALLOWEDITS,        ALLOW_EDITS,         36, ALLOW_EDITS,       FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( ALLOWDELETIONS,    ALLOW_DELETIONS,     37, ALLOW_DELETIONS,   FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( INSERTONLY,        DATAENTRY,           38, DATAENTRY,         FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_4( NAVIGATION,        NAVIGATION,          39, NAVIGATION,        FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_4( CYCLE,             CYCLE,               40, CYCLE,             FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_3( FILTERPROPOSAL,    FILTERPROPOSAL,      41, FILTERPROPOSAL,    FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_4( LISTSOURCETYPE,    LISTSOURCETYPE,      42, LISTSOURCETYPE,    FORM_VISIBLE, DATA_PROPERTY, ENUM, COMPOSEABLE ),
        DEF_INFO_3( LISTSOURCE,        LISTSOURCE,          43, LISTSOURCE,        FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),
        DEF_INFO_3( BOUNDCOLUMN,       BOUNDCOLUMN,         44, BOUNDCOLUMN,       FORM_VISIBLE, DATA_PROPERTY, COMPOSEABLE ),

        // <!----------------->
        // XML node binding
        DEF_INFO_2( LIST_BINDING,      LIST_BINDING,        45, LIST_BINDING,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XML_DATA_MODEL,    XML_DATA_MODEL,      46, XML_DATA_MODEL,    FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( BINDING_NAME,      BINDING_NAME,        47, BINDING_NAME,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( BIND_EXPRESSION,   BIND_EXPRESSION,     48, BIND_EXPRESSION,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_REQUIRED,      XSD_REQUIRED,        49, XSD_REQUIRED,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_RELEVANT,      XSD_RELEVANT,        50, XSD_RELEVANT,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_READONLY,      XSD_READONLY,        51, XSD_READONLY,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_CONSTRAINT,    XSD_CONSTRAINT,      52, XSD_CONSTRAINT,    FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_CALCULATION,   XSD_CALCULATION,     53, XSD_CALCULATION,   FORM_VISIBLE, DATA_PROPERTY ),

        // data type
        DEF_INFO_2( XSD_DATA_TYPE,     XSD_DATA_TYPE,       54, XSD_DATA_TYPE,     FORM_VISIBLE, DATA_PROPERTY ),
        // data types facets
        //  common
        DEF_INFO_3( XSD_WHITESPACES,   XSD_WHITESPACES,     55, XSD_WHITESPACES,   FORM_VISIBLE, DATA_PROPERTY, ENUM ),
        DEF_INFO_2( XSD_PATTERN,       XSD_PATTERN,         56, XSD_PATTERN,       FORM_VISIBLE, DATA_PROPERTY ),
        //  string
        DEF_INFO_2( XSD_LENGTH,        XSD_LENGTH,          57, XSD_LENGTH,        FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_LENGTH,    XSD_MIN_LENGTH,      58, XSD_MIN_LENGTH,    FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_LENGTH,    XSD_MAX_LENGTH,      59, XSD_MAX_LENGTH,    FORM_VISIBLE, DATA_PROPERTY ),
        //  decimal
        DEF_INFO_2( XSD_TOTAL_DIGITS,   XSD_TOTAL_DIGITS,   60, XSD_TOTAL_DIGITS,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_FRACTION_DIGITS,XSD_FRACTION_DIGITS,61,XSD_FRACTION_DIGITS,FORM_VISIBLE, DATA_PROPERTY ),
        //  int value types (year, month, day)
        DEF_INFO_2( XSD_MAX_INCLUSIVE_INT, XSD_MAX_INCLUSIVE, 62, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_INT, XSD_MAX_EXCLUSIVE, 63, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_INT, XSD_MIN_INCLUSIVE, 64, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_INT, XSD_MIN_EXCLUSIVE, 65, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        //  double value types (double, float, decimal)
        DEF_INFO_2( XSD_MAX_INCLUSIVE_DOUBLE, XSD_MAX_INCLUSIVE, 66, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_DOUBLE, XSD_MAX_EXCLUSIVE, 67, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_DOUBLE, XSD_MIN_INCLUSIVE, 68, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_DOUBLE, XSD_MIN_EXCLUSIVE, 69, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // date value type
        DEF_INFO_2( XSD_MAX_INCLUSIVE_DATE, XSD_MAX_INCLUSIVE, 70, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_DATE, XSD_MAX_EXCLUSIVE, 71, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_DATE, XSD_MIN_INCLUSIVE, 72, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_DATE, XSD_MIN_EXCLUSIVE, 73, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // time value type
        DEF_INFO_2( XSD_MAX_INCLUSIVE_TIME, XSD_MAX_INCLUSIVE, 74, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_TIME, XSD_MAX_EXCLUSIVE, 75, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_TIME, XSD_MIN_INCLUSIVE, 76, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_TIME, XSD_MIN_EXCLUSIVE, 77, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // dateTime value type
        DEF_INFO_2( XSD_MAX_INCLUSIVE_DATE_TIME, XSD_MAX_INCLUSIVE, 78, XSD_MAX_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MAX_EXCLUSIVE_DATE_TIME, XSD_MAX_EXCLUSIVE, 79, XSD_MAX_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_INCLUSIVE_DATE_TIME, XSD_MIN_INCLUSIVE, 80, XSD_MIN_INCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( XSD_MIN_EXCLUSIVE_DATE_TIME, XSD_MIN_EXCLUSIVE, 81, XSD_MIN_EXCLUSIVE, FORM_VISIBLE, DATA_PROPERTY ),
        // <!----------------->

        DEF_INFO_2( HIDDEN_VALUE,      VALUE,               82, HIDDEN_VALUE,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( VALUE,             VALUE,               83, VALUE,             DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VALUEMIN,          VALUEMIN,            84, VALUEMIN,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VALUEMAX,          VALUEMAX,            85, VALUEMAX,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VALUESTEP,         VALUESTEP,           86, VALUESTEP,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_VALUE,     DEFAULTVALUE,        87, DEFAULT_LONG_VALUE,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DECIMAL_ACCURACY,  DECIMAL_ACCURACY,    88, DECIMAL_ACCURACY,  FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SHOWTHOUSANDSEP,   SHOWTHOUSANDSEP,     89, SHOWTHOUSANDSEP,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( CURRENCYSYMBOL,    CURRENCYSYMBOL,      90, CURRENCYSYMBOL,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( CURRSYM_POSITION,  CURRSYM_POSITION,    91, CURRSYM_POSITION,  FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( DATE,              DATE,                92, DATE,              DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DATEMIN,           DATEMIN,             93, DATEMIN,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DATEMAX,           DATEMAX,             94, DATEMAX,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( DATEFORMAT,        DATEFORMAT,          95, DATEFORMAT,        FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_DATE,      DEFAULTDATE,         96, DEFAULT_DATE,      FORM_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( TIME,              TIME,                97, TIME,              DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TIMEMIN,           TIMEMIN,             98, TIMEMIN,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TIMEMAX,           TIMEMAX,             99, TIMEMAX,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( TIMEFORMAT,        TIMEFORMAT,         100, TIMEFORMAT,        FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_TIME,      DEFAULTTIME,        101, DEFAULT_TIME,      FORM_VISIBLE, COMPOSEABLE ),

        DEF_INFO_1( EFFECTIVE_VALUE,   VALUE,              102, VALUE,             DIALOG_VISIBLE ),
        DEF_INFO_3( EFFECTIVE_MIN,     VALUEMIN,           103, EFFECTIVEMIN,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( EFFECTIVE_MAX,     VALUEMAX,           104, EFFECTIVEMAX,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( EFFECTIVE_DEFAULT, DEFAULTVALUE,       105, EFFECTIVEDEFAULT,  FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( FORMATKEY,         FORMATKEY,          106, FORMATKEY,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( PROGRESSVALUE,     PROGRESSVALUE,      107, PROGRESSVALUE,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( PROGRESSVALUE_MIN, PROGRESSVALUE_MIN,  108, PROGRESSVALUE_MIN, FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( PROGRESSVALUE_MAX, PROGRESSVALUE_MAX,  109, PROGRESSVALUE_MAX, FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( SCROLLVALUE,       SCROLLVALUE,        110, SCROLLVALUE,       DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SCROLLVALUE_MIN,   SCROLLVALUE_MIN,    111, SCROLLVALUE_MIN,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SCROLLVALUE_MAX,   SCROLLVALUE_MAX,    112, SCROLLVALUE_MAX,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SCROLL_WIDTH,      SCROLL_WIDTH,       113, SCROLL_WIDTH,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SCROLL_HEIGHT,     SCROLL_HEIGHT,      114, SCROLL_HEIGHT,     DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SCROLL_TOP,        SCROLL_TOP,         115, SCROLL_TOP,        DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SCROLL_LEFT,       SCROLL_LEFT,        116, SCROLL_LEFT,       DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_SCROLLVALUE,DEFAULT_SCROLLVALUE,117,DEFAULT_SCROLLVALUE,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LINEINCREMENT,     LINEINCREMENT,      118, LINEINCREMENT,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( BLOCKINCREMENT,    BLOCKINCREMENT,     119, BLOCKINCREMENT,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_2( SPINVALUE,        VALUE,               120, SPINVALUE,         DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SPINVALUE_MIN,    VALUEMIN,            121, SPINVALUE_MIN,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SPINVALUE_MAX,    VALUEMAX,            122, SPINVALUE_MAX,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_SPINVALUE,DEFAULTVALUE,        123, DEFAULT_SPINVALUE, FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SPININCREMENT,    VALUESTEP,           124, SPININCREMENT,     FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( SPIN,              SPIN,               125, SPIN,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( REPEAT,            REPEAT,             126, REPEAT,            FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( REPEAT_DELAY,      REPEAT_DELAY,       127, REPEAT_DELAY,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VISIBLESIZE,       VISIBLESIZE,        128, VISIBLESIZE,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( ORIENTATION,       ORIENTATION,        129, ORIENTATION,       FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( FOCUSONCLICK,      FOCUSONCLICK,       130, FOCUSONCLICK,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TOGGLE,            TOGGLE,             131, TOGGLE,            FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( DEFAULT_STATE,     DEFAULT_STATE,      132, DEFAULT_STATE,     FORM_VISIBLE, ENUM, COMPOSEABLE ),

        DEF_INFO_3( TEXT_ANCHOR_TYPE,  ANCHOR_TYPE,        133, ANCHOR_TYPE,       FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( SHEET_ANCHOR_TYPE, ANCHOR_TYPE,        134, ANCHOR_TYPE,       FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( POSITIONX,         POSITIONX,          135, POSITIONX,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( POSITIONY,         POSITIONY,          136, POSITIONY,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( WIDTH,             WIDTH,              137, WIDTH,             FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HEIGHT,            HEIGHT,             138, HEIGHT,            FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),

        DEF_INFO_1( LISTINDEX,         LISTINDEX,          139, LISTINDEX,         FORM_VISIBLE ),
        DEF_INFO_3( STRINGITEMLIST,    STRINGITEMLIST,     140, STRINGITEMLIST,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DEFAULT_TEXT,      DEFAULTTEXT,        141, DEFAULTVALUE,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( FONT,              FONT,               142, FONT,              FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( VISUALEFFECT,      VISUALEFFECT,       143, VISUALEFFECT,      FORM_VISIBLE, DIALOG_VISIBLE, ENUM_ONE, COMPOSEABLE ),
        DEF_INFO_4( ALIGN,             ALIGN,              144, ALIGN,             FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( VERTICAL_ALIGN,    VERTICAL_ALIGN,     145, VERTICAL_ALIGN,    FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( ROWHEIGHT,         ROWHEIGHT,          146, ROWHEIGHT,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( BACKGROUNDCOLOR,   BACKGROUNDCOLOR,    147, BACKGROUNDCOLOR,   FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SYMBOLCOLOR,       SYMBOLCOLOR,        148, SYMBOLCOLOR,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( FILLCOLOR,         FILLCOLOR,          149, FILLCOLOR,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LINECOLOR,         LINECOLOR,          150, LINECOLOR,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( BORDER,            BORDER,             151, BORDER,            FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( BORDERCOLOR,       BORDERCOLOR,        152, BORDERCOLOR,       FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ICONSIZE,          ICONSIZE,           153, ICONSIZE,          FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( SHOW_POSITION,     SHOW_POSITION,      154, SHOW_POSITION,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOW_NAVIGATION,   SHOW_NAVIGATION,    155, SHOW_NAVIGATION,   FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOW_RECORDACTIONS,SHOW_RECORDACTIONS, 156, SHOW_RECORDACTIONS,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOW_FILTERSORT,   SHOW_FILTERSORT,    157, SHOW_FILTERSORT,   FORM_VISIBLE, COMPOSEABLE ),

        DEF_INFO_3( DROPDOWN,          DROPDOWN,           158, DROPDOWN,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( LINECOUNT,         LINECOUNT,          159, LINECOUNT,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( AUTOCOMPLETE,      AUTOCOMPLETE,       160, AUTOCOMPLETE,      FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( MULTILINE,         MULTILINE,          161, MULTILINE,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( WORDBREAK,         WORDBREAK,          162, WORDBREAK,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TEXTTYPE,          TEXTTYPE,           163, TEXTTYPE,          FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( LINEEND_FORMAT,    LINEEND_FORMAT,     164, LINEEND_FORMAT,    FORM_VISIBLE, ENUM_ONE, COMPOSEABLE ),
        DEF_INFO_3( MULTISELECTION,    MULTISELECTION,     165, MULTISELECTION,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( SHOW_SCROLLBARS,   SHOW_SCROLLBARS,    166, SHOW_SCROLLBARS,   FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( HSCROLL,           HSCROLL,            167, HSCROLL,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( VSCROLL,           VSCROLL,            168, VSCROLL,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( BUTTONTYPE,        BUTTONTYPE,         169, BUTTONTYPE,        FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( XFORMS_BUTTONTYPE, BUTTONTYPE,         170, BUTTONTYPE,        FORM_VISIBLE, ENUM ),
        DEF_INFO_1( SUBMISSION_ID,     SUBMISSION_ID,      171, SUBMISSION_ID,     FORM_VISIBLE ),
        DEF_INFO_2( PUSHBUTTONTYPE,    PUSHBUTTONTYPE,     172, PUSHBUTTONTYPE,    DIALOG_VISIBLE, ENUM ),
        DEF_INFO_2( TARGET_URL,        TARGET_URL,         173, TARGET_URL,        FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_1( TARGET_FRAME,      TARGET_FRAME,       174, TARGET_FRAME,      FORM_VISIBLE ),
        DEF_INFO_2( SUBMIT_ACTION,     SUBMIT_ACTION,      175, SUBMIT_ACTION,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SUBMIT_TARGET,     SUBMIT_TARGET,      176, SUBMIT_TARGET,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SUBMIT_ENCODING,   SUBMIT_ENCODING,    177, SUBMIT_ENCODING,   FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( SUBMIT_METHOD,     SUBMIT_METHOD,      178, SUBMIT_METHOD,     FORM_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( STATE,             STATE,              179, STATE,             DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( DEFAULTBUTTON,     DEFAULT_BUTTON,     180, DEFAULT_BUTTON,    FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( IMAGE_URL,         IMAGE_URL,          181, IMAGE_URL,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( IMAGEPOSITION,     IMAGEPOSITION,      182, IMAGEPOSITION,     FORM_VISIBLE, DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_3( SCALEIMAGE,        SCALEIMAGE,         183, SCALEIMAGE,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_4( SCALE_MODE,        SCALEIMAGE,         184, SCALEIMAGE,        FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE, ENUM ),
        DEF_INFO_2( DEFAULT_SELECT_SEQ,DEFAULT_SELECT_SEQ, 185, DEFAULT_SELECT_SEQ,FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SELECTEDITEMS,     SELECTEDITEMS,      186, SELECTEDITEMS,     DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( ECHO_CHAR,         ECHO_CHAR,          187, ECHO_CHAR,         FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HIDEINACTIVESELECTION, HIDEINACTIVESELECTION, 188, HIDEINACTIVESELECTION, FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TRISTATE,          TRISTATE,           189, TRISTATE,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( HASNAVIGATION,     NAVIGATION,         190, NAVIGATIONBAR,     FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( RECORDMARKER,      RECORDMARKER,       191, RECORDMARKER,      FORM_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( TAG,               TAG,                192, TAG,               FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HELPTEXT,          HELPTEXT,           193, HELPTEXT,          FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( HELPURL,           HELPURL,            194, HELPURL,           FORM_VISIBLE, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_3( SELECTION_TYPE,    SELECTION_TYPE,     195, SELECTION_TYPE,                  DIALOG_VISIBLE, ENUM, COMPOSEABLE ),
        DEF_INFO_2( ROOT_DISPLAYED,    ROOT_DISPLAYED,     196, ROOT_DISPLAYED,                  DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOWS_HANDLES,     SHOWS_HANDLES,      197, SHOWS_HANDLES,                   DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( SHOWS_ROOT_HANDLES, SHOWS_ROOT_HANDLES, 198, SHOWS_ROOT_HANDLES,             DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( EDITABLE,          EDITABLE,           199, EDITABLE,                        DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( INVOKES_STOP_NOT_EDITING, INVOKES_STOP_NOT_EDITING, 200, INVOKES_STOP_NOT_EDITING, DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( DECORATION,        DECORATION,         201, DECORATION,                      DIALOG_VISIBLE, COMPOSEABLE ),
        DEF_INFO_2( NOLABEL,           NOLABEL,            202, NOLABEL,                         DIALOG_VISIBLE, COMPOSEABLE )
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
    OString OPropertyInfoService::getPropertyHelpId(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->sHelpId : OString();
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
    ::std::vector< OUString > OPropertyInfoService::getPropertyEnumRepresentations(sal_Int32 _nId) const
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

        ::std::vector< OUString > aReturn;

        if ( nStringItemsResId )
        {
            PcrRes aResId( nStringItemsResId );
            ::svt::OLocalResourceAccess aEnumStrings( aResId, RSC_RESOURCE );

            sal_Int16 i = 1;
            PcrRes aLocalId( i );
            while ( aEnumStrings.IsAvailableRes( aLocalId.SetRT( RSC_STRING ) ) )
            {
                aReturn.push_back( aLocalId.toString() );
                aLocalId = PcrRes( ++i );
            }
        }

        return aReturn;
    }

    //------------------------------------------------------------------------
    sal_Bool OPropertyInfoService::isComposeable( const OUString& _rPropertyName ) const
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
    ::std::vector< OUString > SAL_CALL DefaultEnumRepresentation::getDescriptions() const
    {
        return m_rMetaData.getPropertyEnumRepresentations( m_nPropertyId );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DefaultEnumRepresentation::getValueFromDescription( const OUString& _rDescription, Any& _out_rValue ) const
    {
        sal_uInt32  nPropertyUIFlags = m_rMetaData.getPropertyUIFlags( m_nPropertyId );
        ::std::vector< OUString > aEnumStrings = m_rMetaData.getPropertyEnumRepresentations( m_nPropertyId );
        ::std::vector< OUString >::const_iterator pos = ::std::find( aEnumStrings.begin(), aEnumStrings.end(), _rDescription );
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
    OUString SAL_CALL DefaultEnumRepresentation::getDescriptionForValue( const Any& _rEnumValue ) const
    {
        OUString sReturn;
        sal_Int32 nIntValue = -1;
        OSL_VERIFY( ::cppu::enum2int( nIntValue, _rEnumValue ) );

        sal_uInt32 nUIFlags = m_rMetaData.getPropertyUIFlags( m_nPropertyId );
        if ( ( nUIFlags & PROP_FLAG_ENUM_ONE ) == PROP_FLAG_ENUM_ONE )
            // enum value starting with 1
            --nIntValue;

        ::std::vector< OUString > aEnumStrings = m_rMetaData.getPropertyEnumRepresentations( m_nPropertyId );
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
        return osl_atomic_increment( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL DefaultEnumRepresentation::release()
    {
        if ( 0 == osl_atomic_decrement( &m_refCount ) )
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

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

#include "frm_strings.hxx"
#include "property.hxx"

#include "property.hrc"
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>

//... namespace frm .......................................................
namespace frm
{
//.........................................................................

//==================================================================
//= PropertyInfoService
//==================================================================
PropertyInfoService::PropertyMap PropertyInfoService::s_AllKnownProperties;
//------------------------------------------------------------------
sal_Int32 PropertyInfoService::getPropertyId(const OUString& _rName)
{
    initialize();

    PropertyAssignment aCompareName(_rName, -1);

    ::std::pair<PropertyMapIterator,PropertyMapIterator> aPair = ::std::equal_range(
        s_AllKnownProperties.begin(),
        s_AllKnownProperties.end(),
        aCompareName,
        PropertyAssignmentNameCompareLess());

    sal_Int32 nHandle = -1;
    if (aPair.first != aPair.second)
    {   // we found something _and_ we have an identity
        nHandle = aPair.first->nHandle;
    }

    return nHandle;
}

//------------------------------------------------------------------
sal_Int32 ConcreteInfoService::getPreferedPropertyId(const OUString& _rName)
{
    return PropertyInfoService::getPropertyId(_rName);
}

//------------------------------------------------------------------
#define ADD_PROP_ASSIGNMENT(varname) \
    s_AllKnownProperties.push_back(PropertyAssignment(PROPERTY_##varname, PROPERTY_ID_##varname))
//..................................................................
void PropertyInfoService::initialize()
{
    if (!s_AllKnownProperties.empty())
        return;

    s_AllKnownProperties.reserve(220);

    ADD_PROP_ASSIGNMENT(NAME);
    ADD_PROP_ASSIGNMENT(TAG);
    ADD_PROP_ASSIGNMENT(TABINDEX);
    ADD_PROP_ASSIGNMENT(CLASSID);
    ADD_PROP_ASSIGNMENT(ALIGN);
    ADD_PROP_ASSIGNMENT(FETCHSIZE);
    ADD_PROP_ASSIGNMENT(VALUE);
    ADD_PROP_ASSIGNMENT(VALUEMIN);
    ADD_PROP_ASSIGNMENT(VALUEMAX);
    ADD_PROP_ASSIGNMENT(VALUESTEP);
    ADD_PROP_ASSIGNMENT(TEXT);
    ADD_PROP_ASSIGNMENT(LABEL);
    ADD_PROP_ASSIGNMENT(NAVIGATION);
    ADD_PROP_ASSIGNMENT(CYCLE);
    ADD_PROP_ASSIGNMENT(CONTROLSOURCE);
    ADD_PROP_ASSIGNMENT(ENABLED);
    ADD_PROP_ASSIGNMENT(ENABLEVISIBLE);
    ADD_PROP_ASSIGNMENT(SPIN);
    ADD_PROP_ASSIGNMENT(READONLY);
    ADD_PROP_ASSIGNMENT(FILTER);
    ADD_PROP_ASSIGNMENT(WIDTH);
    ADD_PROP_ASSIGNMENT(SEARCHABLE);
    ADD_PROP_ASSIGNMENT(MULTILINE);
    ADD_PROP_ASSIGNMENT(TARGET_URL);
    ADD_PROP_ASSIGNMENT(DEFAULTCONTROL);
    ADD_PROP_ASSIGNMENT(MAXTEXTLEN);
    ADD_PROP_ASSIGNMENT(SIZE);
    ADD_PROP_ASSIGNMENT(DATE);
    ADD_PROP_ASSIGNMENT(TIME);
    ADD_PROP_ASSIGNMENT(STATE);
    ADD_PROP_ASSIGNMENT(TRISTATE);
    ADD_PROP_ASSIGNMENT(HIDDEN_VALUE);
    ADD_PROP_ASSIGNMENT(TARGET_FRAME);
    ADD_PROP_ASSIGNMENT(BUTTONTYPE);
    ADD_PROP_ASSIGNMENT(STRINGITEMLIST);
    ADD_PROP_ASSIGNMENT(DEFAULT_TEXT);
    ADD_PROP_ASSIGNMENT(DEFAULT_STATE);
    ADD_PROP_ASSIGNMENT(DEFAULT_DATE);
    ADD_PROP_ASSIGNMENT(DEFAULT_TIME);
    ADD_PROP_ASSIGNMENT(DEFAULT_VALUE);
    ADD_PROP_ASSIGNMENT(FORMATKEY);
    ADD_PROP_ASSIGNMENT(FORMATSSUPPLIER);
    ADD_PROP_ASSIGNMENT(SUBMIT_ACTION);
    ADD_PROP_ASSIGNMENT(SUBMIT_TARGET);
    ADD_PROP_ASSIGNMENT(SUBMIT_METHOD);
    ADD_PROP_ASSIGNMENT(SUBMIT_ENCODING);
    ADD_PROP_ASSIGNMENT(IMAGE_URL);
    ADD_PROP_ASSIGNMENT(GRAPHIC);
    ADD_PROP_ASSIGNMENT(EMPTY_IS_NULL);
    ADD_PROP_ASSIGNMENT(LISTSOURCETYPE);
    ADD_PROP_ASSIGNMENT(LISTSOURCE);
    ADD_PROP_ASSIGNMENT(SELECT_SEQ);
    ADD_PROP_ASSIGNMENT(VALUE_SEQ);
    ADD_PROP_ASSIGNMENT(DEFAULT_SELECT_SEQ);
    ADD_PROP_ASSIGNMENT(MULTISELECTION);
    ADD_PROP_ASSIGNMENT(DECIMAL_ACCURACY);
    ADD_PROP_ASSIGNMENT(EDITMASK);
    ADD_PROP_ASSIGNMENT(ISREADONLY);
    ADD_PROP_ASSIGNMENT(FIELDTYPE);
    ADD_PROP_ASSIGNMENT(DECIMALS);
    ADD_PROP_ASSIGNMENT(REFVALUE);
    ADD_PROP_ASSIGNMENT(STRICTFORMAT);
    ADD_PROP_ASSIGNMENT(DATASOURCE);
    ADD_PROP_ASSIGNMENT(ALLOWADDITIONS);
    ADD_PROP_ASSIGNMENT(ALLOWEDITS);
    ADD_PROP_ASSIGNMENT(ALLOWDELETIONS);
    ADD_PROP_ASSIGNMENT(MASTERFIELDS);
    ADD_PROP_ASSIGNMENT(ISPASSTHROUGH);
    ADD_PROP_ASSIGNMENT(QUERY);
    ADD_PROP_ASSIGNMENT(LITERALMASK);
    ADD_PROP_ASSIGNMENT(SHOWTHOUSANDSEP);
    ADD_PROP_ASSIGNMENT(CURRENCYSYMBOL);
    ADD_PROP_ASSIGNMENT(DATEFORMAT);
    ADD_PROP_ASSIGNMENT(DATEMIN);
    ADD_PROP_ASSIGNMENT(DATEMAX);
    ADD_PROP_ASSIGNMENT(DATE_SHOW_CENTURY);
    ADD_PROP_ASSIGNMENT(TIMEFORMAT);
    ADD_PROP_ASSIGNMENT(TIMEMIN);
    ADD_PROP_ASSIGNMENT(TIMEMAX);
    ADD_PROP_ASSIGNMENT(LINECOUNT);
    ADD_PROP_ASSIGNMENT(BOUNDCOLUMN);
    ADD_PROP_ASSIGNMENT(HASNAVIGATION);
    ADD_PROP_ASSIGNMENT(FONT);
    ADD_PROP_ASSIGNMENT(BACKGROUNDCOLOR);
    ADD_PROP_ASSIGNMENT(FILLCOLOR);
    ADD_PROP_ASSIGNMENT(TEXTCOLOR);
    ADD_PROP_ASSIGNMENT(LINECOLOR);
    ADD_PROP_ASSIGNMENT(BORDER);
    ADD_PROP_ASSIGNMENT(DROPDOWN);
    ADD_PROP_ASSIGNMENT(HSCROLL);
    ADD_PROP_ASSIGNMENT(VSCROLL);
    ADD_PROP_ASSIGNMENT(TABSTOP);
    ADD_PROP_ASSIGNMENT(AUTOCOMPLETE);
    ADD_PROP_ASSIGNMENT(HARDLINEBREAKS);
    ADD_PROP_ASSIGNMENT(PRINTABLE);
    ADD_PROP_ASSIGNMENT(ECHO_CHAR);
    ADD_PROP_ASSIGNMENT(ROWHEIGHT);
    ADD_PROP_ASSIGNMENT(HELPTEXT);
    ADD_PROP_ASSIGNMENT(FONT_NAME);
    ADD_PROP_ASSIGNMENT(FONT_STYLENAME);
    ADD_PROP_ASSIGNMENT(FONT_FAMILY);
    ADD_PROP_ASSIGNMENT(FONT_CHARSET);
    ADD_PROP_ASSIGNMENT(FONT_HEIGHT);
    ADD_PROP_ASSIGNMENT(FONT_WEIGHT);
    ADD_PROP_ASSIGNMENT(FONT_SLANT);
    ADD_PROP_ASSIGNMENT(FONT_UNDERLINE);
    ADD_PROP_ASSIGNMENT(FONT_WORDLINEMODE);
    ADD_PROP_ASSIGNMENT(FONT_STRIKEOUT);
    ADD_PROP_ASSIGNMENT(TEXTLINECOLOR);
    ADD_PROP_ASSIGNMENT(FONTEMPHASISMARK);
    ADD_PROP_ASSIGNMENT(FONTRELIEF);
    ADD_PROP_ASSIGNMENT(HELPURL);
    ADD_PROP_ASSIGNMENT(RECORDMARKER);
    ADD_PROP_ASSIGNMENT(BOUNDFIELD);
    ADD_PROP_ASSIGNMENT(INPUT_REQUIRED);
    ADD_PROP_ASSIGNMENT(TREATASNUMERIC);
    ADD_PROP_ASSIGNMENT(EFFECTIVE_VALUE);
    ADD_PROP_ASSIGNMENT(EFFECTIVE_DEFAULT);
    ADD_PROP_ASSIGNMENT(EFFECTIVE_MIN);
    ADD_PROP_ASSIGNMENT(EFFECTIVE_MAX);
    ADD_PROP_ASSIGNMENT(HIDDEN);
    ADD_PROP_ASSIGNMENT(FILTERPROPOSAL);
    ADD_PROP_ASSIGNMENT(FIELDSOURCE);
    ADD_PROP_ASSIGNMENT(TABLENAME);
    ADD_PROP_ASSIGNMENT(CONTROLLABEL);
    ADD_PROP_ASSIGNMENT(CURRSYM_POSITION);
    ADD_PROP_ASSIGNMENT(CURSORCOLOR);
    ADD_PROP_ASSIGNMENT(ALWAYSSHOWCURSOR);
    ADD_PROP_ASSIGNMENT(DISPLAYSYNCHRON);
    ADD_PROP_ASSIGNMENT(ISMODIFIED);
    ADD_PROP_ASSIGNMENT(ISNEW);
    ADD_PROP_ASSIGNMENT(PRIVILEGES);
    ADD_PROP_ASSIGNMENT(DETAILFIELDS);
    ADD_PROP_ASSIGNMENT(COMMAND);
    ADD_PROP_ASSIGNMENT(COMMANDTYPE);
    ADD_PROP_ASSIGNMENT(RESULTSET_CONCURRENCY);
    ADD_PROP_ASSIGNMENT(INSERTONLY);
    ADD_PROP_ASSIGNMENT(RESULTSET_TYPE);
    ADD_PROP_ASSIGNMENT(ESCAPE_PROCESSING);
    ADD_PROP_ASSIGNMENT(APPLYFILTER);
    ADD_PROP_ASSIGNMENT(ISNULLABLE);
    ADD_PROP_ASSIGNMENT(ACTIVECOMMAND);
    ADD_PROP_ASSIGNMENT(ISCURRENCY);
    ADD_PROP_ASSIGNMENT(URL);
    ADD_PROP_ASSIGNMENT(TITLE);
    ADD_PROP_ASSIGNMENT(ACTIVE_CONNECTION);
    ADD_PROP_ASSIGNMENT(SCALE);
    ADD_PROP_ASSIGNMENT(SORT);
    ADD_PROP_ASSIGNMENT(PERSISTENCE_MAXTEXTLENGTH);
    ADD_PROP_ASSIGNMENT(SCROLL_VALUE);
    ADD_PROP_ASSIGNMENT(SPIN_VALUE);
    ADD_PROP_ASSIGNMENT(DEFAULT_SCROLL_VALUE);
    ADD_PROP_ASSIGNMENT(DEFAULT_SPIN_VALUE);
    ADD_PROP_ASSIGNMENT( WRITING_MODE );
    ADD_PROP_ASSIGNMENT( CONTEXT_WRITING_MODE );
    ADD_PROP_ASSIGNMENT( GENERATEVBAEVENTS );

    // now sort the array by name

    std::sort(
        s_AllKnownProperties.begin(),
        s_AllKnownProperties.end(),
        PropertyAssignmentNameCompareLess()
    );
}

//.........................................................................
}
//... namespace frm .......................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

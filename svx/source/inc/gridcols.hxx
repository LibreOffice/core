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
#ifndef _SVX_GRIDCOLS_HXX
#define _SVX_GRIDCOLS_HXX

#include <sal/types.h>

namespace rtl { class OUString; }

#define FM_COL_TEXTFIELD      rtl::OUString( "TextField" )
#define FM_COL_COMBOBOX       rtl::OUString( "ComboBox" )
#define FM_COL_CHECKBOX       rtl::OUString( "CheckBox" )
#define FM_COL_TIMEFIELD      rtl::OUString( "TimeField" )
#define FM_COL_DATEFIELD      rtl::OUString( "DateField" )
#define FM_COL_NUMERICFIELD   rtl::OUString( "NumericField" )
#define FM_COL_CURRENCYFIELD  rtl::OUString( "CurrencyField" )
#define FM_COL_PATTERNFIELD   rtl::OUString( "PatternField" )
#define FM_COL_LISTBOX        rtl::OUString( "ListBox" )
#define FM_COL_FORMATTEDFIELD rtl::OUString( "FormattedField" )

// column type ids
#define TYPE_CHECKBOX       0
#define TYPE_COMBOBOX       1
#define TYPE_CURRENCYFIELD  2
#define TYPE_DATEFIELD      3
#define TYPE_FORMATTEDFIELD 4
#define TYPE_LISTBOX        5
#define TYPE_NUMERICFIELD   6
#define TYPE_PATTERNFIELD   7
#define TYPE_TEXTFIELD      8
#define TYPE_TIMEFIELD      9

//------------------------------------------------------------------------------
sal_Int32 getColumnTypeByModelName(const ::rtl::OUString& aModelName);


#endif // _SVX_GRIDCOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

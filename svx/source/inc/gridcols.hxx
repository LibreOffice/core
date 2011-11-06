/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_GRIDCOLS_HXX
#define _SVX_GRIDCOLS_HXX

#include <sal/types.h>

namespace rtl { class OUString; }

#define FM_COL_TEXTFIELD      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TextField" ) )
#define FM_COL_COMBOBOX       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ComboBox" ) )
#define FM_COL_CHECKBOX       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CheckBox" ) )
#define FM_COL_TIMEFIELD      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TimeField" ) )
#define FM_COL_DATEFIELD      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateField" ) )
#define FM_COL_NUMERICFIELD   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NumericField" ) )
#define FM_COL_CURRENCYFIELD  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CurrencyField" ) )
#define FM_COL_PATTERNFIELD   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PatternField" ) )
#define FM_COL_LISTBOX        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListBox" ) )
#define FM_COL_FORMATTEDFIELD rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FormattedField" ) )

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


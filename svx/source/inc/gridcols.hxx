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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

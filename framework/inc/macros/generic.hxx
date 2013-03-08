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

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#define __FRAMEWORK_MACROS_GENERIC_HXX_

#include <rtl/ustring.hxx>
#include <rtl/textenc.h>

//*****************************************************************************************************************
//  generic macros
//*****************************************************************************************************************

/*_________________________________________________________________________________________________________________
    DECLARE_ASCII( SASCIIVALUE )

    Use it to declare a constant ascii value at compile time in code.
    zB. OUSting sTest = DECLARE_ASCII( "Test" )
_________________________________________________________________________________________________________________*/

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    OUString( SASCIIVALUE )

/*_________________________________________________________________________________________________________________
    U2B( SUNICODEVALUE )

    Use it to convert unicode strings to ascii values and reverse ...
    We use UTF8 as default textencoding.
_________________________________________________________________________________________________________________*/

#define U2B( SUNICODEVALUE )                                                                                    \
    OUStringToOString( SUNICODEVALUE, RTL_TEXTENCODING_UTF8 )

#endif  //  #ifndef __FRAMEWORK_MACROS_GENERIC_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

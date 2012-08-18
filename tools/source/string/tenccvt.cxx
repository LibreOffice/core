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

#include <rtl/tencinfo.h>
#include <tools/tenccvt.hxx>

rtl_TextEncoding GetExtendedCompatibilityTextEncoding( rtl_TextEncoding eEncoding )
{
    // Latin1
    if ( eEncoding == RTL_TEXTENCODING_ISO_8859_1 )
        return RTL_TEXTENCODING_MS_1252;
    // Turkey
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_9 )
        return RTL_TEXTENCODING_MS_1254;
    else
        return eEncoding;
}

rtl_TextEncoding GetExtendedTextEncoding( rtl_TextEncoding eEncoding )
{
    // Cyr
    if ( eEncoding == RTL_TEXTENCODING_ISO_8859_5 )
        return RTL_TEXTENCODING_MS_1251;
    // Greek (2 Characters different: A1 (0x2018/0x0385), A2 (0x2019/0x0386) -
    // so it is handled in this function and not in GetExtendedCompatibilityTextEncoding)
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_7 )
        return RTL_TEXTENCODING_MS_1253;
    // East-Europe - Latin2
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_2 )
        return RTL_TEXTENCODING_MS_1250;
    // Latin-15 - Latin 1 mit Euro-Sign
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_15 )
        return RTL_TEXTENCODING_MS_1252;
    else
        return GetExtendedCompatibilityTextEncoding( eEncoding );
}

rtl_TextEncoding GetOneByteTextEncoding( rtl_TextEncoding eEncoding )
{
    rtl_TextEncodingInfo aTextEncInfo;
    aTextEncInfo.StructSize = sizeof( aTextEncInfo );
    if ( rtl_getTextEncodingInfo( eEncoding, &aTextEncInfo ) )
    {
        if ( aTextEncInfo.MaximumCharSize > 1 )
            return RTL_TEXTENCODING_MS_1252;
        else
            return eEncoding;
    }
    else
        return RTL_TEXTENCODING_MS_1252;
}

rtl_TextEncoding GetSOLoadTextEncoding( rtl_TextEncoding eEncoding )
{
    return GetExtendedCompatibilityTextEncoding( GetOneByteTextEncoding( eEncoding ) );
}

rtl_TextEncoding GetSOStoreTextEncoding( rtl_TextEncoding eEncoding )
{
    return GetExtendedTextEncoding( GetOneByteTextEncoding( eEncoding ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

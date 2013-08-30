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


#include <svx/dialogs.hrc> // -> RID_SVXSTR_TEXTENCODING_TABLE
#include <svx/dialmgr.hxx>
#include "svx/txenctab.hxx"
#include <tools/shl.hxx>

//------------------------------------------------------------------------

SvxTextEncodingTable::SvxTextEncodingTable()
    :
    ResStringArray( SVX_RES( RID_SVXSTR_TEXTENCODING_TABLE ) )
{
}

//------------------------------------------------------------------------

SvxTextEncodingTable::~SvxTextEncodingTable()
{
}

//------------------------------------------------------------------------

const OUString SvxTextEncodingTable::GetTextString( const rtl_TextEncoding nEnc ) const
{
    sal_uInt32 nPos = FindIndex( (long)nEnc );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );

    return OUString();
}

//------------------------------------------------------------------------

rtl_TextEncoding SvxTextEncodingTable::GetTextEncoding( const OUString& rStr ) const
{
    sal_uInt32 nCount = Count();

    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        if (ResStringArray::GetString( i ).equals(rStr))
            return rtl_TextEncoding( GetValue( i ) );
    }
    return RTL_TEXTENCODING_DONTKNOW;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

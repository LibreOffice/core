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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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

const String& SvxTextEncodingTable::GetTextString( const rtl_TextEncoding nEnc ) const
{
    sal_uInt32 nPos = FindIndex( (long)nEnc );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );

    static String aEmptyString;
    return aEmptyString;
}

//------------------------------------------------------------------------

rtl_TextEncoding SvxTextEncodingTable::GetTextEncoding( const String& rStr ) const
{
    sal_uInt32 nCount = Count();

    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        if ( rStr == ResStringArray::GetString( i ) )
            return rtl_TextEncoding( GetValue( i ) );
    }
    return RTL_TEXTENCODING_DONTKNOW;
}


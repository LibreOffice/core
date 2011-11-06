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
#include <tools/shl.hxx>

// include ---------------------------------------------------------------

#include <svx/dialogs.hrc> // -> RID_SVXSTR_LANGUAGE_TABLE
#include <svx/dialmgr.hxx>
#include <svx/strarray.hxx>

//------------------------------------------------------------------------

SvxStringArray::SvxStringArray( sal_uInt32 nResId ) :

    ResStringArray( SVX_RES( nResId ) )

{
}

//------------------------------------------------------------------------

SvxStringArray::SvxStringArray( const ResId& rResId ) :
    ResStringArray( rResId )
{
}


//------------------------------------------------------------------------

SvxStringArray::~SvxStringArray()
{
}

//------------------------------------------------------------------------

const String& SvxStringArray::GetStringByPos( sal_uInt32 nPos ) const
{
    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );
    else
        return String::EmptyString();
}

//------------------------------------------------------------------------

const String& SvxStringArray::GetStringByType( long nType ) const
{
    sal_uInt32 nPos = FindIndex( nType );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );
    else
        return String::EmptyString();
}

//------------------------------------------------------------------------

long SvxStringArray::GetValueByStr( const String& rStr ) const
{
    long nType = 0;
    sal_uInt32 nCount = Count();

    for ( sal_uInt32 i = 0; i < nCount; ++i )
        if ( rStr == ResStringArray::GetString( i ) )
        {
            nType = GetValue( i );
            break;
        }
    return nType;
}



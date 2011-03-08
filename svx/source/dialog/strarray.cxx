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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

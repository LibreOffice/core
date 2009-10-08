/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langtab.cxx,v $
 * $Revision: 1.3.140.1 $
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
#include "precompiled_svtools.hxx"

// include ---------------------------------------------------------------

#include <tools/shl.hxx>
#include <tools/debug.hxx>

#include <i18npool/lang.h>
#include <i18npool/mslangid.hxx>

#include <svtools/svtools.hrc>
#include <svtools/svtdata.hxx>
#include <svtools/langtab.hxx>

//------------------------------------------------------------------------

SvtLanguageTable::SvtLanguageTable() :
    ResStringArray( SvtResId( STR_ARR_SVT_LANGUAGE_TABLE ) )
{
}

//------------------------------------------------------------------------

SvtLanguageTable::~SvtLanguageTable()
{
}

//------------------------------------------------------------------------

const String& SvtLanguageTable::GetString( const LanguageType eType ) const
{
    LanguageType eLang = MsLangId::getReplacementForObsoleteLanguage( eType);
    sal_uInt32 nPos = FindIndex( eLang );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );
    else
    {
        // If we knew what a simple "en" should alias to (en_US?) we could
        // generally raise an error.
        OSL_ENSURE(
            eLang == LANGUAGE_ENGLISH, "language entry not found in resource" );

        nPos = FindIndex( LANGUAGE_DONTKNOW );

        if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
            return ResStringArray::GetString( nPos );
    }
    static String aEmptyStr;
    return aEmptyStr;
}

//------------------------------------------------------------------------

LanguageType SvtLanguageTable::GetType( const String& rStr ) const
{
    LanguageType eType = LANGUAGE_DONTKNOW;
    sal_uInt32 nCount = Count();

    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        if ( rStr == ResStringArray::GetString( i ) )
        {
            eType = LanguageType( GetValue( i ) );
            break;
        }
    }
    return eType;
}

//------------------------------------------------------------------------

sal_uInt32 SvtLanguageTable::GetEntryCount() const
{
    return Count();
}

//------------------------------------------------------------------------

LanguageType SvtLanguageTable::GetTypeAtIndex( sal_uInt32 nIndex ) const
{
    LanguageType nType = LANGUAGE_DONTKNOW;
    if (nIndex < Count())
        nType = LanguageType( GetValue( nIndex ) );
    return nType;
}

//------------------------------------------------------------------------


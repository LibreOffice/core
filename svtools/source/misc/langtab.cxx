/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: langtab.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:50:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

// include ---------------------------------------------------------------

#include <tools/shl.hxx>
#include <tools/debug.hxx>

#include <i18npool/lang.h>

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
    // no_NO is an alias for nb_NO
    LanguageType eLang = (eType == LANGUAGE_NORWEGIAN ?
            LANGUAGE_NORWEGIAN_BOKMAL : eType);
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


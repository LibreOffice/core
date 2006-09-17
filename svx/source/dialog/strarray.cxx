/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strarray.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:40:10 $
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
#include "precompiled_svx.hxx"

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

// include ---------------------------------------------------------------

#include "dialogs.hrc" // -> RID_SVXSTR_LANGUAGE_TABLE
#include "dialmgr.hxx"
#include "strarray.hxx"

//------------------------------------------------------------------------

SvxStringArray::SvxStringArray( USHORT nResId ) :

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

const String& SvxStringArray::GetStringByPos( USHORT nPos ) const
{
    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );
    else
        return String::EmptyString();
}

//------------------------------------------------------------------------

const String& SvxStringArray::GetStringByType( long nType ) const
{
    USHORT nPos = FindIndex( nType );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );
    else
        return String::EmptyString();
}

//------------------------------------------------------------------------

long SvxStringArray::GetValueByStr( const String& rStr ) const
{
    long nType = 0;
    USHORT nCount = Count();

    for ( USHORT i = 0; i < nCount; ++i )
        if ( rStr == ResStringArray::GetString( i ) )
        {
            nType = GetValue( i );
            break;
        }
    return nType;
}



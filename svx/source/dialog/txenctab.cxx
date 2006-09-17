/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txenctab.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:46:40 $
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

#include "dialogs.hrc" // -> RID_SVXSTR_TEXTENCODING_TABLE
#include "dialmgr.hxx"
#include "txenctab.hxx"

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

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
    USHORT nPos = FindIndex( (long)nEnc );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );

    static String aEmptyString;
    return aEmptyString;
}

//------------------------------------------------------------------------

rtl_TextEncoding SvxTextEncodingTable::GetTextEncoding( const String& rStr ) const
{
    USHORT nCount = Count();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        if ( rStr == ResStringArray::GetString( i ) )
            return rtl_TextEncoding( GetValue( i ) );
    }
    return RTL_TEXTENCODING_DONTKNOW;
}


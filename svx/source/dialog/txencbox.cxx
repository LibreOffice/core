/*************************************************************************
 *
 *  $RCSfile: txencbox.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: er $ $Date: 2001-08-14 11:45:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#include "txencbox.hxx"
#include "txenctab.hxx"
#include "dialogs.hrc"

#ifndef SVX_DBCHARSETHELPER_HXX
#include "dbcharsethelper.hxx"
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

//========================================================================
//  class SvxTextEncodingBox
//========================================================================

SvxTextEncodingBox::SvxTextEncodingBox( Window* pParent, const ResId& rResId )
    :
    ListBox( pParent, rResId )
{
    m_pEncTable = new SvxTextEncodingTable;
}

//------------------------------------------------------------------------

SvxTextEncodingBox::~SvxTextEncodingBox()
{
    delete m_pEncTable;
}

//------------------------------------------------------------------------

USHORT SvxTextEncodingBox::EncodingToPos_Impl( rtl_TextEncoding nEnc ) const
{
    USHORT nCount = GetEntryCount();
    for ( USHORT i=0; i<nCount; i++ )
    {
        if ( nEnc == rtl_TextEncoding( (ULONG)GetEntryData(i) ) )
            return i;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::FillFromTextEncodingTable(
            sal_uInt32 nExcludeInfoFlags, sal_uInt32 nButIncludeInfoFlags )
{
    USHORT nCount = m_pEncTable->Count();
    if ( nExcludeInfoFlags )
    {
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(rtl_TextEncodingInfo);
        for ( USHORT j=0; j<nCount; j++ )
        {
            rtl_TextEncoding nEnc = rtl_TextEncoding( m_pEncTable->GetValue( j ) );
            if ( rtl_getTextEncodingInfo( nEnc, &aInfo ) )
            {
                if ( (aInfo.Flags & nExcludeInfoFlags) == 0 )
                {
                    if ( (nExcludeInfoFlags & RTL_TEXTENCODING_INFO_UNICODE) &&
                            ((nEnc == RTL_TEXTENCODING_UCS2) ||
                            nEnc == RTL_TEXTENCODING_UCS4) )
                        ;   // InfoFlags don't work for Unicode :-(
                    else
                        InsertTextEncoding( nEnc, m_pEncTable->GetString( j ) );
                }
                else if ( (aInfo.Flags & nButIncludeInfoFlags) != 0 )
                    InsertTextEncoding( nEnc, m_pEncTable->GetString( j ) );
            }
        }
    }
    else
    {
        for ( USHORT j=0; j<nCount; j++ )
        {
            InsertTextEncoding( rtl_TextEncoding( m_pEncTable->GetValue( j ) ),
                m_pEncTable->GetString( j ) );
        }
    }
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::FillFromDbTextEncodingMap(
        sal_uInt32 nExcludeInfoFlags, sal_uInt32 nButIncludeInfoFlags )
{
    svxform::ODataAccessCharsetHelper aCSH;
    ::std::vector< rtl_TextEncoding > aEncs;
    sal_Int32 nCount = aCSH.getSupportedTextEncodings( aEncs );
    if ( nExcludeInfoFlags )
    {
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(rtl_TextEncodingInfo);
        for ( sal_Int32 j=0; j<nCount; j++ )
        {
            rtl_TextEncoding nEnc = rtl_TextEncoding( aEncs[j] );
            if ( rtl_getTextEncodingInfo( nEnc, &aInfo ) )
            {
                if ( (aInfo.Flags & nExcludeInfoFlags) == 0 )
                {
                    if ( (nExcludeInfoFlags & RTL_TEXTENCODING_INFO_UNICODE) &&
                            ((nEnc == RTL_TEXTENCODING_UCS2) ||
                            nEnc == RTL_TEXTENCODING_UCS4) )
                        ;   // InfoFlags don't work for Unicode :-(
                    else
                        InsertTextEncoding( nEnc );
                }
                else if ( (aInfo.Flags & nButIncludeInfoFlags) != 0 )
                    InsertTextEncoding( nEnc );
            }
        }
    }
    else
    {
        for ( sal_Int32 j=0; j<nCount; j++ )
        {
            rtl_TextEncoding nEnc = rtl_TextEncoding( aEncs[j] );
            // CharsetMap offers a RTL_TEXTENCODING_DONTKNOW for internal use,
            // makes no sense here and would result in an empty string as list
            // entry.
            if ( nEnc != RTL_TEXTENCODING_DONTKNOW )
                InsertTextEncoding( nEnc );
        }
    }
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::FillWithMimeAndSelectBest()
{
    FillFromTextEncodingTable( 0xffffffff, RTL_TEXTENCODING_INFO_MIME );
    const sal_Char* pCharSet = rtl_getBestMimeCharsetFromTextEncoding( gsl_getSystemTextEncoding() );
    rtl_TextEncoding nEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );
    SelectTextEncoding( nEnc );
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc,
            const String& rEntry, USHORT nPos )
{
    USHORT nAt = InsertEntry( rEntry, nPos );
    SetEntryData( nAt, (void*)(ULONG)nEnc );
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc, USHORT nPos )
{
    const String& rEntry = m_pEncTable->GetTextString( nEnc );
    InsertTextEncoding( nEnc, rEntry, nPos );
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::RemoveTextEncoding( const rtl_TextEncoding nEnc )
{
    USHORT nAt = EncodingToPos_Impl( nEnc );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        RemoveEntry( nAt );
}

//------------------------------------------------------------------------

rtl_TextEncoding SvxTextEncodingBox::GetSelectTextEncoding() const
{
    USHORT nPos = GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return rtl_TextEncoding( (ULONG)GetEntryData(nPos) );
    else
        return RTL_TEXTENCODING_DONTKNOW;
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::SelectTextEncoding( const rtl_TextEncoding nEnc, BOOL bSelect )
{
    USHORT nAt = EncodingToPos_Impl( nEnc );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        SelectEntryPos( nAt, bSelect );
}

//------------------------------------------------------------------------

BOOL SvxTextEncodingBox::IsTextEncodingSelected( const rtl_TextEncoding nEnc ) const
{
    USHORT nAt = EncodingToPos_Impl( nEnc );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nAt );
    else
        return FALSE;
}


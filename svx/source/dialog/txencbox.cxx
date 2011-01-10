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

#include "svx/txencbox.hxx"
#include "svx/txenctab.hxx"
#include <svx/dialogs.hrc>
#include "svx/dbcharsethelper.hxx"
#include <vcl/svapp.hxx>
#include <rtl/tencinfo.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>

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

sal_uInt16 SvxTextEncodingBox::EncodingToPos_Impl( rtl_TextEncoding nEnc ) const
{
    sal_uInt16 nCount = GetEntryCount();
    for ( sal_uInt16 i=0; i<nCount; i++ )
    {
        if ( nEnc == rtl_TextEncoding( (sal_uIntPtr)GetEntryData(i) ) )
            return i;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::FillFromTextEncodingTable(
        sal_Bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags,
        sal_uInt32 nButIncludeInfoFlags )
{
    rtl_TextEncodingInfo aInfo;
    aInfo.StructSize = sizeof(rtl_TextEncodingInfo);
    sal_uInt32 nCount = m_pEncTable->Count();
    for ( sal_uInt32 j=0; j<nCount; j++ )
    {
        sal_Bool bInsert = sal_True;
        rtl_TextEncoding nEnc = rtl_TextEncoding( m_pEncTable->GetValue( j ) );
        if ( nExcludeInfoFlags )
        {
            if ( !rtl_getTextEncodingInfo( nEnc, &aInfo ) )
                bInsert = sal_False;
            else
            {
                if ( (aInfo.Flags & nExcludeInfoFlags) == 0 )
                {
                    if ( (nExcludeInfoFlags & RTL_TEXTENCODING_INFO_UNICODE) &&
                            ((nEnc == RTL_TEXTENCODING_UCS2) ||
                            nEnc == RTL_TEXTENCODING_UCS4) )
                        bInsert = sal_False;    // InfoFlags don't work for Unicode :-(
                }
                else if ( (aInfo.Flags & nButIncludeInfoFlags) == 0 )
                    bInsert = sal_False;
            }
        }
        if ( bInsert )
        {
            if ( bExcludeImportSubsets )
            {
                switch ( nEnc )
                {
                    // subsets of RTL_TEXTENCODING_GB_18030
                    case RTL_TEXTENCODING_GB_2312 :
                    case RTL_TEXTENCODING_GBK :
                    case RTL_TEXTENCODING_MS_936 :
                        bInsert = sal_False;
                    break;
                }
            }
            if ( bInsert )
                InsertTextEncoding( nEnc, m_pEncTable->GetString( j ) );
        }
    }
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::FillFromDbTextEncodingMap(
        sal_Bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags,
        sal_uInt32 nButIncludeInfoFlags )
{
    rtl_TextEncodingInfo aInfo;
    aInfo.StructSize = sizeof(rtl_TextEncodingInfo);
    svxform::ODataAccessCharsetHelper aCSH;
    ::std::vector< rtl_TextEncoding > aEncs;
    sal_Int32 nCount = aCSH.getSupportedTextEncodings( aEncs );
    for ( sal_uInt16 j=0; j<nCount; j++ )
    {
        sal_Bool bInsert = sal_True;
        rtl_TextEncoding nEnc = rtl_TextEncoding( aEncs[j] );
        if ( nExcludeInfoFlags )
        {
            if ( !rtl_getTextEncodingInfo( nEnc, &aInfo ) )
                bInsert = sal_False;
            else
            {
                if ( (aInfo.Flags & nExcludeInfoFlags) == 0 )
                {
                    if ( (nExcludeInfoFlags & RTL_TEXTENCODING_INFO_UNICODE) &&
                            ((nEnc == RTL_TEXTENCODING_UCS2) ||
                            nEnc == RTL_TEXTENCODING_UCS4) )
                        bInsert = sal_False;    // InfoFlags don't work for Unicode :-(
                }
                else if ( (aInfo.Flags & nButIncludeInfoFlags) == 0 )
                    bInsert = sal_False;
            }
        }
        if ( bInsert )
        {
            if ( bExcludeImportSubsets )
            {
                switch ( nEnc )
                {
                    // subsets of RTL_TEXTENCODING_GB_18030
                    case RTL_TEXTENCODING_GB_2312 :
                    case RTL_TEXTENCODING_GBK :
                    case RTL_TEXTENCODING_MS_936 :
                        bInsert = sal_False;
                    break;
                }
            }
            // CharsetMap offers a RTL_TEXTENCODING_DONTKNOW for internal use,
            // makes no sense here and would result in an empty string as list
            // entry.
            if ( bInsert && nEnc != RTL_TEXTENCODING_DONTKNOW )
                InsertTextEncoding( nEnc );
        }
    }
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::FillWithMimeAndSelectBest()
{
    FillFromTextEncodingTable( sal_False, 0xffffffff, RTL_TEXTENCODING_INFO_MIME );
    rtl_TextEncoding nEnc = SvtSysLocale::GetBestMimeEncoding();
    SelectTextEncoding( nEnc );
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc,
            const String& rEntry, sal_uInt16 nPos )
{
    sal_uInt16 nAt = InsertEntry( rEntry, nPos );
    SetEntryData( nAt, (void*)(sal_uIntPtr)nEnc );
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc, sal_uInt16 nPos )
{
    const String& rEntry = m_pEncTable->GetTextString( nEnc );
    if ( rEntry.Len() )
        InsertTextEncoding( nEnc, rEntry, nPos );
    else
    {
#ifdef DBG_UTIL
        ByteString aMsg( "SvxTextEncodingBox::InsertTextEncoding: no resource string for text encoding: " );
        aMsg += ByteString::CreateFromInt32( nEnc );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::RemoveTextEncoding( const rtl_TextEncoding nEnc )
{
    sal_uInt16 nAt = EncodingToPos_Impl( nEnc );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        RemoveEntry( nAt );
}

//------------------------------------------------------------------------

rtl_TextEncoding SvxTextEncodingBox::GetSelectTextEncoding() const
{
    sal_uInt16 nPos = GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return rtl_TextEncoding( (sal_uIntPtr)GetEntryData(nPos) );
    else
        return RTL_TEXTENCODING_DONTKNOW;
}

//------------------------------------------------------------------------

void SvxTextEncodingBox::SelectTextEncoding( const rtl_TextEncoding nEnc, sal_Bool bSelect )
{
    sal_uInt16 nAt = EncodingToPos_Impl( nEnc );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        SelectEntryPos( nAt, bSelect );
}

//------------------------------------------------------------------------

sal_Bool SvxTextEncodingBox::IsTextEncodingSelected( const rtl_TextEncoding nEnc ) const
{
    sal_uInt16 nAt = EncodingToPos_Impl( nEnc );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nAt );
    else
        return sal_False;
}


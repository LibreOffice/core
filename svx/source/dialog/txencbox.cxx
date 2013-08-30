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


#include "svx/txencbox.hxx"
#include "svx/txenctab.hxx"
#include <svx/dialogs.hrc>
#ifndef DISABLE_DBCONNECTIVITY
#include "svx/dbcharsethelper.hxx"
#endif
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include <rtl/tencinfo.h>
#include <rtl/locale.h>
#include <rtl/strbuf.hxx>
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

SvxTextEncodingBox::SvxTextEncodingBox( Window* pParent, WinBits nBits )
    : ListBox( pParent, nBits )
{
    m_pEncTable = new SvxTextEncodingTable;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxTextEncodingBox(Window *pParent, VclBuilder::stringmap &)
{
    WinBits nWinBits = WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
    SvxTextEncodingBox *pListBox = new SvxTextEncodingBox(pParent, nWinBits);
    pListBox->EnableAutoSize(true);
    return pListBox;
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
#ifdef DISABLE_DBCONNECTIVITY
    (void)bExcludeImportSubsets;
    (void)nExcludeInfoFlags;
    (void)nButIncludeInfoFlags;
#else
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
#endif
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
            const OUString& rEntry, sal_uInt16 nPos )
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
        SAL_WARN( "svx.dialog", "SvxTextEncodingBox::InsertTextEncoding: no resource string for text encoding: " << static_cast<sal_Int32>( nEnc ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

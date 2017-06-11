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

#include <config_features.h>

#include "svx/txencbox.hxx"
#include <svx/dialmgr.hxx>
#include "svx/txenctab.hxx"
#include <svx/strings.hrc>
#if HAVE_FEATURE_DBCONNECTIVITY
#include "svx/dbcharsethelper.hxx"
#endif
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/tencinfo.h>
#include <rtl/locale.h>
#include <rtl/strbuf.hxx>
#include <osl/nlsupport.h>
#include "txenctab.hrc"

SvxTextEncodingBox::SvxTextEncodingBox( vcl::Window* pParent, WinBits nBits )
    : ListBox( pParent, nBits )
{
}

VCL_BUILDER_DECL_FACTORY(SvxTextEncodingBox)
{
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
    bool bDropdown = VclBuilder::extractDropdown(rMap);
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    OUString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinBits |= WB_BORDER;
    VclPtrInstance<SvxTextEncodingBox> pListBox(pParent, nWinBits);
    if (bDropdown)
        pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

SvxTextEncodingBox::~SvxTextEncodingBox()
{
    disposeOnce();
}

sal_Int32 SvxTextEncodingBox::EncodingToPos_Impl( rtl_TextEncoding nEnc ) const
{
    sal_Int32 nCount = GetEntryCount();
    for ( sal_Int32 i=0; i<nCount; i++ )
    {
        if ( nEnc == rtl_TextEncoding( reinterpret_cast<sal_uIntPtr>(GetEntryData(i)) ) )
            return i;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}


void SvxTextEncodingBox::FillFromTextEncodingTable(
        bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags,
        sal_uInt32 nButIncludeInfoFlags )
{
    rtl_TextEncodingInfo aInfo;
    aInfo.StructSize = sizeof(rtl_TextEncodingInfo);
    sal_uInt32 nCount = SAL_N_ELEMENTS(RID_SVXSTR_TEXTENCODING_TABLE);
    for ( sal_uInt32 j=0; j<nCount; j++ )
    {
        bool bInsert = true;
        rtl_TextEncoding nEnc = RID_SVXSTR_TEXTENCODING_TABLE[j].second;
        if ( nExcludeInfoFlags )
        {
            if ( !rtl_getTextEncodingInfo( nEnc, &aInfo ) )
                bInsert = false;
            else
            {
                if ( (aInfo.Flags & nExcludeInfoFlags) == 0 )
                {
                    if ( (nExcludeInfoFlags & RTL_TEXTENCODING_INFO_UNICODE) &&
                            ((nEnc == RTL_TEXTENCODING_UCS2) ||
                            nEnc == RTL_TEXTENCODING_UCS4) )
                        bInsert = false;    // InfoFlags don't work for Unicode :-(
                }
                else if ( (aInfo.Flags & nButIncludeInfoFlags) == 0 )
                    bInsert = false;
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
                        bInsert = false;
                    break;
                }
            }
            if ( bInsert )
                InsertTextEncoding(nEnc, SvxResId(RID_SVXSTR_TEXTENCODING_TABLE[j].first));
        }
    }
}


void SvxTextEncodingBox::FillFromDbTextEncodingMap(
        bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void)bExcludeImportSubsets;
    (void)nExcludeInfoFlags;
#else
    rtl_TextEncodingInfo aInfo;
    aInfo.StructSize = sizeof(rtl_TextEncodingInfo);
    ::std::vector< rtl_TextEncoding > aEncs;
    sal_Int32 nCount = svxform::charset_helper::getSupportedTextEncodings( aEncs );
    for ( sal_Int32 j=0; j<nCount; j++ )
    {
        bool bInsert = true;
        rtl_TextEncoding nEnc = rtl_TextEncoding( aEncs[j] );
        if ( nExcludeInfoFlags )
        {
            if ( !rtl_getTextEncodingInfo( nEnc, &aInfo ) )
                bInsert = false;
            else
            {
                if ( (aInfo.Flags & nExcludeInfoFlags) == 0 )
                {
                    if ( (nExcludeInfoFlags & RTL_TEXTENCODING_INFO_UNICODE) &&
                            ((nEnc == RTL_TEXTENCODING_UCS2) ||
                            nEnc == RTL_TEXTENCODING_UCS4) )
                        bInsert = false;    // InfoFlags don't work for Unicode :-(
                }
                else
                    bInsert = false;
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
                        bInsert = false;
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


void SvxTextEncodingBox::FillWithMimeAndSelectBest()
{
    FillFromTextEncodingTable( false, 0xffffffff, RTL_TEXTENCODING_INFO_MIME );
    rtl_TextEncoding nEnc = SvtSysLocale::GetBestMimeEncoding();
    SelectTextEncoding( nEnc );
}


void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc,
            const OUString& rEntry )
{
    sal_Int32 nAt = InsertEntry( rEntry );
    SetEntryData( nAt, reinterpret_cast<void*>(nEnc) );
}


void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc )
{
    const OUString& rEntry = SvxTextEncodingTable::GetTextString(nEnc);
    if ( !rEntry.isEmpty() )
        InsertTextEncoding( nEnc, rEntry );
    else
        SAL_WARN( "svx.dialog", "SvxTextEncodingBox::InsertTextEncoding: no resource string for text encoding: " << static_cast<sal_Int32>( nEnc ) );
}


rtl_TextEncoding SvxTextEncodingBox::GetSelectTextEncoding() const
{
    sal_Int32 nPos = GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return rtl_TextEncoding( reinterpret_cast<sal_uIntPtr>(GetEntryData(nPos)) );
    else
        return RTL_TEXTENCODING_DONTKNOW;
}


void SvxTextEncodingBox::SelectTextEncoding( const rtl_TextEncoding nEnc )
{
    sal_Int32 nAt = EncodingToPos_Impl( nEnc );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        SelectEntryPos( nAt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

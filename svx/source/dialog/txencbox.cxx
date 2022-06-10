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

#include <svx/txencbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/txenctab.hxx>
#if HAVE_FEATURE_DBCONNECTIVITY
#include <dbcharsethelper.hxx>
#endif
#include <unotools/syslocale.hxx>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <txenctab.hrc>

namespace
{
    std::vector<rtl_TextEncoding> FillFromDbTextEncodingMap(bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags)
    {
        std::vector<rtl_TextEncoding> aRet;
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
                    aRet.push_back(nEnc);
            }
        }
#endif
        return aRet;
    }
}

void SvxTextEncodingBox::FillFromDbTextEncodingMap(
        bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags )
{
    m_xControl->freeze();
    auto aEncs = ::FillFromDbTextEncodingMap(bExcludeImportSubsets, nExcludeInfoFlags);
    for (auto nEnc : aEncs)
        InsertTextEncoding(nEnc);
    m_xControl->thaw();
}

void SvxTextEncodingTreeView::FillFromDbTextEncodingMap(
        bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags )
{
    m_xControl->freeze();
    auto aEncs = ::FillFromDbTextEncodingMap(bExcludeImportSubsets, nExcludeInfoFlags);
    for (auto nEnc : aEncs)
        InsertTextEncoding(nEnc);
    m_xControl->thaw();
}

SvxTextEncodingBox::SvxTextEncodingBox(std::unique_ptr<weld::ComboBox> pControl)
    : m_xControl(std::move(pControl))
{
    m_xControl->make_sorted();
}

SvxTextEncodingTreeView::SvxTextEncodingTreeView(std::unique_ptr<weld::TreeView> pControl)
    : m_xControl(std::move(pControl))
{
    m_xControl->make_sorted();
}

SvxTextEncodingBox::~SvxTextEncodingBox()
{
}

SvxTextEncodingTreeView::~SvxTextEncodingTreeView()
{
}

namespace
{
    std::vector<int> FillFromTextEncodingTable(bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags)
    {
        std::vector<int> aRet;

        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(rtl_TextEncodingInfo);
        const sal_uInt32 nCount = SAL_N_ELEMENTS(RID_SVXSTR_TEXTENCODING_TABLE);
        for (sal_uInt32 j = 0; j < nCount; ++j)
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
                    else
                        bInsert = false;
                }
            }
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
                aRet.push_back(j);
        }
        return aRet;
    }
}

void SvxTextEncodingBox::FillFromTextEncodingTable(
        bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags )
{
    std::vector<int> aRet(::FillFromTextEncodingTable(bExcludeImportSubsets, nExcludeInfoFlags));
    m_xControl->freeze();
    for (auto j : aRet)
    {
        rtl_TextEncoding nEnc = RID_SVXSTR_TEXTENCODING_TABLE[j].second;
        InsertTextEncoding(nEnc, SvxResId(RID_SVXSTR_TEXTENCODING_TABLE[j].first));
    }
    m_xControl->thaw();
}

void SvxTextEncodingTreeView::FillFromTextEncodingTable(
        bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags )
{
    std::vector<int> aRet(::FillFromTextEncodingTable(bExcludeImportSubsets, nExcludeInfoFlags));
    m_xControl->freeze();
    for (auto j : aRet)
    {
        rtl_TextEncoding nEnc = RID_SVXSTR_TEXTENCODING_TABLE[j].second;
        InsertTextEncoding(nEnc, SvxResId(RID_SVXSTR_TEXTENCODING_TABLE[j].first));
    }
    m_xControl->thaw();
}

void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc,
            const OUString& rEntry )
{
    m_xControl->append(OUString::number(nEnc), rEntry);
}

void SvxTextEncodingTreeView::InsertTextEncoding( const rtl_TextEncoding nEnc,
            const OUString& rEntry )
{
    m_xControl->append(OUString::number(nEnc), rEntry);
}

void SvxTextEncodingBox::InsertTextEncoding( const rtl_TextEncoding nEnc )
{
    const OUString& rEntry = SvxTextEncodingTable::GetTextString(nEnc);
    if (!rEntry.isEmpty())
        InsertTextEncoding( nEnc, rEntry );
    else
        SAL_WARN( "svx.dialog", "SvxTextEncodingBox::InsertTextEncoding: no resource string for text encoding: " << static_cast<sal_Int32>( nEnc ) );
}

void SvxTextEncodingTreeView::InsertTextEncoding( const rtl_TextEncoding nEnc )
{
    const OUString& rEntry = SvxTextEncodingTable::GetTextString(nEnc);
    if (!rEntry.isEmpty())
        InsertTextEncoding( nEnc, rEntry );
    else
        SAL_WARN( "svx.dialog", "SvxTextEncodingTreeView::InsertTextEncoding: no resource string for text encoding: " << static_cast<sal_Int32>( nEnc ) );
}

rtl_TextEncoding SvxTextEncodingBox::GetSelectTextEncoding() const
{
    OUString sId(m_xControl->get_active_id());
    if (!sId.isEmpty())
        return rtl_TextEncoding(sId.toInt32());
    else
        return RTL_TEXTENCODING_DONTKNOW;
}

rtl_TextEncoding SvxTextEncodingTreeView::GetSelectTextEncoding() const
{
    OUString sId(m_xControl->get_selected_id());
    if (!sId.isEmpty())
        return rtl_TextEncoding(sId.toInt32());
    else
        return RTL_TEXTENCODING_DONTKNOW;
}

void SvxTextEncodingBox::SelectTextEncoding( const rtl_TextEncoding nEnc )
{
    m_xControl->set_active_id(OUString::number(nEnc));
}

void SvxTextEncodingTreeView::SelectTextEncoding( const rtl_TextEncoding nEnc )
{
    m_xControl->select_id(OUString::number(nEnc));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <sal/config.h>

#include <string_view>

#include "fileview.hxx"
#include "iodlgimp.hxx"
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <svtools/inettbc.hxx>
#include "iodlg.hxx"
#include <svtools/imagemgr.hxx>
#include <svl/svlresid.hxx>
#include <svl/svl.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::utl;

SvtFileDialogFilter_Impl::SvtFileDialogFilter_Impl( const OUString& rName, const OUString& rType )
    : m_aName( rName )
    , m_aType( rType )
{
    m_aType = m_aType.toAsciiLowerCase();
}

SvtFileDialogFilter_Impl::~SvtFileDialogFilter_Impl()
{
}

//= SvtUpButton_Impl
SvtUpButton_Impl::SvtUpButton_Impl(std::unique_ptr<weld::Toolbar> xToolbar,
                                   std::unique_ptr<weld::Menu> xMenu,
                                   SvtFileDialog* pDlg)
    : m_xToolbar(std::move(xToolbar))
    , m_xMenu(std::move(xMenu))
    , m_pDlg(pDlg)
{
    m_xToolbar->set_item_menu("up_btn", m_xMenu.get());
    m_xToolbar->connect_clicked(LINK(this, SvtUpButton_Impl, ClickHdl));
    m_xMenu->connect_activate(LINK(this, SvtUpButton_Impl, SelectHdl));
}

void SvtUpButton_Impl::FillURLMenu()
{
    SvtFileView* pBox = m_pDlg->GetView();

    sal_uInt16 nItemId = 1;

    aURLs.clear();
    m_xMenu->clear();

    // determine parent levels
    INetURLObject aObject( pBox->GetViewURL() );
    sal_Int32 nCount = aObject.getSegmentCount();

    ::svtools::VolumeInfo aVolInfo( true /* volume */, false /* remote */,
                                    false /* removable */, false /* floppy */,
                                    false /* compact disk */ );
    OUString aVolumeImage( SvFileInformationManager::GetFolderImageId( aVolInfo ) );

    while ( nCount >= 1 )
    {
        aObject.removeSegment();
        OUString aParentURL(aObject.GetMainURL(INetURLObject::DecodeMechanism::NONE));

        OUString aTitle;

        if (nCount == 1) // adjust the title of the top level entry (the workspace)
            aTitle = SvlResId(STR_SVT_MIMETYPE_CNT_FSYSBOX);
        else if (!m_pDlg->ContentGetTitle(aParentURL, aTitle) || aTitle.isEmpty())
            aTitle = aObject.getName();

        OUString aImage = ( nCount > 1 ) // if nCount == 1 means workplace, which detects the wrong image
            ? SvFileInformationManager::GetImageId( aObject ) : aVolumeImage;

        m_xMenu->append(OUString::number(nItemId), aTitle, aImage);
        aURLs.push_back(aParentURL);

        ++nItemId;
        --nCount;
    }
}

IMPL_LINK(SvtUpButton_Impl, SelectHdl, const OString&, rId, void)
{
    sal_uInt32 nId = rId.toUInt32();
    if (nId)
    {
        --nId;
        assert( nId <= aURLs.size() &&  "SvtUpButton_Impl: wrong index" );

        m_pDlg->OpenURL_Impl(aURLs[nId]);
    }
}

IMPL_LINK_NOARG(SvtUpButton_Impl, ClickHdl, const OString&, void)
{
    m_pDlg->PrevLevel_Impl();
}

// SvtExpFileDlg_Impl
SvtExpFileDlg_Impl::SvtExpFileDlg_Impl()
    : m_pCurFilter( nullptr )
    , m_eMode( FILEDLG_MODE_OPEN )
    , m_eDlgType( FILEDLG_TYPE_FILEDLG )
    , m_nStyle( PickerFlags::NONE )
    , m_bDoubleClick( false )
    , m_bMultiSelection( false )
{
}

SvtExpFileDlg_Impl::~SvtExpFileDlg_Impl()
{
}

void SvtExpFileDlg_Impl::SetStandardDir( const OUString& _rDir )
{
    m_aStdDir = _rDir;
    if (m_aStdDir.isEmpty())
        m_aStdDir = "file:///";
}

namespace {
    OUString lcl_DecoratedFilter( std::u16string_view _rOriginalFilter )
    {
        return "<" + OUString::Concat(_rOriginalFilter) + ">";
    }
}

void SvtExpFileDlg_Impl::SetCurFilter( SvtFileDialogFilter_Impl const * pFilter, const OUString& rDisplayName )
{
    DBG_ASSERT( pFilter, "SvtExpFileDlg_Impl::SetCurFilter: invalid filter!" );
    DBG_ASSERT( ( rDisplayName == pFilter->GetName() )
            ||  ( rDisplayName == lcl_DecoratedFilter( pFilter->GetName() ) ),
            "SvtExpFileDlg_Impl::SetCurFilter: arguments are inconsistent!" );

    m_pCurFilter = pFilter;
    m_sCurrentFilterDisplayName = rDisplayName;
}

void SvtExpFileDlg_Impl::InsertFilterListEntry(const SvtFileDialogFilter_Impl* pFilterDesc)
{
    // insert and set user data
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pFilterDesc)));
    OUString sName = pFilterDesc->GetName();
    if (pFilterDesc->isGroupSeparator())
        m_xLbFilter->append_separator(sId);
    else
        m_xLbFilter->append(sId, sName);
}

void SvtExpFileDlg_Impl::InitFilterList( )
{
    // clear the current list
    m_xLbFilter->clear();

    // reinit it
    sal_uInt16 nPos = m_aFilter.size();

    // search for the first entry which is no group separator
    while ( nPos-- && m_aFilter[ nPos ]->isGroupSeparator() )
        ;

    // add all following entries
    while ( static_cast<sal_Int16>(nPos) >= 0 )
        InsertFilterListEntry( m_aFilter[ nPos-- ].get() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

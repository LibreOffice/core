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

#include "fileview.hxx"
#include "iodlgimp.hxx"
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/menu.hxx>
#include <svtools/inettbc.hxx>
#include "iodlg.hxx"
#include <bitmaps.hlst>
#include <svtools/imagemgr.hxx>
#include <svl/svlresid.hxx>
#include <svl/svl.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::utl;

SvtFileDialogFilter_Impl::SvtFileDialogFilter_Impl( const OUString& rName, const OUString& rType )
    :m_aName( rName )
    ,m_aType( rType )
{
    m_aType = m_aType.toAsciiLowerCase();
}


SvtFileDialogFilter_Impl::~SvtFileDialogFilter_Impl()
{
}

// SvtFileDialogFilterList_Impl

//= SvtFileDialogURLSelector
SvtFileDialogURLSelector::SvtFileDialogURLSelector(std::unique_ptr<weld::MenuButton> xButton, SvtFileDialog* pDlg, const OUString& rButtonId)
    : m_xButton(std::move(xButton))
    , m_pDlg(pDlg)
//TODO    , m_pMenu    ( VclPtr<PopupMenu>::Create() )
{
//TODO    SetStyle( GetStyle() | WB_NOPOINTERFOCUS | WB_RECTSTYLE | WB_SMALLSTYLE );
    m_xButton->set_from_icon_name(rButtonId);
//TODO    SetDelayMenu(true);
//TODO    SetDropDown(PushButtonDropdownStyle::Toolbox);
}

SvtFileDialogURLSelector::~SvtFileDialogURLSelector()
{
}

#if 0
void SvtFileDialogURLSelector::Activate()
{
    m_pMenu->Clear();
    FillURLMenu( m_pMenu );
    SetPopupMenu( m_pMenu );
}
#endif

//= SvtUpButton_Impl
SvtUpButton_Impl::SvtUpButton_Impl(std::unique_ptr<weld::MenuButton> xButton, SvtFileDialog* pDlg)
    : SvtFileDialogURLSelector(std::move(xButton), pDlg, BMP_FILEDLG_BTN_UP)
{
}

SvtUpButton_Impl::~SvtUpButton_Impl()
{
}

void SvtUpButton_Impl::FillURLMenu()
{
    SvtFileView* pBox = m_pDlg->GetView();

    sal_uInt16 nItemId = 1;

    aURLs.clear();

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

        m_xButton->append_item(OUString::number(nItemId), aTitle, aImage);
        aURLs.push_back(aParentURL);

        ++nItemId;
        --nCount;
    }
}

#if 0
void SvtUpButton_Impl::Select()
{
    sal_uInt16 nId = GetCurItemId();

    if ( nId )
    {
        --nId;
        assert( nId <= aURLs.size() &&  "SvtUpButton_Impl: wrong index" );

        m_pDlg->OpenURL_Impl(aURLs[nId]);
    }
}

void SvtUpButton_Impl::Click()
{
    m_pDlg->PrevLevel_Impl();
}
#endif

#if 0
Size SvtUpButton_Impl::GetOptimalSize() const
{
    return LogicToPixel(Size(12, 12), MapMode(MapUnit::MapAppFont));
}
#endif

// SvtExpFileDlg_Impl
SvtExpFileDlg_Impl::SvtExpFileDlg_Impl()
    : m_pCurFilter( nullptr )
    , m_eMode( FILEDLG_MODE_OPEN )
    , m_eDlgType( FILEDLG_TYPE_FILEDLG )
    , m_nStyle( PickerFlags::NONE )
    , m_bDoubleClick( false )
    , m_bNeedDelayedFilterExecute ( false )
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
    OUString lcl_DecoratedFilter( const OUString& _rOriginalFilter )
    {
        return "<" + _rOriginalFilter + ">";
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
    OUString sName = pFilterDesc->GetName();
    if (pFilterDesc->isGroupSeparator())
        sName = "------------------------------------------";
    else
        sName = pFilterDesc->GetName();

    // insert and set user data
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pFilterDesc)));
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

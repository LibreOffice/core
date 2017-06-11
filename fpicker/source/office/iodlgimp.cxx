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

#include "iodlgimp.hxx"
#include "svtools/headbar.hxx"
#include <tools/debug.hxx>
#include <tools/simplerm.hxx>
#include <tools/urlobj.hxx>
#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include "svl/ctypeitm.hxx"
#include "svl/eitem.hxx"
#include "unotools/viewoptions.hxx"
#include "svtools/fileview.hxx"
#include "svtools/inettbc.hxx"
#include "iodlg.hxx"
#include "strings.hrc"
#include "bitmaps.hlst"
#include "svtools/imagemgr.hxx"
#include <unotools/localfilehelper.hxx>
#include "unotools/useroptions.hxx"
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
SvtFileDialogURLSelector::SvtFileDialogURLSelector(vcl::Window* _pParent, SvtFileDialog* _pDlg, WinBits nBits, const OUString& rButtonId)
    :MenuButton ( _pParent, nBits )
    ,m_pDlg     ( _pDlg )
    ,m_pMenu    ( VclPtr<PopupMenu>::Create() )
{
    SetStyle( GetStyle() | WB_NOPOINTERFOCUS | WB_RECTSTYLE | WB_SMALLSTYLE );
    SetModeImage(SvtFileDialog::GetButtonImage(rButtonId));
    SetDelayMenu(true);
    SetDropDown(PushButtonDropdownStyle::Toolbox);
}


SvtFileDialogURLSelector::~SvtFileDialogURLSelector()
{
    disposeOnce();
}

void SvtFileDialogURLSelector::dispose()
{
    m_pMenu.disposeAndClear();
    m_pDlg.clear();
    MenuButton::dispose();
}


void SvtFileDialogURLSelector::Activate()
{
    m_pMenu->Clear();

    FillURLMenu( m_pMenu );

    SetPopupMenu( m_pMenu );
}


//= SvtUpButton_Impl


SvtUpButton_Impl::SvtUpButton_Impl( vcl::Window *pParent, SvtFileDialog* pDlg, WinBits nBits )
    :SvtFileDialogURLSelector( pParent, pDlg, nBits, BMP_FILEDLG_BTN_UP )
{
}


SvtUpButton_Impl::~SvtUpButton_Impl()
{
}


void SvtUpButton_Impl::FillURLMenu( PopupMenu* _pMenu )
{
    SvtFileView* pBox = GetDialogParent()->GetView();

    sal_uInt16 nItemId = 1;

    _aURLs.clear();

    // determine parent levels
    INetURLObject aObject( pBox->GetViewURL() );
    sal_Int32 nCount = aObject.getSegmentCount();

    ::svtools::VolumeInfo aVolInfo( true /* volume */, false /* remote */,
                                    false /* removable */, false /* floppy */,
                                    false /* compact disk */ );
    Image aVolumeImage( SvFileInformationManager::GetFolderImage( aVolInfo ) );

    while ( nCount >= 1 )
    {
        aObject.removeSegment();
        OUString aParentURL(aObject.GetMainURL(INetURLObject::DecodeMechanism::NONE));

        OUString aTitle;
        if (!GetDialogParent()->ContentGetTitle(aParentURL, aTitle) || aTitle.isEmpty())
            aTitle = aObject.getName();

        Image aImage = ( nCount > 1 ) // if nCount == 1 means workplace, which detects the wrong image
            ? SvFileInformationManager::GetImage( aObject ) : aVolumeImage;

        _pMenu->InsertItem( nItemId++, aTitle, aImage );
        _aURLs.push_back(aParentURL);

        if ( nCount == 1 )
        {
            // adjust the title of the top level entry (the workspace)
            std::locale loc = Translate::Create("svl");
            _pMenu->SetItemText(--nItemId, Translate::get(STR_SVT_MIMETYPE_CNT_FSYSBOX, loc));
        }
        --nCount;
    }
}

void SvtUpButton_Impl::Select()
{
    sal_uInt16 nId = GetCurItemId();

    if ( nId )
    {
        --nId;
        assert( nId <= _aURLs.size() &&  "SvtUpButton_Impl:falscher Index" );

        GetDialogParent()->OpenURL_Impl(_aURLs[nId]);
    }
}

void SvtUpButton_Impl::Click()
{
    GetDialogParent()->PrevLevel_Impl();
}

Size SvtUpButton_Impl::GetOptimalSize() const
{
    return LogicToPixel(Size(12, 12), MapUnit::MapAppFont);
}

// SvtExpFileDlg_Impl
SvtExpFileDlg_Impl::SvtExpFileDlg_Impl()   :

    _pCurFilter         ( nullptr ),
    _pUserFilter        ( nullptr ),
    _pFtFileName        ( nullptr ),
    _pEdFileName        ( nullptr ),
    _pFtFileVersion     ( nullptr ),
    _pLbFileVersion     ( nullptr ),
    _pFtTemplates       ( nullptr ),
    _pLbTemplates       ( nullptr ),
    _pFtImageTemplates  ( nullptr ),
    _pLbImageTemplates  ( nullptr ),
    _pFtFileType        ( nullptr ),
    _pLbFilter          ( nullptr ),
    _pBtnFileOpen       ( nullptr ),
    _pBtnCancel         ( nullptr ),
    _pBtnHelp           ( nullptr ),
    _pBtnUp             ( nullptr ),
    _pBtnNewFolder      ( nullptr ),
    _pCbPassword        ( nullptr ),
    _pEdCurrentPath     ( nullptr ),
    _pCbAutoExtension   ( nullptr ),
    _pCbOptions         ( nullptr ),
    _pPlaces            ( nullptr ),
    _pBtnConnectToServer( nullptr ),
    _eMode              ( FILEDLG_MODE_OPEN ),
    _eDlgType           ( FILEDLG_TYPE_FILEDLG ),
    _nStyle             ( PickerFlags::NONE ),
    _bDoubleClick       ( false ),
    m_bNeedDelayedFilterExecute ( false ),
    _bMultiSelection    ( false )
{
}


SvtExpFileDlg_Impl::~SvtExpFileDlg_Impl()
{
    _pBtnUp.disposeAndClear();
    delete _pUserFilter;
    _pPlaces.disposeAndClear();
}


void SvtExpFileDlg_Impl::SetStandardDir( const OUString& _rDir )
{
    _aStdDir = _rDir;
    if ( _aStdDir.isEmpty() )
        _aStdDir = "file:///";
}

namespace {
    OUString lcl_DecoratedFilter( const OUString& _rOriginalFilter )
    {
        OUStringBuffer aDecoratedFilter;
        aDecoratedFilter.append('<');
        aDecoratedFilter.append(_rOriginalFilter);
        aDecoratedFilter.append('>');
        return aDecoratedFilter.makeStringAndClear();
    }
}

void SvtExpFileDlg_Impl::SetCurFilter( SvtFileDialogFilter_Impl* pFilter, const OUString& rDisplayName )
{
    DBG_ASSERT( pFilter, "SvtExpFileDlg_Impl::SetCurFilter: invalid filter!" );
    DBG_ASSERT( ( rDisplayName == pFilter->GetName() )
            ||  ( rDisplayName == lcl_DecoratedFilter( pFilter->GetName() ) ),
            "SvtExpFileDlg_Impl::SetCurFilter: arguments are inconsistent!" );

    _pCurFilter = pFilter;
    m_sCurrentFilterDisplayName = rDisplayName;
}


void SvtExpFileDlg_Impl::InsertFilterListEntry( const SvtFileDialogFilter_Impl* _pFilterDesc )
{
    OUString sName = _pFilterDesc->GetName();
    if ( _pFilterDesc->isGroupSeparator() )
        sName = "------------------------------------------";
    else
        sName = _pFilterDesc->GetName();

    // insert an set user data
    const sal_Int32 nPos = _pLbFilter->InsertEntry( sName );
    _pLbFilter->SetEntryData( nPos, const_cast< void* >( static_cast< const void* >( _pFilterDesc ) ) );
}


void SvtExpFileDlg_Impl::InitFilterList( )
{
    // clear the current list
    _pLbFilter->Clear();

    // reinit it
    sal_uInt16 nPos = m_aFilter.size();

    // search for the first entry which is no group separator
    while ( nPos-- && m_aFilter[ nPos ]->isGroupSeparator() )
        ;

    // add all following entries
    while ( (sal_Int16)nPos >= 0 )
        InsertFilterListEntry( m_aFilter[ nPos-- ].get() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include "iodlg.hrc"
#include "svtools/imagemgr.hxx"
#include <unotools/localfilehelper.hxx>
#include "unotools/useroptions.hxx"
#include "rtl/instance.hxx"
#include <osl/getglobalmutex.hxx>
#include <svl/svl.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::utl;


// ResMgrHolder / SvtSimpleResId

namespace
{
    struct ResMgrHolder
    {
        ResMgr * operator ()()
        {
            return ResMgr::CreateResMgr ("svl");
        }
        static ResMgr * getOrCreate()
        {
            return rtl_Instance<
                ResMgr, ResMgrHolder,
                osl::MutexGuard, osl::GetGlobalMutex >::create (
                    ResMgrHolder(), osl::GetGlobalMutex());
        }
    };

    struct SvtSimpleResId : public ResId
    {
        SvtSimpleResId (sal_uInt16 nId) : ResId (nId, *ResMgrHolder::getOrCreate()) {}
    };
}

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



SvtFileDialogURLSelector::SvtFileDialogURLSelector( vcl::Window* _pParent, SvtFileDialog* _pDlg, WinBits nBits, sal_uInt16 _nButtonId )
    :MenuButton ( _pParent, nBits )
    ,m_pDlg     ( _pDlg )
    ,m_pMenu    ( new PopupMenu )
{
    SetStyle( GetStyle() | WB_NOPOINTERFOCUS | WB_RECTSTYLE | WB_SMALLSTYLE );
    SetModeImage( _pDlg->GetButtonImage( _nButtonId ) );
    SetMenuMode( MENUBUTTON_MENUMODE_TIMED );
    SetDropDown( PushButtonDropdownStyle::Toolbox );
}


SvtFileDialogURLSelector::~SvtFileDialogURLSelector()
{
    disposeOnce();
}

void SvtFileDialogURLSelector::dispose()
{
    delete m_pMenu;
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
    :SvtFileDialogURLSelector( pParent, pDlg, nBits, IMG_FILEDLG_BTN_UP )
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
        OUString aParentURL(aObject.GetMainURL(INetURLObject::NO_DECODE));

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
            _pMenu->SetItemText( --nItemId, SvtSimpleResId(STR_SVT_MIMETYPE_CNT_FSYSBOX).toString() );
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
        DBG_ASSERT( nId <= _aURLs.size(), "SvtUpButton_Impl:falscher Index" );

        GetDialogParent()->OpenURL_Impl(_aURLs[nId]);
    }
}

void SvtUpButton_Impl::Click()
{
    GetDialogParent()->PrevLevel_Impl();
}

Size SvtUpButton_Impl::GetOptimalSize() const
{
    return LogicToPixel(Size(12, 12), MAP_APPFONT);
}

// SvtExpFileDlg_Impl
SvtExpFileDlg_Impl::SvtExpFileDlg_Impl( WinBits )   :

    _pCurFilter         ( NULL ),
    _pUserFilter        ( NULL ),
    _pFtFileName        ( NULL ),
    _pEdFileName        ( NULL ),
    _pFtFileVersion     ( NULL ),
    _pLbFileVersion     ( NULL ),
    _pFtTemplates       ( NULL ),
    _pLbTemplates       ( NULL ),
    _pFtImageTemplates  ( NULL ),
    _pLbImageTemplates  ( NULL ),
    _pFtFileType        ( NULL ),
    _pLbFilter          ( NULL ),
    _pBtnFileOpen       ( NULL ),
    _pBtnCancel         ( NULL ),
    _pBtnHelp           ( NULL ),
    _pBtnUp             ( NULL ),
    _pBtnNewFolder      ( NULL ),
    _pCbPassword        ( NULL ),
    _pEdCurrentPath     ( NULL ),
    _pCbAutoExtension   ( NULL ),
    _pCbOptions         ( NULL ),
    _pPlaces            ( NULL ),
    _pBtnConnectToServer( NULL ),
    _eMode              ( FILEDLG_MODE_OPEN ),
    _eDlgType           ( FILEDLG_TYPE_FILEDLG ),
    _nState             ( FILEDLG_STATE_REMOTE ),
    _nStyle             ( 0 ),
    _bDoubleClick       ( false ),
    m_bNeedDelayedFilterExecute ( false ),
    _pDefaultFilter     ( NULL ),
    _bMultiSelection    ( false ),
    _bFolderHasOpened   ( false )
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

void SvtExpFileDlg_Impl::ClearFilterList( )
{
    _pLbFilter->Clear();
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
    ClearFilterList( );

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

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
// #97148# ---------------
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
#include <svl/svl.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::utl;

// some stuff for easier changes for SvtViewOptions
static const sal_Char*      pViewOptDataName = "dialog data";
#define VIEWOPT_DATANAME    ::rtl::OUString::createFromAscii( pViewOptDataName )

static inline void SetViewOptUserItem( SvtViewOptions& rOpt, const String& rData )
{
    rOpt.SetUserItem( VIEWOPT_DATANAME, makeAny( ::rtl::OUString( rData ) ) );
}

static inline String GetViewOptUserItem( const SvtViewOptions& rOpt )
{
    Any aAny( rOpt.GetUserItem( VIEWOPT_DATANAME ) );
    ::rtl::OUString aUserData;
    aAny >>= aUserData;

    return String( aUserData );
}

//*****************************************************************************
// ResMgrHolder / SvtSimpleResId
//*****************************************************************************
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

//*****************************************************************************
// SvtFileDialogFilter_Impl
//*****************************************************************************

DBG_NAME( SvtFileDialogFilter_Impl )
SvtFileDialogFilter_Impl::SvtFileDialogFilter_Impl( const String& rName, const String& rType )
    :m_aName( rName )
    ,m_aType( rType )
{
    DBG_CTOR( SvtFileDialogFilter_Impl, NULL );

    m_aType.ToLowerAscii();
}

//*****************************************************************************

SvtFileDialogFilter_Impl::~SvtFileDialogFilter_Impl()
{
    DBG_DTOR( SvtFileDialogFilter_Impl, NULL );
}

//*****************************************************************************
// SvtFileDialogFilterList_Impl
//*****************************************************************************

//=============================================================================
//= SvtFileDialogURLSelector
//=============================================================================

//-----------------------------------------------------------------------------
SvtFileDialogURLSelector::SvtFileDialogURLSelector( SvtFileDialog* _pParent, const ResId& _rResId, sal_uInt16 _nButtonId )
    :MenuButton ( _pParent, _rResId )
    ,m_pParent  ( _pParent )
    ,m_pMenu    ( new PopupMenu )
{
    SetStyle( GetStyle() | WB_NOPOINTERFOCUS | WB_RECTSTYLE | WB_SMALLSTYLE );
    SetModeImage( m_pParent->GetButtonImage( _nButtonId ) );
    SetMenuMode( MENUBUTTON_MENUMODE_TIMED );
    SetDropDown( PUSHBUTTON_DROPDOWN_TOOLBOX );
}

//-----------------------------------------------------------------------------
SvtFileDialogURLSelector::~SvtFileDialogURLSelector()
{
    delete m_pMenu;
}

//-----------------------------------------------------------------------------
void SvtFileDialogURLSelector::Activate()
{
    m_pMenu->Clear();

    FillURLMenu( m_pMenu );

    SetPopupMenu( m_pMenu );
}

//=============================================================================
//= SvtUpButton_Impl
//=============================================================================

//-----------------------------------------------------------------------------
SvtUpButton_Impl::SvtUpButton_Impl( SvtFileDialog* pParent, const ResId& rResId )
    :SvtFileDialogURLSelector( pParent, rResId, IMG_FILEDLG_BTN_UP )
{
}

//-----------------------------------------------------------------------------
SvtUpButton_Impl::~SvtUpButton_Impl()
{
}

//-----------------------------------------------------------------------------
void SvtUpButton_Impl::FillURLMenu( PopupMenu* _pMenu )
{
    SvtFileView* pBox = GetDialogParent()->GetView();

    sal_uInt16 nItemId = 1;

    _aURLs.clear();

    // determine parent levels
    INetURLObject aObject( pBox->GetViewURL() );
    sal_Int32 nCount = aObject.getSegmentCount();

    ::svtools::VolumeInfo aVolInfo( sal_True /* volume */, sal_False /* remote */,
                                    sal_False /* removable */, sal_False /* floppy */,
                                    sal_False /* compact disk */ );
    Image aVolumeImage( SvFileInformationManager::GetFolderImage( aVolInfo ) );

    while ( nCount >= 1 )
    {
        aObject.removeSegment();
        String aParentURL(aObject.GetMainURL(INetURLObject::NO_DECODE));

        if (GetDialogParent()->isUrlAllowed(aParentURL))
        {
            String aTitle;
            // 97148# --------------------------------
            if (!GetDialogParent()->ContentGetTitle(aParentURL, aTitle) || aTitle.Len() == 0)
                aTitle = aObject.getName();

            Image aImage = ( nCount > 1 ) // if nCount == 1 means workplace, which detects the wrong image
                ? SvFileInformationManager::GetImage( aObject ) : aVolumeImage;

            _pMenu->InsertItem( nItemId++, aTitle, aImage );
            _aURLs.push_back(aParentURL);

            if ( nCount == 1 )
            {
                // adjust the title of the top level entry (the workspace)
                _pMenu->SetItemText( --nItemId, SvtSimpleResId( STR_SVT_MIMETYPE_CNT_FSYSBOX ) );
            }
        }

        --nCount;
    }
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void SvtUpButton_Impl::Click()
{
    GetDialogParent()->PrevLevel_Impl();
}

//*****************************************************************************
// SvtExpFileDlg_Impl
//*****************************************************************************

SvtExpFileDlg_Impl::SvtExpFileDlg_Impl( WinBits )   :

    _pLbFilter          ( NULL ),
    _pCurFilter         ( NULL ),
    _pFilter            ( new SvtFileDialogFilterList_Impl() ),
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
    _nState             ( FILEDLG_STATE_REMOTE ),
    _nStyle             ( 0 ),
    _bDoubleClick       ( sal_False ),
    m_bNeedDelayedFilterExecute ( sal_False ),
    _pDefaultFilter     ( NULL ),
    _bMultiSelection    ( sal_False ),
    _nFixDeltaHeight    ( 0 ),
    _bFolderHasOpened   ( sal_False )
{
}

//*****************************************************************************

SvtExpFileDlg_Impl::~SvtExpFileDlg_Impl()
{
    delete _pEdCurrentPath;
    delete _pCbPassword;
    delete _pCbAutoExtension;
    delete _pCbOptions;
    delete _pBtnNewFolder;
    delete _pBtnUp;
    delete _pBtnHelp;
    delete _pBtnCancel;
    delete _pBtnFileOpen;
    delete _pLbFilter;
    delete _pFtFileType;
    delete _pLbFileVersion;
    delete _pFtFileVersion;
    delete _pFtTemplates;
    delete _pLbTemplates;
    delete _pFtImageTemplates;
    delete _pLbImageTemplates;
    delete _pEdFileName;
    delete _pFtFileName;
    delete _pUserFilter;
    delete _pFilter;
    delete _pPlaces;
    delete _pBtnConnectToServer;
}

//*****************************************************************************

void SvtExpFileDlg_Impl::SetStandardDir( const String& _rDir )
{
    _aStdDir = _rDir;
    if ( 0 == _aStdDir.Len() )
        _aStdDir.AssignAscii( "file:///" );
}

//*****************************************************************************
#if defined DBG_UTIL
//-----------------------------------------------------------------------------
namespace {
    String lcl_DecoratedFilter( const String& _rOriginalFilter )
    {
        rtl::OUStringBuffer aDecoratedFilter;
        aDecoratedFilter.append('<');
        aDecoratedFilter.append(_rOriginalFilter);
        aDecoratedFilter.append('>');
        return aDecoratedFilter.makeStringAndClear();
    }
}
#endif
//-----------------------------------------------------------------------------

void SvtExpFileDlg_Impl::ClearFilterList( )
{
    _pLbFilter->Clear();
}

//-----------------------------------------------------------------------------
void SvtExpFileDlg_Impl::SetCurFilter( SvtFileDialogFilter_Impl* pFilter, const String& rDisplayName )
{
    DBG_ASSERT( pFilter, "SvtExpFileDlg_Impl::SetCurFilter: invalid filter!" );
    DBG_ASSERT( ( rDisplayName == pFilter->GetName() )
            ||  ( rDisplayName == lcl_DecoratedFilter( pFilter->GetName() ) ),
            "SvtExpFileDlg_Impl::SetCurFilter: arguments are inconsistent!" );

    _pCurFilter = pFilter;
    m_sCurrentFilterDisplayName = rDisplayName;
}

//-----------------------------------------------------------------------------
void SvtExpFileDlg_Impl::InsertFilterListEntry( const SvtFileDialogFilter_Impl* _pFilterDesc )
{
    String sName = _pFilterDesc->GetName();
    if ( _pFilterDesc->isGroupSeparator() )
        sName = rtl::OUString( "------------------------------------------" );
    else
        sName = _pFilterDesc->GetName();

    // insert an set user data
    sal_uInt16 nPos = _pLbFilter->InsertEntry( sName );
    _pLbFilter->SetEntryData( nPos, const_cast< void* >( static_cast< const void* >( _pFilterDesc ) ) );
}

//-----------------------------------------------------------------------------

void SvtExpFileDlg_Impl::InitFilterList( )
{
    // clear the current list
    ClearFilterList( );

    // reinit it
    sal_uInt16 nPos = _pFilter->size();

    // search for the first entry which is no group separator
    while ( nPos-- && (*_pFilter)[ nPos ].isGroupSeparator() )
        ;

    // add all following entries
    while ( (sal_Int16)nPos >= 0 )
        InsertFilterListEntry( &(*_pFilter)[ nPos-- ] );
}

//-----------------------------------------------------------------------------

void SvtExpFileDlg_Impl::CreateFilterListControl( Window* _pParent, const ResId& _rId )
{
    DBG_ASSERT( !_pLbFilter, "SvtExpFileDlg_Impl::CreateFilterListControl: already created the control!" );
    if ( !_pLbFilter )
    {
        _pLbFilter = new ListBox( _pParent, _rId );
        _pLbFilter->SetDropDownLineCount( 10 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

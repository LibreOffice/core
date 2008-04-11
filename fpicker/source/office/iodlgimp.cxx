/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iodlgimp.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_fpicker.hxx"

// includes *******************************************************************

#include "iodlgimp.hxx"
#include "svtools/headbar.hxx"
#include <tools/debug.hxx>
#include <tools/wldcrd.hxx>
#include <tools/urlobj.hxx>
#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
// #97148# ---------------
#include "svtools/ctypeitm.hxx"
#include "svtools/eitem.hxx"
#include "svtools/viewoptions.hxx"
#include "svtools/fileview.hxx"
#include "svtools/inettbc.hxx"
#include "iodlg.hxx"
#ifndef _SVTOOLS_IODLGIMPL_HRC
#include "iodlg.hrc"
#endif
#include "svtools/imagemgr.hxx"
#include <unotools/localfilehelper.hxx>
#include "svtools/useroptions.hxx"
#ifndef _RTL_INSTANCE_HXX_
#include "rtl/instance.hxx"
#endif

#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#include "svtools/svstdarr.hxx"

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


// defines f"ur den Style der BrowseBox

#define STYLE_MULTI_SELECTION   \
    CNTVIEWSTYLE_NODE_BUTTONS | \
    CNTVIEWSTYLE_NODE_BUTTONS_AT_ROOT | \
    CNTVIEWSTYLE_SHOW_MESSAGES | \
    CNTVIEWSTYLE_SHOW_FOLDERS | \
    CNTVIEWSTYLE_NO_SMARTHIGHLIGHT | \
    CNTVIEWSTYLE_HIDE_OPENMENU | \
    CNTVIEWSTYLE_DEFAULT_APPEARANCE | \
    CNTVIEWSTYLE_SORT_BY_FOLDER

#define STYLE_SINGLE_SELECTION  \
    STYLE_MULTI_SELECTION | CNTVIEWSTYLE_SINGLE_SELECTION

#define BOOL_NOT_INITIALIZE     ((sal_Bool)2)

//*****************************************************************************
// ResMgrHolder / SvtSimpleResId
//*****************************************************************************
namespace
{
    struct ResMgrHolder
    {
        ResMgr * operator ()()
        {
            return ResMgr::CreateResMgr (CREATEVERSIONRESMGR_NAME(svs));
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
        SvtSimpleResId (USHORT nId) : ResId (nId, *ResMgrHolder::getOrCreate()) {}
    };
}

//*****************************************************************************

String GetRegularExpression_Impl( const String& rFilter )
{
    String aFilter = rFilter;
    aFilter.EraseLeadingChars().EraseTrailingChars();
    String aRegExp = '^';

    for ( const sal_Unicode *pc = aFilter.GetBuffer(); *pc; ++pc )
    {
        if ( '*' == *pc )
            aRegExp += String(RTL_CONSTASCII_USTRINGPARAM(".*"));
        else if ( '?' == *pc )
            aRegExp += '.';
        else
        {
            aRegExp += '\'';

            while ( *pc && *pc != '*' && *pc != '?' )
            {
                if ( '\'' == *pc )
                    aRegExp += String(RTL_CONSTASCII_USTRINGPARAM("\\\'"));
                else
                    aRegExp += *pc;
                pc++;
            }
            aRegExp += '\'';

            if ( !*pc )
                break;
            else
                pc--;
        }
    }
    aRegExp += '$';
    return aRegExp;
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

SV_IMPL_PTRARR( SvtFileDialogFilterList_Impl, SvtFileDialogFilter_Impl* );

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
void SvtFileDialogURLSelector::OpenURL( const String& rURL )
{
    INetURLObject aObj( rURL );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "SvtFileDialogURLSelector::OpenURL: Invalid URL!" );
    m_pParent->OpenURL_Impl( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
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
    ,_pURLs                  ( NULL )
{
}

//-----------------------------------------------------------------------------
SvtUpButton_Impl::~SvtUpButton_Impl()
{
    delete _pURLs;
}

//-----------------------------------------------------------------------------
void SvtUpButton_Impl::FillURLMenu( PopupMenu* _pMenu )
{
    SvtFileView* pBox = GetDialogParent()->GetView();

    sal_uInt16 nItemId = 1;

    delete _pURLs;
    _pURLs = new SvStringsDtor;

    // "Ubergeordnete Ebenen bestimmen.
    INetURLObject aObject( pBox->GetViewURL() );
    sal_Int32 nCount = aObject.getSegmentCount();

    ::svtools::VolumeInfo aVolInfo( sal_True /* volume */, sal_False /* remote */,
                                    sal_False /* removable */, sal_False /* floppy */,
                                    sal_False /* compact disk */ );
    sal_Bool bIsHighContrast = pBox->GetDisplayBackground().GetColor().IsDark();
    Image aVolumeImage( SvFileInformationManager::GetFolderImage( aVolInfo, bIsHighContrast ) );

    while ( nCount >= 1 )
    {
        aObject.removeSegment();
        String* pParentURL = new String( aObject.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( GetDialogParent()->isUrlAllowed( *pParentURL ) )
        {
            String aTitle;
            // 97148# --------------------------------
            if ( !GetDialogParent()->ContentGetTitle( *pParentURL, aTitle ) || aTitle.Len() == 0 )
                aTitle = aObject.getName();

            Image aImage = ( nCount > 1 ) // if nCount == 1 means workplace, which detects the wrong image
                ? SvFileInformationManager::GetImage( aObject, bIsHighContrast )
                : aVolumeImage;

            _pMenu->InsertItem( nItemId++, aTitle, aImage );
            _pURLs->Insert( pParentURL, _pURLs->Count() );

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
        DBG_ASSERT( nId <= _pURLs->Count(), "SvtUpButton_Impl:falscher Index" );

        String aURL = *(_pURLs->GetObject( nId ));
        GetDialogParent()->OpenURL_Impl( aURL );
    }
}

//-----------------------------------------------------------------------------
void SvtUpButton_Impl::Click()
{
    GetDialogParent()->PrevLevel_Impl();
}

//=============================================================================
//= SvtTravelButton_Impl
//=============================================================================

//-----------------------------------------------------------------------------
SvtTravelButton_Impl::SvtTravelButton_Impl( SvtFileDialog* pParent, const ResId& rResId )
    :SvtFileDialogURLSelector   ( pParent, rResId, IMG_FILEDLG_BTN_STD )
{
    SetDropDown( 0 );   // by default, don't drop down, as we don't have favourites
}

//-----------------------------------------------------------------------------
void SvtTravelButton_Impl::SetFavouriteLocations( const ::std::vector< String >& _rLocations )
{
    m_aFavourites = _rLocations;
    // enable the drop down if and only if we have favourites
    SetDropDown( m_aFavourites.empty() ? 0 : PUSHBUTTON_DROPDOWN_TOOLBOX );
}

//-----------------------------------------------------------------------------
SvtTravelButton_Impl::~SvtTravelButton_Impl()
{
}

//-----------------------------------------------------------------------------
void SvtTravelButton_Impl::FillURLMenu( PopupMenu* _pMenu )
{
    if ( m_aFavourites.empty() )
        // though we claimed that we do not want to have a drop down button
        // in this case, VCL nevertheless behaves as if we had one .... :(
        return;

    _pMenu->Clear();

    sal_Bool bIsHighContrast = GetDialogParent()->GetView()->GetDisplayBackground().GetColor().IsDark();

    USHORT nItemId = 1;
    String sDisplayName;

    ::std::vector< String >::const_iterator aLoop;
    for ( aLoop = m_aFavourites.begin(); aLoop != m_aFavourites.end(); ++aLoop, ++nItemId )
    {
        if ( GetDialogParent()->isUrlAllowed( *aLoop ) )
        {
            Image aImage = SvFileInformationManager::GetImage(
                INetURLObject(*aLoop), bIsHighContrast );
            if ( LocalFileHelper::ConvertURLToSystemPath(*aLoop, sDisplayName) )
                _pMenu->InsertItem( nItemId, sDisplayName, aImage );
            else
                _pMenu->InsertItem( nItemId, *aLoop, aImage );
        }
    }
}

//-----------------------------------------------------------------------------
void SvtTravelButton_Impl::Select()
{
    sal_uInt16 nId = GetCurItemId();
    if ( nId )
    {
        --nId;
        DBG_ASSERT( nId < m_aFavourites.size(), "SvtTravelButton_Impl::Select: invalid index!" );
        if ( nId < m_aFavourites.size() )
            OpenURL( m_aFavourites[ nId ] );
    }
}

//-----------------------------------------------------------------------------
void SvtTravelButton_Impl::Click()
{
    OpenURL( GetDialogParent()->GetStandardDir() );
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
    _pBtnStandard       ( NULL ),
    _pCbPassword        ( NULL ),
    _pFtCurrentPath     ( NULL ),
    _pCbAutoExtension   ( NULL ),
    _pCbOptions         ( NULL ),
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
    delete _pFtCurrentPath;
    delete _pCbPassword;
    delete _pCbAutoExtension;
    delete _pCbOptions;
    delete _pBtnStandard;
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
}

//*****************************************************************************

void SvtExpFileDlg_Impl::SetStandardDir( const String& _rDir )
{
    _aStdDir = _rDir;
    if ( 0 == _aStdDir.Len() )
        _aStdDir.AssignAscii( "file:///" );
}

//*****************************************************************************
#if OSL_DEBUG_LEVEL > 0
//-----------------------------------------------------------------------------
namespace {
    String lcl_DecoratedFilter( const String& _rOriginalFilter )
    {
        String aDecoratedFilter = '<';
        aDecoratedFilter += _rOriginalFilter;
        aDecoratedFilter += '>';
        return aDecoratedFilter;
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
        sName = String::CreateFromAscii( "------------------------------------------" );
    else
        sName = _pFilterDesc->GetName();

    // insert an set user data
    USHORT nPos = _pLbFilter->InsertEntry( sName );
    _pLbFilter->SetEntryData( nPos, const_cast< void* >( static_cast< const void* >( _pFilterDesc ) ) );
}

//-----------------------------------------------------------------------------

void SvtExpFileDlg_Impl::InitFilterList( )
{
    // clear the current list
    ClearFilterList( );

    // reinit it
    USHORT nPos = _pFilter->Count();

    // search for the first entry which is no group separator
    while ( nPos-- && _pFilter->GetObject( nPos ) && _pFilter->GetObject( nPos )->isGroupSeparator() )
        ;

    // add all following entries
    while ( (sal_Int16)nPos >= 0 )
        InsertFilterListEntry( _pFilter->GetObject( nPos-- ) );
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

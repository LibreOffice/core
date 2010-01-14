/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbregister.cxx,v $
 * $Revision: 1.16 $
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
#include "precompiled_cui.hxx"

#include "dbregister.hxx"
#include "dbregister.hrc"
#include "dbregistersettings.hxx"
#include "connpooloptions.hxx"
#include <svl/filenotation.hxx>
#include <cuires.hrc>
#include "helpid.hrc"
#include <svtools/editbrowsebox.hxx>
#include <cuires.hrc>
#include <vcl/field.hxx>
#include <svl/eitem.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <vcl/msgbox.hxx>
#include <svtools/svtabbx.hxx>
#include <svl/itemset.hxx>
#include "doclinkdialog.hxx"
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include "optHeaderTabListbox.hxx"
#include <sfx2/docfilt.hxx>
#include <dialmgr.hxx>
#include "dbregisterednamesconfig.hxx"
#include <svx/dialogs.hrc>

#define TAB_WIDTH1      80
#define TAB_WIDTH_MIN   10
#define TAB_WIDTH2      1000
#define ITEMID_TYPE       1
#define ITEMID_PATH       2
//........................................................................
namespace svx
{
//........................................................................

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::svt;

// class RegistrationItemSetHolder  -------------------------------------------------

RegistrationItemSetHolder::RegistrationItemSetHolder( const SfxItemSet& _rMasterSet )
    :m_aRegistrationItems( _rMasterSet )
{
    DbRegisteredNamesConfig::GetOptions( m_aRegistrationItems );
}

RegistrationItemSetHolder::~RegistrationItemSetHolder()
{
}

// class DatabaseRegistrationDialog  ------------------------------------------------

DatabaseRegistrationDialog::DatabaseRegistrationDialog( Window* pParent, const SfxItemSet& rInAttrs )
    :RegistrationItemSetHolder( rInAttrs )
    ,SfxSingleTabDialog( pParent, getRegistrationItems(), RID_SFXPAGE_DBREGISTER )
{
    SfxTabPage* page = DbRegistrationOptionsPage::Create( this, getRegistrationItems() );

    SetTabPage( page );
    SetText( page->GetText() );
}

DatabaseRegistrationDialog::~DatabaseRegistrationDialog()
{
}

short DatabaseRegistrationDialog::Execute()
{
    short result = SfxSingleTabDialog::Execute();
    if ( result == RET_OK )
    {
        DBG_ASSERT( GetOutputItemSet(), "DatabaseRegistrationDialog::Execute: no output items!" );
        if ( GetOutputItemSet() )
            DbRegisteredNamesConfig::SetOptions( *GetOutputItemSet() );
    }
    return result;
}

// class DbRegistrationOptionsPage --------------------------------------------------

DbRegistrationOptionsPage::DbRegistrationOptionsPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, CUI_RES( RID_SFXPAGE_DBREGISTER), rSet ),

    aTypeText       ( this, CUI_RES( FT_TYPE ) ),
    aPathText       ( this, CUI_RES( FT_PATH ) ),
    aPathCtrl       ( this, CUI_RES( LB_PATH ) ),
    m_aNew          ( this, CUI_RES( BTN_NEW ) ),
    m_aEdit         ( this, CUI_RES( BTN_EDIT ) ),
    m_aDelete       ( this, CUI_RES( BTN_DELETE ) ),
    aStdBox         ( this, CUI_RES( GB_STD ) ),
    pHeaderBar      ( NULL ),
    pPathBox        ( NULL ),
    m_pCurEntry     ( NULL ),
    m_nOldCount     ( 0 ),
    m_bModified     ( FALSE )
{
    m_aNew.SetClickHdl( LINK( this, DbRegistrationOptionsPage, NewHdl ) );
    m_aEdit.SetClickHdl( LINK( this, DbRegistrationOptionsPage, EditHdl ) );
    m_aDelete.SetClickHdl( LINK( this, DbRegistrationOptionsPage, DeleteHdl ) );

    Size aBoxSize = aPathCtrl.GetOutputSizePixel();


    WinBits nBits = WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP;
    pPathBox = new ::svx::OptHeaderTabListBox( &aPathCtrl, nBits );

    pHeaderBar = new HeaderBar( &aPathCtrl, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    pHeaderBar->SetPosSizePixel( Point( 0, 0 ), Size( aBoxSize.Width(), 16 ) );
    pHeaderBar->SetSelectHdl( LINK( this, DbRegistrationOptionsPage, HeaderSelect_Impl ) );
    pHeaderBar->SetEndDragHdl( LINK( this, DbRegistrationOptionsPage, HeaderEndDrag_Impl ) );
    Size aSz;
    aSz.Width() = TAB_WIDTH1;
    pHeaderBar->InsertItem( ITEMID_TYPE, aTypeText.GetText(),
                            LogicToPixel( aSz, MapMode( MAP_APPFONT ) ).Width(),
                            HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW );
    aSz.Width() = TAB_WIDTH2;
    pHeaderBar->InsertItem( ITEMID_PATH, aPathText.GetText(),
                            LogicToPixel( aSz, MapMode( MAP_APPFONT ) ).Width(),
                            HIB_LEFT | HIB_VCENTER );

    static long nTabs[] = {3, 0, TAB_WIDTH1, TAB_WIDTH1 + TAB_WIDTH2 };
    Size aHeadSize = pHeaderBar->GetSizePixel();

    aPathCtrl.SetFocusControl( pPathBox );
    pPathBox->SetWindowBits( nBits );
    pPathBox->SetDoubleClickHdl( LINK( this, DbRegistrationOptionsPage, EditHdl ) );
    pPathBox->SetSelectHdl( LINK( this, DbRegistrationOptionsPage, PathSelect_Impl ) );
    pPathBox->SetSelectionMode( SINGLE_SELECTION );
    pPathBox->SetPosSizePixel( Point( 0, aHeadSize.Height() ),
                               Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    pPathBox->SetTabs( &nTabs[0], MAP_APPFONT );
    pPathBox->InitHeaderBar( pHeaderBar );
    pPathBox->SetHighlightRange();

    pPathBox->SetHelpId( HID_DBPATH_CTL_PATH );
    pHeaderBar->SetHelpId( HID_DBPATH_HEADERBAR );

    pPathBox->Show();
    pHeaderBar->Show();

    FreeResource();
}

// -----------------------------------------------------------------------

DbRegistrationOptionsPage::~DbRegistrationOptionsPage()
{
    // #110603# do not grab focus to a destroyed window !!!
    aPathCtrl.SetFocusControl( NULL );

    pHeaderBar->Hide();
    for ( USHORT i = 0; i < pPathBox->GetEntryCount(); ++i )
        delete static_cast<String*>(pPathBox->GetEntry(i)->GetUserData());
    delete pPathBox;
    delete pHeaderBar;
}

// -----------------------------------------------------------------------

SfxTabPage* DbRegistrationOptionsPage::Create( Window* pParent,
                                    const SfxItemSet& rAttrSet )
{
    return ( new DbRegistrationOptionsPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

BOOL DbRegistrationOptionsPage::FillItemSet( SfxItemSet& rCoreSet )
{
    // the settings for the single drivers
    sal_Bool bModified = sal_False;
    TNameLocationMap aMap;
    ULONG nCount = pPathBox->GetEntryCount();
    for ( ULONG i = 0; i < nCount; ++i )
    {
        SvLBoxEntry* pEntry = pPathBox->GetEntry(i);
        String* pPath = static_cast<String*>(pEntry->GetUserData());
        if ( pPath && pPath->Len() )
        {
            OFileNotation aTransformer(*pPath);
            aMap.insert(TNameLocationMap::value_type(::rtl::OUString(pPathBox->GetEntryText(pEntry,0)),aTransformer.get(OFileNotation::N_URL)));
        }
    }
    if ( m_nOldCount != aMap.size() || m_bModified )
    {
        rCoreSet.Put(DatabaseMapItem(SID_SB_DB_REGISTER, aMap), SID_SB_DB_REGISTER);
        bModified = sal_True;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void DbRegistrationOptionsPage::Reset( const SfxItemSet& rSet )
{
    // the settings for the single drivers
    SFX_ITEMSET_GET( rSet, pSettings, DatabaseMapItem, SID_SB_DB_REGISTER, sal_True );

    if ( pSettings )
    {
        // TabListBox f"ullen
        pPathBox->Clear();

        const TNameLocationMap& rMap = pSettings->getSettings();
        m_nOldCount = rMap.size();
        TNameLocationMap::const_iterator aIter = rMap.begin();
        TNameLocationMap::const_iterator aEnd = rMap.end();
        for (; aIter != aEnd; ++aIter)
        {
            OFileNotation aTransformer(aIter->second);
            insertNewEntry(aIter->first,aTransformer.get(OFileNotation::N_SYSTEM));
        }

        String aUserData = GetUserData();

        if ( aUserData.Len() )
        {
            // Spaltenbreite restaurieren
            pHeaderBar->SetItemSize( ITEMID_TYPE, aUserData.GetToken(0).ToInt32() );
            HeaderEndDrag_Impl( NULL );
            // Sortierrichtung restaurieren
            BOOL bUp = (BOOL)(USHORT)aUserData.GetToken(1).ToInt32();
            HeaderBarItemBits nBits = pHeaderBar->GetItemBits(ITEMID_TYPE);

            if ( bUp )
            {
                nBits &= ~HIB_UPARROW;
                nBits |= HIB_DOWNARROW;
            }
            else
            {
                nBits &= ~HIB_DOWNARROW;
                nBits |= HIB_UPARROW;
            }
            pHeaderBar->SetItemBits( ITEMID_TYPE, nBits );
            HeaderSelect_Impl( NULL );
        }
    }
}

// -----------------------------------------------------------------------

void DbRegistrationOptionsPage::FillUserData()
{
    String aUserData = String::CreateFromInt32( pHeaderBar->GetItemSize( ITEMID_TYPE ) );
    aUserData += ';';
    HeaderBarItemBits nBits = pHeaderBar->GetItemBits( ITEMID_TYPE );
    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    aUserData += bUp ? '1' : '0';
    SetUserData( aUserData );
}
// -----------------------------------------------------------------------

IMPL_LINK( DbRegistrationOptionsPage, DeleteHdl, void *, EMPTYARG )
{
    SvLBoxEntry* pEntry = pPathBox->FirstSelected();
    if ( pEntry )
    {
        QueryBox aQuery(this,CUI_RES(QUERY_DELETE_CONFIRM));
        if ( aQuery.Execute() == RET_YES )
            pPathBox->GetModel()->Remove(pEntry);
    }
    return 0;
}
// -----------------------------------------------------------------------
IMPL_LINK( DbRegistrationOptionsPage, NewHdl, void *, EMPTYARG )
{
    String sNewName,sNewLocation;
    openLinkDialog(sNewName,sNewLocation);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( DbRegistrationOptionsPage, EditHdl, void *, EMPTYARG )
{
    SvLBoxEntry* pEntry = pPathBox->GetCurEntry();
    if ( pEntry )
    {
        String* pOldLocation = static_cast<String*>(pEntry->GetUserData());
        String sOldName = pPathBox->GetEntryText(pEntry,0);
        m_pCurEntry = pEntry;
        openLinkDialog(sOldName,*pOldLocation,pEntry);
        m_pCurEntry = NULL;
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( DbRegistrationOptionsPage, HeaderSelect_Impl, HeaderBar*, pBar )
{
    if ( pBar && pBar->GetCurItemId() != ITEMID_TYPE )
        return 0;

    HeaderBarItemBits nBits = pHeaderBar->GetItemBits(ITEMID_TYPE);
    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    SvSortMode eMode = SortAscending;

    if ( bUp )
    {
        nBits &= ~HIB_UPARROW;
        nBits |= HIB_DOWNARROW;
        eMode = SortDescending;
    }
    else
    {
        nBits &= ~HIB_DOWNARROW;
        nBits |= HIB_UPARROW;
    }
    pHeaderBar->SetItemBits( ITEMID_TYPE, nBits );
    SvTreeList* pModel = pPathBox->GetModel();
    pModel->SetSortMode( eMode );
    pModel->Resort();
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( DbRegistrationOptionsPage, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    if ( pBar && !pBar->GetCurItemId() )
        return 0;

    if ( !pHeaderBar->IsItemMode() )
    {
        Size aSz;
        USHORT nTabs = pHeaderBar->GetItemCount();
        long nTmpSz = 0;
        long nWidth = pHeaderBar->GetItemSize(ITEMID_TYPE);
        long nBarWidth = pHeaderBar->GetSizePixel().Width();

        if(nWidth < TAB_WIDTH_MIN)
            pHeaderBar->SetItemSize( ITEMID_TYPE, TAB_WIDTH_MIN);
        else if ( ( nBarWidth - nWidth ) < TAB_WIDTH_MIN )
            pHeaderBar->SetItemSize( ITEMID_TYPE, nBarWidth - TAB_WIDTH_MIN );

        for ( USHORT i = 1; i <= nTabs; ++i )
        {
            long _nWidth = pHeaderBar->GetItemSize(i);
            aSz.Width() =  _nWidth + nTmpSz;
            nTmpSz += _nWidth;
            pPathBox->SetTab( i, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
        }
    }
    return 1;
}
// -----------------------------------------------------------------------

IMPL_LINK( DbRegistrationOptionsPage, PathSelect_Impl, SvTabListBox *, EMPTYARG )

/*  [Beschreibung]

*/

{
    SvLBoxEntry* pEntry = pPathBox->FirstSelected();

    m_aEdit.Enable( pEntry != NULL);
    m_aDelete.Enable( pEntry != NULL);
    return 0;
}
// -----------------------------------------------------------------------------
void DbRegistrationOptionsPage::insertNewEntry(const ::rtl::OUString& _sName,const ::rtl::OUString& _sLocation)
{
    String aStr( _sName );
    aStr += '\t';
    aStr += String(_sLocation);
    SvLBoxEntry* pEntry = pPathBox->InsertEntry( aStr );
    String* pLocation = new String( _sLocation );
    pEntry->SetUserData( pLocation );
}
// -----------------------------------------------------------------------------
String DbRegistrationOptionsPage::getFileLocation(const String& _sLocation)
{
    try
    {
        rtl::OUString aService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ) );
        Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
        Reference < XFilePicker > xFilePicker( xFactory->createInstance( aService ), UNO_QUERY );
        OSL_ENSURE(xFilePicker.is() ,"Could create file picker service!");
        Reference < XFilterManager> xFilterManager(xFilePicker,UNO_QUERY);
        static const String s_sDatabaseType = String::CreateFromAscii("StarOffice XML (Base)");
        const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sDatabaseType);
        if ( pFilter )
        {
            xFilterManager->appendFilter( pFilter->GetUIName(),pFilter->GetDefaultExtension());
            xFilterManager->setCurrentFilter(pFilter->GetUIName());
        }

        INetURLObject aURL( _sLocation, INET_PROT_FILE );
        xFilePicker->setMultiSelectionMode(sal_False);
        xFilePicker->setDisplayDirectory( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
        short nRet = xFilePicker->execute();

        if ( ExecutableDialogResults::OK == nRet )
        {

            // old path is an URL?
            INetURLObject aObj( _sLocation );
            FASTBOOL bURL = ( aObj.GetProtocol() != INET_PROT_NOT_VALID );
            Sequence< ::rtl::OUString > aFiles = xFilePicker->getFiles();
            INetURLObject aNewObj( aFiles[0] );
            aNewObj.removeFinalSlash();

            // then the new path also an URL else system path
            String sNewLocation = bURL ? rtl::OUString(aFiles[0]) : aNewObj.getFSysPath( INetURLObject::FSYS_DETECT );

            if (
#ifdef UNX
    // Unix is case sensitive
                                ( sNewLocation != _sLocation )
#else
                                ( sNewLocation.CompareIgnoreCaseToAscii( _sLocation ) != COMPARE_EQUAL )
#endif
            )
                return sNewLocation;
        }
    }
    catch( Exception& )
    {
        DBG_ERRORFILE( "DbRegistrationOptionsPage::EditLocationHdl: exception from folder picker" );
    }

    return String();
}
// -----------------------------------------------------------------------------
void DbRegistrationOptionsPage::openLinkDialog(const String& _sOldName,const String& _sOldLocation,SvLBoxEntry* _pEntry)
{
    ODocumentLinkDialog aDlg(this,_pEntry == NULL);

    aDlg.set(_sOldName,_sOldLocation);
    aDlg.setNameValidator(LINK( this, DbRegistrationOptionsPage, NameValidator ) );

    if ( aDlg.Execute() == RET_OK )
    {
        String sNewName,sNewLocation;
        aDlg.get(sNewName,sNewLocation);
        if ( _pEntry == NULL || sNewName != _sOldName || sNewLocation != _sOldLocation )
        {
            if ( _pEntry )
            {
                delete static_cast<String*>(_pEntry->GetUserData());
                pPathBox->GetModel()->Remove(_pEntry);
            }
            insertNewEntry(sNewName,sNewLocation);
            m_bModified = sal_True;
        }
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK( DbRegistrationOptionsPage, NameValidator, String*, _pName )
{
    if ( _pName )
    {
        ULONG nCount = pPathBox->GetEntryCount();
        for ( ULONG i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = pPathBox->GetEntry(i);
            if ( (!m_pCurEntry || m_pCurEntry != pEntry) && pPathBox->GetEntryText(pEntry,0) == *_pName )
                return 0L;
        }
    }
    return 1L;
}
//........................................................................
}   // namespace svx
//........................................................................



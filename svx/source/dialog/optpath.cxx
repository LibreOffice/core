/*************************************************************************
 *
 *  $RCSfile: optpath.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:40:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------
#pragma hdrstop
#include "svxdlg.hxx" //CHINA001

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _PICKERHELPER_HXX
#include <svtools/pickerhelper.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif
#ifndef _SVT_FILEDLG_HXX //autogen
#include <svtools/filedlg.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <svtools/defaultoptions.hxx>
#include <unotools/localfilehelper.hxx>

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#define _SVX_OPTPATH_CXX

#include "optpath.hxx"
#include "dialmgr.hxx"
//CHINA001 #include "multipat.hxx"
#include "optpath.hrc"
#include "dialogs.hrc"
#include "helpid.hrc"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef SVX_OPTHEADERTABLISTBOX_HXX
#include "optHeaderTabListbox.hxx"
#endif
#ifndef _SVX_READONLYIMAGE_HXX
#include <readonlyimage.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace svx;

// define ----------------------------------------------------------------

#define TAB_WIDTH1      80
#define TAB_WIDTH_MIN   10
#define TAB_WIDTH2      1000
#define ITEMID_TYPE     1
#define ITEMID_PATH     2

// struct OptPath_Impl ---------------------------------------------------

struct OptPath_Impl
{
    SvtDefaultOptions _aDefOpt;
    Image aLockImage;
    Image aLockImageHC;

    OptPath_Impl(const ResId& rLockRes, const ResId& rLockResHC) :
        aLockImage(rLockRes),
        aLockImageHC(rLockResHC){}
};

// struct PathUserData_Impl ----------------------------------------------

struct PathUserData_Impl
{
    USHORT          nRealId;
    SfxItemState    eState;
    String          aPathStr;

    PathUserData_Impl( USHORT nId ) :
        nRealId( nId ), eState( SFX_ITEM_UNKNOWN ) {}
};

#define MULTIPATH_DELIMITER     ';'

String Convert_Impl( const String& rValue )
{
    char cDelim = MULTIPATH_DELIMITER;
    USHORT nCount = rValue.GetTokenCount( cDelim );
    String aReturn;
    for ( USHORT i=0; i<nCount ; ++i )
    {
        String aValue = rValue.GetToken( i, cDelim );
        INetURLObject aObj( aValue );
        if ( aObj.GetProtocol() == INET_PROT_FILE )
            aReturn += aObj.PathToFileName();
        else if ( ::utl::LocalFileHelper::IsFileContent( aValue ) )
            aReturn += aObj.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
        if ( i+1 < nCount)
            aReturn += MULTIPATH_DELIMITER;
    }

    return aReturn;
}

// class SvxPathControl_Impl ---------------------------------------------

long SvxPathControl_Impl::Notify( NotifyEvent& rNEvt )
{
    long nRet = Control::Notify( rNEvt );

    if ( m_pFocusCtrl && rNEvt.GetWindow() != m_pFocusCtrl && rNEvt.GetType() == EVENT_GETFOCUS )
        m_pFocusCtrl->GrabFocus();
    return nRet;
}

// functions -------------------------------------------------------------

BOOL IsMultiPath_Impl( const USHORT nIndex )
{
    return ( SvtPathOptions::PATH_AUTOCORRECT == nIndex ||
             SvtPathOptions::PATH_AUTOTEXT == nIndex ||
             SvtPathOptions::PATH_BASIC == nIndex ||
             SvtPathOptions::PATH_GALLERY == nIndex ||
             SvtPathOptions::PATH_TEMPLATE == nIndex );
}

// ------------------------------------------------------------------------

String GetEntryURL_Impl( const USHORT nIndex, const OptHeaderTabListBox* pBox )
{
    String aURL;
    for ( USHORT i = 0; pBox && i < pBox->GetEntryCount(); ++i )
    {
        PathUserData_Impl* pUserData = (PathUserData_Impl*)pBox->GetEntry(i)->GetUserData();
        if ( nIndex == pUserData->nRealId )
        {
            INetURLObject aEntry( pUserData->aPathStr, INET_PROT_FILE );
            aEntry.setFinalSlash();
            aURL = aEntry.GetMainURL( INetURLObject::DECODE_TO_IURI );
            break;
        }
    }
    return aURL;
}

// class SvxPathTabPage --------------------------------------------------

SvxPathTabPage::SvxPathTabPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SFXPAGE_PATH ), rSet ),

    aTypeText       ( this, ResId( FT_TYPE ) ),
    aPathText       ( this, ResId( FT_PATH ) ),
    aPathCtrl       ( this, ResId( LB_PATH ) ),
    aStandardBtn    ( this, ResId( BTN_STANDARD ) ),
    aPathBtn        ( this, ResId( BTN_PATH ) ),
    aStdBox         ( this, ResId( GB_STD ) ),

    pHeaderBar      ( NULL ),
    pPathBox        ( NULL ),
    pImpl           ( new OptPath_Impl(ResId(IMG_LOCK), ResId(IMG_LOCK_HC) ))

{
    aStandardBtn.SetClickHdl( LINK( this, SvxPathTabPage, StandardHdl_Impl ) );
    Link aLink = LINK( this, SvxPathTabPage, PathHdl_Impl );
    aPathBtn.SetClickHdl( aLink );
    Size aBoxSize = aPathCtrl.GetOutputSizePixel();
    pHeaderBar = new HeaderBar( &aPathCtrl, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    pHeaderBar->SetPosSizePixel( Point( 0, 0 ), Size( aBoxSize.Width(), 16 ) );
    pHeaderBar->SetSelectHdl( LINK( this, SvxPathTabPage, HeaderSelect_Impl ) );
    pHeaderBar->SetEndDragHdl( LINK( this, SvxPathTabPage, HeaderEndDrag_Impl ) );
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

    WinBits nBits = WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP;
    pPathBox = new OptHeaderTabListBox( &aPathCtrl, nBits );
    aPathCtrl.SetFocusControl( pPathBox );
    pPathBox->SetWindowBits( nBits );
    pPathBox->SetDoubleClickHdl( aLink );
    pPathBox->SetSelectHdl( LINK( this, SvxPathTabPage, PathSelect_Impl ) );
    pPathBox->SetSelectionMode( MULTIPLE_SELECTION );
    pPathBox->SetPosSizePixel( Point( 0, aHeadSize.Height() ),
                               Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    pPathBox->SetTabs( &nTabs[0], MAP_APPFONT );
    pPathBox->InitHeaderBar( pHeaderBar );
    pPathBox->SetHighlightRange();
    pPathBox->SetHelpId( HID_OPTPATH_CTL_PATH );
    pHeaderBar->SetHelpId( HID_OPTPATH_HEADERBAR );
    pPathBox->Show();
    pHeaderBar->Show();

    FreeResource();
}

// -----------------------------------------------------------------------

SvxPathTabPage::~SvxPathTabPage()
{
    // #110603# do not grab focus to a destroyed window !!!
    aPathCtrl.SetFocusControl( NULL );

    pHeaderBar->Hide();
    for ( USHORT i = 0; i < pPathBox->GetEntryCount(); ++i )
        delete (PathUserData_Impl*)pPathBox->GetEntry(i)->GetUserData();
    delete pPathBox;
    delete pHeaderBar;
    delete pImpl;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxPathTabPage::Create( Window* pParent,
                                    const SfxItemSet& rAttrSet )
{
    return ( new SvxPathTabPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

BOOL SvxPathTabPage::FillItemSet( SfxItemSet& )
{
    SvtPathOptions aPathOpt;
    for ( USHORT i = 0; i < pPathBox->GetEntryCount(); ++i )
    {
        PathUserData_Impl* pPathImpl = (PathUserData_Impl*)pPathBox->GetEntry(i)->GetUserData();
        USHORT nRealId = pPathImpl->nRealId;
        if(pPathImpl->eState == SFX_ITEM_SET)
        {
            String sPath(pPathImpl->aPathStr);
            if(nRealId == SvtPathOptions::PATH_ADDIN ||
                    nRealId == SvtPathOptions::PATH_FILTER ||
                    nRealId == SvtPathOptions::PATH_HELP  ||
                    nRealId == SvtPathOptions::PATH_MODULE  ||
                    nRealId == SvtPathOptions::PATH_PLUGIN  ||
                    nRealId == SvtPathOptions::PATH_STORAGE  )
                ::utl::LocalFileHelper::ConvertURLToPhysicalName( pPathImpl->aPathStr, sPath );
            aPathOpt.SetPath(SvtPathOptions::Pathes(nRealId), sPath);
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------

void SvxPathTabPage::Reset( const SfxItemSet& )
{
    pPathBox->Clear();
    SvtPathOptions aPathOpt;

    for( USHORT i = 0; i <= (USHORT)SvtPathOptions::PATH_WORK; ++i )
        switch(i)
        {
            case SvtPathOptions::PATH_CONFIG:
            case SvtPathOptions::PATH_FAVORITES:
            case SvtPathOptions::PATH_HELP:
            case SvtPathOptions::PATH_MODULE:
            case SvtPathOptions::PATH_STORAGE:
            /* #i29834# */
            case SvtPathOptions::PATH_ADDIN:
            case SvtPathOptions::PATH_PLUGIN:
            case SvtPathOptions::PATH_FILTER:
            /* #i29834# */
            break;
            default:
            {
                String aStr( SVX_RES(RID_SVXSTR_PATH_NAME_START + i));
                String sTmpPath(aPathOpt.GetPath(SvtPathOptions::Pathes(i)));
                String aValue( sTmpPath );
                if(i == SvtPathOptions::PATH_ADDIN ||
                        i == SvtPathOptions::PATH_FILTER ||
                        i == SvtPathOptions::PATH_HELP  ||
                        i == SvtPathOptions::PATH_MODULE  ||
                        i == SvtPathOptions::PATH_PLUGIN  ||
                        i == SvtPathOptions::PATH_STORAGE  )
                    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sTmpPath, aValue );
                aStr += '\t';
                aStr += Convert_Impl( aValue );
                SvLBoxEntry* pEntry = pPathBox->InsertEntry( aStr );
                BOOL   bReadonly = aPathOpt.IsPathReadonly((SvtPathOptions::Pathes) i);
                if(bReadonly)
                {
                    pPathBox->SetCollapsedEntryBmp( pEntry, pImpl->aLockImage,   BMP_COLOR_NORMAL );
                    pPathBox->SetCollapsedEntryBmp( pEntry, pImpl->aLockImageHC,   BMP_COLOR_HIGHCONTRAST  );
                }
                PathUserData_Impl* pPathImpl = new PathUserData_Impl( i );
                pPathImpl->aPathStr = aValue;
                pEntry->SetUserData( pPathImpl );
            }
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
    PathSelect_Impl( NULL );
}

// -----------------------------------------------------------------------

void SvxPathTabPage::FillUserData()
{
    String aUserData = String::CreateFromInt32( pHeaderBar->GetItemSize( ITEMID_TYPE ) );
    aUserData += ';';
    HeaderBarItemBits nBits = pHeaderBar->GetItemBits( ITEMID_TYPE );
    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    aUserData += bUp ? '1' : '0';
    SetUserData( aUserData );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPathTabPage, PathSelect_Impl, OptHeaderTabListBox *, EMPTYARG )

/*  [Beschreibung]

*/

{
    USHORT nSelCount = 0;
    SvLBoxEntry* pEntry = pPathBox->FirstSelected();

    //the entry image indicates whether the path is write protected
    Image aEntryImage;
    if(pEntry)
        aEntryImage = pPathBox->GetCollapsedEntryBmp( pEntry );
    BOOL bEnable = !aEntryImage;
    while ( pEntry && ( nSelCount < 2 ) )
    {
        nSelCount++;
        pEntry = pPathBox->NextSelected( pEntry );
    }

    aPathBtn.Enable( 1 == nSelCount && bEnable);
    aStandardBtn.Enable( nSelCount > 0 && bEnable);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPathTabPage, StandardHdl_Impl, PushButton *, EMPTYARG )
{
    SvLBoxEntry* pEntry = pPathBox->FirstSelected();
    while ( pEntry )
    {
        PathUserData_Impl* pPathImpl = (PathUserData_Impl*)pEntry->GetUserData();
        String aOldPath = pImpl->_aDefOpt.GetDefaultPath( pPathImpl->nRealId );

        if ( aOldPath.Len() )
        {
            pPathBox->SetEntryText( Convert_Impl( aOldPath ), pEntry, 1 );
            pPathImpl->eState = SFX_ITEM_SET;
            pPathImpl->aPathStr = aOldPath;
        }
        pEntry = pPathBox->NextSelected( pEntry );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPathTabPage, PathHdl_Impl, PushButton *, EMPTYARG )
{
    SvLBoxEntry* pEntry = pPathBox->GetCurEntry();
    USHORT nPos = ( pEntry != NULL ) ? ( (PathUserData_Impl*)pEntry->GetUserData() )->nRealId : 0;
    String aPathName;
    if ( pEntry )
    {
        PathUserData_Impl* pPathImpl = (PathUserData_Impl*)pEntry->GetUserData();
        aPathName = pPathImpl->aPathStr;
    }

    if(pEntry && !(!((OptHeaderTabListBox*)pPathBox)->GetCollapsedEntryBmp(pEntry)))
        return 0;

    if ( IsMultiPath_Impl( nPos ) )
    {
        //CHINA001 SvxMultiPathDialog aDlg( this );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractSvxMultiPathDialog* aDlg = pFact->CreateSvxMultiPathDialog( this, ResId(RID_SVXDLG_MULTIPATH) );
            DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
            aDlg->SetPath( aPathName ); //CHINA001 aDlg.SetPath( aPathName );

            if ( aDlg->Execute() == RET_OK && pEntry ) //CHINA001 if ( aDlg.Execute() == RET_OK && pEntry )
            {
                String aNewPath = aDlg->GetPath(); //CHINA001 String aNewPath = aDlg.GetPath();
                pPathBox->SetEntryText( Convert_Impl( aNewPath ), pEntry, 1 );

                // merken, da\s er ver"andert wurde
                nPos = (USHORT)pPathBox->GetModel()->GetAbsPos( pEntry );
                PathUserData_Impl* pPathImpl = (PathUserData_Impl*)pPathBox->GetEntry(nPos)->GetUserData();
                pPathImpl->eState = SFX_ITEM_SET;
                pPathImpl->aPathStr = aNewPath;
            }
            delete aDlg; //add by CHINA001
        }
    }
    else if ( pEntry )
    {
        try
        {
            rtl::OUString aService( RTL_CONSTASCII_USTRINGPARAM( FOLDER_PICKER_SERVICE_NAME ) );
            Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            Reference < XFolderPicker > xFolderPicker( xFactory->createInstance( aService ), UNO_QUERY );

//          svt::SetDialogHelpId( xFolderPicker, HID_OPTIONS_PATHS_SELECTFOLDER );

            INetURLObject aURL( aPathName, INET_PROT_FILE );
            xFolderPicker->setDisplayDirectory( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            short nRet = xFolderPicker->execute();

            if ( ExecutableDialogResults::OK != nRet )
                return 0;

            // old path is an URL?
            INetURLObject aObj( aPathName );
            FASTBOOL bURL = ( aObj.GetProtocol() != INET_PROT_NOT_VALID );
            String aPathStr = xFolderPicker->getDirectory();
            INetURLObject aNewObj( aPathStr );
            aNewObj.removeFinalSlash();

            // then the new path also an URL else system path
            String aNewPathStr = bURL ? aPathStr : aNewObj.getFSysPath( INetURLObject::FSYS_DETECT );

            FASTBOOL bChanged =
#ifdef UNX
// Unix is case sensitive
                                ( aNewPathStr != aPathName );
#else
                                ( aNewPathStr.CompareIgnoreCaseToAscii( aPathName ) != COMPARE_EQUAL );
#endif
            if ( bChanged )
            {
                pPathBox->SetEntryText( Convert_Impl( aNewPathStr ), pEntry, 1 );
                nPos = (USHORT)pPathBox->GetModel()->GetAbsPos( pEntry );
                PathUserData_Impl* pPathImpl = (PathUserData_Impl*)pPathBox->GetEntry(nPos)->GetUserData();
                pPathImpl->eState = SFX_ITEM_SET;
                pPathImpl->aPathStr = aNewPathStr;
            }
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "SvxPathTabPage::PathHdl_Impl: exception from folder picker" )
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPathTabPage, HeaderSelect_Impl, HeaderBar*, pBar )
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

IMPL_LINK( SvxPathTabPage, HeaderEndDrag_Impl, HeaderBar*, pBar )
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
            long nWidth = pHeaderBar->GetItemSize(i);
            aSz.Width() =  nWidth + nTmpSz;
            nTmpSz += nWidth;
            pPathBox->SetTab( i, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
        }
    }
    return 1;
}


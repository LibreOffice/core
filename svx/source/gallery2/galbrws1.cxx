/*************************************************************************
 *
 *  $RCSfile: galbrws1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:18 $
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

#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <vcl/msgbox.hxx>
#include <ucbhelper/content.hxx>
#include "gallery1.hxx"
#include "galtheme.hxx"
#include "galmisc.hxx"
#include "galdlg.hxx"
#include "galbrws1.hxx"

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

// --------------
// - Namespaces -
// --------------

using namespace ::ucb;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;

// -----------------------
// - GalleryThemeListBox -
// -----------------------

GalleryThemeListBox::GalleryThemeListBox( GalleryBrowser1* pParent, WinBits nWinBits ) :
    ListBox( pParent, nWinBits )
{
}

// -----------------------------------------------------------------------------

GalleryThemeListBox::~GalleryThemeListBox()
{
}

// -----------------------------------------------------------------------------

long GalleryThemeListBox::PreNotify( NotifyEvent& rNEvt )
{
    long nRet = ListBox::PreNotify( rNEvt );

    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();

        if( pCEvt->GetCommand() == COMMAND_CONTEXTMENU )
            ( (GalleryBrowser1*) GetParent() )->ShowContextMenu();
    }

    return nRet;
}

// -------------------
// - GalleryBrowser1 -
// -------------------

GalleryBrowser1::GalleryBrowser1( GalleryBrowser* pParent, const ResId& rResId, Gallery* pGallery ) :
    Control     ( pParent, rResId ),
    maNewTheme  ( this, WB_3DLOOK | WB_BORDER ),
    mpThemes    ( new GalleryThemeListBox( this, WB_3DLOOK | WB_BORDER | WB_HSCROLL | WB_VSCROLL | WB_AUTOHSCROLL | WB_SORT ) ),
    mpGallery   ( pGallery )
{
    StartListening( *mpGallery );

    maNewTheme.SetText( String( GAL_RESID( RID_SVXSTR_GALLERY_CREATETHEME ) ) );
    maNewTheme.SetClickHdl( LINK( this, GalleryBrowser1, ClickNewThemeHdl ) );

    mpThemes->SetControlForeground( COL_WHITE );
    mpThemes->SetControlBackground( COL_GRAY );
    mpThemes->SetSelectHdl( LINK( this, GalleryBrowser1, SelectThemeHdl ) );

    for( ULONG i = 0, nCount = mpGallery->GetThemeCount(); i < nCount; i++ )
        ImplInsertThemeEntry( mpGallery->GetThemeInfo( i ) );

    ImplAdjustControls();
    maNewTheme.Show( TRUE );
    mpThemes->Show( TRUE );
}

// -----------------------------------------------------------------------------

GalleryBrowser1::~GalleryBrowser1()
{
    EndListening( *mpGallery );
    delete mpThemes;
}

// -----------------------------------------------------------------------------

ULONG GalleryBrowser1::ImplInsertThemeEntry( const GalleryThemeEntry* pEntry )
{
    ULONG nRet = LISTBOX_ENTRY_NOTFOUND;

    if( pEntry && !pEntry->IsHidden() )
    {
        Bitmap aBMP( GAL_RESID( RID_SVXBMP_THEME_NORMAL ) );
        static const Image aImgNormal( BitmapEx( aBMP, COL_LIGHTMAGENTA ) );
        aBMP = Bitmap( GAL_RESID( RID_SVXBMP_THEME_DEFAULT ) );
        static const Image aImgDefault( BitmapEx( aBMP, COL_LIGHTMAGENTA ) );
        aBMP = Bitmap( GAL_RESID( RID_SVXBMP_THEME_READONLY ) );
        static const Image aImgReadOnly( BitmapEx( aBMP, COL_LIGHTMAGENTA ) );
        aBMP = Bitmap( GAL_RESID( RID_SVXBMP_THEME_IMPORTED ) );
        static const Image aImgImported( BitmapEx( aBMP, COL_LIGHTMAGENTA ) );

        const Image* pImage;

        if( pEntry->IsImported() )
            pImage = &aImgImported;
        else if( pEntry->IsReadOnly() )
            pImage = &aImgReadOnly;
        else if( pEntry->IsDefault() )
            pImage = &aImgDefault;
        else
            pImage = &aImgNormal;

        nRet = mpThemes->InsertEntry( pEntry->GetThemeName(), *pImage );
    }

    return nRet;
}

// -----------------------------------------------------------------------------

void GalleryBrowser1::ImplAdjustControls()
{
    const Size  aOutSize( GetOutputSizePixel() );
    const long  nNewThemeHeight = LogicToPixel( Size( 0, 14 ), MAP_APPFONT ).Height();

    maNewTheme.SetPosSizePixel( Point(),
                                Size( aOutSize.Width(), nNewThemeHeight ) );

    mpThemes->SetPosSizePixel( Point( 0, nNewThemeHeight ),
                               Size( aOutSize.Width(), aOutSize.Height() - nNewThemeHeight ) );
}

// -----------------------------------------------------------------------------

void GalleryBrowser1::ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData )
{
    rData.pTheme = (GalleryTheme*) pThm;
    rData.aEditedTitle = pThm->GetName();

    try
    {
        Content         aCnt( INetURLObject( pThm->GetThmPath(), INET_PROT_FILE ).GetMainURL(),
                              uno::Reference< XCommandEnvironment >() );
        util::DateTime  aDateTimeCreated, aDateTimeModified;
        DateTime        aDateTime;

        aCnt.getPropertyValue( OUString::createFromAscii( "DateCreated" ) ) >>= aDateTimeCreated;
        ::utl::typeConvert( aDateTimeCreated, aDateTime );
        rData.aThemeCreateDate = aDateTime;
        rData.aThemeCreateTime = aDateTime;

        aCnt.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aDateTimeModified;
        ::utl::typeConvert( aDateTimeModified, aDateTime );
        rData.aThemeChangeDate = aDateTime;
        rData.aThemeChangeTime = aDateTime;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "GalleryBrowser1::ImplFillExchangeData: ucb exception" );
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser1::Resize()
{
    Control::Resize();
    ImplAdjustControls();
}

// -----------------------------------------------------------------------------

void GalleryBrowser1::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = (const GalleryHint&) rHint;

    switch( rGalleryHint.GetType() )
    {
        case( GALLERY_HINT_THEME_CREATED ):
            ImplInsertThemeEntry( mpGallery->GetThemeInfo( rGalleryHint.GetThemeName() ) );
        break;

        case( GALLERY_HINT_THEME_RENAMED ):
        {
            const USHORT nCurSelectPos = mpThemes->GetSelectEntryPos();
            const USHORT nRenameEntryPos = mpThemes->GetEntryPos( rGalleryHint.GetThemeName() );

            mpThemes->RemoveEntry( rGalleryHint.GetThemeName() );
            ImplInsertThemeEntry( mpGallery->GetThemeInfo( rGalleryHint.GetStringData() ) );

            if( nCurSelectPos == nRenameEntryPos )
            {
                mpThemes->SelectEntry( rGalleryHint.GetStringData() );
                SelectThemeHdl( NULL );
            }
        }
        break;

        case( GALLERY_HINT_THEME_REMOVED ):
        {
            mpThemes->RemoveEntry( rGalleryHint.GetThemeName() );
        }
        break;

        case( GALLERY_HINT_CLOSE_THEME ):
        {
            const USHORT nCurSelectPos = mpThemes->GetSelectEntryPos();
            const USHORT nCloseEntryPos = mpThemes->GetEntryPos( rGalleryHint.GetThemeName() );

            if( nCurSelectPos == nCloseEntryPos )
            {
                if( nCurSelectPos < ( mpThemes->GetEntryCount() - 1 ) )
                    mpThemes->SelectEntryPos( nCurSelectPos + 1 );
                else if( nCurSelectPos )
                    mpThemes->SelectEntryPos( nCurSelectPos - 1 );
                else
                    mpThemes->SetNoSelection();

                SelectThemeHdl( NULL );
            }
        }
        break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------------

void GalleryBrowser1::ShowContextMenu()
{
    Application::PostUserEvent( LINK( this, GalleryBrowser1, ShowContextMenuHdl ), this );
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser1, ShowContextMenuHdl, void*, p )
{
    PopupMenu aMenu( GAL_RESID( RID_SVXMN_GALLERY1 ) );

    aMenu.SetSelectHdl( LINK( this, GalleryBrowser1, PopupMenuHdl ) );
    aMenu.RemoveDisabledEntries();
    aMenu.Execute( this, GetPointerPosPixel() );

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser1, PopupMenuHdl, Menu*, pMenu )
{
    const USHORT nId = pMenu->GetCurItemId();

    switch( nId )
    {
        case( MN_ACTUALIZE ):
        {
            GalleryTheme*       pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );
            ActualizeProgress   aActualizeProgress( this, pTheme );

            aActualizeProgress.Update();
            aActualizeProgress.Execute();
            mpGallery->ReleaseTheme( pTheme, *this );
        }
        break;

        case( MN_DELETE  ):
        {
            mpGallery->RemoveTheme( mpThemes->GetSelectEntry() );
        }
        break;

        case( MN_RENAME ):
        {
            GalleryTheme*   pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );
            const String    aOldName( pTheme->GetName() );
            TitleDialog     aDlg( this, aOldName );

            if( aDlg.Execute() == RET_OK )
            {
                const String aNewName( aDlg.GetTitle() );

                if( aNewName != aOldName )
                {
                    String  aName( aNewName );
                    USHORT  nCount = 0;

                    while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
                    {
                        aName = aNewName;
                        aName += ' ';
                        aName += String::CreateFromInt32( nCount );
                    }

                    mpGallery->RenameTheme( aOldName, aName );
                }
            }

            mpGallery->ReleaseTheme( pTheme, *this );
        }
        break;

        case( MN_PROPERTIES ):
        {
            SfxItemSet              aSet( SFX_APP()->GetPool() );
            GalleryTheme*           pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );
            ExchangeData            aData; ImplFillExchangeData( pTheme, aData );
            GalleryThemeProperties  aThemeProps( NULL, &aData, &aSet );

            if( RET_OK == aThemeProps.Execute() )
            {
                String aName( pTheme->GetName() );

                if( aName != aData.aEditedTitle )
                {
                    const String    aOldName( aName );
                    String          aName( aData.aEditedTitle );
                    USHORT          nCount = 0;

                    while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
                    {
                        aName = aData.aEditedTitle;
                        aName += ' ';
                        aName += String::CreateFromInt32( nCount );
                    }

                    mpGallery->RenameTheme( aOldName, aName );
                }
            }

            mpGallery->ReleaseTheme( pTheme, *this );
        }
        break;
    }

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser1, SelectThemeHdl, void*, p )
{
    ( (GalleryBrowser*) GetParent() )->ThemeSelectionHasChanged();

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser1, ClickNewThemeHdl, void*, p )
{
    String  aNewTheme( GAL_RESID( RID_SVXSTR_GALLERY_NEWTHEME ) );
    String  aName( aNewTheme );
    ULONG   nCount = 0;

    while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
    {
        aName = aNewTheme;
        aName += ' ';
        aName += String::CreateFromInt32( nCount );
    }

    if( !mpGallery->HasTheme( aName ) && mpGallery->CreateTheme( aName ) )
    {
        GalleryTheme*           pTheme = mpGallery->AcquireTheme( aName, *this );
        SfxItemSet              aSet( SFX_APP()->GetPool() );
        ExchangeData            aData; ImplFillExchangeData( pTheme, aData );
        GalleryThemeProperties  aThemeProps( NULL, &aData, &aSet );

        if( RET_OK == aThemeProps.Execute() )
        {
            String aName( pTheme->GetName() );

            if( aName != aData.aEditedTitle )
            {
                const String    aOldName( aName );
                String          aName( aData.aEditedTitle );
                USHORT          nCount = 0;

                while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
                {
                    aName = aData.aEditedTitle;
                    aName += ' ';
                    aName += String::CreateFromInt32( nCount );
                }

                mpGallery->RenameTheme( aOldName, aName );
            }

            mpThemes->SelectEntry( pTheme->GetName() );
            SelectThemeHdl( NULL );
            mpGallery->ReleaseTheme( pTheme, *this );
        }
        else
        {
            mpGallery->ReleaseTheme( pTheme, *this );
            mpGallery->RemoveTheme( aName );
        }
    }

    return 0L;
}

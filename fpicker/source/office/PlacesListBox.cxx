/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 Cedric Bosdonnat <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <iodlg.hrc>
#include <PlacesListBox.hxx>
#include <svtools/PlaceEditDialog.hxx>

#include <vcl/msgbox.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svtresid.hxx>

#define COLUMN_NAME     1

using rtl::OUString;

PlacesListBox_Impl::PlacesListBox_Impl( PlacesListBox* pParent, const rtl::OUString& rTitle ) :
    SvHeaderTabListBox( pParent, WB_TABSTOP | WB_NOINITIALSELECTION ),
    mpHeaderBar( NULL ),
    mpParent( pParent )
{
    Size aBoxSize = pParent->GetSizePixel( );
    mpHeaderBar = new HeaderBar( pParent, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), Size( 600, 16 ) );

    long pTabs[] = { 2, 20, 600 };
    SetTabs( &pTabs[0], MAP_PIXEL );
    mpHeaderBar->InsertItem( COLUMN_NAME, rTitle, 600, HIB_LEFT | HIB_VCENTER );

    Size aHeadSize = mpHeaderBar->GetSizePixel();
    SetPosSizePixel( Point( 0, aHeadSize.getHeight() ),
                  Size( aBoxSize.getWidth(), aBoxSize.getHeight() - aHeadSize.getHeight() ) );

    InitHeaderBar( mpHeaderBar );

    Show( );
    mpHeaderBar->Show();
}

PlacesListBox_Impl::~PlacesListBox_Impl( )
{
    delete mpHeaderBar;
    mpParent = NULL;
}

void PlacesListBox_Impl::MouseButtonUp( const MouseEvent& rMEvt )
{
    SvHeaderTabListBox::MouseButtonUp( rMEvt );
    mpParent->updateView( );
}

PlacesListBox::PlacesListBox( SvtFileDialog* pFileDlg, const rtl::OUString& rTitle, const ResId& rResId ) :
    Control( pFileDlg, rResId ),
    maPlaces( ),
    mpDlg( pFileDlg ),
    mpImpl( NULL ),
    mpAddBtn( ),
    mpDelBtn( ),
    mnNbEditables( 0 ),
    mbUpdated( false ),
    mbSelectionChanged( false )
{
    mpImpl = new PlacesListBox_Impl( this, rTitle );

    mpImpl->SetSelectHdl( LINK( this, PlacesListBox, Selection ) );
    mpImpl->SetDoubleClickHdl( LINK( this, PlacesListBox, DoubleClick ) ) ;

    mpAddBtn = new ImageButton( this, 0 );
    mpAddBtn->SetText( rtl::OUString( "+" ) );
    mpAddBtn->SetPosSizePixel( Point( 0, 0 ), Size( 24, 24 ) );
    mpAddBtn->Show();

    mpDelBtn = new ImageButton( this, 0 );
    mpDelBtn->SetText( rtl::OUString( "-" ) );
    mpDelBtn->SetPosSizePixel( Point( 0, 0 ), Size( 24, 24 ) );
    mpDelBtn->Show();
}

PlacesListBox::~PlacesListBox( )
{
    delete mpImpl;
    delete mpAddBtn;
    delete mpDelBtn;
}

void PlacesListBox::AppendPlace( PlacePtr pPlace )
{
    maPlaces.push_back( pPlace );
    mpImpl->InsertEntry( pPlace->GetName( ),
            getEntryIcon( pPlace ), getEntryIcon( pPlace ) );

    if(pPlace->IsEditable()) {
        ++mnNbEditables;
        mbUpdated = true;
    }
}

sal_Int32 PlacesListBox::GetNbEditablePlaces() {
    return mnNbEditables;
}

bool PlacesListBox::IsUpdated() {
    if(mbUpdated) {
        mbUpdated = false;
        return true;
    }
    return false;
}

const std::vector<PlacePtr>& PlacesListBox::GetPlaces() {
    return maPlaces;
}

void PlacesListBox::RemovePlace( sal_uInt16 nPos )
{
    if ( nPos < maPlaces.size() )
    {
        if(maPlaces[nPos]->IsEditable()) {
            --mnNbEditables;
            mbUpdated = true;
        }
        maPlaces.erase( maPlaces.begin() + nPos );
        SvTreeListEntry* pEntry = mpImpl->GetEntry( nPos );
        mpImpl->RemoveEntry( pEntry );
    }
}

void PlacesListBox::RemoveSelectedPlace() {
    RemovePlace(mpImpl->GetCurrRow());
}

void PlacesListBox::SetAddHdl( const Link& rHdl )
{
    mpAddBtn->SetClickHdl( rHdl );
}

void PlacesListBox::SetDelHdl( const Link& rHdl )
{
    mpDelBtn->SetClickHdl( rHdl );
}

void PlacesListBox::SetDelEnabled( bool enabled )
{
    mpDelBtn->Enable( enabled );
}

void PlacesListBox::SetSizePixel( const Size& rNewSize )
{
    Control::SetSizePixel( rNewSize );
    Size aListSize( rNewSize );
    aListSize.Height() -= 26 + 18;
    mpImpl->SetSizePixel( aListSize );

    sal_Int32 nBtnY = rNewSize.Height() - 26;
    mpAddBtn->SetPosPixel( Point( 3, nBtnY ) );
    mpDelBtn->SetPosPixel( Point( 6 + 24, nBtnY ) );
}

Image PlacesListBox::getEntryIcon( PlacePtr pPlace )
{
    Image theImage = mpDlg->GetButtonImage( IMG_FILEDLG_PLACE_LOCAL );
    if ( !pPlace->IsLocal( ) )
        theImage = mpDlg->GetButtonImage( IMG_FILEDLG_PLACE_REMOTE );
    return theImage;
}

IMPL_LINK( PlacesListBox, Selection, void* , EMPTYARG )
{
    sal_uInt32 nSelected = mpImpl->GetCurrRow();
    PlacePtr pPlace = maPlaces[nSelected];

    mbSelectionChanged = true;
    if(pPlace->IsEditable())
        mpDlg->RemovablePlaceSelected();
    else
        mpDlg->RemovablePlaceSelected(false);
    return 0;
}

IMPL_LINK ( PlacesListBox, DoubleClick, void*, EMPTYARG )
{
    sal_uInt16 nSelected = mpImpl->GetCurrRow();
    PlacePtr pPlace = maPlaces[nSelected];
    if ( pPlace->IsEditable() == true && !pPlace->IsLocal( ) )
    {
        PlaceEditDialog aDlg( mpDlg, pPlace );
        short aRetCode = aDlg.Execute();
        switch(aRetCode) {
            case RET_OK :
            {
                pPlace->SetName ( aDlg.GetServerName() );
                pPlace->SetUrl( aDlg.GetServerUrl() );
                mbUpdated = true;
                break;
            }
            case RET_NO :
            {
                RemovePlace(nSelected);
                break;
            }
            default:
                break;
        };
    }
    return 0;
}

void PlacesListBox::updateView( )
{
    if ( mbSelectionChanged )
    {
        mbSelectionChanged = false;
        sal_uInt32 nSelected = mpImpl->GetCurrRow();
        PlacePtr pPlace = maPlaces[nSelected];
        mpDlg->OpenURL_Impl( pPlace->GetUrl( ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "PlacesListBox.hxx"
#include <svtools/PlaceEditDialog.hxx>

#include <vcl/headbar.hxx>
#include <vcl/event.hxx>
#include <bitmaps.hlst>

#define COLUMN_NAME     1


PlacesListBox_Impl::PlacesListBox_Impl( PlacesListBox* pParent, const OUString& rTitle ) :
    SvHeaderTabListBox( pParent, WB_TABSTOP | WB_NOINITIALSELECTION ),
    mpHeaderBar( nullptr ),
    mpParent( pParent )
{
    Size aBoxSize = pParent->GetSizePixel( );
    mpHeaderBar = VclPtr<HeaderBar>::Create( pParent, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), Size( 600, 16 ) );

    long aTabPositions[] = { 20, 600 };
    SetTabs( SAL_N_ELEMENTS(aTabPositions), aTabPositions, MapUnit::MapPixel );
    mpHeaderBar->InsertItem( COLUMN_NAME, rTitle, 600, HeaderBarItemBits::LEFT );

    Size aHeadSize = mpHeaderBar->GetSizePixel();
    SetPosSizePixel( Point( 0, aHeadSize.getHeight() ),
                  Size( aBoxSize.getWidth(), aBoxSize.getHeight() - aHeadSize.getHeight() ) );

    InitHeaderBar( mpHeaderBar );

    Show( );
    mpHeaderBar->Show();
}

PlacesListBox_Impl::~PlacesListBox_Impl( )
{
    disposeOnce();
}

void PlacesListBox_Impl::dispose()
{
    mpHeaderBar.disposeAndClear();
    mpParent.clear();
    SvHeaderTabListBox::dispose();
}

void PlacesListBox_Impl::MouseButtonUp( const MouseEvent& rMEvt )
{
    SvHeaderTabListBox::MouseButtonUp( rMEvt );
    mpParent->updateView( );
}

PlacesListBox::PlacesListBox( vcl::Window* pParent, SvtFileDialog* pFileDlg, const OUString& rTitle, WinBits nBits ) :
    Control( pParent, nBits ),
    maPlaces( ),
    mpDlg( pFileDlg ),
    mpImpl( nullptr ),
//    mxAddBtn(m_xBuilder->weld_button("add")),
//    mxDelBtn(m_xBuilder->weld_button("del")),
    mnNbEditables( 0 ),
    mbUpdated( false ),
    mbSelectionChanged( false )
{
    mpImpl = VclPtr<PlacesListBox_Impl>::Create( this, rTitle );

    mpImpl->SetSelectHdl( LINK( this, PlacesListBox, Selection ) );
    mpImpl->SetDoubleClickHdl( LINK( this, PlacesListBox, DoubleClick ) ) ;

    mxAddBtn->set_label( "+" );
    mxAddBtn->show();

    mxDelBtn->set_label( "-" );
    mxDelBtn->show();
}

PlacesListBox::~PlacesListBox( )
{
    disposeOnce();
}

void PlacesListBox::dispose()
{
    mpImpl.disposeAndClear();
    Control::dispose();
}

void PlacesListBox::AppendPlace( const PlacePtr& pPlace )
{
    maPlaces.push_back( pPlace );
    mpImpl->InsertEntry( pPlace->GetName( ),
            getEntryIcon( pPlace ), getEntryIcon( pPlace ) );

    if(pPlace->IsEditable()) {
        ++mnNbEditables;
        mbUpdated = true;
    }
}


bool PlacesListBox::IsUpdated() {
    if(mbUpdated) {
        mbUpdated = false;
        return true;
    }
    return false;
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

void PlacesListBox::SetAddHdl( const Link<weld::Button&,void>& rHdl )
{
    mxAddBtn->connect_clicked( rHdl );
}

void PlacesListBox::SetDelHdl( const Link<weld::Button&,void>& rHdl )
{
    mxDelBtn->connect_clicked( rHdl );
}

void PlacesListBox::SetDelEnabled( bool enabled )
{
    mxDelBtn->set_sensitive( enabled );
}

void PlacesListBox::SetSizePixel( const Size& rNewSize )
{
    Control::SetSizePixel( rNewSize );
    Size aListSize( rNewSize );
    aListSize.AdjustHeight( -(26 + 18) );
    mpImpl->SetSizePixel( aListSize );
}

bool PlacesListBox::EventNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();

        if( rCode.GetCode() == KEY_RETURN )
        {
            mbSelectionChanged = true;
            updateView();
            return true;
        }
    }
    return Control::EventNotify(rNEvt);
}

Image PlacesListBox::getEntryIcon( const PlacePtr& pPlace )
{
    Image theImage = Image(StockImage::Yes, BMP_FILEDLG_PLACE_LOCAL);
    if ( !pPlace->IsLocal( ) )
        theImage = Image(StockImage::Yes, BMP_FILEDLG_PLACE_REMOTE);
    return theImage;
}

IMPL_LINK_NOARG( PlacesListBox, Selection, SvTreeListBox*, void )
{
    sal_uInt32 nSelected = mpImpl->GetCurrRow();
    PlacePtr pPlace = maPlaces[nSelected];

    mbSelectionChanged = true;
    if(pPlace->IsEditable())
        mpDlg->RemovablePlaceSelected();
    else
        mpDlg->RemovablePlaceSelected(false);
}

IMPL_LINK_NOARG( PlacesListBox, DoubleClick, SvTreeListBox*, bool )
{
    sal_uInt16 nSelected = mpImpl->GetCurrRow();
    PlacePtr pPlace = maPlaces[nSelected];
    if ( pPlace->IsEditable() && !pPlace->IsLocal( ) )
    {
        PlaceEditDialog aDlg(mpDlg->getDialog(), pPlace);
        short aRetCode = aDlg.run();
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
    return false;
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

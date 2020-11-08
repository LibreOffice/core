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

#include <bitmaps.hlst>

PlacesListBox::PlacesListBox(std::unique_ptr<weld::TreeView> xControl,
                             std::unique_ptr<weld::Button> xAdd,
                             std::unique_ptr<weld::Button> xDel,
                             SvtFileDialog* pFileDlg)
    : maPlaces( )
    , mpDlg(pFileDlg)
    , mxImpl(std::move(xControl))
    , mxAddBtn(std::move(xAdd))
    , mxDelBtn(std::move(xDel))
    , mnNbEditables(0)
    , mbUpdated( false )
{
    Size aSize(mxImpl->get_approximate_digit_width() * 18,
               mxImpl->get_height_rows(9));
    mxImpl->set_size_request(aSize.Width(), aSize.Height());

    mxImpl->connect_changed( LINK( this, PlacesListBox, Selection ) );
    mxImpl->connect_row_activated( LINK( this, PlacesListBox, DoubleClick ) ) ;
}

PlacesListBox::~PlacesListBox( )
{
}

void PlacesListBox::AppendPlace( const PlacePtr& pPlace )
{
    maPlaces.push_back( pPlace );
    mxImpl->append_text(pPlace->GetName());
    mxImpl->set_image(maPlaces.size() - 1, getEntryIcon(pPlace));

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
        mxImpl->remove(nPos);
    }
}

void PlacesListBox::RemoveSelectedPlace() {
    RemovePlace(mxImpl->get_cursor_index());
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

OUString PlacesListBox::getEntryIcon( const PlacePtr& pPlace )
{
    OUString theImage = BMP_FILEDLG_PLACE_LOCAL;
    if ( !pPlace->IsLocal( ) )
        theImage = BMP_FILEDLG_PLACE_REMOTE;
    return theImage;
}

IMPL_LINK_NOARG( PlacesListBox, Selection, weld::TreeView&, void )
{
    sal_uInt32 nSelected = mxImpl->get_cursor_index();
    PlacePtr pPlace = maPlaces[nSelected];

    if (pPlace->IsEditable())
        mpDlg->RemovablePlaceSelected();
    else
        mpDlg->RemovablePlaceSelected(false);

    updateView();
}

IMPL_LINK_NOARG( PlacesListBox, DoubleClick, weld::TreeView&, bool )
{
    sal_uInt16 nSelected = mxImpl->get_cursor_index();
    PlacePtr pPlace = maPlaces[nSelected];
    if ( pPlace->IsEditable() && !pPlace->IsLocal( ) )
    {
        PlaceEditDialog aDlg(mpDlg->getDialog(), pPlace);
        short aRetCode = aDlg.run();
        switch (aRetCode)
        {
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
        }
    }
    return true;
}

void PlacesListBox::updateView( )
{
    sal_uInt32 nSelected = mxImpl->get_cursor_index();
    PlacePtr pPlace = maPlaces[nSelected];
    mpDlg->OpenURL_Impl( pPlace->GetUrl( ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

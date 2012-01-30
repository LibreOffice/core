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
#include "SvtPlaceDialog.hxx"

#include <vcl/msgbox.hxx>
#include <svtools/svtdata.hxx>

namespace css = com::sun::star;
using rtl::OUString;

PlacesListBox::PlacesListBox( SvtFileDialog* pFileDlg, const ResId& rResId ) :
    ListBox( pFileDlg, rResId ),
    maPlaces( ),
    mpDlg( pFileDlg ),
    mnNbEditables( 0 ),
    mbUpdated( false )
{
    SetSelectHdl( LINK( this, PlacesListBox, SelectHdl ) );
    SetDoubleClickHdl( LINK( this, PlacesListBox, DoubleClickHdl ) ) ;
}

PlacesListBox::~PlacesListBox( )
{
}

void PlacesListBox::AppendPlace( PlacePtr pPlace )
{
    maPlaces.push_back( pPlace );
    InsertEntry( pPlace->GetName( ), getEntryIcon( pPlace->GetType( ) ));

    if(pPlace->IsEditable()) {
        ++mnNbEditables;
        mbUpdated = true;
    }
}

sal_Int32 PlacesListBox::GetNbPlaces() {
    return maPlaces.size();
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
        RemoveEntry( nPos );
    }
}

void PlacesListBox::RemoveSelectedPlace() {
    RemovePlace(GetSelectEntryPos());
}

Image PlacesListBox::getEntryIcon(Place::ePlaceType aType)
{
	Image theImage;
	switch (aType) {
	case Place::e_PlaceCmis:
		theImage =  mpDlg->GetButtonImage( IMG_FILEDLG_BTN_UP );
		break;
	case Place::e_PlaceFtp:
		theImage =  mpDlg->GetButtonImage( IMG_FILEDLG_BTN_UP );
		break;
	case Place::e_PlaceLocal:
	default:
		theImage =  mpDlg->GetButtonImage( IMG_FILEDLG_BTN_UP );
		break;
	};
	return theImage;
}

IMPL_LINK( PlacesListBox, SelectHdl, ListBox* , EMPTYARG )
{
    sal_uInt16 nSelected = GetSelectEntryPos();
    PlacePtr pPlace = maPlaces[nSelected];
    mpDlg->OpenURL_Impl( pPlace->GetUrl() );

    if(pPlace->IsEditable())
        mpDlg->RemovablePlaceSelected();
    else
        mpDlg->RemovablePlaceSelected(false);

    return 0;
}

IMPL_LINK ( PlacesListBox, DoubleClickHdl, ListBox*, EMPTYARG )
{
	sal_uInt16 nSelected = GetSelectEntryPos();
	PlacePtr pPlace = maPlaces[nSelected];
	if ( pPlace->IsEditable() == true )
	{
		SvtPlaceDialog aDlg(mpDlg,pPlace);
		short aRetCode = aDlg.Execute();
		switch(aRetCode) {
			case RET_OK :
			{
				pPlace->SetName ( aDlg.GetServerName() );
				pPlace->SetUrl( aDlg.GetServerUrl() );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

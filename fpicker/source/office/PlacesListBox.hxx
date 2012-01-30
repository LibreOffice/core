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
#ifndef _PLACESLISTBOX_HXX_
#define _PLACESLISTBOX_HXX_

#include <iodlg.hxx>
#include <vcl/lstbox.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>

/** Class representing a file location: it mainly consist of display attributes and a URL.
  */
class Place
{
	public:
		enum ePlaceType {
			e_PlaceLocal = 0,
			e_PlaceFtp,
			e_PlaceCmis
		};

    private:
        rtl::OUString msName;
        rtl::OUString msUrl;
        ePlaceType meType;

        sal_Bool mbEditable;

    public:

        Place( rtl::OUString sName, rtl::OUString sUrl, ePlaceType eType, sal_Bool bEditable = false) :
			msName( sName ),
			msUrl( sUrl ),
			meType( eType ),
			mbEditable( bEditable ) {};

        ~Place( ) {};

        Place( const Place& rCopy ) : msName( rCopy.msName ), msUrl( rCopy.msUrl ), meType( rCopy.meType ){ };

		void SetName(const rtl::OUString& aName )    { msName = aName; }
		void SetUrl(const  rtl::OUString& aUrl )	 { msUrl = aUrl; }

        rtl::OUString& GetName( ) { return msName; }
        rtl::OUString& GetUrl( ) { return msUrl; }
        ePlaceType& GetType( ) { return meType; }
        sal_Bool& IsEditable( ) { return mbEditable; }
};

typedef boost::shared_ptr< Place > PlacePtr;

/** ListBox to handle Places.
  */
class PlacesListBox : public ListBox
{
    private:
        std::vector< PlacePtr > maPlaces;
        SvtFileDialog*       mpDlg;
        sal_Int32            mnNbEditables;
        bool                 mbUpdated;

    public:
        PlacesListBox( SvtFileDialog* pFileDlg, const ResId& rResId );
        ~PlacesListBox( );

        void AppendPlace( PlacePtr pPlace );
        void RemovePlace( sal_uInt16 nPos );
        void RemoveSelectedPlace();
        sal_Int32 GetNbPlaces();
        sal_Int32 GetNbEditablePlaces();
        bool IsUpdated();
        const std::vector<PlacePtr>& GetPlaces();

    private:
        Image getEntryIcon( Place::ePlaceType eType);

        DECL_LINK( SelectHdl, ListBox* );
        DECL_LINK( DoubleClickHdl, ListBox* );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

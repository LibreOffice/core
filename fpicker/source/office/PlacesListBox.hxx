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

#include <boost/shared_ptr.hpp>
#include <svtools/place.hxx>
#include <svtools/svtabbx.hxx>

#include <vector>

typedef boost::shared_ptr< Place > PlacePtr;

class PlacesListBox;
class PlacesListBox_Impl : public SvHeaderTabListBox
{
    private:
        HeaderBar*           mpHeaderBar;
        PlacesListBox*       mpParent;

    public:
        PlacesListBox_Impl( PlacesListBox* pParent, const rtl::OUString& rTitle );
        ~PlacesListBox_Impl( );

        virtual void MouseButtonUp( const MouseEvent& rMEvt );
};

/** ListBox to handle Places.
  */
class PlacesListBox : public Control
{
    private:
        std::vector< PlacePtr > maPlaces;
        SvtFileDialog*       mpDlg;
        PlacesListBox_Impl*  mpImpl;
        PushButton*          mpAddBtn;
        PushButton*          mpDelBtn;
        sal_Int32            mnNbEditables;
        bool                 mbUpdated;
        bool                 mbSelectionChanged;

    public:
        PlacesListBox( SvtFileDialog* pFileDlg, const rtl::OUString& rTitle, const ResId& rResId );
        ~PlacesListBox( );

        void AppendPlace( PlacePtr pPlace );
        void RemovePlace( sal_uInt16 nPos );
        void RemoveSelectedPlace();
        sal_Int32 GetNbEditablePlaces();
        bool IsUpdated();
        const std::vector<PlacePtr>& GetPlaces();

        void SetAddHdl( const Link& rHdl );
        void SetDelHdl( const Link& rHdl );
        void SetDelEnabled( bool enabled );
        void SetSizePixel( const Size& rNewSize );
        void updateView( );

    private:

        Image getEntryIcon( PlacePtr pPlace );

        DECL_LINK( Selection, void* );
        DECL_LINK( DoubleClick, void* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

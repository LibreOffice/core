/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_PLACESLISTBOX_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_PLACESLISTBOX_HXX

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
        PlacesListBox_Impl( PlacesListBox* pParent, const OUString& rTitle );
        virtual ~PlacesListBox_Impl( );

        virtual void MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
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
        PlacesListBox( vcl::Window* pParent, SvtFileDialog* pFileDlg, const OUString& rTitle, WinBits nBits );
        virtual ~PlacesListBox( );

        void AppendPlace( PlacePtr pPlace );
        void RemovePlace( sal_uInt16 nPos );
        void RemoveSelectedPlace();
        sal_Int32 GetNbEditablePlaces() { return mnNbEditables;}
        bool IsUpdated();
        const std::vector<PlacePtr>& GetPlaces() { return maPlaces;}

        void SetAddHdl( const Link& rHdl );
        void SetDelHdl( const Link& rHdl );
        void SetDelEnabled( bool enabled );
        void SetSizePixel( const Size& rNewSize ) SAL_OVERRIDE;
        void updateView( );

    private:

        Image getEntryIcon( PlacePtr pPlace );

        DECL_LINK( Selection, void* );
        DECL_LINK( DoubleClick, void* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

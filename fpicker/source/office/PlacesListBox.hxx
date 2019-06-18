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

#include "iodlg.hxx"

#include <svtools/place.hxx>
#include <vcl/svtabbx.hxx>

#include <memory>
#include <vector>

typedef std::shared_ptr< Place > PlacePtr;

class PlacesListBox;
class PlacesListBox_Impl : public SvHeaderTabListBox
{
    private:
        VclPtr<HeaderBar>           mpHeaderBar;
        VclPtr<PlacesListBox>       mpParent;

    public:
        PlacesListBox_Impl( PlacesListBox* pParent, const OUString& rTitle );
        virtual ~PlacesListBox_Impl( ) override;
        virtual void dispose() override;

        virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
};

/** ListBox to handle Places.
  */
class PlacesListBox : public Control
{
    private:
        std::vector< PlacePtr > maPlaces;
        VclPtr<SvtFileDialog>       mpDlg;
        VclPtr<PlacesListBox_Impl>  mpImpl;
        VclPtr<PushButton>          mpAddBtn;
        VclPtr<PushButton>          mpDelBtn;
        sal_Int32            mnNbEditables;
        bool                 mbUpdated;
        bool                 mbSelectionChanged;

    public:
        PlacesListBox( vcl::Window* pParent, SvtFileDialog* pFileDlg, const OUString& rTitle, WinBits nBits );
        virtual ~PlacesListBox( ) override;
        virtual void dispose() override;

        void AppendPlace( const PlacePtr& pPlace );
        void RemovePlace( sal_uInt16 nPos );
        void RemoveSelectedPlace();
        sal_Int32 GetNbEditablePlaces() const { return mnNbEditables;}
        bool IsUpdated();
        const std::vector<PlacePtr>& GetPlaces() const { return maPlaces;}

        void SetAddHdl( const Link<Button*,void>& rHdl );
        void SetDelHdl( const Link<Button*,void>& rHdl );
        void SetDelEnabled( bool enabled );
        void SetSizePixel( const Size& rNewSize ) override;
        void updateView( );

        const VclPtr<PushButton>& GetAddButton() const { return mpAddBtn; }
        const VclPtr<PushButton>& GetDeleteButton() const { return mpDelBtn; }
        const VclPtr<PlacesListBox_Impl>& GetPlacesListBox() const { return mpImpl; }

        virtual bool EventNotify( NotifyEvent& rNEvt ) override;

    private:

        static Image getEntryIcon( const PlacePtr& pPlace );

        DECL_LINK( Selection, SvTreeListBox*, void );
        DECL_LINK( DoubleClick, SvTreeListBox*, bool );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

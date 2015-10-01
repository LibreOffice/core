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

#include <svtools/place.hxx>
#include <svtools/svtabbx.hxx>

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
        virtual ~PlacesListBox_Impl( );
        virtual void dispose() SAL_OVERRIDE;

        virtual void MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
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
        virtual ~PlacesListBox( );
        virtual void dispose() SAL_OVERRIDE;

        void AppendPlace( PlacePtr pPlace );
        void RemovePlace( sal_uInt16 nPos );
        void RemoveSelectedPlace();
        sal_Int32 GetNbEditablePlaces() { return mnNbEditables;}
        bool IsUpdated();
        const std::vector<PlacePtr>& GetPlaces() { return maPlaces;}

        void SetAddHdl( const Link<Button*,void>& rHdl );
        void SetDelHdl( const Link<Button*,void>& rHdl );
        void SetDelEnabled( bool enabled );
        void SetSizePixel( const Size& rNewSize ) SAL_OVERRIDE;
        void updateView( );

        VclPtr<PushButton> GetAddButton() const { return mpAddBtn; }
        VclPtr<PushButton> GetDeleteButton() const { return mpDelBtn; }
        VclPtr<PlacesListBox_Impl> GetPlacesListBox() const { return mpImpl; }

        virtual bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    private:

        Image getEntryIcon( PlacePtr pPlace );

        DECL_LINK_TYPED( Selection, SvTreeListBox*, void );
        DECL_LINK_TYPED( DoubleClick, SvTreeListBox*, bool );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

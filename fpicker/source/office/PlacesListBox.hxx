/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "iodlg.hxx"

#include <svtools/place.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <vector>

typedef std::shared_ptr<Place> PlacePtr;

class PlacesListBox;

/** ListBox to handle Places.
  */
class PlacesListBox
{
private:
    std::vector<PlacePtr> maPlaces;
    SvtFileDialog* mpDlg;
    std::unique_ptr<weld::TreeView> mxImpl;
    std::unique_ptr<weld::Button> mxAddBtn;
    std::unique_ptr<weld::Button> mxDelBtn;
    sal_Int32            mnNbEditables;
    bool                 mbUpdated;

public:
    PlacesListBox(std::unique_ptr<weld::TreeView> xTreeView,
                  std::unique_ptr<weld::Button> xAddBtn,
                  std::unique_ptr<weld::Button> xDelBtn,
                  SvtFileDialog* pFileDlg);
    ~PlacesListBox();

    void AppendPlace( const PlacePtr& pPlace );
    void RemovePlace( sal_uInt16 nPos );
    void RemoveSelectedPlace();
    sal_Int32 GetNbEditablePlaces() const { return mnNbEditables;}
    bool IsUpdated();
    const std::vector<PlacePtr>& GetPlaces() const { return maPlaces;}

    void SetAddHdl( const Link<weld::Button&,void>& rHdl );
    void SetDelHdl( const Link<weld::Button&,void>& rHdl );
    void SetDelEnabled( bool enabled );
    void updateView( );

    void set_help_id(const OString& rHelpId) { mxImpl->set_help_id(rHelpId); }

private:

    static OUString getEntryIcon(const PlacePtr& pPlace);

    DECL_LINK( Selection, weld::TreeView&, void );
    DECL_LINK( DoubleClick, weld::TreeView&, bool );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

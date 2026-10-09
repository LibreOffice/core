/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <pivot.hxx>

#include <memory>
#include <string_view>

class ScPivotLayoutDialog;

enum class ScPivotLayoutMobileArea
{
    Filters = 0,
    Columns,
    Rows,
    Values
};

// One field placed in an area of the phone layout.
class ScPivotLayoutMobileEntry final
{
public:
    // rAreaName is the id prefix of the area's widgets. nIndex numbers the shown entries,
    // nPosition is the place in the field list.
    ScPivotLayoutMobileEntry(ScPivotLayoutDialog& rParent, weld::Container& rContainer,
                             ScPivotLayoutMobileArea eArea, std::u16string_view rAreaName,
                             int nIndex, int nPosition, const OUString& rName,
                             PivotFunc nFunction);
    ~ScPivotLayoutMobileEntry();

    ScPivotLayoutMobileArea GetArea() const { return meArea; }
    int GetPosition() const { return mnPosition; }
    PivotFunc GetFunction() const;

private:
    DECL_LINK(RemoveClickedHdl, weld::Button&, void);
    DECL_LINK(FunctionChangedHdl, weld::ComboBox&, void);

    ScPivotLayoutDialog& mrParent;
    weld::Container& mrContainer;
    ScPivotLayoutMobileArea meArea;
    int mnPosition;

    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxEntry;
    std::unique_ptr<weld::Label> mxName;
    std::unique_ptr<weld::Button> mxRemove;
    std::unique_ptr<weld::ComboBox> mxFunction;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

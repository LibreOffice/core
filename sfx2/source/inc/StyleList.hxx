/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <array>
#include <memory>
#include <optional>

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svl/lstner.hxx>
#include <svl/eitem.hxx>

#include <svl/style.hxx>

#include <vcl/idle.hxx>

#include <sfx2/docfac.hxx>
#include <vcl/svapp.hxx>

class SfxObjectShell;

namespace weld
{

class VCL_DLLPUBLIC StyleList : public TreeView
{

protected:

    std::unique_ptr<weld::StyleList> mxStyleList;
    bool bAllowReParentDrop:1;

    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

public:

    void setStyleFamily(SfxStyleFamily family);
    void setPreview(bool bPreview);
    void setFilter(weld::ComboBox& combobsox);

};
}

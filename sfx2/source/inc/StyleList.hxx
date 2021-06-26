/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_STYLELIST_HXX
#define INCLUDED_VCL_STYLELIST_HXX

#include <sal/config.h>

#include <array>
#include <memory>
#include <optional>

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svl/lstner.hxx>
#include <svl/eitem.hxx>

#include <svl/style.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/templdlg.hxx>

#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include <templdgi.hxx>

enum class StyleFamily
{
    Character,
    Paragraph,
    Frame,
    Page,
    List,
    Table,
};

namespace weld
{
class TreeView;
class ComboBox;

class VCL_DLLPUBLIC StyleList : public TreeView
{

protected:

    std::unique_ptr<weld::TreeView> mxCharTreeBox;
    std::unique_ptr<weld::CheckButton> mxCharPreviewCheckbox;
    std::unique_ptr<weld::ComboBox> mxCharFilterLb;

    bool m_bCharHierarchical :1;
    bool m_bWantCharHierarchical :1;

public:

    void setStyleFamily(SfxStyleFamily family);
    void setPreview(bool bPreview);
    void setFilter(weld::ComboBox& combobox);

};
}
#endif

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DROPDOWNFORMFIELDDIALOG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DROPDOWNFORMFIELDDIALOG_HXX

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/window.hxx>
#include <svx/stddlg.hxx>
#include "actctrl.hxx"

namespace sw
{
namespace mark
{
class IFieldmark;
}
} // namespace sw

/// Dialog to specify the properties of drop-down form field
namespace sw
{
class DropDownFormFieldDialog : public SvxStandardDialog
{
private:
    mark::IFieldmark* m_pDropDownField;
    bool m_bListHasChanged;

    VclPtr<ReturnActionEdit> m_xListItemEntry;
    VclPtr<PushButton> m_xListAddButton;

    VclPtr<ListBox> m_xListItemsTreeView;

    VclPtr<PushButton> m_xListRemoveButton;
    VclPtr<PushButton> m_xListUpButton;
    VclPtr<PushButton> m_xListDownButton;

    DECL_LINK(SelectHdl, ListBox&, void);
    DECL_LINK(ModifyEditHdl, Edit&, void);
    DECL_LINK(ReturnActionHdl, ReturnActionEdit&, void);
    DECL_LINK(ButtonPushedHdl, Button*, void);

    void InitControls();
    void AppendItemToList();
    void UpdateButtons();
    virtual void Apply() override;

public:
    DropDownFormFieldDialog(vcl::Window* pParent, mark::IFieldmark* pDropDownField);
    virtual ~DropDownFormFieldDialog() override;
};

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/InterimItemWindow.hxx>
#include <vcl/weld/ComboBox.hxx>
#include <SheetViewTypes.hxx>

/** Box / container for the combobox for showing / selecting the sheet view */
class SheetViewBox final : public InterimItemWindow
{
public:
    explicit SheetViewBox(vcl::Window* pParent);
    virtual void dispose() override;
    virtual ~SheetViewBox() override;

    virtual void GetFocus() override;

    void Update(sc::SheetViewID nID);

    weld::ComboBox* getWidget() { return m_xWidget.get(); }

private:
    std::unique_ptr<weld::ComboBox> m_xWidget;

    DECL_STATIC_LINK(SheetViewBox, SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

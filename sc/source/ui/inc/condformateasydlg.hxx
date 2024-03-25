/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "anyrefdg.hxx"
#include <svl/lstner.hxx>
#include <conditio.hxx>

class ScViewData;
class ScConditionalFormat;
class ScTabViewShell;

namespace sc
{
class ConditionalFormatEasyDialog : public ScAnyRefDlgController, public SfxListener
{
public:
    ConditionalFormatEasyDialog(SfxBindings*, SfxChildWindow*, weld::Window*, ScViewData*);
    virtual ~ConditionalFormatEasyDialog() override;

    virtual void SetReference(const ScRange&, ScDocument&) override;
    virtual void SetActive() override;
    virtual void Close() override;

    virtual void Notify(SfxBroadcaster&, const SfxHint&) override;

    DECL_LINK(ButtonPressed, weld::Button&, void);

private:
    void SetDescription(std::u16string_view rCondition);

    ScViewData* mpViewData;
    ScDocument* mpDocument;
    ScConditionMode meMode;
    ScAddress maPosition;
    ScTabViewShell* mpTabViewShell;

    std::unique_ptr<weld::Entry> mxNumberEntry;
    std::unique_ptr<weld::Entry> mxNumberEntry2;
    std::unique_ptr<weld::Container> mxAllInputs;
    std::unique_ptr<formula::RefEdit> mxRangeEntry;
    std::unique_ptr<formula::RefButton> mxButtonRangeEdit;
    std::unique_ptr<weld::ComboBox> mxStyles;
    std::unique_ptr<weld::Label> mxDescription;
    std::unique_ptr<weld::Button> mxButtonOk;
    std::unique_ptr<weld::Button> mxButtonCancel;
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

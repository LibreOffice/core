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

#include <PivotLayoutMobileEntry.hxx>
#include <PivotLayoutDialog.hxx>
#include <pvfundlg.hxx>
#include <pvfundlg.hrc>
#include <scresid.hxx>

#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>

#include <iterator>

ScPivotLayoutMobileEntry::ScPivotLayoutMobileEntry(ScPivotLayoutDialog& rParent,
                                                   weld::Container& rContainer,
                                                   ScPivotLayoutMobileArea eArea,
                                                   std::u16string_view rAreaName, int nIndex,
                                                   int nPosition, const OUString& rName,
                                                   PivotFunc nFunction)
    : mrParent(rParent)
    , mrContainer(rContainer)
    , meArea(eArea)
    , mnPosition(nPosition)
    , mxBuilder(Application::CreateBuilder(
          &rContainer, u"modules/scalc/ui/pivottablelayoutentry-mobile.ui"_ustr, true))
    , mxEntry(mxBuilder->weld_container(u"entry"_ustr))
    , mxName(mxBuilder->weld_label(u"name"_ustr))
    , mxRemove(mxBuilder->weld_button(u"remove"_ustr))
    , mxFunction(mxBuilder->weld_combo_box(u"function"_ustr))
{
    // All entries share one ui file, so the names get an area and index suffix.
    OUString aSuffix = OUString::Concat("-") + rAreaName + "-" + OUString::number(nIndex);
    mxEntry->set_buildable_name(u"entry"_ustr + aSuffix);
    mxBuilder->weld_container(u"row"_ustr)->set_buildable_name(u"row"_ustr + aSuffix);
    mxName->set_buildable_name(u"name"_ustr + aSuffix);
    mxRemove->set_buildable_name(u"remove"_ustr + aSuffix);
    mxFunction->set_buildable_name(u"function"_ustr + aSuffix);

    mxName->set_label(rName);
    mxRemove->connect_clicked(LINK(this, ScPivotLayoutMobileEntry, RemoveClickedHdl));

    if (eArea == ScPivotLayoutMobileArea::Values)
    {
        mxName->hide();
        for (const TranslateId& rFunctionName : SCSTR_DPFUNCLISTBOX)
            mxFunction->append_text(ScResId(rFunctionName) + " - " + rName);

        for (size_t nEntry = 0; nEntry < std::size(ScDPDataFieldFunctions); ++nEntry)
        {
            if (ScDPDataFieldFunctions[nEntry] == nFunction)
                mxFunction->set_active(static_cast<int>(nEntry));
        }
        mxFunction->connect_changed(LINK(this, ScPivotLayoutMobileEntry, FunctionChangedHdl));
        mxFunction->show();
    }
}

ScPivotLayoutMobileEntry::~ScPivotLayoutMobileEntry() { mrContainer.move(mxEntry.get(), nullptr); }

PivotFunc ScPivotLayoutMobileEntry::GetFunction() const
{
    int nActive = mxFunction->get_active();
    if (nActive < 0 || o3tl::make_unsigned(nActive) >= std::size(ScDPDataFieldFunctions))
        return PivotFunc::NONE;
    return ScDPDataFieldFunctions[nActive];
}

IMPL_LINK_NOARG(ScPivotLayoutMobileEntry, RemoveClickedHdl, weld::Button&, void)
{
    mrParent.MobileEntryRemoved(*this);
}

IMPL_LINK_NOARG(ScPivotLayoutMobileEntry, FunctionChangedHdl, weld::ComboBox&, void)
{
    mrParent.MobileEntryFunctionChanged(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

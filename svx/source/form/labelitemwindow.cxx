/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/labelitemwindow.hxx>

LabelItemWindow::LabelItemWindow(vcl::Window* pParent, const OUString& rLabel)
    : InterimItemWindow(pParent, "svx/ui/labelbox.ui", "LabelBox")
    , m_xLabel(m_xBuilder->weld_label("label"))
{
    m_xLabel->set_label(rLabel);

    SetOptimalSize();

    m_xLabel->set_toolbar_background();
}

void LabelItemWindow::SetOptimalSize()
{
    Size aSize(m_xLabel->get_preferred_size());
    aSize.AdjustWidth(12);

    SetSizePixel(aSize);
}

void LabelItemWindow::set_label(const OUString& rLabel) { m_xLabel->set_label(rLabel); }

OUString LabelItemWindow::get_label() const { return m_xLabel->get_label(); }

void LabelItemWindow::dispose()
{
    m_xLabel.reset();
    InterimItemWindow::dispose();
}

LabelItemWindow::~LabelItemWindow() { disposeOnce(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

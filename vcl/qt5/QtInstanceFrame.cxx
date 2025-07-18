/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceFrame.hxx>

#include <vcl/qt/QtUtils.hxx>

QtInstanceFrame::QtInstanceFrame(QGroupBox* pGroupBox)
    : QtInstanceContainer(pGroupBox)
    , m_pGroupBox(pGroupBox)
{
    assert(m_pGroupBox);
}

void QtInstanceFrame::set_label(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pGroupBox->setTitle(vclToQtStringWithAccelerator(rText)); });
}

OUString QtInstanceFrame::get_label() const
{
    SolarMutexGuard g;

    OUString sLabel;
    GetQtInstance().RunInMainThread(
        [&] { sLabel = qtToVclStringWithAccelerator(m_pGroupBox->title()); });

    return sLabel;
}

std::unique_ptr<weld::Label> QtInstanceFrame::weld_label_widget() const
{
    assert(false && "Not implemented yet");
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

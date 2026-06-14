/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/dummyeditview.hxx>

#include <svl/itempool.hxx>
#include <vcl/virdev.hxx>

DummyEditView::DummyEditView()
{
    rtl::Reference<SfxItemPool> pItemPool = EditEngine::CreatePool();
    m_xEditEngine.reset(new EditEngine(pItemPool.get()));

    // Headless model - nothing is ever painted
    m_xVirDev = VclPtr<VirtualDevice>::Create();
    m_xVirDev->SetMapMode(MapMode(MapUnit::MapTwip));
    const Size aOutputSize(m_xVirDev->PixelToLogic(Size(500, 100)));
    m_xEditEngine->SetRefDevice(m_xVirDev.get());
    m_xEditEngine->SetPaperSize(aOutputSize);

    m_xEditEngine->SetControlWord(m_xEditEngine->GetControlWord() | EEControlBits::MARKFIELDS);

    m_xEditView.reset(new EditView(*m_xEditEngine, nullptr));
    m_xEditView->setEditViewCallbacks(this);
    m_xEditView->SetOutputArea(tools::Rectangle(Point(0, 0), aOutputSize));
    m_xEditEngine->InsertView(m_xEditView.get());
}

DummyEditView::~DummyEditView()
{
    if (m_xEditView && m_xEditEngine)
        m_xEditEngine->RemoveView(m_xEditView.get());
    m_xEditView.reset();
    m_xEditEngine.reset();
    m_xVirDev.disposeAndClear();
}

OutputDevice& DummyEditView::EditViewOutputDevice() const { return *m_xVirDev; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

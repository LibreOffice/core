/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/EnumContext.hxx>

#include <SparklineShell.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>

#define ShellClass_SparklineShell
#include <scslots.hxx>

namespace sc
{
SFX_IMPL_INTERFACE(SparklineShell, SfxShell)

void SparklineShell::InitInterface_Impl() { GetStaticInterface()->RegisterPopupMenu("sparkline"); }

SparklineShell::SparklineShell(ScTabViewShell* pViewShell)
    : SfxShell(pViewShell)
    , m_pViewShell(pViewShell)
{
    SetPool(&m_pViewShell->GetPool());
    ScViewData& rViewData = m_pViewShell->GetViewData();
    SfxUndoManager* pUndoManager = rViewData.GetSfxDocShell()->GetUndoManager();
    SetUndoManager(pUndoManager);
    if (!rViewData.GetDocument().IsUndoEnabled())
    {
        pUndoManager->SetMaxUndoActionCount(0);
    }
    SetName("Sparkline");
    SfxShell::SetContextName(
        vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Sparkline));
}

SparklineShell::~SparklineShell() = default;

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

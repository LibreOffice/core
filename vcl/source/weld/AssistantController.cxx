/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/svapp.hxx>
#include <vcl/weld/AssistantController.hxx>
#include <vcl/weld/Builder.hxx>

namespace weld
{
AssistantController::AssistantController(weld::Widget* pParent, const OUString& rUIFile,
                                         const OUString& rDialogId)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xAssistant(m_xBuilder->weld_assistant(rDialogId))
{
}

Assistant* AssistantController::getDialog() { return m_xAssistant.get(); }

AssistantController::~AssistantController() {}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

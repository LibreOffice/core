/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/Assistant.hxx>
#include <vcl/weld/DialogController.hxx>

namespace weld
{
class VCL_DLLPUBLIC AssistantController : public DialogController
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Assistant> m_xAssistant;

public:
    SAL_DLLPRIVATE AssistantController(weld::Widget* pParent, const OUString& rUIFile,
                                       const OUString& rDialogId);
    virtual Assistant* getDialog() override;
    SAL_DLLPRIVATE virtual ~AssistantController() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

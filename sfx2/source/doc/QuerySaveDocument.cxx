/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/QuerySaveDocument.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

short ExecuteQuerySaveDocument(weld::Widget* _pParent, std::u16string_view _rTitle)
{
    if (Application::IsHeadlessModeEnabled() || getenv("SAL_NO_QUERYSAVE"))
    {
        // don't block Desktop::terminate() if there's no user to ask
        return RET_NO;
    }

    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(_pParent, u"sfx/ui/querysavedialog.ui"_ustr));
    std::unique_ptr<weld::MessageDialog> xQBox(
        xBuilder->weld_message_dialog(u"QuerySaveDialog"_ustr));
    xQBox->set_primary_text(xQBox->get_primary_text().replaceFirst("$(DOC)", _rTitle));
    return xQBox->run();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

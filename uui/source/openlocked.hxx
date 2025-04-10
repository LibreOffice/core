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
#pragma once

#include <vcl/weld.hxx>

namespace vcl
{
class OpenLockedQueryBox final : public weld::GenericDialogController
{
public:
    OpenLockedQueryBox(weld::Window* pParent, const OUString& rHiddenData, bool bEnableOverride);

private:
    std::unique_ptr<weld::Image> mxQuestionMarkImage;

    std::unique_ptr<weld::Button> mxOpenReadOnlyBtn;
    std::unique_ptr<weld::Button> mxOpenCopyBtn;
    std::unique_ptr<weld::Button> mxOpenBtn;
    std::unique_ptr<weld::Button> mxCancelBtn;

    std::unique_ptr<weld::CheckButton> mxNotifyBtn;

    std::unique_ptr<weld::Label> mxHiddenText;

    DECL_LINK(ClickHdl, weld::Button&, void);
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

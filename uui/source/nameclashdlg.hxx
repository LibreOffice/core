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

enum NameClashResolveDialogResult { ABORT, RENAME, OVERWRITE };

class NameClashDialog : public weld::GenericDialogController
{
    OUString m_aSameName;
    OUString m_aNewName;

    std::unique_ptr<weld::Label> m_xFTMessage;
    std::unique_ptr<weld::Entry> m_xEDNewName;
    std::unique_ptr<weld::Button> m_xBtnOverwrite;
    std::unique_ptr<weld::Button> m_xBtnRename;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    DECL_LINK(ButtonHdl_Impl, weld::Button&, void);

public:
    NameClashDialog(weld::Window* pParent, const std::locale& rLocale,
                    OUString const & rTargetFolderURL,
                    OUString const & rClashingName,
                    OUString const & rProposedNewName,
                    bool bAllowOverwrite);
    virtual ~NameClashDialog() override;
    const OUString& getNewName() const { return m_aNewName; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

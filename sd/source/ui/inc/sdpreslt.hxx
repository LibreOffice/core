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

#include <vector>

#include <vcl/weld.hxx>

class SfxItemSet;
class ValueSet;
namespace weld { class CustomWeld; }

namespace sd {
class DrawDocShell;
}

class SdPresLayoutDlg
    : public weld::GenericDialogController
{
public:
    SdPresLayoutDlg(
        ::sd::DrawDocShell* pDocShell,
        weld::Window* pWindow,
        const SfxItemSet& rInAttrs);
    virtual ~SdPresLayoutDlg() override;

    void                GetAttr(SfxItemSet& rOutAttrs);

    DECL_LINK(ClickLayoutHdl, ValueSet*, void);
    DECL_LINK(ClickLoadHdl, weld::Button&, void);

private:
    ::sd::DrawDocShell* mpDocSh;

    const SfxItemSet&   mrOutAttrs;

    std::vector<OUString> maLayoutNames;

    OUString            maName;          ///< layout name or file name
    tools::Long                mnLayoutCount;   ///< number of master pages in the document
    const OUString      maStrNone;

    std::unique_ptr<weld::CheckButton> m_xCbxMasterPage;
    std::unique_ptr<weld::CheckButton> m_xCbxCheckMasters;
    std::unique_ptr<weld::Button> m_xBtnLoad;
    std::unique_ptr<ValueSet> m_xVS;
    std::unique_ptr<weld::CustomWeld> m_xVSWin;

    void                FillValueSet();
    void                Reset();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_SC_SOURCE_UI_INC_GRAPHSH_HXX
#define INCLUDED_SC_SOURCE_UI_INC_GRAPHSH_HXX

#include <memory>
#include <vector>
#include <sfx2/shell.hxx>
#include <shellids.hxx>

class SdrExternalToolEdit;
class ScViewData;
class SfxModule;

#include "drawsh.hxx"

class ScGraphicShell final : public ScDrawShell
{
public:
    SFX_DECL_INTERFACE(SCID_GRAPHIC_SHELL)

private:
    std::vector<std::unique_ptr<SdrExternalToolEdit>> m_ExternalEdits;

    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    ScGraphicShell(ScViewData& rData);
    virtual ~ScGraphicShell() override;

    void    Execute(SfxRequest& rReq);
    void    GetAttrState(SfxItemSet &rSet);

    void    ExecuteFilter(const SfxRequest& rReq);
    void    GetFilterState(SfxItemSet &rSet);

    void    ExecuteExternalEdit(SfxRequest& rReq);
    void    GetExternalEditState(SfxItemSet &rSet);

    void    ExecuteCompressGraphic(SfxRequest& rReq);
    void    GetCompressGraphicState(SfxItemSet &rSet);

    void    ExecuteCropGraphic(SfxRequest& rReq);
    void    GetCropGraphicState(SfxItemSet &rSet);

    void    ExecuteSaveGraphic(SfxRequest& rReq);
    void    GetSaveGraphicState(SfxItemSet &rSet);

    void    ExecuteChangePicture(SfxRequest& rReq);
    void    GetChangePictureState(SfxItemSet &rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

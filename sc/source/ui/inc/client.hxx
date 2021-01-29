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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CLIENT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CLIENT_HXX

#include <sfx2/ipclient.hxx>

class ScTabViewShell;
class SdrOle2Obj;
class SdrModel;

class ScClient final : public SfxInPlaceClient
{
private:
    SdrModel*       pModel;

    virtual void    ObjectAreaChanged() override;
    virtual void    RequestNewObjectArea( tools::Rectangle& ) override;
    virtual void    ViewChanged() override;

public:
                    ScClient( ScTabViewShell* pViewShell, vcl::Window* pDraw, SdrModel* pSdrModel, const SdrOle2Obj* pObj );
    virtual         ~ScClient() override;

    SdrOle2Obj*     GetDrawObj();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

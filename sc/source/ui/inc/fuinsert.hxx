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

#ifndef INCLUDED_SC_SOURCE_UI_INC_FUINSERT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FUINSERT_HXX

#include "fupoor.hxx"
#include "scdllapi.h"

class FuInsertGraphic : public FuPoor
{
 public:
    FuInsertGraphic( ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertGraphic();

    virtual void Activate() override;
    virtual void Deactivate() override;
};

class FuInsertOLE : public FuPoor
{
 public:
    FuInsertOLE( ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);

    virtual void Activate() override;
    virtual void Deactivate() override;
};

class FuInsertChart : public FuPoor
{
 public:
    FuInsertChart( ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);

    virtual void Activate() override;
    virtual void Deactivate() override;
};

class FuInsertChartFromFile : public FuPoor
{
 public:
    FuInsertChartFromFile( ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq, const OUString& rURL);

    virtual void Activate() override;
    virtual void Deactivate() override;
};

class FuInsertMedia : public FuPoor
{
 public:
    FuInsertMedia( ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertMedia();

    virtual void Activate() override;
    virtual void Deactivate() override;
};

void SC_DLLPUBLIC ScLimitSizeOnDrawPage( Size& rSize, Point& rPos, const Size& rPage );

#endif      // _SD_FUINSERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

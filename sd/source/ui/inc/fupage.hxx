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

#include <memory>
#include "fupoor.hxx"
#include <vcl/weld.hxx>

class SfxItemSet;
class SdBackgroundObjUndoAction;
class SdPage;

namespace sd {
class DrawViewShell;

class FuPage
    : public FuPoor
{
 public:

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq ) override;

    virtual void Activate() override;
    virtual void Deactivate() override;

    const SfxItemSet* ExecuteDialog(weld::Window* pParent, const SfxRequest& rReq);

protected:
    virtual ~FuPage() override;

private:
    FuPage (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq );

    void ApplyItemSet( const SfxItemSet* pArgs );

    SfxRequest&                 mrReq;
    const SfxItemSet*           mpArgs;
    std::unique_ptr<SdBackgroundObjUndoAction>
                                mpBackgroundObjUndoAction;
    Size                        maSize;
    bool                        mbPageBckgrdDeleted;
    bool                        mbMasterPage;
    bool                        mbDisplayBackgroundTabPage;
    SdPage*                     mpPage;
    DrawViewShell*              mpDrawViewShell;
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "fuconstr.hxx"

class E3dCompoundObject;
class E3dScene;
class SdDrawDocument;
class SfxRequest;

namespace sd {

class FuConstruct3dObject final
    : public FuConstruct
{
public:

    static rtl::Reference<FuPoor> Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq, bool bPermanent );
    virtual void DoExecute( SfxRequest& rReq ) override;

    // Mouse- & Key-Events
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    virtual void Activate() override;

    virtual rtl::Reference<SdrObject> CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle) override;

private:
    FuConstruct3dObject (
        ViewShell& rViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument& rDoc,
        SfxRequest& rReq);

    void ImpPrepareBasic3DShape(E3dCompoundObject const * p3DObj, E3dScene *pScene);
    rtl::Reference<E3dCompoundObject> ImpCreateBasic3DShape();
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

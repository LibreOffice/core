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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CONCUSTOMSHAPE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CONCUSTOMSHAPE_HXX

#include <rtl/ustring.hxx>
#include "drawbase.hxx"

class SdrObject;
class SfxRequest;

// draw rectangle
class ConstCustomShape final : public SwDrawBase
{

    OUString aCustomShape;

    void SetAttributes( SdrObject* pObj );

 public:

    ConstCustomShape( SwWrtShell* pSh, SwEditWin* pWin, SwView* pView, SfxRequest const & rReq );

                                       // Mouse- & Key-Events
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    virtual void Activate(const sal_uInt16 nSlotId) override;    // activate function

    const OUString& GetShapeType() const;
    static OUString GetShapeTypeFromRequest( SfxRequest const & rReq );

    virtual void CreateDefaultObject() override;

    // #i33136#
    virtual bool doConstructOrthogonal() const override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_CONCUSTOMSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

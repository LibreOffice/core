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

#ifndef INCLUDED_SC_SOURCE_UI_INC_FUCONUNO_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FUCONUNO_HXX

#include "fuconstr.hxx"

/** Draw Control */
class FuConstUnoControl : public FuConstruct
{
protected:
    sal_uInt32 nInventor;
    sal_uInt16 nIdentifier;

public:
    FuConstUnoControl(ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pView,
                       SdrModel* pDoc, SfxRequest& rReq);

    virtual ~FuConstUnoControl();
                                       // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual bool MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;

    virtual void Activate() SAL_OVERRIDE;
    virtual void Deactivate() SAL_OVERRIDE;

    // Create default drawing objects via keyboard
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle) SAL_OVERRIDE;
};

#endif      // _SD_FUCONCTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

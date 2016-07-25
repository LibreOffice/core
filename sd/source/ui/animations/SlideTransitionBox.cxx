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

#include "ViewShellBase.hxx"
#include <DrawDocShell.hxx>
#include <sfx2/viewfrm.hxx>
#include <SlideTransitionPane.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>

namespace sd
{

class SlideTransitionBox : public VclVBox
{
    VclPtr<SlideTransitionPane> m_pPane;
    bool m_bIsInitialized;

public:
    explicit SlideTransitionBox(vcl::Window* pParent);
    ~SlideTransitionBox() override;

    virtual void dispose() override;
    virtual void StateChanged(StateChangedType nStateChange) override;
};

VCL_BUILDER_FACTORY(SlideTransitionBox);

SlideTransitionBox::SlideTransitionBox(vcl::Window* pParent)
    : VclVBox(pParent)
    , m_bIsInitialized(false)
{
}

SlideTransitionBox::~SlideTransitionBox()
{
    disposeOnce();
}

void SlideTransitionBox::dispose()
{
    m_pPane.disposeAndClear();
    VclVBox::dispose();
}

void SlideTransitionBox::StateChanged(StateChangedType nStateChange)
{
    if(SfxViewFrame::Current() && !m_bIsInitialized)
    {
        ViewShellBase* pBase = ViewShellBase::GetViewShellBase(SfxViewFrame::Current());

        if(pBase && pBase->GetDocShell())
        {
            css::uno::Reference<css::frame::XFrame> xFrame;
            SdDrawDocument* pDoc = pBase->GetDocShell()->GetDoc();
            m_pPane = VclPtr<SlideTransitionPane>::Create(this, *pBase, pDoc, xFrame, false);
            m_pPane->Show();
            m_pPane->SetSizePixel(GetSizePixel());
            m_bIsInitialized = true;
        }
    }
    VclVBox::StateChanged(nStateChange);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

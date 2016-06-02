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
    Timer m_aLateInitTimer;
    VclPtr<SlideTransitionPane> m_pPane;

public:
    SlideTransitionBox(vcl::Window* pParent);
    ~SlideTransitionBox();

    virtual void dispose() override;

    DECL_LINK_TYPED(LateInitCallback, Timer *, void);
};

IMPL_LINK_NOARG_TYPED(SlideTransitionBox, LateInitCallback, Timer *, void)
{
    if(SfxViewFrame::Current())
    {
        ViewShellBase* pBase = ViewShellBase::GetViewShellBase(SfxViewFrame::Current());

        if(pBase && pBase->GetDocShell())
        {
            css::uno::Reference<css::frame::XFrame> xFrame;
            SdDrawDocument* pDoc = pBase->GetDocShell()->GetDoc();
            m_pPane = VclPtr<SlideTransitionPane>::Create(this, *pBase, pDoc, xFrame, false);
        }
    }
}

VCL_BUILDER_FACTORY(SlideTransitionBox);

SlideTransitionBox::SlideTransitionBox(vcl::Window* pParent)
    : VclVBox(pParent)
{
    m_aLateInitTimer.SetTimeout(200);
    m_aLateInitTimer.SetTimeoutHdl(LINK(this, SlideTransitionBox, LateInitCallback));
    m_aLateInitTimer.Start();
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

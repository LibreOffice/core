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

#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/viewfrm.hxx>
#include "OptionalBox.hxx"
#include "PriorityHBox.hxx"
#include "NotebookbarPopup.hxx"

/*
* PriorityMergedHBox is a VclHBox which hides its own children if there is no sufficient space.
*/

class SFX2_DLLPUBLIC PriorityMergedHBox : public PriorityHBox
{
private:
    VclPtr<PushButton> m_pButton;
    VclPtr<NotebookbarPopup> m_pPopup;

    DECL_LINK(PBClickHdl, Button*, void);

public:
    explicit PriorityMergedHBox(vcl::Window *pParent)
        : PriorityHBox(pParent)
    {
        m_pButton = VclPtr<PushButton>::Create(this, WB_FLATBUTTON);
        m_pButton->SetClickHdl(LINK(this, PriorityMergedHBox, PBClickHdl));
        m_pButton->SetSymbol(SymbolType::NEXT);
        m_pButton->set_width_request(15);
        m_pButton->set_pack_type(VclPackType::End);
        m_pButton->Show();
    }

    virtual ~PriorityMergedHBox() override
    {
        disposeOnce();
    }

    virtual void dispose() override
    {
        m_pButton.disposeAndClear();
        if (m_pPopup)
            m_pPopup.disposeAndClear();
        PriorityHBox::dispose();
    }
};

IMPL_LINK(PriorityMergedHBox, PBClickHdl, Button*, /*pButton*/, void)
{
    if (m_pPopup)
        m_pPopup.disposeAndClear();

    m_pPopup = VclPtr<NotebookbarPopup>::Create(this);

    for (int i = 0; i < GetChildCount(); i++)
    {
        if (GetChild(i) != m_pButton)
        {
            vcl::IPrioritable* pChild = dynamic_cast<vcl::IPrioritable*>(GetChild(i));

            if (pChild && pChild->IsHidden())
            {
                pChild->ShowContent();
                GetChild(i)->Show();
                GetChild(i)->SetParent(m_pPopup->getBox());
            }
        }
    }

    m_pPopup->hideSeparators(true);

    m_pPopup->getBox()->set_height_request(GetSizePixel().Height());

    long x = GetPosPixel().getX();
    long y = GetPosPixel().getY() + GetSizePixel().Height();
    tools::Rectangle aRect(x, y, x, y);

    m_pPopup->StartPopupMode(aRect, FloatWinPopupFlags::Down
        | FloatWinPopupFlags::GrabFocus
        | FloatWinPopupFlags::AllMouseButtonClose);
}

VCL_BUILDER_FACTORY(PriorityMergedHBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

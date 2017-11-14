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

/*
 * OptionalBox - shows or hides the content. To use with PriorityHBox
 * or PriorityMergedHBox
 */

OptionalBox::OptionalBox(vcl::Window* pParent)
    : VclHBox(pParent)
    , IPrioritable()
    , m_bInFullView(true)
{
}

OptionalBox::~OptionalBox() { disposeOnce(); }

void OptionalBox::HideContent()
{
    if (m_bInFullView)
    {
        m_bInFullView = false;

        for (int i = 0; i < GetChildCount(); i++)
            GetChild(i)->Hide();

        SetOutputSizePixel(Size(10, GetSizePixel().Height()));
    }
}

void OptionalBox::ShowContent()
{
    if (!m_bInFullView)
    {
        m_bInFullView = true;

        for (int i = 0; i < GetChildCount(); i++)
            GetChild(i)->Show();
    }
}

bool OptionalBox::IsHidden() { return !m_bInFullView; }

VCL_BUILDER_FACTORY(OptionalBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

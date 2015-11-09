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

#include "customcontrolcontainer.hxx"

#include <algorithm>


CCustomControlContainer::~CCustomControlContainer()
{
    RemoveAllControls();
}


void SAL_CALL CCustomControlContainer::Align()
{
    for (auto aCCustomControl : m_ControlContainer)
        aCCustomControl->Align();
}


void SAL_CALL CCustomControlContainer::SetFont(HFONT hFont)
{
    for (auto aCCustomControl : m_ControlContainer)
        aCCustomControl->SetFont(hFont);
}


void SAL_CALL CCustomControlContainer::AddControl(CCustomControl* aCustomControl)
{
    m_ControlContainer.push_back(aCustomControl);
}


void SAL_CALL CCustomControlContainer::RemoveControl(CCustomControl* aCustomControl)
{
    ControlContainer_t::iterator iter_end = m_ControlContainer.end();

    ControlContainer_t::iterator iter =
        std::find(m_ControlContainer.begin(),iter_end,aCustomControl);

    if (iter != iter_end)
    {
        delete *iter;
        m_ControlContainer.remove(aCustomControl);
    }
}


void SAL_CALL CCustomControlContainer::RemoveAllControls()
{
    for (auto aCCustomControl : m_ControlContainer)
        delete aCCustomControl;

    m_ControlContainer.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

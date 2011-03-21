/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"
#include "customcontrolcontainer.hxx"

#include <algorithm>
#include <functional>

//-----------------------------------
//
//-----------------------------------

namespace /* private */
{
    void DeleteCustomControl(CCustomControl* aCustomControl)
    {
        delete aCustomControl;
    };

    void AlignCustomControl(CCustomControl* aCustomControl)
    {
        aCustomControl->Align();
    };

    class CSetFontHelper
    {
    public:
        CSetFontHelper(HFONT hFont) :
            m_hFont(hFont)
        {
        }

        void SAL_CALL operator()(CCustomControl* aCustomControl)
        {
            aCustomControl->SetFont(m_hFont);
        }

    private:
        HFONT m_hFont;
    };
}

//-----------------------------------
//
//-----------------------------------

CCustomControlContainer::~CCustomControlContainer()
{
    RemoveAllControls();
}

//-----------------------------------
//
//-----------------------------------

void SAL_CALL CCustomControlContainer::Align()
{
    std::for_each(
        m_ControlContainer.begin(),
        m_ControlContainer.end(),
        AlignCustomControl);
}

//-----------------------------------
//
//-----------------------------------

void SAL_CALL CCustomControlContainer::SetFont(HFONT hFont)
{
    CSetFontHelper aSetFontHelper(hFont);

    std::for_each(
        m_ControlContainer.begin(),
        m_ControlContainer.end(),
        aSetFontHelper);
}

//-----------------------------------
//
//-----------------------------------

void SAL_CALL CCustomControlContainer::AddControl(CCustomControl* aCustomControl)
{
    m_ControlContainer.push_back(aCustomControl);
}

//-----------------------------------
//
//-----------------------------------

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

//-----------------------------------
//
//-----------------------------------

void SAL_CALL CCustomControlContainer::RemoveAllControls()
{
    std::for_each(
        m_ControlContainer.begin(),
        m_ControlContainer.end(),
        DeleteCustomControl);

    m_ControlContainer.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

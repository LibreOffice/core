/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

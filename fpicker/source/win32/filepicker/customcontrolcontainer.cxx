/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customcontrolcontainer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 17:55:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#ifndef _CUSTOMCONTROLCONTAINER_HXX_
#include "customcontrolcontainer.hxx"
#endif

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

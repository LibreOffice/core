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

#include <marktree.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;


OMarkableTreeListBox::OMarkableTreeListBox( vcl::Window* pParent, WinBits nWinStyle )
    : DBTreeListBox(pParent, nWinStyle)
{

    InitButtonData();
}

OMarkableTreeListBox::~OMarkableTreeListBox()
{
    disposeOnce();
}

void OMarkableTreeListBox::dispose()
{
    m_pCheckButton.reset();
    DBTreeListBox::dispose();
}

void OMarkableTreeListBox::InitButtonData()
{
    m_pCheckButton.reset( new SvLBoxButtonData( this ) );
    EnableCheckButton( m_pCheckButton.get() );
}

void OMarkableTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    // only if there are spaces
    if (rKEvt.GetKeyCode().GetCode() == KEY_SPACE && !rKEvt.GetKeyCode().IsShift() && !rKEvt.GetKeyCode().IsMod1())
    {
        SvTreeListEntry* pCurrentHandlerEntry = GetHdlEntry();
        if(pCurrentHandlerEntry)
        {
            SvButtonState eState = GetCheckButtonState( pCurrentHandlerEntry);
            if(eState == SvButtonState::Checked)
                SetCheckButtonState( pCurrentHandlerEntry, SvButtonState::Unchecked);
            else
                SetCheckButtonState( pCurrentHandlerEntry, SvButtonState::Checked);

            CheckButtonHdl();
        }
        else
            DBTreeListBox::KeyInput(rKEvt);
    }
    else
        DBTreeListBox::KeyInput(rKEvt);
}

SvButtonState OMarkableTreeListBox::implDetermineState(SvTreeListEntry* _pEntry)
{
    SvButtonState eState = GetCheckButtonState(_pEntry);
    if (!GetModel()->HasChildren(_pEntry))
        // nothing to do in this bottom-up routine if there are no children ...
        return eState;

    // loop through the children and check their states
    sal_uInt16 nCheckedChildren = 0;
    sal_uInt16 nChildrenOverall = 0;

    SvTreeListEntry* pChildLoop = GetModel()->FirstChild(_pEntry);
    while (pChildLoop)
    {
        SvButtonState eChildState = implDetermineState(pChildLoop);
        if (SvButtonState::Tristate == eChildState)
            break;

        if (SvButtonState::Checked == eChildState)
            ++nCheckedChildren;
        ++nChildrenOverall;

        pChildLoop = pChildLoop->NextSibling();
    }

    if (pChildLoop)
    {
        // we did not finish the loop because at least one of the children is in tristate
        eState = SvButtonState::Tristate;

        // but this means that we did not finish all the siblings of pChildLoop,
        // so their checking may be incorrect at the moment
        // -> correct this
        while (pChildLoop)
        {
            implDetermineState(pChildLoop);
            pChildLoop = pChildLoop->NextSibling();
        }
    }
    else
        // none if the children are in tristate
        if (nCheckedChildren)
            // we have at least one child checked
            if (nCheckedChildren != nChildrenOverall)
                // not all children are checked
                eState = SvButtonState::Tristate;
            else
                // all children are checked
                eState = SvButtonState::Checked;
        else
            // no children are checked
            eState = SvButtonState::Unchecked;

    // finally set the entry to the state we just determined
    SetCheckButtonState(_pEntry, eState);

    return eState;
}

void OMarkableTreeListBox::CheckButtons()
{
    SvTreeListEntry* pEntry = GetModel()->First();
    while (pEntry)
    {
        implDetermineState(pEntry);
        pEntry = pEntry->NextSibling();
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "marktree.hxx"
#include "dbu_control.hrc"
#include <vcl/svapp.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

#define SPACEBETWEENENTRIES     4

DBG_NAME(OMarkableTreeListBox)

OMarkableTreeListBox::OMarkableTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, WinBits nWinStyle )
    : DBTreeListBox(pParent,_rxORB,nWinStyle)
{
    DBG_CTOR(OMarkableTreeListBox,NULL);

    InitButtonData();
}

OMarkableTreeListBox::OMarkableTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, const ResId& rResId)
    : DBTreeListBox(pParent,_rxORB,rResId)
{
    DBG_CTOR(OMarkableTreeListBox,NULL);

    InitButtonData();
}

OMarkableTreeListBox::~OMarkableTreeListBox()
{
    delete m_pCheckButton;

    DBG_DTOR(OMarkableTreeListBox,NULL);
}

void OMarkableTreeListBox::Paint(const Rectangle& _rRect)
{
    if (!IsEnabled())
    {
        Font aOldFont = GetFont();
        Font aNewFont(aOldFont);

        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        aNewFont.SetColor(aSystemStyle.GetDisableColor());

        SetFont(aNewFont);
        DBTreeListBox::Paint(_rRect);
        SetFont(aOldFont);
    }
    else
        DBTreeListBox::Paint(_rRect);
}

void OMarkableTreeListBox::InitButtonData()
{
    m_pCheckButton = new SvLBoxButtonData( this );
    EnableCheckButton( m_pCheckButton );
}

void OMarkableTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    // only if there are spaces
    if (rKEvt.GetKeyCode().GetCode() == KEY_SPACE && !rKEvt.GetKeyCode().IsShift() && !rKEvt.GetKeyCode().IsMod1())
    {
        SvLBoxEntry* pCurrentHandlerEntry = GetHdlEntry();
        if(pCurrentHandlerEntry)
        {
            SvButtonState eState = GetCheckButtonState( pCurrentHandlerEntry);
            if(eState == SV_BUTTON_CHECKED)
                SetCheckButtonState( pCurrentHandlerEntry, SV_BUTTON_UNCHECKED);
            else
                SetCheckButtonState( pCurrentHandlerEntry, SV_BUTTON_CHECKED);

            CheckButtonHdl();
        }
        else
            DBTreeListBox::KeyInput(rKEvt);
    }
    else
        DBTreeListBox::KeyInput(rKEvt);
}

SvButtonState OMarkableTreeListBox::implDetermineState(SvLBoxEntry* _pEntry)
{
    SvButtonState eState = GetCheckButtonState(_pEntry);
    if (!GetModel()->HasChildren(_pEntry))
        // nothing to do in this bottom-up routine if there are no children ...
        return eState;
#ifdef DBG_UTIL
    String sEntryText  =GetEntryText(_pEntry);
#endif

    // loop through the children and check their states
    sal_uInt16 nCheckedChildren = 0;
    sal_uInt16 nChildrenOverall = 0;

    SvLBoxEntry* pChildLoop = GetModel()->FirstChild(_pEntry);
    while (pChildLoop)
    {
#ifdef DBG_UTIL
        String sChildText  =GetEntryText(pChildLoop);
#endif
        SvButtonState eChildState = implDetermineState(pChildLoop);
        if (SV_BUTTON_TRISTATE == eChildState)
            break;

        if (SV_BUTTON_CHECKED == eChildState)
            ++nCheckedChildren;
        ++nChildrenOverall;

        pChildLoop = GetModel()->NextSibling(pChildLoop);
    }

    if (pChildLoop)
    {
        // we did not finish the loop because at least one of the children is in tristate
        eState = SV_BUTTON_TRISTATE;

        // but this means that we did not finish all the siblings of pChildLoop,
        // so their checking may be incorrect at the moment
        // -> correct this
        while (pChildLoop)
        {
            implDetermineState(pChildLoop);
            pChildLoop = GetModel()->NextSibling(pChildLoop);
        }
    }
    else
        // none if the children are in tristate
        if (nCheckedChildren)
            // we have at least one child checked
            if (nCheckedChildren != nChildrenOverall)
                // not all children are checked
                eState = SV_BUTTON_TRISTATE;
            else
                // all children are checked
                eState = SV_BUTTON_CHECKED;
        else
            // no children are checked
            eState = SV_BUTTON_UNCHECKED;

    // finally set the entry to the state we just determined
    SetCheckButtonState(_pEntry, eState);

    return eState;
}

void OMarkableTreeListBox::CheckButtons()
{
    SvLBoxEntry* pEntry = GetModel()->First();
    while (pEntry)
    {
        implDetermineState(pEntry);
        pEntry = GetModel()->NextSibling(pEntry);
    }
}

void OMarkableTreeListBox::CheckButtonHdl()
{
    checkedButton_noBroadcast(GetHdlEntry());
    if (m_aCheckButtonHandler.IsSet())
        m_aCheckButtonHandler.Call(this);
}

void OMarkableTreeListBox::checkedButton_noBroadcast(SvLBoxEntry* _pEntry)
{
    SvButtonState eState = GetCheckButtonState( _pEntry);
    if (GetModel()->HasChildren(_pEntry)) // if it has children, check those too
    {
        SvLBoxEntry* pChildEntry = GetModel()->Next(_pEntry);
        SvLBoxEntry* pSiblingEntry = GetModel()->NextSibling(_pEntry);
        while(pChildEntry && pChildEntry != pSiblingEntry)
        {
            SetCheckButtonState(pChildEntry, eState);
            pChildEntry = GetModel()->Next(pChildEntry);
        }
    }

    SvLBoxEntry* pEntry = IsSelected(_pEntry) ? FirstSelected() : NULL;
    while(pEntry)
    {
        SetCheckButtonState(pEntry,eState);
        if(GetModel()->HasChildren(pEntry))   // if it has children, check those too
        {
            SvLBoxEntry* pChildEntry = GetModel()->Next(pEntry);
            SvLBoxEntry* pSiblingEntry = GetModel()->NextSibling(pEntry);
            while(pChildEntry && pChildEntry != pSiblingEntry)
            {
                SetCheckButtonState(pChildEntry,eState);
                pChildEntry = GetModel()->Next(pChildEntry);
            }
        }
        pEntry = NextSelected(pEntry);
    }
    CheckButtons();
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

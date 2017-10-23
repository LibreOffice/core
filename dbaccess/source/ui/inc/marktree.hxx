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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_MARKTREE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_MARKTREE_HXX

#include "dbtreelistbox.hxx"

#include <svtools/svlbitm.hxx>

namespace dbaui
{

// OMarkableTreeListBox
/** a tree list box where all entries can be marked (with a checkbox) and
    unmarked. In addition, inner nodes know a third state which applies
    if some, but not all of their descendants are marked.
*/
class OMarkableTreeListBox : public DBTreeListBox
{
    SvLBoxButtonData*   m_pCheckButton;
    Link<void*,void>    m_aCheckButtonHandler;

public:
    OMarkableTreeListBox( vcl::Window* pParent, WinBits nWinStyle=0 );
    virtual ~OMarkableTreeListBox() override;
    virtual void dispose() override;

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    CheckButtonHdl() override;
    void            CheckButtons();     // make the button states consistent (bottom-up)

    /// the handler given is called whenever the check state of one or more items changed
    void SetCheckHandler(const Link<void*,void>& _rHdl) { m_aCheckButtonHandler = _rHdl; }

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& _rRect) override;
    virtual void checkedButton_noBroadcast(SvTreeListEntry* _pEntry);

    SvButtonState   implDetermineState(SvTreeListEntry* _pEntry);
        // determines the check state of the given entry, by analyzing the states of all descendants

private:
    void InitButtonData();
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_MARKTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

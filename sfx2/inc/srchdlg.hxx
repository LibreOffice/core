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
#ifndef _SFX_SRCHDLG_HXX_
#define _SFX_SRCHDLG_HXX_

#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

// ============================================================================

namespace sfx2 {

// ============================================================================
// SearchDialog
// ============================================================================

class SearchDialog : public ModelessDialog
{
private:
    FixedText           m_aSearchLabel;
    ComboBox            m_aSearchEdit;
    CheckBox            m_aWholeWordsBox;
    CheckBox            m_aMatchCaseBox;
    CheckBox            m_aWrapAroundBox;
    CheckBox            m_aBackwardsBox;
    PushButton          m_aFindBtn;
    CancelButton        m_aCancelBtn;

    Link                m_aFindHdl;
    Link                m_aCloseHdl;

    OUString            m_sToggleText;
    OUString            m_sConfigName;
    OString             m_sWinState;

    bool                m_bIsConstructed;

    void                LoadConfig();
    void                SaveConfig();

    DECL_LINK(FindHdl, void *);
    DECL_LINK(ToggleHdl, void *);

public:
    SearchDialog( Window* pWindow, const OUString& rConfigName );
    ~SearchDialog();

    inline void         SetFindHdl( const Link& rLink ) { m_aFindHdl = rLink; }
    inline void         SetCloseHdl( const Link& rLink ) { m_aCloseHdl = rLink; }

    inline OUString     GetSearchText() const { return m_aSearchEdit.GetText(); }
    inline void         SetSearchText( const OUString& _rText ) { m_aSearchEdit.SetText( _rText ); }
    inline bool         IsOnlyWholeWords() const { return ( m_aWholeWordsBox.IsChecked() != sal_False ); }
    inline bool         IsMarchCase() const { return ( m_aMatchCaseBox.IsChecked() != sal_False ); }
    inline bool         IsWrapAround() const { return ( m_aWrapAroundBox.IsChecked() != sal_False ); }
    inline bool         IsSearchBackwards() const { return ( m_aBackwardsBox.IsChecked() != sal_False ); }

    void                SetFocusOnEdit();

    virtual sal_Bool        Close();
    virtual void        Move();
    virtual void        StateChanged( StateChangedType nStateChange );
};

// ============================================================================

} // namespace sfx2

// ============================================================================

#endif // _SFX_SRCHDLG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

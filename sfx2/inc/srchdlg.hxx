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
#ifndef INCLUDED_SFX2_INC_SRCHDLG_HXX
#define INCLUDED_SFX2_INC_SRCHDLG_HXX

#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>



namespace sfx2 {


// SearchDialog


class SearchDialog : public ModelessDialog
{
private:
    VclPtr<ComboBox>           m_pSearchEdit;
    VclPtr<CheckBox>           m_pWholeWordsBox;
    VclPtr<CheckBox>           m_pMatchCaseBox;
    VclPtr<CheckBox>           m_pWrapAroundBox;
    VclPtr<CheckBox>           m_pBackwardsBox;
    VclPtr<PushButton>         m_pFindBtn;

    Link<SearchDialog&,void>   m_aFindHdl;
    Link<SearchDialog*,void>   m_aCloseHdl;

    OUString            m_sConfigName;
    OString             m_sWinState;

    bool                m_bIsConstructed;

    void                LoadConfig();
    void                SaveConfig();

    DECL_LINK_TYPED(FindHdl, Button *, void);

public:
    SearchDialog( vcl::Window* pWindow, const OUString& rConfigName );
    virtual ~SearchDialog();
    virtual void dispose() SAL_OVERRIDE;

    void         SetFindHdl( const Link<SearchDialog&,void>& rLink ) { m_aFindHdl = rLink; }
    void         SetCloseHdl( const Link<SearchDialog*,void>& rLink ) { m_aCloseHdl = rLink; }

    OUString     GetSearchText() const { return m_pSearchEdit->GetText(); }
    void         SetSearchText( const OUString& _rText ) { m_pSearchEdit->SetText( _rText ); }
    bool         IsOnlyWholeWords() const { return ( m_pWholeWordsBox->IsChecked() ); }
    bool         IsMarchCase() const { return ( m_pMatchCaseBox->IsChecked() ); }
    bool         IsWrapAround() const { return ( m_pWrapAroundBox->IsChecked() ); }
    bool         IsSearchBackwards() const { return ( m_pBackwardsBox->IsChecked() ); }

    void            SetFocusOnEdit();

    virtual bool    Close() SAL_OVERRIDE;
    virtual void    Move() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
};



} // namespace sfx2



#endif // INCLUDED_SFX2_INC_SRCHDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

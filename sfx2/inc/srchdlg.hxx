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

#include <sfx2/basedlgs.hxx>

namespace sfx2 {


// SearchDialog


class SearchDialog : public weld::GenericDialogController
{
private:
    Link<SearchDialog&,void>   m_aFindHdl;
    Link<LinkParamNone*,void>  m_aCloseHdl;

    OUString const            m_sConfigName;

    std::unique_ptr<weld::ComboBox> m_xSearchEdit;
    std::unique_ptr<weld::CheckButton> m_xWholeWordsBox;
    std::unique_ptr<weld::CheckButton> m_xMatchCaseBox;
    std::unique_ptr<weld::CheckButton> m_xWrapAroundBox;
    std::unique_ptr<weld::CheckButton> m_xBackwardsBox;
    std::unique_ptr<weld::Button> m_xFindBtn;

    void                LoadConfig();
    void                SaveConfig();

    DECL_LINK(FindHdl, weld::Button&, void);

public:
    SearchDialog(weld::Window* pWindow, const OUString& rConfigName);
    static void runAsync(const std::shared_ptr<SearchDialog>& rController);
    virtual ~SearchDialog() override;

    void         SetFindHdl( const Link<SearchDialog&,void>& rLink ) { m_aFindHdl = rLink; }
    void         SetCloseHdl( const Link<LinkParamNone*,void>& rLink ) { m_aCloseHdl = rLink; }

    OUString     GetSearchText() const { return m_xSearchEdit->get_active_text(); }
    void         SetSearchText( const OUString& _rText ) { m_xSearchEdit->set_entry_text( _rText ); }
    bool         IsOnlyWholeWords() const { return m_xWholeWordsBox->get_active(); }
    bool         IsMarchCase() const { return m_xMatchCaseBox->get_active(); }
    bool         IsWrapAround() const { return m_xWrapAroundBox->get_active(); }
    bool         IsSearchBackwards() const { return m_xBackwardsBox->get_active(); }

    void         SetFocusOnEdit();
};

} // namespace sfx2


#endif // INCLUDED_SFX2_INC_SRCHDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

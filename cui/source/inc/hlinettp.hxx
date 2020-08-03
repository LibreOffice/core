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
#pragma once

#include "cuihyperdlg.hxx"
#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Internet
|*
\************************************************************************/

class SvxHyperlinkInternetTp : public SvxHyperlinkTabPageBase
{
private:
    OUString            maStrOldUser;
    OUString            maStrOldPassword;

    bool                m_bMarkWndOpen;

    std::unique_ptr<weld::RadioButton> m_xRbtLinktypInternet;
    std::unique_ptr<weld::RadioButton> m_xRbtLinktypFTP;
    std::unique_ptr<SvxHyperURLBox> m_xCbbTarget;
    std::unique_ptr<weld::Label> m_xFtTarget;
    std::unique_ptr<weld::Label> m_xFtLogin;
    std::unique_ptr<weld::Entry> m_xEdLogin;
    std::unique_ptr<weld::Label> m_xFtPassword;
    std::unique_ptr<weld::Entry> m_xEdPassword;
    std::unique_ptr<weld::CheckButton> m_xCbAnonymous;

    DECL_LINK( Click_SmartProtocol_Impl, weld::Button&, void ); ///< Radiobutton clicked: Type HTTP or FTP
    DECL_LINK( ClickAnonymousHdl_Impl, weld::Button&, void ); ///< Checkbox : Anonymous User
    DECL_LINK( ModifiedLoginHdl_Impl, weld::Entry&,   void ); ///< Contents of editfield "Login" modified
    DECL_LINK( LostFocusTargetHdl_Impl, weld::Widget&, void ); ///< Combobox "Target" lost its focus
    DECL_LINK( ModifiedTargetHdl_Impl, weld::ComboBox&, void );    ///< Contents of editfield "Target" modified

    DECL_LINK( TimeoutHdl_Impl,             Timer *, void); ///< Handler for timer -timeout


    void    SetScheme(const OUString& rScheme);
    void    RemoveImproperProtocol(const OUString& rProperScheme);
    OUString  GetSchemeFromButtons() const;
    INetProtocol GetSmartProtocolFromButtons() const;

    OUString CreateAbsoluteURL() const;

    void   setAnonymousFTPUser();
    void   setFTPUser(const OUString& rUser, const OUString& rPassword);
    void   RefreshMarkWindow();

protected:
    virtual void FillDlgFields(const OUString& rStrURL) override;
    virtual void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                     OUString& aStrIntName, OUString& aStrFrame,
                                     SvxLinkInsertMode& eMode ) override;
    virtual bool ShouldOpenMarkWnd () override {return ( m_bMarkWndOpen && m_xRbtLinktypInternet->get_active() );}
    virtual void SetMarkWndShouldOpen (bool bOpen) override {m_bMarkWndOpen=bOpen;}

public:
    SvxHyperlinkInternetTp(weld::Container* pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);
    virtual ~SvxHyperlinkInternetTp() override;

    static std::unique_ptr<IconChoicePage> Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);

    virtual void        SetMarkStr ( const OUString& aStrMark ) override;

    virtual void        SetInitFocus() override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

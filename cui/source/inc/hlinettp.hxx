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
#ifndef INCLUDED_CUI_SOURCE_INC_HLINETTP_HXX
#define INCLUDED_CUI_SOURCE_INC_HLINETTP_HXX

#include <svx/hyperdlg.hxx>
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
    RadioButton         *m_pRbtLinktypInternet;
    RadioButton         *m_pRbtLinktypFTP;
    SvxHyperURLBox      *m_pCbbTarget;
    PushButton          *m_pBtBrowse;
    FixedText           *m_pFtLogin;
    Edit                *m_pEdLogin;
    FixedText           *m_pFtPassword;
    Edit                *m_pEdPassword;
    CheckBox            *m_pCbAnonymous;

    OUString            maStrOldUser;
    OUString            maStrOldPassword;

    bool                mbMarkWndOpen;

    DECL_LINK (Click_SmartProtocol_Impl  , void * ); ///< Radiobutton clicked: Type HTTP or FTP
    DECL_LINK (ClickAnonymousHdl_Impl    , void * ); ///< Checkbox : Anonymous User
    DECL_LINK (ClickBrowseHdl_Impl       , void * ); ///< Button : Browse
    DECL_LINK (ModifiedLoginHdl_Impl     , void * ); ///< Contens of editfield "Login" modified
    DECL_LINK (LostFocusTargetHdl_Impl   , void * ); ///< Combobox "Target" lost its focus
    DECL_LINK (ModifiedTargetHdl_Impl    , void * ); ///< Contens of editfield "Target" modified

    DECL_LINK(TimeoutHdl_Impl           , void *); ///< Handler for timer -timeout


    void    SetScheme(const OUString& rScheme);
    void    RemoveImproperProtocol(const OUString& rProperScheme);
    OUString  GetSchemeFromButtons() const;
    INetProtocol GetSmartProtocolFromButtons() const;

    OUString CreateAbsoluteURL() const;

    void   setAnonymousFTPUser();
    void   setFTPUser(const OUString& rUser, const OUString& rPassword);
    void   RefreshMarkWindow();

protected:
    virtual void FillDlgFields(const OUString& rStrURL) SAL_OVERRIDE;
    virtual void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                     OUString& aStrIntName, OUString& aStrFrame,
                                     SvxLinkInsertMode& eMode ) SAL_OVERRIDE;
    virtual bool ShouldOpenMarkWnd () SAL_OVERRIDE {return ( mbMarkWndOpen && m_pRbtLinktypInternet->IsChecked() );}
    virtual void SetMarkWndShouldOpen (bool bOpen) SAL_OVERRIDE {mbMarkWndOpen=bOpen;}

public:
    SvxHyperlinkInternetTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet);
    virtual ~SvxHyperlinkInternetTp ();

    static  IconChoicePage* Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( const OUString& aStrMark ) SAL_OVERRIDE;

    virtual void        SetInitFocus() SAL_OVERRIDE;
};


#endif // INCLUDED_CUI_SOURCE_INC_HLINETTP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

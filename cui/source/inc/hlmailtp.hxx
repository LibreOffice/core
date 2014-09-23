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
#ifndef INCLUDED_CUI_SOURCE_INC_HLMAILTP_HXX
#define INCLUDED_CUI_SOURCE_INC_HLMAILTP_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Mail & News
|*
\************************************************************************/

class SvxHyperlinkMailTp : public SvxHyperlinkTabPageBase
{
private:
    RadioButton         *m_pRbtMail;
    RadioButton         *m_pRbtNews;
    SvxHyperURLBox      *m_pCbbReceiver;
    PushButton          *m_pBtAdrBook;
    FixedText           *m_pFtSubject;
    Edit                *m_pEdSubject;

    DECL_LINK (Click_SmartProtocol_Impl, void * ); ///< Radiobutton clicked: Type EMAIl or News
    DECL_LINK (ClickAdrBookHdl_Impl  , void * ); ///< Button : Address book
    DECL_LINK (ModifiedReceiverHdl_Impl, void * ); ///< Combobox "receiver" modified

    void    SetScheme(const OUString& rScheme);
    void    RemoveImproperProtocol(const OUString& aProperScheme);
    OUString  GetSchemeFromButtons() const;
    INetProtocol GetSmartProtocolFromButtons() const;

    OUString  CreateAbsoluteURL() const;

protected:
    virtual void FillDlgFields(const OUString& rStrURL) SAL_OVERRIDE;
    virtual void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                     OUString& aStrIntName, OUString& aStrFrame,
                                     SvxLinkInsertMode& eMode ) SAL_OVERRIDE;

public:
    SvxHyperlinkMailTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet);
    virtual ~SvxHyperlinkMailTp ();

    static  IconChoicePage* Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet );

    virtual void        SetInitFocus() SAL_OVERRIDE;
};


#endif // INCLUDED_CUI_SOURCE_INC_HLMAILTP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

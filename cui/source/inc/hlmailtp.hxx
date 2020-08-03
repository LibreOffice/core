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

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Mail
|*
\************************************************************************/

class SvxHyperlinkMailTp : public SvxHyperlinkTabPageBase
{
private:
    std::unique_ptr<SvxHyperURLBox> m_xCbbReceiver;
    std::unique_ptr<weld::Button> m_xBtAdrBook;
    std::unique_ptr<weld::Label> m_xFtSubject;
    std::unique_ptr<weld::Entry> m_xEdSubject;

    DECL_STATIC_LINK(SvxHyperlinkMailTp, ClickAdrBookHdl_Impl, weld::Button&, void);
        ///< Button : Address book
    DECL_LINK (ModifiedReceiverHdl_Impl, weld::ComboBox&, void ); ///< Combobox "receiver" modified

    void    SetScheme(const OUString& rScheme);
    void    RemoveImproperProtocol(const OUString& aProperScheme);

    OUString  CreateAbsoluteURL() const;

protected:
    virtual void FillDlgFields(const OUString& rStrURL) override;
    virtual void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                     OUString& aStrIntName, OUString& aStrFrame,
                                     SvxLinkInsertMode& eMode ) override;

public:
    SvxHyperlinkMailTp(weld::Container* pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);
    virtual ~SvxHyperlinkMailTp() override;

    static std::unique_ptr<IconChoicePage> Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);

    virtual void        SetInitFocus() override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

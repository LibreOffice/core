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
|* Tabpage : Hyperlink - New Document
|*
\************************************************************************/
class SvxHyperlinkNewDocTp : public SvxHyperlinkTabPageBase
{
private:
    std::unique_ptr<weld::RadioButton> m_xRbtEditNow;
    std::unique_ptr<weld::RadioButton> m_xRbtEditLater;
    std::unique_ptr<SvxHyperURLBox> m_xCbbPath;
    std::unique_ptr<weld::Button> m_xBtCreate;
    std::unique_ptr<weld::TreeView> m_xLbDocTypes;

    bool                ImplGetURLObject( const OUString& rPath, const OUString& rBase, INetURLObject& aURLObject ) const;
    void                FillDocumentList ();

    DECL_LINK (ClickNewHdl_Impl, weld::Button&, void );
    DECL_STATIC_LINK(SvxHyperlinkNewDocTp, DispatchDocument, void*, void);

protected:
    void FillDlgFields(const OUString& rStrURL) override;
    void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                             OUString& aStrIntName, OUString& aStrFrame,
                             SvxLinkInsertMode& eMode ) override;

public:
    SvxHyperlinkNewDocTp(weld::Container* pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);
    virtual ~SvxHyperlinkNewDocTp () override;

    static std::unique_ptr<IconChoicePage> Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);

    virtual bool        AskApply () override;
    virtual void        DoApply () override;

    virtual void        SetInitFocus() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

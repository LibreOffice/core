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
#ifndef INCLUDED_CUI_SOURCE_INC_HLDOCNTP_HXX
#define INCLUDED_CUI_SOURCE_INC_HLDOCNTP_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - New Document
|*
\************************************************************************/

class SvxHyperlinkNewDocTp : public SvxHyperlinkTabPageBase
{
private:
    VclPtr<RadioButton>         m_pRbtEditNow;
    VclPtr<RadioButton>         m_pRbtEditLater;
    VclPtr<SvxHyperURLBox>      m_pCbbPath;
    VclPtr<PushButton>          m_pBtCreate;
    VclPtr<ListBox>             m_pLbDocTypes;

    bool                ImplGetURLObject( const OUString& rPath, const OUString& rBase, INetURLObject& aURLObject ) const;
    void                FillDocumentList ();

    DECL_LINK_TYPED (ClickNewHdl_Impl, Button*, void );

protected:
    void FillDlgFields(const OUString& rStrURL) override;
    void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                             OUString& aStrIntName, OUString& aStrFrame,
                             SvxLinkInsertMode& eMode ) override;

public:
    SvxHyperlinkNewDocTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet);
    virtual ~SvxHyperlinkNewDocTp ();
    virtual void dispose() override;

    static  VclPtr<IconChoicePage> Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet );

    virtual bool        AskApply () override;
    virtual void        DoApply () override;

    virtual void        SetInitFocus() override;
};


#endif // INCLUDED_CUI_SOURCE_INC_HLDOCNTP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

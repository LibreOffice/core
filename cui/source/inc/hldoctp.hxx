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
|* Tabpage : Hyperlink - Document
|*
\************************************************************************/
class SvxHyperlinkDocTp final : public SvxHyperlinkTabPageBase
{
private:
    std::unique_ptr<SvxHyperURLBox> m_xCbbPath;
    std::unique_ptr<weld::Button> m_xBtFileopen;
    std::unique_ptr<weld::Entry> m_xEdTarget;
    std::unique_ptr<weld::Label> m_xFtFullURL;
    std::unique_ptr<weld::Button> m_xBtBrowse;

    OUString            maStrURL;

    bool                m_bMarkWndOpen;

    DECL_LINK (ClickFileopenHdl_Impl, weld::Button&, void );
    DECL_LINK (ClickTargetHdl_Impl, weld::Button&, void );

    DECL_LINK (ModifiedPathHdl_Impl, weld::ComboBox&, void ); ///< Contents of combobox "Path" modified
    DECL_LINK (ModifiedTargetHdl_Impl, weld::Entry&, void ); ///< Contents of editfield "Target" modified

    DECL_LINK( LostFocusPathHdl_Impl, weld::Widget&, void ); ///< Combobox "path" lost its focus

    DECL_LINK( TimeoutHdl_Impl, Timer *, void ); ///< Handler for timer -timeout

    enum class EPathType { Invalid, ExistsFile };
    static EPathType GetPathType ( const OUString& rStrPath );

    void FillDlgFields(const OUString& rStrURL) override;
    void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                             OUString& aStrIntName, OUString& aStrFrame,
                             SvxLinkInsertMode& eMode ) override;
    virtual bool   ShouldOpenMarkWnd () override {return m_bMarkWndOpen;}
    virtual void   SetMarkWndShouldOpen (bool bOpen) override {m_bMarkWndOpen=bOpen;}
    OUString GetCurrentURL() const;

public:
    SvxHyperlinkDocTp(weld::Container* pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);
    virtual ~SvxHyperlinkDocTp() override;

    static std::unique_ptr<IconChoicePage> Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet);

    virtual void        SetMarkStr ( const OUString& aStrMark ) override;

    virtual void        SetInitFocus() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

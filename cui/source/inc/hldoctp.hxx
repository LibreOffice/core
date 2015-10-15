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
#ifndef INCLUDED_CUI_SOURCE_INC_HLDOCTP_HXX
#define INCLUDED_CUI_SOURCE_INC_HLDOCTP_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Document
|*
\************************************************************************/

class SvxHyperlinkDocTp : public SvxHyperlinkTabPageBase
{
private:
    VclPtr<SvxHyperURLBox>      m_pCbbPath;
    VclPtr<PushButton>          m_pBtFileopen;

    VclPtr<Edit>                m_pEdTarget;
    VclPtr<FixedText>           m_pFtFullURL;
    VclPtr<PushButton>          m_pBtBrowse;

    OUString            maStrURL;

    bool                mbMarkWndOpen;

    DECL_LINK_TYPED (ClickFileopenHdl_Impl, Button*, void );
    DECL_LINK_TYPED (ClickTargetHdl_Impl  , Button*, void );

    DECL_LINK_TYPED (ModifiedPathHdl_Impl  , Edit&, void ); ///< Contents of combobox "Path" modified
    DECL_LINK_TYPED (ModifiedTargetHdl_Impl, Edit&, void ); ///< Contents of editfield "Target" modified

    DECL_LINK_TYPED( LostFocusPathHdl_Impl, Control&, void ); ///< Combobox "path" lost its focus

    DECL_LINK_TYPED( TimeoutHdl_Impl, Timer *, void ); ///< Handler for timer -timeout

    enum EPathType { Type_Unknown, Type_Invalid,
                     Type_ExistsFile, Type_File,
                     Type_ExistsDir, Type_Dir };
    static EPathType GetPathType ( const OUString& rStrPath );

protected:
    void FillDlgFields(const OUString& rStrURL) override;
    void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                             OUString& aStrIntName, OUString& aStrFrame,
                             SvxLinkInsertMode& eMode ) override;
    virtual bool   ShouldOpenMarkWnd () override {return mbMarkWndOpen;}
    virtual void   SetMarkWndShouldOpen (bool bOpen) override {mbMarkWndOpen=bOpen;}
    OUString GetCurrentURL    ();

public:
    SvxHyperlinkDocTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet);
    virtual ~SvxHyperlinkDocTp();
    virtual void dispose() override;

    static VclPtr<IconChoicePage> Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( const OUString& aStrMark ) override;

    virtual void        SetInitFocus() override;
};


#endif // INCLUDED_CUI_SOURCE_INC_HLDOCTP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

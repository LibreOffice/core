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
    SvxHyperURLBox      *m_pCbbPath;
    PushButton          *m_pBtFileopen;

    Edit                *m_pEdTarget;
    FixedText           *m_pFtFullURL;
    PushButton          *m_pBtBrowse;

    OUString            maStrURL;

    bool                mbMarkWndOpen;

    DECL_LINK (ClickFileopenHdl_Impl  , void * );
    DECL_LINK (ClickTargetHdl_Impl    , void * );

    DECL_LINK (ModifiedPathHdl_Impl  , void * ); ///< Contens of combobox "Path" modified
    DECL_LINK (ModifiedTargetHdl_Impl, void * ); ///< Contens of editfield "Target" modified

    DECL_LINK (LostFocusPathHdl_Impl,  void * ); ///< Combobox "path" lost its focus

    DECL_LINK(TimeoutHdl_Impl      , void *);  ///< Handler for timer -timeout

    enum EPathType { Type_Unknown, Type_Invalid,
                     Type_ExistsFile, Type_File,
                     Type_ExistsDir, Type_Dir };
    EPathType GetPathType ( const OUString& rStrPath );

protected:
    void FillDlgFields(const OUString& rStrURL) SAL_OVERRIDE;
    void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                             OUString& aStrIntName, OUString& aStrFrame,
                             SvxLinkInsertMode& eMode ) SAL_OVERRIDE;
    virtual bool   ShouldOpenMarkWnd () SAL_OVERRIDE {return mbMarkWndOpen;}
    virtual void   SetMarkWndShouldOpen (bool bOpen) SAL_OVERRIDE {mbMarkWndOpen=bOpen;}
    OUString GetCurrentURL    ();

public:
    SvxHyperlinkDocTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet);
    virtual ~SvxHyperlinkDocTp ();

    static  IconChoicePage* Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( const OUString& aStrMark ) SAL_OVERRIDE;

    virtual void        SetInitFocus() SAL_OVERRIDE;
};


#endif // INCLUDED_CUI_SOURCE_INC_HLDOCTP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

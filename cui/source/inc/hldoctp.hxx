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
#ifndef _SVX_TABPAGE_DOC_HYPERLINK_HXX
#define _SVX_TABPAGE_DOC_HYPERLINK_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Document
|*
\************************************************************************/

class SvxHyperlinkDocTp : public SvxHyperlinkTabPageBase
{
private:
    FixedLine           maGrpDocument;
    FixedText           maFtPath;
    SvxHyperURLBox      maCbbPath;
    ImageButton         maBtFileopen;

    FixedLine           maGrpTarget;
    FixedText           maFtTarget;
    Edit                maEdTarget;
    FixedText           maFtURL;
    FixedText           maFtFullURL;
    ImageButton         maBtBrowse;

    OUString            maStrURL;

    sal_Bool                mbMarkWndOpen;

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
    void FillDlgFields     ( String& aStrURL );
    void GetCurentItemData ( OUString& rStrURL, String& aStrName,
                             String& aStrIntName, String& aStrFrame,
                             SvxLinkInsertMode& eMode );
    virtual sal_Bool   ShouldOpenMarkWnd () {return mbMarkWndOpen;}
    virtual void   SetMarkWndShouldOpen (sal_Bool bOpen) {mbMarkWndOpen=bOpen;}
    String GetCurrentURL    ();

public:
    SvxHyperlinkDocTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkDocTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( const String& aStrMark );

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_DOC_HYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

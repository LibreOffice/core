/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

    String              maStrURL;

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
    EPathType GetPathType ( String& aStrPath );

protected:
    void FillDlgFields     ( String& aStrURL );
    void GetCurentItemData ( String& aStrURL, String& aStrName,
                             String& aStrIntName, String& aStrFrame,
                             SvxLinkInsertMode& eMode );
    virtual sal_Bool   ShouldOpenMarkWnd () {return mbMarkWndOpen;}
    virtual void   SetMarkWndShouldOpen (sal_Bool bOpen) {mbMarkWndOpen=bOpen;}
    String GetCurrentURL    ();

public:
    SvxHyperlinkDocTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkDocTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( String& aStrMark );

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_DOC_HYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef _SVX_TABPAGE_DOCNEW_HYPERLINK_HXX
#define _SVX_TABPAGE_DOCNEW_HYPERLINK_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - New Document
|*
\************************************************************************/

class SvxHyperlinkNewDocTp : public SvxHyperlinkTabPageBase
{
private:
    FixedLine           maGrpNewDoc;
    RadioButton         maRbtEditNow;
    RadioButton         maRbtEditLater;
    FixedText           maFtPath;
    SvxHyperURLBox      maCbbPath;
    ImageButton         maBtCreate;
    FixedText           maFtDocTypes;
    ListBox             maLbDocTypes;

    sal_Bool            ImplGetURLObject( const String& rPath, const String& rBase, INetURLObject& aURLObject ) const;
    void                FillDocumentList ();

    DECL_LINK (ClickNewHdl_Impl          , void * );

    Image GetImage( sal_uInt16 nId );

protected:
    void FillDlgFields     ( String& aStrURL );
    void GetCurentItemData ( String& aStrURL, String& aStrName,
                             String& aStrIntName, String& aStrFrame,
                             SvxLinkInsertMode& eMode );

public:
    SvxHyperlinkNewDocTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkNewDocTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual sal_Bool        AskApply ();
    virtual void        DoApply ();

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_DOCNEW_HYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

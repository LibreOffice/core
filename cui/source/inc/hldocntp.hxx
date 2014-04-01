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
    FixedLine           maGrpNewDoc;
    RadioButton         maRbtEditNow;
    RadioButton         maRbtEditLater;
    FixedText           maFtPath;
    SvxHyperURLBox      maCbbPath;
    ImageButton         maBtCreate;
    FixedText           maFtDocTypes;
    ListBox             maLbDocTypes;

    sal_Bool            ImplGetURLObject( const OUString& rPath, const OUString& rBase, INetURLObject& aURLObject ) const;
    void                FillDocumentList ();

    DECL_LINK (ClickNewHdl_Impl          , void * );

    Image GetImage( sal_uInt16 nId );

protected:
    void FillDlgFields(const OUString& rStrURL) SAL_OVERRIDE;
    void GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                             OUString& aStrIntName, OUString& aStrFrame,
                             SvxLinkInsertMode& eMode ) SAL_OVERRIDE;

public:
    SvxHyperlinkNewDocTp ( Window *pParent, const SfxItemSet& rItemSet);
    virtual ~SvxHyperlinkNewDocTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual sal_Bool        AskApply () SAL_OVERRIDE;
    virtual void        DoApply () SAL_OVERRIDE;

    virtual void        SetInitFocus() SAL_OVERRIDE;
};


#endif // INCLUDED_CUI_SOURCE_INC_HLDOCNTP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

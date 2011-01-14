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

    DECL_LINK (ClickNewHdl_Impl          , void * );        // Button : New

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

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
#ifndef _BOOKMARK_HXX
#define _BOOKMARK_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>

#include "swlbox.hxx"       // SwComboBox

class SwWrtShell;
class SfxRequest;

class BookmarkCombo : public SwComboBox
{
    sal_uInt16          GetFirstSelEntryPos() const;
    sal_uInt16          GetNextSelEntryPos(sal_uInt16 nPos) const;
    sal_uInt16          GetSelEntryPos(sal_uInt16 nPos) const;

    virtual long    PreNotify(NotifyEvent& rNEvt);
public:
    BookmarkCombo( Window* pWin );
    BookmarkCombo( Window* pWin, const ResId& rResId );

    sal_uInt16          GetSelectEntryCount() const;
    sal_uInt16          GetSelectEntryPos( sal_uInt16 nSelIndex = 0 ) const;

    static const String aForbiddenChars;
};

class SwInsertBookmarkDlg: public SvxStandardDialog
{
    BookmarkCombo*  m_pBookmarkBox;
    OKButton*       m_pOkBtn;
    PushButton*     m_pDeleteBtn;

    String          sRemoveWarning;
    SwWrtShell      &rSh;
    SfxRequest&     rReq;

    DECL_LINK(ModifyHdl, BookmarkCombo *);
    DECL_LINK(DeleteHdl, void *);

    virtual void Apply();

public:
    SwInsertBookmarkDlg(Window *pParent, SwWrtShell &rSh, SfxRequest& rReq);
    ~SwInsertBookmarkDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

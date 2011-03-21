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

#ifndef _SWTEXTSH_HXX
#define _SWTEXTSH_HXX

#include <basesh.hxx>

class AbstractSvxPostItDialog;
class SwFldMgr;
class SvxHyperlinkItem;

class SwTextShell: public SwBaseShell
{
    SwFldMgr*   pPostItFldMgr;
    sal_uInt32  nF3ShiftCounter;

    void InsertSymbol( SfxRequest& );
    void InsertHyperlink(const SvxHyperlinkItem& rHlnkItem);
    bool InsertMediaDlg( SfxRequest& );
    void ChangeHeaderOrFooter(const String& rStyleName, sal_Bool bHeader, sal_Bool bOn, sal_Bool bShowWarning);

public:
    SFX_DECL_INTERFACE(SW_TEXTSHELL)
    TYPEINFO();

    DECL_LINK( RedlineNextHdl, AbstractSvxPostItDialog * );
    DECL_LINK( RedlinePrevHdl, AbstractSvxPostItDialog * );

    void    Execute(SfxRequest &);
    void    GetState(SfxItemSet &);

    void    ExecInsert(SfxRequest &);
    void    StateInsert(SfxItemSet&);
    void    ExecDelete(SfxRequest &);
    void    ExecEnterNum(SfxRequest &);
    void    ExecBasicMove(SfxRequest &);
    void    ExecMove(SfxRequest &);
    void    ExecMovePage(SfxRequest &);
    void    ExecMoveCol(SfxRequest &);
    void    ExecMoveLingu(SfxRequest &);
    void    ExecMoveMisc(SfxRequest &);
    void    ExecField(SfxRequest &rReq);
    void    StateField(SfxItemSet &);
    void    ExecIdx(SfxRequest &);
    void    GetIdxState(SfxItemSet &);
    void    ExecGlossary(SfxRequest &);

    void    ExecCharAttr(SfxRequest &);
    void    ExecCharAttrArgs(SfxRequest &);
    void    ExecParaAttr(SfxRequest &);
    void    ExecParaAttrArgs(SfxRequest &);
    void    ExecAttr(SfxRequest &);
    void    ExecDB(SfxRequest &);
    void    ExecTransliteration(SfxRequest &);
    void    ExecRotateTransliteration(SfxRequest &);

    void    GetAttrState(SfxItemSet &);

             SwTextShell(SwView &rView);
    virtual ~SwTextShell();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

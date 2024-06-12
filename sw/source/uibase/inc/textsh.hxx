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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_TEXTSH_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_TEXTSH_HXX

#include "basesh.hxx"
#include <unotools/caserotate.hxx>
#include <com/sun/star/ui/dialogs/DialogClosedEvent.hpp>

class AbstractSvxPostItDialog;
class SwFieldMgr;
class SwFlyFrameAttrMgr;
class SvxHyperlinkItem;
class SwInsertChart;

class SW_DLLPUBLIC SwTextShell: public SwBaseShell
{
    RotateTransliteration m_aRotateCase;

    void InsertSymbol( SfxRequest& );
    void InsertHyperlink( const SvxHyperlinkItem& rHlnkItem );
    bool InsertMediaDlg( SfxRequest const & );

public:
    SFX_DECL_INTERFACE(SW_TEXTSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    DECL_DLLPRIVATE_LINK( RedlineNextHdl, AbstractSvxPostItDialog&, void );
    DECL_DLLPRIVATE_LINK( RedlinePrevHdl, AbstractSvxPostItDialog&, void );
    DECL_DLLPRIVATE_STATIC_LINK( SwTextShell, DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void );

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
    void    ExecSetNumber(SfxRequest const &);
    void    StateField(SfxItemSet &);
    void    ExecIdx(SfxRequest const &);
    void    GetIdxState(SfxItemSet &);
    void    ExecGlossary(SfxRequest &);

    void    ExecCharAttr(SfxRequest &);
    void    ExecCharAttrArgs(SfxRequest &);
    void    ExecParaAttr(SfxRequest &);
    void    ExecParaAttrArgs(SfxRequest &);
    void    ExecDB(SfxRequest const &);
    void    ExecTransliteration(SfxRequest &);
    void    ExecRotateTransliteration(SfxRequest &);

    void    GetAttrState(SfxItemSet &);

             SwTextShell(SwView &rView);
    virtual ~SwTextShell() override;
    /// Create item set for the insert frame dialog.
    SfxItemSet CreateInsertFrameItemSet(SwFlyFrameAttrMgr& rMgr);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

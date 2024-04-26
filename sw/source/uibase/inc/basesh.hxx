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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_BASESH_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_BASESH_HXX

#include <shellid.hxx>

#include <tools/link.hxx>
#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>
#include <mdiexp.hxx>
#include <memory>
#include <set>
#include <vector>

class SwPaM;
class SwWrtShell;
class SwView;
class SfxItemSet;
class SwCursorShell;

struct DBTextStruct_Impl;
class SW_DLLPUBLIC SwBaseShell: public SfxShell
{
    SwView      &m_rView;

    // DragMode
    static FlyMode s_eFrameMode;

    // Bug 75078 - if in GetState the async call of GetGraphic returns
    //              synch, the set the state directly into the itemset
    SfxItemSet*         m_pGetStateSet;

    // Update-Timer for graphic
    std::set<sal_uInt16> m_aGrfUpdateSlots;

    DECL_DLLPRIVATE_LINK( GraphicArrivedHdl, SwCursorShell&, void );

protected:
    SwWrtShell&         GetShell();
    SwWrtShell*         GetShellPtr();

    SwView&      GetView()                       { return m_rView; }
    void         SetGetStateSet( SfxItemSet* p ) { m_pGetStateSet = p; }
    bool         AddGrfUpdateSlot( sal_uInt16 nSlot ){ return m_aGrfUpdateSlots.insert( nSlot ).second; }

    DECL_DLLPRIVATE_LINK(    InsertDBTextHdl, void*, void );

    void                InsertURLButton( const OUString& rURL, const OUString& rTarget, const OUString& rText );
    void                InsertTable( SfxRequest& _rRequest );

public:
    SwBaseShell(SwView &rShell);
    virtual     ~SwBaseShell() override;

    SFX_DECL_INTERFACE(SW_BASESHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    void        ExecDelete(SfxRequest &);

    void        ExecClpbrd(SfxRequest &);
    void        StateClpbrd(SfxItemSet &);

    void        ExecUndo(SfxRequest &);
    void        StateUndo(SfxItemSet &);

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        StateStyle(SfxItemSet &);

    void        ExecuteGallery(SfxRequest&);
    void        GetGalleryState(SfxItemSet&);

    void        ExecDlg(SfxRequest &);

    void        ExecTextCtrl(SfxRequest& rReq);
    void        GetTextFontCtrlState(SfxItemSet& rSet);
    void        GetTextCtrlState(SfxItemSet& rSet);
    void        GetBorderState(SfxItemSet &rSet);
    void        GetBckColState(SfxItemSet &rSet);

    void        ExecBckCol(SfxRequest& rReq);
    void        SetWrapMode( sal_uInt16 nSlot );

    static void StateDisableItems(SfxItemSet &);

    void        EditRegionDialog(SfxRequest const & rReq);
    void        InsertRegionDialog(SfxRequest& rReq);

    void        ExecField(SfxRequest const & rReq);

    static void    SetFrameMode( FlyMode eMode, SwWrtShell *pShell );  // with update!
    static void   SetFrameMode_( FlyMode eMode )   { s_eFrameMode = eMode; }
    static FlyMode  GetFrameMode()                 { return s_eFrameMode;  }

    // duplicate rOrig and rOrig's multi-selection Ring so the first element of the returned
    // vector can be used equivalently to rOrig to affect the same selections
    static std::shared_ptr<std::vector<std::unique_ptr<SwPaM>>> CopyPaMRing(SwPaM& rOrig);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#pragma once

#include <sfx2/shell.hxx>
#include <shellids.hxx>
#include <unotools/caserotate.hxx>
#include <tools/link.hxx>
#include <memory>
#include "formatsh.hxx"
#include <rtl/ref.hxx>
#include <sot/formats.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>
#include <tools/solar.h>

class SvxClipboardFormatItem;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractScLinkedAreaDlg;

struct CellShell_Impl
{
    rtl::Reference<TransferableClipboardListener>
                                    m_xClipEvtLstnr;
    VclPtr<AbstractScLinkedAreaDlg> m_pLinkedDlg;
    SfxRequest*                     m_pRequest;

    CellShell_Impl();
    ~CellShell_Impl();
};

class ScCellShell final : public ScFormatShell
{
private:
    std::unique_ptr<CellShell_Impl> pImpl;
    bool            bPastePossible;

    void        GetPossibleClipboardFormats( SvxClipboardFormatItem& rFormats );
    bool        HasClipboardFormat( SotClipboardFormatId nFormatId );
    void        ExecuteExternalSource(
                    const OUString& _rFile, const OUString& _rFilter, const OUString& _rOptions,
                    const OUString& _rSource, sal_uLong _nRefresh, SfxRequest& _rRequest );

    void ExecuteDataPilotDialog();
    void ExecuteXMLSourceDialog();
    void ExecuteSubtotals(SfxRequest& rReq);

    void ExecuteFillSingleEdit();

    DECL_LINK( ClipboardChanged, TransferableDataHelper*, void );

    RotateTransliteration m_aRotateCase;

    VclPtr<vcl::Window> pFrameWin;

public:
    SFX_DECL_INTERFACE(SCID_CELL_SHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                ScCellShell( ScViewData& rData, const VclPtr<vcl::Window>& pFrameWin );
    virtual     ~ScCellShell() override;

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);

    void        ExecuteEdit( SfxRequest& rReq );
    void        ExecuteTrans( SfxRequest& rReq );
    void        ExecuteRotateTrans( const SfxRequest& rReq );

    void        GetBlockState( SfxItemSet& rSet );
    void        GetCellState( SfxItemSet& rSet );

    void        ExecuteDB( SfxRequest& rReq );
    void        GetDBState( SfxItemSet& rSet );

    void        GetClipState( SfxItemSet& rSet );
    void        GetHLinkState( SfxItemSet& rSet );

    void        ExecuteCursor( SfxRequest& rReq );
    void        ExecuteCursorSel( SfxRequest& rReq );
    void        ExecutePage( SfxRequest& rReq );
    void        ExecutePageSel( SfxRequest& rReq );
    void        ExecuteMove( SfxRequest& rReq );

    const VclPtr<vcl::Window>& GetFrameWin() const;

    static void GetStateCursor( SfxItemSet& rSet );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

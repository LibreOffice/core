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

#ifndef SC_CELLSH_HXX
#define SC_CELLSH_HXX

#include <sfx2/shell.hxx>
#include "shellids.hxx"
#include <sfx2/module.hxx>
#include <svx/svdmark.hxx>
#include <tools/link.hxx>
#include "formatsh.hxx"
#include "address.hxx"

class SvxClipboardFmtItem;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractScLinkedAreaDlg;
class ScTabViewShell;

struct CellShell_Impl
{
    TransferableClipboardListener*  m_pClipEvtLstnr;
    AbstractScLinkedAreaDlg*        m_pLinkedDlg;
    SfxRequest*                     m_pRequest;

    CellShell_Impl() :
        m_pClipEvtLstnr( NULL ),
        m_pLinkedDlg( NULL ),
        m_pRequest( NULL ) {}
};

class ScCellShell: public ScFormatShell
{
private:
    CellShell_Impl* pImpl;
    sal_Bool            bPastePossible;

    void        GetPossibleClipboardFormats( SvxClipboardFmtItem& rFormats );
    void        ExecuteExternalSource(
                    const String& _rFile, const String& _rFilter, const String& _rOptions,
                    const String& _rSource, sal_uLong _nRefresh, SfxRequest& _rRequest );

    void ExecuteDataPilotDialog();

    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
    DECL_LINK( DialogClosed, AbstractScLinkedAreaDlg* );

public:

    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_CELL_SHELL)

                ScCellShell(ScViewData* pData);
    virtual     ~ScCellShell();

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);

    void        ExecuteEdit( SfxRequest& rReq );
    void        ExecuteTrans( SfxRequest& rReq );

    void        GetBlockState( SfxItemSet& rSet );
    void        GetCellState( SfxItemSet& rSet );

    void        ExecuteDB( SfxRequest& rReq );
    void        GetDBState( SfxItemSet& rSet );

    void        ExecImageMap(SfxRequest& rReq);     // ImageMap
    void        GetImageMapState(SfxItemSet& rSet);

    void        GetClipState( SfxItemSet& rSet );
    void        GetHLinkState( SfxItemSet& rSet );

    void        ExecuteCursor( SfxRequest& rReq );
    void        ExecuteCursorSel( SfxRequest& rReq );
    void        ExecutePage( SfxRequest& rReq );
    void        ExecutePageSel( SfxRequest& rReq );
    void        ExecuteMove( SfxRequest& rReq );
    void        GetStateCursor( SfxItemSet& rSet );

    static void PasteFromClipboard( ScViewData* pViewData, ScTabViewShell* pTabViewShell, bool bShowDialog );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

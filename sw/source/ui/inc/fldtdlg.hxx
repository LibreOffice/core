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
#ifndef _SWFLDTDLG_HXX
#define _SWFLDTDLG_HXX
#include <sfx2/tabdlg.hxx>

class SfxBindings;
class SfxTabPage;
class SwChildWinWrapper;
struct SfxChildWinInfo;

class SwFldDlg: public SfxTabDialog
{
    SwChildWinWrapper*  m_pChildWin;
    SfxBindings*        m_pBindings;
    BOOL                m_bHtmlMode;
    BOOL                m_bDataBaseMode;

    virtual BOOL        Close();
    virtual SfxItemSet* CreateInputItemSet( USHORT nId );
    virtual void        Activate();
    virtual void        PageCreated(USHORT nId, SfxTabPage& rPage);

    void                ReInitTabPage( USHORT nPageId,
                                        BOOL bOnlyActivate = FALSE );

public:
    SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent);
    virtual ~SwFldDlg();

    DECL_LINK( OKHdl, Button * );

    void                Initialize(SfxChildWinInfo *pInfo);
    void                ReInitDlg();
    void                EnableInsert(BOOL bEnable);
    void                InsertHdl();
    void                ActivateDatabasePage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

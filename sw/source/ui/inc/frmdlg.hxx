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

#ifndef _FRMDLG_HXX
#define _FRMDLG_HXX

#include "globals.hrc"
#include <sfx2/tabdlg.hxx>
class SwWrtShell;

/*--------------------------------------------------------------------
   Description: frame dialog
 --------------------------------------------------------------------*/
class SwFrmDlg : public SfxTabDialog
{
    sal_Bool                m_bFormat;
    sal_Bool                m_bNew;
    sal_Bool                m_bHTMLMode;
    const SfxItemSet&   m_rSet;
    sal_uInt16              m_nDlgType;
    SwWrtShell*         m_pWrtShell;


    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:
    SwFrmDlg(   SfxViewFrame *pFrame, Window *pParent,
                const SfxItemSet& rCoreSet,
                sal_Bool            bNewFrm  = sal_True,
                sal_uInt16          nResType = DLG_FRM_STD,
                sal_Bool            bFmt     = sal_False,
                sal_uInt16          nDefPage = 0,
                const String*   pFmtStr  = 0);

    ~SwFrmDlg();

    inline SwWrtShell*  GetWrtShell()   { return m_pWrtShell; }
};

#endif // _FRMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

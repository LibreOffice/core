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
   Beschreibung:    Rahmendialog
 --------------------------------------------------------------------*/

class SwFrmDlg : public SfxTabDialog
{
    BOOL                m_bFormat;
    BOOL                m_bNew;
    BOOL                m_bHTMLMode;
    bool                m_bEnableVertPos;
    const SfxItemSet&   m_rSet;
    USHORT              m_nDlgType;
    SwWrtShell*         m_pWrtShell;


    virtual void PageCreated( USHORT nId, SfxTabPage &rPage );

public:
    SwFrmDlg(   SfxViewFrame *pFrame, Window *pParent,
                const SfxItemSet& rCoreSet,
                BOOL            bNewFrm  = TRUE,
                USHORT          nResType = DLG_FRM_STD,
                BOOL            bFmt     = FALSE,
                UINT16          nDefPage = 0,
                const String*   pFmtStr  = 0);

    ~SwFrmDlg();

    inline SwWrtShell*  GetWrtShell()   { return m_pWrtShell; }
};


#endif // _FRMDLG_HXX

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

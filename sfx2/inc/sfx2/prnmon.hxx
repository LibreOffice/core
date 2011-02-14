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
#ifndef _SFX_PRNMON_HXX
#define _SFX_PRNMON_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <sfx2/printer.hxx>
//#include <sfx2/progress.hxx>

class SfxViewShell;
//class SfxProgress;
struct SfxPrintProgress_Impl;

// ------------------------------------------------------------------------

#define PAGE_MAX    9999        //max. Anzahl der Seiten die gedruckt werden

//--------------------------------------------------------------------
/*
class SFX2_DLLPUBLIC SfxPrintProgress: public SfxProgress
{
    SfxPrintProgress_Impl*  pImp;

private:
//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( PrintErrorNotify, void * );
    DECL_DLLPRIVATE_LINK( StartPrintNotify, void * );
    DECL_DLLPRIVATE_LINK( EndPrintNotify, void * );
//#endif
public:
                            SfxPrintProgress( SfxViewShell* pViewSh,
                                              FASTBOOL bShow = sal_True );
    virtual                 ~SfxPrintProgress();

    virtual void            SetText( const String &rText );
            sal_Bool            SetStateText( sal_uIntPtr nVal, const String &rVal, sal_uIntPtr nNewRange = 0 );
    virtual sal_Bool            SetState( sal_uIntPtr nVal, sal_uIntPtr nNewRange = 0 );

    void                    RestoreOnEndPrint( SfxPrinter *pOldPrinter );
    void                    RestoreOnEndPrint( SfxPrinter *pOldPrinter,
                                               sal_Bool bOldEnablePrintFile );
    void                    DeleteOnEndPrint();
    void                    SetCancelHdl( const Link& aCancelHdl );
    sal_Bool                    IsAborted() const;
};
*/
// ------------------------------------------------------------------------

struct SfxPrintOptDlg_Impl;
class SfxPrintOptionsDialog : public ModalDialog
{
private:
    OKButton                aOkBtn;
    CancelButton            aCancelBtn;
    HelpButton              aHelpBtn;
    SfxPrintOptDlg_Impl*    pDlgImpl;
    SfxViewShell*           pViewSh;
    SfxItemSet*             pOptions;
    SfxTabPage*             pPage;

public:
                            SfxPrintOptionsDialog( Window *pParent,
                                                   SfxViewShell *pViewShell,
                                                   const SfxItemSet *rOptions );
    virtual                 ~SfxPrintOptionsDialog();

    sal_Bool                    Construct();
    virtual short           Execute();
    virtual long            Notify( NotifyEvent& rNEvt );

    SfxTabPage*             GetTabPage() const { return pPage; }
    const SfxItemSet&       GetOptions() const { return *pOptions; }
    void                    DisableHelp();
};

#endif

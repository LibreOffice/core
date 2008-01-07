/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prnmon.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 09:02:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFX_PRNMON_HXX
#define _SFX_PRNMON_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <sfx2/printer.hxx>
#include <sfx2/progress.hxx>

class SfxViewShell;
class SfxProgress;
struct SfxPrintProgress_Impl;

// ------------------------------------------------------------------------

#define PAGE_MAX    9999        //max. Anzahl der Seiten die gedruckt werden

//--------------------------------------------------------------------

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
                                              FASTBOOL bShow = TRUE );
    virtual                 ~SfxPrintProgress();

    virtual void            SetText( const String &rText );
            BOOL            SetStateText( ULONG nVal, const String &rVal, ULONG nNewRange = 0 );
    virtual BOOL            SetState( ULONG nVal, ULONG nNewRange = 0 );

    void                    RestoreOnEndPrint( SfxPrinter *pOldPrinter );
    void                    RestoreOnEndPrint( SfxPrinter *pOldPrinter,
                                               BOOL bOldEnablePrintFile );
    void                    DeleteOnEndPrint();
    void                    SetCancelHdl( const Link& aCancelHdl );
    BOOL                    IsAborted() const;
};

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

    BOOL                    Construct();
    virtual short           Execute();
    virtual long            Notify( NotifyEvent& rNEvt );

    SfxTabPage*             GetTabPage() const { return pPage; }
    const SfxItemSet&       GetOptions() const { return *pOptions; }
    void                    DisableHelp();
};

#endif

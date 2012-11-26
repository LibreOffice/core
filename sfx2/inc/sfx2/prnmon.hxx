/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
                                              bool bShow = sal_True );
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

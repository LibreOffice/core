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


#ifndef _BASEDLGS_HXX
#define _BASEDLGS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#ifndef _FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif
#include <vcl/timer.hxx>
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

class TabPage;
class SfxTabPage;
class SfxBindings;
class SfxChildWindow;
struct SfxChildWinInfo;
class SfxItemSet;
class SfxItemPool;
class OKButton;
class CancelButton;
class HelpButton;
class Button;
class FixedLine;

// class SfxModalDefParentHelper -----------------------------------------

class SfxModalDefParentHelper
{
private:
    Window *pOld;

public:
    SfxModalDefParentHelper(Window* pWindow);
    ~SfxModalDefParentHelper();
};

// class SfxModalDialog --------------------------------------------------

class SFX2_DLLPUBLIC SfxModalDialog: public ModalDialog
{
    sal_uInt32              nUniqId;
    String                  aExtraData;
    const SfxItemSet*       pInputSet;
    SfxItemSet*             pOutputSet;

private:
    SAL_DLLPRIVATE SfxModalDialog(SfxModalDialog &); // not defined
    SAL_DLLPRIVATE void operator =(SfxModalDialog &); // not defined

    SAL_DLLPRIVATE void SetDialogData_Impl();
    SAL_DLLPRIVATE void GetDialogData_Impl();
    SAL_DLLPRIVATE void init();

protected:
    SfxModalDialog(Window *pParent, const ResId& );
    SfxModalDialog(Window* pParent, sal_uInt32 nUniqueId, WinBits nWinStyle = WB_STDMODAL );

    String&             GetExtraData()      { return aExtraData; }
    sal_uInt32          GetUniqId() const   { return nUniqId; }
    SfxItemSet*         GetItemSet()        { return pOutputSet; }
    void                CreateOutputItemSet( SfxItemPool& rPool );
    void                CreateOutputItemSet( const SfxItemSet& rInput );
    void                SetInputSet( const SfxItemSet* pInSet ) { pInputSet = pInSet; }
    SfxItemSet*         GetOutputSetImpl() { return pOutputSet; }

public:
    ~SfxModalDialog();
    const SfxItemSet*   GetOutputItemSet() const { return pOutputSet; }
    const SfxItemSet*   GetInputItemSet() const { return pInputSet; }
};

// class SfxModelessDialog --------------------------------------------------
class SfxModelessDialog_Impl;
class SFX2_DLLPUBLIC SfxModelessDialog: public ModelessDialog
{
    SfxBindings*            pBindings;
    Size                    aSize;
    SfxModelessDialog_Impl* pImp;

    SAL_DLLPRIVATE SfxModelessDialog(SfxModelessDialog &); // not defined
    SAL_DLLPRIVATE void operator =(SfxModelessDialog &); // not defined

protected:
                            SfxModelessDialog( SfxBindings*, SfxChildWindow*,
                                Window*, const ResId& );
                            SfxModelessDialog( SfxBindings*, SfxChildWindow*,
                                Window*, WinBits nWinStyle = WB_STDMODELESS );
                            ~SfxModelessDialog();
    virtual sal_Bool            Close();
    virtual void            Resize();
    virtual void            Move();
    virtual void            StateChanged( StateChangedType nStateChange );

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo* pInfo);
    virtual long            Notify( NotifyEvent& rNEvt );
    SfxBindings&            GetBindings()
                            { return *pBindings; }

    DECL_LINK( TimerHdl, Timer* );

};

// class SfxFloatingWindow --------------------------------------------------
class SfxFloatingWindow_Impl;
class SFX2_DLLPUBLIC SfxFloatingWindow: public FloatingWindow
{
    SfxBindings*            pBindings;
    Size                    aSize;
    SfxFloatingWindow_Impl* pImp;

    SAL_DLLPRIVATE SfxFloatingWindow(SfxFloatingWindow &); // not defined
    SAL_DLLPRIVATE void operator =(SfxFloatingWindow &); // not defined

protected:
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              Window* pParent,
                                              WinBits nWinBits=WB_STDMODELESS);
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              Window* pParent,
                                              const ResId& rResId);
                            ~SfxFloatingWindow();

    virtual void            StateChanged( StateChangedType nStateChange );
    virtual sal_Bool            Close();
    virtual void            Resize();
    virtual void            Move();
    virtual long            Notify( NotifyEvent& rNEvt );
    SfxBindings&            GetBindings()
                            { return *pBindings; }

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo* pInfo);

    DECL_LINK( TimerHdl, Timer* );

};

// class SfxSingleTabDialog --------------------------------------------------

namespace svt { class FixedHyperlinkImage; }

struct SingleTabDlgImpl
{
    TabPage*                    m_pTabPage;
    SfxTabPage*                 m_pSfxPage;
    FixedLine*                  m_pLine;
    ::svt::FixedHyperlinkImage* m_pInfoImage;
    String                      m_sInfoURL;
    Link                        m_aInfoLink;

    SingleTabDlgImpl() :
        m_pTabPage( NULL ), m_pSfxPage( NULL ), m_pLine( NULL ), m_pInfoImage( NULL ) {}
};

typedef sal_uInt16* (*GetTabPageRanges)(); // liefert internationale Which-Werte

class SFX2_DLLPUBLIC SfxSingleTabDialog : public SfxModalDialog
{
public:
    SfxSingleTabDialog( Window* pParent, const SfxItemSet& rOptionsSet, sal_uInt16 nUniqueId );
    SfxSingleTabDialog( Window* pParent, sal_uInt16 nUniqueId, const SfxItemSet* pInSet = 0 );
    SfxSingleTabDialog( Window* pParent, sal_uInt16 nUniqueId, const String& rInfoURL );

    virtual             ~SfxSingleTabDialog();

    void                SetPage( TabPage* pNewPage );
    void                SetTabPage( SfxTabPage* pTabPage, GetTabPageRanges pRangesFunc = 0 );
    SfxTabPage*         GetTabPage() const { return pImpl->m_pSfxPage; }

    const sal_uInt16*       GetInputRanges( const SfxItemPool& rPool );
//  void                SetInputSet( const SfxItemSet* pInSet ) { pOptions = pInSet; }
//  const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }
    OKButton*           GetOKButton() const { return pOKBtn; }
    CancelButton*       GetCancelButton() const { return pCancelBtn; }
    void                SetInfoLink( const Link& rLink );

private:
    GetTabPageRanges    fnGetRanges;
    sal_uInt16*             pRanges;

    OKButton*           pOKBtn;
    CancelButton*       pCancelBtn;
    HelpButton*         pHelpBtn;

    SingleTabDlgImpl*   pImpl;
//  const SfxItemSet*   pOptions;
//  SfxItemSet*         pOutSet;

    DECL_DLLPRIVATE_LINK( OKHdl_Impl, Button * );
};

#endif


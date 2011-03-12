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
#ifndef _BASEDLGS_HXX
#define _BASEDLGS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/dialog.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

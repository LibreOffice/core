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
#ifndef INCLUDED_SFX2_BASEDLGS_HXX
#define INCLUDED_SFX2_BASEDLGS_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <vcl/builder.hxx>
#include <vcl/dialog.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>

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

//There is a hack which hides the HelpId for a Dialog in SfxModelessDialog
//and SfxDockingWindow, where it is changed into a UniqueId and cleared
//This reverses the clear of the HelpId

inline void reverseUniqueHelpIdHack(vcl::Window &rWindow)
{
    if (rWindow.GetHelpId().isEmpty())
        rWindow.SetHelpId(rWindow.GetUniqueId());
}

// class SfxModalDialog --------------------------------------------------

class SFX2_DLLPUBLIC SfxModalDialog: public ModalDialog
{
    sal_uInt32              nUniqId;
    OUString                aExtraData;
    const SfxItemSet*       pInputSet;
    SfxItemSet*             pOutputSet;

private:
    SfxModalDialog(SfxModalDialog &) = delete;
    void operator =(SfxModalDialog &) = delete;

    SAL_DLLPRIVATE void SetDialogData_Impl();
    SAL_DLLPRIVATE void GetDialogData_Impl();
    SAL_DLLPRIVATE void init();

protected:
    SfxModalDialog(vcl::Window *pParent, const OUString& rID, const OUString& rUIXMLDescription);

    OUString&           GetExtraData()      { return aExtraData; }
    sal_uInt32          GetUniqId() const   { return nUniqId; }
    void                SetUniqId(sal_uInt32 nSettingsId)  { nUniqId = nSettingsId; }
    SfxItemSet*         GetItemSet()        { return pOutputSet; }
    void                CreateOutputItemSet( SfxItemPool& rPool );
    void                CreateOutputItemSet( const SfxItemSet& rInput );
    void                SetInputSet( const SfxItemSet* pInSet ) { pInputSet = pInSet; }
    SfxItemSet*         GetOutputSetImpl() { return pOutputSet; }

public:
    virtual ~SfxModalDialog();
    virtual void dispose() SAL_OVERRIDE;
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

    SfxModelessDialog(SfxModelessDialog &) = delete;
    void operator =(SfxModelessDialog &) = delete;

    void Init(SfxBindings *pBindinx, SfxChildWindow *pCW);

protected:
    SfxModelessDialog( SfxBindings*, SfxChildWindow*,
        vcl::Window*, const OUString& rID, const OUString& rUIXMLDescription );
    virtual ~SfxModelessDialog();
    virtual void dispose() SAL_OVERRIDE;
    virtual bool            Close() SAL_OVERRIDE;
    virtual void            Resize() SAL_OVERRIDE;
    virtual void            Move() SAL_OVERRIDE;
    virtual void            StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo* pInfo);
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    SfxBindings&            GetBindings()
                            { return *pBindings; }

    DECL_LINK_TYPED(TimerHdl, Idle *, void);

};

// class SfxFloatingWindow --------------------------------------------------
class SfxFloatingWindow_Impl;
class SFX2_DLLPUBLIC SfxFloatingWindow: public FloatingWindow
{
    SfxBindings*            pBindings;
    Size                    aSize;
    SfxFloatingWindow_Impl* pImp;

    SfxFloatingWindow(SfxFloatingWindow &) = delete;
    void operator =(SfxFloatingWindow &) = delete;

protected:
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              vcl::Window* pParent,
                                              WinBits nWinBits=WB_STDMODELESS);
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              vcl::Window* pParent,
                                              const OString& rID, const OUString& rUIXMLDescription,
                                              const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
                            virtual ~SfxFloatingWindow();
    virtual void            dispose() SAL_OVERRIDE;

    virtual void            StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
    virtual bool            Close() SAL_OVERRIDE;
    virtual void            Resize() SAL_OVERRIDE;
    virtual void            Move() SAL_OVERRIDE;
    virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    SfxBindings&            GetBindings()
                            { return *pBindings; }

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo* pInfo);

    DECL_LINK_TYPED(TimerHdl, Idle *, void);

};

// class SfxNoLayoutSingleTabDialog --------------------------------------------------

struct SingleTabDlgImpl
{
    VclPtr<SfxTabPage>          m_pSfxPage;
    VclPtr<FixedLine>           m_pLine;

    SingleTabDlgImpl();
};

typedef const sal_uInt16* (*GetTabPageRanges)(); // liefert internationale Which-Werte

class SFX2_DLLPUBLIC SfxSingleTabDialog : public SfxModalDialog
{
public:
    SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet& rOptionsSet,
        const OUString& rID = OUString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet* pInSet = 0,
        const OUString& rID = OUString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    virtual             ~SfxSingleTabDialog();
    virtual void        dispose() SAL_OVERRIDE;

    void                SetTabPage(SfxTabPage* pTabPage, GetTabPageRanges pRangesFunc = 0, sal_uInt32 nSettingsId = 0);
    SfxTabPage*         GetTabPage() const { return pImpl->m_pSfxPage; }

    OKButton*           GetOKButton() const { return pOKBtn; }

protected:
    GetTabPageRanges    fnGetRanges;

    VclPtr<OKButton>      pOKBtn;
    VclPtr<CancelButton>  pCancelBtn;
    VclPtr<HelpButton>    pHelpBtn;

    SingleTabDlgImpl*   pImpl;

    DECL_DLLPRIVATE_LINK_TYPED(OKHdl_Impl, Button*, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

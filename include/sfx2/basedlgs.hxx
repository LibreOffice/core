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

SFX2_DLLPUBLIC inline void reverseUniqueHelpIdHack(Window &rWindow)
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
    SAL_DLLPRIVATE SfxModalDialog(SfxModalDialog &); // not defined
    SAL_DLLPRIVATE void operator =(SfxModalDialog &); // not defined

    SAL_DLLPRIVATE void SetDialogData_Impl();
    SAL_DLLPRIVATE void GetDialogData_Impl();
    SAL_DLLPRIVATE void init();

protected:
    SfxModalDialog(Window *pParent, const ResId& );
    SfxModalDialog(Window *pParent, const OString& rID, const OUString& rUIXMLDescription);

    OUString&           GetExtraData()      { return aExtraData; }
    sal_uInt32          GetUniqId() const   { return nUniqId; }
    void                SetUniqId(sal_uInt32 nSettingsId)  { nUniqId = nSettingsId; }
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

    void Init(SfxBindings *pBindinx, SfxChildWindow *pCW);

protected:
                            SfxModelessDialog( SfxBindings*, SfxChildWindow*,
                                Window*, const ResId& );
                            SfxModelessDialog( SfxBindings*, SfxChildWindow*,
                                Window*, const OString& rID, const OUString& rUIXMLDescription );
                            ~SfxModelessDialog();
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

    DECL_LINK(TimerHdl, void *);

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

    DECL_LINK(TimerHdl, void *);

};

// class SfxNoLayoutSingleTabDialog --------------------------------------------------

struct SingleTabDlgImpl
{
    SfxTabPage*                 m_pSfxPage;
    FixedLine*                  m_pLine;
    OUString                    m_sInfoURL;
    Link                        m_aInfoLink;

    SingleTabDlgImpl()
        : m_pSfxPage(NULL)
        , m_pLine(NULL)
    {
    }
};

typedef sal_uInt16* (*GetTabPageRanges)(); // liefert internationale Which-Werte

class SFX2_DLLPUBLIC SfxSingleTabDialog : public SfxModalDialog
{
public:
    SfxSingleTabDialog(Window *pParent, const SfxItemSet& rOptionsSet,
        const OString& rID = OString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    SfxSingleTabDialog(Window *pParent, const SfxItemSet* pInSet = 0,
        const OString& rID = OString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    virtual             ~SfxSingleTabDialog();

    void                SetTabPage(SfxTabPage* pTabPage, GetTabPageRanges pRangesFunc = 0, sal_uInt32 nSettingsId = 0);
    SfxTabPage*         GetTabPage() const { return pImpl->m_pSfxPage; }

    OKButton*           GetOKButton() const { return pOKBtn; }
    CancelButton*       GetCancelButton() const { return pCancelBtn; }

protected:
    GetTabPageRanges    fnGetRanges;

    OKButton*           pOKBtn;
    CancelButton*       pCancelBtn;
    HelpButton*         pHelpBtn;

    SingleTabDlgImpl*   pImpl;

    DECL_DLLPRIVATE_LINK(OKHdl_Impl, void *);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

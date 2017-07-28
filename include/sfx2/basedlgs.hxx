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

#include <memory>
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

protected:
    SfxModalDialog(vcl::Window *pParent, const OUString& rID, const OUString& rUIXMLDescription);

    OUString&           GetExtraData()      { return aExtraData; }
    sal_uInt32          GetUniqId() const   { return nUniqId; }
    void                SetUniqId(sal_uInt32 nSettingsId)  { nUniqId = nSettingsId; }
    void                CreateOutputItemSet( SfxItemPool& rPool );
    void                CreateOutputItemSet( const SfxItemSet& rInput );
    void                SetInputSet( const SfxItemSet* pInSet ) { pInputSet = pInSet; }
    SfxItemSet*         GetOutputSetImpl() { return pOutputSet; }

public:
    virtual ~SfxModalDialog() override;
    virtual void dispose() override;
    const SfxItemSet*   GetOutputItemSet() const { return pOutputSet; }
    const SfxItemSet*   GetInputItemSet() const { return pInputSet; }
};

// class SfxModelessDialog --------------------------------------------------
class SfxModelessDialog_Impl;
class SFX2_DLLPUBLIC SfxModelessDialog: public ModelessDialog
{
    SfxBindings*            pBindings;
    Size                    aSize;
    std::unique_ptr< SfxModelessDialog_Impl > pImpl;

    SfxModelessDialog(SfxModelessDialog &) = delete;
    void operator =(SfxModelessDialog &) = delete;

    void Init(SfxBindings *pBindinx, SfxChildWindow *pCW);

protected:
    SfxModelessDialog( SfxBindings*, SfxChildWindow*,
        vcl::Window*, const OUString& rID, const OUString& rUIXMLDescription );
    virtual ~SfxModelessDialog() override;
    virtual void dispose() override;
    virtual bool            Close() override;
    virtual void            Resize() override;
    virtual void            Move() override;
    virtual void            StateChanged( StateChangedType nStateChange ) override;

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo const * pInfo);
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    SfxBindings&            GetBindings()
                            { return *pBindings; }

    virtual void            LogicInvalidate(const tools::Rectangle* pRectangle) override;

    DECL_LINK(TimerHdl, Timer *, void);

};

// class SfxFloatingWindow --------------------------------------------------
class SfxFloatingWindow_Impl;
class SFX2_DLLPUBLIC SfxFloatingWindow: public FloatingWindow
{
    SfxBindings*            pBindings;
    Size                    aSize;
    std::unique_ptr< SfxFloatingWindow_Impl > pImpl;

    SfxFloatingWindow(SfxFloatingWindow &) = delete;
    void operator =(SfxFloatingWindow &) = delete;

protected:
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              vcl::Window* pParent,
                                              WinBits nWinBits);
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              vcl::Window* pParent,
                                              const OString& rID, const OUString& rUIXMLDescription,
                                              const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
                            virtual ~SfxFloatingWindow() override;
    virtual void            dispose() override;

    virtual void            StateChanged( StateChangedType nStateChange ) override;
    virtual bool            Close() override;
    virtual void            Resize() override;
    virtual void            Move() override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    SfxBindings&            GetBindings()
                            { return *pBindings; }

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo const * pInfo);

    DECL_LINK(TimerHdl, Timer *, void);

};

// class SfxNoLayoutSingleTabDialog --------------------------------------------------

struct SingleTabDlgImpl
{
    VclPtr<SfxTabPage>          m_pSfxPage;
    VclPtr<FixedLine>           m_pLine;

    SingleTabDlgImpl();
};

typedef const sal_uInt16* (*GetTabPageRanges)(); // provides international Which values

class SFX2_DLLPUBLIC SfxSingleTabDialog : public SfxModalDialog
{
public:
    SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet& rOptionsSet,
        const OUString& rID = OUString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet* pInSet,
        const OUString& rID = OUString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    virtual             ~SfxSingleTabDialog() override;
    virtual void        dispose() override;

    void                SetTabPage(SfxTabPage* pTabPage, sal_uInt32 nSettingsId = 0);
    SfxTabPage*         GetTabPage() const { return pImpl->m_pSfxPage; }

    OKButton*           GetOKButton() const { return pOKBtn; }

protected:
    VclPtr<OKButton>      pOKBtn;
    VclPtr<CancelButton>  pCancelBtn;
    VclPtr<HelpButton>    pHelpBtn;

    DECL_DLLPRIVATE_LINK(OKHdl_Impl, Button*, void);

private:
    std::unique_ptr<SingleTabDlgImpl>   pImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

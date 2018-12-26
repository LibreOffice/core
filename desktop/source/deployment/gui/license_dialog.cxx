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


#include <cppuhelper/implementationentry.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/unwrapargs.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <svtools/svmedit.hxx>
#include <svl/lstner.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/threadex.hxx>
#include <vcl/builderfactory.hxx>

#include "license_dialog.hxx"

#include <functional>

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_gui {

class LicenseView : public MultiLineEdit, public SfxListener
{
    bool            mbEndReached;
    Link<LicenseView&,void> maEndReachedHdl;
    Link<LicenseView&,void> maScrolledHdl;

public:
    LicenseView( vcl::Window* pParent, WinBits nStyle );
    virtual ~LicenseView() override;
    virtual void dispose() override;

    void ScrollDown( ScrollType eScroll );

    bool IsEndReached() const;
    bool EndReached() const { return mbEndReached; }

    void SetEndReachedHdl( const Link<LicenseView&,void>& rHdl ) { maEndReachedHdl = rHdl; }

    void SetScrolledHdl( const Link<LicenseView&,void>& rHdl ) { maScrolledHdl = rHdl; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
};

struct LicenseDialogImpl : public ModalDialog
{
    VclPtr<FixedText> m_pFtHead;
    VclPtr<FixedImage> m_pArrow1;
    VclPtr<FixedImage> m_pArrow2;
    VclPtr<LicenseView> m_pLicense;
    VclPtr<PushButton> m_pDown;

    VclPtr<PushButton> m_pAcceptButton;
    VclPtr<PushButton> m_pDeclineButton;

    DECL_LINK(PageDownHdl, Button*, void);
    DECL_LINK(ScrolledHdl, LicenseView&, void);
    DECL_LINK(EndReachedHdl, LicenseView&, void);
    DECL_LINK(CancelHdl, Button*, void);
    DECL_LINK(AcceptHdl, Button*, void);

    bool m_bLicenseRead;

    LicenseDialogImpl(
        vcl::Window * pParent,
        const OUString & sExtensionName,
        const OUString & sLicenseText);
    virtual ~LicenseDialogImpl() override { disposeOnce(); }
    virtual void dispose() override;

    virtual void Activate() override;

};

void LicenseDialogImpl::dispose()
{
    m_pFtHead.clear();
    m_pArrow1.clear();
    m_pArrow2.clear();
    m_pLicense.clear();
    m_pDown.clear();
    m_pAcceptButton.clear();
    m_pDeclineButton.clear();
    ModalDialog::dispose();
}


LicenseView::LicenseView( vcl::Window* pParent, WinBits nStyle )
    : MultiLineEdit( pParent, nStyle )
{
    SetLeftMargin( 5 );
    mbEndReached = IsEndReached();
    StartListening( *GetTextEngine() );
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(LicenseView, WB_CLIPCHILDREN|WB_LEFT|WB_VSCROLL)

LicenseView::~LicenseView()
{
    disposeOnce();
}

void LicenseView::dispose()
{
    maEndReachedHdl = Link<LicenseView&,void>();
    maScrolledHdl   = Link<LicenseView&,void>();
    EndListeningAll();
    MultiLineEdit::dispose();
}

void LicenseView::ScrollDown( ScrollType eScroll )
{
    ScrollBar*  pScroll = GetVScrollBar();
    if ( pScroll )
        pScroll->DoScrollAction( eScroll );
}

bool LicenseView::IsEndReached() const
{
    bool bEndReached;

    TextView*       pView = GetTextView();
    ExtTextEngine*  pEdit = GetTextEngine();
    const long      nHeight = pEdit->GetTextHeight();
    Size            aOutSize = pView->GetWindow()->GetOutputSizePixel();
    Point           aBottom( 0, aOutSize.Height() );

    bEndReached = pView->GetDocPos( aBottom ).Y() >= nHeight - 1;

    return bEndReached;
}

void LicenseView::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const TextHint* pTextHint = dynamic_cast<const TextHint*>(&rHint);
    if ( pTextHint )
    {
        bool    bLastVal = EndReached();
        const SfxHintId nId = pTextHint->GetId();

        if ( nId == SfxHintId::TextParaInserted )
        {
            if ( bLastVal )
                mbEndReached = IsEndReached();
        }
        else if ( nId == SfxHintId::TextViewScrolled )
        {
            if ( ! mbEndReached )
                mbEndReached = IsEndReached();
            maScrolledHdl.Call( *this );
        }

        if ( EndReached() && !bLastVal )
        {
            maEndReachedHdl.Call( *this );
        }
    }
}


LicenseDialogImpl::LicenseDialogImpl(
    vcl::Window * pParent,
    const OUString & sExtensionName,
    const OUString & sLicenseText)
    : ModalDialog(pParent, "LicenseDialog", "desktop/ui/licensedialog.ui")
    , m_bLicenseRead(false)
{
    get(m_pFtHead, "head");
    get(m_pArrow1, "arrow1");
    get(m_pArrow2, "arrow2");
    get(m_pDown, "down");
    get(m_pAcceptButton, "accept");
    get(m_pDeclineButton, "decline");
    m_pArrow1->Show();
    m_pArrow2->Show(false);
    get(m_pLicense, "textview");

    Size aSize(m_pLicense->LogicToPixel(Size(290, 170), MapMode(MapUnit::MapAppFont)));
    m_pLicense->set_width_request(aSize.Width());
    m_pLicense->set_height_request(aSize.Height());

    m_pLicense->SetText(sLicenseText);
    m_pFtHead->SetText(m_pFtHead->GetText() + "\n" + sExtensionName);

    m_pAcceptButton->SetClickHdl( LINK(this, LicenseDialogImpl, AcceptHdl) );
    m_pDeclineButton->SetClickHdl( LINK(this, LicenseDialogImpl, CancelHdl) );

    m_pLicense->SetEndReachedHdl( LINK(this, LicenseDialogImpl, EndReachedHdl) );
    m_pLicense->SetScrolledHdl( LINK(this, LicenseDialogImpl, ScrolledHdl) );
    m_pDown->SetClickHdl( LINK(this, LicenseDialogImpl, PageDownHdl) );

    // We want a automatic repeating page down button
    WinBits aStyle = m_pDown->GetStyle();
    aStyle |= WB_REPEAT;
    m_pDown->SetStyle( aStyle );
}

IMPL_LINK_NOARG(LicenseDialogImpl, AcceptHdl, Button*, void)
{
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(LicenseDialogImpl, CancelHdl, Button*, void)
{
    EndDialog();
}

void LicenseDialogImpl::Activate()
{
    if (!m_bLicenseRead)
    {
        //Only enable the scroll down button if the license text does not fit into the window
        if (m_pLicense->IsEndReached())
        {
            m_pDown->Disable();
            m_pAcceptButton->Enable();
            m_pAcceptButton->GrabFocus();
        }
        else
        {
            m_pDown->Enable();
            m_pDown->GrabFocus();
            m_pAcceptButton->Disable();
        }
    }
}

IMPL_LINK_NOARG(LicenseDialogImpl, ScrolledHdl, LicenseView&, void)
{
    if (m_pLicense->IsEndReached())
        m_pDown->Disable();
    else
        m_pDown->Enable();
}

IMPL_LINK_NOARG(LicenseDialogImpl, PageDownHdl, Button*, void)
{
    m_pLicense->ScrollDown( ScrollType::PageDown );
}

IMPL_LINK_NOARG(LicenseDialogImpl, EndReachedHdl, LicenseView&, void)
{
    m_pAcceptButton->Enable();
    m_pAcceptButton->GrabFocus();
    m_pArrow1->Show(false);
    m_pArrow2->Show();
    m_bLicenseRead = true;
}


LicenseDialog::LicenseDialog( Sequence<Any> const& args,
                          Reference<XComponentContext> const& )
{
    comphelper::unwrapArgs( args, m_parent, m_sExtensionName, m_sLicenseText );
}

// XExecutableDialog

void LicenseDialog::setTitle( OUString const & )
{

}


sal_Int16 LicenseDialog::execute()
{
    return vcl::solarthread::syncExecute(
        std::bind(&LicenseDialog::solar_execute, this));
}

sal_Int16 LicenseDialog::solar_execute()
{
    ScopedVclPtrInstance<LicenseDialogImpl> dlg(
            VCLUnoHelper::GetWindow(m_parent), m_sExtensionName, m_sLicenseText);

    return dlg->Execute();
}

} // namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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


#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "unotools/configmgr.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/unwrapargs.hxx"
#include "i18nlangtag/mslangid.hxx"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "toolkit/helper/vclunohelper.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "svtools/svmedit.hxx"
#include "svl/lstner.hxx"
#include "vcl/xtextedt.hxx"
#include <vcl/scrbar.hxx>
#include "vcl/threadex.hxx"



#include "boost/bind.hpp"
#include "dp_gui_shared.hxx"
#include "license_dialog.hxx"
#include "dp_gui.hrc"

using namespace ::dp_misc;
namespace cssu = ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_gui {

class LicenseView : public MultiLineEdit, public SfxListener
{
    sal_Bool            mbEndReached;
    Link            maEndReachedHdl;
    Link            maScrolledHdl;

public:
    LicenseView( Window* pParent, WinBits nStyle );
    ~LicenseView();

    void ScrollDown( ScrollType eScroll );

    sal_Bool IsEndReached() const;
    sal_Bool EndReached() const { return mbEndReached; }
    void SetEndReached( sal_Bool bEnd ) { mbEndReached = bEnd; }

    void SetEndReachedHdl( const Link& rHdl )  { maEndReachedHdl = rHdl; }
    const Link& GetAutocompleteHdl() const { return maEndReachedHdl; }

    void SetScrolledHdl( const Link& rHdl )  { maScrolledHdl = rHdl; }
    const Link& GetScrolledHdl() const { return maScrolledHdl; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

protected:
    using MultiLineEdit::Notify;
};

struct LicenseDialogImpl : public ModalDialog
{
    cssu::Reference<cssu::XComponentContext> m_xComponentContext;
    FixedText* m_pFtHead;
    FixedImage* m_pArrow1;
    FixedImage* m_pArrow2;
    LicenseView* m_pLicense;
    PushButton* m_pDown;

    PushButton* m_pAcceptButton;
    PushButton* m_pDeclineButton;

    DECL_LINK(PageDownHdl, void *);
    DECL_LINK(ScrolledHdl, void *);
    DECL_LINK(EndReachedHdl, void *);
    DECL_LINK(CancelHdl, void *);
    DECL_LINK(AcceptHdl, void *);

    bool m_bLicenseRead;

    LicenseDialogImpl(
        Window * pParent,
        css::uno::Reference< css::uno::XComponentContext > const & xContext,
        const OUString & sExtensionName,
        const OUString & sLicenseText);

    virtual void Activate();

};

LicenseView::LicenseView( Window* pParent, WinBits nStyle )
    : MultiLineEdit( pParent, nStyle )
{
    SetLeftMargin( 5 );
    mbEndReached = IsEndReached();
    StartListening( *GetTextEngine() );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeLicenseView(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_CLIPCHILDREN|WB_LEFT;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new LicenseView(pParent, nWinStyle | WB_VSCROLL);
}

LicenseView::~LicenseView()
{
    maEndReachedHdl = Link();
    maScrolledHdl   = Link();
    EndListeningAll();
}

void LicenseView::ScrollDown( ScrollType eScroll )
{
    ScrollBar*  pScroll = GetVScrollBar();
    if ( pScroll )
        pScroll->DoScrollAction( eScroll );
}

sal_Bool LicenseView::IsEndReached() const
{
    sal_Bool bEndReached;

    ExtTextView*    pView = GetTextView();
    ExtTextEngine*  pEdit = GetTextEngine();
    sal_uLong           nHeight = pEdit->GetTextHeight();
    Size            aOutSize = pView->GetWindow()->GetOutputSizePixel();
    Point           aBottom( 0, aOutSize.Height() );

    if ( (sal_uLong) pView->GetDocPos( aBottom ).Y() >= nHeight - 1 )
        bEndReached = sal_True;
    else
        bEndReached = sal_False;

    return bEndReached;
}

void LicenseView::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA( TYPE(TextHint) ) )
    {
        sal_Bool    bLastVal = EndReached();
        sal_uLong   nId = ((const TextHint&)rHint).GetId();

        if ( nId == TEXT_HINT_PARAINSERTED )
        {
            if ( bLastVal )
                mbEndReached = IsEndReached();
        }
        else if ( nId == TEXT_HINT_VIEWSCROLLED )
        {
            if ( ! mbEndReached )
                mbEndReached = IsEndReached();
            maScrolledHdl.Call( this );
        }

        if ( EndReached() && !bLastVal )
        {
            maEndReachedHdl.Call( this );
        }
    }
}

//==============================================================================================================

LicenseDialogImpl::LicenseDialogImpl(
    Window * pParent,
    cssu::Reference< cssu::XComponentContext > const & xContext,
    const OUString & sExtensionName,
    const OUString & sLicenseText)
    : ModalDialog(pParent, "LicenseDialog", "desktop/ui/licensedialog.ui")
    , m_xComponentContext(xContext)
    , m_bLicenseRead(false)
{
    get(m_pFtHead, "head");
    get(m_pArrow1, "arrow1");
    get(m_pArrow2, "arrow2");
    get(m_pDown, "down");
    get(m_pAcceptButton, "accept");
    get(m_pDeclineButton, "decline");
    m_pArrow1->Show(true);
    m_pArrow2->Show(false);
    get(m_pLicense, "textview");

    Size aSize(m_pLicense->LogicToPixel(Size(290, 170), MAP_APPFONT));
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

IMPL_LINK_NOARG(LicenseDialogImpl, AcceptHdl)
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(LicenseDialogImpl, CancelHdl)
{
    EndDialog(RET_CANCEL);
    return 0;
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

IMPL_LINK_NOARG(LicenseDialogImpl, ScrolledHdl)
{

    if (m_pLicense->IsEndReached())
        m_pDown->Disable();
    else
        m_pDown->Enable();

    return 0;
}

IMPL_LINK_NOARG(LicenseDialogImpl, PageDownHdl)
{
    m_pLicense->ScrollDown( SCROLL_PAGEDOWN );
    return 0;
}

IMPL_LINK_NOARG(LicenseDialogImpl, EndReachedHdl)
{
    m_pAcceptButton->Enable();
    m_pAcceptButton->GrabFocus();
    m_pArrow1->Show(false);
    m_pArrow2->Show(true);
    m_bLicenseRead = true;
    return 0;
}

//=================================================================================




LicenseDialog::LicenseDialog( Sequence<Any> const& args,
                          Reference<XComponentContext> const& xComponentContext)
    : m_xComponentContext(xComponentContext)
{
    comphelper::unwrapArgs( args, m_parent, m_sExtensionName, m_sLicenseText );
}

// XExecutableDialog

void LicenseDialog::setTitle( OUString const & ) throw (RuntimeException, std::exception)
{

}


sal_Int16 LicenseDialog::execute() throw (RuntimeException, std::exception)
{
    return vcl::solarthread::syncExecute(
        boost::bind( &LicenseDialog::solar_execute, this));
}

sal_Int16 LicenseDialog::solar_execute()
{
    std::auto_ptr<LicenseDialogImpl> dlg(
        new LicenseDialogImpl(
            VCLUnoHelper::GetWindow(m_parent),
            m_xComponentContext, m_sExtensionName, m_sLicenseText));

    return dlg->Execute();
}

} // namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

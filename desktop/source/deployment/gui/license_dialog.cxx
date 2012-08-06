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


#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "unotools/configmgr.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/unwrapargs.hxx"
#include "i18npool/mslangid.hxx"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "toolkit/helper/vclunohelper.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"
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
using ::rtl::OUString;

namespace dp_gui {

class LicenseView : public MultiLineEdit, public SfxListener
{
    sal_Bool            mbEndReached;
    Link            maEndReachedHdl;
    Link            maScrolledHdl;

public:
    LicenseView( Window* pParent, const ResId& rResId );
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
    FixedText m_ftHead;
    FixedText m_ftBody1;
    FixedText m_ftBody1Txt;
    FixedText m_ftBody2;
    FixedText m_ftBody2Txt;
    FixedImage m_fiArrow1;
    FixedImage m_fiArrow2;
    LicenseView m_mlLicense;
    PushButton m_pbDown;
    FixedLine m_flBottom;

    OKButton m_acceptButton;
    CancelButton m_declineButton;

    DECL_LINK(PageDownHdl, void *);
    DECL_LINK(ScrolledHdl, void *);
    DECL_LINK(EndReachedHdl, void *);

    bool m_bLicenseRead;

    virtual ~LicenseDialogImpl();

    LicenseDialogImpl(
        Window * pParent,
        css::uno::Reference< css::uno::XComponentContext > const & xContext,
        const ::rtl::OUString & sExtensionName,
        const ::rtl::OUString & sLicenseText);

    virtual void Activate();

};

LicenseView::LicenseView( Window* pParent, const ResId& rResId )
    : MultiLineEdit( pParent, rResId )
{
    SetLeftMargin( 5 );
    mbEndReached = IsEndReached();
    StartListening( *GetTextEngine() );
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
    const ::rtl::OUString & sExtensionName,
    const ::rtl::OUString & sLicenseText):
        ModalDialog(pParent, DpGuiResId(RID_DLG_LICENSE))
        ,m_xComponentContext(xContext)
        ,m_ftHead(this, DpGuiResId(FT_LICENSE_HEADER))
        ,m_ftBody1(this, DpGuiResId(FT_LICENSE_BODY_1))
        ,m_ftBody1Txt(this, DpGuiResId(FT_LICENSE_BODY_1_TXT))
        ,m_ftBody2(this, DpGuiResId(FT_LICENSE_BODY_2))
        ,m_ftBody2Txt(this, DpGuiResId(FT_LICENSE_BODY_2_TXT))
        ,m_fiArrow1(this, DpGuiResId(FI_LICENSE_ARROW1))
        ,m_fiArrow2(this, DpGuiResId(FI_LICENSE_ARROW2))
        ,m_mlLicense(this, DpGuiResId(ML_LICENSE))
        ,m_pbDown(this, DpGuiResId(PB_LICENSE_DOWN))
        ,m_flBottom(this, DpGuiResId(FL_LICENSE))
        ,m_acceptButton(this, DpGuiResId(BTN_LICENSE_ACCEPT))
        ,m_declineButton(this, DpGuiResId(BTN_LICENSE_DECLINE))
        ,m_bLicenseRead(false)

{

    FreeResource();

    m_acceptButton.SetUniqueId(UID_BTN_LICENSE_ACCEPT);
    m_fiArrow1.Show(true);
    m_fiArrow2.Show(false);
    m_mlLicense.SetText(sLicenseText);
    m_ftHead.SetText(m_ftHead.GetText() + OUString('\n') + sExtensionName);

    m_mlLicense.SetEndReachedHdl( LINK(this, LicenseDialogImpl, EndReachedHdl) );
    m_mlLicense.SetScrolledHdl( LINK(this, LicenseDialogImpl, ScrolledHdl) );
    m_pbDown.SetClickHdl( LINK(this, LicenseDialogImpl, PageDownHdl) );

    // We want a automatic repeating page down button
    WinBits aStyle = m_pbDown.GetStyle();
    aStyle |= WB_REPEAT;
    m_pbDown.SetStyle( aStyle );
}

LicenseDialogImpl::~LicenseDialogImpl()
{
}

void LicenseDialogImpl::Activate()
{
    if (!m_bLicenseRead)
    {
        //Only enable the scroll down button if the license text does not fit into the window
        if (m_mlLicense.IsEndReached())
        {
            m_pbDown.Disable();
            m_acceptButton.Enable();
            m_acceptButton.GrabFocus();
        }
        else
        {
            m_pbDown.Enable();
            m_pbDown.GrabFocus();
            m_acceptButton.Disable();
        }
    }
}

IMPL_LINK_NOARG(LicenseDialogImpl, ScrolledHdl)
{

    if (m_mlLicense.IsEndReached())
        m_pbDown.Disable();
    else
        m_pbDown.Enable();

    return 0;
}

IMPL_LINK_NOARG(LicenseDialogImpl, PageDownHdl)
{
    m_mlLicense.ScrollDown( SCROLL_PAGEDOWN );
    return 0;
}

IMPL_LINK_NOARG(LicenseDialogImpl, EndReachedHdl)
{
    m_acceptButton.Enable();
    m_acceptButton.GrabFocus();
    m_fiArrow1.Show(false);
    m_fiArrow2.Show(true);
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
//______________________________________________________________________________
void LicenseDialog::setTitle( OUString const & ) throw (RuntimeException)
{

}

//______________________________________________________________________________
sal_Int16 LicenseDialog::execute() throw (RuntimeException)
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

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
#include <vcl/event.hxx>
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
#include <vcl/weld.hxx>

#include "license_dialog.hxx"

#include <functional>

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_gui {

struct LicenseDialogImpl : public weld::GenericDialogController
{
    bool m_bLicenseRead;
    Idle m_aResized;
    AutoTimer m_aRepeat;

    std::unique_ptr<weld::Label> m_xFtHead;
    std::unique_ptr<weld::Widget> m_xArrow1;
    std::unique_ptr<weld::Widget> m_xArrow2;
    std::unique_ptr<weld::TextView> m_xLicense;
    std::unique_ptr<weld::Button> m_xDown;
    std::unique_ptr<weld::Button> m_xAcceptButton;
    std::unique_ptr<weld::Button> m_xDeclineButton;

    void PageDown();
    DECL_LINK(ScrollTimerHdl, Timer*, void);
    DECL_LINK(ScrolledHdl, weld::TextView&, void);
    DECL_LINK(ResizedHdl, Timer*, void);
    DECL_LINK(CancelHdl, weld::Button&, void);
    DECL_LINK(AcceptHdl, weld::Button&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_STATIC_LINK(LicenseDialogImpl, KeyReleaseHdl, const KeyEvent&, bool);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, bool);
    DECL_LINK(SizeAllocHdl, const Size&, void);

    LicenseDialogImpl(weld::Window * pParent,
                      const OUString & sExtensionName,
                      const OUString & sLicenseText);

    bool IsEndReached() const;
};

LicenseDialogImpl::LicenseDialogImpl(
    weld::Window * pParent,
    const OUString & sExtensionName,
    const OUString & sLicenseText)
    : GenericDialogController(pParent, "desktop/ui/licensedialog.ui", "LicenseDialog")
    , m_bLicenseRead(false)
    , m_xFtHead(m_xBuilder->weld_label("head"))
    , m_xArrow1(m_xBuilder->weld_widget("arrow1"))
    , m_xArrow2(m_xBuilder->weld_widget("arrow2"))
    , m_xLicense(m_xBuilder->weld_text_view("textview"))
    , m_xDown(m_xBuilder->weld_button("down"))
    , m_xAcceptButton(m_xBuilder->weld_button("ok"))
    , m_xDeclineButton(m_xBuilder->weld_button("cancel"))
{
    m_xArrow1->show();
    m_xArrow2->hide();

    m_xLicense->connect_size_allocate(LINK(this, LicenseDialogImpl, SizeAllocHdl));
    m_xLicense->set_size_request(m_xLicense->get_approximate_digit_width() * 72,
                                     m_xLicense->get_height_rows(21));

    m_xLicense->set_text(sLicenseText);
    m_xFtHead->set_label(m_xFtHead->get_label() + "\n" + sExtensionName);

    m_xAcceptButton->connect_clicked( LINK(this, LicenseDialogImpl, AcceptHdl) );
    m_xDeclineButton->connect_clicked( LINK(this, LicenseDialogImpl, CancelHdl) );

    m_xLicense->connect_vadjustment_changed(LINK(this, LicenseDialogImpl, ScrolledHdl));
    m_xDown->connect_mouse_press(LINK(this, LicenseDialogImpl, MousePressHdl));
    m_xDown->connect_mouse_release(LINK(this, LicenseDialogImpl, MouseReleaseHdl));
    m_xDown->connect_key_press(LINK(this, LicenseDialogImpl, KeyInputHdl));
    m_xDown->connect_key_release(LINK(this, LicenseDialogImpl, KeyReleaseHdl));

    m_aRepeat.SetTimeout(Application::GetSettings().GetMouseSettings().GetButtonRepeat());
    m_aRepeat.SetInvokeHandler(LINK(this, LicenseDialogImpl, ScrollTimerHdl));

    m_aResized.SetPriority(TaskPriority::LOWEST);
    m_aResized.SetInvokeHandler(LINK(this, LicenseDialogImpl, ResizedHdl));
}

IMPL_LINK_NOARG(LicenseDialogImpl, SizeAllocHdl, const Size&, void)
{
    m_aResized.Start();
}

IMPL_LINK_NOARG(LicenseDialogImpl, AcceptHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(LicenseDialogImpl, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

bool LicenseDialogImpl::IsEndReached() const
{
    return m_xLicense->vadjustment_get_value() + m_xLicense->vadjustment_get_page_size() >= m_xLicense->vadjustment_get_upper();
}

IMPL_LINK_NOARG(LicenseDialogImpl, ScrolledHdl, weld::TextView&, void)
{
    if (IsEndReached())
    {
        m_xDown->set_sensitive(false);
        m_aRepeat.Stop();

        if (!m_bLicenseRead)
        {
            m_xAcceptButton->set_sensitive(true);
            m_xAcceptButton->grab_focus();
            m_xArrow1->hide();
            m_xArrow2->show();
            m_bLicenseRead = true;
        }
    }
    else
        m_xDown->set_sensitive(true);
}

void LicenseDialogImpl::PageDown()
{
    m_xLicense->vadjustment_set_value(m_xLicense->vadjustment_get_value() +
                                      m_xLicense->vadjustment_get_page_size());
    ScrolledHdl(*m_xLicense);
}

IMPL_LINK(LicenseDialogImpl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    if (aKeyCode.GetCode() == KEY_RETURN || aKeyCode.GetCode() == KEY_SPACE)
        PageDown();
    return false;
}

IMPL_LINK_NOARG(LicenseDialogImpl, ResizedHdl, Timer*, void)
{
    ScrolledHdl(*m_xLicense);
}

IMPL_LINK_NOARG(LicenseDialogImpl, ScrollTimerHdl, Timer*, void)
{
    PageDown();
}

IMPL_STATIC_LINK_NOARG(LicenseDialogImpl, KeyReleaseHdl, const KeyEvent&, bool)
{
    return false;
}

IMPL_LINK_NOARG(LicenseDialogImpl, MousePressHdl, const MouseEvent&, bool)
{
    PageDown();
    m_aRepeat.Start();
    return false;
}

IMPL_LINK_NOARG(LicenseDialogImpl, MouseReleaseHdl, const MouseEvent&, bool)
{
    m_aRepeat.Stop();
    return false;
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
    LicenseDialogImpl dlg(Application::GetFrameWeld(m_parent), m_sExtensionName, m_sLicenseText);
    return dlg.run();
}

} // namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

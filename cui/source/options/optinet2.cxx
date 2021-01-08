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

#include <sal/config.h>

#include <string_view>

#include <officecfg/Inet.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Security.hxx>
#include <vcl/weld.hxx>
#include <sfx2/filedlghelper.hxx>
#include <vcl/svapp.hxx>
#include <unotools/securityoptions.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/diagnose_ex.h>

#include <dialmgr.hxx>
#include "optinet2.hxx"
#include <strings.hrc>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include "securityoptions.hxx"
#include "webconninfo.hxx"
#include "certpath.hxx"
#include "tsaurls.hxx"

#include <svtools/restartdialog.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;

namespace {

bool isValidPort(OUString const & value) {
    if (!comphelper::string::isdigitAsciiString(value)) {
        return false;
    }
    auto const n = value.toUInt64();
    if (n > 65535) {
        return false;
    }
    if (n != 0) {
        return true;
    }
    // Overflow in OUString::toUInt64 returns 0, so need to check value contains only zeroes:
    return std::u16string_view(value).find_first_not_of(u'0') == std::u16string_view::npos;
}

}

IMPL_LINK(SvxProxyTabPage, PortChangedHdl, weld::Entry&, rEdit, void)
{
    if (!isValidPort(rEdit.get_text()))
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Warning, VclButtonsType::Ok,
                                                       CuiResId( RID_SVXSTR_OPT_PROXYPORTS)));
        xErrorBox->run();
    }
}

constexpr OUStringLiteral g_aProxyModePN = u"ooInetProxyType";
constexpr OUStringLiteral g_aHttpProxyPN = u"ooInetHTTPProxyName";
constexpr OUStringLiteral g_aHttpPortPN = u"ooInetHTTPProxyPort";
constexpr OUStringLiteral g_aHttpsProxyPN = u"ooInetHTTPSProxyName";
constexpr OUStringLiteral g_aHttpsPortPN = u"ooInetHTTPSProxyPort";
constexpr OUStringLiteral g_aFtpProxyPN = u"ooInetFTPProxyName";
constexpr OUStringLiteral g_aFtpPortPN = u"ooInetFTPProxyPort";
constexpr OUStringLiteral g_aNoProxyDescPN = u"ooInetNoProxy";

IMPL_STATIC_LINK(SvxProxyTabPage, NumberOnlyTextFilterHdl, OUString&, rTest, bool)
{
    OUStringBuffer sAllowed;
    for (sal_Int32 i = 0, nLen = rTest.getLength(); i < nLen; ++i)
    {
        if (rTest[i] >= '0' && rTest[i] <= '9')
            sAllowed.append(rTest[i]);
    }
    rTest = sAllowed.makeStringAndClear();
    return true;
}

IMPL_STATIC_LINK(SvxProxyTabPage, NoSpaceTextFilterHdl, OUString&, rTest, bool)
{
    rTest = rTest.replaceAll(" ", "");
    return true;
}

/********************************************************************/
/*                                                                  */
/*  SvxProxyTabPage                                                 */
/*                                                                  */
/********************************************************************/
SvxProxyTabPage::SvxProxyTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optproxypage.ui", "OptProxyPage", &rSet)
    , m_xProxyModeLB(m_xBuilder->weld_combo_box("proxymode"))
    , m_xHttpProxyFT(m_xBuilder->weld_label("httpft"))
    , m_xHttpProxyED(m_xBuilder->weld_entry("http"))
    , m_xHttpPortFT(m_xBuilder->weld_label("httpportft"))
    , m_xHttpPortED(m_xBuilder->weld_entry("httpport"))
    , m_xHttpsProxyFT(m_xBuilder->weld_label("httpsft"))
    , m_xHttpsProxyED(m_xBuilder->weld_entry("https"))
    , m_xHttpsPortFT(m_xBuilder->weld_label("httpsportft"))
    , m_xHttpsPortED(m_xBuilder->weld_entry("httpsport"))
    , m_xFtpProxyFT(m_xBuilder->weld_label("ftpft"))
    , m_xFtpProxyED(m_xBuilder->weld_entry("ftp"))
    , m_xFtpPortFT(m_xBuilder->weld_label("ftpportft"))
    , m_xFtpPortED(m_xBuilder->weld_entry("ftpport"))
    , m_xNoProxyForFT(m_xBuilder->weld_label("noproxyft"))
    , m_xNoProxyForED(m_xBuilder->weld_entry("noproxy"))
    , m_xNoProxyDescFT(m_xBuilder->weld_label("noproxydesc"))
{
    m_xHttpProxyED->connect_insert_text(LINK(this, SvxProxyTabPage, NoSpaceTextFilterHdl));
    m_xHttpPortED->connect_insert_text(LINK(this, SvxProxyTabPage, NumberOnlyTextFilterHdl));
    m_xHttpPortED->connect_changed(LINK(this, SvxProxyTabPage, PortChangedHdl));
    m_xHttpsProxyED->connect_insert_text(LINK(this, SvxProxyTabPage, NoSpaceTextFilterHdl));
    m_xHttpsPortED->connect_insert_text(LINK(this, SvxProxyTabPage, NumberOnlyTextFilterHdl));
    m_xHttpsPortED->connect_changed(LINK(this, SvxProxyTabPage, PortChangedHdl));
    m_xFtpProxyED->connect_insert_text(LINK(this, SvxProxyTabPage, NoSpaceTextFilterHdl));
    m_xFtpPortED->connect_insert_text(LINK(this, SvxProxyTabPage, NumberOnlyTextFilterHdl));
    m_xFtpPortED->connect_changed(LINK(this, SvxProxyTabPage, PortChangedHdl));

    Link<weld::Widget&,void> aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    m_xHttpPortED->connect_focus_out( aLink );
    m_xHttpsPortED->connect_focus_out( aLink );
    m_xFtpPortED->connect_focus_out( aLink );

    m_xProxyModeLB->connect_changed(LINK( this, SvxProxyTabPage, ProxyHdl_Impl ));

    Reference< css::lang::XMultiServiceFactory >
        xConfigurationProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

    beans::NamedValue aProperty;
    aProperty.Name  = "nodepath";
    aProperty.Value <<= OUString( "org.openoffice.Inet/Settings" );

    Sequence< Any > aArgumentList( 1 );
    aArgumentList[0] <<= aProperty;

    m_xConfigurationUpdateAccess = xConfigurationProvider->createInstanceWithArguments(
        "com.sun.star.configuration.ConfigurationUpdateAccess",
        aArgumentList );
}

SvxProxyTabPage::~SvxProxyTabPage()
{
}

std::unique_ptr<SfxTabPage> SvxProxyTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxProxyTabPage>(pPage, pController, *rAttrSet);
}

void SvxProxyTabPage::ReadConfigData_Impl()
{
    sal_Int32 nIntValue = 0;

    std::optional<sal_Int32> x(officecfg::Inet::Settings::ooInetProxyType::get());
    if (x)
    {
        nIntValue = *x;
        m_xProxyModeLB->set_active(nIntValue);
    }

    m_xHttpProxyED->set_text( officecfg::Inet::Settings::ooInetHTTPProxyName::get() );
    x = officecfg::Inet::Settings::ooInetHTTPProxyPort::get();
    if (x)
    {
        nIntValue = *x;
        m_xHttpPortED->set_text( OUString::number( nIntValue ));
    }

    m_xHttpsProxyED->set_text( officecfg::Inet::Settings::ooInetHTTPSProxyName::get() );
    x = officecfg::Inet::Settings::ooInetHTTPSProxyPort::get();
    if (x)
    {
        nIntValue = *x;
        m_xHttpsPortED->set_text( OUString::number( nIntValue ));
    }

    m_xFtpProxyED->set_text( officecfg::Inet::Settings::ooInetFTPProxyName::get() );
    x = officecfg::Inet::Settings::ooInetFTPProxyPort::get();
    if (x)
    {
        nIntValue = *x;
        m_xFtpPortED->set_text( OUString::number( nIntValue ));
    }

    m_xNoProxyForED->set_text( officecfg::Inet::Settings::ooInetNoProxy::get() );
}

void SvxProxyTabPage::ReadConfigDefaults_Impl()
{
    try
    {
        Reference< beans::XPropertyState > xPropertyState(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        sal_Int32 nIntValue = 0;
        OUString  aStringValue;

        if( xPropertyState->getPropertyDefault(g_aHttpProxyPN) >>= aStringValue )
        {
            m_xHttpProxyED->set_text( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(g_aHttpPortPN) >>= nIntValue )
        {
            m_xHttpPortED->set_text( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(g_aHttpsProxyPN) >>= aStringValue )
        {
            m_xHttpsProxyED->set_text( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(g_aHttpsPortPN) >>= nIntValue )
        {
            m_xHttpsPortED->set_text( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(g_aFtpProxyPN) >>= aStringValue )
        {
            m_xFtpProxyED->set_text( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(g_aFtpPortPN) >>= nIntValue )
        {
            m_xFtpPortED->set_text( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(g_aNoProxyDescPN) >>= aStringValue )
        {
            m_xNoProxyForED->set_text( aStringValue );
        }
    }
    catch (const beans::UnknownPropertyException &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const css::lang::WrappedTargetException &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const RuntimeException &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
}

void SvxProxyTabPage::RestoreConfigDefaults_Impl()
{
    try
    {
        Reference< beans::XPropertyState > xPropertyState(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        xPropertyState->setPropertyToDefault(g_aProxyModePN);
        xPropertyState->setPropertyToDefault(g_aHttpProxyPN);
        xPropertyState->setPropertyToDefault(g_aHttpPortPN);
        xPropertyState->setPropertyToDefault(g_aHttpsProxyPN);
        xPropertyState->setPropertyToDefault(g_aHttpsPortPN);
        xPropertyState->setPropertyToDefault(g_aFtpProxyPN);
        xPropertyState->setPropertyToDefault(g_aFtpPortPN);
        xPropertyState->setPropertyToDefault(g_aNoProxyDescPN);

        Reference< util::XChangesBatch > xChangesBatch(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }
    catch (const beans::UnknownPropertyException &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const css::lang::WrappedTargetException &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const RuntimeException &)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
}

void SvxProxyTabPage::Reset(const SfxItemSet*)
{
    ReadConfigData_Impl();

    m_xProxyModeLB->save_value();
    m_xHttpProxyED->save_value();
    m_xHttpPortED->save_value();
    m_xHttpsProxyED->save_value();
    m_xHttpsPortED->save_value();
    m_xFtpProxyED->save_value();
    m_xFtpPortED->save_value();
    m_xNoProxyForED->save_value();

    EnableControls_Impl();
}

bool SvxProxyTabPage::FillItemSet(SfxItemSet* )
{
    bool bModified = false;

    try {
        Reference< beans::XPropertySet > xPropertySet(m_xConfigurationUpdateAccess, UNO_QUERY_THROW );

        sal_Int32 nSelPos = m_xProxyModeLB->get_active();
        if(m_xProxyModeLB->get_value_changed_from_saved())
        {
            if( nSelPos == 1 )
            {
                RestoreConfigDefaults_Impl();
                return true;
            }

            xPropertySet->setPropertyValue(g_aProxyModePN, Any(nSelPos));
            bModified = true;
        }

        if(m_xHttpProxyED->get_value_changed_from_saved())
        {
            xPropertySet->setPropertyValue( g_aHttpProxyPN, Any(m_xHttpProxyED->get_text()));
            bModified = true;
        }

        if ( m_xHttpPortED->get_value_changed_from_saved())
        {
            xPropertySet->setPropertyValue( g_aHttpPortPN, Any(m_xHttpPortED->get_text().toInt32()));
            bModified = true;
        }

        if( m_xHttpsProxyED->get_value_changed_from_saved() )
        {
            xPropertySet->setPropertyValue( g_aHttpsProxyPN, Any(m_xHttpsProxyED->get_text()) );
            bModified = true;
        }

        if ( m_xHttpsPortED->get_value_changed_from_saved() )
        {
            xPropertySet->setPropertyValue( g_aHttpsPortPN, Any(m_xHttpsPortED->get_text().toInt32()) );
            bModified = true;
        }

        if( m_xFtpProxyED->get_value_changed_from_saved())
        {
            xPropertySet->setPropertyValue( g_aFtpProxyPN, Any(m_xFtpProxyED->get_text()) );
            bModified = true;
        }

        if ( m_xFtpPortED->get_value_changed_from_saved() )
        {
            xPropertySet->setPropertyValue( g_aFtpPortPN, Any(m_xFtpPortED->get_text().toInt32()));
            bModified = true;
        }

        if ( m_xNoProxyForED->get_value_changed_from_saved() )
        {
            xPropertySet->setPropertyValue( g_aNoProxyDescPN, Any( m_xNoProxyForED->get_text()));
            bModified = true;
        }

        Reference< util::XChangesBatch > xChangesBatch(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }
    catch (const css::lang::IllegalArgumentException &) {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const beans::UnknownPropertyException &) {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const beans::PropertyVetoException &) {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const css::lang::WrappedTargetException &) {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
    catch (const RuntimeException &) {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }

    return bModified;
}

void SvxProxyTabPage::EnableControls_Impl()
{
    m_xProxyModeLB->set_sensitive(!officecfg::Inet::Settings::ooInetNoProxy::isReadOnly());

    const bool bManualConfig = m_xProxyModeLB->get_active() == 2;

    const bool bHTTPProxyNameEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPProxyName::isReadOnly();
    const bool bHTTPProxyPortEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPProxyPort::isReadOnly();
    m_xHttpProxyFT->set_sensitive(bHTTPProxyNameEnabled);
    m_xHttpProxyED->set_sensitive(bHTTPProxyNameEnabled);
    m_xHttpPortFT->set_sensitive(bHTTPProxyPortEnabled);
    m_xHttpPortED->set_sensitive(bHTTPProxyPortEnabled);

    const bool bHTTPSProxyNameEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPSProxyName::isReadOnly();
    const bool bHTTPSProxyPortEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPSProxyPort::isReadOnly();
    m_xHttpsProxyFT->set_sensitive(bHTTPSProxyNameEnabled);
    m_xHttpsProxyED->set_sensitive(bHTTPSProxyNameEnabled);
    m_xHttpsPortFT->set_sensitive(bHTTPSProxyPortEnabled);
    m_xHttpsPortED->set_sensitive(bHTTPSProxyPortEnabled);

    const bool bFTPProxyNameEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetFTPProxyName::isReadOnly();
    const bool bFTPProxyPortEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetFTPProxyPort::isReadOnly();
    m_xFtpProxyFT->set_sensitive(bFTPProxyNameEnabled);
    m_xFtpProxyED->set_sensitive(bFTPProxyNameEnabled);
    m_xFtpPortFT->set_sensitive(bFTPProxyPortEnabled);
    m_xFtpPortED->set_sensitive(bFTPProxyPortEnabled);

    const bool bInetNoProxyEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetNoProxy::isReadOnly();
    m_xNoProxyForFT->set_sensitive(bInetNoProxyEnabled);
    m_xNoProxyForED->set_sensitive(bInetNoProxyEnabled);
    m_xNoProxyDescFT->set_sensitive(bInetNoProxyEnabled);
}

IMPL_LINK(SvxProxyTabPage, ProxyHdl_Impl, weld::ComboBox&, rBox, void)
{
    sal_Int32 nPos = rBox.get_active();

    // Restore original system values
    if( nPos == 1 )
    {
        ReadConfigDefaults_Impl();
    }

    EnableControls_Impl();
}

IMPL_STATIC_LINK(SvxProxyTabPage, LoseFocusHdl_Impl, weld::Widget&, rControl, void)
{
    weld::Entry* pEdit = dynamic_cast<weld::Entry*>(&rControl);
    if (pEdit && !isValidPort(pEdit->get_text()))
        pEdit->set_text(OUString('0'));
}

/********************************************************************/
/*                                                                  */
/*  SvxSecurityTabPage                                             */
/*                                                                  */
/********************************************************************/
SvxSecurityTabPage::SvxSecurityTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optsecuritypage.ui", "OptSecurityPage", &rSet)
    , mpSecOptions(new SvtSecurityOptions)
    , m_xSecurityOptionsPB(m_xBuilder->weld_button("options"))
    , m_xSavePasswordsCB(m_xBuilder->weld_check_button("savepassword"))
    , m_xShowConnectionsPB(m_xBuilder->weld_button("connections"))
    , m_xMasterPasswordCB(m_xBuilder->weld_check_button("usemasterpassword"))
    , m_xMasterPasswordFT(m_xBuilder->weld_label("masterpasswordtext"))
    , m_xMasterPasswordPB(m_xBuilder->weld_button("masterpassword"))
    , m_xMacroSecFrame(m_xBuilder->weld_container("macrosecurity"))
    , m_xMacroSecPB(m_xBuilder->weld_button("macro"))
    , m_xCertFrame(m_xBuilder->weld_container("certificatepath"))
    , m_xCertPathPB(m_xBuilder->weld_button("cert"))
    , m_xTSAURLsFrame(m_xBuilder->weld_container("tsaurls"))
    , m_xTSAURLsPB(m_xBuilder->weld_button("tsas"))
    , m_xNoPasswordSaveFT(m_xBuilder->weld_label("nopasswordsave"))
{
    //fdo#65595, we need height-for-width support here, but for now we can
    //bodge it
    Size aPrefSize(m_xSavePasswordsCB->get_preferred_size());
    int nMaxWidth = m_xSavePasswordsCB->get_approximate_digit_width() * 40;
    if (aPrefSize.Width() > nMaxWidth)
    {
        m_xSavePasswordsCB->set_label_line_wrap(true);
        m_xSavePasswordsCB->set_size_request(nMaxWidth, -1);
    }

    m_sPasswordStoringDeactivateStr = m_xNoPasswordSaveFT->get_label();

    InitControls();

    m_xSecurityOptionsPB->connect_clicked( LINK( this, SvxSecurityTabPage, SecurityOptionsHdl ) );
    m_xSavePasswordsCB->connect_clicked( LINK( this, SvxSecurityTabPage, SavePasswordHdl ) );
    m_xMasterPasswordPB->connect_clicked( LINK( this, SvxSecurityTabPage, MasterPasswordHdl ) );
    m_xMasterPasswordCB->connect_clicked( LINK( this, SvxSecurityTabPage, MasterPasswordCBHdl ) );
    m_xShowConnectionsPB->connect_clicked( LINK( this, SvxSecurityTabPage, ShowPasswordsHdl ) );
    m_xMacroSecPB->connect_clicked( LINK( this, SvxSecurityTabPage, MacroSecPBHdl ) );
    m_xCertPathPB->connect_clicked( LINK( this, SvxSecurityTabPage, CertPathPBHdl ) );
    m_xTSAURLsPB->connect_clicked( LINK( this, SvxSecurityTabPage, TSAURLsPBHdl ) );

    ActivatePage( rSet );
}

SvxSecurityTabPage::~SvxSecurityTabPage()
{
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SecurityOptionsHdl, weld::Button&, void)
{
    if (!m_xSecOptDlg)
        m_xSecOptDlg.reset(new svx::SecurityOptionsDialog(GetFrameWeld(), mpSecOptions.get()));
    m_xSecOptDlg->run();
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SavePasswordHdl, weld::Button&, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( m_xSavePasswordsCB->get_active() )
        {
            bool bOldValue = xMasterPasswd->allowPersistentStoring( true );
            xMasterPasswd->removeMasterPassword();

            uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                                  GetDialogController()->getDialog()->GetXWindow()));

            if ( xMasterPasswd->changeMasterPassword(xTmpHandler) )
            {
                m_xMasterPasswordPB->set_sensitive(true);
                m_xMasterPasswordCB->set_active(true);
                m_xMasterPasswordCB->set_sensitive(true);
                m_xMasterPasswordFT->set_sensitive(true);
                m_xShowConnectionsPB->set_sensitive(true);
            }
            else
            {
                xMasterPasswd->allowPersistentStoring( bOldValue );
                m_xSavePasswordsCB->set_active( false );
            }
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           m_sPasswordStoringDeactivateStr));
            xQueryBox->set_default_response(RET_NO);

            sal_uInt16 nRet = xQueryBox->run();

            if( RET_YES == nRet )
            {
                xMasterPasswd->allowPersistentStoring( false );
                m_xMasterPasswordCB->set_active(true);
                m_xMasterPasswordPB->set_sensitive( false );
                m_xMasterPasswordCB->set_sensitive( false );
                m_xMasterPasswordFT->set_sensitive( false );
                m_xShowConnectionsPB->set_sensitive( false );
            }
            else
            {
                m_xSavePasswordsCB->set_active(true);
                m_xMasterPasswordPB->set_sensitive(true);
                m_xShowConnectionsPB->set_sensitive(true);
            }
        }
    }
    catch (const Exception&)
    {
        m_xSavePasswordsCB->set_active( !m_xSavePasswordsCB->get_active() );
    }
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MasterPasswordHdl, weld::Button&, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                                  GetDialogController()->getDialog()->GetXWindow()));
            xMasterPasswd->changeMasterPassword(xTmpHandler);
        }
    }
    catch (const Exception&)
    {}
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MasterPasswordCBHdl, weld::Button&, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                              GetDialogController()->getDialog()->GetXWindow()));

        if ( m_xMasterPasswordCB->get_active() )
        {
            if (xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->changeMasterPassword(xTmpHandler))
            {
                m_xMasterPasswordPB->set_sensitive(true);
                m_xMasterPasswordFT->set_sensitive(true);
            }
            else
            {
                m_xMasterPasswordCB->set_active( false );
                m_xMasterPasswordPB->set_sensitive(true);
                m_xMasterPasswordFT->set_sensitive(true);
            }
        }
        else
        {
            if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->useDefaultMasterPassword(xTmpHandler) )
            {
                m_xMasterPasswordPB->set_sensitive( false );
                m_xMasterPasswordFT->set_sensitive( false );
            }
            else
            {
                m_xMasterPasswordCB->set_active(true);
                m_xMasterPasswordPB->set_sensitive(true);
                m_xShowConnectionsPB->set_sensitive(true);
            }
        }
    }
    catch (const Exception&)
    {
        m_xSavePasswordsCB->set_active( !m_xSavePasswordsCB->get_active() );
    }
}

IMPL_LINK_NOARG(SvxSecurityTabPage, ShowPasswordsHdl, weld::Button&, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                              GetDialogController()->getDialog()->GetXWindow()));

        if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->authorizateWithMasterPassword(xTmpHandler) )
        {
            svx::WebConnectionInfoDialog aDlg(GetFrameWeld());
            aDlg.run();
        }
    }
    catch (const Exception&)
    {}
}

IMPL_LINK_NOARG(SvxSecurityTabPage, CertPathPBHdl, weld::Button&, void)
{
    if (!mpCertPathDlg)
        mpCertPathDlg.reset(new CertPathDialog(GetFrameWeld()));
    mpCertPathDlg->Init();

    if (mpCertPathDlg->run() == RET_OK && !mpCertPathDlg->isActiveServicePath())
    {
        SolarMutexGuard aGuard;
        if (svtools::executeRestartDialog(comphelper::getProcessComponentContext(), nullptr, svtools::RESTART_REASON_ADDING_PATH))
            GetDialogController()->response(RET_OK);
    }
}

IMPL_LINK_NOARG(SvxSecurityTabPage, TSAURLsPBHdl, weld::Button&, void)
{
    // Unlike the mpCertPathDlg, we *don't* keep the same dialog object around between
    // invocations. Seems clearer to my little brain that way.
    TSAURLsDialog aTSAURLsDlg(GetFrameWeld());
    aTSAURLsDlg.run();
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MacroSecPBHdl, weld::Button&, void)
{
    try
    {
        Reference< security::XDocumentDigitalSignatures > xD(
            security::DocumentDigitalSignatures::createDefault(comphelper::getProcessComponentContext() ) );
        xD->setParentWindow(GetDialogController()->getDialog()->GetXWindow());
        xD->manageTrustedSources();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "cui.options", "");
    }
}

void SvxSecurityTabPage::InitControls()
{
#ifndef UNX
    m_xCertFrame->hide();
#endif

    m_xMasterPasswordPB->set_sensitive( false );
    m_xMasterPasswordCB->set_sensitive( false );
    m_xMasterPasswordCB->set_active(true);
    m_xMasterPasswordFT->set_sensitive( false );
    m_xShowConnectionsPB->set_sensitive( false );

    // initialize the password saving checkbox
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            m_xMasterPasswordCB->set_sensitive(true);
            m_xShowConnectionsPB->set_sensitive(true);
            m_xSavePasswordsCB->set_active(true);

            if ( xMasterPasswd->isDefaultMasterPasswordUsed() )
                m_xMasterPasswordCB->set_active( false );
            else
            {
                m_xMasterPasswordPB->set_sensitive(true);
                m_xMasterPasswordCB->set_active(true);
                m_xMasterPasswordFT->set_sensitive(true);
            }
        }
    }
    catch (const Exception&)
    {
        m_xSavePasswordsCB->set_sensitive( false );
    }
}

std::unique_ptr<SfxTabPage> SvxSecurityTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxSecurityTabPage>(pPage, pController, *rAttrSet);
}

void SvxSecurityTabPage::ActivatePage( const SfxItemSet& )
{
}

DeactivateRC SvxSecurityTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

namespace
{
    bool CheckAndSave( SvtSecurityOptions& _rOpt, SvtSecurityOptions::EOption _eOpt, const bool _bIsChecked, bool& _rModified )
    {
        bool bModified = false;
        if ( _rOpt.IsOptionEnabled( _eOpt ) )
        {
            bModified = _rOpt.IsOptionSet( _eOpt ) != _bIsChecked;
            if ( bModified )
            {
                _rOpt.SetOption( _eOpt, _bIsChecked );
                _rModified = true;
            }
        }

        return bModified;
    }
}

bool SvxSecurityTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;

    if (m_xSecOptDlg)
    {
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::DocWarnSaveOrSend, m_xSecOptDlg->IsSaveOrSendDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::DocWarnSigning, m_xSecOptDlg->IsSignDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::DocWarnPrint, m_xSecOptDlg->IsPrintDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::DocWarnCreatePdf, m_xSecOptDlg->IsCreatePdfChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo, m_xSecOptDlg->IsRemovePersInfoChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::DocWarnRecommendPassword, m_xSecOptDlg->IsRecommPasswdChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::CtrlClickHyperlink, m_xSecOptDlg->IsCtrlHyperlinkChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::EOption::BlockUntrustedRefererLinks, m_xSecOptDlg->IsBlockUntrustedRefererLinksChecked(), bModified );
    }

    return bModified;
}

/*--------------------------------------------------------------------*/

void SvxSecurityTabPage::Reset( const SfxItemSet* )
{
}

struct SvxEMailTabPage_Impl
{
    SvxEMailTabPage_Impl():
        sProgram(officecfg::Office::Common::ExternalMailer::Program::get()),
        bROProgram(
            officecfg::Office::Common::ExternalMailer::Program::isReadOnly()),
        bHideContent(
            officecfg::Office::Security::HiddenContent::RemoveHiddenContent::get()),
        bROHideContent(
            officecfg::Office::Security::HiddenContent::RemoveHiddenContent::isReadOnly())
    {}

    OUString sProgram;
    bool bROProgram;
    bool bHideContent;
    bool bROHideContent;
};

SvxEMailTabPage::SvxEMailTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage( pPage, pController, "cui/ui/optemailpage.ui", "OptEmailPage", &rSet)
    , pImpl(new SvxEMailTabPage_Impl)
    , m_xMailContainer(m_xBuilder->weld_container("program"))
    , m_xMailerURLFI(m_xBuilder->weld_image("lockemail"))
    , m_xMailerURLED(m_xBuilder->weld_entry("url"))
    , m_xMailerURLPB(m_xBuilder->weld_button("browse"))
    , m_xSuppressHiddenContainer(m_xBuilder->weld_container("suppressHiddenCont"))
    , m_xSuppressHiddenFI(m_xBuilder->weld_image("lockSuppressHidden"))
    , m_xSuppressHidden(m_xBuilder->weld_check_button("suppressHidden"))
    , m_xDefaultFilterFT(m_xBuilder->weld_label("browsetitle"))
{
    m_sDefaultFilterName = m_xDefaultFilterFT->get_label();
    m_xMailerURLPB->connect_clicked( LINK( this, SvxEMailTabPage, FileDialogHdl_Impl ) );
}

/* -------------------------------------------------------------------------*/

SvxEMailTabPage::~SvxEMailTabPage()
{
}

/* -------------------------------------------------------------------------*/

std::unique_ptr<SfxTabPage> SvxEMailTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxEMailTabPage>(pPage, pController, *rAttrSet);
}

/* -------------------------------------------------------------------------*/

bool SvxEMailTabPage::FillItemSet( SfxItemSet* )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    if (!pImpl->bROProgram && m_xMailerURLED->get_value_changed_from_saved())
    {
        pImpl->sProgram = m_xMailerURLED->get_text();
        officecfg::Office::Common::ExternalMailer::Program::set(
            pImpl->sProgram, batch);
    }
    if (!pImpl->bROHideContent
        && pImpl->bHideContent != m_xSuppressHidden->get_active())
    {
        pImpl->bHideContent = m_xSuppressHidden->get_active();
        officecfg::Office::Security::HiddenContent::RemoveHiddenContent::set(
            pImpl->bHideContent, batch);
    }
    batch->commit();
    return false;
}

/* -------------------------------------------------------------------------*/

void SvxEMailTabPage::Reset( const SfxItemSet* )
{
    m_xMailerURLED->set_sensitive(true);
    m_xMailerURLPB->set_sensitive(true);

    if (pImpl->bROProgram)
        m_xMailerURLFI->show();

    m_xMailerURLED->set_text(pImpl->sProgram);
    m_xMailerURLED->save_value();

    m_xMailContainer->set_sensitive(!pImpl->bROProgram);

    if (pImpl->bROHideContent)
        m_xSuppressHiddenFI->show();

    m_xSuppressHidden->set_active(pImpl->bHideContent);

    m_xSuppressHiddenContainer->set_sensitive(!pImpl->bROHideContent);
}

/* -------------------------------------------------------------------------*/

IMPL_LINK_NOARG(SvxEMailTabPage, FileDialogHdl_Impl, weld::Button&, void)
{
    if (pImpl->bROProgram)
        return;

    FileDialogHelper aHelper(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, GetFrameWeld());
    OUString sPath = m_xMailerURLED->get_text();
    if ( sPath.isEmpty() )
        sPath = "/usr/bin";

    OUString sUrl;
    osl::FileBase::getFileURLFromSystemPath(sPath, sUrl);
    aHelper.SetDisplayDirectory(sUrl);
    aHelper.AddFilter( m_sDefaultFilterName, "*");

    if ( ERRCODE_NONE == aHelper.Execute() )
    {
        sUrl = aHelper.GetPath();
        if (osl::FileBase::getSystemPathFromFileURL(sUrl, sPath)
            != osl::FileBase::E_None)
        {
            sPath.clear();
        }
        m_xMailerURLED->set_text(sPath);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

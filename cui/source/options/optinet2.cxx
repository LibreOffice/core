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
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/diagnose_ex.hxx>

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
#include <securityoptions.hxx>
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
                                                       CuiResId(RID_CUISTR_OPT_PROXYPORTS)));
        xErrorBox->run();
    }
}

constexpr OUString g_aProxyModePN = u"ooInetProxyType"_ustr;
constexpr OUString g_aHttpProxyPN = u"ooInetHTTPProxyName"_ustr;
constexpr OUString g_aHttpPortPN = u"ooInetHTTPProxyPort"_ustr;
constexpr OUString g_aHttpsProxyPN = u"ooInetHTTPSProxyName"_ustr;
constexpr OUString g_aHttpsPortPN = u"ooInetHTTPSProxyPort"_ustr;
constexpr OUString g_aNoProxyDescPN = u"ooInetNoProxy"_ustr;

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
    : SfxTabPage(pPage, pController, u"cui/ui/optproxypage.ui"_ustr, u"OptProxyPage"_ustr, &rSet)
    , m_xProxyModeFT(m_xBuilder->weld_label(u"label2"_ustr))
    , m_xProxyModeLB(m_xBuilder->weld_combo_box(u"proxymode"_ustr))
    , m_xProxyModeImg(m_xBuilder->weld_widget(u"lockproxymode"_ustr))
    , m_xHttpProxyFT(m_xBuilder->weld_label(u"httpft"_ustr))
    , m_xHttpProxyED(m_xBuilder->weld_entry(u"http"_ustr))
    , m_xHttpProxyImg(m_xBuilder->weld_widget(u"lockhttp"_ustr))
    , m_xHttpPortFT(m_xBuilder->weld_label(u"httpportft"_ustr))
    , m_xHttpPortED(m_xBuilder->weld_entry(u"httpport"_ustr))
    , m_xHttpPortImg(m_xBuilder->weld_widget(u"lockhttpport"_ustr))
    , m_xHttpsProxyFT(m_xBuilder->weld_label(u"httpsft"_ustr))
    , m_xHttpsProxyED(m_xBuilder->weld_entry(u"https"_ustr))
    , m_xHttpsProxyImg(m_xBuilder->weld_widget(u"lockhttps"_ustr))
    , m_xHttpsPortFT(m_xBuilder->weld_label(u"httpsportft"_ustr))
    , m_xHttpsPortED(m_xBuilder->weld_entry(u"httpsport"_ustr))
    , m_xHttpsPortImg(m_xBuilder->weld_widget(u"lockhttpsport"_ustr))
    , m_xNoProxyForFT(m_xBuilder->weld_label(u"noproxyft"_ustr))
    , m_xNoProxyForED(m_xBuilder->weld_entry(u"noproxy"_ustr))
    , m_xNoProxyForImg(m_xBuilder->weld_widget(u"locknoproxy"_ustr))
    , m_xNoProxyDescFT(m_xBuilder->weld_label(u"noproxydesc"_ustr))
{
    m_xHttpProxyED->connect_insert_text(LINK(this, SvxProxyTabPage, NoSpaceTextFilterHdl));
    m_xHttpPortED->connect_insert_text(LINK(this, SvxProxyTabPage, NumberOnlyTextFilterHdl));
    m_xHttpPortED->connect_changed(LINK(this, SvxProxyTabPage, PortChangedHdl));
    m_xHttpsProxyED->connect_insert_text(LINK(this, SvxProxyTabPage, NoSpaceTextFilterHdl));
    m_xHttpsPortED->connect_insert_text(LINK(this, SvxProxyTabPage, NumberOnlyTextFilterHdl));
    m_xHttpsPortED->connect_changed(LINK(this, SvxProxyTabPage, PortChangedHdl));

    Link<weld::Widget&,void> aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    m_xHttpPortED->connect_focus_out( aLink );
    m_xHttpsPortED->connect_focus_out( aLink );

    m_xProxyModeLB->connect_changed(LINK( this, SvxProxyTabPage, ProxyHdl_Impl ));

    Reference< css::lang::XMultiServiceFactory >
        xConfigurationProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

    beans::NamedValue aProperty;
    aProperty.Name  = "nodepath";
    aProperty.Value <<= u"org.openoffice.Inet/Settings"_ustr;

    Sequence< Any > aArgumentList{ Any(aProperty) };

    m_xConfigurationUpdateAccess = xConfigurationProvider->createInstanceWithArguments(
        u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr,
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
    else
        m_xHttpPortED->set_text( u""_ustr );

    m_xHttpsProxyED->set_text( officecfg::Inet::Settings::ooInetHTTPSProxyName::get() );
    x = officecfg::Inet::Settings::ooInetHTTPSProxyPort::get();
    if (x)
    {
        nIntValue = *x;
        m_xHttpsPortED->set_text( OUString::number( nIntValue ));
    }
    else
        m_xHttpsPortED->set_text( u""_ustr );

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
    m_xNoProxyForED->save_value();

    EnableControls_Impl();
}

OUString SvxProxyTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr,    u"label2"_ustr,     u"httpft"_ustr,      u"httpsft"_ustr,
                          u"noproxyft"_ustr, u"httpportft"_ustr, u"httpsportft"_ustr, u"noproxydesc"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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
    bool bEnable = !officecfg::Inet::Settings::ooInetNoProxy::isReadOnly();
    m_xProxyModeFT->set_sensitive(bEnable);
    m_xProxyModeLB->set_sensitive(bEnable);
    m_xProxyModeImg->set_visible(!bEnable);

    const bool bManualConfig = m_xProxyModeLB->get_active() == 2;

    bEnable = !officecfg::Inet::Settings::ooInetHTTPProxyName::isReadOnly();
    const bool bHTTPProxyNameEnabled = bManualConfig && bEnable;
    const bool bHTTPProxyPortEnabled = bManualConfig && bEnable;
    m_xHttpProxyFT->set_sensitive(bHTTPProxyNameEnabled);
    m_xHttpProxyED->set_sensitive(bHTTPProxyNameEnabled);
    m_xHttpProxyImg->set_visible(!bEnable);
    m_xHttpPortFT->set_sensitive(bHTTPProxyPortEnabled);
    m_xHttpPortED->set_sensitive(bHTTPProxyPortEnabled);
    m_xHttpPortImg->set_visible(!bEnable);

    bEnable = !officecfg::Inet::Settings::ooInetHTTPSProxyName::isReadOnly();
    const bool bHTTPSProxyNameEnabled = bManualConfig && bEnable;
    const bool bHTTPSProxyPortEnabled = bManualConfig && bEnable;
    m_xHttpsProxyFT->set_sensitive(bHTTPSProxyNameEnabled);
    m_xHttpsProxyED->set_sensitive(bHTTPSProxyNameEnabled);
    m_xHttpsProxyImg->set_visible(!bEnable);
    m_xHttpsPortFT->set_sensitive(bHTTPSProxyPortEnabled);
    m_xHttpsPortED->set_sensitive(bHTTPSProxyPortEnabled);
    m_xHttpsPortImg->set_visible(!bEnable);

    bEnable = !officecfg::Inet::Settings::ooInetNoProxy::isReadOnly();
    const bool bInetNoProxyEnabled = bManualConfig && bEnable;
    m_xNoProxyForFT->set_sensitive(bInetNoProxyEnabled);
    m_xNoProxyForED->set_sensitive(bInetNoProxyEnabled);
    m_xNoProxyForImg->set_visible(!bEnable);
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
    : SfxTabPage(pPage, pController, u"cui/ui/optsecuritypage.ui"_ustr, u"OptSecurityPage"_ustr, &rSet)
    , m_xSecurityOptionsPB(m_xBuilder->weld_button(u"options"_ustr))
    , m_xSavePasswordsCB(m_xBuilder->weld_check_button(u"savepassword"_ustr))
    , m_xSavePasswordsImg(m_xBuilder->weld_widget(u"locksavepassword"_ustr))
    , m_xShowConnectionsPB(m_xBuilder->weld_button(u"connections"_ustr))
    , m_xMasterPasswordCB(m_xBuilder->weld_check_button(u"usemasterpassword"_ustr))
    , m_xMasterPasswordImg(m_xBuilder->weld_widget(u"lockusemasterpassword"_ustr))
    , m_xMasterPasswordFT(m_xBuilder->weld_label(u"masterpasswordtext"_ustr))
    , m_xMasterPasswordPB(m_xBuilder->weld_button(u"masterpassword"_ustr))
    , m_xMacroSecFrame(m_xBuilder->weld_container(u"macrosecurity"_ustr))
    , m_xMacroSecPB(m_xBuilder->weld_button(u"macro"_ustr))
    , m_xCertFrame(m_xBuilder->weld_container(u"certificatepath"_ustr))
    , m_xCertPathPB(m_xBuilder->weld_button(u"cert"_ustr))
    , m_xCertPathImg(m_xBuilder->weld_widget(u"lockcertipath"_ustr))
    , m_xCertPathLabel(m_xBuilder->weld_label(u"label7"_ustr))
    , m_xTSAURLsFrame(m_xBuilder->weld_container(u"tsaurls"_ustr))
    , m_xTSAURLsPB(m_xBuilder->weld_button(u"tsas"_ustr))
    , m_xTSAURLsImg(m_xBuilder->weld_widget(u"locktsas"_ustr))
    , m_xTSAURLsLabel(m_xBuilder->weld_label(u"label9"_ustr))
    , m_xNoPasswordSaveFT(m_xBuilder->weld_label(u"nopasswordsave"_ustr))
    , m_xCertMgrPathLB(m_xBuilder->weld_button(u"browse"_ustr))
    , m_xParameterEdit(m_xBuilder->weld_entry(u"parameterfield"_ustr))
    , m_xCertMgrPathImg(m_xBuilder->weld_widget(u"lockcertimanager"_ustr))
    , m_xCertMgrPathLabel(m_xBuilder->weld_label(u"label11"_ustr))
{
    //fdo#65595, we need height-for-width support here, but for now we can
    //bodge it
    Size aPrefSize(m_xSavePasswordsCB->get_preferred_size());
    int nMaxWidth = m_xSavePasswordsCB->get_approximate_digit_width() * 40;
    if (aPrefSize.Width() > nMaxWidth)
    {
        m_xSavePasswordsCB->set_label_wrap(true);
        m_xSavePasswordsCB->set_size_request(nMaxWidth, -1);
    }

    m_sPasswordStoringDeactivateStr = m_xNoPasswordSaveFT->get_label();

    InitControls();

    m_xSecurityOptionsPB->connect_clicked( LINK( this, SvxSecurityTabPage, SecurityOptionsHdl ) );
    m_xSavePasswordsCB->connect_toggled( LINK( this, SvxSecurityTabPage, SavePasswordHdl ) );
    m_xMasterPasswordPB->connect_clicked( LINK( this, SvxSecurityTabPage, MasterPasswordHdl ) );
    m_xMasterPasswordCB->connect_toggled( LINK( this, SvxSecurityTabPage, MasterPasswordCBHdl ) );
    m_xShowConnectionsPB->connect_clicked( LINK( this, SvxSecurityTabPage, ShowPasswordsHdl ) );
    m_xMacroSecPB->connect_clicked( LINK( this, SvxSecurityTabPage, MacroSecPBHdl ) );
    m_xCertPathPB->connect_clicked( LINK( this, SvxSecurityTabPage, CertPathPBHdl ) );
    m_xTSAURLsPB->connect_clicked( LINK( this, SvxSecurityTabPage, TSAURLsPBHdl ) );
    m_xCertMgrPathLB->connect_clicked( LINK( this, SvxSecurityTabPage, CertMgrPBHdl ) );

    ActivatePage( rSet );
}

IMPL_LINK_NOARG(SvxSecurityTabPage, CertMgrPBHdl, weld::Button&, void)
{
    try
    {
        FileDialogHelper aHelper(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                 FileDialogFlags::NONE, nullptr);
        OUString sPath = m_xParameterEdit->get_text();
        if (sPath.isEmpty())
            sPath = "/usr/bin";

        OUString sUrl;
        osl::FileBase::getFileURLFromSystemPath(sPath, sUrl);
        aHelper.SetDisplayDirectory(sUrl);

        if (ERRCODE_NONE == aHelper.Execute())
        {
            sUrl = aHelper.GetPath();
            if (osl::FileBase::getSystemPathFromFileURL(sUrl, sPath) != osl::FileBase::E_None)
            {
                sPath.clear();
            }
            m_xParameterEdit->set_text(sPath);
        }
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        OUString sCurCertMgr = m_xParameterEdit->get_text();
        officecfg::Office::Common::Security::Scripting::CertMgrPath::set(sCurCertMgr, pBatch);
        pBatch->commit();
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "CertMgrPBHdl");
    }
}

SvxSecurityTabPage::~SvxSecurityTabPage()
{
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SecurityOptionsHdl, weld::Button&, void)
{
    if (!m_xSecOptDlg)
        m_xSecOptDlg.reset(new svx::SecurityOptionsDialog(GetFrameWeld()));
    m_xSecOptDlg->run();
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SavePasswordHdl, weld::Toggleable&, void)
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

IMPL_LINK_NOARG(SvxSecurityTabPage, MasterPasswordCBHdl, weld::Toggleable&, void)
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

        if (officecfg::Office::Common::Passwords::UseStorage::isReadOnly())
        {
            m_xSavePasswordsCB->set_sensitive(false);
            m_xShowConnectionsPB->set_sensitive(false);
            m_xSavePasswordsImg->set_visible(true);
            m_xMasterPasswordCB->set_sensitive(false);
            m_xMasterPasswordPB->set_sensitive(false);
            m_xMasterPasswordImg->set_visible(true);
        }
    }
    catch (const Exception&)
    {
        m_xSavePasswordsCB->set_sensitive( false );
    }

    try
    {
        OUString sCurCertMgr = officecfg::Office::Common::Security::Scripting::CertMgrPath::get();

        if (!sCurCertMgr.isEmpty())
            m_xParameterEdit->set_text(sCurCertMgr);

        bool bEnable = !officecfg::Office::Common::Security::Scripting::CertMgrPath::isReadOnly();
        m_xCertMgrPathLB->set_sensitive(bEnable);
        m_xParameterEdit->set_sensitive(bEnable);
        m_xCertMgrPathLabel->set_sensitive(bEnable);
        m_xCertMgrPathImg->set_visible(!bEnable);

        bEnable = !officecfg::Office::Common::Security::Scripting::TSAURLs::isReadOnly();
        m_xTSAURLsPB->set_sensitive(bEnable);
        m_xTSAURLsLabel->set_sensitive(bEnable);
        m_xTSAURLsImg->set_visible(!bEnable);

#ifndef UNX
        bEnable = !officecfg::Office::Common::Security::Scripting::CertDir::isReadOnly() ||
            !officecfg::Office::Common::Security::Scripting::ManualCertDir::isReadOnly();
        m_xCertPathPB->set_sensitive(bEnable);
        m_xCertPathLabel->set_sensitive(bEnable);
        m_xCertPathImg->set_visible(!bEnable);
#endif
    }
    catch (const uno::Exception&)
    {
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

OUString SvxSecurityTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr,         u"label4"_ustr,  u"label2"_ustr, u"masterpasswordtext"_ustr,
                          u"nopasswordsave"_ustr, u"label3"_ustr,  u"label5"_ustr, u"label8"_ustr,
                          u"label7"_ustr,         u"label10"_ustr, u"label9"_ustr, u"label12"_ustr,
                          u"label11"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"savepassword"_ustr, u"usemasterpassword"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    // TODO: Should we exclude button strings from the search?
    // button id: "browse" is excluded
    OUString buttons[] = { u"options"_ustr, u"connections"_ustr, u"masterpassword"_ustr, u"macro"_ustr, u"cert"_ustr, u"tsas"_ustr };

    for (const auto& btn : buttons)
    {
        if (const auto& pString = m_xBuilder->weld_button(btn))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxSecurityTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;

    if (m_xSecOptDlg) {
        bModified = m_xSecOptDlg->SetSecurityOptions();
    }

    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    if (m_xParameterEdit->get_value_changed_from_saved())
    {
        OUString sCurCertMgr = m_xParameterEdit->get_text();
        officecfg::Office::Common::Security::Scripting::CertMgrPath::set(sCurCertMgr, pBatch);
        pBatch->commit();
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
    : SfxTabPage( pPage, pController, u"cui/ui/optemailpage.ui"_ustr, u"OptEmailPage"_ustr, &rSet)
    , pImpl(new SvxEMailTabPage_Impl)
    , m_xMailContainer(m_xBuilder->weld_container(u"program"_ustr))
    , m_xMailerURLFI(m_xBuilder->weld_image(u"lockemail"_ustr))
    , m_xMailerURLED(m_xBuilder->weld_entry(u"url"_ustr))
    , m_xMailerURLPB(m_xBuilder->weld_button(u"browse"_ustr))
    , m_xSuppressHiddenContainer(m_xBuilder->weld_container(u"suppressHiddenCont"_ustr))
    , m_xSuppressHiddenFI(m_xBuilder->weld_image(u"lockSuppressHidden"_ustr))
    , m_xSuppressHidden(m_xBuilder->weld_check_button(u"suppressHidden"_ustr))
    , m_xDefaultFilterFT(m_xBuilder->weld_label(u"browsetitle"_ustr))
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

OUString SvxEMailTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"browsetitle"_ustr, u"suppress"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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
    aHelper.AddFilter( m_sDefaultFilterName, u"*"_ustr);

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

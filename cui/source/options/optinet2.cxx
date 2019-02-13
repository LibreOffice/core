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
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/config.hxx>
#include <vcl/weld.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/slstitm.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/filedlghelper.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/help.hxx>
#include <vcl/builderfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <dialmgr.hxx>
#include "optinet2.hxx"
#include <svx/svxdlg.hxx>
#include <svx/ofaitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/svxids.hrc>
#include <strings.hrc>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

#ifdef UNX
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>
#include <rtl/textenc.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>
#endif
#include <sal/types.h>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
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
#include <comphelper/solarmutex.hxx>

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

VCL_BUILDER_FACTORY_ARGS(SvxNoSpaceEdit, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK)

void SvxNoSpaceEdit::KeyInput( const KeyEvent& rKEvent )
{
    bool bValid = rKEvent.GetKeyCode().GetCode() != KEY_SPACE;
    if (bValid && bOnlyNumeric)
    {
        const vcl::KeyCode& rKeyCode = rKEvent.GetKeyCode();
        sal_uInt16 nGroup = rKeyCode.GetGroup();
        sal_uInt16 nKey = rKeyCode.GetCode();
        bValid = ( KEYGROUP_NUM == nGroup || KEYGROUP_CURSOR == nGroup ||
                 ( KEYGROUP_MISC == nGroup && ( nKey < KEY_ADD || nKey > KEY_EQUAL ) ) );
        if ( !bValid && ( rKeyCode.IsMod1() && (
             KEY_A == nKey || KEY_C == nKey || KEY_V == nKey || KEY_X == nKey || KEY_Z == nKey ) ) )
            // Erase, Copy, Paste, Select All and Undo should work
            bValid = true;
    }
    if (bValid)
        Edit::KeyInput(rKEvent);
}

void SvxNoSpaceEdit::Modify()
{
    Edit::Modify();

    if ( bOnlyNumeric )
    {
        if ( !isValidPort(GetText()) )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Warning, VclButtonsType::Ok,
                                                           CuiResId( RID_SVXSTR_OPT_PROXYPORTS)));
            xErrorBox->run();
        }
    }
}

bool SvxNoSpaceEdit::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "only-numeric")
        bOnlyNumeric = toBool(rValue);
    else
        return Edit::set_property(rKey, rValue);
    return true;
}


static const char g_aProxyModePN[] = "ooInetProxyType";
static const char g_aHttpProxyPN[] = "ooInetHTTPProxyName";
static const char g_aHttpPortPN[] = "ooInetHTTPProxyPort";
static const char g_aHttpsProxyPN[] = "ooInetHTTPSProxyName";
static const char g_aHttpsPortPN[] = "ooInetHTTPSProxyPort";
static const char g_aFtpProxyPN[] = "ooInetFTPProxyName";
static const char g_aFtpPortPN[] = "ooInetFTPProxyPort";
static const char g_aNoProxyDescPN[] = "ooInetNoProxy";

/********************************************************************/
/*                                                                  */
/*  SvxProxyTabPage                                                 */
/*                                                                  */
/********************************************************************/

SvxProxyTabPage::SvxProxyTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptProxyPage","cui/ui/optproxypage.ui", &rSet)
{
    get(m_pProxyModeLB, "proxymode");

    get(m_pHttpProxyFT, "httpft");
    get(m_pHttpProxyED, "http");
    get(m_pHttpPortFT, "httpportft");
    get(m_pHttpPortED, "httpport");

    get(m_pHttpsProxyFT, "httpsft");
    get(m_pHttpsProxyED, "https");
    get(m_pHttpsPortFT, "httpsportft");
    get(m_pHttpsPortED, "httpsport");

    get(m_pFtpProxyFT, "ftpft");
    get(m_pFtpProxyED, "ftp");
    get(m_pFtpPortFT, "ftpportft");
    get(m_pFtpPortED, "ftpport");

    get(m_pNoProxyForFT, "noproxyft");
    get(m_pNoProxyForED, "noproxy");
    get(m_pNoProxyDescFT, "noproxydesc");

    Link<Control&,void> aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    m_pHttpPortED->SetLoseFocusHdl( aLink );
    m_pHttpsPortED->SetLoseFocusHdl( aLink );
    m_pFtpPortED->SetLoseFocusHdl( aLink );

    m_pProxyModeLB->SetSelectHdl(LINK( this, SvxProxyTabPage, ProxyHdl_Impl ));

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
    disposeOnce();
}

void SvxProxyTabPage::dispose()
{
    m_pProxyModeLB.clear();
    m_pHttpProxyFT.clear();
    m_pHttpProxyED.clear();
    m_pHttpPortFT.clear();
    m_pHttpPortED.clear();
    m_pHttpsProxyFT.clear();
    m_pHttpsProxyED.clear();
    m_pHttpsPortFT.clear();
    m_pHttpsPortED.clear();
    m_pFtpProxyFT.clear();
    m_pFtpProxyED.clear();
    m_pFtpPortFT.clear();
    m_pFtpPortED.clear();
    m_pNoProxyForFT.clear();
    m_pNoProxyForED.clear();
    m_pNoProxyDescFT.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxProxyTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxProxyTabPage>::Create(pParent.pParent, *rAttrSet);
}

void SvxProxyTabPage::ReadConfigData_Impl()
{
    try {
        Reference< container::XNameAccess > xNameAccess(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        sal_Int32 nIntValue = 0;
        OUString  aStringValue;

        if( xNameAccess->getByName(g_aProxyModePN) >>= nIntValue )
        {
            m_pProxyModeLB->SelectEntryPos( nIntValue );
        }

        if( xNameAccess->getByName(g_aHttpProxyPN) >>= aStringValue )
        {
            m_pHttpProxyED->SetText( aStringValue );
        }

        if( xNameAccess->getByName(g_aHttpPortPN) >>= nIntValue )
        {
            m_pHttpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(g_aHttpsProxyPN) >>= aStringValue )
        {
            m_pHttpsProxyED->SetText( aStringValue );
        }

        if( xNameAccess->getByName(g_aHttpsPortPN) >>= nIntValue )
        {
            m_pHttpsPortED->SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(g_aFtpProxyPN) >>= aStringValue )
        {
            m_pFtpProxyED->SetText( aStringValue );
        }

        if( xNameAccess->getByName(g_aFtpPortPN) >>= nIntValue )
        {
            m_pFtpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(g_aNoProxyDescPN) >>= aStringValue )
        {
            m_pNoProxyForED->SetText( aStringValue );
        }
    }
    catch (const container::NoSuchElementException&) {
        SAL_WARN("cui.options", "SvxProxyTabPage::ReadConfigData_Impl: NoSuchElementException caught" );
    }
    catch (const css::lang::WrappedTargetException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::ReadConfigData_Impl: WrappedTargetException caught" );
    }
    catch (const RuntimeException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::ReadConfigData_Impl: RuntimeException caught" );
    }
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
            m_pHttpProxyED->SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(g_aHttpPortPN) >>= nIntValue )
        {
            m_pHttpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(g_aHttpsProxyPN) >>= aStringValue )
        {
            m_pHttpsProxyED->SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(g_aHttpsPortPN) >>= nIntValue )
        {
            m_pHttpsPortED->SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(g_aFtpProxyPN) >>= aStringValue )
        {
            m_pFtpProxyED->SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(g_aFtpPortPN) >>= nIntValue )
        {
            m_pFtpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(g_aNoProxyDescPN) >>= aStringValue )
        {
            m_pNoProxyForED->SetText( aStringValue );
        }
    }
    catch (const beans::UnknownPropertyException &)
    {
        SAL_WARN("cui.options", "SvxProxyTabPage::RestoreConfigDefaults_Impl: UnknownPropertyException caught" );
    }
    catch (const css::lang::WrappedTargetException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::RestoreConfigDefaults_Impl: WrappedTargetException caught" );
    }
    catch (const RuntimeException &)
    {
        SAL_WARN("cui.options", "SvxProxyTabPage::RestoreConfigDefaults_Impl: RuntimeException caught" );
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
        SAL_WARN("cui.options", "SvxProxyTabPage::RestoreConfigDefaults_Impl: UnknownPropertyException caught" );
    }
    catch (const css::lang::WrappedTargetException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::RestoreConfigDefaults_Impl: WrappedTargetException caught" );
    }
    catch (const RuntimeException &)
    {
        SAL_WARN("cui.options", "SvxProxyTabPage::RestoreConfigDefaults_Impl: RuntimeException caught" );
    }
}

void SvxProxyTabPage::Reset(const SfxItemSet*)
{
    ReadConfigData_Impl();

    m_pProxyModeLB->SaveValue();
    m_pHttpProxyED->SaveValue();
    m_pHttpPortED->SaveValue();
    m_pHttpsProxyED->SaveValue();
    m_pHttpsPortED->SaveValue();
    m_pFtpProxyED->SaveValue();
    m_pFtpPortED->SaveValue();
    m_pNoProxyForED->SaveValue();

    EnableControls_Impl();
}

bool SvxProxyTabPage::FillItemSet(SfxItemSet* )
{
    bool bModified = false;

    try {
        Reference< beans::XPropertySet > xPropertySet(m_xConfigurationUpdateAccess, UNO_QUERY_THROW );

        sal_Int32 nSelPos = m_pProxyModeLB->GetSelectedEntryPos();
        if(m_pProxyModeLB->IsValueChangedFromSaved())
        {
            if( nSelPos == 1 )
            {
                RestoreConfigDefaults_Impl();
                return true;
            }

            xPropertySet->setPropertyValue(g_aProxyModePN, Any(nSelPos));
            bModified = true;
        }

        if(m_pHttpProxyED->IsValueChangedFromSaved())
        {
            xPropertySet->setPropertyValue( g_aHttpProxyPN, Any(m_pHttpProxyED->GetText()));
            bModified = true;
        }

        if ( m_pHttpPortED->IsValueChangedFromSaved())
        {
            xPropertySet->setPropertyValue( g_aHttpPortPN, Any(m_pHttpPortED->GetText().toInt32()));
            bModified = true;
        }

        if( m_pHttpsProxyED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( g_aHttpsProxyPN, Any(m_pHttpsProxyED->GetText()) );
            bModified = true;
        }

        if ( m_pHttpsPortED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( g_aHttpsPortPN, Any(m_pHttpsPortED->GetText().toInt32()) );
            bModified = true;
        }

        if( m_pFtpProxyED->IsValueChangedFromSaved())
        {
            xPropertySet->setPropertyValue( g_aFtpProxyPN, Any(m_pFtpProxyED->GetText()) );
            bModified = true;
        }

        if ( m_pFtpPortED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( g_aFtpPortPN, Any(m_pFtpPortED->GetText().toInt32()));
            bModified = true;
        }

        if ( m_pNoProxyForED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( g_aNoProxyDescPN, Any( m_pNoProxyForED->GetText()));
            bModified = true;
        }

        Reference< util::XChangesBatch > xChangesBatch(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }
    catch (const css::lang::IllegalArgumentException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::FillItemSet: IllegalArgumentException caught" );
    }
    catch (const beans::UnknownPropertyException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::FillItemSet: UnknownPropertyException caught" );
    }
    catch (const beans::PropertyVetoException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::FillItemSet: PropertyVetoException caught" );
    }
    catch (const css::lang::WrappedTargetException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::FillItemSet: WrappedTargetException caught" );
    }
    catch (const RuntimeException &) {
        SAL_WARN("cui.options", "SvxProxyTabPage::FillItemSet: RuntimeException caught" );
    }

    return bModified;
}

void SvxProxyTabPage::EnableControls_Impl()
{
    m_pProxyModeLB->Enable(!officecfg::Inet::Settings::ooInetNoProxy::isReadOnly());

    const bool bManualConfig = m_pProxyModeLB->GetSelectedEntryPos() == 2;

    const bool bHTTPProxyNameEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPProxyName::isReadOnly();
    const bool bHTTPProxyPortEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPProxyPort::isReadOnly();
    m_pHttpProxyFT->Enable(bHTTPProxyNameEnabled);
    m_pHttpProxyED->Enable(bHTTPProxyNameEnabled);
    m_pHttpPortFT->Enable(bHTTPProxyPortEnabled);
    m_pHttpPortED->Enable(bHTTPProxyPortEnabled);

    const bool bHTTPSProxyNameEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPSProxyName::isReadOnly();
    const bool bHTTPSProxyPortEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetHTTPSProxyPort::isReadOnly();
    m_pHttpsProxyFT->Enable(bHTTPSProxyNameEnabled);
    m_pHttpsProxyED->Enable(bHTTPSProxyNameEnabled);
    m_pHttpsPortFT->Enable(bHTTPSProxyPortEnabled);
    m_pHttpsPortED->Enable(bHTTPSProxyPortEnabled);

    const bool bFTPProxyNameEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetFTPProxyName::isReadOnly();
    const bool bFTPProxyPortEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetFTPProxyPort::isReadOnly();
    m_pFtpProxyFT->Enable(bFTPProxyNameEnabled);
    m_pFtpProxyED->Enable(bFTPProxyNameEnabled);
    m_pFtpPortFT->Enable(bFTPProxyPortEnabled);
    m_pFtpPortED->Enable(bFTPProxyPortEnabled);

    const bool bInetNoProxyEnabled = bManualConfig && !officecfg::Inet::Settings::ooInetNoProxy::isReadOnly();
    m_pNoProxyForFT->Enable(bInetNoProxyEnabled);
    m_pNoProxyForED->Enable(bInetNoProxyEnabled);
    m_pNoProxyDescFT->Enable(bInetNoProxyEnabled);
}


IMPL_LINK( SvxProxyTabPage, ProxyHdl_Impl, ListBox&, rBox, void )
{
    sal_Int32 nPos = rBox.GetSelectedEntryPos();

    // Restore original system values
    if( nPos == 1 )
    {
        ReadConfigDefaults_Impl();
    }

    EnableControls_Impl();
}


IMPL_STATIC_LINK( SvxProxyTabPage, LoseFocusHdl_Impl, Control&, rControl, void )
{
    Edit* pEdit = static_cast<Edit*>(&rControl);
    if ( !isValidPort(pEdit->GetText()) )
        pEdit->SetText( OUString('0') );
}



/********************************************************************/
/*                                                                  */
/*  SvxSecurityTabPage                                             */
/*                                                                  */
/********************************************************************/

SvxSecurityTabPage::SvxSecurityTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptSecurityPage", "cui/ui/optsecuritypage.ui", &rSet)
    , mpSecOptions(new SvtSecurityOptions)
{
    get(m_pSecurityOptionsPB, "options");
    get(m_pSavePasswordsCB, "savepassword");

    //fdo#65595, we need height-for-width support here, but for now we can
    //bodge it
    Size aPrefSize(m_pSavePasswordsCB->get_preferred_size());
    Size aSize(m_pSavePasswordsCB->CalcMinimumSize(56*approximate_char_width()));
    if (aPrefSize.Width() > aSize.Width())
    {
        m_pSavePasswordsCB->set_width_request(aSize.Width());
        m_pSavePasswordsCB->set_height_request(aSize.Height());
    }

    get(m_pShowConnectionsPB, "connections");
    get(m_pMasterPasswordCB, "usemasterpassword");
    get(m_pMasterPasswordFT, "masterpasswordtext");
    get(m_pMasterPasswordPB, "masterpassword");
    get(m_pMacroSecFrame, "macrosecurity");
    get(m_pMacroSecPB, "macro");
    get(m_pCertFrame, "certificatepath");
    get(m_pCertPathPB, "cert");
    get(m_pTSAURLsFrame, "tsaurls");
    get(m_pTSAURLsPB, "tsas");
    m_sPasswordStoringDeactivateStr = get<FixedText>("nopasswordsave")->GetText();

    InitControls();

    m_pSecurityOptionsPB->SetClickHdl( LINK( this, SvxSecurityTabPage, SecurityOptionsHdl ) );
    m_pSavePasswordsCB->SetClickHdl( LINK( this, SvxSecurityTabPage, SavePasswordHdl ) );
    m_pMasterPasswordPB->SetClickHdl( LINK( this, SvxSecurityTabPage, MasterPasswordHdl ) );
    m_pMasterPasswordCB->SetClickHdl( LINK( this, SvxSecurityTabPage, MasterPasswordCBHdl ) );
    m_pShowConnectionsPB->SetClickHdl( LINK( this, SvxSecurityTabPage, ShowPasswordsHdl ) );
    m_pMacroSecPB->SetClickHdl( LINK( this, SvxSecurityTabPage, MacroSecPBHdl ) );
    m_pCertPathPB->SetClickHdl( LINK( this, SvxSecurityTabPage, CertPathPBHdl ) );
    m_pTSAURLsPB->SetClickHdl( LINK( this, SvxSecurityTabPage, TSAURLsPBHdl ) );

    ActivatePage( rSet );
}

SvxSecurityTabPage::~SvxSecurityTabPage()
{
    disposeOnce();
}

void SvxSecurityTabPage::dispose()
{
    mpSecOptions.reset();
    mpCertPathDlg.reset();
    m_xSecOptDlg.reset();
    m_pSecurityOptionsPB.clear();
    m_pSavePasswordsCB.clear();
    m_pShowConnectionsPB.clear();
    m_pMasterPasswordCB.clear();
    m_pMasterPasswordFT.clear();
    m_pMasterPasswordPB.clear();
    m_pMacroSecFrame.clear();
    m_pMacroSecPB.clear();
    m_pCertFrame.clear();
    m_pCertPathPB.clear();
    m_pTSAURLsFrame.clear();
    m_pTSAURLsPB.clear();

    SfxTabPage::dispose();
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SecurityOptionsHdl, Button*, void)
{
    if (!m_xSecOptDlg)
        m_xSecOptDlg.reset(new svx::SecurityOptionsDialog(GetFrameWeld(), mpSecOptions.get()));
    (void)m_xSecOptDlg->run();
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SavePasswordHdl, Button*, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( m_pSavePasswordsCB->IsChecked() )
        {
            bool bOldValue = xMasterPasswd->allowPersistentStoring( true );
            xMasterPasswd->removeMasterPassword();

            uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                                  VCLUnoHelper::GetInterface(GetParentDialog())));

            if ( xMasterPasswd->changeMasterPassword(xTmpHandler) )
            {
                m_pMasterPasswordPB->Enable();
                m_pMasterPasswordCB->Check();
                m_pMasterPasswordCB->Enable();
                m_pMasterPasswordFT->Enable();
                m_pShowConnectionsPB->Enable();
            }
            else
            {
                xMasterPasswd->allowPersistentStoring( bOldValue );
                m_pSavePasswordsCB->Check( false );
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
                m_pMasterPasswordCB->Check();
                m_pMasterPasswordPB->Enable( false );
                m_pMasterPasswordCB->Enable( false );
                m_pMasterPasswordFT->Enable( false );
                m_pShowConnectionsPB->Enable( false );
            }
            else
            {
                m_pSavePasswordsCB->Check();
                m_pMasterPasswordPB->Enable();
                m_pShowConnectionsPB->Enable();
            }
        }
    }
    catch (const Exception&)
    {
        m_pSavePasswordsCB->Check( !m_pSavePasswordsCB->IsChecked() );
    }
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MasterPasswordHdl, Button*, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                                  VCLUnoHelper::GetInterface(GetParentDialog())));
            xMasterPasswd->changeMasterPassword(xTmpHandler);
        }
    }
    catch (const Exception&)
    {}
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MasterPasswordCBHdl, Button*, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                              VCLUnoHelper::GetInterface(GetParentDialog())));

        if ( m_pMasterPasswordCB->IsChecked() )
        {
            if (xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->changeMasterPassword(xTmpHandler))
            {
                m_pMasterPasswordPB->Enable();
                m_pMasterPasswordFT->Enable();
            }
            else
            {
                m_pMasterPasswordCB->Check( false );
                m_pMasterPasswordPB->Enable();
                m_pMasterPasswordFT->Enable();
            }
        }
        else
        {
            if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->useDefaultMasterPassword(xTmpHandler) )
            {
                m_pMasterPasswordPB->Enable( false );
                m_pMasterPasswordFT->Enable( false );
            }
            else
            {
                m_pMasterPasswordCB->Check();
                m_pMasterPasswordPB->Enable();
                m_pShowConnectionsPB->Enable();
            }
        }
    }
    catch (const Exception&)
    {
        m_pSavePasswordsCB->Check( !m_pSavePasswordsCB->IsChecked() );
    }
}

IMPL_LINK_NOARG(SvxSecurityTabPage, ShowPasswordsHdl, Button*, void)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        uno::Reference<task::XInteractionHandler> xTmpHandler(task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                              VCLUnoHelper::GetInterface(GetParentDialog())));

        if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->authorizateWithMasterPassword(xTmpHandler) )
        {
            svx::WebConnectionInfoDialog aDlg(GetDialogFrameWeld());
            aDlg.run();
        }
    }
    catch (const Exception&)
    {}
}

IMPL_LINK_NOARG(SvxSecurityTabPage, CertPathPBHdl, Button*, void)
{
    if (!mpCertPathDlg)
        mpCertPathDlg.reset(new CertPathDialog(GetDialogFrameWeld()));

    OUString sOrig = mpCertPathDlg->getDirectory();
    short nRet = mpCertPathDlg->run();

    if (nRet == RET_OK && sOrig != mpCertPathDlg->getDirectory())
    {
        SolarMutexGuard aGuard;
        if (svtools::executeRestartDialog(comphelper::getProcessComponentContext(), nullptr, svtools::RESTART_REASON_ADDING_PATH))
            GetParentDialog()->EndDialog(RET_OK);
    }
}

IMPL_LINK_NOARG(SvxSecurityTabPage, TSAURLsPBHdl, Button*, void)
{
    // Unlike the mpCertPathDlg, we *don't* keep the same dialog object around between
    // invocations. Seems clearer to my little brain that way.
    TSAURLsDialog aTSAURLsDlg(GetDialogFrameWeld());
    aTSAURLsDlg.run();
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MacroSecPBHdl, Button*, void)
{
    try
    {
        Reference< security::XDocumentDigitalSignatures > xD(
            security::DocumentDigitalSignatures::createDefault(comphelper::getProcessComponentContext() ) );
        xD->setParentWindow(VCLUnoHelper::GetInterface(GetParentDialog()));
        xD->manageTrustedSources();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "cui.options", e);
    }
}


void SvxSecurityTabPage::InitControls()
{
    // Hide all controls which belong to the macro security button in case the macro
    // security settings managed by the macro security dialog opened via the button
    // are all readonly or if the macros are disabled in general.
    // @@@ Better would be to query the dialog whether it is 'useful' or not. Exposing
    //     macro security dialog implementations here, which is bad.
    if (    mpSecOptions->IsMacroDisabled()
         || (    mpSecOptions->IsReadOnly( SvtSecurityOptions::EOption::MacroSecLevel )
              && mpSecOptions->IsReadOnly( SvtSecurityOptions::EOption::MacroTrustedAuthors )
              && mpSecOptions->IsReadOnly( SvtSecurityOptions::EOption::SecureUrls ) ) )
    {
        //Hide these
        m_pMacroSecFrame->Hide();
    }

#ifndef UNX
    m_pCertFrame->Hide();
#endif

    m_pMasterPasswordPB->Enable( false );
    m_pMasterPasswordCB->Enable( false );
    m_pMasterPasswordCB->Check();
    m_pMasterPasswordFT->Enable( false );
    m_pShowConnectionsPB->Enable( false );

    // initialize the password saving checkbox
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            m_pMasterPasswordCB->Enable();
            m_pShowConnectionsPB->Enable();
            m_pSavePasswordsCB->Check();

            if ( xMasterPasswd->isDefaultMasterPasswordUsed() )
                m_pMasterPasswordCB->Check( false );
            else
            {
                m_pMasterPasswordPB->Enable();
                m_pMasterPasswordCB->Check();
                m_pMasterPasswordFT->Enable();
            }
        }
    }
    catch (const Exception&)
    {
        m_pSavePasswordsCB->Enable( false );
    }
}

VclPtr<SfxTabPage> SvxSecurityTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxSecurityTabPage>::Create(pParent.pParent, *rAttrSet);
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
    bool CheckAndSave( SvtSecurityOptions& _rOpt, SvtSecurityOptions::EOption _eOpt, const bool _bIsChecked, bool& _rModfied )
    {
        bool bModified = false;
        if ( _rOpt.IsOptionEnabled( _eOpt ) )
        {
            bModified = _rOpt.IsOptionSet( _eOpt ) != _bIsChecked;
            if ( bModified )
            {
                _rOpt.SetOption( _eOpt, _bIsChecked );
                _rModfied = true;
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

SvxEMailTabPage::SvxEMailTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, "OptEmailPage", "cui/ui/optemailpage.ui", &rSet)
    , pImpl(new SvxEMailTabPage_Impl)
{
    get(m_pMailContainer, "program");
    get(m_pMailerURLFI, "lockemail");
    get(m_pMailerURLED, "url");
    get(m_pMailerURLPB, "browse");
    get(m_pSuppressHiddenContainer, "suppressHiddenCont");
    get(m_pSuppressHiddenFI, "lockSuppressHidden");
    get(m_pSuppressHidden, "suppressHidden");
    m_sDefaultFilterName = get<FixedText>("browsetitle")->GetText();
    m_pMailerURLPB->SetClickHdl( LINK( this, SvxEMailTabPage, FileDialogHdl_Impl ) );
}

/* -------------------------------------------------------------------------*/

SvxEMailTabPage::~SvxEMailTabPage()
{
    disposeOnce();
}

void SvxEMailTabPage::dispose()
{
    pImpl.reset();
    m_pMailContainer.clear();
    m_pMailerURLFI.clear();
    m_pMailerURLED.clear();
    m_pMailerURLPB.clear();
    m_pSuppressHiddenContainer.clear();
    m_pSuppressHiddenFI.clear();
    m_pSuppressHidden.clear();
    SfxTabPage::dispose();
}

/* -------------------------------------------------------------------------*/

VclPtr<SfxTabPage>  SvxEMailTabPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxEMailTabPage>::Create(pParent.pParent, *rAttrSet);
}

/* -------------------------------------------------------------------------*/

bool SvxEMailTabPage::FillItemSet( SfxItemSet* )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    if (!pImpl->bROProgram && m_pMailerURLED->IsValueChangedFromSaved())
    {
        pImpl->sProgram = m_pMailerURLED->GetText();
        officecfg::Office::Common::ExternalMailer::Program::set(
            pImpl->sProgram, batch);
    }
    if (!pImpl->bROHideContent
        && pImpl->bHideContent != m_pSuppressHidden->IsChecked())
    {
        pImpl->bHideContent = m_pSuppressHidden->IsChecked();
        officecfg::Office::Security::HiddenContent::RemoveHiddenContent::set(
            pImpl->bHideContent, batch);
    }
    batch->commit();
    return false;
}

/* -------------------------------------------------------------------------*/

void SvxEMailTabPage::Reset( const SfxItemSet* )
{
    m_pMailerURLED->Enable();
    m_pMailerURLPB->Enable();

    if (pImpl->bROProgram)
        m_pMailerURLFI->Show();

    m_pMailerURLED->SetText(pImpl->sProgram);
    m_pMailerURLED->SaveValue();

    m_pMailContainer->Enable(!pImpl->bROProgram);

    if (pImpl->bROHideContent)
        m_pSuppressHiddenFI->Show();

    m_pSuppressHidden->Check(pImpl->bHideContent);

    m_pSuppressHiddenContainer->Enable(!pImpl->bROHideContent);
}

/* -------------------------------------------------------------------------*/

IMPL_LINK(  SvxEMailTabPage, FileDialogHdl_Impl, Button*, pButton, void )
{
    if (m_pMailerURLPB == pButton && !pImpl->bROProgram)
    {
        FileDialogHelper aHelper(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, GetFrameWeld());
        OUString sPath = m_pMailerURLED->GetText();
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
            m_pMailerURLED->SetText(sPath);
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

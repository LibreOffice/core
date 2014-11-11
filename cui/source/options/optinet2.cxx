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

#include <tools/config.hxx>
#include <vcl/msgbox.hxx>
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
#include <vcl/layout.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <dialmgr.hxx>
#include "optinet2.hxx"
#include <svx/svxdlg.hxx>
#include <cuires.hrc>
#include "helpid.hrc"
#include <svx/ofaitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/svxids.hrc>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>

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

#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/task/PasswordContainer.hpp"
#include "com/sun/star/task/XPasswordContainer2.hpp"
#include "securityoptions.hxx"
#include "webconninfo.hxx"
#include "certpath.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;

// static ----------------------------------------------------------------

#include <sal/config.h>



extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSvxNoSpaceEdit(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new SvxNoSpaceEdit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
}

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
            // Erase, Copy, Paste, Select All und Undo soll funktionieren
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
        OUString aValue = GetText();

        if ( !comphelper::string::isdigitAsciiString(aValue) || (long)aValue.toInt32() > USHRT_MAX )
            // the maximum value of a port number is USHRT_MAX
            MessageDialog( this, CUI_RES( RID_SVXSTR_OPT_PROXYPORTS ) ).Execute();
    }
}

bool SvxNoSpaceEdit::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "only-numeric")
        bOnlyNumeric = toBool(rValue);
    else
        return Edit::set_property(rKey, rValue);
    return true;
}


/********************************************************************/
/*                                                                  */
/*  SvxProxyTabPage                                                 */
/*                                                                  */
/********************************************************************/

SvxProxyTabPage::SvxProxyTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptProxyPage","cui/ui/optproxypage.ui", &rSet)
    , aProxyModePN("ooInetProxyType")
    , aHttpProxyPN("ooInetHTTPProxyName")
    , aHttpPortPN("ooInetHTTPProxyPort")
    , aHttpsProxyPN("ooInetHTTPSProxyName")
    , aHttpsPortPN("ooInetHTTPSProxyPort")
    , aFtpProxyPN("ooInetFTPProxyName")
    , aFtpPortPN("ooInetFTPProxyPort")
    , aNoProxyDescPN("ooInetNoProxy")
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

    Link aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    m_pHttpPortED->SetLoseFocusHdl( aLink );
    m_pHttpsPortED->SetLoseFocusHdl( aLink );
    m_pFtpPortED->SetLoseFocusHdl( aLink );

    m_pProxyModeLB->SetSelectHdl(LINK( this, SvxProxyTabPage, ProxyHdl_Impl ));

    Reference< com::sun::star::lang::XMultiServiceFactory >
        xConfigurationProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

    OUString aConfigRoot( "org.openoffice.Inet/Settings" );

    beans::NamedValue aProperty;
    aProperty.Name  = "nodepath";
    aProperty.Value = makeAny( aConfigRoot );

    Sequence< Any > aArgumentList( 1 );
    aArgumentList[0] = makeAny( aProperty );

    m_xConfigurationUpdateAccess = xConfigurationProvider->createInstanceWithArguments(
        OUString( "com.sun.star.configuration.ConfigurationUpdateAccess" ),
        aArgumentList );
}

SvxProxyTabPage::~SvxProxyTabPage()
{
}

SfxTabPage* SvxProxyTabPage::Create(vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return new SvxProxyTabPage(pParent, *rAttrSet);
}

void SvxProxyTabPage::ReadConfigData_Impl()
{
    try {
        Reference< container::XNameAccess > xNameAccess(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        sal_Int32 nIntValue = 0;
        OUString  aStringValue;

        if( xNameAccess->getByName(aProxyModePN) >>= nIntValue )
        {
            m_pProxyModeLB->SelectEntryPos( nIntValue );
        }

        if( xNameAccess->getByName(aHttpProxyPN) >>= aStringValue )
        {
            m_pHttpProxyED->SetText( aStringValue );
        }

        if( xNameAccess->getByName(aHttpPortPN) >>= nIntValue )
        {
            m_pHttpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(aHttpsProxyPN) >>= aStringValue )
        {
            m_pHttpsProxyED->SetText( aStringValue );
        }

        if( xNameAccess->getByName(aHttpsPortPN) >>= nIntValue )
        {
            m_pHttpsPortED->SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(aFtpProxyPN) >>= aStringValue )
        {
            m_pFtpProxyED->SetText( aStringValue );
        }

        if( xNameAccess->getByName(aFtpPortPN) >>= nIntValue )
        {
            m_pFtpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(aNoProxyDescPN) >>= aStringValue )
        {
            m_pNoProxyForED->SetText( aStringValue );
        }
    }

    catch (const container::NoSuchElementException&) {
        OSL_TRACE( "SvxProxyTabPage::ReadConfigData_Impl: NoSuchElementException caught" );
    }

    catch (const com::sun::star::lang::WrappedTargetException &) {
        OSL_TRACE( "SvxProxyTabPage::ReadConfigData_Impl: WrappedTargetException caught" );
    }

    catch (const RuntimeException &) {
        OSL_TRACE( "SvxProxyTabPage::ReadConfigData_Impl: RuntimeException caught" );
    }

}

void SvxProxyTabPage::ReadConfigDefaults_Impl()
{
    try
    {
        Reference< beans::XPropertyState > xPropertyState(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        sal_Int32 nIntValue = 0;
        OUString  aStringValue;

        if( xPropertyState->getPropertyDefault(aHttpProxyPN) >>= aStringValue )
        {
            m_pHttpProxyED->SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aHttpPortPN) >>= nIntValue )
        {
            m_pHttpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aHttpsProxyPN) >>= aStringValue )
        {
            m_pHttpsProxyED->SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aHttpsPortPN) >>= nIntValue )
        {
            m_pHttpsPortED->SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aFtpProxyPN) >>= aStringValue )
        {
            m_pFtpProxyED->SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aFtpPortPN) >>= nIntValue )
        {
            m_pFtpPortED->SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aNoProxyDescPN) >>= aStringValue )
        {
            m_pNoProxyForED->SetText( aStringValue );
        }
    }
    catch (const beans::UnknownPropertyException &)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: UnknownPropertyException caught" );
    }

    catch (const com::sun::star::lang::WrappedTargetException &) {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: WrappedTargetException caught" );
    }

    catch (const RuntimeException &)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: RuntimeException caught" );
    }
}

void SvxProxyTabPage::RestoreConfigDefaults_Impl()
{
    try
    {
        Reference< beans::XPropertyState > xPropertyState(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        xPropertyState->setPropertyToDefault(aProxyModePN);
        xPropertyState->setPropertyToDefault(aHttpProxyPN);
        xPropertyState->setPropertyToDefault(aHttpPortPN);
        xPropertyState->setPropertyToDefault(aHttpsProxyPN);
        xPropertyState->setPropertyToDefault(aHttpsPortPN);
        xPropertyState->setPropertyToDefault(aFtpProxyPN);
        xPropertyState->setPropertyToDefault(aFtpPortPN);
        xPropertyState->setPropertyToDefault(aNoProxyDescPN);

        Reference< util::XChangesBatch > xChangesBatch(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }

    catch (const beans::UnknownPropertyException &)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: UnknownPropertyException caught" );
    }

    catch (const com::sun::star::lang::WrappedTargetException &) {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: WrappedTargetException caught" );
    }

    catch (const RuntimeException &)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: RuntimeException caught" );
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

    EnableControls_Impl( m_pProxyModeLB->GetSelectEntryPos() == 2 );
}

bool SvxProxyTabPage::FillItemSet(SfxItemSet* )
{
    bool bModified = false;

    try {
        Reference< beans::XPropertySet > xPropertySet(m_xConfigurationUpdateAccess, UNO_QUERY_THROW );

        sal_Int32 nSelPos = m_pProxyModeLB->GetSelectEntryPos();
        if(m_pProxyModeLB->IsValueChangedFromSaved())
        {
            if( nSelPos == 1 )
            {
                RestoreConfigDefaults_Impl();
                return true;
            }

            xPropertySet->setPropertyValue(aProxyModePN,
                makeAny((sal_Int32) nSelPos));
            bModified = true;
        }

        if(m_pHttpProxyED->IsValueChangedFromSaved())
        {
            xPropertySet->setPropertyValue( aHttpProxyPN, makeAny(m_pHttpProxyED->GetText()));
            bModified = true;
        }

        if ( m_pHttpPortED->IsValueChangedFromSaved())
        {
            xPropertySet->setPropertyValue( aHttpPortPN, makeAny(m_pHttpPortED->GetText().toInt32()));
            bModified = true;
        }

        if( m_pHttpsProxyED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( aHttpsProxyPN, makeAny(m_pHttpsProxyED->GetText()) );
            bModified = true;
        }

        if ( m_pHttpsPortED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( aHttpsPortPN, makeAny(m_pHttpsPortED->GetText().toInt32()) );
            bModified = true;
        }

        if( m_pFtpProxyED->IsValueChangedFromSaved())
        {
            xPropertySet->setPropertyValue( aFtpProxyPN, makeAny(m_pFtpProxyED->GetText()) );
            bModified = true;
        }

        if ( m_pFtpPortED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( aFtpPortPN, makeAny(m_pFtpPortED->GetText().toInt32()));
            bModified = true;
        }

        if ( m_pNoProxyForED->IsValueChangedFromSaved() )
        {
            xPropertySet->setPropertyValue( aNoProxyDescPN, makeAny( m_pNoProxyForED->GetText()));
            bModified = true;
        }

        Reference< util::XChangesBatch > xChangesBatch(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }

    catch (const com::sun::star::lang::IllegalArgumentException &) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: IllegalArgumentException caught" );
    }

    catch (const beans::UnknownPropertyException &) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: UnknownPropertyException caught" );
    }

    catch (const beans::PropertyVetoException &) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: PropertyVetoException caught" );
    }

    catch (const com::sun::star::lang::WrappedTargetException &) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: WrappedTargetException caught" );
    }

    catch (const RuntimeException &) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: RuntimeException caught" );
    }

    return bModified;
}

void SvxProxyTabPage::EnableControls_Impl(bool bEnable)
{
    m_pHttpProxyFT->Enable(bEnable);
    m_pHttpProxyED->Enable(bEnable);
    m_pHttpPortFT->Enable(bEnable);
    m_pHttpPortED->Enable(bEnable);

    m_pHttpsProxyFT->Enable(bEnable);
    m_pHttpsProxyED->Enable(bEnable);
    m_pHttpsPortFT->Enable(bEnable);
    m_pHttpsPortED->Enable(bEnable);

    m_pFtpProxyFT->Enable(bEnable);
    m_pFtpProxyED->Enable(bEnable);
    m_pFtpPortFT->Enable(bEnable);
    m_pFtpPortED->Enable(bEnable);

    m_pNoProxyForFT->Enable(bEnable);
    m_pNoProxyForED->Enable(bEnable);
    m_pNoProxyDescFT->Enable(bEnable);
}



IMPL_LINK( SvxProxyTabPage, ProxyHdl_Impl, ListBox *, pBox )
{
    sal_Int32 nPos = pBox->GetSelectEntryPos();

    // Restore original system values
    if( nPos == 1 )
    {
        ReadConfigDefaults_Impl();
    }

    EnableControls_Impl(nPos == 2);
    return 0;
}



IMPL_LINK( SvxProxyTabPage, LoseFocusHdl_Impl, Edit *, pEdit )
{
    OUString aValue = pEdit->GetText();

    if ( !comphelper::string::isdigitAsciiString(aValue) || (long)aValue.toInt32() > USHRT_MAX )
        pEdit->SetText( OUString('0') );
    return 0;
}



//#98647#----------------------------------------------
void SvxScriptExecListBox::RequestHelp( const HelpEvent& rHEvt )
{   // try to show tips just like as on toolbars
    sal_Int32 nPos=LISTBOX_ENTRY_NOTFOUND;
    sal_Int32 nTop = GetTopEntry();
    sal_uInt16 nCount = GetDisplayLineCount(); // Attention: Not GetLineCount()
    Point aPt = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
    Rectangle aItemRect;
    if( nCount > 0 ) // if there're some entries, find it.
         for( nPos = nTop ; nPos <= nTop+nCount-1 ; nPos++ ) {
            aItemRect = GetBoundingRectangle(nPos);
            if( aPt.Y() < aItemRect.Top() || aPt.Y() > aItemRect.Bottom() )
                continue;
            else
                break;
        }
     else // if not, nothing happens.
         return;
     OUString aHelpText;
     if( nPos <= nTop+nCount-1 ) // if find the matching entry, get its content.
         aHelpText = GetEntry(nPos);
    if( aHelpText.getLength() && GetTextWidth(aHelpText)<GetOutputSizePixel().Width() )
        aHelpText = ""; // if the entry is quite short, clear the helping tip content.
    aItemRect = Rectangle(Point(0,0),GetSizePixel());
    aPt = Point(OutputToScreenPixel( aItemRect.TopLeft() ));
    aItemRect.Left()   = aPt.X();
    aItemRect.Top()    = aPt.Y();
    aPt = OutputToScreenPixel( aItemRect.BottomRight() );
    aItemRect.Right()  = aPt.X();
    aItemRect.Bottom() = aPt.Y();
    if( rHEvt.GetMode() == HelpEventMode::BALLOON )
        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aHelpText);
    else
        Help::ShowQuickHelp( this, aItemRect, aHelpText );
}

/********************************************************************/
/*                                                                  */
/*  SvxSecurityTabPage                                             */
/*                                                                  */
/********************************************************************/

SvxSecurityTabPage::SvxSecurityTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptSecurityPage", "cui/ui/optsecuritypage.ui", &rSet)
    , mpSecOptions(new SvtSecurityOptions)
    , mpSecOptDlg(NULL)
    , mpCertPathDlg(NULL)
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
    m_sPasswordStoringDeactivateStr = get<FixedText>("nopasswordsave")->GetText();

    InitControls();

    m_pSecurityOptionsPB->SetClickHdl( LINK( this, SvxSecurityTabPage, SecurityOptionsHdl ) );
    m_pSavePasswordsCB->SetClickHdl( LINK( this, SvxSecurityTabPage, SavePasswordHdl ) );
    m_pMasterPasswordPB->SetClickHdl( LINK( this, SvxSecurityTabPage, MasterPasswordHdl ) );
    m_pMasterPasswordCB->SetClickHdl( LINK( this, SvxSecurityTabPage, MasterPasswordCBHdl ) );
    m_pShowConnectionsPB->SetClickHdl( LINK( this, SvxSecurityTabPage, ShowPasswordsHdl ) );
    m_pMacroSecPB->SetClickHdl( LINK( this, SvxSecurityTabPage, MacroSecPBHdl ) );
    m_pCertPathPB->SetClickHdl( LINK( this, SvxSecurityTabPage, CertPathPBHdl ) );

    ActivatePage( rSet );
}

SvxSecurityTabPage::~SvxSecurityTabPage()
{
    delete mpCertPathDlg;

    delete mpSecOptions;
    delete mpSecOptDlg;
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SecurityOptionsHdl)
{
    if ( !mpSecOptDlg )
        mpSecOptDlg = new svx::SecurityOptionsDialog( this, mpSecOptions );
    mpSecOptDlg->Execute();
    return 0;
}

IMPL_LINK_NOARG(SvxSecurityTabPage, SavePasswordHdl)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( m_pSavePasswordsCB->IsChecked() )
        {
            bool bOldValue = xMasterPasswd->allowPersistentStoring( sal_True );
            xMasterPasswd->removeMasterPassword();
            if ( xMasterPasswd->changeMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                m_pMasterPasswordPB->Enable( true );
                m_pMasterPasswordCB->Check( true );
                m_pMasterPasswordCB->Enable( true );
                m_pMasterPasswordFT->Enable( true );
                m_pShowConnectionsPB->Enable( true );
            }
            else
            {
                xMasterPasswd->allowPersistentStoring( bOldValue );
                m_pSavePasswordsCB->Check( false );
            }
        }
        else
        {
            QueryBox aQuery( this, WB_YES_NO|WB_DEF_NO, m_sPasswordStoringDeactivateStr );
            sal_uInt16 nRet = aQuery.Execute();

            if( RET_YES == nRet )
            {
                xMasterPasswd->allowPersistentStoring( sal_False );
                m_pMasterPasswordCB->Check( true );
                m_pMasterPasswordPB->Enable( false );
                m_pMasterPasswordCB->Enable( false );
                m_pMasterPasswordFT->Enable( false );
                m_pShowConnectionsPB->Enable( false );
            }
            else
            {
                m_pSavePasswordsCB->Check( true );
                m_pMasterPasswordPB->Enable( true );
                m_pShowConnectionsPB->Enable( true );
            }
        }
    }
    catch (const Exception&)
    {
        m_pSavePasswordsCB->Check( !m_pSavePasswordsCB->IsChecked() );
    }

    return 0;
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MasterPasswordHdl)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
            xMasterPasswd->changeMasterPassword( Reference< task::XInteractionHandler >() );
    }
    catch (const Exception&)
    {}

    return 0;
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MasterPasswordCBHdl)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( m_pMasterPasswordCB->IsChecked() )
        {
            if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->changeMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                m_pMasterPasswordPB->Enable( true );
                m_pMasterPasswordFT->Enable( true );
            }
            else
            {
                m_pMasterPasswordCB->Check( false );
                m_pMasterPasswordPB->Enable( true );
                m_pMasterPasswordFT->Enable( true );
            }
        }
        else
        {
            if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->useDefaultMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                m_pMasterPasswordPB->Enable( false );
                m_pMasterPasswordFT->Enable( false );
            }
            else
            {
                m_pMasterPasswordCB->Check( true );
                m_pMasterPasswordPB->Enable( true );
                m_pShowConnectionsPB->Enable( true );
            }
        }
    }
    catch (const Exception&)
    {
        m_pSavePasswordsCB->Check( !m_pSavePasswordsCB->IsChecked() );
    }

    return 0;
}

IMPL_LINK_NOARG(SvxSecurityTabPage, ShowPasswordsHdl)
{
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->authorizateWithMasterPassword( Reference< task::XInteractionHandler>() ) )
        {
            svx::WebConnectionInfoDialog aDlg( this );
            aDlg.Execute();
        }
    }
    catch (const Exception&)
    {}
    return 0;
}

IMPL_LINK_NOARG(SvxSecurityTabPage, CertPathPBHdl)
{
    if (!mpCertPathDlg)
        mpCertPathDlg = new CertPathDialog(this);

    OUString sOrig = mpCertPathDlg->getDirectory();
    short nRet = mpCertPathDlg->Execute();

    if (nRet == RET_OK && sOrig != mpCertPathDlg->getDirectory())
    {
        MessageDialog aWarnBox(this, CUI_RES(RID_SVXSTR_OPTIONS_RESTART), VCL_MESSAGE_INFO);
        aWarnBox.Execute();
    }

    return 0;
}

IMPL_LINK_NOARG(SvxSecurityTabPage, MacroSecPBHdl)
{
    try
    {
        Reference< security::XDocumentDigitalSignatures > xD(
            security::DocumentDigitalSignatures::createDefault(comphelper::getProcessComponentContext() ) );
        xD->manageTrustedSources();
    }
    catch (const Exception& e)
    {
        OSL_FAIL(OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
        (void)e;
    }
    return 0;
}


void SvxSecurityTabPage::InitControls()
{
    // Hide all controls which belong to the macro security button in case the macro
    // security settings managed by the macro security dialog opened via the button
    // are all readonly or if the macros are disabled in general.
    // @@@ Better would be to query the dialog whether it is 'useful' or not. Exposing
    //     macro security dialog implementations here, which is bad.
    if (    mpSecOptions->IsMacroDisabled()
         || (    mpSecOptions->IsReadOnly( SvtSecurityOptions::E_MACRO_SECLEVEL )
              && mpSecOptions->IsReadOnly( SvtSecurityOptions::E_MACRO_TRUSTEDAUTHORS )
              && mpSecOptions->IsReadOnly( SvtSecurityOptions::E_SECUREURLS ) ) )
    {
        //Hide these
        m_pMacroSecFrame->Hide();
    }

#ifndef UNX
    m_pCertFrame->Hide();
#endif

    m_pMasterPasswordPB->Enable( false );
    m_pMasterPasswordCB->Enable( false );
    m_pMasterPasswordCB->Check( true );
    m_pMasterPasswordFT->Enable( false );
    m_pShowConnectionsPB->Enable( false );

    // initialize the password saving checkbox
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            m_pMasterPasswordCB->Enable( true );
            m_pShowConnectionsPB->Enable( true );
            m_pSavePasswordsCB->Check( true );

            if ( xMasterPasswd->isDefaultMasterPasswordUsed() )
                m_pMasterPasswordCB->Check( false );
            else
            {
                m_pMasterPasswordPB->Enable( true );
                m_pMasterPasswordCB->Check( true );
                m_pMasterPasswordFT->Enable( true );
            }
        }
    }
    catch (const Exception&)
    {
        m_pSavePasswordsCB->Enable( false );
    }
}

SfxTabPage* SvxSecurityTabPage::Create(vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return new SvxSecurityTabPage(pParent, *rAttrSet);
}

void SvxSecurityTabPage::ActivatePage( const SfxItemSet& )
{
}

int SvxSecurityTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );
    return LEAVE_PAGE;
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

    if ( mpSecOptDlg )
    {
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SAVEORSEND, mpSecOptDlg->IsSaveOrSendDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SIGNING, mpSecOptDlg->IsSignDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_PRINT, mpSecOptDlg->IsPrintDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_CREATEPDF, mpSecOptDlg->IsCreatePdfChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO, mpSecOptDlg->IsRemovePersInfoChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD, mpSecOptDlg->IsRecommPasswdChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_CTRLCLICK_HYPERLINK, mpSecOptDlg->IsCtrlHyperlinkChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_BLOCKUNTRUSTEDREFERERLINKS, mpSecOptDlg->IsBlockUntrustedRefererLinksChecked(), bModified );
    }

    return bModified;
}

/*--------------------------------------------------------------------*/

void SvxSecurityTabPage::Reset( const SfxItemSet* )
{
}

class MailerProgramCfg_Impl : public utl::ConfigItem
{
    friend class SvxEMailTabPage;
    // variables
    OUString sProgram;
    // readonly states
    bool bROProgram;

    const Sequence<OUString> GetPropertyNames();
public:
    MailerProgramCfg_Impl();
    virtual ~MailerProgramCfg_Impl();

    virtual void    Commit() SAL_OVERRIDE;
    virtual void Notify( const com::sun::star::uno::Sequence< OUString >& _rPropertyNames) SAL_OVERRIDE;
};

/* -------------------------------------------------------------------------*/

MailerProgramCfg_Impl::MailerProgramCfg_Impl() :
    utl::ConfigItem("Office.Common/ExternalMailer"),
    bROProgram(false)
{
    const Sequence< OUString > aNames = GetPropertyNames();
    const Sequence< Any > aValues = GetProperties(aNames);
    const Sequence< sal_Bool > aROStates = GetReadOnlyStates(aNames);
    const Any* pValues = aValues.getConstArray();
    const sal_Bool* pROStates = aROStates.getConstArray();
    for(sal_Int32 nProp = 0; nProp < aValues.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case 0 :
                {
                    pValues[nProp] >>= sProgram;
                    bROProgram = pROStates[nProp];
                }
                break;
            }
        }
    }
}

/* -------------------------------------------------------------------------*/

MailerProgramCfg_Impl::~MailerProgramCfg_Impl()
{
}

/* -------------------------------------------------------------------------*/

const Sequence<OUString> MailerProgramCfg_Impl::GetPropertyNames()
{
    Sequence<OUString> aRet(1);
    OUString* pRet = aRet.getArray();
    pRet[0] = "Program";
    return aRet;
}

/* -------------------------------------------------------------------------*/

void MailerProgramCfg_Impl::Commit()
{
    const Sequence< OUString > aOrgNames = GetPropertyNames();
    sal_Int32 nOrgCount = aOrgNames.getLength();

    Sequence< OUString > aNames(nOrgCount);
    Sequence< Any > aValues(nOrgCount);
    sal_Int32 nRealCount = 0;

    for(int nProp = 0; nProp < nOrgCount; nProp++)
    {
    switch(nProp)
    {
            case  0:
            {
                if (!bROProgram)
                {
                    aNames[nRealCount] = aOrgNames[nProp];
                    aValues[nRealCount] <<= sProgram;
                    ++nRealCount;
                }
            }
            break;
        }
    }

    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties(aNames, aValues);
}

void MailerProgramCfg_Impl::Notify( const com::sun::star::uno::Sequence< OUString >& )
{
}

/* -------------------------------------------------------------------------*/

struct SvxEMailTabPage_Impl
{
    MailerProgramCfg_Impl aMailConfig;
};

SvxEMailTabPage::SvxEMailTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, "OptEmailPage", "cui/ui/optemailpage.ui", &rSet)
    , pImpl(new SvxEMailTabPage_Impl)
{
    get(m_pMailContainer, "OptEmailPage");
    get(m_pMailerURLFI, "lockemail");
    get(m_pMailerURLED, "url");
    get(m_pMailerURLPB, "browse");
    m_sDefaultFilterName = get<FixedText>("browsetitle")->GetText();
    m_pMailerURLPB->SetClickHdl( LINK( this, SvxEMailTabPage, FileDialogHdl_Impl ) );
}

/* -------------------------------------------------------------------------*/

SvxEMailTabPage::~SvxEMailTabPage()
{
    delete pImpl;
}

/* -------------------------------------------------------------------------*/

SfxTabPage*  SvxEMailTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return new SvxEMailTabPage(pParent, *rAttrSet);
}

/* -------------------------------------------------------------------------*/

bool SvxEMailTabPage::FillItemSet( SfxItemSet* )
{
    bool bMailModified = false;
    if(!pImpl->aMailConfig.bROProgram && m_pMailerURLED->IsValueChangedFromSaved())
    {
        pImpl->aMailConfig.sProgram = m_pMailerURLED->GetText();
        bMailModified = true;
    }
    if ( bMailModified )
        pImpl->aMailConfig.Commit();

    return false;
}

/* -------------------------------------------------------------------------*/

void SvxEMailTabPage::Reset( const SfxItemSet* )
{
    m_pMailerURLED->Enable(true );
    m_pMailerURLPB->Enable(true );

    if (pImpl->aMailConfig.bROProgram)
        m_pMailerURLFI->Show();

    m_pMailerURLED->SetText(pImpl->aMailConfig.sProgram);
    m_pMailerURLED->SaveValue();

    m_pMailContainer->Enable(!pImpl->aMailConfig.bROProgram);
}

/* -------------------------------------------------------------------------*/

IMPL_LINK(  SvxEMailTabPage, FileDialogHdl_Impl, PushButton*, pButton )
{
    if (m_pMailerURLPB == pButton && !pImpl->aMailConfig.bROProgram)
    {
        FileDialogHelper aHelper(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );
        OUString sPath = m_pMailerURLED->GetText();
        if ( sPath.isEmpty() )
            sPath = "/usr/bin";

        OUString sUrl;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL(sPath, sUrl);
        aHelper.SetDisplayDirectory(sUrl);
        aHelper.AddFilter( m_sDefaultFilterName, OUString("*"));

        if ( ERRCODE_NONE == aHelper.Execute() )
        {
            sUrl = aHelper.GetPath();
            ::utl::LocalFileHelper::ConvertURLToPhysicalName(sUrl, sPath);
            m_pMailerURLED->SetText(sPath);
        }
    }
    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

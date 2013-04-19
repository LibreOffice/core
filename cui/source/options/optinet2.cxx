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

#include <tools/shl.hxx>
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
#include <svl/cntwids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/help.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#define _SVX_OPTINET2_CXX
#include <dialmgr.hxx>
#include "optinet2.hxx"
#include <svx/svxdlg.hxx>
#include <cuires.hrc>
#include "optinet2.hrc"
#include "helpid.hrc"
#include <svx/ofaitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/svxids.hrc> // slot ids, mostly for changetracking

// for security TP
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

// -----------------------------------------------------------------------

void SvxNoSpaceEdit::KeyInput( const KeyEvent& rKEvent )
{
    if ( bOnlyNumeric )
    {
        const KeyCode& rKeyCode = rKEvent.GetKeyCode();
        sal_uInt16 nGroup = rKeyCode.GetGroup();
        sal_uInt16 nKey = rKeyCode.GetCode();
        sal_Bool bValid = ( KEYGROUP_NUM == nGroup || KEYGROUP_CURSOR == nGroup ||
                        ( KEYGROUP_MISC == nGroup && ( nKey < KEY_ADD || nKey > KEY_EQUAL ) ) );
        if ( !bValid && ( rKeyCode.IsMod1() && (
             KEY_A == nKey || KEY_C == nKey || KEY_V == nKey || KEY_X == nKey || KEY_Z == nKey ) ) )
            // Erase, Copy, Paste, Select All und Undo soll funktionieren
            bValid = sal_True;

        if ( bValid )
            Edit::KeyInput(rKEvent);
    }
    else if( rKEvent.GetKeyCode().GetCode() != KEY_SPACE )
        Edit::KeyInput(rKEvent);
}

// -----------------------------------------------------------------------

void SvxNoSpaceEdit::Modify()
{
    Edit::Modify();

    if ( bOnlyNumeric )
    {
        OUString aValue = GetText();

        if ( !comphelper::string::isdigitAsciiString(aValue) || (long)aValue.toInt32() > USHRT_MAX )
            // the maximum value of a port number is USHRT_MAX
            ErrorBox( this, CUI_RES( RID_SVXERR_OPT_PROXYPORTS ) ).Execute();
    }
}

/********************************************************************/
/*                                                                  */
/*  SvxProxyTabPage                                                 */
/*                                                                  */
/********************************************************************/

SvxProxyTabPage::SvxProxyTabPage(Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_INET_PROXY ), rSet ),
    aOptionGB   (this, CUI_RES(GB_SETTINGS)),

    aProxyModeFT  (this, CUI_RES(FT_PROXYMODE)),
    aProxyModeLB  (this, CUI_RES(LB_PROXYMODE)),

    aHttpProxyFT      (this, CUI_RES( FT_HTTP_PROXY   )),
    aHttpProxyED      (this, CUI_RES( ED_HTTP_PROXY     )),
    aHttpPortFT       (this, CUI_RES( FT_HTTP_PORT      )),
    aHttpPortED       (this, CUI_RES( ED_HTTP_PORT      ), sal_True),

    aHttpsProxyFT      (this, CUI_RES( FT_HTTPS_PROXY     )),
    aHttpsProxyED      (this, CUI_RES( ED_HTTPS_PROXY     )),
    aHttpsPortFT       (this, CUI_RES( FT_HTTPS_PORT      )),
    aHttpsPortED       (this, CUI_RES( ED_HTTPS_PORT      ), sal_True),

    aFtpProxyFT       (this, CUI_RES( FT_FTP_PROXY      )),
    aFtpProxyED       (this, CUI_RES( ED_FTP_PROXY      )),
    aFtpPortFT        (this, CUI_RES( FT_FTP_PORT       )),
    aFtpPortED        (this, CUI_RES( ED_FTP_PORT       ), sal_True),

    aNoProxyForFT     (this, CUI_RES( FT_NOPROXYFOR     )),
    aNoProxyForED     (this, CUI_RES( ED_NOPROXYFOR     )),
    aNoProxyDescFT    (this, CUI_RES( ED_NOPROXYDESC    )),
    sFromBrowser        (       CUI_RES( ST_PROXY_FROM_BROWSER ) ),
    aProxyModePN("ooInetProxyType"),
    aHttpProxyPN("ooInetHTTPProxyName"),
    aHttpPortPN("ooInetHTTPProxyPort"),
    aHttpsProxyPN("ooInetHTTPSProxyName"),
    aHttpsPortPN("ooInetHTTPSProxyPort"),
    aFtpProxyPN("ooInetFTPProxyName"),
    aFtpPortPN("ooInetFTPProxyPort"),
    aNoProxyDescPN("ooInetNoProxy")
{
    FreeResource();

    aHttpPortED.SetMaxTextLen(5);
    aHttpsPortED.SetMaxTextLen(5);
    aFtpPortED.SetMaxTextLen(5);
    Link aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    aHttpPortED.SetLoseFocusHdl( aLink );
    aHttpsPortED.SetLoseFocusHdl( aLink );
    aFtpPortED.SetLoseFocusHdl( aLink );

    aProxyModeLB.SetSelectHdl(LINK( this, SvxProxyTabPage, ProxyHdl_Impl ));

    Reference< com::sun::star::lang::XMultiServiceFactory >
        xConfigurationProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

    OUString aConfigRoot( "org.openoffice.Inet/Settings" );

    beans::NamedValue aProperty;
    aProperty.Name  = OUString( "nodepath" );
    aProperty.Value = makeAny( aConfigRoot );

    Sequence< Any > aArgumentList( 1 );
    aArgumentList[0] = makeAny( aProperty );

    m_xConfigurationUpdateAccess = xConfigurationProvider->createInstanceWithArguments(
        OUString( "com.sun.star.configuration.ConfigurationUpdateAccess" ),
        aArgumentList );

    ArrangeControls_Impl();
}

SvxProxyTabPage::~SvxProxyTabPage()
{
}

SfxTabPage* SvxProxyTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxProxyTabPage(pParent, rAttrSet);
}

void SvxProxyTabPage::ReadConfigData_Impl()
{
    try {
        Reference< container::XNameAccess > xNameAccess(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        sal_Int32 nIntValue = 0;
        OUString  aStringValue;

        if( xNameAccess->getByName(aProxyModePN) >>= nIntValue )
        {
            aProxyModeLB.SelectEntryPos( (sal_uInt16) nIntValue );
        }

        if( xNameAccess->getByName(aHttpProxyPN) >>= aStringValue )
        {
            aHttpProxyED.SetText( aStringValue );
        }

        if( xNameAccess->getByName(aHttpPortPN) >>= nIntValue )
        {
            aHttpPortED.SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(aHttpsProxyPN) >>= aStringValue )
        {
            aHttpsProxyED.SetText( aStringValue );
        }

        if( xNameAccess->getByName(aHttpsPortPN) >>= nIntValue )
        {
            aHttpsPortED.SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(aFtpProxyPN) >>= aStringValue )
        {
            aFtpProxyED.SetText( aStringValue );
        }

        if( xNameAccess->getByName(aFtpPortPN) >>= nIntValue )
        {
            aFtpPortED.SetText( OUString::number( nIntValue ));
        }

        if( xNameAccess->getByName(aNoProxyDescPN) >>= aStringValue )
        {
            aNoProxyForED.SetText( aStringValue );
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
            aHttpProxyED.SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aHttpPortPN) >>= nIntValue )
        {
            aHttpPortED.SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aHttpsProxyPN) >>= aStringValue )
        {
            aHttpsProxyED.SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aHttpsPortPN) >>= nIntValue )
        {
            aHttpsPortED.SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aFtpProxyPN) >>= aStringValue )
        {
            aFtpProxyED.SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aFtpPortPN) >>= nIntValue )
        {
            aFtpPortED.SetText( OUString::number( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aNoProxyDescPN) >>= aStringValue )
        {
            aNoProxyForED.SetText( aStringValue );
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

void SvxProxyTabPage::Reset(const SfxItemSet&)
{
    ReadConfigData_Impl();

    aProxyModeLB.SaveValue();
    aHttpProxyED.SaveValue();
    aHttpPortED.SaveValue();
    aHttpsProxyED.SaveValue();
    aHttpsPortED.SaveValue();
    aFtpProxyED.SaveValue();
    aFtpPortED.SaveValue();
    aNoProxyForED.SaveValue();

    EnableControls_Impl( aProxyModeLB.GetSelectEntryPos() == 2 );
}

sal_Bool SvxProxyTabPage::FillItemSet(SfxItemSet& )
{
    sal_Bool bModified=sal_False;

    try {
        Reference< beans::XPropertySet > xPropertySet(m_xConfigurationUpdateAccess, UNO_QUERY_THROW );

        sal_uInt16 nSelPos = aProxyModeLB.GetSelectEntryPos();
        if(aProxyModeLB.GetSavedValue() != nSelPos)
        {
            if( nSelPos == 1 )
            {
                RestoreConfigDefaults_Impl();
                return sal_True;
            }

            xPropertySet->setPropertyValue(aProxyModePN,
                makeAny((sal_Int32) nSelPos));
            bModified = sal_True;
        }

        if(aHttpProxyED.GetSavedValue() != aHttpProxyED.GetText())
        {
            xPropertySet->setPropertyValue( aHttpProxyPN, makeAny(aHttpProxyED.GetText()));
            bModified = sal_True;
        }

        if ( aHttpPortED.GetSavedValue() != aHttpPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aHttpPortPN, makeAny(aHttpPortED.GetText().toInt32()));
            bModified = sal_True;
        }

        if( aHttpsProxyED.GetSavedValue() != aHttpsProxyED.GetText() )
        {
            xPropertySet->setPropertyValue( aHttpsProxyPN, makeAny(aHttpsProxyED.GetText()) );
            bModified = sal_True;
        }

        if ( aHttpsPortED.GetSavedValue() != aHttpsPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aHttpsPortPN, makeAny(aHttpsPortED.GetText().toInt32()) );
            bModified = sal_True;
        }

        if( aFtpProxyED.GetSavedValue() != aFtpProxyED.GetText())
        {
            xPropertySet->setPropertyValue( aFtpProxyPN, makeAny(aFtpProxyED.GetText()) );
            bModified = sal_True;
        }

        if ( aFtpPortED.GetSavedValue() != aFtpPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aFtpPortPN, makeAny(aFtpPortED.GetText().toInt32()));
            bModified = sal_True;
        }

        if ( aNoProxyForED.GetSavedValue() != aNoProxyForED.GetText() )
        {
            xPropertySet->setPropertyValue( aNoProxyDescPN, makeAny( aNoProxyForED.GetText()));
            bModified = sal_True;
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

void SvxProxyTabPage::ArrangeControls_Impl()
{
    // calculate dynamic width of controls, to not cut-off translated strings #i71445#
    long nWidth = aProxyModeFT.GetCtrlTextWidth( aProxyModeFT.GetText() );
    long nTemp = aHttpProxyFT.GetCtrlTextWidth( aHttpProxyFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;
    nTemp = aHttpsProxyFT.GetCtrlTextWidth( aHttpsProxyFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;
    nTemp = aFtpProxyFT.GetCtrlTextWidth( aFtpProxyFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;
    nTemp = aNoProxyForFT.GetCtrlTextWidth( aNoProxyForFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;

    nWidth += 10; // To be sure the length of the FixedText is enough on all platforms
    const long nFTWidth = aProxyModeFT.GetSizePixel().Width();
    if ( nWidth > nFTWidth )
    {
        Size aNewSize = aProxyModeFT.GetSizePixel();
        aNewSize.Width() = nWidth;

        aProxyModeFT.SetSizePixel( aNewSize );
        aHttpProxyFT.SetSizePixel( aNewSize );
        aHttpsProxyFT.SetSizePixel( aNewSize );
        aFtpProxyFT.SetSizePixel( aNewSize );
        aNoProxyForFT.SetSizePixel( aNewSize );

        const long nDelta = nWidth - nFTWidth;
        Point aNewPos = aProxyModeLB.GetPosPixel();
        aNewPos.X() += nDelta;

        aProxyModeLB.SetPosPixel( aNewPos );

        aNewSize = aHttpProxyED.GetSizePixel();
        aNewSize.Width() -= nDelta;

        aNewPos.Y() = aHttpProxyED.GetPosPixel().Y();
        aHttpProxyED.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = aHttpsProxyED.GetPosPixel().Y();
        aHttpsProxyED.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = aFtpProxyED.GetPosPixel().Y();
        aFtpProxyED.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = aNoProxyForED.GetPosPixel().Y();
        aNoProxyForED.SetPosSizePixel( aNewPos, aNewSize );
    }
}

void SvxProxyTabPage::EnableControls_Impl(sal_Bool bEnable)
{
    aHttpProxyFT.Enable(bEnable);
    aHttpProxyED.Enable(bEnable);
    aHttpPortFT.Enable(bEnable);
    aHttpPortED.Enable(bEnable);

    aHttpsProxyFT.Enable(bEnable);
    aHttpsProxyED.Enable(bEnable);
    aHttpsPortFT.Enable(bEnable);
    aHttpsPortED.Enable(bEnable);

    aFtpProxyFT.Enable(bEnable);
    aFtpProxyED.Enable(bEnable);
    aFtpPortFT.Enable(bEnable);
    aFtpPortED.Enable(bEnable);

    aNoProxyForFT.Enable(bEnable);
    aNoProxyForED.Enable(bEnable);
    aNoProxyDescFT.Enable(bEnable);
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxProxyTabPage, ProxyHdl_Impl, ListBox *, pBox )
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();

    // Restore original system values
    if( nPos == 1 )
    {
        ReadConfigDefaults_Impl();
    }

    EnableControls_Impl(nPos == 2);
    return 0;
}

// -----------------------------------------------------------------------

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
    sal_uInt16 nPos=LISTBOX_ENTRY_NOTFOUND;
    sal_uInt16 nTop = GetTopEntry();
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
        aHelpText = OUString(); // if the entry is quite short, clear the helping tip content.
    aItemRect = Rectangle(Point(0,0),GetSizePixel());
    aPt = Point(OutputToScreenPixel( aItemRect.TopLeft() ));
    aItemRect.Left()   = aPt.X();
    aItemRect.Top()    = aPt.Y();
    aPt = OutputToScreenPixel( aItemRect.BottomRight() );
    aItemRect.Right()  = aPt.X();
    aItemRect.Bottom() = aPt.Y();
    if( rHEvt.GetMode() == HELPMODE_BALLOON )
        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aHelpText);
    else
        Help::ShowQuickHelp( this, aItemRect, aHelpText );
}

/********************************************************************/
/*                                                                  */
/*  SvxSecurityTabPage                                             */
/*                                                                  */
/********************************************************************/

SvxSecurityTabPage::SvxSecurityTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptSecurityPage", "cui/ui/optsecuritypage.ui", rSet)
    , mpSecOptions(new SvtSecurityOptions)
    , mpSecOptDlg(NULL)
    , mpCertPathDlg(NULL)
{
    get(m_pSecurityOptionsPB, "options");
    get(m_pSavePasswordsCB, "savepassword");
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
            sal_Bool bOldValue = xMasterPasswd->allowPersistentStoring( sal_True );
            xMasterPasswd->removeMasterPassword();
            if ( xMasterPasswd->changeMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                m_pMasterPasswordPB->Enable( sal_True );
                m_pMasterPasswordCB->Check( sal_True );
                m_pMasterPasswordCB->Enable( sal_True );
                m_pMasterPasswordFT->Enable( sal_True );
                m_pShowConnectionsPB->Enable( sal_True );
            }
            else
            {
                xMasterPasswd->allowPersistentStoring( bOldValue );
                m_pSavePasswordsCB->Check( sal_False );
            }
        }
        else
        {
            QueryBox aQuery( this, WB_YES_NO|WB_DEF_NO, m_sPasswordStoringDeactivateStr );
            sal_uInt16 nRet = aQuery.Execute();

            if( RET_YES == nRet )
            {
                xMasterPasswd->allowPersistentStoring( sal_False );
                m_pMasterPasswordCB->Check( sal_True );
                m_pMasterPasswordPB->Enable( sal_False );
                m_pMasterPasswordCB->Enable( sal_False );
                m_pMasterPasswordFT->Enable( sal_False );
                m_pShowConnectionsPB->Enable( sal_False );
            }
            else
            {
                m_pSavePasswordsCB->Check( sal_True );
                m_pMasterPasswordPB->Enable( sal_True );
                m_pShowConnectionsPB->Enable( sal_True );
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
                m_pMasterPasswordPB->Enable( sal_True );
                m_pMasterPasswordFT->Enable( sal_True );
            }
            else
            {
                m_pMasterPasswordCB->Check( sal_False );
                m_pMasterPasswordPB->Enable( sal_True );
                m_pMasterPasswordFT->Enable( sal_True );
            }
        }
        else
        {
            if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->useDefaultMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                m_pMasterPasswordPB->Enable( sal_False );
                m_pMasterPasswordFT->Enable( sal_False );
            }
            else
            {
                m_pMasterPasswordCB->Check( sal_True );
                m_pMasterPasswordPB->Enable( sal_True );
                m_pShowConnectionsPB->Enable( sal_True );
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
        WarningBox aWarnBox(this, CUI_RES(RID_SVX_MSGBOX_OPTIONS_RESTART));
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

    m_pMasterPasswordPB->Enable( sal_False );
    m_pMasterPasswordCB->Enable( sal_False );
    m_pMasterPasswordCB->Check( sal_True );
    m_pMasterPasswordFT->Enable( sal_False );
    m_pShowConnectionsPB->Enable( sal_False );

    // initialize the password saving checkbox
    try
    {
        Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            m_pMasterPasswordCB->Enable( sal_True );
            m_pShowConnectionsPB->Enable( sal_True );
            m_pSavePasswordsCB->Check( sal_True );

            if ( xMasterPasswd->isDefaultMasterPasswordUsed() )
                m_pMasterPasswordCB->Check( sal_False );
            else
            {
                m_pMasterPasswordPB->Enable( sal_True );
                m_pMasterPasswordCB->Check( sal_True );
                m_pMasterPasswordFT->Enable( sal_True );
            }
        }
    }
    catch (const Exception&)
    {
        m_pSavePasswordsCB->Enable( sal_False );
    }
}

SfxTabPage* SvxSecurityTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxSecurityTabPage(pParent, rAttrSet);
}

void SvxSecurityTabPage::ActivatePage( const SfxItemSet& )
{
}

int SvxSecurityTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( *_pSet );
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

sal_Bool SvxSecurityTabPage::FillItemSet( SfxItemSet& )
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
    }

    return bModified;
}

/*--------------------------------------------------------------------*/

void SvxSecurityTabPage::Reset( const SfxItemSet& )
{
}

MozPluginTabPage::MozPluginTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptBrowserPage", "cui/ui/optbrowserpage.ui", rSet)
{
    get(m_pWBasicCodeCB, "display");
}

MozPluginTabPage::~MozPluginTabPage()
{
}

SfxTabPage* MozPluginTabPage::Create( Window* pParent,
                                        const SfxItemSet& rAttrSet )
{
    return new MozPluginTabPage( pParent, rAttrSet );
}
sal_Bool MozPluginTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool hasInstall = isInstalled();
    sal_Bool hasChecked = m_pWBasicCodeCB->IsChecked();
    if(hasInstall && (!hasChecked)){
        //try to uninstall
        uninstallPlugin();
    }
    else if((!hasInstall) && hasChecked){
        //try to install
        installPlugin();
    }
    else{
        // do nothing
    }
    return sal_True;
}
void MozPluginTabPage::Reset( const SfxItemSet& )
{
        m_pWBasicCodeCB->Check( isInstalled());
        m_pWBasicCodeCB->SaveValue();
}

#ifdef WNT
extern "C" {
    int lc_isInstalled(const  char* realFilePath);
    int lc_installPlugin(const  char* realFilePath);
    int lc_uninstallPlugin(const  char* realFilePath);
}
#endif

#define NPP_PATH_MAX 2048
inline bool getDllURL(OString * path)
{
    OSL_ASSERT(path != NULL);
    OUString dirPath/*dllPath, */;
    if (osl_getExecutableFile(&dirPath.pData) != osl_Process_E_None) {
        return false;
    }
    dirPath = dirPath.copy(0, dirPath.lastIndexOf('/'));
    OUString sysDirPath;
    osl::FileBase::getSystemPathFromFileURL(dirPath, sysDirPath);
    *path = OUStringToOString(sysDirPath, RTL_TEXTENCODING_ASCII_US);
    return true;
}

sal_Bool MozPluginTabPage::isInstalled()
{
#ifdef UNIX
    // get the real file referred by .so lnk file
    char lnkFilePath[NPP_PATH_MAX] = {0};
    char lnkReferFilePath[NPP_PATH_MAX] = {0};
    char* pHome = getpwuid(getuid())->pw_dir;
    strcat(lnkFilePath, pHome);
    strcat(lnkFilePath, "/.mozilla/plugins/libnpsoplugin" SAL_DLLEXTENSION);

    struct stat sBuf;
    if (0 > lstat(lnkFilePath, &sBuf))
        return false;
    if (!S_ISLNK(sBuf.st_mode))
        return false;
    if (0 >= readlink(lnkFilePath, lnkReferFilePath, NPP_PATH_MAX))
        return false;
    // If the link is relative, then we regard it as non-standard
    if (lnkReferFilePath[0] != '/')
        return false;

    // get the real file path
    char realFilePath[NPP_PATH_MAX] = {0};
    OString tempString;
    if (!getDllURL(&tempString)) {
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX - 1);
    strcat(realFilePath, "/libnpsoplugin" SAL_DLLEXTENSION);

    if (0 != strcmp(lnkReferFilePath, realFilePath))
        return false;
    return true;
#endif
#ifdef WNT
    // get the value from registry
        sal_Bool ret = true;
    OString tempString;
    char realFilePath[NPP_PATH_MAX] = {0};
    if (!getDllURL(&tempString)){
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if(! lc_isInstalled(realFilePath))
        ret =true;
    else
        ret = false;
    return ret;
#endif
}

sal_Bool MozPluginTabPage::installPlugin()
{
#ifdef UNIX
    // get the real file referred by .so lnk file
    char lnkFilePath[NPP_PATH_MAX] = {0};
    char* pHome = getpwuid(getuid())->pw_dir;
    strcat(lnkFilePath, pHome);
    strcat(lnkFilePath, "/.mozilla/plugins/libnpsoplugin" SAL_DLLEXTENSION);
    remove(lnkFilePath);

    // create the dirs if necessary
    struct stat buf;
    char tmpDir[NPP_PATH_MAX] = {0};
    sprintf(tmpDir, "%s/.mozilla", pHome);
    if (0 > stat(lnkFilePath, &buf))
    {
        mkdir(tmpDir, 0755);
        strcat(tmpDir, "/plugins");
        mkdir(tmpDir, 0755);
    }

    // get the real file path
    char realFilePath[NPP_PATH_MAX] = {0};
    OString tempString;
    if (!getDllURL(&tempString)) {
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX - 1);
    strcat(realFilePath, "/libnpsoplugin" SAL_DLLEXTENSION);

    // create the link
    if (0 != symlink(realFilePath, lnkFilePath))
        return false;
    return true;
#endif
#ifdef WNT
    OString tempString;
    char realFilePath[NPP_PATH_MAX] = {0};
    if (!getDllURL(&tempString)) {
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if( !lc_installPlugin(realFilePath))
        return true;
    else
        return false;
#endif
}

sal_Bool MozPluginTabPage::uninstallPlugin()
{
#ifdef UNIX
    // get the real file referred by .so lnk file
    char lnkFilePath[NPP_PATH_MAX] = {0};
    char* pHome = getpwuid(getuid())->pw_dir;
    strcat(lnkFilePath, pHome);
    strcat(lnkFilePath, "/.mozilla/plugins/libnpsoplugin" SAL_DLLEXTENSION);

    if(0 > remove(lnkFilePath))
        return false;
    return true;
#endif
#ifdef WNT
    OString tempString;
    char realFilePath[NPP_PATH_MAX] = {0};
    if (!getDllURL(&tempString)) {
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if(!lc_uninstallPlugin(realFilePath))
        return true;
    else
        return false;
#endif
}

/* -------------------------------------------------------------------------*/

class MailerProgramCfg_Impl : public utl::ConfigItem
{
    friend class SvxEMailTabPage;
    // variables
    OUString sProgram;
    // readonly states
    sal_Bool bROProgram;

    const Sequence<OUString> GetPropertyNames();
public:
    MailerProgramCfg_Impl();
    virtual ~MailerProgramCfg_Impl();

    virtual void    Commit();
    virtual void Notify( const com::sun::star::uno::Sequence< OUString >& _rPropertyNames);
};

/* -------------------------------------------------------------------------*/

MailerProgramCfg_Impl::MailerProgramCfg_Impl() :
    utl::ConfigItem("Office.Common/ExternalMailer"),
    bROProgram(sal_False)
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

SvxEMailTabPage::SvxEMailTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, "OptEmailPage", "cui/ui/optemailpage.ui", rSet)
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

SfxTabPage*  SvxEMailTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxEMailTabPage(pParent, rAttrSet);
}

/* -------------------------------------------------------------------------*/

sal_Bool SvxEMailTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bMailModified = sal_False;
    if(!pImpl->aMailConfig.bROProgram && m_pMailerURLED->GetSavedValue() != m_pMailerURLED->GetText())
    {
        pImpl->aMailConfig.sProgram = m_pMailerURLED->GetText();
        bMailModified = sal_True;
    }
    if ( bMailModified )
        pImpl->aMailConfig.Commit();

    return sal_False;
}

/* -------------------------------------------------------------------------*/

void SvxEMailTabPage::Reset( const SfxItemSet& )
{
    m_pMailerURLED->Enable(sal_True );
    m_pMailerURLPB->Enable(sal_True );

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
            sPath = OUString("/usr/bin");

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

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: pages.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 15:49:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "pages.hxx"
#include "wizard.hrc"
#include "wizard.hxx"
#include "migration.hxx"
#include "../app/desktopresid.hxx"
#include <vcl/msgbox.hxx>
#include <app.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <unotools/bootstrap.hxx>
#include <svtools/regoptions.hxx>
#include <svtools/useroptions.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/XDesktop.hpp>

namespace desktop
{

using namespace rtl;
using namespace osl;
using namespace utl;
using namespace svt;
using namespace com::sun::star::system;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;

static void _setBold(FixedText& ft)
{
    Font f = ft.GetControlFont();
    f.SetWeight(WEIGHT_BOLD);
    ft.SetControlFont(f);
}

WelcomePage::WelcomePage( svt::OWizardMachine* parent, const ResId& resid)
    : OWizardPage(parent, resid)
    , m_ftHead(this, DesktopResId(FT_WELCOME_HEADER))
    , m_ftBody(this, DesktopResId(FT_WELCOME_BODY))
{
    FreeResource();

    _setBold(m_ftHead);

    // we need to choose the welcome text that is diplayed
    // choices are the default text, default text+migradtion,
    // OEM and extended OEM
    switch (checkOEM())
    {
    case OEM_NONE:
        // check for migration
        if (Migration::checkMigration())
        {
            String aText(DesktopResId(STR_WELCOME_MIGRATION));
            // replace %OLDPRODUCT with found version name
            aText.SearchAndReplaceAll( UniString::CreateFromAscii("%OLD_VERSION"), Migration::getOldVersionName());
            m_ftBody.SetText( aText );
        }
        break;
    case OEM_NORMAL:
        m_ftBody.SetText(String(DesktopResId(STR_WELCOME_OEM)));
        break;
    case OEM_EXTENDED:
        m_ftBody.SetText(String(DesktopResId(STR_WELCOME_OEM_EXT)));
        break;
    }

}

WelcomePage::OEMType WelcomePage::checkOEM()
{
    return OEM_NONE;
}




LicensePage::LicensePage( svt::OWizardMachine* parent, const ResId& resid)
    : OWizardPage(parent, resid)
    , m_ftHead(this, DesktopResId(FT_LICENSE_HEADER))
    , m_ftBody1(this, DesktopResId(FT_LICENSE_BODY_1))
    , m_ftBody1Txt(this, DesktopResId(FT_LICENSE_BODY_1_TXT))
    , m_ftBody2(this, DesktopResId(FT_LICENSE_BODY_2))
    , m_ftBody2Txt(this, DesktopResId(FT_LICENSE_BODY_2_TXT))
    , m_mlLicense(this, DesktopResId(ML_LICENSE))
    , m_pbDown(this, DesktopResId(PB_LICENSE_DOWN))
    , m_bLicenseRead(sal_False)
    , m_pParent(parent)
{
    FreeResource();

    _setBold(m_ftHead);

    m_mlLicense.SetEndReachedHdl( LINK(this, LicensePage, EndReachedHdl) );
    m_mlLicense.SetScrolledHdl( LINK(this, LicensePage, ScrolledHdl) );
    m_pbDown.SetClickHdl( LINK(this, LicensePage, PageDownHdl) );

    // We want a automatic repeating page down button
    WinBits aStyle = m_pbDown.GetStyle();
    aStyle |= WB_REPEAT;
    m_pbDown.SetStyle( aStyle );

    // replace %PAGEDOWN in text2 with button text
    String aText = m_ftBody1Txt.GetText();
    aText.SearchAndReplaceAll( UniString::CreateFromAscii("%PAGEDOWN"), m_pbDown.GetText());
    m_ftBody1Txt.SetText( aText );

    OUString aLicensePath = FirstStartWizard::getLicensePath();
    // load license text
    File aLicenseFile(aLicensePath);
    if ( aLicenseFile.open(OpenFlag_Read) == FileBase::E_None)
    {
        DirectoryItem d;
        DirectoryItem::get(aLicensePath, d);
        FileStatus fs(FileStatusMask_FileSize);
        d.getFileStatus(fs);
        sal_uInt64 nBytesRead = 0;
        sal_uInt64 nPosition = 0;
        sal_uInt32 nBytes = (sal_uInt32)fs.getFileSize();
        sal_Char *pBuffer = new sal_Char[nBytes];
        // FileBase RC r = FileBase::E_None;
        while (aLicenseFile.read(pBuffer+nPosition, nBytes-nPosition, nBytesRead) == FileBase::E_None
            && nPosition + nBytesRead < nBytes)
        {
            nPosition += nBytesRead;
        }
        OUString aLicenseString(pBuffer, nBytes, RTL_TEXTENCODING_UTF8,
                OSTRING_TO_OUSTRING_CVTFLAGS | RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE);
        delete[] pBuffer;
        m_mlLicense.SetText(aLicenseString);
    }
}

sal_Bool LicensePage::determineNextButtonState()
{
    return m_bLicenseRead;
}

IMPL_LINK( LicensePage, PageDownHdl, PushButton *, EMPTYARG )
{
    m_mlLicense.ScrollDown( SCROLL_PAGEDOWN );
    return 0;
}

IMPL_LINK( LicensePage, EndReachedHdl, LicenseView *, EMPTYARG )
{
    m_bLicenseRead = TRUE;
    implCheckNextButton();
    return 0;
}

IMPL_LINK( LicensePage, ScrolledHdl, LicenseView *, EMPTYARG )
{
    implCheckNextButton();
    return 0;
}


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

BOOL LicenseView::IsEndReached() const
{
    BOOL bEndReached;

    ExtTextView*    pView = GetTextView();
    ExtTextEngine*  pEdit = GetTextEngine();
    ULONG           nHeight = pEdit->GetTextHeight();
    Size            aOutSize = pView->GetWindow()->GetOutputSizePixel();
    Point           aBottom( 0, aOutSize.Height() );

    if ( (ULONG) pView->GetDocPos( aBottom ).Y() >= nHeight - 1 )
        bEndReached = TRUE;
    else
        bEndReached = FALSE;

    return bEndReached;
}

void LicenseView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.IsA( TYPE(TextHint) ) )
    {
        BOOL    bLastVal = EndReached();
        ULONG   nId = ((const TextHint&)rHint).GetId();

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



// -------------------------------------------------------------------

MigrationPage::MigrationPage( svt::OWizardMachine* parent, const ResId& resid)
    : OWizardPage(parent, resid)
    , m_ftHead(this, DesktopResId(FT_MIGRATION_HEADER))
    , m_ftBody(this, DesktopResId(FT_MIGRATION_BODY))
    , m_cbMigration(this, DesktopResId(CB_MIGRATION))
    , m_bMigrationDone(sal_False)
{
    FreeResource();
    _setBold(m_ftHead);

    // replace %OLDPRODUCT with found version name
    String aText = m_ftBody.GetText();
    aText.SearchAndReplaceAll( UniString::CreateFromAscii("%OLDPRODUCT"), Migration::getOldVersionName());
    m_ftBody.SetText( aText );
}

sal_Bool MigrationPage::commitPage(COMMIT_REASON _eReason)
{
    if (_eReason == eTravelForward && m_cbMigration.IsChecked() && !m_bMigrationDone)
    {
        Migration::doMigration();
        m_bMigrationDone = sal_True;
    }
    else
        Migration::cancelMigration();
    return sal_True;
}

UserPage::UserPage( svt::OWizardMachine* parent, const ResId& resid)
    : OWizardPage(parent, resid)
    , m_ftHead(this, DesktopResId(FT_USER_HEADER))
    , m_ftBody(this, DesktopResId(FT_USER_BODY))
    , m_ftFirst(this, DesktopResId(FT_USER_FIRST))
    , m_ftLast(this, DesktopResId(FT_USER_LAST))
    , m_ftInitials(this, DesktopResId(FT_USER_INITIALS))
    , m_ftFather(this, DesktopResId(FT_USER_FATHER))
    , m_edFirst(this, DesktopResId(ED_USER_FIRST))
    , m_edLast(this, DesktopResId(ED_USER_LAST))
    , m_edInitials(this, DesktopResId(ED_USER_INITIALS))
    , m_edFather(this, DesktopResId(ED_USER_FATHER))
    , m_lang(Application::GetSettings().GetUILanguage())
{
    FreeResource();
    _setBold(m_ftHead);

    // check whether this is a russian version. otherwise
    // we'll hide the 'Fathers name' field
    SvtUserOptions aUserOpt;
    m_edFirst.SetText(aUserOpt.GetFirstName());
    m_edLast.SetText(aUserOpt.GetLastName());
    m_edInitials.SetText(aUserOpt.GetID());
    if (m_lang == LANGUAGE_RUSSIAN)
    {
        m_ftFather.Show();
        m_edFather.Show();
        m_edFather.SetText(aUserOpt.GetFathersName());
    }
}

sal_Bool UserPage::commitPage(COMMIT_REASON _eReason)
{
    SvtUserOptions aUserOpt;
    aUserOpt.SetFirstName(m_edFirst.GetText());
    aUserOpt.SetLastName(m_edLast.GetText());
    aUserOpt.SetID( m_edInitials.GetText());
    if (m_lang == LANGUAGE_RUSSIAN)
        aUserOpt.SetFathersName(m_edFather.GetText());

    return sal_True;
}


RegistrationPage::RegistrationPage( svt::OWizardMachine* parent, const ResId& resid)
    : OWizardPage(parent, resid)
    , m_ftHeader(this, DesktopResId(FT_REGISTRATION_HEADER))
    , m_fiImage(this, DesktopResId(IMG_REGISTRATION))
    , m_ftBody(this, DesktopResId(FT_REGISTRATION_BODY))
    , m_rbNow(this, DesktopResId(RB_REGISTRATION_NOW))
    , m_rbLater(this, DesktopResId(RB_REGISTRATION_LATER))
    , m_rbNever(this, DesktopResId(RB_REGISTRATION_NEVER))
    , m_rbReg(this, DesktopResId(RB_REGISTRATION_REG))
    , m_flSeparator(this, DesktopResId(FL_REGISTRATION))
    , m_ftEnd(this, DesktopResId(FT_REGISTRATION_END))
{
    FreeResource();
    _setBold(m_ftHeader);
}

sal_Bool RegistrationPage::determineNextButtonState()
{
    return sal_False;
}

sal_Bool RegistrationPage::commitPage(COMMIT_REASON _eReason)
{
    if ( _eReason == eFinish )
    {
        RegOptions aOptions;
        if ( m_rbNow.IsChecked())
        {
            sal_Bool bSuccess = sal_False;
            try
            {
                // create the Desktop component which can load components
                Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
                Reference< XSystemShellExecute > xSystemShell(
                    xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.system.SystemShellExecute" ) ),
                    UNO_QUERY_THROW
                );

                // access the configuration to retrieve the URL we shall use for registration
                ::rtl::OUString sRegistrationURL( aOptions.getRegistrationURL( ) );
                OSL_ENSURE( sRegistrationURL.getLength(), "OProductRegistration::doOnlineRegistration: invalid URL found!" );
                if ( xSystemShell.is() && sRegistrationURL.getLength() )
                {
                    xSystemShell->execute( sRegistrationURL, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
                    bSuccess = sal_True;
                }
            }
            catch( const Exception& )
            {
            }
            if ( !bSuccess )
            {
                ErrorBox aRegistrationError( this, DesktopResId( ERRBOX_REG_NOSYSBROWSER ) );
                aRegistrationError.Execute();
            }
        }
        else if (m_rbLater.IsChecked())
        {
            aOptions.activateReminder(7);
            // avtivate a reminder job...
        }
        // aOptions.markSessionDone();
    }
    return sal_True;
}

} // namespace desktop



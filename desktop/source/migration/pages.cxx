/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "pages.hxx"
#include "wizard.hrc"
#include "wizard.hxx"
#include "migration.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/mnemonic.hxx>
#include <vos/security.hxx>
#include <app.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/regoptions.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/basedlgs.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <comphelper/configurationhelper.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <unotools/bootstrap.hxx>
#include <tools/config.hxx>

using namespace rtl;
using namespace osl;
using namespace utl;
using namespace svt;
using namespace com::sun::star;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace desktop {

static void _setBold(FixedText& ft)
{
    Font f = ft.GetControlFont();
    f.SetWeight(WEIGHT_BOLD);
    ft.SetControlFont(f);
}

WelcomePage::WelcomePage( svt::OWizardMachine* parent, const ResId& resid, sal_Bool bLicenseNeedsAcceptance )
    : OWizardPage(parent, resid)
    , m_ftHead(this, WizardResId(FT_WELCOME_HEADER))
    , m_ftBody(this, WizardResId(FT_WELCOME_BODY))
    , m_pParent(parent)
    , m_bLicenseNeedsAcceptance( bLicenseNeedsAcceptance )
    , bIsEvalVersion(false)
    , bNoEvalText(false)
{
    FreeResource();

    _setBold(m_ftHead);

    checkEval();

    // check for migration
    if (Migration::checkMigration())
    {
        String aText(WizardResId(STR_WELCOME_MIGRATION));
        // replace %OLDPRODUCT with found version name
        aText.SearchAndReplaceAll( UniString::CreateFromAscii("%OLD_VERSION"), Migration::getOldVersionName());
        m_ftBody.SetText( aText );
    }
    else if ( ! m_bLicenseNeedsAcceptance )
    {
        String aText(WizardResId(STR_WELCOME_WITHOUT_LICENSE));
        m_ftBody.SetText( aText );
    }
}


void WelcomePage::checkEval()
{
    Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    Reference< XMaterialHolder > xHolder(xFactory->createInstance(
        OUString::createFromAscii("com.sun.star.tab.tabreg")), UNO_QUERY);
    if (xHolder.is()) {
        Any aData = xHolder->getMaterial();
        Sequence < NamedValue > aSeq;
        if (aData >>= aSeq) {
            bIsEvalVersion = true;
            for (int i=0; i< aSeq.getLength(); i++) {
                if (aSeq[i].Name.equalsAscii("NoEvalText")) {
                    aSeq[i].Value >>= bNoEvalText;
                }
            }
        }
    }
}


void WelcomePage::ActivatePage()
{
    OWizardPage::ActivatePage();
    // this page has no controls, so forwarding to default
    // button (next) won't work if we grap focus
    // GrabFocus();
}

// -------------------------------------------------------------------

class MigrationThread : public ::osl::Thread
{
    public:
        MigrationThread();

        virtual void SAL_CALL run();
        virtual void SAL_CALL onTerminated();
};

MigrationThread::MigrationThread()
{
}

void MigrationThread::run()
{
    try
    {
        Migration::doMigration();
    }
    catch ( uno::Exception& )
    {
    }
}

void MigrationThread::onTerminated()
{
}

// -------------------------------------------------------------------

MigrationPage::MigrationPage(
    svt::OWizardMachine* parent,
    const ResId& resid, Throbber& i_throbber )
    : OWizardPage(parent, resid)
    , m_ftHead(this, WizardResId(FT_MIGRATION_HEADER))
    , m_ftBody(this, WizardResId(FT_MIGRATION_BODY))
    , m_cbMigration(this, WizardResId(CB_MIGRATION))
    , m_rThrobber(i_throbber)
    , m_bMigrationDone(sal_False)
{
    FreeResource();
    _setBold(m_ftHead);

    // replace %OLDPRODUCT with found version name
    String aText = m_ftBody.GetText();
    aText.SearchAndReplaceAll( UniString::CreateFromAscii("%OLDPRODUCT"), Migration::getOldVersionName());
    m_ftBody.SetText( aText );
}

sal_Bool MigrationPage::commitPage( svt::WizardTypes::CommitPageReason _eReason )
{
    if (_eReason == svt::WizardTypes::eTravelForward && m_cbMigration.IsChecked() && !m_bMigrationDone)
    {
        GetParent()->EnterWait();
        FirstStartWizard* pWizard = dynamic_cast< FirstStartWizard* >( GetParent() );
        if ( pWizard )
            pWizard->DisableButtonsWhileMigration();

        m_rThrobber.Show();
        m_rThrobber.start();
        MigrationThread* pMigThread = new MigrationThread();
        pMigThread->create();

        while ( pMigThread->isRunning() )
        {
            Application::Reschedule();
        }

        m_rThrobber.stop();
        GetParent()->LeaveWait();
        // Next state will enable buttons - so no EnableButtons necessary!
        m_rThrobber.Hide();
        pMigThread->join();
        delete pMigThread;
        m_bMigrationDone = sal_True;
    }
    else
        Migration::cancelMigration();
    return sal_True;
}

void MigrationPage::ActivatePage()
{
    OWizardPage::ActivatePage();
    GrabFocus();
}

UserPage::UserPage( svt::OWizardMachine* parent, const ResId& resid)
    : OWizardPage(parent, resid)
    , m_ftHead(this, WizardResId(FT_USER_HEADER))
    , m_ftBody(this, WizardResId(FT_USER_BODY))
    , m_ftFirst(this, WizardResId(FT_USER_FIRST))
    , m_edFirst(this, WizardResId(ED_USER_FIRST))
    , m_ftLast(this, WizardResId(FT_USER_LAST))
    , m_edLast(this, WizardResId(ED_USER_LAST))
    , m_ftInitials(this, WizardResId(FT_USER_INITIALS))
    , m_edInitials(this, WizardResId(ED_USER_INITIALS))
    , m_ftFather(this, WizardResId(FT_USER_FATHER))
    , m_edFather(this, WizardResId(ED_USER_FATHER))
    , m_lang(Application::GetSettings().GetUILanguage())
{
    FreeResource();
    _setBold(m_ftHead);

    // check whether this is a russian version. otherwise
    // we'll hide the 'Fathers name' field
    SvtUserOptions aUserOpt;
    m_edFirst.SetText(aUserOpt.GetFirstName());
    m_edLast.SetText(aUserOpt.GetLastName());
#if 0
    rtl::OUString aUserName;
    vos::OSecurity().getUserName( aUserName );
       aUserOpt.SetID( aUserName );
#endif

    m_edInitials.SetText(aUserOpt.GetID());
    if (m_lang == LANGUAGE_RUSSIAN)
    {
        m_ftFather.Show();
        m_edFather.Show();
        m_edFather.SetText(aUserOpt.GetFathersName());
    }
}

sal_Bool UserPage::commitPage( svt::WizardTypes::CommitPageReason )
{
    SvtUserOptions aUserOpt;
    aUserOpt.SetFirstName(m_edFirst.GetText());
    aUserOpt.SetLastName(m_edLast.GetText());
    aUserOpt.SetID( m_edInitials.GetText());

    if (m_lang == LANGUAGE_RUSSIAN)
        aUserOpt.SetFathersName(m_edFather.GetText());

    return sal_True;
}

void UserPage::ActivatePage()
{
    OWizardPage::ActivatePage();
    GrabFocus();
}

// -------------------------------------------------------------------
UpdateCheckPage::UpdateCheckPage( svt::OWizardMachine* parent, const ResId& resid)
    : OWizardPage(parent, resid)
    , m_ftHead(this, WizardResId(FT_UPDATE_CHECK_HEADER))
    , m_ftBody(this, WizardResId(FT_UPDATE_CHECK_BODY))
    , m_cbUpdateCheck(this, WizardResId(CB_UPDATE_CHECK))
{
    FreeResource();
    _setBold(m_ftHead);
}

sal_Bool UpdateCheckPage::commitPage( svt::WizardTypes::CommitPageReason _eReason )
{
    if ( _eReason == svt::WizardTypes::eTravelForward )
    {
        try {
            Reference < XNameReplace > xUpdateAccess;
            Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

            xUpdateAccess = Reference < XNameReplace >(
                xFactory->createInstance( UNISTRING( "com.sun.star.setup.UpdateCheckConfig" ) ), UNO_QUERY_THROW );

            if ( !xUpdateAccess.is() )
                return sal_False;

            sal_Bool bAutoUpdChk = m_cbUpdateCheck.IsChecked();
            xUpdateAccess->replaceByName( UNISTRING("AutoCheckEnabled"), makeAny( bAutoUpdChk ) );

            Reference< XChangesBatch > xChangesBatch( xUpdateAccess, UNO_QUERY);
            if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
                xChangesBatch->commitChanges();
        } catch (RuntimeException)
        {
        }
    }

    return sal_True;
}

void UpdateCheckPage::ActivatePage()
{
    OWizardPage::ActivatePage();
    GrabFocus();
}

} // namespace desktop


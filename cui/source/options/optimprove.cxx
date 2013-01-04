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
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#define _SVX_OPTIMPROVE_CXX

#include <optimprove.hxx>
#include <dialmgr.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dialogs.hrc>
#include "optimprove.hrc"
#include "helpid.hrc"
#include <cuires.hrc>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/oooimprovement/XCoreController.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/uieventslogger.hxx>
#include <tools/testtoolloader.hxx>

namespace lang  = ::com::sun::star::lang;
namespace uno   = ::com::sun::star::uno;
using namespace com::sun::star::system;

// class SvxImprovementPage ----------------------------------------------

SvxImprovementPage::SvxImprovementPage( Window* pParent ) :

    TabPage( pParent, CUI_RES( RID_SVXPAGE_IMPROVEMENT ) ),

    m_aImproveFL                ( this, CUI_RES( FL_IMPROVE ) ),
    m_aInvitationFT             ( this, CUI_RES( FT_INVITATION ) ),
    m_aYesRB                    ( this, CUI_RES( RB_YES ) ),
    m_aNoRB                     ( this, CUI_RES( RB_NO ) ),
    m_aDataFL                   ( this, CUI_RES( FL_DATA ) ),
    m_aNumberOfReportsFT        ( this, CUI_RES( FT_NR_REPORTS ) ),
    m_aNumberOfReportsValueFT   ( this, CUI_RES( FT_NR_REPORTS_VALUE ) ),
    m_aNumberOfActionsFT        ( this, CUI_RES( FT_NR_ACTIONS ) ),
    m_aNumberOfActionsValueFT   ( this, CUI_RES( FT_NR_ACTIONS_VALUE ) ),
    m_aShowDataPB               ( this, CUI_RES( PB_SHOWDATA ) ),

    m_sInfo                     (       CUI_RES( STR_INFO ) ),
    m_sMoreInfo                 (       CUI_RES( STR_MOREINFO ) )

{
    FreeResource();

    m_aInvitationFT.Show();
    m_aDataFL.Hide();
    m_aNumberOfReportsFT.Hide();
    m_aNumberOfReportsValueFT.Hide();
    m_aNumberOfActionsFT.Hide();
    m_aNumberOfActionsValueFT.Hide();
    m_aShowDataPB.Hide();

    Size aNewSize = m_aInvitationFT.GetSizePixel();
    const long nMinWidth = m_aYesRB.CalcMinimumSize().Width();
    const long nNewWidth = std::max( aNewSize.Width() * 4 / 5, nMinWidth );
    const long nWDelta = aNewSize.Width() - nNewWidth;
    aNewSize.Width() = nNewWidth;
    const Size aCalcSize = m_aInvitationFT.CalcMinimumSize( nNewWidth );
    const long nHDelta = aCalcSize.Height() - aNewSize.Height();
    aNewSize.Height() = aCalcSize.Height();
    m_aInvitationFT.SetSizePixel( aNewSize );

    aNewSize = m_aYesRB.GetSizePixel();
    aNewSize.Width() = nNewWidth;
    Point aNewPos = m_aYesRB.GetPosPixel();
    aNewPos.Y() += nHDelta;
    m_aYesRB.SetPosSizePixel( aNewPos, aNewSize );
    aNewSize = m_aNoRB.GetSizePixel();
    aNewSize.Width() = nNewWidth;
    aNewPos = m_aNoRB.GetPosPixel();
    aNewPos.Y() += nHDelta;
    m_aNoRB.SetPosSizePixel( aNewPos, aNewSize );
    aNewSize = m_aImproveFL.GetSizePixel();
    aNewSize.Width() -= nWDelta;
    m_aImproveFL.SetSizePixel( aNewSize );

    Size aSize = GetOutputSizePixel();
    aSize.Width() -= nWDelta;
    aSize.Height() = m_aDataFL.GetPosPixel().Y();
    aSize.Height() += nHDelta;
    SetSizePixel( aSize );
}

// -----------------------------------------------------------------------

SvxImprovementPage::~SvxImprovementPage()
{
}

// class SvxImprovementDialog --------------------------------------------

SvxImprovementDialog::SvxImprovementDialog( Window* pParent, const String& rInfoURL ) :

    SfxSingleTabDialog( pParent, RID_SVXPAGE_IMPROVEMENT, rInfoURL ),

    m_pPage( NULL )

{
    m_pPage = new SvxImprovementPage( this );
    SetInfoLink( LINK( this, SvxImprovementDialog, HandleHyperlink ) );
    SetPage( m_pPage );
    if ( GetOKButton() )
        GetOKButton()->SetClickHdl( LINK( this, SvxImprovementDialog, HandleOK ) );
}

IMPL_LINK( SvxImprovementDialog, HandleHyperlink, svt::FixedHyperlinkImage*, pHyperlinkImage )
{
    ::rtl::OUString sURL( pHyperlinkImage->GetURL() );

    if ( sURL.getLength() > 0 )
    {
        try
        {
            uno::Reference< XSystemShellExecute > xSystemShell(
                com::sun::star::system::SystemShellExecute::create(
                    ::comphelper::getProcessComponentContext() ) );
            if ( xSystemShell.is() )
            {
                xSystemShell->execute(
                    sURL, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
            }
        }
        catch( const uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }

    return 0;
}

IMPL_LINK( SvxImprovementDialog, HandleOK, OKButton*, EMPTYARG )
{
    uno::Reference< lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    uno::Reference< com::sun::star::oooimprovement::XCoreController > core_c(
            xSMGR->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.oooimprovement.CoreController")),
            uno::UNO_QUERY);
    if(core_c.is())
    {
        ::comphelper::ConfigurationHelper::writeDirectKey(
            xSMGR,
            ::rtl::OUString::createFromAscii("/org.openoffice.Office.OOoImprovement.Settings"),
            ::rtl::OUString::createFromAscii("Participation"),
            ::rtl::OUString::createFromAscii("ShowedInvitation"),
            uno::makeAny( true ),
            ::comphelper::ConfigurationHelper::E_STANDARD );
        ::comphelper::ConfigurationHelper::writeDirectKey(
            xSMGR,
            ::rtl::OUString::createFromAscii("/org.openoffice.Office.OOoImprovement.Settings"),
            ::rtl::OUString::createFromAscii("Participation"),
            ::rtl::OUString::createFromAscii("InvitationAccepted"),
            uno::makeAny( m_pPage->IsYesChecked() ),
            ::comphelper::ConfigurationHelper::E_STANDARD );
        // TODO: refactor
        ::comphelper::UiEventsLogger::reinit();
        ::tools::InitTestToolLib();
    }
    EndDialog( RET_OK );
    return 0;
}

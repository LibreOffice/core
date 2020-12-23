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

#include <config_features.h>
#include <config_vclplug.h>
#include <svl/zforlist.hxx>
#include <svl/currencytable.hxx>
#include <svtools/langhelp.hxx>
#include <unotools/lingucfg.hxx>
#if defined(_WIN32)
#include <unotools/resmgr.hxx>
#endif
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/compatibility.hxx>
#include <svtools/menuoptions.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/propertysequence.hxx>
#include <svtools/langtab.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/searchopt.hxx>
#include <sal/log.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <comphelper/configuration.hxx>
#include <tools/diagnose_ex.h>
#if HAVE_FEATURE_BREAKPAD
#include <desktop/crashreport.hxx>
#endif

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/office/Quickstart.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <comphelper/dispatchcommand.hxx>

#include <vcl/vclenum.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include "optgdlg.hxx"
#include <svtools/apearcfg.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <svtools/restartdialog.hxx>
#include <svtools/imgdef.hxx>

#if defined(_WIN32)
#include <o3tl/char16_t2wchar_t.hxx>
#include <prewin.h>
#include <shobjidl.h>
#include <postwin.h>
#endif
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::utl;

// class OfaMiscTabPage --------------------------------------------------

DeactivateRC OfaMiscTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( pSet_ );
    return DeactivateRC::LeavePage;
}

namespace
{
OUString impl_SystemFileOpenServiceName()
{
    #if defined(_WIN32)
    return "com.sun.star.ui.dialogs.SystemFilePicker";
    #elif defined MACOSX
    return "com.sun.star.ui.dialogs.AquaFilePicker";
    #else
    return OUString();
    #endif
}

bool lcl_HasSystemFilePicker()
{
    if( Application::hasNativeFileSelection() )
        return true;

    // Otherwise fall-back on querying services
    bool bRet = false;
    Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();

    Reference< XContentEnumerationAccess > xEnumAccess( xFactory, UNO_QUERY );
    Reference< XSet > xSet( xFactory, UNO_QUERY );

    if ( ! xEnumAccess.is() || ! xSet.is() )
        return bRet;

    try
    {
        OUString aFileService = impl_SystemFileOpenServiceName();
        Reference< XEnumeration > xEnum = xEnumAccess->createContentEnumeration( aFileService );
        if ( xEnum.is() && xEnum->hasMoreElements() )
            bRet = true;
    }
    catch (const IllegalArgumentException&)
    {
    }
    catch (const ElementExistException&)
    {
    }
    return bRet;
}
}

OfaMiscTabPage::OfaMiscTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optgeneralpage.ui", "OptGeneralPage", &rSet)
    , m_xExtHelpCB(m_xBuilder->weld_check_button("exthelp"))
    , m_xPopUpNoHelpCB(m_xBuilder->weld_check_button("popupnohelp"))
    , m_xShowTipOfTheDay(m_xBuilder->weld_check_button("cbShowTipOfTheDay"))
    , m_xFileDlgFrame(m_xBuilder->weld_widget("filedlgframe"))
    , m_xPrintDlgFrame(m_xBuilder->weld_widget("printdlgframe"))
    , m_xFileDlgROImage(m_xBuilder->weld_widget("lockimage"))
    , m_xFileDlgCB(m_xBuilder->weld_check_button("filedlg"))
    , m_xPrintDlgCB(m_xBuilder->weld_check_button("printdlg"))
    , m_xDocStatusCB(m_xBuilder->weld_check_button("docstatus"))
    , m_xYearFrame(m_xBuilder->weld_widget("yearframe"))
    , m_xYearValueField(m_xBuilder->weld_spin_button("year"))
    , m_xToYearFT(m_xBuilder->weld_label("toyear"))
    , m_xCollectUsageInfo(m_xBuilder->weld_check_button("collectusageinfo"))
    , m_xCrashReport(m_xBuilder->weld_check_button("crashreport"))
    , m_xQuickStarterFrame(m_xBuilder->weld_widget("quickstarter"))
#if defined(UNX)
    , m_xQuickLaunchCB(m_xBuilder->weld_check_button("systray"))
#else
    , m_xQuickLaunchCB(m_xBuilder->weld_check_button("quicklaunch"))
#endif
#if defined(_WIN32)
    , m_xFileAssocFrame(m_xBuilder->weld_widget("fileassoc"))
    , m_xFileAssocBtn(m_xBuilder->weld_button("assocfiles"))
#endif
{
    if (!lcl_HasSystemFilePicker())
        m_xFileDlgFrame->hide();
    else if (officecfg::Office::Common::Misc::UseSystemFileDialog::isReadOnly())
    {
        m_xFileDlgROImage->show();
        m_xFileDlgCB->set_sensitive(false);
    }

#if !ENABLE_GTK3
    m_xPrintDlgFrame->hide();
#else
    if (!officecfg::Office::Common::Misc::ExperimentalMode::get())
    {
        m_xPrintDlgFrame->hide();
    }
#endif

    m_xQuickLaunchCB->show();

    //Only available in Win or if building the gtk systray
#if !defined(_WIN32)
    m_xQuickStarterFrame->hide();
#endif

#if defined(_WIN32)
    m_xFileAssocFrame->show();
    m_xFileAssocBtn->connect_clicked(LINK(this, OfaMiscTabPage, FileAssocClick));
#endif

    m_aStrDateInfo = m_xToYearFT->get_label();
    m_xYearValueField->connect_value_changed( LINK( this, OfaMiscTabPage, TwoFigureHdl ) );

    SetExchangeSupport();
}

OfaMiscTabPage::~OfaMiscTabPage()
{
}

std::unique_ptr<SfxTabPage> OfaMiscTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaMiscTabPage>( pPage, pController, *rAttrSet );
}

bool OfaMiscTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    SvtHelpOptions aHelpOptions;
    if ( m_xPopUpNoHelpCB->get_state_changed_from_saved() )
        aHelpOptions.SetOfflineHelpPopUp( m_xPopUpNoHelpCB->get_active() );

    if ( m_xExtHelpCB->get_state_changed_from_saved() )
        aHelpOptions.SetExtendedHelp( m_xExtHelpCB->get_active() );

    if ( m_xShowTipOfTheDay->get_state_changed_from_saved() )
    {
        officecfg::Office::Common::Misc::ShowTipOfTheDay::set(m_xShowTipOfTheDay->get_active(), batch);
        bModified = true;
    }

    if ( m_xFileDlgCB->get_state_changed_from_saved() )
    {
        std::shared_ptr< comphelper::ConfigurationChanges > xChanges(
                comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::UseSystemFileDialog::set( !m_xFileDlgCB->get_active(), xChanges );
        xChanges->commit();
        bModified = true;
    }

    if ( m_xPrintDlgCB->get_state_changed_from_saved() )
    {
        std::shared_ptr< comphelper::ConfigurationChanges > xChanges(
                comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::UseSystemPrintDialog::set( !m_xPrintDlgCB->get_active(), xChanges );
        xChanges->commit();
        bModified = true;
    }

    if (m_xDocStatusCB->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Print::PrintingModifiesDocument::set(m_xDocStatusCB->get_active(), batch);
        bModified = true;
    }

    const SfxUInt16Item* pUInt16Item = dynamic_cast< const SfxUInt16Item* >( GetOldItem( *rSet, SID_ATTR_YEAR2000 ) );
    sal_uInt16 nNum = static_cast<sal_uInt16>(m_xYearValueField->get_text().toInt32());
    if ( pUInt16Item && pUInt16Item->GetValue() != nNum )
    {
        bModified = true;
        rSet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, nNum ) );
    }

    if (m_xCollectUsageInfo->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Misc::CollectUsageInformation::set(m_xCollectUsageInfo->get_active(), batch);
        bModified = true;
    }

#if HAVE_FEATURE_BREAKPAD
    if (m_xCrashReport->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Misc::CrashReport::set(m_xCrashReport->get_active(), batch);
        bModified = true;
    }
#endif

    batch->commit();

    if( m_xQuickLaunchCB->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(SID_ATTR_QUICKLAUNCHER, m_xQuickLaunchCB->get_active()));
        bModified = true;
    }

    return bModified;
}

void OfaMiscTabPage::Reset( const SfxItemSet* rSet )
{
    SvtHelpOptions aHelpOptions;
    m_xExtHelpCB->set_active( aHelpOptions.IsHelpTips() && aHelpOptions.IsExtendedHelp() );
    m_xExtHelpCB->save_state();
    m_xPopUpNoHelpCB->set_active( aHelpOptions.IsOfflineHelpPopUp() );
    m_xPopUpNoHelpCB->save_state();
    m_xShowTipOfTheDay->set_active( officecfg::Office::Common::Misc::ShowTipOfTheDay::get() );
    m_xShowTipOfTheDay->save_state();
    m_xFileDlgCB->set_active( !officecfg::Office::Common::Misc::UseSystemFileDialog::get() );
    m_xFileDlgCB->save_state();
    m_xPrintDlgCB->set_active( !officecfg::Office::Common::Misc::UseSystemPrintDialog::get() );
    m_xPrintDlgCB->save_state();

    m_xDocStatusCB->set_active(officecfg::Office::Common::Print::PrintingModifiesDocument::get());
    m_xDocStatusCB->save_state();

    const SfxPoolItem* pItem = nullptr;
    if ( SfxItemState::SET == rSet->GetItemState( SID_ATTR_YEAR2000, false, &pItem ) )
    {
        m_xYearValueField->set_value( static_cast<const SfxUInt16Item*>(pItem)->GetValue() );
        TwoFigureHdl(*m_xYearValueField);
    }
    else
        m_xYearFrame->set_sensitive(false);

    m_xCollectUsageInfo->set_active(officecfg::Office::Common::Misc::CollectUsageInformation::get());
    m_xCollectUsageInfo->set_sensitive(!officecfg::Office::Common::Misc::CollectUsageInformation::isReadOnly());
    m_xCollectUsageInfo->save_state();

#if HAVE_FEATURE_BREAKPAD
    m_xCrashReport->set_active(officecfg::Office::Common::Misc::CrashReport::get() && CrashReporter::IsDumpEnable());
    m_xCrashReport->set_sensitive(!officecfg::Office::Common::Misc::CrashReport::isReadOnly() && CrashReporter::IsDumpEnable());
    m_xCrashReport->save_state();
#else
    m_xCrashReport->hide();
#endif

    SfxItemState eState = rSet->GetItemState( SID_ATTR_QUICKLAUNCHER, false, &pItem );
    if ( SfxItemState::SET == eState )
        m_xQuickLaunchCB->set_active( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
    else if ( SfxItemState::DISABLED == eState )
    {
        // quickstart not installed
        m_xQuickStarterFrame->hide();
    }

    m_xQuickLaunchCB->save_state();
}

IMPL_LINK_NOARG( OfaMiscTabPage, TwoFigureHdl, weld::SpinButton&, void )
{
    OUString aOutput( m_aStrDateInfo );
    OUString aStr( m_xYearValueField->get_text() );
    sal_Int32 nNum = aStr.toInt32();
    if ( aStr.getLength() != 4 || nNum < m_xYearValueField->get_min() || nNum > m_xYearValueField->get_max() )
        aOutput += "????";
    else
    {
        nNum += 99;
        aOutput += OUString::number( nNum );
    }
    m_xToYearFT->set_label( aOutput );
}

#if defined(_WIN32)
IMPL_STATIC_LINK_NOARG(OfaMiscTabPage, FileAssocClick, weld::Button&, void)
{
    const bool bUninit = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
    IApplicationAssociationRegistrationUI* pIf = nullptr;
    HRESULT res = CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI, nullptr,
                                   CLSCTX_INPROC_SERVER, IID_IApplicationAssociationRegistrationUI,
                                   reinterpret_cast<LPVOID*>(&pIf));

    if (SUCCEEDED(res) && pIf)
    {
        // LaunchAdvancedAssociationUI only works for applications registered under
        // Software\RegisteredApplications. See scp2/source/ooo/registryitem_ooo.scp
        const OUString expanded = Translate::ExpandVariables("%PRODUCTNAME %PRODUCTVERSION");
        // This will only show "To change your default apps, go to Settings > Apps > Default apps"
        // on Win10; this is expected. At least this will self-document it to users.
        pIf->LaunchAdvancedAssociationUI(o3tl::toW(expanded.getStr()));
        pIf->Release();
    }
    if (bUninit)
        CoUninitialize();
}
#endif

class CanvasSettings
{
public:
    CanvasSettings();

    bool    IsHardwareAccelerationEnabled() const;
    bool    IsHardwareAccelerationAvailable() const;
    bool    IsHardwareAccelerationRO() const;
    void    EnabledHardwareAcceleration( bool _bEnabled ) const;

private:
    typedef std::vector< std::pair<OUString,Sequence<OUString> > > ServiceVector;

    Reference<XNameAccess> mxForceFlagNameAccess;
    ServiceVector          maAvailableImplementations;
    mutable bool           mbHWAccelAvailable;
    mutable bool           mbHWAccelChecked;
};

CanvasSettings::CanvasSettings() :
    mxForceFlagNameAccess(),
    mbHWAccelAvailable(false),
    mbHWAccelChecked(false)
{
    try
    {
        Reference<XMultiServiceFactory> xConfigProvider(
            css::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));

        Sequence<Any> aArgs1(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(OUString("/org.openoffice.Office.Canvas"))}
        }));
        mxForceFlagNameAccess.set(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationUpdateAccess",
                aArgs1 ),
            UNO_QUERY_THROW );

        Sequence<Any> aArgs2(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(OUString("/org.openoffice.Office.Canvas/CanvasServiceList"))}
        }));
        Reference<XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                aArgs2 ), UNO_QUERY_THROW );
        Reference<XHierarchicalNameAccess> xHierarchicalNameAccess(
            xNameAccess, UNO_QUERY_THROW);

        Sequence<OUString> serviceNames = xNameAccess->getElementNames();
        const OUString* pCurr = serviceNames.getConstArray();
        const OUString* const pEnd = pCurr + serviceNames.getLength();
        while( pCurr != pEnd )
        {
            Reference<XNameAccess> xEntryNameAccess(
                xHierarchicalNameAccess->getByHierarchicalName(*pCurr),
                UNO_QUERY );

            if( xEntryNameAccess.is() )
            {
                Sequence<OUString> preferredImplementations;
                if( xEntryNameAccess->getByName("PreferredImplementations") >>= preferredImplementations )
                    maAvailableImplementations.emplace_back(*pCurr,preferredImplementations );
            }

            ++pCurr;
        }
    }
    catch (const Exception&)
    {
    }
}

bool CanvasSettings::IsHardwareAccelerationAvailable() const
{
    if( !mbHWAccelChecked )
    {
        mbHWAccelChecked = true;

        Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();

        // check whether any of the service lists has an
        // implementation that presents the "HardwareAcceleration" property
        for (auto const& availableImpl : maAvailableImplementations)
        {
            const OUString* pCurrImpl = availableImpl.second.getConstArray();
            const OUString* const pEndImpl = pCurrImpl + availableImpl.second.getLength();

            while( pCurrImpl != pEndImpl )
            {
                try
                {
                    Reference<XPropertySet> xPropSet( xFactory->createInstance(
                                                          pCurrImpl->trim() ),
                                                      UNO_QUERY_THROW );
                    bool bHasAccel(false);
                    if( xPropSet->getPropertyValue("HardwareAcceleration") >>= bHasAccel )
                        if( bHasAccel )
                        {
                            mbHWAccelAvailable = true;
                            return mbHWAccelAvailable;
                        }
                }
                catch (const Exception&)
                {
                }

                ++pCurrImpl;
            }
        }
    }

    return mbHWAccelAvailable;
}

bool CanvasSettings::IsHardwareAccelerationEnabled() const
{
    bool bForceLastEntry(false);
    if( !mxForceFlagNameAccess.is() )
        return true;

    if( !(mxForceFlagNameAccess->getByName("ForceSafeServiceImpl") >>= bForceLastEntry) )
        return true;

    return !bForceLastEntry;
}

bool CanvasSettings::IsHardwareAccelerationRO() const
{
    Reference< XPropertySet > xSet(mxForceFlagNameAccess, UNO_QUERY);
    if (!xSet.is())
        return true;

    Reference< XPropertySetInfo > xInfo = xSet->getPropertySetInfo();
    Property aProp = xInfo->getPropertyByName("ForceSafeServiceImpl");
    return ((aProp.Attributes & css::beans::PropertyAttribute::READONLY ) == css::beans::PropertyAttribute::READONLY);
}

void CanvasSettings::EnabledHardwareAcceleration( bool _bEnabled ) const
{
    Reference< XNameReplace > xNameReplace(
        mxForceFlagNameAccess, UNO_QUERY );

    if( !xNameReplace.is() )
        return;

    xNameReplace->replaceByName( "ForceSafeServiceImpl", Any(!_bEnabled) );

    Reference< XChangesBatch > xChangesBatch(
        mxForceFlagNameAccess, UNO_QUERY );

    if( !xChangesBatch.is() )
        return;

    xChangesBatch->commitChanges();
}

// class OfaViewTabPage --------------------------------------------------

static bool DisplayNameCompareLessThan(const vcl::IconThemeInfo& rInfo1, const vcl::IconThemeInfo& rInfo2)
{
    return rInfo1.GetDisplayName().compareTo(rInfo2.GetDisplayName()) < 0;
}

OfaViewTabPage::OfaViewTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optviewpage.ui", "OptViewPage", &rSet)
    , nSizeLB_InitialSelection(0)
    , nSidebarSizeLB_InitialSelection(0)
    , nNotebookbarSizeLB_InitialSelection(0)
    , nStyleLB_InitialSelection(0)
    , pAppearanceCfg(new SvtTabAppearanceCfg)
    , pCanvasSettings(new CanvasSettings)
    , mpDrawinglayerOpt(new SvtOptionsDrawinglayer)
    , m_xIconSizeLB(m_xBuilder->weld_combo_box("iconsize"))
    , m_xSidebarIconSizeLB(m_xBuilder->weld_combo_box("sidebariconsize"))
    , m_xNotebookbarIconSizeLB(m_xBuilder->weld_combo_box("notebookbariconsize"))
    , m_xIconStyleLB(m_xBuilder->weld_combo_box("iconstyle"))
    , m_xFontAntiAliasing(m_xBuilder->weld_check_button("aafont"))
    , m_xAAPointLimitLabel(m_xBuilder->weld_label("aafrom"))
    , m_xAAPointLimit(m_xBuilder->weld_metric_spin_button("aanf", FieldUnit::PIXEL))
    , m_xMenuIconBox(m_xBuilder->weld_widget("menuiconsbox"))
    , m_xMenuIconsLB(m_xBuilder->weld_combo_box("menuicons"))
    , m_xContextMenuShortcutsLB(m_xBuilder->weld_combo_box("contextmenushortcuts"))
    , m_xFontShowCB(m_xBuilder->weld_check_button("showfontpreview"))
    , m_xUseHardwareAccell(m_xBuilder->weld_check_button("useaccel"))
    , m_xUseAntiAliase(m_xBuilder->weld_check_button("useaa"))
    , m_xUseSkia(m_xBuilder->weld_check_button("useskia"))
    , m_xForceSkiaRaster(m_xBuilder->weld_check_button("forceskiaraster"))
    , m_xSkiaStatusEnabled(m_xBuilder->weld_label("skiaenabled"))
    , m_xSkiaStatusDisabled(m_xBuilder->weld_label("skiadisabled"))
    , m_xMousePosLB(m_xBuilder->weld_combo_box("mousepos"))
    , m_xMouseMiddleLB(m_xBuilder->weld_combo_box("mousemiddle"))
    , m_xMoreIcons(m_xBuilder->weld_button("btnMoreIcons"))
{
    if (Application::GetToolkitName() == "gtk3")
        m_xMenuIconBox->hide();

    m_xFontAntiAliasing->connect_toggled( LINK( this, OfaViewTabPage, OnAntialiasingToggled ) );

    m_xUseSkia->connect_toggled(LINK(this, OfaViewTabPage, OnUseSkiaToggled));

    // Set known icon themes
    OUString sAutoStr( m_xIconStyleLB->get_text( 0 ) );
    m_xIconStyleLB->clear();
    StyleSettings aStyleSettings = Application::GetSettings().GetStyleSettings();
    mInstalledIconThemes = aStyleSettings.GetInstalledIconThemes();
    std::sort(mInstalledIconThemes.begin(), mInstalledIconThemes.end(), DisplayNameCompareLessThan);

    // Start with the automatically chosen icon theme
    OUString autoThemeId = aStyleSettings.GetAutomaticallyChosenIconTheme();
    const vcl::IconThemeInfo& autoIconTheme = vcl::IconThemeInfo::FindIconThemeById(mInstalledIconThemes, autoThemeId);

    OUString entryForAuto = sAutoStr + " (" +
                                autoIconTheme.GetDisplayName() +
                                ")";
    m_xIconStyleLB->append("auto", entryForAuto); // index 0 means choose style automatically

    // separate auto and other icon themes
    m_xIconStyleLB->append_separator("");

    for (auto const& installIconTheme : mInstalledIconThemes)
        m_xIconStyleLB->append(installIconTheme.GetThemeId(), installIconTheme.GetDisplayName());

    m_xIconStyleLB->set_active(0);

    m_xMoreIcons->set_from_icon_name("cmd/sc_additionsdialog.png");
    m_xMoreIcons->connect_clicked(LINK(this, OfaViewTabPage, OnMoreIconsClick));
}

OfaViewTabPage::~OfaViewTabPage()
{
}

IMPL_STATIC_LINK_NOARG(OfaViewTabPage, OnMoreIconsClick, weld::Button&, void)
{
    css::uno::Sequence<css::beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "AdditionsTag";
    aArgs[0].Value <<= OUString("Icons");
    comphelper::dispatchCommand(".uno:AdditionsDialog", aArgs);
}

IMPL_LINK_NOARG( OfaViewTabPage, OnAntialiasingToggled, weld::ToggleButton&, void )
{
    bool bAAEnabled = m_xFontAntiAliasing->get_active();

    m_xAAPointLimitLabel->set_sensitive(bAAEnabled);
    m_xAAPointLimit->set_sensitive(bAAEnabled);
}

IMPL_LINK_NOARG(OfaViewTabPage, OnUseSkiaToggled, weld::ToggleButton&, void)
{
    UpdateSkiaStatus();
}

void OfaViewTabPage::HideSkiaWidgets()
{
    m_xUseSkia->hide();
    m_xForceSkiaRaster->hide();
    m_xSkiaStatusEnabled->hide();
    m_xSkiaStatusDisabled->hide();
}

void OfaViewTabPage::UpdateSkiaStatus()
{
#if HAVE_FEATURE_SKIA
    bool skiaHidden = true;

    // For now Skia is used mainly on Windows, enable the controls there.
    if (Application::GetToolkitName() == "win")
        skiaHidden = false;
    // It can also be used on Linux, but only with the rarely used 'gen' backend.
    if (Application::GetToolkitName() == "x11")
        skiaHidden = false;

    if (skiaHidden)
    {
        HideSkiaWidgets();
        return;
    }

    // Easier than a custom translation string.
    bool bEnabled = SkiaHelper::isVCLSkiaEnabled();
    m_xSkiaStatusEnabled->set_visible(bEnabled);
    m_xSkiaStatusDisabled->set_visible(!bEnabled);

    // FIXME: should really add code to show a 'lock' icon here.
    m_xUseSkia->set_sensitive(!officecfg::Office::Common::VCL::UseSkia::isReadOnly());
    m_xForceSkiaRaster->set_sensitive(m_xUseSkia->get_active() && !officecfg::Office::Common::VCL::ForceSkiaRaster::isReadOnly());

    // Technically the 'use hardware acceleration' option could be used to mean !forceSkiaRaster, but the implementation
    // of the option is so tied to the implementation of the canvas module that it's simpler to ignore it.
    UpdateHardwareAccelStatus();
#else
    HideSkiaWidgets();
#endif
}

std::unique_ptr<SfxTabPage> OfaViewTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaViewTabPage>(pPage, pController, *rAttrSet);
}

bool OfaViewTabPage::FillItemSet( SfxItemSet* )
{
    SvtMenuOptions aMenuOpt;

    bool bModified = false;
    bool bMenuOptModified = false;
    bool bRepaintWindows(false);

    SvtMiscOptions aMiscOptions;
    const sal_Int32 nSizeLB_NewSelection = m_xIconSizeLB->get_active();
    if( nSizeLB_InitialSelection != nSizeLB_NewSelection )
    {
        // from now on it's modified, even if via auto setting the same size was set as now selected in the LB
        sal_Int16 eSet = SFX_SYMBOLS_SIZE_AUTO;
        switch( nSizeLB_NewSelection )
        {
            case 0: eSet = SFX_SYMBOLS_SIZE_AUTO;  break;
            case 1: eSet = SFX_SYMBOLS_SIZE_SMALL; break;
            case 2: eSet = SFX_SYMBOLS_SIZE_LARGE; break;
            case 3: eSet = SFX_SYMBOLS_SIZE_32; break;
            default:
                OSL_FAIL( "OfaViewTabPage::FillItemSet(): This state of m_xIconSizeLB should not be possible!" );
        }
        aMiscOptions.SetSymbolsSize( eSet );
    }

    const sal_Int32 nSidebarSizeLB_NewSelection = m_xSidebarIconSizeLB->get_active();
    if( nSidebarSizeLB_InitialSelection != nSidebarSizeLB_NewSelection )
    {
        // from now on it's modified, even if via auto setting the same size was set as now selected in the LB
        ToolBoxButtonSize eSet = ToolBoxButtonSize::DontCare;
        switch( nSidebarSizeLB_NewSelection )
        {
            case 0: eSet = ToolBoxButtonSize::DontCare;  break;
            case 1: eSet = ToolBoxButtonSize::Small; break;
            case 2: eSet = ToolBoxButtonSize::Large; break;
            default:
                OSL_FAIL( "OfaViewTabPage::FillItemSet(): This state of m_xSidebarIconSizeLB should not be possible!" );
        }
        aMiscOptions.SetSidebarIconSize( eSet );
    }

    const sal_Int32 nNotebookbarSizeLB_NewSelection = m_xNotebookbarIconSizeLB->get_active();
    if( nNotebookbarSizeLB_InitialSelection != nNotebookbarSizeLB_NewSelection )
    {
        // from now on it's modified, even if via auto setting the same size was set as now selected in the LB
        ToolBoxButtonSize eSet = ToolBoxButtonSize::DontCare;
        switch( nNotebookbarSizeLB_NewSelection )
        {
            case 0: eSet = ToolBoxButtonSize::DontCare;  break;
            case 1: eSet = ToolBoxButtonSize::Small; break;
            case 2: eSet = ToolBoxButtonSize::Large; break;
            default:
                OSL_FAIL( "OfaViewTabPage::FillItemSet(): This state of m_xNotebookbarIconSizeLB should not be possible!" );
        }
        aMiscOptions.SetNotebookbarIconSize( eSet );
    }

    const sal_Int32 nStyleLB_NewSelection = m_xIconStyleLB->get_active();
    if( nStyleLB_InitialSelection != nStyleLB_NewSelection )
    {
        aMiscOptions.SetIconTheme(m_xIconStyleLB->get_active_id());
        nStyleLB_InitialSelection = nStyleLB_NewSelection;
    }

    bool bAppearanceChanged = false;

    // Mouse Snap Mode
    SnapType eOldSnap = pAppearanceCfg->GetSnapMode();
    SnapType eNewSnap = static_cast<SnapType>(m_xMousePosLB->get_active());
    if(eNewSnap > SnapType::NONE)
        eNewSnap = SnapType::NONE;

    if ( eNewSnap != eOldSnap )
    {
        pAppearanceCfg->SetSnapMode(eNewSnap );
        bAppearanceChanged = true;
    }

    // Middle Mouse Button
    MouseMiddleButtonAction eOldMiddleMouse = pAppearanceCfg->GetMiddleMouseButton();
    short eNewMiddleMouse = m_xMouseMiddleLB->get_active();
    if(eNewMiddleMouse > 2)
        eNewMiddleMouse = 2;

    if ( eNewMiddleMouse != static_cast<short>(eOldMiddleMouse) )
    {
        pAppearanceCfg->SetMiddleMouseButton( static_cast<MouseMiddleButtonAction>(eNewMiddleMouse) );
        bAppearanceChanged = true;
    }

    if (m_xFontAntiAliasing->get_state_changed_from_saved())
    {
        pAppearanceCfg->SetFontAntiAliasing(m_xFontAntiAliasing->get_active());
        bAppearanceChanged = true;
    }

    if (m_xAAPointLimit->get_value_changed_from_saved())
    {
        pAppearanceCfg->SetFontAntialiasingMinPixelHeight(m_xAAPointLimit->get_value(FieldUnit::PIXEL));
        bAppearanceChanged = true;
    }

    if (m_xFontShowCB->get_state_changed_from_saved())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Font::View::ShowFontBoxWYSIWYG::set(m_xFontShowCB->get_active(), batch);
        batch->commit();
        bModified = true;
    }

    if (m_xMenuIconsLB->get_value_changed_from_saved())
    {
        aMenuOpt.SetMenuIconsState(m_xMenuIconsLB->get_active() == 0 ?
            TRISTATE_INDET :
            static_cast<TriState>(m_xMenuIconsLB->get_active() - 1));
        bModified = true;
        bMenuOptModified = true;
        bAppearanceChanged = true;
    }

    if (m_xContextMenuShortcutsLB->get_value_changed_from_saved())
    {
        aMenuOpt.SetContextMenuShortcuts(m_xContextMenuShortcutsLB->get_active() == 0 ?
            TRISTATE_INDET :
            static_cast<TriState>(m_xContextMenuShortcutsLB->get_active() - 1));
        bModified = true;
        bMenuOptModified = true;
        bAppearanceChanged = true;
    }

    // #i95644#  if disabled, do not use value, see in ::Reset()
    if (m_xUseHardwareAccell->get_sensitive())
    {
        if(m_xUseHardwareAccell->get_state_changed_from_saved())
        {
            pCanvasSettings->EnabledHardwareAcceleration(m_xUseHardwareAccell->get_active());
            bModified = true;
        }
    }

    // #i95644#  if disabled, do not use value, see in ::Reset()
    if (m_xUseAntiAliase->get_sensitive())
    {
        if (m_xUseAntiAliase->get_active() != mpDrawinglayerOpt->IsAntiAliasing())
        {
            mpDrawinglayerOpt->SetAntiAliasing(m_xUseAntiAliase->get_active());
            bModified = true;
            bRepaintWindows = true;
        }
    }

    if (m_xUseSkia->get_state_changed_from_saved() ||
        m_xForceSkiaRaster->get_state_changed_from_saved())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::VCL::UseSkia::set(m_xUseSkia->get_active(), batch);
        officecfg::Office::Common::VCL::ForceSkiaRaster::set(m_xForceSkiaRaster->get_active(), batch);
        batch->commit();
        bModified = true;
    }

    if( bMenuOptModified )
    {
        // Set changed settings to the application instance
        AllSettings aAllSettings = Application::GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        aAllSettings.SetStyleSettings(aStyleSettings);
        Application::MergeSystemSettings( aAllSettings );
        Application::SetSettings(aAllSettings);
    }

    if ( bAppearanceChanged )
    {
        pAppearanceCfg->Commit();
        pAppearanceCfg->SetApplicationDefaults ( GetpApp() );
    }

    if(bRepaintWindows)
    {
        vcl::Window* pAppWindow = Application::GetFirstTopLevelWindow();

        while(pAppWindow)
        {
            pAppWindow->Invalidate();
            pAppWindow = Application::GetNextTopLevelWindow(pAppWindow);
        }
    }

    if (m_xUseSkia->get_state_changed_from_saved() ||
        m_xForceSkiaRaster->get_state_changed_from_saved())
    {
        SolarMutexGuard aGuard;
        if( svtools::executeRestartDialog(
                comphelper::getProcessComponentContext(), nullptr,
                svtools::RESTART_REASON_SKIA))
            GetDialogController()->response(RET_OK);
    }

    return bModified;
}

void OfaViewTabPage::Reset( const SfxItemSet* )
{
    SvtMiscOptions aMiscOptions;

    if (aMiscOptions.GetSymbolsSize() != SFX_SYMBOLS_SIZE_AUTO)
    {
        nSizeLB_InitialSelection = 1;

        if (aMiscOptions.GetSymbolsSize() == SFX_SYMBOLS_SIZE_LARGE)
            nSizeLB_InitialSelection = 2;
        else if (aMiscOptions.GetSymbolsSize() == SFX_SYMBOLS_SIZE_32)
            nSizeLB_InitialSelection = 3;
    }
    m_xIconSizeLB->set_active( nSizeLB_InitialSelection );
    m_xIconSizeLB->save_value();

    if( aMiscOptions.GetSidebarIconSize() == ToolBoxButtonSize::DontCare )
        ; // do nothing
    else if( aMiscOptions.GetSidebarIconSize() == ToolBoxButtonSize::Small )
        nSidebarSizeLB_InitialSelection = 1;
    else if( aMiscOptions.GetSidebarIconSize() == ToolBoxButtonSize::Large )
        nSidebarSizeLB_InitialSelection = 2;
    m_xSidebarIconSizeLB->set_active( nSidebarSizeLB_InitialSelection );
    m_xSidebarIconSizeLB->save_value();
    if( aMiscOptions.GetNotebookbarIconSize() == ToolBoxButtonSize::DontCare )
        ; // do nothing
    else if( aMiscOptions.GetNotebookbarIconSize() == ToolBoxButtonSize::Small )
        nNotebookbarSizeLB_InitialSelection = 1;
    else if( aMiscOptions.GetNotebookbarIconSize() == ToolBoxButtonSize::Large )
        nNotebookbarSizeLB_InitialSelection = 2;
    m_xNotebookbarIconSizeLB->set_active(nNotebookbarSizeLB_InitialSelection);
    m_xNotebookbarIconSizeLB->save_value();

    if (aMiscOptions.IconThemeWasSetAutomatically()) {
        nStyleLB_InitialSelection = 0;
    }
    else {
        const OUString& selected = aMiscOptions.GetIconTheme();
        const vcl::IconThemeInfo& selectedInfo =
                vcl::IconThemeInfo::FindIconThemeById(mInstalledIconThemes, selected);
        nStyleLB_InitialSelection = m_xIconStyleLB->find_text(selectedInfo.GetDisplayName());
    }

    m_xIconStyleLB->set_active(nStyleLB_InitialSelection);
    m_xIconStyleLB->save_value();

    // Mouse Snap
    m_xMousePosLB->set_active(static_cast<sal_Int32>(pAppearanceCfg->GetSnapMode()));
    m_xMousePosLB->save_value();

    // Mouse Snap
    m_xMouseMiddleLB->set_active(static_cast<short>(pAppearanceCfg->GetMiddleMouseButton()));
    m_xMouseMiddleLB->save_value();

    m_xFontAntiAliasing->set_active( pAppearanceCfg->IsFontAntiAliasing() );
    m_xAAPointLimit->set_value(pAppearanceCfg->GetFontAntialiasingMinPixelHeight(), FieldUnit::PIXEL);

    // WorkingSet
    m_xFontShowCB->set_active(officecfg::Office::Common::Font::View::ShowFontBoxWYSIWYG::get());
    SvtMenuOptions aMenuOpt;
    m_xMenuIconsLB->set_active(aMenuOpt.GetMenuIconsState() == 2 ? 0 : aMenuOpt.GetMenuIconsState() + 1);
    m_xMenuIconsLB->save_value();

    TriState eContextMenuShortcuts = aMenuOpt.GetContextMenuShortcuts();
    bool bContextMenuShortcutsNonDefault = eContextMenuShortcuts == TRISTATE_FALSE || eContextMenuShortcuts == TRISTATE_TRUE;
    m_xContextMenuShortcutsLB->set_active(bContextMenuShortcutsNonDefault ? eContextMenuShortcuts + 1 : 0);
    m_xContextMenuShortcutsLB->save_value();

    UpdateHardwareAccelStatus();
    m_xUseHardwareAccell->save_state();

    { // #i95644# AntiAliasing
        if(mpDrawinglayerOpt->IsAAPossibleOnThisSystem())
        {
            m_xUseAntiAliase->set_active(mpDrawinglayerOpt->IsAntiAliasing());
        }
        else
        {
            m_xUseAntiAliase->set_active(false);
            m_xUseAntiAliase->set_sensitive(false);
        }

        m_xUseAntiAliase->save_state();
    }

    m_xUseSkia->set_active(officecfg::Office::Common::VCL::UseSkia::get());
    m_xForceSkiaRaster->set_active(officecfg::Office::Common::VCL::ForceSkiaRaster::get());
    m_xUseSkia->save_state();
    m_xForceSkiaRaster->save_state();

    m_xFontAntiAliasing->save_state();
    m_xAAPointLimit->save_value();
    m_xFontShowCB->save_state();

    OnAntialiasingToggled(*m_xFontAntiAliasing);
    UpdateSkiaStatus();
}

void OfaViewTabPage::UpdateHardwareAccelStatus()
{
    // #i95644# HW accel (unified to disable mechanism)
    if(pCanvasSettings->IsHardwareAccelerationAvailable())
    {
        m_xUseHardwareAccell->set_active(pCanvasSettings->IsHardwareAccelerationEnabled());
        m_xUseHardwareAccell->set_sensitive(!pCanvasSettings->IsHardwareAccelerationRO());
    }
    else
    {
        m_xUseHardwareAccell->set_active(false);
        m_xUseHardwareAccell->set_sensitive(false);
    }
#if HAVE_FEATURE_SKIA
    m_xUseHardwareAccell->set_sensitive(!m_xUseSkia->get_active());
#endif
}

struct LanguageConfig_Impl
{
    SvtLanguageOptions aLanguageOptions;
    SvtSysLocaleOptions aSysLocaleOptions;
    SvtLinguConfig aLinguConfig;
};

static bool bLanguageCurrentDoc_Impl = false;

// some things we'll need...
const OUStringLiteral sAccessSrvc = u"com.sun.star.configuration.ConfigurationAccess";
const OUStringLiteral sAccessUpdSrvc = u"com.sun.star.configuration.ConfigurationUpdateAccess";
const OUStringLiteral sInstalledLocalesPath = u"org.openoffice.Setup/Office/InstalledLocales";
const OUStringLiteral sUserLocalePath = u"org.openoffice.Office.Linguistic/General";
const OUStringLiteral sUserLocaleKey = u"UILocale";
static Sequence< OUString > seqInstalledLanguages;

static OUString lcl_getDatePatternsConfigString( const LocaleDataWrapper& rLocaleWrapper )
{
    Sequence< OUString > aDateAcceptancePatterns = rLocaleWrapper.getDateAcceptancePatterns();
    sal_Int32 nPatterns = aDateAcceptancePatterns.getLength();
    OUStringBuffer aBuf( nPatterns * 6 );   // 6 := length of Y-M-D;
    SAL_WARN_IF( !nPatterns, "cui.options", "No date acceptance pattern");
    if (nPatterns)
    {
        const OUString* pPatterns = aDateAcceptancePatterns.getConstArray();
        aBuf.append( pPatterns[0]);
        for (sal_Int32 i=1; i < nPatterns; ++i)
            aBuf.append(';').append( pPatterns[i]);
    }
    return aBuf.makeStringAndClear();
}

namespace
{
    //what ui language will be selected by default if the user override of General::UILocale is unset ?
    LanguageTag GetInstalledLocaleForSystemUILanguage()
    {
        css::uno::Sequence<OUString> inst(officecfg::Setup::Office::InstalledLocales::get()->getElementNames());
        return LanguageTag(getInstalledLocaleForSystemUILanguage(inst, false)).makeFallback();
    }
}

OfaLanguagesTabPage::OfaLanguagesTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optlanguagespage.ui", "OptLanguagesPage", &rSet)
    , pLangConfig(new LanguageConfig_Impl)
    , m_bDatePatternsValid(false)
    , m_xUserInterfaceLB(m_xBuilder->weld_combo_box("userinterface"))
    , m_xLocaleSettingFT(m_xBuilder->weld_label("localesettingFT"))
    , m_xLocaleSettingLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("localesetting")))
    , m_xDecimalSeparatorCB(m_xBuilder->weld_check_button("decimalseparator"))
    , m_xCurrencyFT(m_xBuilder->weld_label("defaultcurrency"))
    , m_xCurrencyLB(m_xBuilder->weld_combo_box("currencylb"))
    , m_xDatePatternsFT(m_xBuilder->weld_label("dataaccpatterns"))
    , m_xDatePatternsED(m_xBuilder->weld_entry("datepatterns"))
    , m_xWesternLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("westernlanguage")))
    , m_xWesternLanguageFT(m_xBuilder->weld_label("western"))
    , m_xAsianLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("asianlanguage")))
    , m_xComplexLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("complexlanguage")))
    , m_xCurrentDocCB(m_xBuilder->weld_check_button("currentdoc"))
    , m_xAsianSupportCB(m_xBuilder->weld_check_button("asiansupport"))
    , m_xCTLSupportCB(m_xBuilder->weld_check_button("ctlsupport"))
    , m_xIgnoreLanguageChangeCB(m_xBuilder->weld_check_button("ignorelanguagechange"))
{
    // tdf#125483 save original default label
    m_sDecimalSeparatorLabel = m_xDecimalSeparatorCB->get_label();

    // initialize user interface language selection
    m_sSystemDefaultString = SvtLanguageTable::GetLanguageString( LANGUAGE_SYSTEM );

    OUString aUILang = m_sSystemDefaultString +
                       " - " +
                       SvtLanguageTable::GetLanguageString(GetInstalledLocaleForSystemUILanguage().getLanguageType());

    m_xUserInterfaceLB->append("0", aUILang);
    m_xUserInterfaceLB->append_separator("");
    try
    {
        Reference< XMultiServiceFactory > theConfigProvider(
            css::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));
        Sequence< Any > theArgs(1);
        Reference< XNameAccess > theNameAccess;

        // find out which locales are currently installed and add them to the listbox
        theArgs[0] <<= NamedValue("nodepath", Any(OUString(sInstalledLocalesPath)));
        theNameAccess.set(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs ), UNO_QUERY_THROW );
        seqInstalledLanguages = theNameAccess->getElementNames();
        LanguageType aLang = LANGUAGE_DONTKNOW;
        std::vector< std::pair<sal_Int32, OUString> > aUILanguages;
        for (sal_Int32 i=0; i<seqInstalledLanguages.getLength(); i++)
        {
            aLang = LanguageTag::convertToLanguageTypeWithFallback(seqInstalledLanguages[i]);
            if (aLang != LANGUAGE_DONTKNOW)
            {
                OUString aLangStr( SvtLanguageTable::GetLanguageString( aLang ) );
                aUILanguages.emplace_back(i+1, aLangStr);
            }
        }

        std::sort(aUILanguages.begin(), aUILanguages.end(), [](const auto& l1, const auto& l2) {
            static const auto aSorter = comphelper::string::NaturalStringSorter(
                comphelper::getProcessComponentContext(),
                Application::GetSettings().GetLanguageTag().getLocale());
            return aSorter.compare(l1.second, l2.second) < 0;
        });

        // tdf#114694: append the sorted list after the default entry and separator.
        for (const auto & [ nGroupID, sGroupName ] : aUILanguages)
        {
            m_xUserInterfaceLB->append(OUString::number(nGroupID), sGroupName);
        }

        m_xUserInterfaceLB->set_active(0);

        // find out whether the user has a specific locale specified
        Sequence< Any > theArgs2(1);
        theArgs2[0] <<= NamedValue("nodepath", Any(OUString(sUserLocalePath)));
        theNameAccess.set(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs2 ), UNO_QUERY_THROW );
        if (theNameAccess->hasByName(sUserLocaleKey))
            theNameAccess->getByName(sUserLocaleKey) >>= m_sUserLocaleValue;
        // select the user specified locale in the listbox
        if (!m_sUserLocaleValue.isEmpty())
        {
            for (sal_Int32 i = 0, nEntryCount = m_xUserInterfaceLB->get_count(); i < nEntryCount; ++i)
            {
                sal_Int32 d = m_xUserInterfaceLB->get_id(i).toInt32();
                if ( d > 0 && seqInstalledLanguages.getLength() > d-1 && seqInstalledLanguages[d-1] == m_sUserLocaleValue)
                    m_xUserInterfaceLB->set_active(i);
            }
        }

    }
    catch (const Exception &)
    {
        // we'll just leave the box in its default setting and won't
        // even give it event handler...
        TOOLS_WARN_EXCEPTION("cui.options", "ignoring" );
    }

    m_xWesternLanguageLB->SetLanguageList(
        SvxLanguageListFlags::WESTERN | SvxLanguageListFlags::ONLY_KNOWN, true, false, true, true,
        LANGUAGE_SYSTEM, css::i18n::ScriptType::LATIN);

    m_xAsianLanguageLB->SetLanguageList(
        SvxLanguageListFlags::CJK | SvxLanguageListFlags::ONLY_KNOWN, true, false, true, true,
        LANGUAGE_SYSTEM, css::i18n::ScriptType::ASIAN);

    m_xComplexLanguageLB->SetLanguageList(
        SvxLanguageListFlags::CTL | SvxLanguageListFlags::ONLY_KNOWN, true, false, true, true,
        LANGUAGE_SYSTEM, css::i18n::ScriptType::COMPLEX);

    m_xLocaleSettingLB->SetLanguageList(
        SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false, false, false, true,
        LANGUAGE_USER_SYSTEM_CONFIG, css::i18n::ScriptType::WEAK);

    const NfCurrencyTable& rCurrTab = SvNumberFormatter::GetTheCurrencyTable();
    const NfCurrencyEntry& rCurr = SvNumberFormatter::GetCurrencyEntry( LANGUAGE_SYSTEM );
    // insert SYSTEM entry
    OUString aDefaultCurr = m_sSystemDefaultString + " - " + rCurr.GetBankSymbol();
    m_xCurrencyLB->append("default", aDefaultCurr);
    m_xCurrencyLB->append_separator("");

    assert(m_xCurrencyLB->find_id("default") != -1);
    // all currencies
    OUString aTwoSpace( "  " );
    sal_uInt16 nCurrCount = rCurrTab.size();
    std::vector< const NfCurrencyEntry* > aCurrencies;
    // first entry is SYSTEM, skip it
    for ( sal_uInt16 j=1; j < nCurrCount; ++j )
    {
        aCurrencies.push_back(&rCurrTab[j]);
    }
    std::sort(aCurrencies.begin(), aCurrencies.end(),
              [](const NfCurrencyEntry* c1, const NfCurrencyEntry* c2) {
                  return c1->GetBankSymbol().compareTo(c2->GetBankSymbol()) < 0;
              });

    for (auto &v : aCurrencies)
    {
        OUString aStr_ = v->GetBankSymbol() +
                         aTwoSpace +
                         v->GetSymbol();
        aStr_ = ApplyLreOrRleEmbedding( aStr_ ) +
                aTwoSpace +
                ApplyLreOrRleEmbedding( SvtLanguageTable::GetLanguageString( v->GetLanguage() ) );
        m_xCurrencyLB->append(OUString::number(reinterpret_cast<sal_Int64>(v)), aStr_);
    }

    m_xCurrencyLB->set_active(0);

    m_xLocaleSettingLB->connect_changed( LINK( this, OfaLanguagesTabPage, LocaleSettingHdl ) );
    m_xDatePatternsED->connect_changed( LINK( this, OfaLanguagesTabPage, DatePatternsHdl ) );

    Link<weld::ToggleButton&,void> aLink( LINK( this, OfaLanguagesTabPage, SupportHdl ) );
    m_xAsianSupportCB->connect_toggled( aLink );
    m_xCTLSupportCB->connect_toggled( aLink );

    m_bOldAsian = pLangConfig->aLanguageOptions.IsAnyEnabled();
    m_xAsianSupportCB->set_active(m_bOldAsian);
    m_xAsianSupportCB->save_state();
    bool bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK);
    m_xAsianSupportCB->set_sensitive(!bReadonly);
    SupportHdl(*m_xAsianSupportCB);

    m_bOldCtl = pLangConfig->aLanguageOptions.IsCTLFontEnabled();
    m_xCTLSupportCB->set_active(m_bOldCtl);
    m_xCTLSupportCB->save_state();
    bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT);
    m_xCTLSupportCB->set_sensitive(!bReadonly);
    SupportHdl(*m_xCTLSupportCB);

    m_xIgnoreLanguageChangeCB->set_active( pLangConfig->aSysLocaleOptions.IsIgnoreLanguageChange() );
}

OfaLanguagesTabPage::~OfaLanguagesTabPage()
{
}

std::unique_ptr<SfxTabPage> OfaLanguagesTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaLanguagesTabPage>(pPage, pController, *rAttrSet);
}

static void lcl_Update(std::unique_ptr<SfxVoidItem> pInvalidItems[], std::unique_ptr<SfxBoolItem> pBoolItems[], sal_uInt16 nCount)
{
    SfxViewFrame* pCurrentFrm = SfxViewFrame::Current();
    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst();
    while(pViewFrm)
    {
        SfxBindings& rBind = pViewFrm->GetBindings();
        for(sal_uInt16 i = 0; i < nCount; i++)
        {
            if(pCurrentFrm == pViewFrm)
                rBind.InvalidateAll(false);
            rBind.SetState( *pInvalidItems[i] );
            rBind.SetState( *pBoolItems[i] );
        }
        pViewFrm = SfxViewFrame::GetNext(*pViewFrm);
    }
}

bool OfaLanguagesTabPage::FillItemSet( SfxItemSet* rSet )
{
    // lock configuration broadcasters so that we can coordinate the notifications
    pLangConfig->aSysLocaleOptions.BlockBroadcasts( true );
    pLangConfig->aLanguageOptions.BlockBroadcasts( true );
    pLangConfig->aLinguConfig.BlockBroadcasts( true );

    /*
     * Sequence checking only matters when CTL support is enabled.
     *
     * So we only need to check for sequence checking if
     * a) previously it was unchecked and is now checked or
     * b) it was already checked but the CTL language has changed
     */
    if (
         m_xCTLSupportCB->get_active() &&
         (m_xCTLSupportCB->get_saved_state() != TRISTATE_TRUE ||
         m_xComplexLanguageLB->get_active_id_changed_from_saved())
       )
    {
        //sequence checking has to be switched on depending on the selected CTL language
        LanguageType eCTLLang = m_xComplexLanguageLB->get_active_id();
        bool bOn = MsLangId::needsSequenceChecking( eCTLLang);
        pLangConfig->aLanguageOptions.SetCTLSequenceCheckingRestricted(bOn);
        pLangConfig->aLanguageOptions.SetCTLSequenceChecking(bOn);
        pLangConfig->aLanguageOptions.SetCTLSequenceCheckingTypeAndReplace(bOn);
    }
    try
    {
        // handle settings for UI Language
        // a change of setting needs to bring up a warning message
        OUString aLangString;
        sal_Int32 d = m_xUserInterfaceLB->get_active_id().toInt32();
        if( d > 0 && seqInstalledLanguages.getLength() > d-1)
            aLangString = seqInstalledLanguages[d-1];

        /*
        if( m_xUserInterfaceLB->GetSelectedEntryPos() > 0)
            aLangString = ConvertLanguageToIsoString(m_xUserInterfaceLB->get_active_id());
        */
        Reference< XMultiServiceFactory > theConfigProvider(
            css::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));
        Sequence< Any > theArgs(1);
        theArgs[0] <<= NamedValue("nodepath", Any(OUString(sUserLocalePath)));
        Reference< XPropertySet >xProp(
            theConfigProvider->createInstanceWithArguments(sAccessUpdSrvc, theArgs ), UNO_QUERY_THROW );
        if ( m_sUserLocaleValue != aLangString)
        {
            // OSL_FAIL("UserInterface language was changed, restart.");
            // write new value
            xProp->setPropertyValue(sUserLocaleKey, Any(aLangString));
            Reference< XChangesBatch >(xProp, UNO_QUERY_THROW)->commitChanges();
            // display info
            SolarMutexGuard aGuard;
            if (svtools::executeRestartDialog(
                    comphelper::getProcessComponentContext(), GetFrameWeld(),
                    svtools::RESTART_REASON_LANGUAGE_CHANGE))
                GetDialogController()->response(RET_OK);

            // tell quickstarter to stop being a veto listener

            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext());
            css::office::Quickstart::createAndSetVeto(xContext, false, false, false/*DisableVeto*/);
        }
    }
    catch (const Exception&)
    {
        // we'll just leave the box in its default setting and won't
        // even give it event handler...
        TOOLS_WARN_EXCEPTION("cui.options", "ignoring");
    }

    LanguageTag aLanguageTag( pLangConfig->aSysLocaleOptions.GetLanguageTag());
    LanguageType eOldLocale = (aLanguageTag.isSystemLocale() ? LANGUAGE_SYSTEM :
            aLanguageTag.makeFallback().getLanguageType());
    LanguageType eNewLocale = m_xLocaleSettingLB->get_active_id();

    // If the "Default ..." entry was selected that means SYSTEM, the actual
    // eNewLocale value is temporary for the dialog only, do not resolve to
    // what system currently is.
    if (eNewLocale == LANGUAGE_USER_SYSTEM_CONFIG)
        eNewLocale = LANGUAGE_SYSTEM;

    if ( eOldLocale != eNewLocale )
    {
        // an empty string denotes SYSTEM locale
        OUString sNewLang;
        if ( eNewLocale != LANGUAGE_SYSTEM )
            sNewLang = LanguageTag::convertToBcp47( eNewLocale);

        // locale nowadays get to AppSettings via notification
        // this will happen after releasing the lock on the ConfigurationBroadcaster at
        // the end of this method
        pLangConfig->aSysLocaleOptions.SetLocaleConfigString( sNewLang );
        rSet->Put( SfxBoolItem( SID_OPT_LOCALE_CHANGED, true ) );

        SvtScriptType nNewType = SvtLanguageOptions::GetScriptTypeOfLanguage( eNewLocale );
        bool bNewCJK = bool( nNewType & SvtScriptType::ASIAN );
        SvtCompatibilityOptions aCompatOpts;
        aCompatOpts.SetDefault( SvtCompatibilityEntry::Index::ExpandWordSpace, !bNewCJK );
    }

    if(m_xDecimalSeparatorCB->get_state_changed_from_saved())
        pLangConfig->aSysLocaleOptions.SetDecimalSeparatorAsLocale(m_xDecimalSeparatorCB->get_active());

    if(m_xIgnoreLanguageChangeCB->get_state_changed_from_saved())
        pLangConfig->aSysLocaleOptions.SetIgnoreLanguageChange(m_xIgnoreLanguageChangeCB->get_active());

    // Configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default.
    OUString sOldCurr = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    OUString sId = m_xCurrencyLB->get_active_id();
    const NfCurrencyEntry* pCurr = sId == "default" ? nullptr : reinterpret_cast<const NfCurrencyEntry*>(sId.toInt64());
    OUString sNewCurr;
    if ( pCurr )
        sNewCurr = SvtSysLocaleOptions::CreateCurrencyConfigString(
            pCurr->GetBankSymbol(), pCurr->GetLanguage() );
    if ( sOldCurr != sNewCurr )
        pLangConfig->aSysLocaleOptions.SetCurrencyConfigString( sNewCurr );

    // Configured date acceptance patterns, for example Y-M-D;M-D or empty for
    // locale default.
    if (m_bDatePatternsValid && m_xDatePatternsED->get_value_changed_from_saved())
        pLangConfig->aSysLocaleOptions.SetDatePatternsConfigString( m_xDatePatternsED->get_text());

    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    Reference< css::linguistic2::XLinguProperties > xLinguProp = LinguMgr::GetLinguPropertySet();
    bool bCurrentDocCBChecked = m_xCurrentDocCB->get_active();
    if (m_xCurrentDocCB->get_sensitive())
        bLanguageCurrentDoc_Impl = bCurrentDocCBChecked;
    bool bCurrentDocCBChanged = m_xCurrentDocCB->get_state_changed_from_saved();

    bool bValChanged = m_xWesternLanguageLB->get_active_id_changed_from_saved();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_xWesternLanguageLB->get_active_id();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag::convertToLocale( eSelectLang, false);
            aValue <<= aLocale;
            pLangConfig->aLinguConfig.SetProperty( "DefaultLocale", aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet->Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, css::i18n::ScriptType::LATIN),
                SID_ATTR_LANGUAGE));
        }
    }
    bValChanged = m_xAsianLanguageLB->get_active_id_changed_from_saved();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_xAsianLanguageLB->get_active_id();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag::convertToLocale( eSelectLang, false);
            aValue <<= aLocale;
            pLangConfig->aLinguConfig.SetProperty( "DefaultLocale_CJK", aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale_CJK( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet->Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, css::i18n::ScriptType::ASIAN),
                SID_ATTR_CHAR_CJK_LANGUAGE));
        }
    }
    bValChanged = m_xComplexLanguageLB->get_active_id_changed_from_saved();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_xComplexLanguageLB->get_active_id();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag::convertToLocale( eSelectLang, false);
            aValue <<= aLocale;
            pLangConfig->aLinguConfig.SetProperty( "DefaultLocale_CTL", aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale_CTL( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet->Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, css::i18n::ScriptType::COMPLEX),
                SID_ATTR_CHAR_CTL_LANGUAGE));
        }
    }

    if(m_xAsianSupportCB->get_state_changed_from_saved() )
    {
        bool bChecked = m_xAsianSupportCB->get_active();
        pLangConfig->aLanguageOptions.SetAll(bChecked);

        //iterate over all bindings to invalidate vertical text direction
        const sal_uInt16 STATE_COUNT = 2;

        std::unique_ptr<SfxBoolItem> pBoolItems[STATE_COUNT];
        pBoolItems[0].reset(new SfxBoolItem(SID_VERTICALTEXT_STATE, false));
        pBoolItems[1].reset(new SfxBoolItem(SID_TEXT_FITTOSIZE_VERTICAL, false));

        std::unique_ptr<SfxVoidItem> pInvalidItems[STATE_COUNT];
        pInvalidItems[0].reset(new SfxVoidItem(SID_VERTICALTEXT_STATE));
        pInvalidItems[1].reset(new SfxVoidItem(SID_TEXT_FITTOSIZE_VERTICAL));

        lcl_Update(pInvalidItems, pBoolItems, STATE_COUNT);
    }

    if ( m_xCTLSupportCB->get_state_changed_from_saved() )
    {
        SvtSearchOptions aOpt;
        aOpt.SetIgnoreDiacritics_CTL(true);
        aOpt.SetIgnoreKashida_CTL(true);
        aOpt.Commit();
        pLangConfig->aLanguageOptions.SetCTLFontEnabled( m_xCTLSupportCB->get_active() );

        const sal_uInt16 STATE_COUNT = 1;
        std::unique_ptr<SfxBoolItem> pBoolItems[STATE_COUNT];
        pBoolItems[0].reset(new SfxBoolItem(SID_CTLFONT_STATE, false));
        std::unique_ptr<SfxVoidItem> pInvalidItems[STATE_COUNT];
        pInvalidItems[0].reset(new SfxVoidItem(SID_CTLFONT_STATE));
        lcl_Update(pInvalidItems, pBoolItems, STATE_COUNT);
    }

    if ( pLangConfig->aSysLocaleOptions.IsModified() )
        pLangConfig->aSysLocaleOptions.Commit();

    // first release the lock on the ConfigurationBroadcaster for Locale changes
    // it seems that our code relies on the fact that before other changes like e.g. currency
    // are broadcasted locale changes have been done
    pLangConfig->aSysLocaleOptions.BlockBroadcasts( false );
    pLangConfig->aLanguageOptions.BlockBroadcasts( false );
    pLangConfig->aLinguConfig.BlockBroadcasts( false );

    return false;
}

void OfaLanguagesTabPage::Reset( const SfxItemSet* rSet )
{
    LanguageTag aLanguageTag( pLangConfig->aSysLocaleOptions.GetLanguageTag());
    if ( aLanguageTag.isSystemLocale() )
        m_xLocaleSettingLB->set_active_id( LANGUAGE_USER_SYSTEM_CONFIG );
    else
        m_xLocaleSettingLB->set_active_id( aLanguageTag.makeFallback().getLanguageType());
    bool bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::EOption::Locale);
    m_xLocaleSettingLB->set_sensitive(!bReadonly);
    m_xLocaleSettingFT->set_sensitive(!bReadonly);


    m_xDecimalSeparatorCB->set_active( pLangConfig->aSysLocaleOptions.IsDecimalSeparatorAsLocale());
    m_xDecimalSeparatorCB->save_state();

    m_xIgnoreLanguageChangeCB->set_active( pLangConfig->aSysLocaleOptions.IsIgnoreLanguageChange());
    m_xIgnoreLanguageChangeCB->save_state();

    // let LocaleSettingHdl enable/disable checkboxes for CJK/CTL support
    // #i15812# must be done *before* the configured currency is set
    // and update the decimal separator used for the given locale
    LocaleSettingHdl(*m_xLocaleSettingLB->get_widget());

    // configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default
    const NfCurrencyEntry* pCurr = nullptr;
    OUString sCurrency = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    if ( !sCurrency.isEmpty() )
    {
        LanguageType eLang;
        OUString aAbbrev;
        SvtSysLocaleOptions::GetCurrencyAbbrevAndLanguage( aAbbrev, eLang, sCurrency );
        pCurr = SvNumberFormatter::GetCurrencyEntry( aAbbrev, eLang );
    }
    // if pCurr==nullptr the SYSTEM entry is selected
    OUString sId = !pCurr ? OUString("default") : OUString::number(reinterpret_cast<sal_Int64>(pCurr));
    m_xCurrencyLB->set_active_id(sId);
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::EOption::Currency);
    m_xCurrencyLB->set_sensitive(!bReadonly);
    m_xCurrencyFT->set_sensitive(!bReadonly);

    // date acceptance patterns
    OUString aDatePatternsString = pLangConfig->aSysLocaleOptions.GetDatePatternsConfigString();
    if (aDatePatternsString.isEmpty())
    {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        aDatePatternsString = lcl_getDatePatternsConfigString( rLocaleWrapper);
    }
    // Let's assume patterns are valid at this point.
    m_bDatePatternsValid = true;
    m_xDatePatternsED->set_text(aDatePatternsString);
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::EOption::DatePatterns);
    m_xDatePatternsED->set_sensitive(!bReadonly);
    m_xDatePatternsFT->set_sensitive(!bReadonly);
    m_xDatePatternsED->save_value();

    //western/CJK/CLK language
    LanguageType eCurLang = LANGUAGE_NONE;
    LanguageType eCurLangCJK = LANGUAGE_NONE;
    LanguageType eCurLangCTL = LANGUAGE_NONE;
    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    //collect the configuration values first
    m_xCurrentDocCB->set_sensitive(false);

    Any aWestLang;
    Any aCJKLang;
    Any aCTLLang;
    try
    {
        aWestLang = pLangConfig->aLinguConfig.GetProperty("DefaultLocale");
        Locale aLocale;
        aWestLang >>= aLocale;

        eCurLang = LanguageTag::convertToLanguageType( aLocale, false);

        aCJKLang = pLangConfig->aLinguConfig.GetProperty("DefaultLocale_CJK");
        aLocale = Locale();
        aCJKLang >>= aLocale;
        eCurLangCJK = LanguageTag::convertToLanguageType( aLocale, false);

        aCTLLang = pLangConfig->aLinguConfig.GetProperty("DefaultLocale_CTL");
        aLocale = Locale();
        aCTLLang >>= aLocale;
        eCurLangCTL = LanguageTag::convertToLanguageType( aLocale, false);
    }
    catch (const Exception&)
    {
    }
    //overwrite them by the values provided by the DocShell
    if(pCurrentDocShell)
    {
        m_xCurrentDocCB->set_sensitive(true);
        m_xCurrentDocCB->set_active(bLanguageCurrentDoc_Impl);
        const SfxPoolItem* pLang;
        if( SfxItemState::SET == rSet->GetItemState(SID_ATTR_LANGUAGE, false, &pLang))
        {
            LanguageType eTempCurLang = static_cast<const SvxLanguageItem*>(pLang)->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLang, css::i18n::ScriptType::LATIN) != eTempCurLang)
                eCurLang = eTempCurLang;
        }

        if( SfxItemState::SET == rSet->GetItemState(SID_ATTR_CHAR_CJK_LANGUAGE, false, &pLang))
        {
            LanguageType eTempCurLang = static_cast<const SvxLanguageItem*>(pLang)->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLangCJK, css::i18n::ScriptType::ASIAN) != eTempCurLang)
                eCurLangCJK = eTempCurLang;
        }

        if( SfxItemState::SET == rSet->GetItemState(SID_ATTR_CHAR_CTL_LANGUAGE, false, &pLang))
        {
            LanguageType eTempCurLang = static_cast<const SvxLanguageItem*>(pLang)->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLangCTL, css::i18n::ScriptType::COMPLEX) != eTempCurLang)
                eCurLangCTL = eTempCurLang;
        }
    }
    if(LANGUAGE_NONE == eCurLang || LANGUAGE_DONTKNOW == eCurLang)
        m_xWesternLanguageLB->set_active_id(LANGUAGE_NONE);
    else
        m_xWesternLanguageLB->set_active_id(eCurLang);

    if(LANGUAGE_NONE == eCurLangCJK || LANGUAGE_DONTKNOW == eCurLangCJK)
        m_xAsianLanguageLB->set_active_id(LANGUAGE_NONE);
    else
        m_xAsianLanguageLB->set_active_id(eCurLangCJK);

    if(LANGUAGE_NONE == eCurLangCTL || LANGUAGE_DONTKNOW == eCurLangCTL)
        m_xComplexLanguageLB->set_active_id(LANGUAGE_NONE);
    else
        m_xComplexLanguageLB->set_active_id(eCurLangCTL);

    m_xWesternLanguageLB->save_active_id();
    m_xAsianLanguageLB->save_active_id();
    m_xComplexLanguageLB->save_active_id();
    m_xIgnoreLanguageChangeCB->save_state();
    m_xCurrentDocCB->save_state();

    bool bEnable = !pLangConfig->aLinguConfig.IsReadOnly( "DefaultLocale" );
    m_xWesternLanguageFT->set_sensitive( bEnable );
    m_xWesternLanguageLB->set_sensitive( bEnable );

    // check the box "For the current document only"
    // set the focus to the Western Language box
    const SfxPoolItem* pLang = nullptr;
    if ( SfxItemState::SET == rSet->GetItemState(SID_SET_DOCUMENT_LANGUAGE, false, &pLang ) && static_cast<const SfxBoolItem*>(pLang)->GetValue() )
    {
        m_xWesternLanguageLB->grab_focus();
        m_xCurrentDocCB->set_sensitive(true);
        m_xCurrentDocCB->set_active(true);
    }
}

IMPL_LINK(OfaLanguagesTabPage, SupportHdl, weld::ToggleButton&, rBox, void)
{
    bool bCheck = rBox.get_active();
    if ( m_xAsianSupportCB.get() == &rBox )
    {
        bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly("DefaultLocale_CJK");
        bCheck = ( bCheck && !bReadonly );
        m_xAsianLanguageLB->set_sensitive( bCheck );
        if (rBox.get_sensitive())
            m_bOldAsian = bCheck;
    }
    else if ( m_xCTLSupportCB.get() == &rBox )
    {
        bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly("DefaultLocale_CTL");
        bCheck = ( bCheck && !bReadonly  );
        m_xComplexLanguageLB->set_sensitive( bCheck );
        if (rBox.get_sensitive())
            m_bOldCtl = bCheck;
    }
    else
        SAL_WARN( "cui.options", "OfaLanguagesTabPage::SupportHdl(): wrong rBox" );
}

namespace
{
    void lcl_checkLanguageCheckBox(weld::CheckButton& _rCB, bool _bNewValue, bool _bOldValue)
    {
        if ( _bNewValue )
            _rCB.set_active(true);
        else
            _rCB.set_active( _bOldValue );
// #i15082# do not call save_state() in running dialog...
//      _rCB.save_state();
        _rCB.set_sensitive( !_bNewValue );
    }
}

IMPL_LINK_NOARG(OfaLanguagesTabPage, LocaleSettingHdl, weld::ComboBox&, void)
{
    LanguageType eLang = m_xLocaleSettingLB->get_active_id();
    SvtScriptType nType = SvtLanguageOptions::GetScriptTypeOfLanguage(eLang);
    // first check if CTL must be enabled
    // #103299# - if CTL font setting is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT))
    {
        bool bIsCTLFixed = bool(nType & SvtScriptType::COMPLEX);
        lcl_checkLanguageCheckBox(*m_xCTLSupportCB, bIsCTLFixed, m_bOldCtl);
        SupportHdl(*m_xCTLSupportCB);
    }
    // second check if CJK must be enabled
    // #103299# - if CJK support is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK))
    {
        bool bIsCJKFixed = bool(nType & SvtScriptType::ASIAN);
        lcl_checkLanguageCheckBox(*m_xAsianSupportCB, bIsCJKFixed, m_bOldAsian);
        SupportHdl(*m_xAsianSupportCB);
    }

    const NfCurrencyEntry& rCurr = SvNumberFormatter::GetCurrencyEntry(
            (eLang == LANGUAGE_USER_SYSTEM_CONFIG) ? MsLangId::getSystemLanguage() : eLang);
    const OUString aDefaultID = "default";
    // Update the "Default ..." currency.
    m_xCurrencyLB->remove_id(aDefaultID);
    OUString aDefaultCurr = m_sSystemDefaultString + " - " + rCurr.GetBankSymbol();
    m_xCurrencyLB->insert(0, aDefaultCurr, &aDefaultID, nullptr, nullptr);
    assert(m_xCurrencyLB->find_id(aDefaultID) != -1);
    m_xCurrencyLB->set_active_text(aDefaultCurr);

    // obtain corresponding locale data
    LanguageTag aLanguageTag( eLang);
    LocaleDataWrapper aLocaleWrapper( aLanguageTag );

    // update the decimal separator key of the related CheckBox
    OUString sTempLabel(m_sDecimalSeparatorLabel);
    sTempLabel = sTempLabel.replaceFirst("%1", aLocaleWrapper.getNumDecimalSep() );
    m_xDecimalSeparatorCB->set_label(sTempLabel);

    // update the date acceptance patterns
    OUString aDatePatternsString = lcl_getDatePatternsConfigString( aLocaleWrapper);
    m_bDatePatternsValid = true;
    m_xDatePatternsED->set_text( aDatePatternsString);
}

IMPL_LINK( OfaLanguagesTabPage, DatePatternsHdl, weld::Entry&, rEd, void )
{
    const OUString aPatterns(rEd.get_text());
    OUStringBuffer aBuf( aPatterns);
    sal_Int32 nChar = 0;
    bool bValid = true;
    bool bModified = false;
    if (!aPatterns.isEmpty())
    {
        for (sal_Int32 nIndex=0; nIndex >= 0 && bValid; ++nChar)
        {
            const OUString aPat( aPatterns.getToken( 0, ';', nIndex));
            if (aPat.isEmpty() && nIndex < 0)
            {
                // Indicating failure when about to append a pattern is too
                // confusing. Empty patterns are ignored anyway when sequencing
                // to SvtSysLocale.
                continue;   // for
            }
            else if (aPat.getLength() < 2)
                bValid = false;
            else
            {
                bool bY, bM, bD;
                bY = bM = bD = false;
                bool bSep = true;
                for (sal_Int32 i = 0; i < aPat.getLength() && bValid; /*nop*/)
                {
                    const sal_Int32 j = i;
                    const sal_uInt32 c = aPat.iterateCodePoints( &i);
                    // Only one Y,M,D per pattern, separated by any character(s).
                    switch (c)
                    {
                        case 'y':
                        case 'Y':
                            if (bY || !bSep)
                                bValid = false;
                            else if (c == 'y')
                            {
                                aBuf[nChar] = 'Y';
                                bModified = true;
                            }
                            bY = true;
                            bSep = false;
                            break;
                        case 'm':
                        case 'M':
                            if (bM || !bSep)
                                bValid = false;
                            else if (c == 'm')
                            {
                                aBuf[nChar] = 'M';
                                bModified = true;
                            }
                            bM = true;
                            bSep = false;
                            break;
                        case 'd':
                        case 'D':
                            if (bD || !bSep)
                                bValid = false;
                            else if (c == 'd')
                            {
                                aBuf[nChar] = 'D';
                                bModified = true;
                            }
                            bD = true;
                            bSep = false;
                            break;
                        default:
                            // A pattern must not start with a separator (but
                            // may end with).
                            if (!(bY || bM || bD))
                                bValid = false;
                            bSep = true;
                    }
                    nChar += i-j;
                }
                // At least one of Y,M,D
                bValid &= (bY || bM || bD);
            }
        }
    }
    if (bModified)
        rEd.replace_selection(aBuf.makeStringAndClear());
    if (bValid)
        rEd.set_message_type(weld::EntryMessageType::Normal);
    else
        rEd.set_message_type(weld::EntryMessageType::Error);
    m_bDatePatternsValid = bValid;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <svl/numformat.hxx>
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
#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <svtools/langtab.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>
#include <svl/voiditem.hxx>
#include <GraphicsTestsDialog.hxx>
#include <unotools/searchopt.hxx>
#include <sal/log.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/diagnose_ex.hxx>
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
#include <bitmaps.hlst>

#include "optgdlg.hxx"
#include <svtools/apearcfg.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <svtools/restartdialog.hxx>
#include <svtools/imgdef.hxx>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <vcl/unohelp2.hxx>

#if defined(_WIN32)
#include <systools/win32/winstoreutil.hxx>
#include <vcl/fileregistration.hxx>
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
    , m_xExtHelpImg(m_xBuilder->weld_widget("lockexthelp"))
    , m_xPopUpNoHelpCB(m_xBuilder->weld_check_button("popupnohelp"))
    , m_xPopUpNoHelpImg(m_xBuilder->weld_widget("lockpopupnohelp"))
    , m_xShowTipOfTheDay(m_xBuilder->weld_check_button("cbShowTipOfTheDay"))
    , m_xShowTipOfTheDayImg(m_xBuilder->weld_widget("lockcbShowTipOfTheDay"))
    , m_xFileDlgFrame(m_xBuilder->weld_widget("filedlgframe"))
    , m_xFileDlgROImage(m_xBuilder->weld_widget("lockimage"))
    , m_xFileDlgCB(m_xBuilder->weld_check_button("filedlg"))
    , m_xDocStatusCB(m_xBuilder->weld_check_button("docstatus"))
    , m_xDocStatusImg(m_xBuilder->weld_widget("lockdocstatus"))
    , m_xYearFrame(m_xBuilder->weld_widget("yearframe"))
    , m_xYearLabel(m_xBuilder->weld_label("yearslabel"))
    , m_xYearValueField(m_xBuilder->weld_spin_button("year"))
    , m_xToYearFT(m_xBuilder->weld_label("toyear"))
    , m_xYearFrameImg(m_xBuilder->weld_widget("lockyears"))
#if HAVE_FEATURE_BREAKPAD
    , m_xPrivacyFrame(m_xBuilder->weld_widget("privacyframe"))
    , m_xCrashReport(m_xBuilder->weld_check_button("crashreport"))
    , m_xCrashReportImg(m_xBuilder->weld_widget("lockcrashreport"))
#endif
#if defined(_WIN32)
    , m_xQuickStarterFrame(m_xBuilder->weld_widget("quickstarter"))
    , m_xQuickLaunchCB(m_xBuilder->weld_check_button("quicklaunch"))
    , m_xQuickLaunchImg(m_xBuilder->weld_widget("lockquicklaunch"))
    , m_xFileAssocFrame(m_xBuilder->weld_widget("fileassoc"))
    , m_xFileAssocBtn(m_xBuilder->weld_button("assocfiles"))
    , m_xPerformFileExtCheck(m_xBuilder->weld_check_button("cbPerformFileExtCheck"))
    , m_xPerformFileExtImg(m_xBuilder->weld_widget("lockcbPerformFileExtCheck"))
#endif
{
#if HAVE_FEATURE_BREAKPAD
    m_xPrivacyFrame->show();
#endif

#if defined(_WIN32)
    // Store-packaged apps (located under the protected Program Files\WindowsApps) can't use normal
    // shell shortcuts to their exe. TODO: show a button to open "Startup Apps" system applet?
    if (!sal::systools::IsStorePackagedApp())
        m_xQuickStarterFrame->show();

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

OUString OfaMiscTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { "label1", "label2", "label4", "label5", "yearslabel",
                          "toyear", "label7", "label8", "label9" };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[]
        = { "exthelp",   "popupnohelp", "cbShowTipOfTheDay", "filedlg",
            "docstatus", "crashreport", "quicklaunch",       "cbPerformFileExtCheck" };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    if (const auto& pString = m_xBuilder->weld_button("assocfiles"))
        sAllStrings += pString->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool OfaMiscTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    if ( m_xPopUpNoHelpCB->get_state_changed_from_saved() )
        officecfg::Office::Common::Help::BuiltInHelpNotInstalledPopUp::set(m_xPopUpNoHelpCB->get_active(), batch);

    if ( m_xExtHelpCB->get_state_changed_from_saved() )
        officecfg::Office::Common::Help::ExtendedTip::set(m_xExtHelpCB->get_active(), batch);

    if ( m_xShowTipOfTheDay->get_state_changed_from_saved() )
    {
        officecfg::Office::Common::Misc::ShowTipOfTheDay::set(m_xShowTipOfTheDay->get_active(), batch);
        bModified = true;
    }

    if ( m_xFileDlgCB->get_state_changed_from_saved() )
    {
        officecfg::Office::Common::Misc::UseSystemFileDialog::set( !m_xFileDlgCB->get_active(), batch );
        bModified = true;
    }

    if (m_xDocStatusCB->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Print::PrintingModifiesDocument::set(m_xDocStatusCB->get_active(), batch);
        bModified = true;
    }

    const SfxUInt16Item* pUInt16Item = GetOldItem( *rSet, SID_ATTR_YEAR2000 );
    sal_uInt16 nNum = static_cast<sal_uInt16>(m_xYearValueField->get_text().toInt32());
    if ( pUInt16Item && pUInt16Item->GetValue() != nNum )
    {
        bModified = true;
        rSet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, nNum ) );
    }

#if HAVE_FEATURE_BREAKPAD
    if (m_xCrashReport->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Misc::CrashReport::set(m_xCrashReport->get_active(), batch);
        bModified = true;
    }
#endif

#if defined(_WIN32)
    if (m_xPerformFileExtCheck->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Misc::PerformFileExtCheck::set(
            m_xPerformFileExtCheck->get_active(), batch);
        bModified = true;
    }

    if( m_xQuickLaunchCB->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(SID_ATTR_QUICKLAUNCHER, m_xQuickLaunchCB->get_active()));
        bModified = true;
    }
#endif

    batch->commit();

    return bModified;
}

void OfaMiscTabPage::Reset( const SfxItemSet* rSet )
{
    bool bEnable = !officecfg::Office::Common::Help::ExtendedTip::isReadOnly();
    m_xExtHelpCB->set_active( officecfg::Office::Common::Help::Tip::get() &&
            officecfg::Office::Common::Help::ExtendedTip::get() );
    m_xExtHelpCB->set_sensitive(bEnable);
    m_xExtHelpImg->set_visible(!bEnable);
    m_xExtHelpCB->save_state();

    bEnable = !officecfg::Office::Common::Help::BuiltInHelpNotInstalledPopUp::isReadOnly();
    m_xPopUpNoHelpCB->set_active( officecfg::Office::Common::Help::BuiltInHelpNotInstalledPopUp::get() );
    m_xPopUpNoHelpCB->set_sensitive(bEnable);
    m_xPopUpNoHelpImg->set_visible(!bEnable);
    m_xPopUpNoHelpCB->save_state();

    bEnable = !officecfg::Office::Common::Misc::ShowTipOfTheDay::isReadOnly();
    m_xShowTipOfTheDay->set_active( officecfg::Office::Common::Misc::ShowTipOfTheDay::get() );
    m_xShowTipOfTheDay->set_sensitive(bEnable);
    m_xShowTipOfTheDayImg->set_visible(!bEnable);
    m_xShowTipOfTheDay->save_state();

    if (!lcl_HasSystemFilePicker())
        m_xFileDlgFrame->hide();
    else
    {
        bEnable = !officecfg::Office::Common::Misc::UseSystemFileDialog::isReadOnly();
        m_xFileDlgCB->set_sensitive(bEnable);
        m_xFileDlgROImage->set_visible(!bEnable);
    }
    m_xFileDlgCB->set_active(!officecfg::Office::Common::Misc::UseSystemFileDialog::get());
    m_xFileDlgCB->save_state();

    bEnable = !officecfg::Office::Common::Print::PrintingModifiesDocument::isReadOnly();
    m_xDocStatusCB->set_active(officecfg::Office::Common::Print::PrintingModifiesDocument::get());
    m_xDocStatusCB->set_sensitive(bEnable);
    m_xDocStatusImg->set_visible(!bEnable);
    m_xDocStatusCB->save_state();

    bEnable = !officecfg::Office::Common::DateFormat::TwoDigitYear::isReadOnly();
    m_xYearLabel->set_sensitive(bEnable);
    m_xYearValueField->set_sensitive(bEnable);
    m_xToYearFT->set_sensitive(bEnable);
    m_xYearFrameImg->set_visible(!bEnable);

    if ( const SfxUInt16Item* pYearItem = rSet->GetItemIfSet( SID_ATTR_YEAR2000, false ) )
    {
        m_xYearValueField->set_value( pYearItem->GetValue() );
        TwoFigureHdl(*m_xYearValueField);
    }
    else
        m_xYearFrame->set_sensitive(false);

#if HAVE_FEATURE_BREAKPAD
    m_xCrashReport->set_active(officecfg::Office::Common::Misc::CrashReport::get() && CrashReporter::IsDumpEnable());
    m_xCrashReport->set_sensitive(!officecfg::Office::Common::Misc::CrashReport::isReadOnly() && CrashReporter::IsDumpEnable());
    m_xCrashReportImg->set_visible(officecfg::Office::Common::Misc::CrashReport::isReadOnly() && CrashReporter::IsDumpEnable());
    m_xCrashReport->save_state();
#endif

#if defined(_WIN32)
    const SfxPoolItem* pItem = nullptr;
    SfxItemState eState = rSet->GetItemState( SID_ATTR_QUICKLAUNCHER, false, &pItem );
    if ( SfxItemState::SET == eState )
        m_xQuickLaunchCB->set_active( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
    else if ( SfxItemState::DISABLED == eState )
    {
        // quickstart not installed
        m_xQuickStarterFrame->hide();
    }

    m_xQuickLaunchCB->save_state();

    m_xPerformFileExtCheck->set_active(
        officecfg::Office::Common::Misc::PerformFileExtCheck::get());
    m_xPerformFileExtCheck->save_state();
    m_xPerformFileExtCheck->set_sensitive(!officecfg::Office::Common::Misc::PerformFileExtCheck::isReadOnly());
    m_xPerformFileExtImg->set_visible(officecfg::Office::Common::Misc::PerformFileExtCheck::isReadOnly());
#endif
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
    vcl::fileregistration::LaunchRegistrationUI();
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

        for (auto& serviceName : xNameAccess->getElementNames())
        {
            Reference<XNameAccess> xEntryNameAccess(
                xHierarchicalNameAccess->getByHierarchicalName(serviceName),
                UNO_QUERY );

            if( xEntryNameAccess.is() )
            {
                Sequence<OUString> preferredImplementations;
                if( xEntryNameAccess->getByName("PreferredImplementations") >>= preferredImplementations )
                    maAvailableImplementations.emplace_back(serviceName, preferredImplementations);
            }
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
            for (auto& currImpl : availableImpl.second)
            {
                try
                {
                    Reference<XPropertySet> xPropSet( xFactory->createInstance(
                                                          currImpl.trim() ),
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
    , pCanvasSettings(new CanvasSettings)
    , m_xIconSizeLabel(m_xBuilder->weld_label("label14"))
    , m_xIconSizeLB(m_xBuilder->weld_combo_box("iconsize"))
    , m_xIconSizeImg(m_xBuilder->weld_widget("lockiconsize"))
    , m_xSidebarIconSizeLabel(m_xBuilder->weld_label("label9"))
    , m_xSidebarIconSizeLB(m_xBuilder->weld_combo_box("sidebariconsize"))
    , m_xSidebarIconSizeImg(m_xBuilder->weld_widget("locksidebariconsize"))
    , m_xNotebookbarIconSizeLabel(m_xBuilder->weld_label("label8"))
    , m_xNotebookbarIconSizeLB(m_xBuilder->weld_combo_box("notebookbariconsize"))
    , m_xNotebookbarIconSizeImg(m_xBuilder->weld_widget("locknotebookbariconsize"))
    , m_xDarkModeFrame(m_xBuilder->weld_widget("darkmode"))
    , m_xAppearanceStyleLabel(m_xBuilder->weld_label("label7"))
    , m_xAppearanceStyleLB(m_xBuilder->weld_combo_box("appearance"))
    , m_xAppearanceStyleImg(m_xBuilder->weld_widget("lockappearance"))
    , m_xIconStyleLabel(m_xBuilder->weld_label("label6"))
    , m_xIconStyleLB(m_xBuilder->weld_combo_box("iconstyle"))
    , m_xIconStyleImg(m_xBuilder->weld_widget("lockiconstyle"))
    , m_xFontAntiAliasing(m_xBuilder->weld_check_button("aafont"))
    , m_xFontAntiAliasingImg(m_xBuilder->weld_widget("lockaafont"))
    , m_xAAPointLimitLabel(m_xBuilder->weld_label("aafrom"))
    , m_xAAPointLimitLabelImg(m_xBuilder->weld_widget("lockaafrom"))
    , m_xAAPointLimit(m_xBuilder->weld_metric_spin_button("aanf", FieldUnit::PIXEL))
    , m_xFontShowCB(m_xBuilder->weld_check_button("showfontpreview"))
    , m_xFontShowImg(m_xBuilder->weld_widget("lockshowfontpreview"))
    , m_xUseHardwareAccell(m_xBuilder->weld_check_button("useaccel"))
    , m_xUseHardwareAccellImg(m_xBuilder->weld_widget("lockuseaccel"))
    , m_xUseAntiAliase(m_xBuilder->weld_check_button("useaa"))
    , m_xUseAntiAliaseImg(m_xBuilder->weld_widget("lockuseaa"))
    , m_xUseSkia(m_xBuilder->weld_check_button("useskia"))
    , m_xUseSkiaImg(m_xBuilder->weld_widget("lockuseskia"))
    , m_xForceSkiaRaster(m_xBuilder->weld_check_button("forceskiaraster"))
    , m_xForceSkiaRasterImg(m_xBuilder->weld_widget("lockforceskiaraster"))
    , m_xSkiaStatusEnabled(m_xBuilder->weld_label("skiaenabled"))
    , m_xSkiaStatusDisabled(m_xBuilder->weld_label("skiadisabled"))
    , m_xSkiaLog(m_xBuilder->weld_button("btnSkialog"))
    , m_xMouseMiddleLabel(m_xBuilder->weld_label("label12"))
    , m_xMouseMiddleLB(m_xBuilder->weld_combo_box("mousemiddle"))
    , m_xMouseMiddleImg(m_xBuilder->weld_widget("lockmousemiddle"))
    , m_xMoreIcons(m_xBuilder->weld_button("btnMoreIcons"))
    , m_xRunGPTests(m_xBuilder->weld_button("btn_rungptest"))
    , m_sAutoStr(m_xIconStyleLB->get_text(0))
{
    OUString sToolKitName(Application::GetToolkitName());
    const bool bHasDarkMode = sToolKitName.startsWith("gtk") || sToolKitName == "osx" || sToolKitName == "win";
    if (!bHasDarkMode)
        m_xDarkModeFrame->hide();

    m_xFontAntiAliasing->connect_toggled( LINK( this, OfaViewTabPage, OnAntialiasingToggled ) );

    m_xUseSkia->connect_toggled(LINK(this, OfaViewTabPage, OnUseSkiaToggled));
    m_xSkiaLog->connect_clicked(LINK(this, OfaViewTabPage, OnCopySkiaLog));

    UpdateIconThemes();

    m_xIconStyleLB->set_active(0);

    m_xMoreIcons->connect_clicked(LINK(this, OfaViewTabPage, OnMoreIconsClick));
    m_xRunGPTests->connect_clicked( LINK( this, OfaViewTabPage, OnRunGPTestClick));
}

void OfaViewTabPage::UpdateIconThemes()
{
    // Set known icon themes
    m_xIconStyleLB->clear();
    StyleSettings aStyleSettings = Application::GetSettings().GetStyleSettings();
    mInstalledIconThemes = aStyleSettings.GetInstalledIconThemes();
    std::sort(mInstalledIconThemes.begin(), mInstalledIconThemes.end(), DisplayNameCompareLessThan);

    // Start with the automatically chosen icon theme
    OUString autoThemeId = aStyleSettings.GetAutomaticallyChosenIconTheme();
    const vcl::IconThemeInfo& autoIconTheme = vcl::IconThemeInfo::FindIconThemeById(mInstalledIconThemes, autoThemeId);

    OUString entryForAuto = m_sAutoStr + " (" + autoIconTheme.GetDisplayName() + ")";
    m_xIconStyleLB->append("auto", entryForAuto); // index 0 means choose style automatically

    // separate auto and other icon themes
    m_xIconStyleLB->append_separator("");

    for (auto const& installIconTheme : mInstalledIconThemes)
        m_xIconStyleLB->append(installIconTheme.GetThemeId(), installIconTheme.GetDisplayName());
}

OfaViewTabPage::~OfaViewTabPage()
{
}

IMPL_LINK_NOARG(OfaViewTabPage, OnRunGPTestClick, weld::Button&, void)
{
    GraphicsTestsDialog m_xGraphicsTestDialog(m_xContainer.get());
    m_xGraphicsTestDialog.run();
}

IMPL_STATIC_LINK_NOARG(OfaViewTabPage, OnMoreIconsClick, weld::Button&, void)
{
    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        "AdditionsTag", OUString("Icons")) };
    comphelper::dispatchCommand(".uno:AdditionsDialog", aArgs);
}

IMPL_LINK_NOARG( OfaViewTabPage, OnAntialiasingToggled, weld::Toggleable&, void )
{
    bool bAAEnabled = m_xFontAntiAliasing->get_active() && !officecfg::Office::Common::View::FontAntiAliasing::MinPixelHeight::isReadOnly();

    m_xAAPointLimitLabel->set_sensitive(bAAEnabled);
    m_xAAPointLimit->set_sensitive(bAAEnabled);
}

IMPL_LINK_NOARG(OfaViewTabPage, OnUseSkiaToggled, weld::Toggleable&, void)
{
    UpdateSkiaStatus();
}

IMPL_LINK_NOARG(OfaViewTabPage, OnCopySkiaLog, weld::Button&, void)
{
#if HAVE_FEATURE_SKIA
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> xClipboard =
        css::datatransfer::clipboard::SystemClipboard::create(
            comphelper::getProcessComponentContext());
    OUString sInfo = SkiaHelper::readLog();
    vcl::unohelper::TextDataObject::CopyStringTo(sInfo, xClipboard);
    m_xSkiaLog->set_from_icon_name(RID_SVXBMP_COPY);
#endif
}

void OfaViewTabPage::HideSkiaWidgets()
{
    m_xUseSkia->hide();
    m_xForceSkiaRaster->hide();
    m_xSkiaStatusEnabled->hide();
    m_xSkiaStatusDisabled->hide();
    m_xSkiaLog->hide();
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
    // OSX backend has Skia support too.
    if (Application::GetToolkitName() == "osx")
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

    m_xUseSkia->set_sensitive(!officecfg::Office::Common::VCL::UseSkia::isReadOnly());
    m_xUseSkiaImg->set_visible(officecfg::Office::Common::VCL::UseSkia::isReadOnly());
    m_xForceSkiaRaster->set_sensitive(m_xUseSkia->get_active() && !officecfg::Office::Common::VCL::ForceSkiaRaster::isReadOnly());
    m_xForceSkiaRasterImg->set_visible(officecfg::Office::Common::VCL::ForceSkiaRaster::isReadOnly());
    m_xSkiaLog->set_sensitive(bEnabled);

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

OUString OfaViewTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { "label16", "label7",      "label1",       "label6", "label15",
                          "label14", "label8",      "label9",       "label4", "label12",
                          "label2",  "skiaenabled", "skiadisabled", "label5", "aafrom" };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[]
        = { "useaccel", "useaa", "useskia", "forceskiaraster", "showfontpreview", "aafont" };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    sAllStrings += m_xSkiaLog->get_label() + " " + m_xRunGPTests->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool OfaViewTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;
    bool bDarkModeOptModified = false;
    bool bRepaintWindows(false);
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(comphelper::ConfigurationChanges::create());

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
                SAL_WARN("cui.options", "OfaViewTabPage::FillItemSet(): This state of m_xIconSizeLB should not be possible!");
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
                SAL_WARN("cui.options", "OfaViewTabPage::FillItemSet(): This state of m_xSidebarIconSizeLB should not be possible!");
        }
        officecfg::Office::Common::Misc::SidebarIconSize::set(static_cast<sal_Int16>(eSet), xChanges);
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
                SAL_WARN("cui.options", "OfaViewTabPage::FillItemSet(): This state of m_xNotebookbarIconSizeLB should not be possible!");
        }
        officecfg::Office::Common::Misc::NotebookbarIconSize::set(static_cast<sal_Int16>(eSet), xChanges);
    }

    const sal_Int32 nStyleLB_NewSelection = m_xIconStyleLB->get_active();
    if( nStyleLB_InitialSelection != nStyleLB_NewSelection )
    {
        aMiscOptions.SetIconTheme(m_xIconStyleLB->get_active_id());
        nStyleLB_InitialSelection = nStyleLB_NewSelection;
    }

    bool bAppearanceChanged = false;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    // Middle Mouse Button
    MouseMiddleButtonAction eOldMiddleMouse = static_cast<MouseMiddleButtonAction>(officecfg::Office::Common::View::Dialog::MiddleMouseButton::get());
    short eNewMiddleMouse = m_xMouseMiddleLB->get_active();
    if(eNewMiddleMouse > 2)
        eNewMiddleMouse = 2;

    if ( eNewMiddleMouse != static_cast<short>(eOldMiddleMouse) )
    {
        officecfg::Office::Common::View::Dialog::MiddleMouseButton::set(eNewMiddleMouse, batch);
        bAppearanceChanged = true;
    }

    if (m_xFontAntiAliasing->get_state_changed_from_saved())
    {
        bool b = m_xFontAntiAliasing->get_active();
        officecfg::Office::Common::View::FontAntiAliasing::Enabled::set(b, batch);
        bAppearanceChanged = true;
    }

    if (m_xAAPointLimit->get_value_changed_from_saved())
    {
        sal_Int64 i = m_xAAPointLimit->get_value(FieldUnit::PIXEL);
        officecfg::Office::Common::View::FontAntiAliasing::MinPixelHeight::set(i, batch);
        bAppearanceChanged = true;
    }

    if (m_xFontShowCB->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Font::View::ShowFontBoxWYSIWYG::set(m_xFontShowCB->get_active(), xChanges);
        bModified = true;
    }

    if (m_xAppearanceStyleLB->get_value_changed_from_saved())
    {
        bDarkModeOptModified = true;
        bModified = true;
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
        if (m_xUseAntiAliase->get_active() != SvtOptionsDrawinglayer::IsAntiAliasing())
        {
            SvtOptionsDrawinglayer::SetAntiAliasing(m_xUseAntiAliase->get_active(), /*bTemporary*/false);
            bModified = true;
            bRepaintWindows = true;
        }
    }

    if (m_xUseSkia->get_state_changed_from_saved() ||
        m_xForceSkiaRaster->get_state_changed_from_saved())
    {
        officecfg::Office::Common::VCL::UseSkia::set(m_xUseSkia->get_active(), xChanges);
        officecfg::Office::Common::VCL::ForceSkiaRaster::set(m_xForceSkiaRaster->get_active(), xChanges);
        bModified = true;
    }

    xChanges->commit();

    if (bDarkModeOptModified)
        MiscSettings::SetDarkMode(m_xAppearanceStyleLB->get_active());

    if ( bAppearanceChanged )
    {
        batch->commit();
        SvtTabAppearanceCfg::SetApplicationDefaults ( GetpApp() );
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
    bool bEnable = true;

    if (SvtMiscOptions::GetSymbolsSize() != SFX_SYMBOLS_SIZE_AUTO)
    {
        nSizeLB_InitialSelection = 1;

        if (SvtMiscOptions::GetSymbolsSize() == SFX_SYMBOLS_SIZE_LARGE)
            nSizeLB_InitialSelection = 2;
        else if (SvtMiscOptions::GetSymbolsSize() == SFX_SYMBOLS_SIZE_32)
            nSizeLB_InitialSelection = 3;
    }
    bEnable = !officecfg::Office::Common::Misc::SymbolSet::isReadOnly();
    m_xIconSizeLB->set_active( nSizeLB_InitialSelection );
    m_xIconSizeLabel->set_sensitive(bEnable);
    m_xIconSizeLB->set_sensitive(bEnable);
    m_xMoreIcons->set_sensitive(bEnable);
    m_xIconSizeImg->set_visible(!bEnable);
    m_xIconSizeLB->save_value();

    ToolBoxButtonSize eSidebarIconSize = static_cast<ToolBoxButtonSize>(officecfg::Office::Common::Misc::SidebarIconSize::get());
    if( eSidebarIconSize == ToolBoxButtonSize::DontCare )
        ; // do nothing
    else if( eSidebarIconSize == ToolBoxButtonSize::Small )
        nSidebarSizeLB_InitialSelection = 1;
    else if( eSidebarIconSize == ToolBoxButtonSize::Large )
        nSidebarSizeLB_InitialSelection = 2;

    bEnable = !officecfg::Office::Common::Misc::SidebarIconSize::isReadOnly();
    m_xSidebarIconSizeLB->set_active( nSidebarSizeLB_InitialSelection );
    m_xSidebarIconSizeLabel->set_sensitive(bEnable);
    m_xSidebarIconSizeLB->set_sensitive(bEnable);
    m_xSidebarIconSizeImg->set_visible(!bEnable);
    m_xSidebarIconSizeLB->save_value();

    ToolBoxButtonSize eNotebookbarIconSize = static_cast<ToolBoxButtonSize>(officecfg::Office::Common::Misc::NotebookbarIconSize::get());
    if( eNotebookbarIconSize == ToolBoxButtonSize::DontCare )
        ; // do nothing
    else if( eNotebookbarIconSize == ToolBoxButtonSize::Small )
        nNotebookbarSizeLB_InitialSelection = 1;
    else if( eNotebookbarIconSize == ToolBoxButtonSize::Large )
        nNotebookbarSizeLB_InitialSelection = 2;

    bEnable = !officecfg::Office::Common::Misc::NotebookbarIconSize::isReadOnly();
    m_xNotebookbarIconSizeLB->set_active(nNotebookbarSizeLB_InitialSelection);
    m_xNotebookbarIconSizeLabel->set_sensitive(bEnable);
    m_xNotebookbarIconSizeLB->set_sensitive(bEnable);
    m_xNotebookbarIconSizeImg->set_visible(!bEnable);
    m_xNotebookbarIconSizeLB->save_value();

    // tdf#153497 set name of automatic icon theme, it may have changed due to "Apply" while this page is visible
    UpdateIconThemes();

    if (aMiscOptions.IconThemeWasSetAutomatically()) {
        nStyleLB_InitialSelection = 0;
    }
    else {
        const OUString& selected = SvtMiscOptions::GetIconTheme();
        const vcl::IconThemeInfo& selectedInfo =
                vcl::IconThemeInfo::FindIconThemeById(mInstalledIconThemes, selected);
        nStyleLB_InitialSelection = m_xIconStyleLB->find_text(selectedInfo.GetDisplayName());
    }

    bEnable = !officecfg::Office::Common::Misc::SymbolStyle::isReadOnly();
    m_xIconStyleLB->set_active(nStyleLB_InitialSelection);
    m_xIconStyleLabel->set_sensitive(bEnable);
    m_xIconStyleLB->set_sensitive(bEnable);
    m_xIconStyleImg->set_visible(!bEnable);
    m_xIconStyleLB->save_value();

    bEnable = !officecfg::Office::Common::Misc::Appearance::isReadOnly();
    m_xAppearanceStyleLB->set_active(officecfg::Office::Common::Misc::Appearance::get());
    m_xAppearanceStyleLabel->set_sensitive(bEnable);
    m_xAppearanceStyleLB->set_sensitive(bEnable);
    m_xAppearanceStyleImg->set_visible(!bEnable);
    m_xAppearanceStyleLB->save_value();

    // Middle Mouse Button
    bEnable = !officecfg::Office::Common::View::Dialog::MiddleMouseButton::isReadOnly();
    sal_Int16 nMiddleMouseButton = officecfg::Office::Common::View::Dialog::MiddleMouseButton::get();
    m_xMouseMiddleLB->set_active(static_cast<short>(nMiddleMouseButton));
    m_xMouseMiddleLabel->set_sensitive(bEnable);
    m_xMouseMiddleLB->set_sensitive(bEnable);
    m_xMouseMiddleImg->set_visible(!bEnable);
    m_xMouseMiddleLB->save_value();

    bEnable = !officecfg::Office::Common::View::FontAntiAliasing::Enabled::isReadOnly();
    bool bFontAntiAliasing = officecfg::Office::Common::View::FontAntiAliasing::Enabled::get();
    m_xFontAntiAliasing->set_active( bFontAntiAliasing );
    m_xFontAntiAliasing->set_sensitive(bEnable);
    m_xFontAntiAliasingImg->set_visible(!bEnable);

    bEnable = !officecfg::Office::Common::View::FontAntiAliasing::MinPixelHeight::isReadOnly();
    sal_Int16 nFontAntiAliasingMinPixelHeight = officecfg::Office::Common::View::FontAntiAliasing::MinPixelHeight::get();
    m_xAAPointLimit->set_value(nFontAntiAliasingMinPixelHeight, FieldUnit::PIXEL);
    m_xAAPointLimit->set_sensitive(bEnable);
    m_xAAPointLimitLabelImg->set_visible(!bEnable);

    // WorkingSet
    bEnable = !officecfg::Office::Common::Font::View::ShowFontBoxWYSIWYG::isReadOnly();
    m_xFontShowCB->set_active(officecfg::Office::Common::Font::View::ShowFontBoxWYSIWYG::get());
    m_xFontShowCB->set_sensitive(bEnable);
    m_xFontShowImg->set_visible(!bEnable);

    UpdateHardwareAccelStatus();
    m_xUseHardwareAccell->save_state();

    { // #i95644# AntiAliasing
        if(SvtOptionsDrawinglayer::IsAAPossibleOnThisSystem())
        {
            m_xUseAntiAliase->set_active(SvtOptionsDrawinglayer::IsAntiAliasing());
        }
        else
        {
            m_xUseAntiAliase->set_active(false);
            m_xUseAntiAliase->set_sensitive(false);
            m_xUseAntiAliaseImg->set_visible(true);
        }

        bEnable = !officecfg::Office::Common::Drawinglayer::AntiAliasing::isReadOnly();
        m_xUseAntiAliase->set_sensitive(bEnable);
        m_xUseAntiAliaseImg->set_visible(!bEnable);
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
        m_xUseHardwareAccellImg->set_visible(pCanvasSettings->IsHardwareAccelerationRO());
    }
    else
    {
        m_xUseHardwareAccell->set_active(false);
        m_xUseHardwareAccell->set_sensitive(false);
        m_xUseHardwareAccellImg->set_visible(true);
    }
#if HAVE_FEATURE_SKIA
    m_xUseHardwareAccell->set_sensitive(!m_xUseSkia->get_active());
#endif
}

struct LanguageConfig_Impl
{
    SvtCTLOptions aCTLLanguageOptions;
    SvtSysLocaleOptions aSysLocaleOptions;
    SvtLinguConfig aLinguConfig;
};

static bool bLanguageCurrentDoc_Impl = false;

// some things we'll need...
constexpr OUString sAccessSrvc = u"com.sun.star.configuration.ConfigurationAccess"_ustr;
constexpr OUStringLiteral sAccessUpdSrvc = u"com.sun.star.configuration.ConfigurationUpdateAccess";
constexpr OUString sInstalledLocalesPath = u"org.openoffice.Setup/Office/InstalledLocales"_ustr;
constexpr OUString sUserLocalePath = u"org.openoffice.Office.Linguistic/General"_ustr;
constexpr OUString sUserLocaleKey = u"UILocale"_ustr;
static Sequence< OUString > seqInstalledLanguages;

static OUString lcl_getDatePatternsConfigString( const LocaleDataWrapper& rLocaleWrapper )
{
    Sequence< OUString > aDateAcceptancePatterns = rLocaleWrapper.getDateAcceptancePatterns();
    sal_Int32 nPatterns = aDateAcceptancePatterns.getLength();
    OUStringBuffer aBuf( nPatterns * 6 );   // 6 := length of Y-M-D;
    SAL_WARN_IF( !nPatterns, "cui.options", "No date acceptance pattern");
    if (nPatterns)
    {
        aBuf.append(aDateAcceptancePatterns[0]);
        for (sal_Int32 i=1; i < nPatterns; ++i)
            aBuf.append(";" + aDateAcceptancePatterns[i]);
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
    , m_xLocaleSettingImg(m_xBuilder->weld_widget("locklocalesetting"))
    , m_xDecimalSeparatorFT(m_xBuilder->weld_label("label6"))
    , m_xDecimalSeparatorCB(m_xBuilder->weld_check_button("decimalseparator"))
    , m_xDecimalSeparatorImg(m_xBuilder->weld_widget("lockdecimalseparator"))
    , m_xCurrencyFT(m_xBuilder->weld_label("defaultcurrency"))
    , m_xCurrencyLB(m_xBuilder->weld_combo_box("currencylb"))
    , m_xCurrencyImg(m_xBuilder->weld_widget("lockcurrencylb"))
    , m_xDatePatternsFT(m_xBuilder->weld_label("dataaccpatterns"))
    , m_xDatePatternsED(m_xBuilder->weld_entry("datepatterns"))
    , m_xDatePatternsImg(m_xBuilder->weld_widget("lockdatepatterns"))
    , m_xWesternLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("westernlanguage")))
    , m_xWesternLanguageFT(m_xBuilder->weld_label("western"))
    , m_xWesternLanguageImg(m_xBuilder->weld_widget("lockwesternlanguage"))
    , m_xAsianLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("asianlanguage")))
    , m_xComplexLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("complexlanguage")))
    , m_xCurrentDocCB(m_xBuilder->weld_check_button("currentdoc"))
    , m_xAsianSupportCB(m_xBuilder->weld_check_button("asiansupport"))
    , m_xAsianSupportImg(m_xBuilder->weld_widget("lockasiansupport"))
    , m_xCTLSupportCB(m_xBuilder->weld_check_button("ctlsupport"))
    , m_xCTLSupportImg(m_xBuilder->weld_widget("lockctlsupport"))
    , m_xIgnoreLanguageChangeCB(m_xBuilder->weld_check_button("ignorelanguagechange"))
    , m_xIgnoreLanguageChangeImg(m_xBuilder->weld_widget("lockignorelanguagechange"))
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
        // find out which locales are currently installed and add them to the listbox
        Sequence< Any > theArgs{ Any(NamedValue("nodepath", Any(sInstalledLocalesPath))) };
        Reference< XNameAccess > theNameAccess(
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
                Application::GetSettings().GetUILanguageTag().getLocale());
            return aSorter.compare(l1.second, l2.second) < 0;
        });

        // tdf#114694: append the sorted list after the default entry and separator.
        for (const auto & [ nGroupID, sGroupName ] : aUILanguages)
        {
            m_xUserInterfaceLB->append(OUString::number(nGroupID), sGroupName);
        }

        m_xUserInterfaceLB->set_active(0);

        // find out whether the user has a specific locale specified
        Sequence< Any > theArgs2{ Any(NamedValue("nodepath", Any(sUserLocalePath))) };
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
        m_xCurrencyLB->append(weld::toId(v), aStr_);
    }

    m_xCurrencyLB->set_active(0);

    m_xLocaleSettingLB->connect_changed( LINK( this, OfaLanguagesTabPage, LocaleSettingHdl ) );
    m_xDatePatternsED->connect_changed( LINK( this, OfaLanguagesTabPage, DatePatternsHdl ) );

    Link<weld::Toggleable&,void> aLink( LINK( this, OfaLanguagesTabPage, SupportHdl ) );
    m_xAsianSupportCB->connect_toggled( aLink );
    m_xCTLSupportCB->connect_toggled( aLink );

    m_bOldAsian = SvtCJKOptions::IsAnyEnabled();
    m_xAsianSupportCB->set_active(m_bOldAsian);
    m_xAsianSupportCB->save_state();
    bool bReadonly = SvtCJKOptions::IsAnyReadOnly();
    m_xAsianSupportCB->set_sensitive(!bReadonly);
    m_xAsianSupportImg->set_visible(bReadonly);
    SupportHdl(*m_xAsianSupportCB);

    m_bOldCtl = SvtCTLOptions::IsCTLFontEnabled();
    m_xCTLSupportCB->set_active(m_bOldCtl);
    m_xCTLSupportCB->save_state();
    bReadonly = pLangConfig->aCTLLanguageOptions.IsReadOnly(SvtCTLOptions::E_CTLFONT);
    m_xCTLSupportCB->set_sensitive(!bReadonly);
    m_xCTLSupportImg->set_visible(bReadonly);
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

OUString OfaLanguagesTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[]
        = { "label1", "label4",          "label7", "localesettingFT", "defaultcurrency",
            "label6", "dataaccpatterns", "label2", "western",         "label3" };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { "decimalseparator", "asiansupport", "ctlsupport", "currentdoc",
                               "ignorelanguagechange" };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool OfaLanguagesTabPage::FillItemSet( SfxItemSet* rSet )
{
    // lock configuration broadcasters so that we can coordinate the notifications
    pLangConfig->aSysLocaleOptions.BlockBroadcasts( true );
    pLangConfig->aCTLLanguageOptions.BlockBroadcasts( true );
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
        pLangConfig->aCTLLanguageOptions.SetCTLSequenceCheckingRestricted(bOn);
        pLangConfig->aCTLLanguageOptions.SetCTLSequenceChecking(bOn);
        pLangConfig->aCTLLanguageOptions.SetCTLSequenceCheckingTypeAndReplace(bOn);
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
        Sequence< Any > theArgs{ Any(NamedValue("nodepath", Any(sUserLocalePath))) };
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
        auto batch = comphelper::ConfigurationChanges::create();
        SvtCompatibilityDefault aCompatOpts(batch);
        aCompatOpts.set(u"ExpandWordSpace"_ustr, !bNewCJK);
        batch->commit();
    }

    if(m_xDecimalSeparatorCB->get_state_changed_from_saved())
        pLangConfig->aSysLocaleOptions.SetDecimalSeparatorAsLocale(m_xDecimalSeparatorCB->get_active());

    if(m_xIgnoreLanguageChangeCB->get_state_changed_from_saved())
        pLangConfig->aSysLocaleOptions.SetIgnoreLanguageChange(m_xIgnoreLanguageChangeCB->get_active());

    // Configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default.
    OUString sOldCurr = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    OUString sId = m_xCurrencyLB->get_active_id();
    const NfCurrencyEntry* pCurr = sId == "default" ? nullptr : weld::fromId<const NfCurrencyEntry*>(sId);
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
            pLangConfig->aLinguConfig.SetProperty( u"DefaultLocale", aValue );
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
            pLangConfig->aLinguConfig.SetProperty( u"DefaultLocale_CJK", aValue );
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
            pLangConfig->aLinguConfig.SetProperty( u"DefaultLocale_CTL", aValue );
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
        SvtCJKOptions::SetAll(bChecked);

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
        pLangConfig->aCTLLanguageOptions.SetCTLFontEnabled( m_xCTLSupportCB->get_active() );

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
    pLangConfig->aCTLLanguageOptions.BlockBroadcasts( false );
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
    m_xLocaleSettingImg->set_visible(bReadonly);


    m_xDecimalSeparatorCB->set_active( pLangConfig->aSysLocaleOptions.IsDecimalSeparatorAsLocale());
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::EOption::DecimalSeparator);
    m_xDecimalSeparatorCB->set_sensitive(!bReadonly);
    m_xDecimalSeparatorFT->set_sensitive(!bReadonly);
    m_xDecimalSeparatorImg->set_visible(bReadonly);
    m_xDecimalSeparatorCB->save_state();

    m_xIgnoreLanguageChangeCB->set_active( pLangConfig->aSysLocaleOptions.IsIgnoreLanguageChange());
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::EOption::IgnoreLanguageChange);
    m_xIgnoreLanguageChangeCB->set_sensitive(!bReadonly);
    m_xIgnoreLanguageChangeImg->set_visible(bReadonly);
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
    OUString sId = !pCurr ? OUString("default") : weld::toId(pCurr);
    m_xCurrencyLB->set_active_id(sId);
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::EOption::Currency);
    m_xCurrencyLB->set_sensitive(!bReadonly);
    m_xCurrencyFT->set_sensitive(!bReadonly);
    m_xCurrencyImg->set_visible(bReadonly);

    // date acceptance patterns
    OUString aDatePatternsString = pLangConfig->aSysLocaleOptions.GetDatePatternsConfigString();
    if (aDatePatternsString.isEmpty())
    {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        aDatePatternsString = lcl_getDatePatternsConfigString( rLocaleWrapper);
        // Let's assume patterns are valid at this point.
        m_bDatePatternsValid = true;
    }
    else
    {
        bool bModified = false;
        m_bDatePatternsValid = validateDatePatterns( bModified, aDatePatternsString);
    }
    m_xDatePatternsED->set_text(aDatePatternsString);
    m_xDatePatternsED->set_message_type( m_bDatePatternsValid ?
            weld::EntryMessageType::Normal : weld::EntryMessageType::Error);
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::EOption::DatePatterns);
    m_xDatePatternsED->set_sensitive(!bReadonly);
    m_xDatePatternsFT->set_sensitive(!bReadonly);
    m_xDatePatternsImg->set_visible(bReadonly);
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
        aWestLang = pLangConfig->aLinguConfig.GetProperty(u"DefaultLocale");
        Locale aLocale;
        aWestLang >>= aLocale;

        eCurLang = LanguageTag::convertToLanguageType( aLocale, false);

        aCJKLang = pLangConfig->aLinguConfig.GetProperty(u"DefaultLocale_CJK");
        aLocale = Locale();
        aCJKLang >>= aLocale;
        eCurLangCJK = LanguageTag::convertToLanguageType( aLocale, false);

        aCTLLang = pLangConfig->aLinguConfig.GetProperty(u"DefaultLocale_CTL");
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
        if( const SvxLanguageItem* pLangItem = rSet->GetItemIfSet(SID_ATTR_LANGUAGE, false))
        {
            LanguageType eTempCurLang = pLangItem->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLang, css::i18n::ScriptType::LATIN) != eTempCurLang)
                eCurLang = eTempCurLang;
        }

        if( const SvxLanguageItem* pLang = rSet->GetItemIfSet(SID_ATTR_CHAR_CJK_LANGUAGE, false))
        {
            LanguageType eTempCurLang = pLang->GetValue();
            if (MsLangId::resolveSystemLanguageByScriptType(eCurLangCJK, css::i18n::ScriptType::ASIAN) != eTempCurLang)
                eCurLangCJK = eTempCurLang;
        }

        if( const SvxLanguageItem* pLang = rSet->GetItemIfSet(SID_ATTR_CHAR_CTL_LANGUAGE, false))
        {
            LanguageType eTempCurLang = pLang->GetValue();
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

    bool bEnable = !pLangConfig->aLinguConfig.IsReadOnly( u"DefaultLocale" );
    m_xWesternLanguageFT->set_sensitive( bEnable );
    m_xWesternLanguageLB->set_sensitive( bEnable );
    m_xWesternLanguageImg->set_visible( !bEnable );

    // check the box "For the current document only"
    // set the focus to the Western Language box
    const SfxBoolItem* pLang = rSet->GetItemIfSet(SID_SET_DOCUMENT_LANGUAGE, false );
    if ( pLang && pLang->GetValue() )
    {
        m_xWesternLanguageLB->grab_focus();
        m_xCurrentDocCB->set_sensitive(true);
        m_xCurrentDocCB->set_active(true);
    }
}

IMPL_LINK(OfaLanguagesTabPage, SupportHdl, weld::Toggleable&, rBox, void)
{
    bool bCheck = rBox.get_active();
    if ( m_xAsianSupportCB.get() == &rBox )
    {
        bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly(u"DefaultLocale_CJK");
        bCheck = ( bCheck && !bReadonly );
        m_xAsianLanguageLB->set_sensitive( bCheck );
        if (rBox.get_sensitive())
            m_bOldAsian = bCheck;
    }
    else if ( m_xCTLSupportCB.get() == &rBox )
    {
        bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly(u"DefaultLocale_CTL");
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
    if(!pLangConfig->aCTLLanguageOptions.IsReadOnly(SvtCTLOptions::E_CTLFONT))
    {
        bool bIsCTLFixed = bool(nType & SvtScriptType::COMPLEX);
        lcl_checkLanguageCheckBox(*m_xCTLSupportCB, bIsCTLFixed, m_bOldCtl);
        SupportHdl(*m_xCTLSupportCB);
    }
    // second check if CJK must be enabled
    // #103299# - if CJK support is not readonly
    if(!SvtCJKOptions::IsAnyReadOnly())
    {
        bool bIsCJKFixed = bool(nType & SvtScriptType::ASIAN);
        lcl_checkLanguageCheckBox(*m_xAsianSupportCB, bIsCJKFixed, m_bOldAsian);
        SupportHdl(*m_xAsianSupportCB);
    }

    const NfCurrencyEntry& rCurr = SvNumberFormatter::GetCurrencyEntry(
            (eLang == LANGUAGE_USER_SYSTEM_CONFIG) ? MsLangId::getConfiguredSystemLanguage() : eLang);
    constexpr OUString aDefaultID = u"default"_ustr;
    // Update the "Default ..." currency.
    m_xCurrencyLB->remove_id(aDefaultID);
    OUString aDefaultCurr = m_sSystemDefaultString + " - " + rCurr.GetBankSymbol();
    m_xCurrencyLB->insert(0, aDefaultCurr, &aDefaultID, nullptr, nullptr);
    assert(m_xCurrencyLB->find_id(aDefaultID) != -1);
    m_xCurrencyLB->set_active_text(aDefaultCurr);

    // obtain corresponding locale data
    LocaleDataWrapper aLocaleWrapper(( LanguageTag(eLang) ));

    // update the decimal separator key of the related CheckBox
    OUString sTempLabel(m_sDecimalSeparatorLabel);
    sTempLabel = sTempLabel.replaceFirst("%1", aLocaleWrapper.getNumDecimalSep() );
    m_xDecimalSeparatorCB->set_label(sTempLabel);

    // update the date acceptance patterns
    OUString aDatePatternsString = lcl_getDatePatternsConfigString( aLocaleWrapper);
    m_bDatePatternsValid = true;
    m_xDatePatternsED->set_text( aDatePatternsString);
    m_xDatePatternsED->set_message_type(weld::EntryMessageType::Normal);
}

IMPL_LINK( OfaLanguagesTabPage, DatePatternsHdl, weld::Entry&, rEd, void )
{
    OUString aPatterns(rEd.get_text());
    bool bModified = false;
    const bool bValid = validateDatePatterns( bModified, aPatterns);
    if (bModified)
    {
        // gtk3 keeps the cursor position on equal length set_text() but at
        // least the 'gen' backend does not and resets to 0.
        const int nCursorPos = rEd.get_position();
        rEd.set_text(aPatterns);
        rEd.set_position(nCursorPos);
    }
    if (bValid)
        rEd.set_message_type(weld::EntryMessageType::Normal);
    else
        rEd.set_message_type(weld::EntryMessageType::Error);
    m_bDatePatternsValid = bValid;
}

bool OfaLanguagesTabPage::validateDatePatterns( bool& rbModified, OUString& rPatterns )
{
    bool bValid = true;
    if (!rPatterns.isEmpty())
    {
        OUStringBuffer aBuf( rPatterns);
        sal_Int32 nChar = 0;
        for (sal_Int32 nIndex=0; nIndex >= 0 && bValid; ++nChar)
        {
            const OUString aPat( rPatterns.getToken( 0, ';', nIndex));
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
                if (aPat.getLength() == 3)
                {
                    // Disallow a pattern that would match a numeric input with
                    // decimal separator, like M.D
                    const LanguageType eLang = m_xLocaleSettingLB->get_active_id();
                    const LocaleDataWrapper aLocaleWrapper(( LanguageTag(eLang)));
                    if (    aPat[1] == aLocaleWrapper.getNumDecimalSep().toChar()
                         || aPat[1] == aLocaleWrapper.getNumDecimalSepAlt().toChar())
                    {
                        bValid = false;
                    }
                }
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
                                rbModified = true;
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
                                rbModified = true;
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
                                rbModified = true;
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
        if (rbModified)
            rPatterns = aBuf.makeStringAndClear();
    }
    return bValid;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

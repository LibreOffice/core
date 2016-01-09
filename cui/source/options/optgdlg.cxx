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

#include <svl/zforlist.hxx>
#include <svl/currencytable.hxx>
#include <svtools/grfmgr.hxx>
#include <svl/flagitem.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/lingucfg.hxx>
#include <svl/szitem.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/imgmgr.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/layout.hxx>
#include <vcl/mnemonic.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/compatibility.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/fontoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/printwarningoptions.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/configitem.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <svtools/langtab.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/configmgr.hxx>
#include "cuioptgenrl.hxx"
#include "optpath.hxx"
#include "optsave.hxx"
#include "optlingu.hxx"
#include <svx/xpool.hxx>
#include <svx/dlgutil.hxx>
#include "cuitabarea.hxx"
#include <cuires.hrc>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>
#include <dialmgr.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/searchopt.hxx>
#include <sal/macros.h>
#include <officecfg/Office/Common.hxx>
#include <comphelper/configuration.hxx>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/office/Quickstart.hpp>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>

#include "optgdlg.hxx"
#include <svx/ofaitem.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/optionsdrawinglayer.hxx>

#include <config_vclplug.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::utl;

namespace svt {

class OpenGLCfg
{
private:
    bool mbUseOpenGL;
    bool mbForceOpenGL;
    bool mbModified;

public:
    OpenGLCfg();
    ~OpenGLCfg();

    bool useOpenGL() const;
    bool forceOpenGL() const;

    void setUseOpenGL(bool bOpenGL);
    void setForceOpenGL(bool bOpenGL);

    void reset();
};

OpenGLCfg::OpenGLCfg():
    mbModified(false)
{
    reset();
}

void OpenGLCfg::reset()
{
    mbUseOpenGL = officecfg::Office::Common::VCL::UseOpenGL::get();
    mbForceOpenGL = officecfg::Office::Common::VCL::ForceOpenGL::get();
    mbModified = false;
}

OpenGLCfg::~OpenGLCfg()
{
    if (mbModified)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        if (!officecfg::Office::Common::VCL::UseOpenGL::isReadOnly())
            officecfg::Office::Common::VCL::UseOpenGL::set(mbUseOpenGL, batch);
        if (!officecfg::Office::Common::VCL::ForceOpenGL::isReadOnly())
            officecfg::Office::Common::VCL::ForceOpenGL::set(mbForceOpenGL, batch);
        batch->commit();
    }
}

bool OpenGLCfg::useOpenGL() const
{
    return mbUseOpenGL;
}

bool OpenGLCfg::forceOpenGL() const
{
    return mbForceOpenGL;
}

void OpenGLCfg::setUseOpenGL(bool bOpenGL)
{
    if (bOpenGL != mbUseOpenGL)
    {
        mbUseOpenGL = bOpenGL;
        mbModified = true;
    }
}

void OpenGLCfg::setForceOpenGL(bool bOpenGL)
{
    if (mbForceOpenGL != bOpenGL)
    {
        mbForceOpenGL = bOpenGL;
        mbModified = true;
    }
}

}

// class OfaMiscTabPage --------------------------------------------------

SvxGeneralTabPage::sfxpg OfaMiscTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( pSet_ );
    return LEAVE_PAGE;
}

namespace
{
        static OUString impl_SystemFileOpenServiceName()
        {
            const OUString &rDesktopEnvironment = Application::GetDesktopEnvironment();

            if ( rDesktopEnvironment.equalsIgnoreAsciiCase("kde4") )
            {
                #if ENABLE_KDE4
                return OUString("com.sun.star.ui.dialogs.KDE4FilePicker" );
                #else
                return OUString();
                #endif
            }
            else if ( rDesktopEnvironment.equalsIgnoreAsciiCase("tde") )
            {
                #if ENABLE_TDE
                return OUString("com.sun.star.ui.dialogs.TDEFilePicker");
                #else
                return OUString();
                #endif
            }
            #if defined WNT
            return OUString("com.sun.star.ui.dialogs.SystemFilePicker");
            #elif defined MACOSX
            return OUString("com.sun.star.ui.dialogs.AquaFilePicker");
            #else
            return OUString();
            #endif
        }

        static bool lcl_HasSystemFilePicker()
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

OfaMiscTabPage::OfaMiscTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptGeneralPage", "cui/ui/optgeneralpage.ui", &rSet)
{
    get(m_pExtHelpCB, "exthelp");
    if (!lcl_HasSystemFilePicker())
        get<VclContainer>("filedlgframe")->Hide();
#if ! ENABLE_GTK
    get<VclContainer>("printdlgframe")->Hide();
#else
    if (!SvtMiscOptions().IsExperimentalMode())
    {
        get<VclContainer>("printdlgframe")->Hide();
    }
#endif
    get(m_pFileDlgCB, "filedlg");
    get(m_pFileDlgROImage, "lockimage");
    get(m_pPrintDlgCB, "printdlg");
    get(m_pDocStatusCB, "docstatus");
    get(m_pYearFrame, "yearframe");
    get(m_pYearValueField, "year");
    get(m_pToYearFT, "toyear");
    get(m_pCollectUsageInfo, "collectusageinfo");

    if (m_pFileDlgCB->IsVisible() && SvtMiscOptions().IsUseSystemFileDialogReadOnly())
    {
        m_pFileDlgROImage->Show();
        m_pFileDlgCB->Disable();
    }

    m_aStrDateInfo = m_pToYearFT->GetText();
    m_pYearValueField->SetModifyHdl( LINK( this, OfaMiscTabPage, TwoFigureHdl ) );
    Link<SpinField&,void> aLink = LINK( this, OfaMiscTabPage, TwoFigureConfigHdl );
    m_pYearValueField->SetDownHdl( aLink );
    m_pYearValueField->SetUpHdl( aLink );
    m_pYearValueField->SetLoseFocusHdl( LINK( this, OfaMiscTabPage, TwoFigureConfigFocusHdl ) );
    m_pYearValueField->SetFirstHdl( aLink );
    TwoFigureConfigHdl(*m_pYearValueField);

    SetExchangeSupport();
}

OfaMiscTabPage::~OfaMiscTabPage()
{
    disposeOnce();
}

void OfaMiscTabPage::dispose()
{
    m_pExtHelpCB.clear();
    m_pFileDlgROImage.clear();
    m_pFileDlgCB.clear();
    m_pPrintDlgCB.clear();
    m_pDocStatusCB.clear();
    m_pYearFrame.clear();
    m_pYearValueField.clear();
    m_pToYearFT.clear();
    m_pCollectUsageInfo.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaMiscTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<OfaMiscTabPage>::Create( pParent, *rAttrSet );
}

bool OfaMiscTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    SvtHelpOptions aHelpOptions;
    if ( m_pExtHelpCB->IsChecked() != (m_pExtHelpCB->GetSavedValue() == TRISTATE_TRUE) )
        aHelpOptions.SetExtendedHelp( m_pExtHelpCB->IsChecked() );

    if ( m_pFileDlgCB->IsValueChangedFromSaved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetUseSystemFileDialog( !m_pFileDlgCB->IsChecked() );
        bModified = true;
    }

    if ( m_pPrintDlgCB->IsValueChangedFromSaved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetUseSystemPrintDialog( !m_pPrintDlgCB->IsChecked() );
        bModified = true;
    }

    if ( m_pDocStatusCB->IsValueChangedFromSaved() )
    {
        SvtPrintWarningOptions aPrintOptions;
        aPrintOptions.SetModifyDocumentOnPrintingAllowed( m_pDocStatusCB->IsChecked() );
        bModified = true;
    }

    const SfxUInt16Item* pUInt16Item = dynamic_cast< const SfxUInt16Item* >( GetOldItem( *rSet, SID_ATTR_YEAR2000 ) );
    sal_uInt16 nNum = (sal_uInt16)m_pYearValueField->GetText().toInt32();
    if ( pUInt16Item && pUInt16Item->GetValue() != nNum )
    {
        bModified = true;
        rSet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, nNum ) );
    }

    if (m_pCollectUsageInfo->IsValueChangedFromSaved())
    {
        officecfg::Office::Common::Misc::CollectUsageInformation::set(m_pCollectUsageInfo->IsChecked(), batch);
        bModified = true;
    }

    batch->commit();

    return bModified;
}

void OfaMiscTabPage::Reset( const SfxItemSet* rSet )
{
    SvtHelpOptions aHelpOptions;
    m_pExtHelpCB->Check( aHelpOptions.IsHelpTips() && aHelpOptions.IsExtendedHelp() );
    m_pExtHelpCB->SaveValue();

    SvtMiscOptions aMiscOpt;
    m_pFileDlgCB->Check( !aMiscOpt.UseSystemFileDialog() );
    m_pFileDlgCB->SaveValue();
    m_pPrintDlgCB->Check( !aMiscOpt.UseSystemPrintDialog() );
    m_pPrintDlgCB->SaveValue();

    SvtPrintWarningOptions aPrintOptions;
    m_pDocStatusCB->Check(aPrintOptions.IsModifyDocumentOnPrintingAllowed());
    m_pDocStatusCB->SaveValue();

    const SfxPoolItem* pItem = nullptr;
    if ( SfxItemState::SET == rSet->GetItemState( SID_ATTR_YEAR2000, false, &pItem ) )
    {
        m_pYearValueField->SetValue( static_cast<const SfxUInt16Item*>(pItem)->GetValue() );
        TwoFigureConfigHdl(*m_pYearValueField);
    }
    else
    {
        m_pYearFrame->Enable(false);
    }

    m_pCollectUsageInfo->Check(officecfg::Office::Common::Misc::CollectUsageInformation::get());
    m_pCollectUsageInfo->SaveValue();
}

IMPL_LINK_NOARG_TYPED( OfaMiscTabPage, TwoFigureHdl, Edit&, void )
{
    OUString aOutput( m_aStrDateInfo );
    OUString aStr( m_pYearValueField->GetText() );
    OUString sSep( SvtSysLocale().GetLocaleData().getNumThousandSep() );
    sal_Int32 nIndex = 0;
    while ((nIndex = aStr.indexOf( sSep, nIndex)) != -1)
        aStr = aStr.replaceAt( nIndex, sSep.getLength(), "");
    sal_Int32 nNum = aStr.toInt32();
    if ( aStr.getLength() != 4 || nNum < m_pYearValueField->GetMin() || nNum > m_pYearValueField->GetMax() )
        aOutput += "????";
    else
    {
        nNum += 99;
        aOutput += OUString::number( nNum );
    }
    m_pToYearFT->SetText( aOutput );
}

IMPL_LINK_TYPED( OfaMiscTabPage, TwoFigureConfigFocusHdl, Control&, rControl, void )
{
    TwoFigureConfigHdl(static_cast<SpinField&>(rControl));
}
IMPL_LINK_TYPED( OfaMiscTabPage, TwoFigureConfigHdl, SpinField&, rEd, void )
{
    sal_Int64 nNum = m_pYearValueField->GetValue();
    OUString aOutput(OUString::number(nNum));
    m_pYearValueField->SetText(aOutput);
    m_pYearValueField->SetSelection( Selection( 0, aOutput.getLength() ) );
    TwoFigureHdl( static_cast<Edit&>(rEd) );
}

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

        Any propValue(
            makeAny( NamedValue(
                         OUString("nodepath"),
                         makeAny( OUString("/org.openoffice.Office.Canvas") ) ) ) );

        mxForceFlagNameAccess.set(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationUpdateAccess",
                Sequence<Any>( &propValue, 1 ) ),
            UNO_QUERY_THROW );

        propValue = makeAny(
            NamedValue(
                OUString("nodepath"),
                makeAny( OUString("/org.openoffice.Office.Canvas/CanvasServiceList") ) ) );

        Reference<XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );
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
                if( (xEntryNameAccess->getByName("PreferredImplementations") >>= preferredImplementations) )
                    maAvailableImplementations.push_back( std::make_pair(*pCurr,preferredImplementations) );
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
    if( OpenGLWrapper::isVCLOpenGLEnabled() )
        mbHWAccelAvailable = false;

    else if( !mbHWAccelChecked )
    {
        mbHWAccelChecked = true;

        Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();

        // check whether any of the service lists has an
        // implementation that presents the "HardwareAcceleration" property
        ServiceVector::const_iterator       aCurr=maAvailableImplementations.begin();
        const ServiceVector::const_iterator aEnd=maAvailableImplementations.end();
        while( aCurr != aEnd )
        {
            const OUString* pCurrImpl = aCurr->second.getConstArray();
            const OUString* const pEndImpl = pCurrImpl + aCurr->second.getLength();

            while( pCurrImpl != pEndImpl )
            {
                try
                {
                    Reference<XPropertySet> xPropSet( xFactory->createInstance(
                                                          pCurrImpl->trim() ),
                                                      UNO_QUERY_THROW );
                    bool bHasAccel(false);
                    if( (xPropSet->getPropertyValue("HardwareAcceleration") >>= bHasAccel) )
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

            ++aCurr;
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

    xNameReplace->replaceByName( "ForceSafeServiceImpl", makeAny(!_bEnabled) );

    Reference< XChangesBatch > xChangesBatch(
        mxForceFlagNameAccess, UNO_QUERY );

    if( !xChangesBatch.is() )
        return;

    xChangesBatch->commitChanges();
}

// class OfaViewTabPage --------------------------------------------------

OfaViewTabPage::OfaViewTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptViewPage", "cui/ui/optviewpage.ui", &rSet)
    , nSizeLB_InitialSelection(0)
    , nStyleLB_InitialSelection(0)
    , pAppearanceCfg(new SvtTabAppearanceCfg)
    , pCanvasSettings(new CanvasSettings)
    , mpDrawinglayerOpt(new SvtOptionsDrawinglayer)
    , mpOpenGLConfig(new svt::OpenGLCfg)
{
    get(m_pWindowSizeMF, "windowsize");
    get(m_pIconSizeLB, "iconsize");
    get(m_pIconStyleLB, "iconstyle");

    get(m_pFontAntiAliasing, "aafont");
    get(m_pAAPointLimitLabel, "aafrom");
    get(m_pAAPointLimit, "aanf");
    get(m_pMenuIconsLB, "menuicons");
    get(m_pFontShowCB, "showfontpreview");
    get(m_pUseHardwareAccell, "useaccel");
    get(m_pUseAntiAliase, "useaa");
    get(m_pUseOpenGL, "useopengl");
    get(m_pForceOpenGL, "forceopengl");
    get(m_pOpenGLStatusEnabled, "openglenabled");
    get(m_pOpenGLStatusDisabled, "opengldisabled");
    get(m_pMousePosLB, "mousepos");
    get(m_pMouseMiddleLB, "mousemiddle");

#if defined( UNX )
    m_pFontAntiAliasing->SetToggleHdl( LINK( this, OfaViewTabPage, OnAntialiasingToggled ) );
#else
    // on this platform, we do not have the anti aliasing options
    m_pFontAntiAliasing->Hide();
    m_pAAPointLimitLabel->Hide();
    m_pAAPointLimit->Hide();

#endif

    // Set known icon themes
    OUString sAutoStr( m_pIconStyleLB->GetEntry( 0 ) );
    m_pIconStyleLB->Clear();
    StyleSettings aStyleSettings = Application::GetSettings().GetStyleSettings();
    mInstalledIconThemes = aStyleSettings.GetInstalledIconThemes();

    // Start with the automatically chosen icon theme
    OUString autoThemeId = aStyleSettings.GetAutomaticallyChosenIconTheme();
    const vcl::IconThemeInfo& autoIconTheme = vcl::IconThemeInfo::FindIconThemeById(mInstalledIconThemes, autoThemeId);

    OUString entryForAuto = sAutoStr + " (" +
                                autoIconTheme.GetDisplayName() +
                                ")";
    m_pIconStyleLB->InsertEntry(entryForAuto);

    for (std::vector<vcl::IconThemeInfo>::const_iterator aI = mInstalledIconThemes.begin(); aI != mInstalledIconThemes.end(); ++aI) {
        m_pIconStyleLB->InsertEntry(aI->GetDisplayName());
    }

    // separate auto and other icon themes
    m_pIconStyleLB->SetSeparatorPos( 0 );
    m_pIconStyleLB->SelectEntryPos(0);

    // FIXME: should really add code to show a 'lock' icon here.
    if (officecfg::Office::Common::VCL::UseOpenGL::isReadOnly())
        m_pUseOpenGL->Enable(false);
    if (officecfg::Office::Common::VCL::ForceOpenGL::isReadOnly())
        m_pForceOpenGL->Enable(false);

    UpdateOGLStatus();
}

OfaViewTabPage::~OfaViewTabPage()
{
    disposeOnce();
}

void OfaViewTabPage::dispose()
{
    delete mpDrawinglayerOpt;
    mpDrawinglayerOpt = nullptr;
    delete pCanvasSettings;
    pCanvasSettings = nullptr;
    delete pAppearanceCfg;
    pAppearanceCfg = nullptr;
    m_pWindowSizeMF.clear();
    m_pIconSizeLB.clear();
    m_pIconStyleLB.clear();
    m_pFontAntiAliasing.clear();
    m_pAAPointLimitLabel.clear();
    m_pAAPointLimit.clear();
    m_pMenuIconsLB.clear();
    m_pFontShowCB.clear();
    m_pUseHardwareAccell.clear();
    m_pUseAntiAliase.clear();
    m_pUseOpenGL.clear();
    m_pForceOpenGL.clear();
    m_pOpenGLStatusEnabled.clear();
    m_pOpenGLStatusDisabled.clear();
    m_pMousePosLB.clear();
    m_pMouseMiddleLB.clear();
    SfxTabPage::dispose();
}

#if defined( UNX )
IMPL_LINK_NOARG_TYPED( OfaViewTabPage, OnAntialiasingToggled, CheckBox&, void )
{
    bool bAAEnabled = m_pFontAntiAliasing->IsChecked();

    m_pAAPointLimitLabel->Enable( bAAEnabled );
    m_pAAPointLimit->Enable( bAAEnabled );
}
#endif

VclPtr<SfxTabPage> OfaViewTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<OfaViewTabPage>::Create(pParent, *rAttrSet);
}

bool OfaViewTabPage::FillItemSet( SfxItemSet* )
{
    SvtFontOptions aFontOpt;
    SvtMenuOptions aMenuOpt;

    bool bModified = false;
    bool bMenuOptModified = false;
    bool bRepaintWindows(false);

    SvtMiscOptions aMiscOptions;
    const sal_Int32 nSizeLB_NewSelection = m_pIconSizeLB->GetSelectEntryPos();
    if( nSizeLB_InitialSelection != nSizeLB_NewSelection )
    {
        // from now on it's modified, even if via auto setting the same size was set as now selected in the LB
        sal_Int16 eSet = SFX_SYMBOLS_SIZE_AUTO;
        switch( nSizeLB_NewSelection )
        {
            case 0: eSet = SFX_SYMBOLS_SIZE_AUTO;  break;
            case 1: eSet = SFX_SYMBOLS_SIZE_SMALL; break;
            case 2: eSet = SFX_SYMBOLS_SIZE_LARGE; break;
            default:
                OSL_FAIL( "OfaViewTabPage::FillItemSet(): This state of m_pIconSizeLB should not be possible!" );
        }
        aMiscOptions.SetSymbolsSize( eSet );
    }

    const sal_Int32 nStyleLB_NewSelection = m_pIconStyleLB->GetSelectEntryPos();
    if( nStyleLB_InitialSelection != nStyleLB_NewSelection )
    {
        // 0 means choose style automatically
        if (nStyleLB_NewSelection == 0)
            aMiscOptions.SetIconTheme("auto");
        else
        {
            const sal_Int32 pos = m_pIconStyleLB->GetSelectEntryPos();
            const vcl::IconThemeInfo& iconThemeId = mInstalledIconThemes.at(pos-1);
            aMiscOptions.SetIconTheme(iconThemeId.GetThemeId());
        }
        nStyleLB_InitialSelection = nStyleLB_NewSelection;
    }

    bool bAppearanceChanged = false;


    // Screen Scaling
    sal_uInt16 nOldScale = pAppearanceCfg->GetScaleFactor();
    sal_uInt16 nNewScale = (sal_uInt16)m_pWindowSizeMF->GetValue();

    if ( nNewScale != nOldScale )
    {
        pAppearanceCfg->SetScaleFactor(nNewScale);
        bAppearanceChanged = true;
    }

    // Mouse Snap Mode
    short eOldSnap = pAppearanceCfg->GetSnapMode();
    short eNewSnap = m_pMousePosLB->GetSelectEntryPos();
    if(eNewSnap > 2)
        eNewSnap = 2;

    if ( eNewSnap != eOldSnap )
    {
        pAppearanceCfg->SetSnapMode(eNewSnap );
        bAppearanceChanged = true;
    }

    // Middle Mouse Button
    MouseMiddleButtonAction eOldMiddleMouse = pAppearanceCfg->GetMiddleMouseButton();
    short eNewMiddleMouse = m_pMouseMiddleLB->GetSelectEntryPos();
    if(eNewMiddleMouse > 2)
        eNewMiddleMouse = 2;

    if ( eNewMiddleMouse != static_cast<short>(eOldMiddleMouse) )
    {
        pAppearanceCfg->SetMiddleMouseButton( static_cast<MouseMiddleButtonAction>(eNewMiddleMouse) );
        bAppearanceChanged = true;
    }

#if defined( UNX )
    if ( m_pFontAntiAliasing->IsValueChangedFromSaved() )
    {
        pAppearanceCfg->SetFontAntiAliasing( m_pFontAntiAliasing->IsChecked() );
        bAppearanceChanged = true;
    }

    if ( m_pAAPointLimit->IsValueChangedFromSaved() )
    {
        pAppearanceCfg->SetFontAntialiasingMinPixelHeight( m_pAAPointLimit->GetValue() );
        bAppearanceChanged = true;
    }
#endif

    if ( m_pFontShowCB->IsValueChangedFromSaved() )
    {
        aFontOpt.EnableFontWYSIWYG( m_pFontShowCB->IsChecked() );
        bModified = true;
    }

    if(m_pMenuIconsLB->IsValueChangedFromSaved())
    {
        aMenuOpt.SetMenuIconsState(m_pMenuIconsLB->GetSelectEntryPos() == 0 ?
            TRISTATE_INDET :
            static_cast<TriState>(m_pMenuIconsLB->GetSelectEntryPos() - 1));
        bModified = true;
        bMenuOptModified = true;
        bAppearanceChanged = true;
    }

    // #i95644#  if disabled, do not use value, see in ::Reset()
    if(m_pUseHardwareAccell->IsEnabled())
    {
        if(m_pUseHardwareAccell->IsValueChangedFromSaved())
        {
            pCanvasSettings->EnabledHardwareAcceleration(m_pUseHardwareAccell->IsChecked());
            bModified = true;
        }
    }

    // #i95644#  if disabled, do not use value, see in ::Reset()
    if(m_pUseAntiAliase->IsEnabled())
    {
        if(m_pUseAntiAliase->IsChecked() != mpDrawinglayerOpt->IsAntiAliasing())
        {
            mpDrawinglayerOpt->SetAntiAliasing(m_pUseAntiAliase->IsChecked());
            bModified = true;
            bRepaintWindows = true;
        }
    }

    mpOpenGLConfig->setUseOpenGL(m_pUseOpenGL->IsChecked());
    mpOpenGLConfig->setForceOpenGL(m_pForceOpenGL->IsChecked());

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

    return bModified;
}

void OfaViewTabPage::Reset( const SfxItemSet* )
{
    SvtMiscOptions aMiscOptions;
    mpOpenGLConfig->reset();

    if( aMiscOptions.GetSymbolsSize() != SFX_SYMBOLS_SIZE_AUTO )
        nSizeLB_InitialSelection = ( aMiscOptions.AreCurrentSymbolsLarge() )? 2 : 1;
    m_pIconSizeLB->SelectEntryPos( nSizeLB_InitialSelection );
    m_pIconSizeLB->SaveValue();

    if (aMiscOptions.IconThemeWasSetAutomatically()) {
        nStyleLB_InitialSelection = 0;
    }
    else {
        const OUString& selected = aMiscOptions.GetIconTheme();
        const vcl::IconThemeInfo& selectedInfo =
                vcl::IconThemeInfo::FindIconThemeById(mInstalledIconThemes, selected);
        nStyleLB_InitialSelection = m_pIconStyleLB->GetEntryPos(selectedInfo.GetDisplayName());
    }

    m_pIconStyleLB->SelectEntryPos( nStyleLB_InitialSelection );
    m_pIconStyleLB->SaveValue();

    // Screen Scaling
    m_pWindowSizeMF->SetValue ( pAppearanceCfg->GetScaleFactor() );
    // Mouse Snap
    m_pMousePosLB->SelectEntryPos(pAppearanceCfg->GetSnapMode());
    m_pMousePosLB->SaveValue();

    // Mouse Snap
    m_pMouseMiddleLB->SelectEntryPos(static_cast<short>(pAppearanceCfg->GetMiddleMouseButton()));
    m_pMouseMiddleLB->SaveValue();

#if defined( UNX )
    m_pFontAntiAliasing->Check( pAppearanceCfg->IsFontAntiAliasing() );
    m_pAAPointLimit->SetValue( pAppearanceCfg->GetFontAntialiasingMinPixelHeight() );
#endif

    // WorkingSet
    SvtFontOptions aFontOpt;
    m_pFontShowCB->Check( aFontOpt.IsFontWYSIWYGEnabled() );
    SvtMenuOptions aMenuOpt;
    m_pMenuIconsLB->SelectEntryPos(aMenuOpt.GetMenuIconsState() == 2 ? 0 : aMenuOpt.GetMenuIconsState() + 1);
    m_pMenuIconsLB->SaveValue();

    { // #i95644# HW accel (unified to disable mechanism)
        if(pCanvasSettings->IsHardwareAccelerationAvailable())
        {
            m_pUseHardwareAccell->Check(pCanvasSettings->IsHardwareAccelerationEnabled());
            m_pUseHardwareAccell->Enable(!pCanvasSettings->IsHardwareAccelerationRO());
        }
        else
        {
            m_pUseHardwareAccell->Check(false);
            m_pUseHardwareAccell->Disable();
        }

        m_pUseHardwareAccell->SaveValue();
    }

    { // #i95644# AntiAliasing
        if(mpDrawinglayerOpt->IsAAPossibleOnThisSystem())
        {
            m_pUseAntiAliase->Check(mpDrawinglayerOpt->IsAntiAliasing());
        }
        else
        {
            m_pUseAntiAliase->Check(false);
            m_pUseAntiAliase->Disable();
        }

        m_pUseAntiAliase->SaveValue();
    }
    m_pUseOpenGL->Check(mpOpenGLConfig->useOpenGL());
    m_pForceOpenGL->Check(mpOpenGLConfig->forceOpenGL());

#if defined( UNX )
    m_pFontAntiAliasing->SaveValue();
    m_pAAPointLimit->SaveValue();
#endif
    m_pFontShowCB->SaveValue();

#if defined( UNX )
    LINK( this, OfaViewTabPage, OnAntialiasingToggled ).Call( *m_pFontAntiAliasing );
#endif
}

void OfaViewTabPage::UpdateOGLStatus()
{
    // Easier than a custom translation string.
    bool bEnabled = OpenGLWrapper::isVCLOpenGLEnabled();
    m_pOpenGLStatusEnabled->Show(bEnabled);
    m_pOpenGLStatusDisabled->Show(!bEnabled);
}

struct LanguageConfig_Impl
{
    SvtLanguageOptions aLanguageOptions;
    SvtSysLocaleOptions aSysLocaleOptions;
    SvtLinguConfig aLinguConfig;
};

static bool bLanguageCurrentDoc_Impl = false;

// some things we'll need...
static const char sAccessSrvc[] = "com.sun.star.configuration.ConfigurationAccess";
static const char sAccessUpdSrvc[] = "com.sun.star.configuration.ConfigurationUpdateAccess";
static const char sInstalledLocalesPath[] = "org.openoffice.Setup/Office/InstalledLocales";
static const char sUserLocalePath[] = "org.openoffice.Office.Linguistic/General";
static const char sUserLocaleKey[] = "UILocale";
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

OfaLanguagesTabPage::OfaLanguagesTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent,"OptLanguagesPage","cui/ui/optlanguagespage.ui", &rSet)
    , pLangConfig(new LanguageConfig_Impl)
    , m_bDatePatternsValid(false)
{
    get(m_pUserInterfaceLB, "userinterface");
    m_pUserInterfaceLB->SetStyle(m_pUserInterfaceLB->GetStyle() | WB_SORT);
    get(m_pLocaleSettingLB, "localesetting");
    m_pLocaleSettingLB->SetStyle(m_pLocaleSettingLB->GetStyle() | WB_SORT);
    get(m_pLocaleSettingFT, "localesettingFT");
    get(m_pDecimalSeparatorCB, "decimalseparator");
    get(m_pCurrencyFT, "defaultcurrency");
    get(m_pCurrencyLB, "currencylb");
    m_pCurrencyLB->SetStyle(m_pCurrencyLB->GetStyle() | WB_SORT);
    get(m_pDatePatternsFT,"dataaccpatterns");
    get(m_pDatePatternsED, "datepatterns");

    get(m_pWesternLanguageLB, "westernlanguage");
    m_pWesternLanguageLB->SetStyle(m_pWesternLanguageLB->GetStyle() | WB_SORT);
    get(m_pWesternLanguageFT, "western");
    get(m_pAsianLanguageLB, "asianlanguage");
    m_pAsianLanguageLB->SetStyle(m_pAsianLanguageLB->GetStyle() | WB_SORT);
    get(m_pComplexLanguageLB, "complexlanguage");
    m_pComplexLanguageLB->SetStyle(m_pComplexLanguageLB->GetStyle() | WB_SORT);
    get(m_pCurrentDocCB, "currentdoc");
    get(m_pAsianSupportCB, "asiansupport");
    get(m_pCTLSupportCB, "ctlsupport");
    get(m_pIgnoreLanguageChangeCB, "ignorelanguagechange");

    // initialize user interface language selection
    m_sSystemDefaultString = SvtLanguageTable::GetLanguageString( LANGUAGE_SYSTEM );

    OUString aUILang = m_sSystemDefaultString +
                       " - " +
                       SvtLanguageTable::GetLanguageString( Application::GetSettings().GetUILanguageTag().getLanguageType(), true );

    m_pUserInterfaceLB->InsertEntry(aUILang);
    m_pUserInterfaceLB->SetEntryData(0, nullptr);
    m_pUserInterfaceLB->SelectEntryPos(0);
    try
    {
        Reference< XMultiServiceFactory > theConfigProvider(
            css::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));
        Sequence< Any > theArgs(1);
        Reference< XNameAccess > theNameAccess;

        // find out which locales are currently installed and add them to the listbox
        theArgs[0] = makeAny(NamedValue(OUString("nodepath"), makeAny(OUString(sInstalledLocalesPath))));
        theNameAccess.set(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs ), UNO_QUERY_THROW );
        seqInstalledLanguages = theNameAccess->getElementNames();
        LanguageType aLang = LANGUAGE_DONTKNOW;
        for (sal_IntPtr i=0; i<seqInstalledLanguages.getLength(); i++)
        {
            aLang = LanguageTag::convertToLanguageTypeWithFallback(seqInstalledLanguages[i]);
            if (aLang != LANGUAGE_DONTKNOW)
            {
                //sal_uInt16 p = m_pUserInterfaceLB->InsertLanguage(aLang);
                OUString aLangStr( SvtLanguageTable::GetLanguageString( aLang, true ) );
                const sal_Int32 p = m_pUserInterfaceLB->InsertEntry(aLangStr);
                m_pUserInterfaceLB->SetEntryData(p, reinterpret_cast<void*>(i+1));
            }
        }

        // find out whether the user has a specific locale specified
        Sequence< Any > theArgs2(1);
        theArgs2[0] = makeAny(NamedValue(OUString("nodepath"), makeAny(OUString(sUserLocalePath))));
        theNameAccess.set(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs2 ), UNO_QUERY_THROW );
        if (theNameAccess->hasByName(sUserLocaleKey))
            theNameAccess->getByName(sUserLocaleKey) >>= m_sUserLocaleValue;
        // select the user specified locale in the listbox
        if (!m_sUserLocaleValue.isEmpty())
        {
            sal_Int32 d = 0;
            for (sal_Int32 i=0; i < m_pUserInterfaceLB->GetEntryCount(); i++)
            {
                d = (sal_Int32)reinterpret_cast<sal_IntPtr>(m_pUserInterfaceLB->GetEntryData(i));
                if ( d > 0 && seqInstalledLanguages.getLength() > d-1 && seqInstalledLanguages[d-1].equals(m_sUserLocaleValue))
                    m_pUserInterfaceLB->SelectEntryPos(i);
            }
        }

    }
    catch (const Exception &e)
    {
        // we'll just leave the box in it's default setting and won't
        // even give it event handler...
        SAL_WARN("cui.options", "ignoring Exception \"" << e.Message << "\"");
    }

    m_pWesternLanguageLB->SetLanguageList( SvxLanguageListFlags::WESTERN | SvxLanguageListFlags::ONLY_KNOWN, true, false, true );
    m_pWesternLanguageLB->InsertDefaultLanguage( css::i18n::ScriptType::LATIN );
    m_pAsianLanguageLB->SetLanguageList( SvxLanguageListFlags::CJK     | SvxLanguageListFlags::ONLY_KNOWN, true, false, true );
    m_pAsianLanguageLB->InsertDefaultLanguage( css::i18n::ScriptType::ASIAN );
    m_pComplexLanguageLB->SetLanguageList( SvxLanguageListFlags::CTL     | SvxLanguageListFlags::ONLY_KNOWN, true, false, true );
    m_pComplexLanguageLB->InsertDefaultLanguage( css::i18n::ScriptType::COMPLEX );

    m_pLocaleSettingLB->SetLanguageList( SvxLanguageListFlags::ALL     | SvxLanguageListFlags::ONLY_KNOWN, false );
    m_pLocaleSettingLB->InsertSystemLanguage( );

    const NfCurrencyTable& rCurrTab = SvNumberFormatter::GetTheCurrencyTable();
    const NfCurrencyEntry& rCurr = SvNumberFormatter::GetCurrencyEntry( LANGUAGE_SYSTEM );
    // insert SYSTEM entry
    OUString aDefaultCurr = m_sSystemDefaultString + " - " + rCurr.GetBankSymbol();
    m_pCurrencyLB->InsertEntry( aDefaultCurr );
    // all currencies
    OUString aTwoSpace( "  " );
    sal_uInt16 nCurrCount = rCurrTab.size();
    // first entry is SYSTEM, skip it
    for ( sal_uInt16 j=1; j < nCurrCount; ++j )
    {
        const NfCurrencyEntry* pCurr = &rCurrTab[j];
        OUString aStr_ = pCurr->GetBankSymbol() +
                         aTwoSpace +
                         pCurr->GetSymbol();
        aStr_ = ApplyLreOrRleEmbedding( aStr_ ) +
                aTwoSpace +
                ApplyLreOrRleEmbedding( SvtLanguageTable::GetLanguageString( pCurr->GetLanguage() ) );
        const sal_Int32 nPos = m_pCurrencyLB->InsertEntry( aStr_ );
        m_pCurrencyLB->SetEntryData( nPos, const_cast<NfCurrencyEntry *>(pCurr) );
    }

    m_pLocaleSettingLB->SetSelectHdl( LINK( this, OfaLanguagesTabPage, LocaleSettingHdl ) );
    m_pDatePatternsED->SetModifyHdl( LINK( this, OfaLanguagesTabPage, DatePatternsHdl ) );

    Link<Button*,void> aLink( LINK( this, OfaLanguagesTabPage, SupportHdl ) );
    m_pAsianSupportCB->SetClickHdl( aLink );
    m_pCTLSupportCB->SetClickHdl( aLink );

    m_bOldAsian = pLangConfig->aLanguageOptions.IsAnyEnabled();
    m_pAsianSupportCB->Check(m_bOldAsian);
    m_pAsianSupportCB->SaveValue();
    bool bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK);
    m_pAsianSupportCB->Enable(!bReadonly);
    SupportHdl( m_pAsianSupportCB );

    m_bOldCtl = pLangConfig->aLanguageOptions.IsCTLFontEnabled();
    m_pCTLSupportCB->Check(m_bOldCtl);
    m_pCTLSupportCB->SaveValue();
    bReadonly = pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT);
    m_pCTLSupportCB->Enable(!bReadonly);
    SupportHdl( m_pCTLSupportCB );

    m_pIgnoreLanguageChangeCB->Check( pLangConfig->aSysLocaleOptions.IsIgnoreLanguageChange() );
}

OfaLanguagesTabPage::~OfaLanguagesTabPage()
{
    disposeOnce();
}

void OfaLanguagesTabPage::dispose()
{
    delete pLangConfig;
    pLangConfig = nullptr;
    m_pUserInterfaceLB.clear();
    m_pLocaleSettingFT.clear();
    m_pLocaleSettingLB.clear();
    m_pDecimalSeparatorCB.clear();
    m_pCurrencyFT.clear();
    m_pCurrencyLB.clear();
    m_pDatePatternsFT.clear();
    m_pDatePatternsED.clear();
    m_pWesternLanguageLB.clear();
    m_pWesternLanguageFT.clear();
    m_pAsianLanguageLB.clear();
    m_pComplexLanguageLB.clear();
    m_pCurrentDocCB.clear();
    m_pAsianSupportCB.clear();
    m_pCTLSupportCB.clear();
    m_pIgnoreLanguageChangeCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaLanguagesTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<OfaLanguagesTabPage>::Create(pParent, *rAttrSet);
}

static void lcl_UpdateAndDelete(SfxVoidItem* pInvalidItems[], SfxBoolItem* pBoolItems[], sal_uInt16 nCount)
{
    SfxViewFrame* pCurrentFrm = SfxViewFrame::Current();
    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst();
    while(pViewFrm)
    {
        SfxBindings& rBind = pViewFrm->GetBindings();
        for(sal_Int16 i = 0; i < nCount; i++)
        {
            if(pCurrentFrm == pViewFrm)
                rBind.InvalidateAll(false);
            rBind.SetState( *pInvalidItems[i] );
            rBind.SetState( *pBoolItems[i] );
        }
        pViewFrm = SfxViewFrame::GetNext(*pViewFrm);
    }
    for(sal_Int16 i = 0; i < nCount; i++)
    {
        delete pInvalidItems[i];
        delete pBoolItems[i] ;
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
         m_pCTLSupportCB->IsChecked() &&
         (m_pCTLSupportCB->GetSavedValue() != TRISTATE_TRUE ||
         m_pComplexLanguageLB->IsValueChangedFromSaved())
       )
    {
        //sequence checking has to be switched on depending on the selected CTL language
        LanguageType eCTLLang = m_pComplexLanguageLB->GetSelectLanguage();
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
        sal_Int32 d = (sal_Int32)reinterpret_cast<sal_IntPtr>(m_pUserInterfaceLB->GetSelectEntryData());
        if( d > 0 && seqInstalledLanguages.getLength() > d-1)
            aLangString = seqInstalledLanguages[d-1];

        /*
        if( m_pUserInterfaceLB->GetSelectEntryPos() > 0)
            aLangString = ConvertLanguageToIsoString(m_pUserInterfaceLB->GetSelectLanguage());
        */
        Reference< XMultiServiceFactory > theConfigProvider(
            css::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));
        Sequence< Any > theArgs(1);
        theArgs[0] = makeAny(NamedValue(OUString("nodepath"), makeAny(OUString(sUserLocalePath))));
        Reference< XPropertySet >xProp(
            theConfigProvider->createInstanceWithArguments(sAccessUpdSrvc, theArgs ), UNO_QUERY_THROW );
        if ( !m_sUserLocaleValue.equals(aLangString))
        {
            // OSL_FAIL("UserInterface language was changed, restart.");
            // write new value
            xProp->setPropertyValue(sUserLocaleKey, makeAny(aLangString));
            Reference< XChangesBatch >(xProp, UNO_QUERY_THROW)->commitChanges();
            // display info
            ScopedVclPtrInstance< MessageDialog > aBox(this, CUI_RES(RID_SVXSTR_LANGUAGE_RESTART), VCL_MESSAGE_INFO);
            aBox->Execute();

            // tell quickstarter to stop being a veto listener

            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext());
            css::office::Quickstart::createAndSetVeto(xContext, false, false, false/*DisableVeto*/);
        }
    }
    catch (const Exception& e)
    {
        // we'll just leave the box in it's default setting and won't
        // even give it event handler...
        SAL_WARN("cui.options", "ignoring Exception \"" << e.Message << "\"");
    }

    OUString sLang = pLangConfig->aSysLocaleOptions.GetLocaleConfigString();
    LanguageType eOldLocale = (!sLang.isEmpty() ?
        LanguageTag::convertToLanguageTypeWithFallback( sLang ) : LANGUAGE_SYSTEM);
    LanguageType eNewLocale = m_pLocaleSettingLB->GetSelectLanguage();

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
        aCompatOpts.SetDefault( COMPATIBILITY_PROPERTYNAME_EXPANDWORDSPACE, !bNewCJK );
    }

    if(m_pDecimalSeparatorCB->IsValueChangedFromSaved())
        pLangConfig->aSysLocaleOptions.SetDecimalSeparatorAsLocale(m_pDecimalSeparatorCB->IsChecked());

    if(m_pIgnoreLanguageChangeCB->IsValueChangedFromSaved())
        pLangConfig->aSysLocaleOptions.SetIgnoreLanguageChange(m_pIgnoreLanguageChangeCB->IsChecked());

    // Configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default.
    OUString sOldCurr = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    const NfCurrencyEntry* pCurr = static_cast<const NfCurrencyEntry*>(m_pCurrencyLB->GetSelectEntryData());
    OUString sNewCurr;
    if ( pCurr )
        sNewCurr = SvtSysLocaleOptions::CreateCurrencyConfigString(
            pCurr->GetBankSymbol(), pCurr->GetLanguage() );
    if ( sOldCurr != sNewCurr )
        pLangConfig->aSysLocaleOptions.SetCurrencyConfigString( sNewCurr );

    // Configured date acceptance patterns, for example Y-M-D;M-D or empty for
    // locale default.
    if (m_bDatePatternsValid && m_pDatePatternsED->IsValueChangedFromSaved())
        pLangConfig->aSysLocaleOptions.SetDatePatternsConfigString( m_pDatePatternsED->GetText());

    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    Reference< css::linguistic2::XLinguProperties > xLinguProp = LinguMgr::GetLinguPropertySet();
    bool bCurrentDocCBChecked = m_pCurrentDocCB->IsChecked();
    if(m_pCurrentDocCB->IsEnabled())
        bLanguageCurrentDoc_Impl = bCurrentDocCBChecked;
    bool bCurrentDocCBChanged = m_pCurrentDocCB->IsValueChangedFromSaved();

    bool bValChanged = m_pWesternLanguageLB->IsValueChangedFromSaved();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_pWesternLanguageLB->GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag::convertToLocale( eSelectLang, false);
            aValue <<= aLocale;
            OUString aPropName( "DefaultLocale" );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet->Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, css::i18n::ScriptType::LATIN),
                SID_ATTR_LANGUAGE));
        }
    }
    bValChanged = m_pAsianLanguageLB->IsValueChangedFromSaved();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_pAsianLanguageLB->GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag::convertToLocale( eSelectLang, false);
            aValue <<= aLocale;
            OUString aPropName( "DefaultLocale_CJK" );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale_CJK( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet->Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, css::i18n::ScriptType::ASIAN),
                SID_ATTR_CHAR_CJK_LANGUAGE));
        }
    }
    bValChanged = m_pComplexLanguageLB->IsValueChangedFromSaved();
    if( (bCurrentDocCBChanged && !bCurrentDocCBChecked) || bValChanged)
    {
        LanguageType eSelectLang = m_pComplexLanguageLB->GetSelectLanguage();
        if(!bCurrentDocCBChecked)
        {
            Any aValue;
            Locale aLocale = LanguageTag::convertToLocale( eSelectLang, false);
            aValue <<= aLocale;
            OUString aPropName( "DefaultLocale_CTL" );
            pLangConfig->aLinguConfig.SetProperty( aPropName, aValue );
            if (xLinguProp.is())
                xLinguProp->setDefaultLocale_CTL( aLocale );
        }
        if(pCurrentDocShell)
        {
            rSet->Put(SvxLanguageItem(MsLangId::resolveSystemLanguageByScriptType(eSelectLang, css::i18n::ScriptType::COMPLEX),
                SID_ATTR_CHAR_CTL_LANGUAGE));
        }
    }

    if(m_pAsianSupportCB->IsValueChangedFromSaved() )
    {
        bool bChecked = m_pAsianSupportCB->IsChecked();
        pLangConfig->aLanguageOptions.SetAll(bChecked);

        //iterate over all bindings to invalidate vertical text direction
        const sal_uInt16 STATE_COUNT = 2;

        SfxBoolItem* pBoolItems[STATE_COUNT];
        pBoolItems[0] = new SfxBoolItem(SID_VERTICALTEXT_STATE, false);
        pBoolItems[1] = new SfxBoolItem(SID_TEXT_FITTOSIZE_VERTICAL, false);

        SfxVoidItem* pInvalidItems[STATE_COUNT];
        pInvalidItems[0] = new SfxVoidItem(SID_VERTICALTEXT_STATE);
        pInvalidItems[1] = new SfxVoidItem(SID_TEXT_FITTOSIZE_VERTICAL);

        lcl_UpdateAndDelete(pInvalidItems, pBoolItems, STATE_COUNT);
    }

    if ( m_pCTLSupportCB->IsValueChangedFromSaved() )
    {
        SvtSearchOptions aOpt;
        aOpt.SetIgnoreDiacritics_CTL(true);
        aOpt.SetIgnoreKashida_CTL(true);
        aOpt.Commit();
        pLangConfig->aLanguageOptions.SetCTLFontEnabled( m_pCTLSupportCB->IsChecked() );

        const sal_uInt16 STATE_COUNT = 1;
        SfxBoolItem* pBoolItems[STATE_COUNT];
        pBoolItems[0] = new SfxBoolItem(SID_CTLFONT_STATE, false);
        SfxVoidItem* pInvalidItems[STATE_COUNT];
        pInvalidItems[0] = new SfxVoidItem(SID_CTLFONT_STATE);
        lcl_UpdateAndDelete(pInvalidItems, pBoolItems, STATE_COUNT);
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
    OUString sLang = pLangConfig->aSysLocaleOptions.GetLocaleConfigString();
    if ( !sLang.isEmpty() )
        m_pLocaleSettingLB->SelectLanguage(LanguageTag::convertToLanguageTypeWithFallback(sLang));
    else
        m_pLocaleSettingLB->SelectLanguage( LANGUAGE_USER_SYSTEM_CONFIG );
    bool bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_LOCALE);
    m_pLocaleSettingLB->Enable(!bReadonly);
    m_pLocaleSettingFT->Enable(!bReadonly);


    m_pDecimalSeparatorCB->Check( pLangConfig->aSysLocaleOptions.IsDecimalSeparatorAsLocale());
    m_pDecimalSeparatorCB->SaveValue();

    m_pIgnoreLanguageChangeCB->Check( pLangConfig->aSysLocaleOptions.IsIgnoreLanguageChange());
    m_pIgnoreLanguageChangeCB->SaveValue();

    // let LocaleSettingHdl enable/disable checkboxes for CJK/CTL support
    // #i15812# must be done *before* the configured currency is set
    // and update the decimal separator used for the given locale
    LocaleSettingHdl(*m_pLocaleSettingLB);

    // configured currency, for example, USD-en-US or EUR-de-DE, or empty for locale default
    OUString aAbbrev;
    LanguageType eLang;
    const NfCurrencyEntry* pCurr = nullptr;
    sLang = pLangConfig->aSysLocaleOptions.GetCurrencyConfigString();
    if ( !sLang.isEmpty() )
    {
        SvtSysLocaleOptions::GetCurrencyAbbrevAndLanguage( aAbbrev, eLang, sLang );
        pCurr = SvNumberFormatter::GetCurrencyEntry( aAbbrev, eLang );
    }
    // if pCurr==NULL the SYSTEM entry is selected
    const sal_Int32 nPos = m_pCurrencyLB->GetEntryPos( static_cast<void const *>(pCurr) );
    m_pCurrencyLB->SelectEntryPos( nPos );
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_CURRENCY);
    m_pCurrencyLB->Enable(!bReadonly);
    m_pCurrencyFT->Enable(!bReadonly);

    // date acceptance patterns
    OUString aDatePatternsString = pLangConfig->aSysLocaleOptions.GetDatePatternsConfigString();
    if (aDatePatternsString.isEmpty())
    {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        aDatePatternsString = lcl_getDatePatternsConfigString( rLocaleWrapper);
    }
    // Let's assume patterns are valid at this point.
    m_bDatePatternsValid = true;
    m_pDatePatternsED->SetText( aDatePatternsString);
    bReadonly = pLangConfig->aSysLocaleOptions.IsReadOnly(SvtSysLocaleOptions::E_DATEPATTERNS);
    m_pDatePatternsED->Enable(!bReadonly);
    m_pDatePatternsFT->Enable(!bReadonly);
    m_pDatePatternsED->SaveValue();

    //western/CJK/CLK language
    LanguageType eCurLang = LANGUAGE_NONE;
    LanguageType eCurLangCJK = LANGUAGE_NONE;
    LanguageType eCurLangCTL = LANGUAGE_NONE;
    SfxObjectShell* pCurrentDocShell = SfxObjectShell::Current();
    //collect the configuration values first
    m_pCurrentDocCB->Enable(false);

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
        m_pCurrentDocCB->Enable();
        m_pCurrentDocCB->Check(bLanguageCurrentDoc_Impl);
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
        m_pWesternLanguageLB->SelectLanguage(LANGUAGE_NONE);
    else
        m_pWesternLanguageLB->SelectLanguage(eCurLang);

    if(LANGUAGE_NONE == eCurLangCJK || LANGUAGE_DONTKNOW == eCurLangCJK)
        m_pAsianLanguageLB->SelectLanguage(LANGUAGE_NONE);
    else
        m_pAsianLanguageLB->SelectLanguage(eCurLangCJK);

    if(LANGUAGE_NONE == eCurLangCTL || LANGUAGE_DONTKNOW == eCurLangCTL)
        m_pComplexLanguageLB->SelectLanguage(LANGUAGE_NONE);
    else
        m_pComplexLanguageLB->SelectLanguage(eCurLangCTL);

    m_pWesternLanguageLB->SaveValue();
    m_pAsianLanguageLB->SaveValue();
    m_pComplexLanguageLB->SaveValue();
    m_pIgnoreLanguageChangeCB->SaveValue();
    m_pCurrentDocCB->SaveValue();

    bool bEnable = !pLangConfig->aLinguConfig.IsReadOnly( "DefaultLocale" );
    m_pWesternLanguageFT->Enable( bEnable );
    m_pWesternLanguageLB->Enable( bEnable );



    // #i15812# controls for CJK/CTL already enabled/disabled from LocaleSettingHdl
#if 0
    bEnable = ( !pLangConfig->aLinguConfig.IsReadOnly( "DefaultLocale_CJK" ) && m_pAsianSupportCB->IsChecked() );
    m_pAsianLanguageLB->Enable( bEnable );

    bEnable = ( !pLangConfig->aLinguConfig.IsReadOnly( "DefaultLocale_CTL" ) && m_pCTLSupportCB->IsChecked() );
    m_pComplexLanguageLB->Enable( bEnable );
#endif
    // check the box "For the current document only"
    // set the focus to the Western Language box
    const SfxPoolItem* pLang = nullptr;
    if ( SfxItemState::SET == rSet->GetItemState(SID_SET_DOCUMENT_LANGUAGE, false, &pLang ) && static_cast<const SfxBoolItem*>(pLang)->GetValue() )
    {
        m_pWesternLanguageLB->GrabFocus();
        m_pCurrentDocCB->Enable();
        m_pCurrentDocCB->Check();
    }
}

IMPL_LINK_TYPED(  OfaLanguagesTabPage, SupportHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    DBG_ASSERT( pBox, "OfaLanguagesTabPage::SupportHdl(): pBox invalid" );
    bool bCheck = pBox->IsChecked();
    if ( m_pAsianSupportCB == pBox )
    {
        bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly("DefaultLocale_CJK");
        bCheck = ( bCheck && !bReadonly );
        m_pAsianLanguageLB->Enable( bCheck );
        if( pBox->IsEnabled() )
            m_bOldAsian = bCheck;
    }
    else if ( m_pCTLSupportCB == pBox )
    {
        bool bReadonly = pLangConfig->aLinguConfig.IsReadOnly("DefaultLocale_CTL");
        bCheck = ( bCheck && !bReadonly  );
        m_pComplexLanguageLB->Enable( bCheck );
        if( pBox->IsEnabled() )
            m_bOldCtl = bCheck;
    }
    else
        SAL_WARN( "cui.options", "OfaLanguagesTabPage::SupportHdl(): wrong pBox" );
}

namespace
{
    void lcl_checkLanguageCheckBox(CheckBox* _rCB,bool _bNewValue,bool _bOldValue)
    {
        if ( _bNewValue )
            _rCB->Check();
        else
            _rCB->Check( _bOldValue );
// #i15082# do not call SaveValue() in running dialog...
//      _rCB.SaveValue();
        _rCB->Enable( !_bNewValue );
    }
}

IMPL_LINK_TYPED( OfaLanguagesTabPage, LocaleSettingHdl, ListBox&, rListBox, void )
{
    SvxLanguageBox* pBox = static_cast<SvxLanguageBox*>(&rListBox);
    LanguageType eLang = pBox->GetSelectLanguage();
    SvtScriptType nType = SvtLanguageOptions::GetScriptTypeOfLanguage(eLang);
    // first check if CTL must be enabled
    // #103299# - if CTL font setting is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_CTLFONT))
    {
        bool bIsCTLFixed = bool(nType & SvtScriptType::COMPLEX);
        lcl_checkLanguageCheckBox(m_pCTLSupportCB, bIsCTLFixed, m_bOldCtl);
        SupportHdl( m_pCTLSupportCB );
    }
    // second check if CJK must be enabled
    // #103299# - if CJK support is not readonly
    if(!pLangConfig->aLanguageOptions.IsReadOnly(SvtLanguageOptions::E_ALLCJK))
    {
        bool bIsCJKFixed = bool(nType & SvtScriptType::ASIAN);
        lcl_checkLanguageCheckBox(m_pAsianSupportCB, bIsCJKFixed, m_bOldAsian);
        SupportHdl( m_pAsianSupportCB );
    }

    const NfCurrencyEntry* pCurr = &SvNumberFormatter::GetCurrencyEntry(
            ((eLang == LANGUAGE_USER_SYSTEM_CONFIG) ? MsLangId::getSystemLanguage() : eLang));
    sal_Int32 nPos = m_pCurrencyLB->GetEntryPos( nullptr );
    if (pCurr)
    {
        // Update the "Default ..." currency.
        m_pCurrencyLB->RemoveEntry( nPos );
        OUString aDefaultCurr = m_sSystemDefaultString + " - " + pCurr->GetBankSymbol();
        nPos = m_pCurrencyLB->InsertEntry( aDefaultCurr );
    }
    m_pCurrencyLB->SelectEntryPos( nPos );

    // obtain corresponding locale data
    LanguageTag aLanguageTag( eLang);
    LocaleDataWrapper aLocaleWrapper( aLanguageTag );

    // update the decimal separator key of the related CheckBox
    OUString sTempLabel(m_pDecimalSeparatorCB->GetText());
    sTempLabel = sTempLabel.replaceFirst("%1", aLocaleWrapper.getNumDecimalSep() );
    m_pDecimalSeparatorCB->SetText(sTempLabel);

    // update the date acceptance patterns
    OUString aDatePatternsString = lcl_getDatePatternsConfigString( aLocaleWrapper);
    m_bDatePatternsValid = true;
    m_pDatePatternsED->SetText( aDatePatternsString);
}

IMPL_LINK_TYPED( OfaLanguagesTabPage, DatePatternsHdl, Edit&, rEd, void )
{
    const OUString aPatterns( rEd.GetText());
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
    {
        // Do not use SetText(...,GetSelection()) because internally the
        // reference's pointer of the selection is obtained resulting in the
        // entire text being selected at the end.
        Selection aSelection( rEd.GetSelection());
        rEd.SetText( aBuf.makeStringAndClear(), aSelection);
    }
    if (bValid)
    {
        rEd.SetControlForeground();
        rEd.SetControlBackground();
    }
    else
    {
#if 0
        //! Gives white on white!?! instead of white on reddish.
        rEd.SetControlBackground( ::Color( RGB_COLORDATA( 0xff, 0x65, 0x63)));
        rEd.SetControlForeground( ::Color( COL_WHITE));
#else
        rEd.SetControlForeground( ::Color( RGB_COLORDATA( 0xf0, 0, 0)));
#endif
    }
    m_bDatePatternsValid = bValid;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

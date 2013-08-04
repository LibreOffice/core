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


#include <accessibilityoptions.hxx>

#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>

#include <svl/smplhint.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <rtl/instance.hxx>

#include <itemholder2.hxx>

using namespace utl;
using namespace com::sun::star::uno;

#define HELP_TIP_TIMEOUT 0xffff     // max. timeout setting to pretend a non-timeout

namespace
{
    const char s_sAccessibility[] = "org.openoffice.Office.Common/Accessibility";
    const char s_sAutoDetectSystemHC[] = "AutoDetectSystemHC";
    const char s_sIsForPagePreviews[] = "IsForPagePreviews";
    const char s_sIsHelpTipsDisappear[] = "IsHelpTipsDisappear";
    const char s_sHelpTipSeconds[] = "HelpTipSeconds";
    const char s_sIsAllowAnimatedGraphics[] = "IsAllowAnimatedGraphics";
    const char s_sIsAllowAnimatedText[] = "IsAllowAnimatedText";
    const char s_sIsAutomaticFontColor[] = "IsAutomaticFontColor";
    const char s_sIsSystemFont[] = "IsSystemFont";
    const char s_sIsSelectionInReadonly[] = "IsSelectionInReadonly";
    const char s_sColorValueSetMaximumRowCount[] = "ColorValueSetMaximumRowCount";
    const char s_sColorValueSetEntryEdgeLength[] = "ColorValueSetEntryEdgeLength";
    const char s_sColorValueSetColumnCount[] = "ColorValueSetColumnCount";
    const char s_sEdgeBlending[] = "EdgeBlending";
    const char s_sListBoxMaximumLineCount[] = "ListBoxMaximumLineCount";
    const char s_sPreviewUsesCheckeredBackground[]  = "PreviewUsesCheckeredBackground";
}

// class SvtAccessibilityOptions_Impl ---------------------------------------------

class SvtAccessibilityOptions_Impl
{
private:
    css::uno::Reference< css::container::XNameAccess > m_xCfg;
    sal_Bool                                           bIsModified;

public:
    SvtAccessibilityOptions_Impl();
    ~SvtAccessibilityOptions_Impl();

    void        SetVCLSettings();
    sal_Bool    GetAutoDetectSystemHC();
    sal_Bool    GetIsForPagePreviews() const;
    sal_Bool    GetIsHelpTipsDisappear() const;
    sal_Bool    GetIsAllowAnimatedGraphics() const;
    sal_Bool    GetIsAllowAnimatedText() const;
    sal_Bool    GetIsAutomaticFontColor() const;
    sal_Bool    GetIsSystemFont() const;
    sal_Int16   GetHelpTipSeconds() const;
    sal_Bool    IsSelectionInReadonly() const;
    sal_Int16   GetEdgeBlending() const;
    sal_Int16   GetListBoxMaximumLineCount() const;
    sal_Int16   GetColorValueSetColumnCount() const;
    sal_Bool    GetPreviewUsesCheckeredBackground() const;

    void        SetAutoDetectSystemHC(sal_Bool bSet);
    void        SetIsForPagePreviews(sal_Bool bSet);
    void        SetIsHelpTipsDisappear(sal_Bool bSet);
    void        SetIsAllowAnimatedGraphics(sal_Bool bSet);
    void        SetIsAllowAnimatedText(sal_Bool bSet);
    void        SetIsAutomaticFontColor(sal_Bool bSet);
    void        SetIsSystemFont(sal_Bool bSet);
    void        SetHelpTipSeconds(sal_Int16 nSet);
    void        SetSelectionInReadonly(sal_Bool bSet);
    void        SetEdgeBlending(sal_Int16 nSet);
    void        SetListBoxMaximumLineCount(sal_Int16 nSet);
    void        SetColorValueSetColumnCount(sal_Int16 nSet);
    void        SetPreviewUsesCheckeredBackground(sal_Bool bSet);

    sal_Bool    IsModified() const { return bIsModified; };
};

// initialization of static members --------------------------------------

SvtAccessibilityOptions_Impl* SvtAccessibilityOptions::sm_pSingleImplConfig =NULL;
sal_Int32                     SvtAccessibilityOptions::sm_nAccessibilityRefCount(0);

namespace
{
    struct SingletonMutex
        : public rtl::Static< ::osl::Mutex, SingletonMutex > {};
}

// -----------------------------------------------------------------------
// class SvtAccessibilityOptions_Impl ---------------------------------------------

SvtAccessibilityOptions_Impl::SvtAccessibilityOptions_Impl()
{
    try
    {
        m_xCfg = css::uno::Reference< css::container::XNameAccess >(
            ::comphelper::ConfigurationHelper::openConfig(
            comphelper::getProcessComponentContext(),
            s_sAccessibility,
            ::comphelper::ConfigurationHelper::E_STANDARD),
            css::uno::UNO_QUERY);

        bIsModified = sal_False;
    }
    catch(const css::uno::Exception& ex)
    {
        m_xCfg.clear();
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

SvtAccessibilityOptions_Impl::~SvtAccessibilityOptions_Impl()
{
}

// -----------------------------------------------------------------------
sal_Bool SvtAccessibilityOptions_Impl::GetAutoDetectSystemHC()
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_True;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sAutoDetectSystemHC) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

sal_Bool SvtAccessibilityOptions_Impl::GetIsForPagePreviews() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_True;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sIsForPagePreviews) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
    return bRet;
}

sal_Bool SvtAccessibilityOptions_Impl::GetIsHelpTipsDisappear() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_True;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sIsHelpTipsDisappear) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

sal_Bool SvtAccessibilityOptions_Impl::GetIsAllowAnimatedGraphics() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_True;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sIsAllowAnimatedGraphics) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

sal_Bool SvtAccessibilityOptions_Impl::GetIsAllowAnimatedText() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_True;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sIsAllowAnimatedText) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

sal_Bool SvtAccessibilityOptions_Impl::GetIsAutomaticFontColor() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_False;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sIsAutomaticFontColor) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

sal_Bool SvtAccessibilityOptions_Impl::GetIsSystemFont() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_True;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sIsSystemFont) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

sal_Int16 SvtAccessibilityOptions_Impl::GetHelpTipSeconds() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Int16                                       nRet = 4;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sHelpTipSeconds) >>= nRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return nRet;
}

sal_Bool SvtAccessibilityOptions_Impl::IsSelectionInReadonly() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool                                        bRet = sal_False;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sIsSelectionInReadonly) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

sal_Int16 SvtAccessibilityOptions_Impl::GetEdgeBlending() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Int16 nRet = 35;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sEdgeBlending) >>= nRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }

    return nRet;
}

sal_Int16 SvtAccessibilityOptions_Impl::GetListBoxMaximumLineCount() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Int16 nRet = 25;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sListBoxMaximumLineCount) >>= nRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }

    return nRet;
}

sal_Int16 SvtAccessibilityOptions_Impl::GetColorValueSetColumnCount() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Int16 nRet = 12;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sColorValueSetColumnCount) >>= nRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }

    return nRet;
}

sal_Bool SvtAccessibilityOptions_Impl::GetPreviewUsesCheckeredBackground() const
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bRet = sal_False;

    try
    {
        if(xNode.is())
            xNode->getPropertyValue(s_sPreviewUsesCheckeredBackground) >>= bRet;
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }

    return bRet;
}

void SvtAccessibilityOptions_Impl::SetAutoDetectSystemHC(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sAutoDetectSystemHC)!=bSet)
        {
            xNode->setPropertyValue(s_sAutoDetectSystemHC, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetIsForPagePreviews(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sIsForPagePreviews)!=bSet)
        {
            xNode->setPropertyValue(s_sIsForPagePreviews, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetIsHelpTipsDisappear(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sIsHelpTipsDisappear)!=bSet)
        {
            xNode->setPropertyValue(s_sIsHelpTipsDisappear, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetIsAllowAnimatedGraphics(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sIsAllowAnimatedGraphics)!=bSet)
        {
            xNode->setPropertyValue(s_sIsAllowAnimatedGraphics, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetIsAllowAnimatedText(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sIsAllowAnimatedText)!=bSet)
        {
            xNode->setPropertyValue(s_sIsAllowAnimatedText, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetIsAutomaticFontColor(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sIsAutomaticFontColor)!=bSet)
        {
            xNode->setPropertyValue(s_sIsAutomaticFontColor, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetIsSystemFont(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sIsSystemFont)!=bSet)
        {
            xNode->setPropertyValue(s_sIsSystemFont, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetHelpTipSeconds(sal_Int16 nSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sHelpTipSeconds)!=nSet)
        {
            xNode->setPropertyValue(s_sHelpTipSeconds, css::uno::makeAny(nSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetSelectionInReadonly(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sIsSelectionInReadonly)!=bSet)
        {
            xNode->setPropertyValue(s_sIsSelectionInReadonly, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools.config", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetVCLSettings()
{
    AllSettings aAllSettings(Application::GetSettings());
    StyleSettings aStyleSettings(aAllSettings.GetStyleSettings());
    HelpSettings aHelpSettings(aAllSettings.GetHelpSettings());
    bool StyleSettingsChanged(false);

    aHelpSettings.SetTipTimeout( GetIsHelpTipsDisappear() ? GetHelpTipSeconds() * 1000 : HELP_TIP_TIMEOUT);
    aAllSettings.SetHelpSettings(aHelpSettings);

    if(aStyleSettings.GetUseSystemUIFonts() != GetIsSystemFont())
    {
        aStyleSettings.SetUseSystemUIFonts(GetIsSystemFont());
        StyleSettingsChanged = true;
    }

    const sal_Int16 nEdgeBlendingCountA(GetEdgeBlending());
    OSL_ENSURE(nEdgeBlendingCountA >= 0, "OOps, negative values for EdgeBlending are not allowed (!)");
    const sal_uInt16 nEdgeBlendingCountB(static_cast< sal_uInt16 >(nEdgeBlendingCountA >= 0 ? nEdgeBlendingCountA : 0));

    if(aStyleSettings.GetEdgeBlending() != nEdgeBlendingCountB)
    {
        aStyleSettings.SetEdgeBlending(nEdgeBlendingCountB);
        StyleSettingsChanged = true;
    }

    const sal_Int16 nMaxLineCountA(GetListBoxMaximumLineCount());
    OSL_ENSURE(nMaxLineCountA >= 0, "OOps, negative values for ListBoxMaximumLineCount are not allowed (!)");
    const sal_uInt16 nMaxLineCountB(static_cast< sal_uInt16 >(nMaxLineCountA >= 0 ? nMaxLineCountA : 0));

    if(aStyleSettings.GetListBoxMaximumLineCount() != nMaxLineCountB)
    {
        aStyleSettings.SetListBoxMaximumLineCount(nMaxLineCountB);
        StyleSettingsChanged = true;
    }

    const sal_Int16 nMaxColumnCountA(GetColorValueSetColumnCount());
    OSL_ENSURE(nMaxColumnCountA >= 0, "OOps, negative values for ColorValueSetColumnCount are not allowed (!)");
    const sal_uInt16 nMaxColumnCountB(static_cast< sal_uInt16 >(nMaxColumnCountA >= 0 ? nMaxColumnCountA : 0));

    if(aStyleSettings.GetColorValueSetColumnCount() != nMaxColumnCountB)
    {
        aStyleSettings.SetColorValueSetColumnCount(nMaxColumnCountB);
        StyleSettingsChanged = true;
    }

    const bool bPreviewUsesCheckeredBackground(GetPreviewUsesCheckeredBackground());

    if(aStyleSettings.GetPreviewUsesCheckeredBackground() != bPreviewUsesCheckeredBackground)
    {
        aStyleSettings.SetPreviewUsesCheckeredBackground(bPreviewUsesCheckeredBackground);
        StyleSettingsChanged = true;
    }

    if(StyleSettingsChanged)
    {
        aAllSettings.SetStyleSettings(aStyleSettings);
        Application::MergeSystemSettings(aAllSettings);
    }

    Application::SetSettings(aAllSettings);
}

void SvtAccessibilityOptions_Impl::SetEdgeBlending(sal_Int16 nSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sEdgeBlending)!=nSet)
        {
            xNode->setPropertyValue(s_sEdgeBlending, css::uno::makeAny(nSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetListBoxMaximumLineCount(sal_Int16 nSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sListBoxMaximumLineCount)!=nSet)
        {
            xNode->setPropertyValue(s_sListBoxMaximumLineCount, css::uno::makeAny(nSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetColorValueSetColumnCount(sal_Int16 nSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sColorValueSetColumnCount)!=nSet)
        {
            xNode->setPropertyValue(s_sColorValueSetColumnCount, css::uno::makeAny(nSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }
}

void SvtAccessibilityOptions_Impl::SetPreviewUsesCheckeredBackground(sal_Bool bSet)
{
    css::uno::Reference< css::beans::XPropertySet > xNode(m_xCfg, css::uno::UNO_QUERY);

    try
    {
        if(xNode.is() && xNode->getPropertyValue(s_sPreviewUsesCheckeredBackground)!=bSet)
        {
            xNode->setPropertyValue(s_sPreviewUsesCheckeredBackground, css::uno::makeAny(bSet));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);

            bIsModified = sal_True;
        }
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN("svtools", "Caught unexpected: " << ex.Message);
    }
}

// -----------------------------------------------------------------------
// class SvtAccessibilityOptions --------------------------------------------------

SvtAccessibilityOptions::SvtAccessibilityOptions()
{
    {
        ::osl::MutexGuard aGuard( SingletonMutex::get() );
        if(!sm_pSingleImplConfig)
        {
            sm_pSingleImplConfig = new SvtAccessibilityOptions_Impl;
            svtools::ItemHolder2::holdConfigItem(E_ACCESSIBILITYOPTIONS);
        }
        ++sm_nAccessibilityRefCount;
    }
    //StartListening( *sm_pSingleImplConfig, sal_True );
}

// -----------------------------------------------------------------------

SvtAccessibilityOptions::~SvtAccessibilityOptions()
{
    //EndListening( *sm_pSingleImplConfig, sal_True );
    ::osl::MutexGuard aGuard( SingletonMutex::get() );
    if( !--sm_nAccessibilityRefCount )
    {
        //if( sm_pSingleImplConfig->IsModified() )
        //  sm_pSingleImplConfig->Commit();
        DELETEZ( sm_pSingleImplConfig );
    }
}

// -----------------------------------------------------------------------

void SvtAccessibilityOptions::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    NotifyListeners(0);
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        if ( ((SfxSimpleHint&)rHint).GetId()  == SFX_HINT_ACCESSIBILITY_CHANGED )
            SetVCLSettings();
    }
}

// -----------------------------------------------------------------------

sal_Bool SvtAccessibilityOptions::IsModified() const
{
    return sm_pSingleImplConfig->IsModified();
}
void SvtAccessibilityOptions::Commit()
{
    //sm_pSingleImplConfig->Commit();
}

// -----------------------------------------------------------------------

sal_Bool SvtAccessibilityOptions::GetAutoDetectSystemHC() const
{
    return sm_pSingleImplConfig->GetAutoDetectSystemHC();
}
sal_Bool SvtAccessibilityOptions::GetIsForPagePreviews() const
{
    return sm_pSingleImplConfig->GetIsForPagePreviews();
}
sal_Bool SvtAccessibilityOptions::GetIsHelpTipsDisappear() const
{
    return sm_pSingleImplConfig->GetIsHelpTipsDisappear();
}
sal_Bool SvtAccessibilityOptions::GetIsAllowAnimatedGraphics() const
{
    return sm_pSingleImplConfig->GetIsAllowAnimatedGraphics();
}
sal_Bool SvtAccessibilityOptions::GetIsAllowAnimatedText() const
{
    return sm_pSingleImplConfig->GetIsAllowAnimatedText();
}
sal_Bool SvtAccessibilityOptions::GetIsAutomaticFontColor() const
{
    return sm_pSingleImplConfig->GetIsAutomaticFontColor();
}
sal_Bool SvtAccessibilityOptions::GetIsSystemFont() const
{
    return sm_pSingleImplConfig->GetIsSystemFont();
}
sal_Int16 SvtAccessibilityOptions::GetHelpTipSeconds() const
{
    return sm_pSingleImplConfig->GetHelpTipSeconds();
}
sal_Bool SvtAccessibilityOptions::IsSelectionInReadonly() const
{
    return sm_pSingleImplConfig->IsSelectionInReadonly();
}
sal_Int16 SvtAccessibilityOptions::GetEdgeBlending() const
{
    return sm_pSingleImplConfig->GetEdgeBlending();
}
sal_Int16 SvtAccessibilityOptions::GetListBoxMaximumLineCount() const
{
    return sm_pSingleImplConfig->GetListBoxMaximumLineCount();
}
sal_Int16 SvtAccessibilityOptions::GetColorValueSetColumnCount() const
{
    return sm_pSingleImplConfig->GetColorValueSetColumnCount();
}
sal_Bool SvtAccessibilityOptions::GetPreviewUsesCheckeredBackground() const
{
    return sm_pSingleImplConfig->GetPreviewUsesCheckeredBackground();
}

// -----------------------------------------------------------------------
void SvtAccessibilityOptions::SetAutoDetectSystemHC(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetAutoDetectSystemHC(bSet);
}
void SvtAccessibilityOptions::SetIsForPagePreviews(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsForPagePreviews(bSet);
}
void SvtAccessibilityOptions::SetIsHelpTipsDisappear(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsHelpTipsDisappear(bSet);
}
void SvtAccessibilityOptions::SetIsAllowAnimatedGraphics(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsAllowAnimatedGraphics(bSet);
}
void SvtAccessibilityOptions::SetIsAllowAnimatedText(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsAllowAnimatedText(bSet);
}
void SvtAccessibilityOptions::SetIsAutomaticFontColor(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsAutomaticFontColor(bSet);
}
void SvtAccessibilityOptions::SetIsSystemFont(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsSystemFont(bSet);
}
void SvtAccessibilityOptions::SetHelpTipSeconds(sal_Int16 nSet)
{
    sm_pSingleImplConfig->SetHelpTipSeconds(nSet);
}
void SvtAccessibilityOptions::SetSelectionInReadonly(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetSelectionInReadonly(bSet);
}
void SvtAccessibilityOptions::SetVCLSettings()
{
    sm_pSingleImplConfig->SetVCLSettings();
}
void SvtAccessibilityOptions::SetEdgeBlending(sal_Int16 nSet)
{
    sm_pSingleImplConfig->SetEdgeBlending(nSet);
}
void SvtAccessibilityOptions::SetListBoxMaximumLineCount(sal_Int16 nSet)
{
    sm_pSingleImplConfig->SetListBoxMaximumLineCount(nSet);
}
void SvtAccessibilityOptions::SetColorValueSetColumnCount(sal_Int16 nSet)
{
    sm_pSingleImplConfig->SetColorValueSetColumnCount(nSet);
}
void SvtAccessibilityOptions::SetPreviewUsesCheckeredBackground(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetPreviewUsesCheckeredBackground(bSet);
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

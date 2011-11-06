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



#include "AccessibilityOptTest.hxx"
#include "configitems/accessibilityoptions_const.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <comphelper/configurationhelper.hxx>
#include <unotools/processfactory.hxx>

namespace css = ::com::sun::star;

AccessibilityOptTest::AccessibilityOptTest()
{
    m_xCfg = css::uno::Reference< css::container::XNameAccess >(
        ::comphelper::ConfigurationHelper::openConfig(
        ::utl::getProcessServiceFactory(),
        s_sAccessibility,
        ::comphelper::ConfigurationHelper::E_STANDARD),
        css::uno::UNO_QUERY);
}

AccessibilityOptTest::~AccessibilityOptTest()
{
    if (m_xCfg.is())
        m_xCfg.clear();
}

//=============================================================================
//test GetAutoDetectSystemHC()
void AccessibilityOptTest::impl_checkGetAutoDetectSystemHC()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bAutoDetectSystemHC;
    sal_Bool bAutoDetectSystemHC_;

    bAutoDetectSystemHC  = aAccessibilityOpt.GetAutoDetectSystemHC();
    xSet->setPropertyValue( s_sAutoDetectSystemHC, css::uno::makeAny(bAutoDetectSystemHC ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bAutoDetectSystemHC_ = aAccessibilityOpt.GetAutoDetectSystemHC();

    if ( bAutoDetectSystemHC_ == bAutoDetectSystemHC )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetAutoDetectSystemHC() error!")), 0);
}

//=============================================================================
//test GetIsForPagePreviews()
void AccessibilityOptTest::impl_checkGetIsForPagePreviews()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsForPagePreviews ;
    sal_Bool bIsForPagePreviews_;

    bIsForPagePreviews  = aAccessibilityOpt.GetIsForPagePreviews();
    xSet->setPropertyValue( s_sIsForPagePreviews, css::uno::makeAny(bIsForPagePreviews ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bIsForPagePreviews_ = aAccessibilityOpt.GetIsForPagePreviews();

    if ( bIsForPagePreviews_ == bIsForPagePreviews )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetIsForPagePreviews() error!")),  0);
}

//=============================================================================
//test impl_checkGetIsHelpTipsDisappear()
void AccessibilityOptTest::impl_checkGetIsHelpTipsDisappear()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsHelpTipsDisappear ;
    sal_Bool bIsHelpTipsDisappear_;

    bIsHelpTipsDisappear  = aAccessibilityOpt.GetIsHelpTipsDisappear();
    xSet->setPropertyValue( s_sIsHelpTipsDisappear, css::uno::makeAny(bIsHelpTipsDisappear ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bIsHelpTipsDisappear_ = aAccessibilityOpt.GetIsHelpTipsDisappear();

    if ( bIsHelpTipsDisappear_ == bIsHelpTipsDisappear )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetIsHelpTipsDisappear() error!")),    0);
}

//=============================================================================
//test impl_checkGetIsAllowAnimatedGraphics()
void AccessibilityOptTest::impl_checkGetIsAllowAnimatedGraphics()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsAllowAnimatedGraphics ;
    sal_Bool bIsAllowAnimatedGraphics_;

    bIsAllowAnimatedGraphics  = aAccessibilityOpt.GetIsAllowAnimatedGraphics();
    xSet->setPropertyValue( s_sIsAllowAnimatedGraphics, css::uno::makeAny(bIsAllowAnimatedGraphics ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bIsAllowAnimatedGraphics_ = aAccessibilityOpt.GetIsAllowAnimatedGraphics();

    if ( bIsAllowAnimatedGraphics_ == bIsAllowAnimatedGraphics )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetIsAllowAnimatedGraphics() error!")),    0);
}

//=============================================================================
//test impl_checkGetIsAllowAnimatedText()
void AccessibilityOptTest::impl_checkGetIsAllowAnimatedText()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsAllowAnimatedText ;
    sal_Bool bIsAllowAnimatedText_;

    bIsAllowAnimatedText  = aAccessibilityOpt.GetIsAllowAnimatedText();
    xSet->setPropertyValue( s_sIsAllowAnimatedText, css::uno::makeAny(bIsAllowAnimatedText ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bIsAllowAnimatedText_ = aAccessibilityOpt.GetIsAllowAnimatedText();

    if ( bIsAllowAnimatedText_ == bIsAllowAnimatedText )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetIsAllowAnimatedText() error!")),    0);
}

//=============================================================================
//test impl_checkGetIsAutomaticFontColor()
void AccessibilityOptTest::impl_checkGetIsAutomaticFontColor()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsAutomaticFontColor ;
    sal_Bool bIsAutomaticFontColor_;

    bIsAutomaticFontColor  = aAccessibilityOpt.GetIsAutomaticFontColor();
    xSet->setPropertyValue( s_sIsAutomaticFontColor, css::uno::makeAny(bIsAutomaticFontColor ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bIsAutomaticFontColor_ = aAccessibilityOpt.GetIsAutomaticFontColor();

    if ( bIsAutomaticFontColor_ == bIsAutomaticFontColor )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetIsAutomaticFontColor() error!")),   0);
}

//=============================================================================
//test impl_checkGetIsSystemFont()
void AccessibilityOptTest::impl_checkGetIsSystemFont()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsSystemFont ;
    sal_Bool bIsSystemFont_;

    bIsSystemFont  = aAccessibilityOpt.GetIsSystemFont();
    xSet->setPropertyValue( s_sIsSystemFont, css::uno::makeAny(bIsSystemFont ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bIsSystemFont_ = aAccessibilityOpt.GetIsSystemFont();

    if ( bIsSystemFont_ == bIsSystemFont )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetIsSystemFont() error!")),   0);
}

//=============================================================================
//test impl_checkGetHelpTipSeconds()
void AccessibilityOptTest::impl_checkGetHelpTipSeconds()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Int16 nHelpTipSeconds ;
    sal_Int16 nHelpTipSeconds_;

    nHelpTipSeconds  = aAccessibilityOpt.GetHelpTipSeconds();
    xSet->setPropertyValue( s_sHelpTipSeconds, css::uno::makeAny(sal_Int16(nHelpTipSeconds+1)) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    nHelpTipSeconds_ = aAccessibilityOpt.GetHelpTipSeconds();

    if ( nHelpTipSeconds_ == nHelpTipSeconds )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GetHelpTipSeconds() error!")), 0);
}

//=============================================================================
//test impl_checkIsSelectionInReadonly()
void AccessibilityOptTest::impl_checkIsSelectionInReadonly()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsSelectionInReadonly ;
    sal_Bool bIsSelectionInReadonly_;

    bIsSelectionInReadonly  = aAccessibilityOpt.IsSelectionInReadonly();
    xSet->setPropertyValue( s_sIsSelectionInReadonly, css::uno::makeAny(bIsSelectionInReadonly ? sal_False:sal_True) );
    ::comphelper::ConfigurationHelper::flush(m_xCfg);
    bIsSelectionInReadonly_ = aAccessibilityOpt.IsSelectionInReadonly();

    if ( bIsSelectionInReadonly_ == bIsSelectionInReadonly )//old config item will not throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSelectionInReadonly() error!")), 0);
}

//=============================================================================
//test SetAutoDetectSystemHC()
void AccessibilityOptTest::impl_checkSetAutoDetectSystemHC()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bAutoDetectSystemHC;
    sal_Bool bAutoDetectSystemHC_;

    xSet->getPropertyValue(s_sAutoDetectSystemHC) >>= bAutoDetectSystemHC;
    aAccessibilityOpt.SetAutoDetectSystemHC( bAutoDetectSystemHC ? sal_False:sal_True );
    xSet->getPropertyValue(s_sAutoDetectSystemHC) >>= bAutoDetectSystemHC_;

    if ( bAutoDetectSystemHC_ == bAutoDetectSystemHC )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetAutoDetectSystemHC() error!")), 0);
}

//=============================================================================
//test SetIsForPagePreviews()
void AccessibilityOptTest::impl_checkSetIsForPagePreviews()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsForPagePreviews ;
    sal_Bool bIsForPagePreviews_;

    xSet->getPropertyValue(s_sIsForPagePreviews) >>= bIsForPagePreviews;
    aAccessibilityOpt.SetIsForPagePreviews( bIsForPagePreviews ? sal_False:sal_True );
    xSet->getPropertyValue(s_sIsForPagePreviews) >>= bIsForPagePreviews_;

    if ( bIsForPagePreviews_ == bIsForPagePreviews )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetIsForPagePreviews() error!")),  0);
}

//=============================================================================
//test impl_checkSetIsHelpTipsDisappear()
void AccessibilityOptTest::impl_checkSetIsHelpTipsDisappear()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsHelpTipsDisappear ;
    sal_Bool bIsHelpTipsDisappear_;

    xSet->getPropertyValue(s_sIsHelpTipsDisappear) >>= bIsHelpTipsDisappear;
    aAccessibilityOpt.SetIsHelpTipsDisappear( bIsHelpTipsDisappear ? sal_False:sal_True );
    xSet->getPropertyValue(s_sIsHelpTipsDisappear) >>= bIsHelpTipsDisappear_;

    if ( bIsHelpTipsDisappear_ == bIsHelpTipsDisappear )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetIsHelpTipsDisappear() error!")),    0);
}

//=============================================================================
//test impl_checkSetIsAllowAnimatedGraphics()
void AccessibilityOptTest::impl_checkSetIsAllowAnimatedGraphics()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsAllowAnimatedGraphics ;
    sal_Bool bIsAllowAnimatedGraphics_;

    xSet->getPropertyValue(s_sIsAllowAnimatedGraphics) >>= bIsAllowAnimatedGraphics;
    aAccessibilityOpt.SetIsAllowAnimatedGraphics( bIsAllowAnimatedGraphics ? sal_False:sal_True );
    xSet->getPropertyValue(s_sIsAllowAnimatedGraphics) >>= bIsAllowAnimatedGraphics_;

    if ( bIsAllowAnimatedGraphics_ == bIsAllowAnimatedGraphics )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetIsAllowAnimatedGraphics() error!")),    0);
}

//=============================================================================
//test impl_checkSetIsAllowAnimatedText()
void AccessibilityOptTest::impl_checkSetIsAllowAnimatedText()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsAllowAnimatedText ;
    sal_Bool bIsAllowAnimatedText_;

    xSet->getPropertyValue(s_sIsAllowAnimatedText) >>= bIsAllowAnimatedText;
    aAccessibilityOpt.SetIsAllowAnimatedText( bIsAllowAnimatedText ? sal_False:sal_True );
    xSet->getPropertyValue(s_sIsAllowAnimatedText) >>= bIsAllowAnimatedText_;

    if ( bIsAllowAnimatedText_ == bIsAllowAnimatedText )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetIsAllowAnimatedText() error!")),    0);
}

//=============================================================================
//test impl_checkSetIsAutomaticFontColor()
void AccessibilityOptTest::impl_checkSetIsAutomaticFontColor()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsAutomaticFontColor ;
    sal_Bool bIsAutomaticFontColor_;

    xSet->getPropertyValue(s_sIsAutomaticFontColor) >>= bIsAutomaticFontColor;
    aAccessibilityOpt.SetIsAutomaticFontColor( bIsAutomaticFontColor ? sal_False:sal_True );
    xSet->getPropertyValue(s_sIsAutomaticFontColor) >>= bIsAutomaticFontColor_;

    if ( bIsAutomaticFontColor_ == bIsAutomaticFontColor )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetIsAutomaticFontColor() error!")),   0);
}

//=============================================================================
//test impl_checkSetIsSystemFont()
void AccessibilityOptTest::impl_checkSetIsSystemFont()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsSystemFont ;
    sal_Bool bIsSystemFont_;

    xSet->getPropertyValue(s_sIsSystemFont) >>= bIsSystemFont;
    aAccessibilityOpt.SetIsSystemFont( bIsSystemFont ? sal_False:sal_True );
    xSet->getPropertyValue(s_sIsSystemFont) >>= bIsSystemFont_;

    if ( bIsSystemFont_ == bIsSystemFont )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetIsSystemFont() error!")),   0);
}

//=============================================================================
//test impl_checkSetHelpTipSeconds()
void AccessibilityOptTest::impl_checkSetHelpTipSeconds()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Int16 nHelpTipSeconds ;
    sal_Int16 nHelpTipSeconds_;

    xSet->getPropertyValue(s_sHelpTipSeconds) >>= nHelpTipSeconds;
    aAccessibilityOpt.SetHelpTipSeconds( sal_Int16(nHelpTipSeconds+1) );
    xSet->getPropertyValue(s_sHelpTipSeconds) >>= nHelpTipSeconds_;

    if ( nHelpTipSeconds_ == nHelpTipSeconds )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetHelpTipSeconds() error!")), 0);
}

//=============================================================================
//test impl_checkSetSelectionInReadonly()
void AccessibilityOptTest::impl_checkSetSelectionInReadonly()
{
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xCfg, css::uno::UNO_QUERY);
    sal_Bool bIsSelectionInReadonly ;
    sal_Bool bIsSelectionInReadonly_;

    xSet->getPropertyValue(s_sIsSelectionInReadonly) >>= bIsSelectionInReadonly;
    aAccessibilityOpt.SetSelectionInReadonly( bIsSelectionInReadonly ? sal_False:sal_True );
    xSet->getPropertyValue(s_sIsSelectionInReadonly) >>= bIsSelectionInReadonly_;

    if ( bIsSelectionInReadonly_ == bIsSelectionInReadonly )//old config item will throw error
        throw css::uno::RuntimeException(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SetSelectionInReadonly() error!")),    0);
}

//=============================================================================
void AccessibilityOptTest::impl_checkAccessibilityOptions()
{
    impl_checkGetAutoDetectSystemHC();
    impl_checkGetIsForPagePreviews();
    impl_checkGetIsHelpTipsDisappear();
    impl_checkGetIsAllowAnimatedGraphics();
    impl_checkGetIsAllowAnimatedText();
    impl_checkGetIsAutomaticFontColor();
    impl_checkGetIsSystemFont();
    impl_checkGetHelpTipSeconds();
    impl_checkIsSelectionInReadonly();

    impl_checkSetAutoDetectSystemHC();
    impl_checkSetIsForPagePreviews();
    impl_checkSetIsHelpTipsDisappear();
    impl_checkSetIsAllowAnimatedGraphics();
    impl_checkSetIsAllowAnimatedText();
    impl_checkSetIsAutomaticFontColor();
    impl_checkSetIsSystemFont();
    impl_checkSetHelpTipSeconds();
    impl_checkSetSelectionInReadonly();
}

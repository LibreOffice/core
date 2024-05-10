/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <config_fonts.h>

#include <test/bootstrapfixture.hxx>

#include <comphelper/processfactory.hxx>

#include <vcl/unohelp.hxx>
#include <vcl/virdev.hxx>

#include <textlayout.hxx>

class VclTextLayoutTest : public test::BootstrapFixture
{
public:
    VclTextLayoutTest()
        : BootstrapFixture(true, false)
    {
    }
};

#if HAVE_MORE_FONTS

CPPUNIT_TEST_FIXTURE(VclTextLayoutTest, testBreakLines_invalid_softbreak)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    vcl::DefaultTextLayout aTextLayout(*device);

    const OUString sTestStr = u"textline_ text_"_ustr;
    const auto nTextWidth = device->GetTextWidth(u"text"_ustr);

    css::uno::Reference<css::linguistic2::XHyphenator> xHyph;
    css::uno::Reference<css::i18n::XBreakIterator> xBI = vcl::unohelper::CreateBreakIterator();

    // softbreak cannot be greater than the string length

    const auto nTextLen = 13;

    auto[nBreakPos, nLineWidth]
        = aTextLayout.BreakLine(nTextWidth, sTestStr, xHyph, xBI, false, nTextWidth, nTextLen, 15);

    const sal_Int32 nExpectedBreakPos = 13;
    CPPUNIT_ASSERT_EQUAL(nExpectedBreakPos, nBreakPos);
}

CPPUNIT_TEST_FIXTURE(VclTextLayoutTest, testBreakLines_hyphens)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    vcl::DefaultTextLayout aTextLayout(*device);

    const OUString sTestStr = u"textline text-moretext"_ustr;
    const auto nTextWidth = device->GetTextWidth(u"textline text-moretex"_ustr);

    css::uno::Reference<css::uno::XComponentContext> xContext(
        comphelper::getProcessComponentContext());
    css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLinguMgr
        = css::linguistic2::LinguServiceManager::create(xContext);

    css::uno::Reference<css::linguistic2::XHyphenator> xHyph = xLinguMgr->getHyphenator();
    css::uno::Reference<css::i18n::XBreakIterator> xBI = vcl::unohelper::CreateBreakIterator();

    auto[nBreakPos, nLineWidth]
        = aTextLayout.BreakLine(nTextWidth, sTestStr, xHyph, xBI, true, nTextWidth, 13, 12);

    const sal_Int32 nExpectedBreakPos = 13;
    CPPUNIT_ASSERT_EQUAL(nExpectedBreakPos, nBreakPos);
}

CPPUNIT_TEST_FIXTURE(VclTextLayoutTest, testBreakLines_hyphen_word_under_two_chars)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    vcl::DefaultTextLayout aTextLayout(*device);

    const OUString sTestStr = u"textline text-moretext"_ustr;
    const auto nTextWidth = device->GetTextWidth(u"te-moretex"_ustr);

    css::uno::Reference<css::uno::XComponentContext> xContext(
        comphelper::getProcessComponentContext());
    css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLinguMgr
        = css::linguistic2::LinguServiceManager::create(xContext);

    css::uno::Reference<css::linguistic2::XHyphenator> xHyph = xLinguMgr->getHyphenator();
    css::uno::Reference<css::i18n::XBreakIterator> xBI = vcl::unohelper::CreateBreakIterator();

    auto[nBreakPos, nLineWidth]
        = aTextLayout.BreakLine(nTextWidth, sTestStr, xHyph, xBI, true, nTextWidth, 2, 10);

    const sal_Int32 nExpectedBreakPos = 2;
    CPPUNIT_ASSERT_EQUAL(nExpectedBreakPos, nBreakPos);
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

CPPUNIT_TEST_FIXTURE(VclTextLayoutTest, testBreakLinesWithIterator_invalid_softbreak)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font("DejaVu Sans", "Book", Size(0, 11)));

    vcl::DefaultTextLayout aTextLayout(*device);

    const OUString sTestStr = u"textline_ text_"_ustr;
    const auto nTextWidth = device->GetTextWidth("text");

    css::uno::Reference<css::linguistic2::XHyphenator> xHyph;
    css::uno::Reference<css::i18n::XBreakIterator> xBI = vcl::unohelper::CreateBreakIterator();

    // softbreak cannot be greater than the string length

    const auto nTextLen = 13;

    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(13),
        aTextLayout.BreakLinesWithIterator(nTextWidth, sTestStr, xHyph, xBI, false, nTextLen, 15));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

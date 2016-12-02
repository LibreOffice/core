/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpftFilterTestBase.hxx"

#include <com/sun/star/document/XFilter.hpp>

#include "WpftLoader.hxx"

namespace uno = com::sun::star::uno;

namespace writerperfect
{
namespace test
{

WpftFilterTestBase::WpftFilterTestBase(const rtl::OUString &rFactoryURL)
    : ::test::FiltersTest()
    , WpftFilterFixture()
    , m_aFactoryURL(rFactoryURL)
    , m_pOptionalMap(nullptr)
{
}

bool WpftFilterTestBase::load(const OUString &, const OUString &rURL, const OUString &,
                              SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    const WpftLoader aLoader(rURL, m_xFilter, m_aFactoryURL, m_xDesktop, m_xTypeMap, m_xContext);
    return aLoader.getDocument().is();
}

void WpftFilterTestBase::doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    testDir(OUString(), m_directories.getURLFromSrc(rPath));
}

void WpftFilterTestBase::doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath, const WpftOptionalMap_t &rOptionalMap)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    m_pOptionalMap = &rOptionalMap;
    testDir(OUString(), m_directories.getURLFromSrc(rPath));
    m_pOptionalMap = nullptr;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

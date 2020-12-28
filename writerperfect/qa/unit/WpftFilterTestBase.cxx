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

#include <tools/urlobj.hxx>

#include "WpftLoader.hxx"

namespace uno = com::sun::star::uno;

namespace writerperfect::test
{
WpftFilterTestBase::WpftFilterTestBase(const OUString& rFactoryURL)
    : ::test::FiltersTest()
    , WpftFilterFixture()
    , m_aFactoryURL(rFactoryURL)
    , m_pOptionalMap(nullptr)
{
}

bool WpftFilterTestBase::load(const OUString&, const OUString& rURL, const OUString&,
                              SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    if (m_pOptionalMap)
    {
        // first check if this test file is supported by the used version of the library
        const INetURLObject aUrl(rURL);
        const WpftOptionalMap_t::const_iterator it(m_pOptionalMap->find(aUrl.getName()));
        if ((it != m_pOptionalMap->end()) && !it->second)
            return true; // skip the file
    }

    const WpftLoader aLoader(rURL, m_xFilter, m_aFactoryURL, m_xDesktop, m_xTypeMap, m_xContext);
    return aLoader.getDocument().is();
}

void WpftFilterTestBase::doTest(const OUString& rFilter, std::u16string_view rPath)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    testDir(OUString(), m_directories.getURLFromSrc(rPath));
}

void WpftFilterTestBase::doTest(const OUString& rFilter, std::u16string_view rPath,
                                const WpftOptionalMap_t& rOptionalMap)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    m_pOptionalMap = &rOptionalMap;
    testDir(OUString(), m_directories.getURLFromSrc(rPath));
    m_pOptionalMap = nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

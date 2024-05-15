/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpftFilterFixture.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/theDesktop.hpp>

namespace document = com::sun::star::document;
namespace frame = com::sun::star::frame;
namespace uno = com::sun::star::uno;

namespace writerperfect::test
{
void WpftFilterFixture::setUp()
{
    ::test::BootstrapFixture::setUp();

    m_xDesktop = frame::theDesktop::get(m_xContext);

    m_xTypeDetection.set(m_xFactory->createInstanceWithContext(
                             u"com.sun.star.document.TypeDetection"_ustr, m_xContext),
                         uno::UNO_QUERY_THROW);
    m_xTypeMap.set(m_xTypeDetection, uno::UNO_QUERY_THROW);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

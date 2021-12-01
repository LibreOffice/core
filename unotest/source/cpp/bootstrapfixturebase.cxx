/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_features.h>

#include <unotest/bootstrapfixturebase.hxx>
#include <comphelper/processfactory.hxx>
#if HAVE_FEATURE_SCRIPTING
#include <basic/sbstar.hxx>
#endif

using namespace ::com::sun::star;

// NB. this constructor is called before any tests are run, once for each
// test function in a rather non-intuitive way. This is why all the 'real'
// heavy lifting is deferred until setUp. setUp and tearDown are interleaved
// between the tests as you might expect.
test::BootstrapFixtureBase::BootstrapFixtureBase() {}

test::BootstrapFixtureBase::~BootstrapFixtureBase() {}

void test::BootstrapFixtureBase::setUp()
{
    m_xContext = comphelper::getProcessComponentContext();
    m_xFactory = m_xContext->getServiceManager();
    m_xSFactory.set(m_xFactory, uno::UNO_QUERY_THROW);
}

void test::BootstrapFixtureBase::tearDown()
{
#if HAVE_FEATURE_SCRIPTING
    StarBASIC::DetachAllDocBasicItems();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

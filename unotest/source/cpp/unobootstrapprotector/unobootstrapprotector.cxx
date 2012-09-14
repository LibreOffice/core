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

#include <limits>
#include <string>
#include <iostream>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "cppuhelper/exc_hlp.hxx"
#include "cppunit/Message.h"
#include "osl/thread.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "protectorfactory.hxx"

namespace {

using namespace com::sun::star;

//cppunit calls instantiates a new TextFixture for each test and calls setUp
//and tearDown on that for every test in a fixture
//
//We basically need to call dispose on our root component context context to
//shut down cleanly in the right order.
//
//But we can't setup and tear down the root component context for
//every test because all the uno singletons will be invalid after
//the first dispose. So lets setup the default context once before
//all tests are run, and tear it down once after all have finished

class Prot : public CppUnit::Protector, private boost::noncopyable
{
public:
    Prot();

    virtual ~Prot();

    virtual bool protect(
        CppUnit::Functor const & functor,
        CppUnit::ProtectorContext const & context);
private:
    uno::Reference<uno::XComponentContext> m_xContext;
};


Prot::Prot()
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory = m_xContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xSFactory(xFactory, uno::UNO_QUERY_THROW);

    comphelper::setProcessServiceFactory(xSFactory);
}

bool Prot::protect(
    CppUnit::Functor const & functor, CppUnit::ProtectorContext const &)
{
    return functor();
}

Prot::~Prot()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT CppUnit::Protector * SAL_CALL unobootstrapprotector()
{
    return new Prot;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

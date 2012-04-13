/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#include "sal/config.h"
#include "sal/precppunit.hxx"

#include <limits>
#include <string>
#include <iostream>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"

#include <cppuhelper/bootstrap.hxx>
#include <ucbhelper/contentbroker.hxx>
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

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

#ifndef INCLUDED_UNOTEST_OFFICECONNECTION_HXX
#define INCLUDED_UNOTEST_OFFICECONNECTION_HXX

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <osl/process.h>
#include <unotest/detail/unotestdllapi.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace test {

// Start up and shut down an OOo instance (details about the OOo instance are
// tunneled in via "arg-..." bootstrap variables):
class OOO_DLLPUBLIC_UNOTEST OfficeConnection: private boost::noncopyable {
public:
    OfficeConnection();

    ~OfficeConnection();

    void setUp();

    void tearDown();

    css::uno::Reference< css::uno::XComponentContext >
    getComponentContext() const { return context_;}

    // Must not be called before setUp or after tearDown:
    bool isStillAlive() const;

private:
    oslProcess process_;
    css::uno::Reference< css::uno::XComponentContext >
        context_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

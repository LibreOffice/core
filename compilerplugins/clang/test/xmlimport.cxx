/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

// Cannot include this, makes clang crash
//#include "xmloff/xmlimp.hxx"

class SvXMLImportContext
{
public:
    virtual ~SvXMLImportContext() {}

    virtual void createFastChildContext() {}
    virtual void startFastElement() {}
    virtual void endFastElement() {}
};

class Test1 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{must override startFastElement too [loplugin:xmlimport]}}
    virtual void createFastChildContext() override;
};

class Test2 : public SvXMLImportContext
{
public:
    // no warning expected
    virtual void createFastChildContext() override;
    virtual void startFastElement() override {}
};

class Test3 : public Test2
{
public:
    // no warning expected
    virtual void createFastChildContext() override;
};

class Test4 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{must override startFastElement too [loplugin:xmlimport]}}
    virtual void endFastElement() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

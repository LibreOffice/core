/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::xml::sax
{
class XAttributeList;
}

class SvXMLImportContext
{
public:
    virtual ~SvXMLImportContext() {}

    virtual void startFastElement() {}
    virtual void endFastElement() {}
    virtual void characters(const OUString&) {}
    virtual void createFastChildContext() {}
    virtual void createUnknownChildContext() {}

    virtual void StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>&) {}
    virtual void EndElement() {}
    virtual void Characters(const OUString&) {}
    virtual void CreateChildContext() {}
};

class Test1 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{cannot override both startFastElement and StartElement [loplugin:xmlimport]}}
    virtual void startFastElement() override {}
    // expected-error@+1 {{cannot override both startFastElement and StartElement [loplugin:xmlimport]}}
    virtual void StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>&) override {}
};

class Test2 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{cannot override both endFastElement and EndElement [loplugin:xmlimport]}}
    virtual void endFastElement() override {}
    // expected-error@+1 {{cannot override both endFastElement and EndElement [loplugin:xmlimport]}}
    virtual void EndElement() override {}
};

class Test3 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{cannot override both characters and Characters [loplugin:xmlimport]}}
    virtual void Characters(const OUString&) override {}
    // expected-error@+1 {{cannot override both characters and Characters [loplugin:xmlimport]}}
    virtual void characters(const OUString&) override {}
};

class Test7 : public SvXMLImportContext
{
public:
    virtual void startFastElement() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::startFastElement();
    }
    virtual void endFastElement() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::endFastElement();
    }
    virtual void characters(const OUString& rChars) override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::characters(rChars);
    }
    virtual void createFastChildContext() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::createFastChildContext();
    }
    virtual void createUnknownChildContext() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::createUnknownChildContext();
    }
};

class Test8 : public SvXMLImportContext
{
public:
    virtual void
    StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList) override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::StartElement(xAttrList);
    }
    virtual void EndElement() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::EndElement();
    }
    virtual void Characters(const OUString& rChars) override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::Characters(rChars);
    }
    virtual void CreateChildContext() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        SvXMLImportContext::CreateChildContext();
    }
};

// no warning expected
class Test9a : public SvXMLImportContext
{
public:
    virtual void StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>&) override {}
};
class Test9b : public Test9a
{
public:
    virtual void
    StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList) override
    {
        Test9a::StartElement(xAttrList);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

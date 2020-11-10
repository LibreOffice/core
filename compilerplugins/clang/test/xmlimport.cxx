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
// Cannot include this, cannot be found
//#include <xmloff/xmlictxt.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::xml::sax
{
class XAttributeList;
class XFastContextHandler;
}

class SvXMLImportContext;
typedef rtl::Reference<SvXMLImportContext> SvXMLImportContextRef;
class SvXMLImportContext
{
public:
    virtual ~SvXMLImportContext() {}

    virtual void startFastElement() {}
    virtual void endFastElement() {}
    virtual void characters(const OUString&) {}
    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> createFastChildContext()
    {
        return nullptr;
    }
    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> createUnknownChildContext()
    {
        return nullptr;
    }

    virtual void StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>&) {}
    virtual void EndElement() {}
    virtual void Characters(const OUString&) {}
    virtual SvXMLImportContextRef CreateChildContext() { return nullptr; }

    void acquire();
    void release();

    void xxx(); // just here to avoid triggering a warning I don't want to check for
};

class Test1 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{cannot override both startFastElement and StartElement [loplugin:xmlimport]}}
    virtual void startFastElement() override { xxx(); }
    // expected-error@+1 {{cannot override both startFastElement and StartElement [loplugin:xmlimport]}}
    virtual void StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>&) override
    {
        xxx();
    }
};

class Test2 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{cannot override both endFastElement and EndElement [loplugin:xmlimport]}}
    virtual void endFastElement() override { xxx(); }
    // expected-error@+1 {{cannot override both endFastElement and EndElement [loplugin:xmlimport]}}
    virtual void EndElement() override { xxx(); }
};

class Test3 : public SvXMLImportContext
{
public:
    // expected-error@+1 {{cannot override both characters and Characters [loplugin:xmlimport]}}
    virtual void Characters(const OUString&) override { xxx(); }
    // expected-error@+1 {{cannot override both characters and Characters [loplugin:xmlimport]}}
    virtual void characters(const OUString&) override { xxx(); }
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
    virtual css::uno::Reference<css::xml::sax::XFastContextHandler>
    createFastChildContext() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        return SvXMLImportContext::createFastChildContext();
    }
    virtual css::uno::Reference<css::xml::sax::XFastContextHandler>
    createUnknownChildContext() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        return SvXMLImportContext::createUnknownChildContext();
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
    virtual SvXMLImportContextRef CreateChildContext() override
    {
        // expected-error@+1 {{don't call this superclass method [loplugin:xmlimport]}}
        return SvXMLImportContext::CreateChildContext();
    }
};

// no warning expected
class Test9a : public SvXMLImportContext
{
public:
    virtual void StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>&) override
    {
        xxx();
    }
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

class Test10a : public SvXMLImportContext
{
public:
    // expected-error@+1 {{empty, should be removed [loplugin:xmlimport]}}
    virtual void startFastElement() override {}
    // expected-error@+1 {{empty, should be removed [loplugin:xmlimport]}}
    virtual void endFastElement() override {}
    // expected-error@+1 {{empty, should be removed [loplugin:xmlimport]}}
    virtual void characters(const OUString&) override {}
    // expected-error@+1 {{empty, should be removed [loplugin:xmlimport]}}
    virtual css::uno::Reference<css::xml::sax::XFastContextHandler>
    createFastChildContext() override
    {
        return nullptr;
    }
    // expected-error@+1 {{empty, should be removed [loplugin:xmlimport]}}
    virtual css::uno::Reference<css::xml::sax::XFastContextHandler>
    createUnknownChildContext() override
    {
        return nullptr;
    }
};
// no warning expected
class Test10b : public SvXMLImportContext
{
public:
    virtual void StartElement(const css::uno::Reference<css::xml::sax::XAttributeList>&) override {}
    virtual void EndElement() override {}
    virtual void Characters(const OUString&) override {}
    virtual SvXMLImportContextRef CreateChildContext() override { return nullptr; }
};

enum XmlTokens
{
    XML_TOK_1
};

void test20(sal_uInt32 p, sal_uInt16 q, XmlTokens e)
{
    // expected-error@+1 {{comparing XML_TOK enum to 'sal_uInt32', expected sal_uInt16 [loplugin:xmlimport]}}
    if (p == XML_TOK_1)
        ;
    // no warning expected
    if (q == XML_TOK_1)
        ;
    switch (p)
    {
        // expected-error@+1 {{comparing XML_TOK enum to 'sal_uInt32', expected sal_uInt16 [loplugin:xmlimport]}}
        case XML_TOK_1:
            break;
    }
    switch (q)
    {
        // no warning expected
        case XML_TOK_1:
            break;
    }
    switch (e)
    {
        // no warning expected
        case XML_TOK_1:
            break;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

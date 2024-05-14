/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <utility>
#include <vcl/ImageTree.hxx>
#include <vcl/svapp.hxx>
#include <ucbhelper/content.hxx>

// A LO-private ("implementation detail") UCP used to access images from help
// content, with theme fallback and localization support as provided by VCL's
// ImageTree.
//
// The URL scheme is
//
//   "vnd.libreoffice.image://"<theme><path>["?lang="<language-tag>]

namespace {

class Provider final:
    public comphelper::WeakComponentImplHelper<
        css::lang::XServiceInfo, css::ucb::XContentProvider>
{
public:
    explicit Provider(
        css::uno::Reference<css::uno::XComponentContext> context):
        context_(std::move(context))
    {}

private:
    OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.comp.ucb.ImageContentProvider"_ustr; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return css::uno::Sequence<OUString>{
            u"com.sun.star.ucb.ImageContentProvider"_ustr};
    }

    css::uno::Reference<css::ucb::XContent> SAL_CALL queryContent(
        css::uno::Reference<css::ucb::XContentIdentifier> const & Identifier)
        override
    {
        css::uno::Reference<css::uno::XComponentContext> context;
        {
            std::unique_lock g(m_aMutex);
            context = context_;
        }
        if (!context.is()) {
            throw css::lang::DisposedException();
        }
        auto url(Identifier->getContentIdentifier());
        auto uri(css::uri::UriReferenceFactory::create(context)->parse(url));
        if (!(uri.is()
              && uri->getScheme().equalsIgnoreAsciiCase(
                  "vnd.libreoffice.image")))
        {
            throw css::ucb::IllegalIdentifierException(url);
        }
        auto auth(
            rtl::Uri::decode(
                uri->getAuthority(), rtl_UriDecodeStrict,
                RTL_TEXTENCODING_UTF8));
        if (auth.isEmpty()) {
            throw css::ucb::IllegalIdentifierException(url);
        }
        auto path(uri->getPath());
        if (path.isEmpty()) {
            throw css::ucb::IllegalIdentifierException(url);
        }
        OUStringBuffer buf;
        assert(path[0] == '/');
        for (sal_Int32 i = 1;;) {
            auto j = path.indexOf('/', i);
            if (j == -1) {
                j = path.getLength();
            }
            auto seg(
                rtl::Uri::decode(
                    path.copy(i, j - i), rtl_UriDecodeStrict,
                    RTL_TEXTENCODING_UTF8));
            if (seg.isEmpty()) {
                throw css::ucb::IllegalIdentifierException(url);
            }
            if (i != 1) {
                buf.append('/');
            }
            buf.append(seg);
            if (j == path.getLength()) {
                break;
            }
            i = j + 1;
        }
        auto decPath(buf.makeStringAndClear());
        OUString lang;
        if (uri->hasQuery()) {
            if (!uri->getQuery().startsWith("lang=", &lang)) {
                throw css::ucb::IllegalIdentifierException(url);
            }
            lang = rtl::Uri::decode(
                lang, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8);
            if (lang.isEmpty()) {
                throw css::ucb::IllegalIdentifierException(url);
            }
        }
        OUString newUrl;
        {
            SolarMutexGuard g;
            newUrl = ImageTree::get().getImageUrl(decPath, auth, lang);
        }
        ucbhelper::Content content;
        return
            ucbhelper::Content::create(
                newUrl, css::uno::Reference<css::ucb::XCommandEnvironment>(),
                context, content)
            ? content.get() : css::uno::Reference<css::ucb::XContent>();
    }

    sal_Int32 SAL_CALL compareContentIds(
        css::uno::Reference<css::ucb::XContentIdentifier> const & Id1,
        css::uno::Reference<css::ucb::XContentIdentifier> const & Id2) override
    {
        return Id1->getContentIdentifier().compareTo(
            Id2->getContentIdentifier());
    }

    void disposing(std::unique_lock<std::mutex>&) override {
        context_.clear();
    }

    css::uno::Reference<css::uno::XComponentContext> context_;
};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_ucb_ImageContentProvider_get_implementation(
    css::uno::XComponentContext * context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new Provider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

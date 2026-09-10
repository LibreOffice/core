/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <StringVector.hpp>
#include <Unit.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <net/HttpRequest.hpp>

#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <string>

/// Verifies that a frame-ancestors list in net.content_security_policy is widened, not obeyed
/// verbatim: coolwsd's own host and the integrator's host are allowed to frame both the document
/// page and the integrator settings page. Without the widening, pinning frame-ancestors to one
/// integrator blocks the settings iframe of every other one.
class UnitCSPFrameAncestors : public UnitWSD
{
    static constexpr const char* kPinnedAncestor = "https://pinned.example";
    static constexpr const char* kIntegratorHost = "integrator.example.org";

public:
    UnitCSPFrameAncestors()
        : UnitWSD("UnitCSPFrameAncestors")
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        config.setString("net.content_security_policy",
                         std::string("frame-ancestors ") + kPinnedAncestor);
    }

    void invokeWSDTest() override
    {
        testDocumentPage();
        testSettingsPageWithoutIntegrator();
        testSettingsPageWithIntegrator();
        testSettingsPageWithMalformedIntegrator();

        exitTest(TestResult::Ok);
    }

private:
    /// The host coolwsd is reached on, as it appears in a frame-ancestors source.
    std::string coolwsdAncestor() const
    {
        return Poco::URI(helpers::getTestServerURI()).getHost() + ":*";
    }

    /// The frame-ancestors sources of a response, or the empty string when the response carries
    /// no such directive.
    std::string getFrameAncestors(const std::shared_ptr<const http::Response>& response) const
    {
        const std::string csp = response->header().get("Content-Security-Policy", std::string());
        TST_LOG("CSP: " << csp);

        StringVector directives = StringVector::tokenize(csp, ';');
        for (std::size_t i = 0; i < directives.size(); ++i)
        {
            const std::string directive = Util::trimmed(directives[i]);
            constexpr std::string_view name = "frame-ancestors";
            if (directive.starts_with(name))
                return Util::trimmed(directive.substr(name.size()));
        }

        return std::string();
    }

    void assertHasAncestor(const std::string& frameAncestors, const std::string& expected,
                           const std::string& what)
    {
        LOK_ASSERT_MESSAGE("Expected [" + expected + "] among the frame ancestors of " + what +
                               ", which were [" + frameAncestors + ']',
                           frameAncestors.find(expected) != std::string::npos);
    }

    /// The settings page the integrator embeds in an iframe. The integrator POSTs a form, and
    /// wopi_setting_base_url is where it names its own host.
    std::shared_ptr<const http::Response> getSettingsPage(const std::string& settingBaseUrl)
    {
        http::Request request("/browser/dist/adminIntegratorSettings.html",
                              http::Request::VERB_POST);
        if (!settingBaseUrl.empty())
        {
            request.setBody("wopi_setting_base_url=" + Uri::encode(settingBaseUrl),
                            "application/x-www-form-urlencoded");
        }

        const std::shared_ptr<http::Session> session =
            http::Session::create(helpers::getTestServerURI());
        const std::shared_ptr<const http::Response> response =
            session->syncRequest(request, http::Session::getDefaultTimeout());
        LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
        return response;
    }

    /// The document page has always widened the configured list; this guards that it still does.
    void testDocumentPage()
    {
        const std::shared_ptr<const http::Response> response =
            http::get(helpers::getTestServerURI(), "/browser/dist/cool.html");
        LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());

        const std::string frameAncestors = getFrameAncestors(response);
        assertHasAncestor(frameAncestors, kPinnedAncestor, "cool.html");
        assertHasAncestor(frameAncestors, coolwsdAncestor(), "cool.html");
    }

    /// The regression: the settings page used to emit the configured list verbatim, so an
    /// integrator running on coolwsd's own host could not frame it.
    void testSettingsPageWithoutIntegrator()
    {
        const std::string frameAncestors = getFrameAncestors(getSettingsPage(std::string()));
        assertHasAncestor(frameAncestors, kPinnedAncestor, "the settings page");
        assertHasAncestor(frameAncestors, coolwsdAncestor(), "the settings page");
    }

    /// An integrator on a host of its own is allowed to frame its settings page too.
    void testSettingsPageWithIntegrator()
    {
        const std::string frameAncestors = getFrameAncestors(getSettingsPage(
            std::string("https://") + kIntegratorHost + "/apps/richdocuments/wopi/settings"));
        assertHasAncestor(frameAncestors, kPinnedAncestor, "the settings page");
        assertHasAncestor(frameAncestors, coolwsdAncestor(), "the settings page");
        assertHasAncestor(frameAncestors, std::string(kIntegratorHost) + ":*",
                          "the settings page");
    }

    /// A wopi_setting_base_url that is no URL at all contributes no ancestor, and does not stop
    /// the page from being served with the ancestors that are known good.
    void testSettingsPageWithMalformedIntegrator()
    {
        const std::string frameAncestors = getFrameAncestors(getSettingsPage(":::not a url"));
        assertHasAncestor(frameAncestors, kPinnedAncestor, "the settings page");
        assertHasAncestor(frameAncestors, coolwsdAncestor(), "the settings page");
        LOK_ASSERT_MESSAGE("A malformed wopi_setting_base_url must not reach frame-ancestors, "
                           "which were [" +
                               frameAncestors + ']',
                           frameAncestors.find("not a url") == std::string::npos);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitCSPFrameAncestors(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

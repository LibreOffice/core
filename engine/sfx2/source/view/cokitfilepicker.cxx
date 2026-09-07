/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/cokitfilepicker.hxx>

#include <COKit/COKit.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/kit/helper.hxx>
#include <tools/link.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>

namespace sfx2::COKitFilePicker
{
namespace
{
std::optional<COKitFilePickerProvider>& getProvider()
{
    static std::optional<COKitFilePickerProvider> oProvider;
    return oProvider;
}

/** One in-flight pick: the strings the provider's C arrays point into, kept alive until
    the provider answered, and the completion to run with the answer. */
struct PickContext
{
    std::vector<OString> aStrings;
    std::vector<COKitFilePickerFilter> aFilters;
    std::function<void(std::optional<OUString>)> aOnPicked;
    std::optional<OUString> oUrl;
};

// The main-loop half of a completed pick, run as a posted user event.
void PickCompletedEvent(void* pEv, void*)
{
    std::unique_ptr<PickContext> pContext(static_cast<PickContext*>(pEv));
    pContext->aOnPicked(pContext->oUrl);
}

/** The completion the provider calls, on whichever thread. The answer travels to the
    engine's main-loop thread as a posted user event, so the completion in the context
    runs where a uno command may run. */
void pickedCallback(void* pContext, const char* pUrl)
{
    PickContext* pPickContext = static_cast<PickContext*>(pContext);
    if (pUrl)
        pPickContext->oUrl = OUString::fromUtf8(pUrl);

    Application::PostUserEvent(LINK_NONMEMBER(pPickContext, PickCompletedEvent));
}
}

void installProvider(const COKitFilePickerProvider* pProvider)
{
    if (pProvider)
        getProvider() = *pProvider;
    else
        getProvider().reset();
}

bool isAvailable()
{
    return comphelper::COKit::isActive() && getProvider().has_value() && getProvider()->pick;
}

void pick(const OUString& rTitle, const std::vector<Filter>& rFilters,
          std::function<void(std::optional<OUString>)> aOnPicked)
{
    if (!isAvailable())
        return;

    auto pContext = std::make_unique<PickContext>();
    pContext->aOnPicked = std::move(aOnPicked);

    // Two strings per filter plus the title; reserve so the C pointers stay valid.
    pContext->aStrings.reserve(2 * rFilters.size() + 1);
    pContext->aStrings.push_back(rTitle.toUtf8());
    const char* pTitle = pContext->aStrings.back().getStr();

    for (const Filter& rFilter : rFilters)
    {
        pContext->aStrings.push_back(rFilter.sName.toUtf8());
        const char* pName = pContext->aStrings.back().getStr();
        pContext->aStrings.push_back(rFilter.sWildcards.toUtf8());
        pContext->aFilters.push_back({ pName, pContext->aStrings.back().getStr() });
    }

    getProvider()->pick(pTitle, pContext->aFilters.data(), pContext->aFilters.size(),
                        pickedCallback, pContext.release());
}

bool requestAndRedispatch(const OUString& rCommand, const OUString& rArgument,
                          const std::vector<Filter>& rFilters, const OUString& rTitle)
{
    if (!isAvailable())
        return false;

    const int nView = KitHelper::getCurrentView();

    pick(rTitle, rFilters,
         [rCommand, rArgument, nView](const std::optional<OUString>& roUrl)
         {
             if (!roUrl)
                 return;

             // Another view can have become current while the picker was open; the
             // command belongs to the view that asked for the file.
             if (nView >= 0 && KitHelper::getCurrentView() != nView)
                 KitHelper::setView(nView);

             comphelper::dispatchCommand(rCommand,
                                         { comphelper::makePropertyValue(rArgument, *roUrl) });
         });

    return true;
}

std::vector<Filter> graphicImportFilters()
{
    std::vector<Filter> aFilters;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    for (sal_uInt16 nFormat = 0; nFormat < rGraphicFilter.GetImportFormatCount(); ++nFormat)
    {
        OUStringBuffer aWildcards;
        for (sal_Int32 nEntry = 0;; ++nEntry)
        {
            const OUString sWildcard = rGraphicFilter.GetImportWildcard(nFormat, nEntry);
            if (sWildcard.isEmpty())
                break;
            if (!aWildcards.isEmpty())
                aWildcards.append(';');
            aWildcards.append(sWildcard);
        }

        if (!aWildcards.isEmpty())
            aFilters.push_back(
                { rGraphicFilter.GetImportFormatName(nFormat), aWildcards.makeStringAndClear() });
    }

    return aFilters;
}

std::vector<Filter> documentImportFilters(const OUString& rFactoryName)
{
    std::vector<Filter> aFilters;
    SfxFilterMatcher aMatcher(rFactoryName);
    SfxFilterMatcherIter aIter(aMatcher, SfxFilterFlags::IMPORT);

    for (std::shared_ptr<const SfxFilter> pFilter = aIter.First(); pFilter;
         pFilter = aIter.Next())
    {
        const OUString& sGlob = pFilter->GetWildcard().getGlob();
        if (!sGlob.isEmpty())
            aFilters.push_back({ pFilter->GetUIName(), sGlob });
    }

    return aFilters;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

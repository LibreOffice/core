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

#include <ResourceId.hxx>
#include <tools/SdGlobalResourceContainer.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>

#include <algorithm>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

namespace sd::framework {

//===== ResourceId ============================================================

WeakReference<util::XURLTransformer> ResourceId::mxURLTransformerWeak;

ResourceId::ResourceId()
    : maResourceURLs(0)
{
}

ResourceId::ResourceId (
    const OUString& rsResourceURL)
    : maResourceURLs(1, rsResourceURL)
{
    // Handle the special case of an empty resource URL.
    if (rsResourceURL.isEmpty())
        maResourceURLs.clear();
    ParseResourceURL();
}

ResourceId::ResourceId (
    const OUString& rsResourceURL, const rtl::Reference<ResourceId>& xAnchor)
{
    maResourceURLs.push_back(rsResourceURL);
    if (xAnchor.is())
    {
        maResourceURLs.push_back(xAnchor->getResourceURL());
        std::vector<OUString> aAnchorURLs (xAnchor->getAnchorURLs());
        maResourceURLs.insert( maResourceURLs.end(), aAnchorURLs.begin(), aAnchorURLs.end() );
    }
    ParseResourceURL();
}

ResourceId::ResourceId (
    const OUString& rsResourceURL,
    const OUString& rsAnchorURL)
    : maResourceURLs(2)
{
    maResourceURLs[0] = rsResourceURL;
    maResourceURLs[1] = rsAnchorURL;
    ParseResourceURL();
}

ResourceId::~ResourceId()
{
    mpURL.reset();
}

OUString ResourceId::getResourceURL() const
{
    if (!maResourceURLs.empty())
        return maResourceURLs[0];
    else
        return OUString();
}

util::URL ResourceId::getFullResourceURL()
{
    if (mpURL != nullptr)
        return *mpURL;

    Reference<util::XURLTransformer> xURLTransformer (mxURLTransformerWeak);
    if (xURLTransformer.is() && !maResourceURLs.empty() )
    {
        mpURL.reset(new util::URL);
        mpURL->Complete = maResourceURLs[0];
        xURLTransformer->parseStrict(*mpURL);
        return *mpURL;
    }

    util::URL aURL;
    if (!maResourceURLs.empty())
        aURL.Complete = maResourceURLs[0];
    return aURL;
}

bool ResourceId::hasAnchor() const
{
    return maResourceURLs.size()>1;
}

rtl::Reference<ResourceId> ResourceId::getAnchor() const
{
    ::rtl::Reference<ResourceId> rResourceId (new ResourceId());
    const sal_Int32 nAnchorCount (maResourceURLs.size()-1);
    if (nAnchorCount > 0)
    {
        rResourceId->maResourceURLs.resize(nAnchorCount);
        for (sal_Int32 nIndex=0; nIndex<nAnchorCount; ++nIndex)
            rResourceId->maResourceURLs[nIndex] = maResourceURLs[nIndex+1];
    }
    return rResourceId;
}

std::vector<OUString> ResourceId::getAnchorURLs() const
{
    const sal_Int32 nAnchorCount (maResourceURLs.size() - 1);
    if (nAnchorCount > 0)
    {
        std::vector<OUString> aAnchorURLs(nAnchorCount);
        std::copy_n(maResourceURLs.begin() + 1, nAnchorCount, aAnchorURLs.begin());
        return aAnchorURLs;
    }
    else
        return {};
}

OUString ResourceId::getResourceTypePrefix() const
{
    if (!maResourceURLs.empty() )
    {
        // Return the "private:resource/<type>/" prefix.

        // Get the prefix that ends with the second "/".
        const OUString& rsResourceURL (maResourceURLs[0]);
        sal_Int32 nPrefixEnd (rsResourceURL.indexOf('/'));
        if (nPrefixEnd >= 0)
            nPrefixEnd = rsResourceURL.indexOf('/', nPrefixEnd+1) + 1;
        else
            nPrefixEnd = 0;

        return rsResourceURL.copy(0,nPrefixEnd);
    }
    else
        return OUString();
}

sal_Int16 ResourceId::compareTo(const rtl::Reference<ResourceId>& rxResourceId) const
{
    sal_Int16 nResult (0);

    if ( ! rxResourceId.is())
    {
        // The empty reference is interpreted as empty resource id object.
        if (!maResourceURLs.empty())
            nResult = +1;
        else
            nResult = 0;
    }
    else
    {
        nResult = CompareToLocalImplementation(*rxResourceId);
    }

    return nResult;
}

sal_Int16 ResourceId::CompareToLocalImplementation (const ResourceId& rId) const
{
    sal_Int16 nResult (0);

    const sal_uInt32 nLocalURLCount (maResourceURLs.size());
    const sal_uInt32 nURLCount(rId.maResourceURLs.size());

    // Start comparison with the top most anchors.
    for (sal_Int32 nIndex=nURLCount-1,nLocalIndex=nLocalURLCount-1;
         nIndex>=0 && nLocalIndex>=0;
         --nIndex,--nLocalIndex)
    {
        const OUString sLocalURL (maResourceURLs[nLocalIndex]);
        const OUString sURL (rId.maResourceURLs[nIndex]);
        const sal_Int32 nLocalResult (sURL.compareTo(sLocalURL));
        if (nLocalResult != 0)
        {
            if (nLocalResult < 0)
                nResult = -1;
            else
                nResult = +1;
            break;
        }
    }

    if (nResult == 0)
    {
        // No difference found yet.  When the lengths are the same then the
        // two resource ids are equivalent.  Otherwise the shorter comes
        // first.
        if (nLocalURLCount != nURLCount)
        {
            if (nLocalURLCount < nURLCount)
                nResult = -1;
            else
                nResult = +1;
        }
    }

    return nResult;
}

bool ResourceId::isBoundTo (
        const rtl::Reference<ResourceId>& rxResourceId,
        AnchorBindingMode eMode) const
{
    if ( ! rxResourceId.is())
    {
        // An empty reference is interpreted as empty resource id.
        return IsBoundToAnchor(nullptr, eMode);
    }

    return IsBoundToAnchor(rxResourceId->maResourceURLs, eMode);
}

bool ResourceId::isBoundToURL (
        const OUString& rsAnchorURL,
        AnchorBindingMode eMode) const
{
    return IsBoundToAnchor(&rsAnchorURL, eMode);
}

/** When eMode is DIRECTLY then the anchor of the called object and the
    anchor represented by the given sequence of anchor URLs have to be
    identical.   When eMode is RECURSIVE then the anchor of the called
    object has to start with the given anchor URLs.
*/
bool ResourceId::IsBoundToAnchor (
    const OUString* psFirstAnchorURL,
    AnchorBindingMode eMode) const
{
    const sal_uInt32 nLocalAnchorURLCount (maResourceURLs.size() - 1);
    const bool bHasFirstAnchorURL (psFirstAnchorURL!=nullptr);
    const sal_uInt32 nAnchorURLCount (bHasFirstAnchorURL?1:0);

    // Check the lengths.
    if (nLocalAnchorURLCount<nAnchorURLCount ||
        (eMode==AnchorBindingMode_DIRECT && nLocalAnchorURLCount!=nAnchorURLCount))
    {
        return false;
    }

    // Compare the nAnchorURLCount bottom-most anchor URLs of this resource
    // id and the given anchor.
    if (bHasFirstAnchorURL)
    {
        if ( *psFirstAnchorURL != maResourceURLs[nLocalAnchorURLCount] )
            return false;
    }

    return true;
}

bool ResourceId::IsBoundToAnchor (
    const ::std::vector<OUString>& rAnchorURLs,
    AnchorBindingMode eMode) const
{
    const sal_uInt32 nLocalAnchorURLCount (maResourceURLs.size() - 1);
    const sal_uInt32 nAnchorURLCount (rAnchorURLs.size());

    // Check the lengths.
    if (nLocalAnchorURLCount<nAnchorURLCount ||
        (eMode==AnchorBindingMode_DIRECT && nLocalAnchorURLCount!=nAnchorURLCount))
    {
        return false;
    }

    // Compare the nAnchorURLCount bottom-most anchor URLs of this resource
    // id and the given anchor.
    for (sal_uInt32 nOffset=0; nOffset<nAnchorURLCount; ++nOffset)
    {
        if ( maResourceURLs[nLocalAnchorURLCount - nOffset] !=
            rAnchorURLs[nAnchorURLCount - 1 - nOffset] )
        {
            return false;
        }
    }

    return true;
}

void ResourceId::ParseResourceURL()
{
    ::osl::Guard< ::osl::Mutex > aGuard (::osl::Mutex::getGlobalMutex());
    Reference<util::XURLTransformer> xURLTransformer (mxURLTransformerWeak);
    if ( ! xURLTransformer.is())
    {
        // Create the URL transformer.
        const Reference<uno::XComponentContext>& xContext(::comphelper::getProcessComponentContext());
        xURLTransformer.set(util::URLTransformer::create(xContext));
        mxURLTransformerWeak = xURLTransformer;
        SdGlobalResourceContainer::Instance().AddResource(
            Reference<XInterface>(xURLTransformer,UNO_QUERY));
    }

    if (xURLTransformer.is() && !maResourceURLs.empty() )
    {
        mpURL.reset(new util::URL);
        mpURL->Complete = maResourceURLs[0];
        xURLTransformer->parseStrict(*mpURL);
        if (mpURL->Main == maResourceURLs[0])
            mpURL.reset();
        else
            maResourceURLs[0] = mpURL->Main;
    }
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

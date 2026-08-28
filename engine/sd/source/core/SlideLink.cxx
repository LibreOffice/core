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

#include <SlideLink.hxx>

#include <string_view>

#include <o3tl/string_view.hxx>
#include <rtl/uri.hxx>

namespace sd
{
namespace
{
/// The scheme a source document is named under.
constexpr OUString gSourceScheme = u"vnd.collabora.slide-source:"_ustr;
}

OUString SlideLink::MakeSourceReference(const OUString& rSourceName)
{
    const OUString aEscaped = rtl::Uri::encode(rSourceName, rtl_UriCharClassPchar,
                                               rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8);
    return gSourceScheme + rtl::Uri::decode(aEscaped, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
}

OUString SlideLink::GetSourceName(std::u16string_view rReference)
{
    if (!o3tl::starts_with(rReference, std::u16string_view(gSourceScheme)))
        return OUString();

    return rtl::Uri::decode(OUString(rReference.substr(gSourceScheme.getLength())),
                            rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

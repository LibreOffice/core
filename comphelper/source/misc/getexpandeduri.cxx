/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrlReference.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <comphelper/getexpandeduri.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

OUString comphelper::getExpandedUri(
    css::uno::Reference<css::uno::XComponentContext> const & context,
    OUString const & uri)
{
    css::uno::Reference<css::uri::XVndSunStarExpandUrlReference> ref(
        css::uri::UriReferenceFactory::create(context)->parse(uri),
        css::uno::UNO_QUERY);
    return ref.is()
        ? ref->expand(css::util::theMacroExpander::get(context)) : uri;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

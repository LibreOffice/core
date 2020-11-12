/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_GETEXPANDEDURI_HXX
#define INCLUDED_COMPHELPER_GETEXPANDEDURI_HXX

#include <sal/config.h>

#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}
namespace com::sun::star::uno
{
template <typename> class Reference;
}

namespace comphelper
{
/**
   A helper function to expand vnd.sun.star.expand URLs.

   If the given URI is a vnd.sun.star.expand URL, it is expanded (using the
   given component context's com.sun.star.util.theMacroExpander); otherwise it
   is returned unchanged.
*/
COMPHELPER_DLLPUBLIC OUString getExpandedUri(
    css::uno::Reference<css::uno::XComponentContext> const& context, OUString const& uri);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

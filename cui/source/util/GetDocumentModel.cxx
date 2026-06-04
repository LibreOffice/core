/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <GetDocumentModel.hxx>

#include <comphelper/documentinfo.hxx>

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/frame/theDesktop.hpp>

css::uno::Reference<css::frame::XModel>
getDocumentModel(const css::uno::Reference<css::uno::XComponentContext>& xContext,
                 std::u16string_view sDocName)
{
    css::uno::Reference<css::frame::XDesktop2> xDesktop = css::frame::theDesktop::get(xContext);

    css::uno::Reference<css::container::XEnumerationAccess> xComponentsAccess
        = xDesktop->getComponents();
    css::uno::Reference<css::container::XEnumeration> xComponents
        = xComponentsAccess->createEnumeration();

    while (xComponents->hasMoreElements())
    {
        css::uno::Reference<css::frame::XModel> xModel(xComponents->nextElement(),
                                                       css::uno::UNO_QUERY);
        if (xModel.is())
        {
            OUString sModelName = ::comphelper::DocumentInfo::getDocumentTitle(xModel);

            if (sModelName == sDocName)
                return xModel;
        }
    }

    return css::uno::Reference<css::frame::XModel>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

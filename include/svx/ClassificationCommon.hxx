/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_CLASSIFICATIONCOMMON_HXX
#define INCLUDED_SVX_CLASSIFICATIONCOMMON_HXX

#include <sal/config.h>
#include <svx/svxdllapi.h>
#include <svx/ClassificationField.hxx>
#include <vector>
#include <sfx2/classificationhelper.hxx>
#include <com/sun/star/beans/XPropertyContainer.hpp>

namespace svx
{
namespace classification
{
SVX_DLLPUBLIC OUString
convertClassificationResultToString(std::vector<svx::ClassificationResult> const& rResults);

SVX_DLLPUBLIC OUString
getProperty(css::uno::Reference<css::beans::XPropertyContainer> const& rxPropertyContainer,
            OUString const& rName);

SVX_DLLPUBLIC bool containsProperty(css::uno::Sequence<css::beans::Property> const& rProperties,
                                    OUString const& rName);

SVX_DLLPUBLIC void
removeAllProperties(css::uno::Reference<css::beans::XPropertyContainer> const& rxPropertyContainer);

SVX_DLLPUBLIC bool addOrInsertDocumentProperty(
    css::uno::Reference<css::beans::XPropertyContainer> const& rxPropertyContainer,
    OUString const& rsKey, OUString const& rsValue);

SVX_DLLPUBLIC void insertFullTextualRepresentationAsDocumentProperty(
    css::uno::Reference<css::beans::XPropertyContainer> const& rxPropertyContainer,
    sfx::ClassificationKeyCreator const& rKeyCreator,
    std::vector<svx::ClassificationResult> const& rResults);

SVX_DLLPUBLIC void insertCreationOrigin(css::uno::Reference<css::beans::XPropertyContainer> const & rxPropertyContainer,
                                        sfx::ClassificationKeyCreator const & rKeyCreator,
                                        sfx::ClassificationCreationOrigin eOrigin);

SVX_DLLPUBLIC sfx::ClassificationCreationOrigin getCreationOriginProperty(css::uno::Reference<css::beans::XPropertyContainer> const & rxPropertyContainer,
                                                                          sfx::ClassificationKeyCreator const & rKeyCreator);

}
} // end svx::classification namespace

#endif // INCLUDED_SVX_CLASSIFICATIONCOMMON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

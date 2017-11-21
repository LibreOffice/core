/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/ClassificationCommon.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace css;

namespace svx
{
namespace classification
{
OUString convertClassificationResultToString(std::vector<svx::ClassificationResult> const& rResults)
{
    OUString sRepresentation = "";

    for (svx::ClassificationResult const& rResult : rResults)
    {
        switch (rResult.meType)
        {
            case svx::ClassificationType::CATEGORY:
            case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
            case svx::ClassificationType::MARKING:
            case svx::ClassificationType::TEXT:
                sRepresentation += rResult.msName;
                break;

            case svx::ClassificationType::PARAGRAPH:
                sRepresentation += " ";
                break;
        }
    }
    return sRepresentation;
}

OUString getProperty(uno::Reference<beans::XPropertyContainer> const& rxPropertyContainer,
                     OUString const& rName)
{
    try
    {
        uno::Reference<beans::XPropertySet> xPropertySet(rxPropertyContainer, uno::UNO_QUERY);
        return xPropertySet->getPropertyValue(rName).get<OUString>();
    }
    catch (const css::uno::Exception&)
    {
    }

    return OUString();
}

bool containsProperty(uno::Sequence<beans::Property> const& rProperties, OUString const& rName)
{
    return std::find_if(rProperties.begin(), rProperties.end(),
                        [&](const beans::Property& rProperty) { return rProperty.Name == rName; })
           != rProperties.end();
}

void removeAllProperties(uno::Reference<beans::XPropertyContainer> const& rxPropertyContainer)
{
    uno::Reference<beans::XPropertySet> xPropertySet(rxPropertyContainer, uno::UNO_QUERY);
    uno::Sequence<beans::Property> aProperties
        = xPropertySet->getPropertySetInfo()->getProperties();

    for (const beans::Property& rProperty : aProperties)
    {
        rxPropertyContainer->removeProperty(rProperty.Name);
    }
}

bool addOrInsertDocumentProperty(
    uno::Reference<beans::XPropertyContainer> const& rxPropertyContainer, OUString const& rsKey,
    OUString const& rsValue)
{
    uno::Reference<beans::XPropertySet> xPropertySet(rxPropertyContainer, uno::UNO_QUERY);

    try
    {
        if (containsProperty(xPropertySet->getPropertySetInfo()->getProperties(), rsKey))
            xPropertySet->setPropertyValue(rsKey, uno::makeAny(rsValue));
        else
            rxPropertyContainer->addProperty(rsKey, beans::PropertyAttribute::REMOVABLE,
                                             uno::makeAny(rsValue));
    }
    catch (const uno::Exception& /*rException*/)
    {
        return false;
    }
    return true;
}

void insertFullTextualRepresentationAsDocumentProperty(
    uno::Reference<beans::XPropertyContainer> const& rxPropertyContainer,
    sfx::ClassificationKeyCreator const& rKeyCreator,
    std::vector<svx::ClassificationResult> const& rResults)
{
    OUString sString = convertClassificationResultToString(rResults);
    addOrInsertDocumentProperty(rxPropertyContainer, rKeyCreator.makeFullTextualRepresentationKey(),
                                sString);
}

void insertCreationOrigin(uno::Reference<beans::XPropertyContainer> const& rxPropertyContainer,
                          sfx::ClassificationKeyCreator const& rKeyCreator,
                          sfx::ClassificationCreationOrigin eOrigin)
{
    // Nothing to do if origin is "NONE"
    if (eOrigin == sfx::ClassificationCreationOrigin::NONE)
        return;

    OUString sValue = (eOrigin == sfx::ClassificationCreationOrigin::BAF_POLICY)
                          ? OUString("BAF_POLICY")
                          : OUString("MANUAL");
    addOrInsertDocumentProperty(rxPropertyContainer, rKeyCreator.makeCreationOriginKey(), sValue);
}
}
} // end svx::classification namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

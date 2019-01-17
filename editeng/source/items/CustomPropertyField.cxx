/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <editeng/CustomPropertyField.hxx>
#include <vcl/metaact.hxx>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace css;

namespace editeng
{

CustomPropertyField::CustomPropertyField(OUString const & rName, OUString const & rCurrentPresentation)
    : SvxFieldData()
    , msName(rName)
    , msCurrentPresentation(rCurrentPresentation)
{}

CustomPropertyField::~CustomPropertyField()
{}

std::unique_ptr<SvxFieldData> CustomPropertyField::Clone() const
{
    return std::make_unique<CustomPropertyField>(msName, msCurrentPresentation);
}

bool CustomPropertyField::operator==(const SvxFieldData& rOther) const
{
    if (typeid(rOther) != typeid(*this))
        return false;

    const CustomPropertyField& rOtherField = static_cast<const CustomPropertyField&>(rOther);
    return (msName               == rOtherField.msName &&
            msCurrentPresentation == rOtherField.msCurrentPresentation);
}

MetaAction* CustomPropertyField::createBeginComment() const
{
    return new MetaCommentAction("FIELD_SEQ_BEGIN");
}

OUString CustomPropertyField::GetFormatted(uno::Reference<document::XDocumentProperties> const & xDocumentProperties)
{
    if (msName.isEmpty())
        return OUString();
    if (!xDocumentProperties.is())
        return OUString();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    if (!xPropertyContainer.is())
        return OUString();
    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    if (!xPropertySet.is())
        return OUString();
    uno::Any aAny = xPropertySet->getPropertyValue(msName);
    if (!aAny.has<OUString>())
        return OUString();
    msCurrentPresentation = aAny.get<OUString>();
    return msCurrentPresentation;
}

} // end editeng namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

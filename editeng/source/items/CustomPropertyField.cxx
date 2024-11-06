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
#include <utility>
#include <vcl/metaact.hxx>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

using namespace css;

namespace editeng
{

CustomPropertyField::CustomPropertyField(OUString aName, OUString aCurrentPresentation)
    : msName(std::move(aName))
    , msCurrentPresentation(std::move(aCurrentPresentation))
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
    return new MetaCommentAction("FIELD_SEQ_BEGIN"_ostr);
}

const OUString & CustomPropertyField::GetFormatted(uno::Reference<document::XDocumentProperties> const & xDocumentProperties)
{
    if (msName.isEmpty())
        return EMPTY_OUSTRING;
    if (!xDocumentProperties.is())
        return EMPTY_OUSTRING;
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    if (!xPropertyContainer.is())
        return EMPTY_OUSTRING;
    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    if (!xPropertySet.is())
        return EMPTY_OUSTRING;
    uno::Any aAny = xPropertySet->getPropertyValue(msName);
    if (!aAny.has<OUString>())
        return EMPTY_OUSTRING;
    msCurrentPresentation = aAny.get<OUString>();
    return msCurrentPresentation;
}

} // end editeng namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

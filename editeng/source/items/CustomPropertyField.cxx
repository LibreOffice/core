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

CustomPropertyField::CustomPropertyField()
    : SvxFieldData()
{}

CustomPropertyField::CustomPropertyField(OUString const & rKey)
    : SvxFieldData()
    , msKey(rKey)
{}

CustomPropertyField::~CustomPropertyField()
{}

SV_IMPL_PERSIST1(CustomPropertyField);

SvxFieldData* CustomPropertyField::Clone() const
{
    return new CustomPropertyField(msKey);
}

bool CustomPropertyField::operator==(const SvxFieldData& rOther) const
{
    if (typeid(rOther) != typeid(*this))
        return false;

    const CustomPropertyField& rOtherField = static_cast<const CustomPropertyField&>(rOther);
    return (msKey == rOtherField.msKey);
}

MetaAction* CustomPropertyField::createBeginComment() const
{
    return new MetaCommentAction("FIELD_SEQ_BEGIN");
}

OUString CustomPropertyField::GetFormatted(uno::Reference<document::XDocumentProperties> const & xDocumentProperties) const
{
    if (!xDocumentProperties.is())
        return OUString();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    if (!xPropertyContainer.is())
        return OUString();
    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    if (!xPropertySet.is())
        return OUString();
    uno::Any aAny = xPropertySet->getPropertyValue(msKey);
    if (!aAny.has<OUString>())
        return OUString();
    return aAny.get<OUString>();
}

} // end editeng namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

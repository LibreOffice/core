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

#include <comphelper/attributelist.hxx>

#include <algorithm>
#include <cassert>

using namespace com::sun::star;


namespace comphelper {

OUString SAL_CALL AttributeList::getValueByName(const OUString& sName)
{
    for (auto const& attribute : mAttributes)
    {
        if( attribute.sName == sName ) {
            return attribute.sValue;
        }
    }
    return OUString();
}

AttributeList::AttributeList()
{
    // performance improvement during adding
    mAttributes.reserve(20);
}

AttributeList::AttributeList(const uno::Reference< xml::sax::XAttributeList>& rAttrList)
{
    if (AttributeList* pImpl = dynamic_cast<AttributeList*>(rAttrList.get()))
        mAttributes = pImpl->mAttributes;
    else
        AppendAttributeList(rAttrList);
}

AttributeList::~AttributeList()
{
}

css::uno::Reference< css::util::XCloneable > AttributeList::createClone()
{
    return new AttributeList( *this );
}

void AttributeList::AddAttribute(const OUString& sName, const OUString& sValue)
{
    assert(!sName.isEmpty() && "empty attribute name is invalid");
    // Either it's 'namespace_prefix:attribute_name',
    // or as in XMLNamespaces::applyNSToAttributeName, it's 'namespace:full:uri^attribute_name'.
    assert((std::count(sName.getStr(), sName.getStr() + sName.getLength(), u':') <= 1
            || std::count(sName.getStr(), sName.getStr() + sName.getLength(), u'^') == 1)
           && "too many colons");
    // TODO: this assertion fails in tests!
//    assert(std::none_of(mAttributes.begin(), mAttributes.end(),
//                        [&sName](const TagAttribute& a) { return a.sName == sName; }));
    mAttributes.push_back({ sName, sValue });
}

void AttributeList::RemoveAttribute(const OUString& sName)
{
    auto ii = std::find_if(mAttributes.begin(), mAttributes.end(),
                           [&sName](const TagAttribute& rAttr) { return rAttr.sName == sName; });

    if (ii != mAttributes.end())
        mAttributes.erase(ii);
}

void AttributeList::AppendAttributeList(const uno::Reference<css::xml::sax::XAttributeList>& r)
{
    assert(r.is());

    sal_Int16 nMax = r->getLength();
    sal_Int16 nTotalSize = mAttributes.size() + nMax;
    mAttributes.reserve(nTotalSize);

    for (sal_Int16 i = 0; i < nMax; ++i)
        AddAttribute(r->getNameByIndex(i), r->getValueByIndex(i));

    assert(nTotalSize == getLength());
}

void AttributeList::SetValueByIndex(sal_Int16 i, const OUString& rValue)
{
    mAttributes[i].sValue = rValue;
}

void AttributeList::RemoveAttributeByIndex(sal_Int16 i)
{
    mAttributes.erase(mAttributes.begin() + i);
}

void AttributeList::RenameAttributeByIndex(sal_Int16 i, const OUString& rNewName)
{
    mAttributes[i].sName = rNewName;
}

sal_Int16 AttributeList::GetIndexByName(const OUString& rName) const
{
    auto ii = std::find_if(mAttributes.begin(), mAttributes.end(),
                           [&rName](const TagAttribute& rAttr) { return rAttr.sName == rName; });

    if (ii != mAttributes.end())
        return static_cast<sal_Int16>(std::distance(mAttributes.begin(), ii));

    return -1;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

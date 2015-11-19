/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "oox/helper/grabbagstack.hxx"
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>

namespace oox
{

using namespace css::beans;
using namespace css::uno;

GrabBagStack::GrabBagStack(const OUString& aName)
{
    mCurrentElement.maName = aName;
}

GrabBagStack::~GrabBagStack()
{}

bool GrabBagStack::isStackEmpty()
{
    return mStack.empty();
}

PropertyValue GrabBagStack::getRootProperty()
{
    while(!mStack.empty())
        pop();

    PropertyValue aProperty;
    aProperty.Name = mCurrentElement.maName;
    aProperty.Value = makeAny(comphelper::containerToSequence(mCurrentElement.maPropertyList));

    return aProperty;
}

void GrabBagStack::appendElement(const OUString& aName, Any aAny)
{
    PropertyValue aValue;
    aValue.Name = aName;
    aValue.Value = aAny;
    mCurrentElement.maPropertyList.push_back(aValue);
}

void GrabBagStack::push(const OUString& aKey)
{
    mStack.push(mCurrentElement);
    mCurrentElement.maName = aKey;
    mCurrentElement.maPropertyList.clear();
}

void GrabBagStack::pop()
{
    OUString aName = mCurrentElement.maName;
    Sequence<PropertyValue> aSequence(comphelper::containerToSequence(mCurrentElement.maPropertyList));
    mCurrentElement = mStack.top();
    mStack.pop();
    appendElement(aName, makeAny(aSequence));
}

void GrabBagStack::addInt32(const OUString& aElementName, sal_Int32 aIntValue)
{
    appendElement(aElementName, makeAny(aIntValue));
}

void GrabBagStack::addString(const OUString& aElementName, const OUString& aStringValue)
{
    appendElement(aElementName, makeAny(aStringValue));
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

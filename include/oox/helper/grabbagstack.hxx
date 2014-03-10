/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_HELPER_GRABBAGSTACK_HXX
#define INCLUDED_OOX_HELPER_GRABBAGSTACK_HXX

#include <oox/dllapi.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vector>
#include <stack>

namespace oox {

struct GrabBagStackElement
{
    OUString maName;
    std::vector<css::beans::PropertyValue> maPropertyList;
};

/// Tool that is useful for construction of a nested Sequence/PropertyValue hierarchy
class OOX_DLLPUBLIC GrabBagStack
{
private:
    std::stack<GrabBagStackElement> mStack;
    GrabBagStackElement mCurrentElement;

public:
    GrabBagStack(OUString aName);

    virtual ~GrabBagStack();

    OUString getCurrentName();

    css::beans::PropertyValue getRootProperty();

    void appendElement(OUString aName, css::uno::Any aAny);
    void push(OUString aKey);
    void pop();
    void addInt32(OUString aElementName, sal_Int32 aIntValue);
    void addString(OUString aElementName, OUString aStringValue);
    bool isStackEmpty();
};

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

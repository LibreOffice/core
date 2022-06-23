/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_clang.h"

template <typename Type> class OptValue
{
public:
    OptValue()
        : maValue()
        , mbHasValue(false)
    {
    }
    explicit OptValue(const Type& rValue)
        : maValue(rValue)
        , mbHasValue(true)
    {
    }

    bool has_value() const { return mbHasValue; }
    bool operator!() const { return !mbHasValue; }

    const Type& value() const { return maValue; }
    const Type& value_or(const Type& rDefValue) const { return mbHasValue ? maValue : rDefValue; }

    Type& operator*() { return maValue; }
    Type& emplace()
    {
        mbHasValue = true;
        maValue = Type();
        return maValue;
    }

    OptValue& operator=(const Type& rValue)
    {
        maValue = rValue;
        mbHasValue = true;
        return *this;
    }
    bool operator==(const OptValue& rValue) const
    {
        return ((!mbHasValue && rValue.mbHasValue == false)
                || (mbHasValue == rValue.mbHasValue && maValue == rValue.maValue));
    }

private:
    Type maValue;
    bool mbHasValue;
};

struct AttributeList
{
    OptValue<int> getInteger();
};

namespace test1
{
void foo(AttributeList& rAttrs)
{
    // expected-error@+1 {{call to OptValue::value() [loplugin:optvalue]}}
    rAttrs.getInteger().value();

    // no warning expected
    OptValue<int> x;
    x.value();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

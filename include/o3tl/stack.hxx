/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_O3TL_STACK_HXX
#define INCLUDED_O3TL_STACK_HXX

#include <stack>

namespace o3tl
{

/**
 *
 * Same as std::stack (it just wraps it) except at destruction time the
 * container elements are destroyed in order starting from the top of the stack
 * which is the order one would rather assume a stack uses, but doesn't have to
 *
 * https://connect.microsoft.com/VisualStudio/feedback/details/765649/std-vector-does-not-destruct-in-reverse-order-of-construction
 *
 **/

template<class T> class stack final
{
private:
    typedef std::stack<T> stack_t;

    stack_t mStack;
public:

    T& top()
    {
        return mStack.top();
    }

    const T& top() const
    {
        return mStack.top();
    }

    void push(const T& val)
    {
        mStack.push(val);
    }

    void push(T&& val)
    {
        mStack.push(val);
    }

    void pop()
    {
        mStack.pop();
    }

    bool empty() const
    {
        return mStack.empty();
    }

    ~stack()
    {
        while (!mStack.empty())
            mStack.pop();
    }
};

}

#endif /* INCLUDED_O3TL_STACK_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

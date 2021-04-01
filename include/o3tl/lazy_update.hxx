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

#ifndef INCLUDED_O3TL_LAZY_UPDATE_HXX
#define INCLUDED_O3TL_LAZY_UPDATE_HXX

namespace o3tl
{
    /** Update output object lazily

        This template collects data in input type, and updates the
        output type with the given update functor, but only if the
        output is requested. Useful if updating is expensive, or input
        changes frequently, but output is only comparatively seldom
        used.

        @example
        <pre>
LazyUpdate<InType,OutType,decltype(F)> myValue(F);
*myValue = newInput;
myValue->updateInput( this, that, those );

output( *myValue );
        </pre>
        or
        <pre>
output( myValue.getOutValue() );
        </pre>
        if the compiler does not recognize the const context.
     */
    template<typename In, typename Out, typename Func> class LazyUpdate {
    public:
        LazyUpdate(Func const & func): func_(func), input_(), dirty_(true) {}

        In const & getInValue() const { return input_; }

        Out const & getOutValue() const { return update(); }

        In & operator *() {
            dirty_ = true;
            return input_;
        }

        In * operator ->() {
            dirty_ = true;
            return &input_;
        }

        Out const & operator *() const { return update();  }

        Out const * operator ->() const { return &update(); }

    private:
        Out const & update() const {
            if (dirty_) {
                output_ = func_(input_);
                dirty_ = false;
            }
            return output_;
        }

        Func const func_;
        In input_;
        mutable Out output_;
        mutable bool dirty_;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

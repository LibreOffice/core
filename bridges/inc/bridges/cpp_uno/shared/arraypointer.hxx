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

#ifndef INCLUDED_BRIDGES_INC_BRIDGES_CPP_UNO_SHARED_ARRAYPOINTER_HXX
#define INCLUDED_BRIDGES_INC_BRIDGES_CPP_UNO_SHARED_ARRAYPOINTER_HXX

#include "sal/config.h"

namespace bridges { namespace cpp_uno { namespace shared {

/**
 * A simple smart pointer that holds an array until it is being released.
 */
template< typename T > class ArrayPointer {
public:
    ArrayPointer(T * p): p_(p) {}

    ~ArrayPointer() { delete[] p_; }

    T * release() { T * t = p_; p_ = 0; return t; }

private:
    ArrayPointer(ArrayPointer &); // not defined
    void operator =(ArrayPointer &); // not defined

    T * p_;
};

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

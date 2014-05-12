/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_DELETER_HXX
#define INCLUDED_O3TL_DELETER_HXX

#include <functional>

namespace o3tl {

/**
 * Function object to allow deleting instances stored in STL containers as
 * pointers.
 */
template<typename T>
struct default_deleter : public std::unary_function<T*, void>
{
    void operator() (T* p)
    {
        delete p;
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_MAKE_UNIQUE_HXX
#define INCLUDED_O3TL_MAKE_UNIQUE_HXX

#include <memory>
#include <utility>

namespace o3tl
{

/**
 * Constructs an object of type T and wraps it in a std::unique_ptr.
 *
 * Can be replaced by std::make_unique when we allow C++14.
 */
template<typename T, typename... Args>
typename std::unique_ptr<T> make_unique(Args&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

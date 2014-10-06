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

#ifndef INCLUDED_O3TL_TYPED_FLAGS_SET_HXX
#define INCLUDED_O3TL_TYPED_FLAGS_SET_HXX

#include <type_traits>

/// The benefits of using this macro are:
///  - no accidental mixing of bit flags from different enums
///  - no accidental conversion to integer types
///  - no accidental assignment of integer values to bit-flag types
///
/// \param T the underlying 'enum class' type
/// \param M the all-bit-set value for the bit-flags
///
#define O3TL_MAKE_OPERATORS_FOR_TYPED_FLAGS_SET(T, M) \
  inline T operator| (T lhs, T rhs)                                              \
  {                                                                              \
      return static_cast<T>(static_cast<std::underlying_type<T>::type>(lhs)      \
                            | static_cast<std::underlying_type<T>::type>(rhs));  \
  }                                                                              \
  inline T operator& (T lhs, T rhs)                                              \
  {                                                                              \
      return static_cast<T>(static_cast<std::underlying_type<T>::type>(lhs)      \
                            & static_cast<std::underlying_type<T>::type>(rhs));  \
  }                                                                              \
  inline T operator~ (T rhs)                                                     \
  {                                                                              \
      return static_cast<T>(M & ~(static_cast<std::underlying_type<T>::type>(rhs))); \
  }                                                                              \
  inline T& operator|= (T& lhs, T rhs)                                           \
  {                                                                              \
      lhs = static_cast<T>(static_cast<std::underlying_type<T>::type>(lhs)       \
                           | static_cast<std::underlying_type<T>::type>(rhs));   \
      return lhs;                                                                \
  }                                                                              \
  inline T& operator&= (T& lhs, T rhs)                                           \
  {                                                                              \
      lhs = static_cast<T>(static_cast<std::underlying_type<T>::type>(lhs)       \
                           & static_cast<std::underlying_type<T>::type>(rhs));   \
      return lhs;                                                                \
  }                                                                              \




#endif /* INCLUDED_O3TL_TYPED_FLAGS_SET_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

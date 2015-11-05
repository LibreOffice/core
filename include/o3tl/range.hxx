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

#ifndef INCLUDED_O3TL_RANGE_HXX
#define INCLUDED_O3TL_RANGE_HXX


#include <cstring>
#include <cassert>



namespace o3tl
{
/** Represents a range of integer or iterator values.

    @tpl T
    Has to be assignable, add- and subtractable. That is:
    either it is
        - an integral type
        - or a random access iterator.
*/
template <class T>
class range
{
  public:
    typedef T           element_type;       /// Provided for generic programming.
    typedef range<T>    self;

  // LIFECYCLE
                        range(
                            T                   i_inclusiveLowerBorder,
                            T                   i_exclusiveUpperBorder );
                        ~range();
  // INQUIRY
    T                   begin() const;
    T                   end() const;
    std::size_t         size() const;

    bool                contains(
                            T                   i_value ) const;
    bool                contains(
                            const self &        i_other ) const;
    bool                overlaps(
                            const self &        i_other ) const;
    /// @return i_other.begin() - this->end()
    long                distance_to(
                            const self &        i_other ) const;
  private:
  // DATA
    T                   nBegin;
    T                   nEnd;
};


template <class T>
inline range<T>
make_range(T i1, T i2)
{
    return range<T>(i1, i2);
}

template <class T>
inline range<typename T::const_iterator>
range_of(const T & i_container)
{
    return make_range( i_container.begin(),
                       i_container.end()
                     );
}

template <class T>
inline range<typename T::iterator>
range_of(T & io_container)
{
    return make_range( io_container.begin(),
                       io_container.end()
                     );
}





// IMPLEMENTATION

template <class T>
range<T>::range( T i_inclusiveLowerBorder,
                 T i_exclusiveUpperBorder )
    :   nBegin(i_inclusiveLowerBorder),
        nEnd(i_exclusiveUpperBorder)
{
    assert( nBegin <= nEnd
            && "Invalid parameters for range<> constructor.");
}

template <class T>
range<T>::~range()
{
}

template <class T>
inline T
range<T>::begin() const
{
    return nBegin;
}

template <class T>
inline T
range<T>::end() const
{
    return nEnd;
}

template <class T>
inline std::size_t
range<T>::size() const
{
    assert( nBegin <= nEnd
            && "Invalid range limits in range<>::size().");
    return static_cast<std::size_t>( end() - begin() );
}

template <class T>
bool
range<T>::contains(T i_value ) const
{
    return      begin() <= i_value
            &&  i_value < end();
}

template <class T>
bool
range<T>::contains(const self & i_other) const
{
    // This is subtle, because this would be wrong:
    //      begin() <= i_other.begin()
    //      &&  i_other.end() <= end();
    // An empty range that begins and starts at my end()
    // must not be contained.

    return      contains(i_other.begin())
            &&  i_other.end() <= end();
}

template <class T>
bool
range<T>::overlaps(const self & i_other) const
{
    return      contains(i_other.begin())
            ||  i_other.contains(begin());
}

template <class T>
long
range<T>::distance_to(const self & i_other) const
{
    return i_other.begin() - end();
}



}   // namespace o3tl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

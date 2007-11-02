/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: range.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:44:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CSV_RANGE_HXX
#define CSV_RANGE_HXX

#include <cstring>  // for std::size_t




namespace csv
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
    csv_assert(  nBegin <= nEnd
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
    csv_assert(  nBegin <= nEnd
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




}   // namespace csv
#endif

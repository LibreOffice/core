/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itrange.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:41:03 $
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

#ifndef ARY_ITRANGE_HXX
#define ARY_ITRANGE_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <utility>




namespace ary
{

template <typename ITER>
class IteratorRange
{
  public:
                        IteratorRange(
                            ITER                i_begin,
                            ITER                i_end )
                                                :   itCurrent(i_begin),
                                                    itEnd(i_end)
                                                {}
                        IteratorRange(
                            std::pair<ITER,ITER>
                                                i_range )
                                                :   itCurrent(i_range.first),
                                                    itEnd(i_range.second)
                                                {}

                        operator bool() const   { return itCurrent != itEnd; }
    IteratorRange &     operator++()            { ++itCurrent; return *this; }

    ITER                cur() const             { return itCurrent; }
    ITER                end() const             { return itEnd; }


  private:
    // DATA
    ITER                itCurrent;
    ITER                itEnd;
};





}   // namespace ary
#endif

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

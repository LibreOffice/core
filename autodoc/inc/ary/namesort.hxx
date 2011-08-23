/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef ARY_NAMESORT_HXX
#define ARY_NAMESORT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace ary
{

/** Provides sensible sorting of ASCII names in programming languages.

    @descr
    Names are compared case insensitive first. Only after they appear
    equal that way, there is an additional case sensitive comparison.
    The second comparison sorts upper case before lower case.

*/
struct LesserName
{
    bool                operator()(
                            const String &      i_s1,
                            const String &      i_s2 ) const;
  private:
    // DATA

    static const csv::CharOrder_Table
                        aOrdering1_;
    static const csv::CharOrder_Table
                        aOrdering2_;
};

inline bool
LesserName::operator()( const String &      i_s1,
                        const String &      i_s2 ) const
{
    int result = i_s1.compare(aOrdering1_,i_s2);
    if (result == 0)
        result = i_s1.compare(aOrdering2_,i_s2);
    return result < 0;
}


}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

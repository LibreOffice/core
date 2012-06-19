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

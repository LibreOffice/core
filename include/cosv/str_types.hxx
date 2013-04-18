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

#ifndef COSV_STR_TYPES_HXX
#define COSV_STR_TYPES_HXX


namespace csv
{

/** Provides some generally used constants.
*/
struct str
{
  public:
    typedef ::size_t    position;
    typedef ::size_t    size;

    static const position npos = -1;
    static const size maxsize = -1;

    enum insert_mode
    {
        overwrite = 0,
        insert = 1
    };
};


/** Is used for string comparisons.

    @collab String
    @collab various csv::compare(...) functions
*/
class CharOrder_Table
{
  public:
    /** @precond
        Parameter i_pCharWeightsArray
        must have size of 256.
    */
                        CharOrder_Table(
                            const int *         i_pCharWeightsArray );

    /** @return the weight of the char i_c.
        @precond
        Even with unusual implementations, where char has more than 8 bit,
        there must be true: 0 <= i_c < 256.
    */
    int                 operator()(
                            char                i_c ) const;
  private:
    int                 cWeights[256];
};


// IMPLEMENTATION

inline int
CharOrder_Table::operator()( char i_c ) const
    { return cWeights[ UINT8(i_c) ]; }



}   // namespace csv

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

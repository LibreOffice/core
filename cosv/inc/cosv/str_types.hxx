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

    enum constants
    {
        npos = position(-1),
        maxsize = size(-1)
    };

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

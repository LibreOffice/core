/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: str_types.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:57:32 $
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

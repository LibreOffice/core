/*************************************************************************
 *
 *  $RCSfile: SwBitArray.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-26 13:48:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SW_BIT_ARRAY_HXX
#define _SW_BIT_ARRAY_HXX

#include <swtypes.hxx>
#include <iostream>

/**
    a bit array
*/
class SwBitArray
{
    /**
       size of a group of bits
    */
    static const size_t mGroupSize = sizeof(sal_uInt32);

    /**
       Returns number of groups.

       @return number of groups
    */
    size_t calcSize() const { return (nSize - 1)/ mGroupSize + 1; }

    /**
       array of group of bits
    */
    sal_uInt32 * mArray;

    /**
       number of groups
    */
    sal_uInt32 nSize;

    /**
       Returns group of bits according to an index.

       @param n     index to search for

       @return group of bits according to given index
     */
    sal_uInt32 * GetGroup(sal_uInt32 n) const { return &mArray[n/mGroupSize]; }

public:
    SwBitArray(sal_uInt32 _nSize);
    SwBitArray(const SwBitArray & rArray);
    ~SwBitArray();

    /**
       Returns if an index is valid.

       @retval TRUE    the index is valid
       @retval FALSE   else
    */
    BOOL IsValid(sal_uInt32 n) const;

    /**
       Returns the number of bits stored in the array.

       @return number of bits in the array
    */
    sal_uInt32 Size() const { return nSize; }

    /**
       Sets/unsets a bit.

       @param n     index of bit to set/unset
       @param nValue   -TRUE   set the bit
                       -FALSE  unset the bit
     */
    void Set(sal_uInt32 n, BOOL nValue);

    /**
       Unsets all bits of the array.
     */
    void Reset();

    /**
       Returns if a certain bit in the array is set.

       @param n     index of the bit in question

       @retval TRUE    the bit is set
       @retval FALSE   else
    */
    BOOL Get(sal_uInt32 n) const;

    /**
       Assigns a bit array to this bit array.

       @param rArray    array to assign

       rArray must have the same size as this array. Otherwise this
       array will not be altered.
     */
    SwBitArray & operator = (const SwBitArray & rArray);

    /**
       Returns the bitwise AND of two bit arrays.

       @param rA
       @param rB      the arrays to combine

       @return bitwise AND of rA and rB
     */
    friend SwBitArray operator & (const SwBitArray & rA,
                                  const SwBitArray & rB);

    /**
       Returns the bitwise OR of two bit arrays.

       @param rA
       @param rB      the arrays to combine

       @return bitwise OR of rA and rB
     */
    friend SwBitArray operator | (const SwBitArray & rA,
                                  const SwBitArray & rB);

    /**
       Returns the bitwise XOR of two bit arrays.

       @param rA
       @param rB      the arrays to combine

       @return bitwise XOR of rA and rB
     */
    friend SwBitArray operator ^ (const SwBitArray & rA,
                                  const SwBitArray & rB);

    /**
       Returns the bitwise NOT of an arrays.

       @param rA     the array to negate

       @return bitwise NOT of rA
     */
    friend SwBitArray operator ~ (const SwBitArray & rA);

    /**
       output operator

       @param o      output stream
       @param rBitArray      bit array to output

       @return o after the output
    */
    friend std::ostream & operator <<
        (std::ostream & o, const SwBitArray & rBitArray);
};


#endif  // _SW_BIT_ARRAY_HXX

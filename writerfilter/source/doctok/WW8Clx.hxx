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

#ifndef INCLUDED_WW8_CLX_HXX
#define INCLUDED_WW8_CLX_HXX

#include <resources.hxx>

#ifndef INCLUDED_WW8_OUTPUT_WITH_DEPTH_HXX
#include <WW8OutputWithDepth.hxx>
#endif

namespace writerfilter {
namespace doctok
{

/**
   Piece table as stored in a Word file.

   It contains character positions (CPs) and file character positions
   (FCs). Each CP is correlated to one FC.
 */
class WW8Clx : public WW8StructBase
{
    /// Pointer to the file information block.
    WW8Fib::Pointer_t pFib;

    /// Offset where FCs begin
    sal_uInt32 nOffsetPieceTable;
    
public:
    /// Pointer to Clx.
    typedef boost::shared_ptr<WW8Clx> Pointer_t;

    WW8Clx(WW8Stream & rStream,
           sal_uInt32 nOffset, sal_uInt32 nCount); 
        
    virtual void dump(OutputWithDepth<string> & o) const;

    /**
       Return the number of pieces.
    */
    sal_uInt32 getPieceCount() const;

    /**
       Return a certain character position.

       @param nIndex     index of the character position
     */
    sal_uInt32 getCp(sal_uInt32 nIndex) const;

    /**
       Return a certain file character position.

       @param nIndex     index of the file character position
     */
    sal_uInt32 getFc(sal_uInt32 nIndex) const;

    /**
       Return if a certain FC is complex.

       @nIndex           index of the FC
     */
    sal_Bool isComplexFc(sal_uInt32 nIndex) const;

};
}}

#endif

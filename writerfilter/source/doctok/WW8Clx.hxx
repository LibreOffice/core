/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Clx.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:45:27 $
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

#ifndef INCLUDED_WW8_CLX_HXX
#define INCLUDED_WW8_CLX_HXX

#ifndef INCLUDED_RESOURCES_HXX
#include <resources.hxx>
#endif

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

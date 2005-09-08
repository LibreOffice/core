/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagepar.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:47:23 $
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

#ifndef SC_PAGEPAR_HXX
#define SC_PAGEPAR_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

//-----------------------------------------------------------------------

struct ScPageTableParam
{
    BOOL    bNotes;
    BOOL    bGrid;
    BOOL    bHeaders;
    BOOL    bCharts;
    BOOL    bObjects;
    BOOL    bDrawings;
    BOOL    bFormulas;
    BOOL    bNullVals;
    BOOL    bTopDown;
    BOOL    bLeftRight;
    BOOL    bSkipEmpty;
    BOOL    bScaleNone;
    BOOL    bScaleAll;
    BOOL    bScaleTo;
    BOOL    bScalePageNum;
    USHORT  nScaleAll;
    USHORT  nScaleWidth;
    USHORT  nScaleHeight;
    USHORT  nScalePageNum;
    USHORT  nFirstPageNo;

    ScPageTableParam();
    ~ScPageTableParam();

    BOOL                operator==  ( const ScPageTableParam& r ) const;
    void                Reset       ();
};

struct ScPageAreaParam
{
    BOOL    bPrintArea;
    BOOL    bRepeatRow;
    BOOL    bRepeatCol;
    ScRange aPrintArea;
    ScRange aRepeatRow;
    ScRange aRepeatCol;

    ScPageAreaParam();
    ~ScPageAreaParam();

    BOOL                operator==  ( const ScPageAreaParam& r ) const;
    void                Reset       ();
};


#endif



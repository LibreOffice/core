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

#ifndef SC_DPOUTPUT_HXX
#define SC_DPOUTPUT_HXX

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>

#include <com/sun/star/sheet/DataResult.hpp>


#include "global.hxx"
class Rectangle;
class SvStream;
namespace binfilter {

class ScDocument;

struct ScDPOutLevelData;




struct ScDPPositionData
{
    long	nDimension;
    long	nHierarchy;
    long	nLevel;
    long	nFlags;
    String	aMemberName;

    ScDPPositionData() { nDimension = nHierarchy = nLevel = -1; nFlags = 0; }	// invalid
};



class ScDPOutput			//! name???
{
private:
    //!	use impl-object?
    ScDocument*				pDoc;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::sheet::XDimensionsSupplier> xSource;
    ScAddress				aStartPos;
    BOOL					bDoFilter;
    ScDPOutLevelData*		pColFields;
    ScDPOutLevelData*		pRowFields;
    ScDPOutLevelData*		pPageFields;
    long					nColFieldCount;
    long					nRowFieldCount;
    long					nPageFieldCount;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::sheet::DataResult> > aData;
    BOOL					bResultsError;
    String					aDataDescription;
    UINT32*					pColNumFmt;
    UINT32*					pRowNumFmt;
    long					nColFmtCount;
    long					nRowFmtCount;

    BOOL					bSizesValid;
    BOOL					bSizeOverflow;
    long					nColCount;
    long					nRowCount;
    long					nHeaderSize;
    USHORT					nTabStartCol;
    USHORT					nTabStartRow;
    USHORT					nMemberStartCol;
    USHORT					nMemberStartRow;
    USHORT					nDataStartCol;
    USHORT					nDataStartRow;
    USHORT					nTabEndCol;
    USHORT					nTabEndRow;


public:
                     ~ScDPOutput();

    void			SetPosition( const ScAddress& rPos );


};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

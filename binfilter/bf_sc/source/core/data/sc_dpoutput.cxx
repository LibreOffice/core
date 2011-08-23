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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <bf_svx/algitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/wghtitem.hxx>

#include "dpoutput.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "markdata.hxx"
#include "attrib.hxx"
#include "compiler.hxx"		// errNoValue
#include "miscuno.hxx"
#include "globstr.hrc"
#include "stlpool.hxx"
#include "stlsheet.hxx"

#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/container/XNamed.hpp>
namespace binfilter {

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

//!	move to a header file
#define DP_PROP_ORIENTATION			"Orientation"
#define DP_PROP_POSITION			"Position"
#define DP_PROP_USEDHIERARCHY		"UsedHierarchy"
#define DP_PROP_DATADESCR			"DataDescription"
#define DP_PROP_ISDATALAYOUT		"IsDataLayoutDimension"
#define DP_PROP_NUMBERFORMAT		"NumberFormat"

// -----------------------------------------------------------------------

//!	dynamic!!!
#define SC_DPOUT_MAXLEVELS	256


struct ScDPOutLevelData
{
    long								nDim;
    long								nHier;
    long								nLevel;
    long								nDimPos;
    uno::Sequence<sheet::MemberResult>	aResult;
    String								aCaption;

    ScDPOutLevelData() { nDim = nHier = nLevel = nDimPos = -1; }

    BOOL operator<(const ScDPOutLevelData& r) const
        { return nDimPos<r.nDimPos || ( nDimPos==r.nDimPos && nHier<r.nHier ) ||
            ( nDimPos==r.nDimPos && nHier==r.nHier && nLevel<r.nLevel ); }

    void Swap(ScDPOutLevelData& r)
//!		{ ScDPOutLevelData aTemp = r; r = *this; *this = aTemp; }
        { ScDPOutLevelData aTemp; aTemp = r; r = *this; *this = aTemp; }

    //!	bug (73840) in uno::Sequence - copy and then assign doesn't work!
};

ScDPOutput::~ScDPOutput()
{
    delete[] pColFields;
    delete[] pRowFields;
    delete[] pPageFields;

    delete[] pColNumFmt;
    delete[] pRowNumFmt;
}

/*N*/ void ScDPOutput::SetPosition( const ScAddress& rPos )
/*N*/ {
/*N*/ 	aStartPos = rPos;
/*N*/  	bSizesValid = bSizeOverflow = FALSE;
/*N*/ }

}

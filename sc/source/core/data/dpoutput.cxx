/*************************************************************************
 *
 *  $RCSfile: dpoutput.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-08 14:25:49 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/wghtitem.hxx>

#include "dpoutput.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "markdata.hxx"
#include "attrib.hxx"
#include "compiler.hxx"     // errNoValue
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

using namespace com::sun::star;

// -----------------------------------------------------------------------

//! move to a header file
#define DP_PROP_ORIENTATION         "Orientation"
#define DP_PROP_POSITION            "Position"
#define DP_PROP_USEDHIERARCHY       "UsedHierarchy"
#define DP_PROP_DATADESCR           "DataDescription"
#define DP_PROP_ISDATALAYOUT        "IsDataLayoutDimension"
#define DP_PROP_NUMBERFORMAT        "NumberFormat"

// -----------------------------------------------------------------------

//! dynamic!!!
#define SC_DPOUT_MAXLEVELS  256


struct ScDPOutLevelData
{
    long                                nDim;
    long                                nHier;
    long                                nLevel;
    long                                nDimPos;
    uno::Sequence<sheet::MemberResult>  aResult;
    String                              aCaption;

    ScDPOutLevelData() { nDim = nHier = nLevel = nDimPos = -1; }

    BOOL operator<(const ScDPOutLevelData& r) const
        { return nDimPos<r.nDimPos || ( nDimPos==r.nDimPos && nHier<r.nHier ) ||
            ( nDimPos==r.nDimPos && nHier==r.nHier && nLevel<r.nLevel ); }

    void Swap(ScDPOutLevelData& r)
//!     { ScDPOutLevelData aTemp = r; r = *this; *this = aTemp; }
        { ScDPOutLevelData aTemp; aTemp = r; r = *this; *this = aTemp; }

    //! bug (73840) in uno::Sequence - copy and then assign doesn't work!
};

// -----------------------------------------------------------------------

void lcl_SetStyleById( ScDocument* pDoc, USHORT nTab,
                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                    USHORT nStrId )
{
    if ( nCol1 > nCol2 || nRow1 > nRow2 )
    {
        DBG_ERROR("SetStyleById: invalid range");
        return;
    }

    String aStyleName = ScGlobal::GetRscString( nStrId );
    ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
    ScStyleSheet* pStyle = (ScStyleSheet*) pStlPool->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
    if (!pStyle)
    {
        //  create new style (was in ScPivot::SetStyle)

        pStyle = (ScStyleSheet*) &pStlPool->Make( aStyleName, SFX_STYLE_FAMILY_PARA,
                                                    SFXSTYLEBIT_USERDEF );
        pStyle->SetParent( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
        SfxItemSet& rSet = pStyle->GetItemSet();
        if ( nStrId==STR_PIVOT_STYLE_RESULT || nStrId==STR_PIVOT_STYLE_TITLE )
            rSet.Put( SvxWeightItem( WEIGHT_BOLD ) );
        if ( nStrId==STR_PIVOT_STYLE_CATEGORY || nStrId==STR_PIVOT_STYLE_TITLE )
            rSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT ) );
    }

    pDoc->ApplyStyleAreaTab( nCol1, nRow1, nCol2, nRow2, nTab, *pStyle );
}

void lcl_SetFrame( ScDocument* pDoc, USHORT nTab,
                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                    USHORT nWidth )
{
    SvxBorderLine aLine;
    aLine.SetOutWidth(nWidth);
    SvxBoxItem aBox;
    aBox.SetLine(&aLine, BOX_LINE_LEFT);
    aBox.SetLine(&aLine, BOX_LINE_TOP);
    aBox.SetLine(&aLine, BOX_LINE_RIGHT);
    aBox.SetLine(&aLine, BOX_LINE_BOTTOM);
    SvxBoxInfoItem aBoxInfo;
    aBoxInfo.SetValid(VALID_HORI,FALSE);
    aBoxInfo.SetValid(VALID_VERT,FALSE);
    aBoxInfo.SetValid(VALID_DISTANCE,FALSE);

    ScMarkData aMark;
    aMark.SelectTable( nTab, TRUE );
    aMark.SetMarkArea( ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab ) );
    pDoc->ApplySelectionFrame( aMark, &aBox, &aBoxInfo );
}

void lcl_AttrArea( ScDocument* pDoc, USHORT nTab,
                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                    const SfxPoolItem& rItem )
{
    ScPatternAttr aPattern( pDoc->GetPool() );
    aPattern.GetItemSet().Put( rItem );
    pDoc->ApplyPatternAreaTab( nCol1,nRow1, nCol2,nRow2, nTab, aPattern );
}

// -----------------------------------------------------------------------

void lcl_FillNumberFormats( UINT32*& rFormats, long& rCount,
                            const uno::Reference<sheet::XDataPilotMemberResults>& xLevRes,
                            const uno::Reference<container::XIndexAccess>& xDims )
{
    if ( rFormats )
        return;                         // already set

    //  xLevRes is from the data layout dimension
    //! use result sequence from ScDPOutLevelData!

    uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();

    long nSize = aResult.getLength();
    if (nSize)
    {
        //  get names/formats for all data dimensions
        //! merge this with the loop to collect ScDPOutLevelData?

        String aDataNames[SC_DPOUT_MAXLEVELS];
        UINT32 nDataFormats[SC_DPOUT_MAXLEVELS];
        long nDataCount = 0;
        BOOL bAnySet = FALSE;

        long nDimCount = xDims->getCount();
        for (long nDim=0; nDim<nDimCount; nDim++)
        {
            uno::Reference<uno::XInterface> xDim =
                    ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
            uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
            if ( xDimProp.is() && xDimName.is() )
            {
                sheet::DataPilotFieldOrientation eDimOrient =
                    (sheet::DataPilotFieldOrientation) ScUnoHelpFunctions::GetEnumProperty(
                        xDimProp, rtl::OUString::createFromAscii(DP_PROP_ORIENTATION),
                        sheet::DataPilotFieldOrientation_HIDDEN );
                if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
                {
                    aDataNames[nDataCount] = String( xDimName->getName() );
                    long nFormat = ScUnoHelpFunctions::GetLongProperty(
                                            xDimProp,
                                            rtl::OUString::createFromAscii(DP_PROP_NUMBERFORMAT) );
                    nDataFormats[nDataCount] = nFormat;
                    if ( nFormat != 0 )
                        bAnySet = TRUE;
                    ++nDataCount;
                }
            }
        }

        if ( bAnySet )      // forget everything if all formats are 0 (or no data dimensions)
        {
            const sheet::MemberResult* pArray = aResult.getConstArray();

            String aName;
            UINT32* pNumFmt = new UINT32[nSize];
            if (nDataCount == 1)
            {
                //  only one data dimension -> use its numberformat everywhere
                long nFormat = nDataFormats[0];
                for (long nPos=0; nPos<nSize; nPos++)
                    pNumFmt[nPos] = nFormat;
            }
            else
            {
                for (long nPos=0; nPos<nSize; nPos++)
                {
                    //  if CONTINUE bit is set, keep previous name
                    //! keep number format instead!
                    if ( !(pArray[nPos].Flags & sheet::MemberResultFlags::CONTINUE) )
                        aName = String( pArray[nPos].Name );

                    UINT32 nFormat = 0;
                    for (long i=0; i<nDataCount; i++)
                        if (aName == aDataNames[i])         //! search more efficiently?
                        {
                            nFormat = nDataFormats[i];
                            break;
                        }
                    pNumFmt[nPos] = nFormat;
                }
            }

            rFormats = pNumFmt;
            rCount = nSize;
        }
    }
}

void lcl_SortFields( ScDPOutLevelData* pFields, long nFieldCount )
{
    for (long i=0; i+1<nFieldCount; i++)
    {
        for (long j=0; j+i+1<nFieldCount; j++)
            if ( pFields[j+1] < pFields[j] )
                pFields[j].Swap( pFields[j+1] );
    }
}

BOOL lcl_MemberEmpty( const uno::Sequence<sheet::MemberResult>& rSeq )
{
    //  used to skip levels that have no members

    long nLen = rSeq.getLength();
    const sheet::MemberResult* pArray = rSeq.getConstArray();
    for (long i=0; i<nLen; i++)
        if (pArray[i].Flags & sheet::MemberResultFlags::HASMEMBER)
            return FALSE;

    return TRUE;    // no member data -> empty
}

ScDPOutput::ScDPOutput( ScDocument* pD, const uno::Reference<sheet::XDimensionsSupplier>& xSrc,
                                const ScAddress& rPos, BOOL bFilter ) :
    pDoc( pD ),
    xSource( xSrc ),
    aStartPos( rPos ),
    bDoFilter( bFilter ),
    bSizesValid( FALSE ),
    bSizeOverflow( FALSE ),
    bResultsError( FALSE ),
    pColNumFmt( NULL ),
    pRowNumFmt( NULL ),
    nColFmtCount( 0 ),
    nRowFmtCount( 0 )
{
    nTabStartCol = nTabStartRow = nMemberStartCol = nMemberStartRow =
        nDataStartCol = nDataStartRow = nTabEndCol = nTabEndRow = 0;

    pColFields  = new ScDPOutLevelData[SC_DPOUT_MAXLEVELS];
    pRowFields  = new ScDPOutLevelData[SC_DPOUT_MAXLEVELS];
    pPageFields = new ScDPOutLevelData[SC_DPOUT_MAXLEVELS];
    nColFieldCount = 0;
    nRowFieldCount = 0;
    nPageFieldCount = 0;

    uno::Reference<sheet::XDataPilotResults> xResult( xSource, uno::UNO_QUERY );
    if ( xSource.is() && xResult.is() )
    {
        //  get dimension results:

        uno::Reference<container::XIndexAccess> xDims =
                new ScNameToIndexAccess( xSource->getDimensions() );
        long nDimCount = xDims->getCount();
        for (long nDim=0; nDim<nDimCount; nDim++)
        {
            uno::Reference<uno::XInterface> xDim =
                    ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
            uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDim, uno::UNO_QUERY );
            if ( xDimProp.is() && xDimSupp.is() )
            {
                sheet::DataPilotFieldOrientation eDimOrient =
                    (sheet::DataPilotFieldOrientation) ScUnoHelpFunctions::GetEnumProperty(
                        xDimProp, rtl::OUString::createFromAscii(DP_PROP_ORIENTATION),
                        sheet::DataPilotFieldOrientation_HIDDEN );
                long nDimPos = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                        rtl::OUString::createFromAscii(DP_PROP_POSITION) );
                BOOL bIsDataLayout = ScUnoHelpFunctions::GetBoolProperty(
                                                xDimProp,
                                                rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );

                if ( eDimOrient != sheet::DataPilotFieldOrientation_HIDDEN )
                {
                    uno::Reference<container::XIndexAccess> xHiers =
                            new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                    long nHierarchy = ScUnoHelpFunctions::GetLongProperty(
                                            xDimProp,
                                            rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY) );
                    if ( nHierarchy >= xHiers->getCount() )
                        nHierarchy = 0;

                    uno::Reference<uno::XInterface> xHier =
                            ScUnoHelpFunctions::AnyToInterface(
                                                xHiers->getByIndex(nHierarchy) );
                    uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
                    if ( xHierSupp.is() )
                    {
                        uno::Reference<container::XIndexAccess> xLevels =
                                new ScNameToIndexAccess( xHierSupp->getLevels() );
                        long nLevCount = xLevels->getCount();
                        for (long nLev=0; nLev<nLevCount; nLev++)
                        {
                            uno::Reference<uno::XInterface> xLevel =
                                        ScUnoHelpFunctions::AnyToInterface(
                                                            xLevels->getByIndex(nLev) );
                            uno::Reference<container::XNamed> xLevNam( xLevel, uno::UNO_QUERY );
                            uno::Reference<sheet::XDataPilotMemberResults> xLevRes(
                                    xLevel, uno::UNO_QUERY );
                            if ( xLevNam.is() && xLevRes.is() )
                            {
                                String aCaption = String(xLevNam->getName());   //! Caption...
                                switch ( eDimOrient )
                                {
                                    case sheet::DataPilotFieldOrientation_COLUMN:
                                        pColFields[nColFieldCount].nDim    = nDim;
                                        pColFields[nColFieldCount].nHier   = nHierarchy;
                                        pColFields[nColFieldCount].nLevel  = nLev;
                                        pColFields[nColFieldCount].nDimPos = nDimPos;
                                        pColFields[nColFieldCount].aResult = xLevRes->getResults();
                                        pColFields[nColFieldCount].aCaption= aCaption;
                                        if (!lcl_MemberEmpty(pColFields[nColFieldCount].aResult))
                                            ++nColFieldCount;
                                        break;
                                    case sheet::DataPilotFieldOrientation_ROW:
                                        pRowFields[nRowFieldCount].nDim    = nDim;
                                        pRowFields[nRowFieldCount].nHier   = nHierarchy;
                                        pRowFields[nRowFieldCount].nLevel  = nLev;
                                        pRowFields[nRowFieldCount].nDimPos = nDimPos;
                                        pRowFields[nRowFieldCount].aResult = xLevRes->getResults();
                                        pRowFields[nRowFieldCount].aCaption= aCaption;
                                        if (!lcl_MemberEmpty(pRowFields[nRowFieldCount].aResult))
                                            ++nRowFieldCount;
                                        break;
                                    case sheet::DataPilotFieldOrientation_PAGE:
                                        pPageFields[nPageFieldCount].nDim    = nDim;
                                        pPageFields[nPageFieldCount].nHier   = nHierarchy;
                                        pPageFields[nPageFieldCount].nLevel  = nLev;
                                        pPageFields[nPageFieldCount].nDimPos = nDimPos;
                                        pPageFields[nPageFieldCount].aResult = xLevRes->getResults();
                                        pPageFields[nPageFieldCount].aCaption= aCaption;
                                        if (!lcl_MemberEmpty(pPageFields[nPageFieldCount].aResult))
                                            ++nPageFieldCount;
                                        break;
                                }

                                // get number formats from data dimensions
                                if ( bIsDataLayout )
                                {
                                    DBG_ASSERT( nLevCount == 1, "data layout: multiple levels?" );
                                    if ( eDimOrient == sheet::DataPilotFieldOrientation_COLUMN )
                                        lcl_FillNumberFormats( pColNumFmt, nColFmtCount, xLevRes, xDims );
                                    else if ( eDimOrient == sheet::DataPilotFieldOrientation_ROW )
                                        lcl_FillNumberFormats( pRowNumFmt, nRowFmtCount, xLevRes, xDims );
                                }
                            }
                        }
                    }
                }
            }
        }
        lcl_SortFields( pColFields, nColFieldCount );
        lcl_SortFields( pRowFields, nRowFieldCount );
        lcl_SortFields( pPageFields, nPageFieldCount );

        //  get data results:

        try
        {
            aData = xResult->getResults();
        }
        catch (uno::RuntimeException&)
        {
            bResultsError = TRUE;
        }
    }

    // get "DataDescription" property (may be missing in external sources)

    uno::Reference<beans::XPropertySet> xSrcProp( xSource, uno::UNO_QUERY );
    if ( xSrcProp.is() )
    {
        try
        {
            uno::Any aAny = xSrcProp->getPropertyValue(
                    rtl::OUString::createFromAscii(DP_PROP_DATADESCR) );
            rtl::OUString aUStr;
            aAny >>= aUStr;
            aDataDescription = String( aUStr );
        }
        catch(uno::Exception&)
        {
        }
    }
}

ScDPOutput::~ScDPOutput()
{
    delete[] pColFields;
    delete[] pRowFields;
    delete[] pPageFields;

    delete[] pColNumFmt;
    delete[] pRowNumFmt;
}

void ScDPOutput::SetPosition( const ScAddress& rPos )
{
    aStartPos = rPos;
    bSizesValid = bSizeOverflow = FALSE;
}

void ScDPOutput::DataCell( USHORT nCol, USHORT nRow, USHORT nTab, const sheet::DataResult& rData )
{
    long nFlags = rData.Flags;
    if ( nFlags & sheet::DataResultFlags::ERROR )
    {
        pDoc->SetError( nCol, nRow, nTab, errNoValue );
    }
    else if ( nFlags & sheet::DataResultFlags::HASDATA )
    {
        pDoc->SetValue( nCol, nRow, nTab, rData.Value );

        //  use number formats from source

        DBG_ASSERT( bSizesValid, "DataCell: !bSizesValid" );
        UINT32 nFormat = 0;
        if ( pColNumFmt )
        {
            if ( nCol >= nDataStartCol )
            {
                long nIndex = nCol - nDataStartCol;
                if ( nIndex < nColFmtCount )
                    nFormat = pColNumFmt[nIndex];
            }
        }
        else if ( pRowNumFmt )
        {
            if ( nRow >= nDataStartRow )
            {
                long nIndex = nRow - nDataStartRow;
                if ( nIndex < nRowFmtCount )
                    nFormat = pRowNumFmt[nIndex];
            }
        }
        if ( nFormat != 0 )
            pDoc->ApplyAttr( nCol, nRow, nTab, SfxUInt32Item( ATTR_VALUE_FORMAT, nFormat ) );
    }
    else
    {
        //pDoc->SetString( nCol, nRow, nTab, EMPTY_STRING );
    }

    //  SubTotal formatting is controlled by headers
}

void ScDPOutput::HeaderCell( USHORT nCol, USHORT nRow, USHORT nTab,
                                const sheet::MemberResult& rData, BOOL bColHeader, long nLevel )
{
    long nFlags = rData.Flags;
    if ( nFlags & sheet::MemberResultFlags::HASMEMBER )
    {
        pDoc->SetString( nCol, nRow, nTab, rData.Caption );
    }
    else
    {
        //pDoc->SetString( nCol, nRow, nTab, EMPTY_STRING );
    }

    if ( nFlags & sheet::MemberResultFlags::SUBTOTAL )
    {
//      SvxWeightItem aItem( WEIGHT_BOLD );     // weight is in the style

        //! limit frames to horizontal or vertical?
        if (bColHeader)
        {
//          lcl_AttrArea( pDoc,nTab, nCol,nMemberStartRow+(USHORT)nLevel, nCol,nTabEndRow, aItem );
            lcl_SetFrame( pDoc,nTab, nCol,nMemberStartRow+(USHORT)nLevel, nCol,nTabEndRow, 20 );
            lcl_SetStyleById( pDoc,nTab, nCol,nMemberStartRow+(USHORT)nLevel, nCol,nDataStartRow-1,
                                    STR_PIVOT_STYLE_TITLE );
            lcl_SetStyleById( pDoc,nTab, nCol,nDataStartRow, nCol,nTabEndRow,
                                    STR_PIVOT_STYLE_RESULT );
        }
        else
        {
//          lcl_AttrArea( pDoc,nTab, nMemberStartCol+(USHORT)nLevel,nRow, nTabEndCol,nRow, aItem );
            lcl_SetFrame( pDoc,nTab, nMemberStartCol+(USHORT)nLevel,nRow, nTabEndCol,nRow, 20 );
            lcl_SetStyleById( pDoc,nTab, nMemberStartCol+(USHORT)nLevel,nRow, nDataStartCol-1,nRow,
                                    STR_PIVOT_STYLE_TITLE );
            lcl_SetStyleById( pDoc,nTab, nDataStartCol,nRow, nTabEndCol,nRow,
                                    STR_PIVOT_STYLE_RESULT );
        }
    }
}

void ScDPOutput::FieldCell( USHORT nCol, USHORT nRow, USHORT nTab, const String& rCaption )
{
    pDoc->SetString( nCol, nRow, nTab, rCaption );
    lcl_SetFrame( pDoc,nTab, nCol,nRow, nCol,nRow, 20 );

    //  Button
    pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr(SC_MF_BUTTON) );

    lcl_SetStyleById( pDoc,nTab, nCol,nRow, nCol,nRow, STR_PIVOT_STYLE_FIELDNAME );
}

void lcl_DoFilterButton( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab )
{
    pDoc->SetString( nCol, nRow, nTab, ScGlobal::GetRscString(STR_CELL_FILTER) );
    pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr(SC_MF_BUTTON) );
}

void ScDPOutput::CalcSizes()
{
    if (!bSizesValid)
    {
        //  get column size of data from first row
        //! allow different sizes (and clear following areas) ???

        nRowCount = aData.getLength();
        const uno::Sequence<sheet::DataResult>* pRowAry = aData.getConstArray();
        nColCount = nRowCount ? ( pRowAry[0].getLength() ) : 0;
        nHeaderSize = 1;            // one row for field names

        //  calculate output positions and sizes

        long nPageSize = 0;     //! use page fields!
        if ( bDoFilter )
            nPageSize = 2;      //  filter button in page field row

        if ( aStartPos.Col() + nRowFieldCount + nColCount - 1 > MAXCOL ||
             aStartPos.Row() + nPageSize + nHeaderSize + nColFieldCount + nRowCount > MAXROW )
        {
            bSizeOverflow = TRUE;
        }

        nTabStartCol = aStartPos.Col();
        nTabStartRow = aStartPos.Row() + (USHORT)nPageSize;         // below page fields
        nMemberStartCol = nTabStartCol;
        nMemberStartRow = nTabStartRow + (USHORT) nHeaderSize;
        nDataStartCol = nMemberStartCol + (USHORT)nRowFieldCount;
        nDataStartRow = nMemberStartRow + (USHORT)nColFieldCount;
        nTabEndCol = nDataStartCol + (USHORT)nColCount - 1;
        nTabEndRow = nDataStartRow + (USHORT)nRowCount - 1;
        bSizesValid = TRUE;
    }
}

void ScDPOutput::Output()
{
    long nField;
    USHORT nTab = aStartPos.Tab();
    const uno::Sequence<sheet::DataResult>* pRowAry = aData.getConstArray();

    //  calculate output positions and sizes

    CalcSizes();
    if ( bSizeOverflow || bResultsError )   // does output area exceed sheet limits?
        return;                             // nothing

    //  clear whole (new) output area
    //! when modifying table, clear old area
    //! include IDF_OBJECTS ???
    pDoc->DeleteAreaTab( aStartPos.Col(), aStartPos.Row(), nTabEndCol, nTabEndRow, nTab, IDF_ALL );

    if ( bDoFilter )
        lcl_DoFilterButton( pDoc, aStartPos.Col(), aStartPos.Row(), nTab );

    //  data description
    //  (may get overwritten by first row field)

    String aDesc = aDataDescription;
    if ( !aDesc.Len() )
    {
        //! use default string ("result") ?
    }
    pDoc->SetString( nTabStartCol, nTabStartRow, nTab, aDesc );

    //  set STR_PIVOT_STYLE_INNER for whole data area (subtotals are overwritten)

    if ( nDataStartRow > nTabStartRow )
        lcl_SetStyleById( pDoc, nTab, nTabStartCol, nTabStartRow, nTabEndCol, nDataStartRow-1,
                            STR_PIVOT_STYLE_TOP );
    lcl_SetStyleById( pDoc, nTab, nDataStartCol, nDataStartRow, nTabEndCol, nTabEndRow,
                        STR_PIVOT_STYLE_INNER );

    //  output column headers:

    for (nField=0; nField<nColFieldCount; nField++)
    {
        USHORT nHdrCol = nDataStartCol + (USHORT)nField;                //! check for overflow
        FieldCell( nHdrCol, nTabStartRow, nTab, pColFields[nField].aCaption );

        USHORT nRowPos = nMemberStartRow + (USHORT)nField;              //! check for overflow
        const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisColCount = rSequence.getLength();
        DBG_ASSERT( nThisColCount == nColCount, "count mismatch" );     //! ???
        for (long nCol=0; nCol<nThisColCount; nCol++)
        {
            USHORT nColPos = nDataStartCol + (USHORT)nCol;              //! check for overflow
            HeaderCell( nColPos, nRowPos, nTab, pArray[nCol], TRUE, nField );
            if ( ( pArray[nCol].Flags & sheet::MemberResultFlags::HASMEMBER ) &&
                !( pArray[nCol].Flags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                if ( nField+1 < nColFieldCount )
                {
                    long nEnd = nCol;
                    while ( nEnd+1 < nThisColCount && ( pArray[nEnd+1].Flags & sheet::MemberResultFlags::CONTINUE ) )
                        ++nEnd;
                    USHORT nEndColPos = nDataStartCol + (USHORT)nEnd;       //! check for overflow
                    lcl_SetFrame( pDoc,nTab, nColPos,nRowPos, nEndColPos,nRowPos, 20 );
                    lcl_SetFrame( pDoc,nTab, nColPos,nRowPos, nEndColPos,nTabEndRow, 20 );

                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nEndColPos,nDataStartRow-1, STR_PIVOT_STYLE_CATEGORY );
                }
                else
                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nColPos,nDataStartRow-1, STR_PIVOT_STYLE_CATEGORY );
            }
        }
    }

    //  output row headers:

    for (nField=0; nField<nRowFieldCount; nField++)
    {
        USHORT nHdrCol = nTabStartCol + (USHORT)nField;                 //! check for overflow
        USHORT nHdrRow = nDataStartRow - 1;
        FieldCell( nHdrCol, nHdrRow, nTab, pRowFields[nField].aCaption );

        USHORT nColPos = nMemberStartCol + (USHORT)nField;              //! check for overflow
        const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisRowCount = rSequence.getLength();
        DBG_ASSERT( nThisRowCount == nRowCount, "count mismatch" );     //! ???
        for (long nRow=0; nRow<nThisRowCount; nRow++)
        {
            USHORT nRowPos = nDataStartRow + (USHORT)nRow;              //! check for overflow
            HeaderCell( nColPos, nRowPos, nTab, pArray[nRow], FALSE, nField );
            if ( ( pArray[nRow].Flags & sheet::MemberResultFlags::HASMEMBER ) &&
                !( pArray[nRow].Flags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                if ( nField+1 < nRowFieldCount )
                {
                    long nEnd = nRow;
                    while ( nEnd+1 < nThisRowCount && ( pArray[nEnd+1].Flags & sheet::MemberResultFlags::CONTINUE ) )
                        ++nEnd;
                    USHORT nEndRowPos = nDataStartRow + (USHORT)nEnd;       //! check for overflow
                    lcl_SetFrame( pDoc,nTab, nColPos,nRowPos, nColPos,nEndRowPos, 20 );
                    lcl_SetFrame( pDoc,nTab, nColPos,nRowPos, nTabEndCol,nEndRowPos, 20 );

                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nDataStartCol-1,nEndRowPos, STR_PIVOT_STYLE_CATEGORY );
                }
                else
                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nDataStartCol-1,nRowPos, STR_PIVOT_STYLE_CATEGORY );
            }
        }
    }

    //  output data results:

    for (long nRow=0; nRow<nRowCount; nRow++)
    {
        USHORT nRowPos = nDataStartRow + (USHORT)nRow;                  //! check for overflow
        const sheet::DataResult* pColAry = pRowAry[nRow].getConstArray();
        long nThisColCount = pRowAry[nRow].getLength();
        DBG_ASSERT( nThisColCount == nColCount, "count mismatch" );     //! ???
        for (long nCol=0; nCol<nThisColCount; nCol++)
        {
            USHORT nColPos = nDataStartCol + (USHORT)nCol;              //! check for overflow
            DataCell( nColPos, nRowPos, nTab, pColAry[nCol] );
        }
    }

    //  frame around the whole table

    lcl_SetFrame( pDoc,nTab, nDataStartCol,nDataStartRow, nTabEndCol,nTabEndRow, 20 );
    if ( nDataStartCol > nMemberStartCol )
        lcl_SetFrame( pDoc,nTab, nMemberStartCol,nDataStartRow, nDataStartCol-1,nTabEndRow, 20 );
    if ( nDataStartRow > nMemberStartRow )
        lcl_SetFrame( pDoc,nTab, nDataStartCol,nMemberStartRow, nTabEndCol,nDataStartRow-1, 20 );

    lcl_SetFrame( pDoc,nTab, nTabStartCol,nTabStartRow, nTabEndCol,nTabEndRow, 40 );
}

ScRange ScDPOutput::GetOutputRange()
{
    CalcSizes();

    USHORT nTab = aStartPos.Tab();
    return ScRange( aStartPos.Col(), aStartPos.Row(), nTab, nTabEndCol, nTabEndRow, nTab);
}

BOOL ScDPOutput::HasError()
{
    CalcSizes();

    return bSizeOverflow || bResultsError;
}

//
//      Methods to find specific parts of the table
//

void ScDPOutput::GetPositionData( ScDPPositionData& rData, const ScAddress& rPos )
{
    //! preset rData to "invalid" ?

    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return;                                     // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column field

    if ( nRow >= nMemberStartRow && nRow < nMemberStartRow + nColFieldCount )
    {
        long nField = nRow - nMemberStartRow;
        const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisColCount = rSequence.getLength();

        if ( nCol >= nDataStartCol && nCol < nDataStartCol + nThisColCount )
        {
            long nItem = nCol - nDataStartCol;
            //  get origin of "continue" fields
            while ( nItem > 0 && ( pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE ) )
                --nItem;
            rData.aMemberName = String(pArray[nItem].Name);
            rData.nFlags      = pArray[nItem].Flags;
            rData.nDimension  = pColFields[nField].nDim;
            rData.nHierarchy  = pColFields[nField].nHier;
            rData.nLevel      = pColFields[nField].nLevel;
            return;
        }
    }

    //  test for row field

    if ( nCol >= nMemberStartCol && nCol < nMemberStartCol + nRowFieldCount )
    {
        long nField = nCol - nMemberStartCol;
        const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisRowCount = rSequence.getLength();

        if ( nRow >= nDataStartRow && nRow < nDataStartRow + nThisRowCount )
        {
            long nItem = nRow - nDataStartRow;
            //  get origin of "continue" fields
            while ( nItem > 0 && ( pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE ) )
                --nItem;
            rData.aMemberName = String(pArray[nItem].Name);
            rData.nFlags      = pArray[nItem].Flags;
            rData.nDimension  = pRowFields[nField].nDim;
            rData.nHierarchy  = pRowFields[nField].nHier;
            rData.nLevel      = pRowFields[nField].nLevel;
            return;
        }
    }
}

BOOL ScDPOutput::IsFilterButton( const ScAddress& rPos )
{
    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() || !bDoFilter )
        return FALSE;                               // wrong sheet or no button at all

    //  filter button is at top left
    return ( nCol == aStartPos.Col() && nRow == aStartPos.Row() );
}

long ScDPOutput::GetHeaderDim( const ScAddress& rPos )
{
    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return -1;                                      // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column header

    if ( nRow == nTabStartRow && nCol >= nDataStartCol && nCol < nDataStartCol + nColFieldCount )
    {
        long nField = nCol - nDataStartCol;
        return pColFields[nField].nDim;
    }

    //  test for row header

    if ( nRow+1 == nDataStartRow && nCol >= nTabStartCol == nCol < nTabStartCol + nRowFieldCount )
    {
        long nField = nCol - nTabStartCol;
        return pRowFields[nField].nDim;
    }

    //! page fields
    //! single data field (?)

    return -1;      // invalid
}

BOOL ScDPOutput::GetHeaderDrag( const ScAddress& rPos, BOOL bMouseLeft, BOOL bMouseTop,
                                long nDragDim,
                                Rectangle& rPosRect, USHORT& rOrient, long& rDimPos )
{
    //  Rectangle instead of ScRange for rPosRect to allow for negative values

    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return FALSE;                                       // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column header

    if ( nCol >= nDataStartCol && nCol <= nTabEndCol &&
            nRow + 1 >= nMemberStartRow && nRow < nMemberStartRow + nColFieldCount )
    {
        long nField = nRow - nMemberStartRow;
        if (nField < 0)
        {
            nField = 0;
            bMouseTop = TRUE;
        }
        //! find start of dimension

        rPosRect = Rectangle( nDataStartCol, nMemberStartRow + nField,
                              nTabEndCol, nMemberStartRow + nField -1 );

        BOOL bFound = FALSE;            // is this within the same orientation?
        BOOL bBeforeDrag = FALSE;
        BOOL bAfterDrag = FALSE;
        for (long nPos=0; nPos<nColFieldCount && !bFound; nPos++)
        {
            if (pColFields[nPos].nDim == nDragDim)
            {
                bFound = TRUE;
                if ( nField < nPos )
                    bBeforeDrag = TRUE;
                else if ( nField > nPos )
                    bAfterDrag = TRUE;
            }
        }

        if ( bFound )
        {
            if (!bBeforeDrag)
            {
                ++rPosRect.Bottom();
                if (bAfterDrag)
                    ++rPosRect.Top();
            }
        }
        else
        {
            if ( !bMouseTop )
            {
                ++rPosRect.Top();
                ++rPosRect.Bottom();
                ++nField;
            }
        }

        rOrient = sheet::DataPilotFieldOrientation_COLUMN;
        rDimPos = nField;                       //!...
        return TRUE;
    }

    //  test for row header

    //  special case if no row fields
    BOOL bSpecial = ( nRow+1 >= nDataStartRow && nRow <= nTabEndRow &&
                        nRowFieldCount == 0 && nCol == nTabStartCol && bMouseLeft );

    if ( bSpecial || ( nRow+1 >= nDataStartRow && nRow <= nTabEndRow &&
                        nCol + 1 >= nTabStartCol && nCol < nTabStartCol + nRowFieldCount ) )
    {
        long nField = nCol - nTabStartCol;
        //! find start of dimension

        rPosRect = Rectangle( nTabStartCol + nField, nDataStartRow - 1,
                              nTabStartCol + nField - 1, nTabEndRow );

        BOOL bFound = FALSE;            // is this within the same orientation?
        BOOL bBeforeDrag = FALSE;
        BOOL bAfterDrag = FALSE;
        for (long nPos=0; nPos<nRowFieldCount && !bFound; nPos++)
        {
            if (pRowFields[nPos].nDim == nDragDim)
            {
                bFound = TRUE;
                if ( nField < nPos )
                    bBeforeDrag = TRUE;
                else if ( nField > nPos )
                    bAfterDrag = TRUE;
            }
        }

        if ( bFound )
        {
            if (!bBeforeDrag)
            {
                ++rPosRect.Right();
                if (bAfterDrag)
                    ++rPosRect.Left();
            }
        }
        else
        {
            if ( !bMouseLeft )
            {
                ++rPosRect.Left();
                ++rPosRect.Right();
                ++nField;
            }
        }

        rOrient = sheet::DataPilotFieldOrientation_ROW;
        rDimPos = nField;                       //!...
        return TRUE;
    }

    return FALSE;
}




/*************************************************************************
 *
 *  $RCSfile: dpobject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2000-10-19 15:29:41 $
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

#include "dpobject.hxx"
#include "dptabsrc.hxx"
#include "dpsave.hxx"
#include "dpoutput.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "pivot.hxx"        // PIVOT_DATA_FIELD
#include "dapiuno.hxx"      // ScDataPilotConversion
#include "miscuno.hxx"
#include "scerrors.hxx"
#include "refupdat.hxx"

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>

#include <unotools/processfactory.hxx>
#include <tools/debug.hxx>
#include <svtools/zforlist.hxx>     // IsNumberFormat

using namespace com::sun::star;

// -----------------------------------------------------------------------

#define MAX_LABELS 256 //!!! from fieldwnd.hxx, must be moved to global.hxx

// -----------------------------------------------------------------------

#define SCDPSOURCE_SERVICE  "com.sun.star.sheet.DataPilotSource"

// -----------------------------------------------------------------------

// incompatible versions of data pilot files
#define SC_DP_VERSION_CURRENT   6

// type of source data
#define SC_DP_SOURCE_SHEET      0
#define SC_DP_SOURCE_DATABASE   1
#define SC_DP_SOURCE_SERVICE    2

// -----------------------------------------------------------------------

//! move to a header file
#define DP_PROP_COLUMNGRAND         "ColumnGrand"
#define DP_PROP_FUNCTION            "Function"
#define DP_PROP_IGNOREEMPTY         "IgnoreEmptyRows"
#define DP_PROP_ISDATALAYOUT        "IsDataLayoutDimension"
//#define DP_PROP_ISVISIBLE         "IsVisible"
#define DP_PROP_ORIENTATION         "Orientation"
#define DP_PROP_ORIGINAL            "Original"
#define DP_PROP_POSITION            "Position"
#define DP_PROP_REPEATIFEMPTY       "RepeatIfEmpty"
#define DP_PROP_ROWGRAND            "RowGrand"
#define DP_PROP_SHOWDETAILS         "ShowDetails"
#define DP_PROP_SHOWEMPTY           "ShowEmpty"
#define DP_PROP_SUBTOTALS           "SubTotals"
#define DP_PROP_USEDHIERARCHY       "UsedHierarchy"

// -----------------------------------------------------------------------

USHORT lcl_GetDataGetOrientation( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    long nRet = sheet::DataPilotFieldOrientation_HIDDEN;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
        long nIntCount = xIntDims->getCount();
        BOOL bFound = FALSE;
        for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimProp.is() )
            {
                bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                    rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
                //! error checking -- is "IsDataLayoutDimension" property required??
                if (bFound)
                    nRet = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, rtl::OUString::createFromAscii(DP_PROP_ORIENTATION),
                            sheet::DataPilotFieldOrientation_HIDDEN );
            }
        }
    }
    return nRet;
}

// -----------------------------------------------------------------------

ScDPObject::ScDPObject( ScDocument* pD ) :
    pDoc( pD ),
    bAlive( FALSE ),
    pSaveData( NULL ),
    pSheetDesc( NULL ),
    pImpDesc( NULL ),
    pServDesc( NULL ),
    pOutput( NULL ),
    bSettingsChanged( FALSE )
{
}

ScDPObject::ScDPObject(const ScDPObject& r) :
    pDoc( r.pDoc ),
    bAlive( FALSE ),
    pSaveData( NULL ),
    aTableName( r.aTableName ),
    aTableTag( r.aTableTag ),
    aOutRange( r.aOutRange ),
    pSheetDesc( NULL ),
    pImpDesc( NULL ),
    pServDesc( NULL ),
    pOutput( NULL ),
    bSettingsChanged( FALSE )
{
    if (r.pSaveData)
        pSaveData = new ScDPSaveData(*r.pSaveData);
    if (r.pSheetDesc)
        pSheetDesc = new ScSheetSourceDesc(*r.pSheetDesc);
    if (r.pImpDesc)
        pImpDesc = new ScImportSourceDesc(*r.pImpDesc);
    if (r.pServDesc)
        pServDesc = new ScDPServiceDesc(*r.pServDesc);
    // xSource (and pOutput) is not copied
}

ScDPObject::~ScDPObject()
{
    delete pOutput;
    delete pSaveData;
    delete pSheetDesc;
    delete pImpDesc;
    delete pServDesc;
}

DataObject* ScDPObject::Clone() const
{
    return new ScDPObject(*this);
}

void ScDPObject::SetAlive(BOOL bSet)
{
    bAlive = bSet;
}

void ScDPObject::SetSaveData(const ScDPSaveData& rData)
{
    delete pSaveData;
    pSaveData = new ScDPSaveData( rData );

    InvalidateData();       // re-init source from SaveData
}

void ScDPObject::SetOutRange(const ScRange& rRange)
{
    aOutRange = rRange;

    if ( pOutput )
        pOutput->SetPosition( rRange.aStart );
}

void ScDPObject::SetSheetDesc(const ScSheetSourceDesc& rDesc)
{
    if ( pSheetDesc && rDesc == *pSheetDesc )
        return;             // nothing to do

    DELETEZ( pImpDesc );
    DELETEZ( pServDesc );

    delete pImpDesc;
    pSheetDesc = new ScSheetSourceDesc(rDesc);

    //  make valid QueryParam

    pSheetDesc->aQueryParam.nCol1 = pSheetDesc->aSourceRange.aStart.Col();
    pSheetDesc->aQueryParam.nRow1 = pSheetDesc->aSourceRange.aStart.Row();
    pSheetDesc->aQueryParam.nCol2 = pSheetDesc->aSourceRange.aEnd.Col();
    pSheetDesc->aQueryParam.nRow2 = pSheetDesc->aSourceRange.aEnd.Row();;
    pSheetDesc->aQueryParam.bHasHeader = TRUE;
    USHORT nCount = pSheetDesc->aQueryParam.GetEntryCount();
    for (USHORT i=0; (i<nCount) && (pSheetDesc->aQueryParam.GetEntry(i).bDoQuery); i++)
    {
        ScQueryEntry& rEntry = pSheetDesc->aQueryParam.GetEntry(i);
        ULONG nIndex = 0;
        rEntry.bQueryByString =
                    !(pDoc->GetFormatTable()->
                        IsNumberFormat(*rEntry.pStr, nIndex, rEntry.nVal));
    }

    InvalidateSource();     // new source must be created
}

void ScDPObject::SetImportDesc(const ScImportSourceDesc& rDesc)
{
    if ( pImpDesc && rDesc == *pImpDesc )
        return;             // nothing to do

    DELETEZ( pSheetDesc );
    DELETEZ( pServDesc );

    delete pImpDesc;
    pImpDesc = new ScImportSourceDesc(rDesc);

    InvalidateSource();     // new source must be created
}

void ScDPObject::SetServiceData(const ScDPServiceDesc& rDesc)
{
    if ( pServDesc && rDesc == *pServDesc )
        return;             // nothing to do

    DELETEZ( pSheetDesc );
    DELETEZ( pImpDesc );

    delete pServDesc;
    pServDesc = new ScDPServiceDesc(rDesc);

    InvalidateSource();     // new source must be created
}

void ScDPObject::WriteSourceDataTo( ScDPObject& rDest ) const
{
    if ( pSheetDesc )
        rDest.SetSheetDesc( *pSheetDesc );
    else if ( pImpDesc )
        rDest.SetImportDesc( *pImpDesc );
    else if ( pServDesc )
        rDest.SetServiceData( *pServDesc );

    //  name/tag are not source data, but needed along with source data

    rDest.aTableName = aTableName;
    rDest.aTableTag  = aTableTag;
}

BOOL ScDPObject::IsSheetData() const
{
    return ( pSheetDesc != NULL );
}

void ScDPObject::SetName(const String& rNew)
{
    aTableName = rNew;
}

void ScDPObject::SetTag(const String& rNew)
{
    aTableTag = rNew;
}

uno::Reference<sheet::XDimensionsSupplier> ScDPObject::GetSource()
{
    CreateObjects();
    return xSource;
}

void ScDPObject::CreateOutput()
{
    CreateObjects();
    if (!pOutput)
        pOutput = new ScDPOutput( pDoc, xSource, aOutRange.aStart, IsSheetData() );
}

void ScDPObject::CreateObjects()
{
    if (!xSource.is())
    {
        //! cache DPSource and/or Output?

        DBG_ASSERT( bAlive, "CreateObjects on non-inserted DPObject" );

        DELETEZ( pOutput );     // not valid when xSource is changed

        if ( pImpDesc )
        {
            ScDatabaseDPData* pData = new ScDatabaseDPData( pDoc->GetServiceManager(), *pImpDesc );
            xSource = new ScDPSource( pData );
        }
        else if ( pServDesc )
        {
            xSource = CreateSource( *pServDesc );
        }

        if ( !xSource.is() )    // sheet data or error in above cases
        {
            DBG_ASSERT( !pImpDesc && !pServDesc, "DPSource could not be created" );
            if (!pSheetDesc)
            {
                DBG_ERROR("no source descriptor");
                pSheetDesc = new ScSheetSourceDesc;     // dummy defaults
            }
            ScSheetDPData* pData = new ScSheetDPData( pDoc, *pSheetDesc );
            xSource = new ScDPSource( pData );
        }

        if (pSaveData)
            pSaveData->WriteToSource( xSource );
    }
    else if (bSettingsChanged)
    {
        DELETEZ( pOutput );     // not valid when xSource is changed

        uno::Reference<util::XRefreshable> xRef( xSource, uno::UNO_QUERY );
        if (xRef.is())
        {
            try
            {
                xRef->refresh();
            }
            catch(uno::Exception&)
            {
                DBG_ERROR("exception in refresh");
            }
        }

        if (pSaveData)
            pSaveData->WriteToSource( xSource );
    }
    bSettingsChanged = FALSE;
}

void ScDPObject::InvalidateData()
{
    bSettingsChanged = TRUE;
}

void ScDPObject::InvalidateSource()
{
    xSource = NULL;
}

ScRange ScDPObject::GetNewOutputRange()
{
    CreateOutput();             // create xSource and pOutput if not already done

    //  don't store the result in aOutRange, because nothing has been output yet
    return pOutput->GetOutputRange();
}

void ScDPObject::Output()
{
    //  clear old output area
    pDoc->DeleteAreaTab( aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                         aOutRange.aEnd.Col(), aOutRange.aEnd.Row(),
                         aOutRange.aStart.Tab(), IDF_ALL );

    CreateOutput();             // create xSource and pOutput if not already done

    pOutput->Output();

    //  aOutRange is always the range that was last output to the document
    aOutRange = pOutput->GetOutputRange();
}

void ScDPObject::UpdateReference( UpdateRefMode eUpdateRefMode,
                                     const ScRange& rRange, short nDx, short nDy, short nDz )
{
    // Output area

    USHORT nCol1 = aOutRange.aStart.Col();
    USHORT nRow1 = aOutRange.aStart.Row();
    USHORT nTab1 = aOutRange.aStart.Tab();
    USHORT nCol2 = aOutRange.aEnd.Col();
    USHORT nRow2 = aOutRange.aEnd.Row();
    USHORT nTab2 = aOutRange.aEnd.Tab();

    ScRefUpdateRes eRes =
        ScRefUpdate::Update( pDoc, eUpdateRefMode,
            rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
            nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    if ( eRes != UR_NOTHING )
        SetOutRange( ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 ) );

    // sheet source data

    if ( pSheetDesc )
    {
        nCol1 = pSheetDesc->aSourceRange.aStart.Col();
        nRow1 = pSheetDesc->aSourceRange.aStart.Row();
        nTab1 = pSheetDesc->aSourceRange.aStart.Tab();
        nCol2 = pSheetDesc->aSourceRange.aEnd.Col();
        nRow2 = pSheetDesc->aSourceRange.aEnd.Row();
        nTab2 = pSheetDesc->aSourceRange.aEnd.Tab();

        eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( eRes != UR_NOTHING )
        {
            ScSheetSourceDesc aNewDesc;
            aNewDesc.aSourceRange = ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );

            short nDiffX = nCol1 - (short) pSheetDesc->aSourceRange.aStart.Col();
            short nDiffY = nRow1 - (short) pSheetDesc->aSourceRange.aStart.Row();

            aNewDesc.aQueryParam = pSheetDesc->aQueryParam;
            aNewDesc.aQueryParam.nCol1 += nDiffX;
            aNewDesc.aQueryParam.nCol2 += nDiffX;
            aNewDesc.aQueryParam.nRow1 += nDiffY;   //! used?
            aNewDesc.aQueryParam.nRow2 += nDiffY;   //! used?
            USHORT nEC = aNewDesc.aQueryParam.GetEntryCount();
            for (USHORT i=0; i<nEC; i++)
                if (aNewDesc.aQueryParam.GetEntry(i).bDoQuery)
                    aNewDesc.aQueryParam.GetEntry(i).nField += nDiffX;

            SetSheetDesc( aNewDesc );       // allocates new pSheetDesc
        }
    }
}

BOOL ScDPObject::RefsEqual( const ScDPObject& r ) const
{
    if ( aOutRange != r.aOutRange )
        return FALSE;

    if ( pSheetDesc && r.pSheetDesc )
    {
        if ( pSheetDesc->aSourceRange != r.pSheetDesc->aSourceRange )
            return FALSE;
    }
    else if ( pSheetDesc || r.pSheetDesc )
    {
        DBG_ERROR("RefsEqual: SheetDesc set at only one object");
        return FALSE;
    }

    return TRUE;
}

void ScDPObject::WriteRefsTo( ScDPObject& r ) const
{
    r.SetOutRange( aOutRange );
    if ( pSheetDesc )
        r.SetSheetDesc( *pSheetDesc );
}

String ScDPObject::GetDimName( long nDim, BOOL& rIsDataLayout )
{
    rIsDataLayout = FALSE;
    String aRet;

    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimName.is() && xDimProp.is() )
            {
                BOOL bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
                //! error checking -- is "IsDataLayoutDimension" property required??

                rtl::OUString aName;
                try
                {
                    aName = xDimName->getName();
                }
                catch(uno::Exception&)
                {
                }
                if ( bData )
                    rIsDataLayout = TRUE;
                else
                    aRet = String( aName );
            }
        }
    }

    return aRet;
}

void ScDPObject::GetPositionData( ScDPPositionData& rData, const ScAddress& rPos )
{
    CreateOutput();             // create xSource and pOutput if not already done

    pOutput->GetPositionData( rData, rPos );
}

BOOL ScDPObject::IsFilterButton( const ScAddress& rPos )
{
    CreateOutput();             // create xSource and pOutput if not already done

    return pOutput->IsFilterButton( rPos );
}

long ScDPObject::GetHeaderDim( const ScAddress& rPos )
{
    CreateOutput();             // create xSource and pOutput if not already done

    return pOutput->GetHeaderDim( rPos );
}

BOOL ScDPObject::GetHeaderDrag( const ScAddress& rPos, BOOL bMouseLeft, BOOL bMouseTop, long nDragDim,
                                Rectangle& rPosRect, USHORT& rOrient, long& rDimPos )
{
    CreateOutput();             // create xSource and pOutput if not already done

    return pOutput->GetHeaderDrag( rPos, bMouseLeft, bMouseTop, nDragDim, rPosRect, rOrient, rDimPos );
}

void ScDPObject::ToggleDetails( ScDPPositionData& rElemDesc, ScDPObject* pDestObj )
{
    CreateObjects();            // create xSource if not already done

    //  find dimension name

    uno::Reference<container::XNamed> xDim;
    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    long nIntCount = xIntDims->getCount();
    if ( rElemDesc.nDimension < nIntCount )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface(
                                    xIntDims->getByIndex(rElemDesc.nDimension) );
        xDim = uno::Reference<container::XNamed>( xIntDim, uno::UNO_QUERY );
    }
    DBG_ASSERT( xDim.is(), "dimension not found" );
    if ( !xDim.is() ) return;
    String aDimName = xDim->getName();

    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    BOOL bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                        rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
    if (bDataLayout)
    {
        //  the elements of the data layout dimension can't be found by their names
        //  -> don't change anything
        return;
    }

    //  query old state

    long nHierCount = 0;
    uno::Reference<container::XIndexAccess> xHiers;
    uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
    if ( xHierSupp.is() )
    {
        uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
        xHiers = new ScNameToIndexAccess( xHiersName );
        nHierCount = xHiers->getCount();
    }
    uno::Reference<uno::XInterface> xHier;
    if ( rElemDesc.nHierarchy < nHierCount )
        xHier = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(rElemDesc.nHierarchy) );
    DBG_ASSERT( xHier.is(), "hierarchy not found" );
    if ( !xHier.is() ) return;

    long nLevCount = 0;
    uno::Reference<container::XIndexAccess> xLevels;
    uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHier, uno::UNO_QUERY );
    if ( xLevSupp.is() )
    {
        uno::Reference<container::XNameAccess> xLevsName = xLevSupp->getLevels();
        xLevels = new ScNameToIndexAccess( xLevsName );
        nLevCount = xLevels->getCount();
    }
    uno::Reference<uno::XInterface> xLevel;
    if ( rElemDesc.nLevel < nLevCount )
        xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(rElemDesc.nLevel) );
    DBG_ASSERT( xLevel.is(), "level not found" );
    if ( !xLevel.is() ) return;

    uno::Reference<container::XNameAccess> xMembers;
    uno::Reference<sheet::XMembersSupplier> xMbrSupp( xLevel, uno::UNO_QUERY );
    if ( xMbrSupp.is() )
        xMembers = xMbrSupp->getMembers();

    BOOL bFound = FALSE;
    BOOL bShowDetails = TRUE;

    if ( xMembers.is() )
    {
        rtl::OUString aName = rElemDesc.aMemberName;
        if ( xMembers->hasByName( aName ) )
        {
            uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
                                            xMembers->getByName( aName ) );
            uno::Reference<beans::XPropertySet> xMbrProp( xMemberInt, uno::UNO_QUERY );
            if ( xMbrProp.is() )
            {
                bShowDetails = ScUnoHelpFunctions::GetBoolProperty( xMbrProp,
                                    rtl::OUString::createFromAscii(DP_PROP_SHOWDETAILS) );
                //! don't set bFound if property is unknown?
                bFound = TRUE;
            }
        }
    }

    DBG_ASSERT( bFound, "member not found" );

    //! use Hierarchy and Level in SaveData !!!!

    //  modify pDestObj if set, this object otherwise
    ScDPSaveData* pModifyData = pDestObj ? ( pDestObj->pSaveData ) : pSaveData;
    DBG_ASSERT( pModifyData, "no data?" );
    if ( pModifyData )
    {
        pModifyData->GetDimensionByName(aDimName)->
            GetMemberByName(rElemDesc.aMemberName)->SetShowDetails( !bShowDetails );    // toggle

        if ( pDestObj )
            pDestObj->InvalidateData();     // re-init source from SaveData
        else
            InvalidateData();               // re-init source from SaveData
    }
}

long lcl_FindName( const rtl::OUString& rString, const uno::Reference<container::XNameAccess>& xCollection )
{
    if ( xCollection.is() )
    {
        uno::Sequence<rtl::OUString> aSeq = xCollection->getElementNames();
        long nCount = aSeq.getLength();
        const rtl::OUString* pArr = aSeq.getConstArray();
        for (long nPos=0; nPos<nCount; nPos++)
            if ( pArr[nPos] == rString )
                return nPos;
    }
    return -1;      // not found
}

USHORT lcl_FirstSubTotal( const uno::Reference<beans::XPropertySet>& xDimProp )     // PIVOT_FUNC mask
{
    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimSupp.is() )
    {
        uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
        long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY) );
        if ( nHierarchy >= xHiers->getCount() )
            nHierarchy = 0;

        uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                    xHiers->getByIndex(nHierarchy) );
        uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
        if ( xHierSupp.is() )
        {
            uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
            uno::Reference<uno::XInterface> xLevel =
                ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( 0 ) );
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            if ( xLevProp.is() )
            {
                uno::Any aSubAny;
                try
                {
                    aSubAny = xLevProp->getPropertyValue(
                            rtl::OUString::createFromAscii(DP_PROP_SUBTOTALS) );
                }
                catch(uno::Exception&)
                {
                }
                uno::Sequence<sheet::GeneralFunction> aSeq;
                if ( aSubAny >>= aSeq )
                {
                    USHORT nMask = 0;
                    const sheet::GeneralFunction* pArray = aSeq.getConstArray();
                    long nCount = aSeq.getLength();
                    for (long i=0; i<nCount; i++)
                        nMask |= ScDataPilotConversion::FunctionBit(pArray[i]);
                    return nMask;
                }
            }
        }
    }

    DBG_ERROR("FirstSubTotal: NULL");
    return 0;
}

USHORT lcl_CountBits( USHORT nBits )
{
    if (!nBits) return 0;

    USHORT nCount = 0;
    USHORT nMask = 1;
    for (USHORT i=0; i<16; i++)
    {
        if ( nBits & nMask )
            ++nCount;
        nMask <<= 1;
    }
    return nCount;
}

USHORT lcl_FillOldFields( PivotField* pFields,
                            const uno::Reference<sheet::XDimensionsSupplier>& xSource,
                            USHORT nOrient, USHORT nColAdd, BOOL bAddData )
{
    USHORT nOutCount = 0;
    BOOL bDataFound = FALSE;

    //! merge multiple occurences (data field with different functions)
    //! force data field in one dimension

    long nPos[PIVOT_MAXFIELD];

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    long nDimCount = xDims->getCount();
    for (long nDim=0; nDim < nDimCount && nOutCount < PIVOT_MAXFIELD; nDim++)
    {
        uno::Reference<uno::XInterface> xIntDim =
            ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
        long nDimOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, rtl::OUString::createFromAscii(DP_PROP_ORIENTATION),
                            sheet::DataPilotFieldOrientation_HIDDEN );
        if ( xDimProp.is() && nDimOrient == nOrient )
        {
            USHORT nMask = 0;
            if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                sheet::GeneralFunction eFunc = (sheet::GeneralFunction)ScUnoHelpFunctions::GetEnumProperty(
                                            xDimProp, rtl::OUString::createFromAscii(DP_PROP_FUNCTION),
                                            sheet::GeneralFunction_NONE );
                if ( eFunc == sheet::GeneralFunction_AUTO )
                {
                    //! test for numeric data
                    eFunc = sheet::GeneralFunction_SUM;
                }
                nMask = ScDataPilotConversion::FunctionBit(eFunc);
            }
            else
                nMask = lcl_FirstSubTotal( xDimProp );      // from first hierarchy

            BOOL bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                    rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
            uno::Any aOrigAny;
            try
            {
                aOrigAny = xDimProp->getPropertyValue(
                                rtl::OUString::createFromAscii(DP_PROP_ORIGINAL) );
            }
            catch(uno::Exception&)
            {
            }

            long nDupSource = -1;
            uno::Reference<uno::XInterface> xIntOrig = ScUnoHelpFunctions::AnyToInterface( aOrigAny );
            if ( xIntOrig.is() )
            {
                uno::Reference<container::XNamed> xNameOrig( xIntOrig, uno::UNO_QUERY );
                if ( xNameOrig.is() )
                    nDupSource = lcl_FindName( xNameOrig->getName(), xDimsName );
            }

            BOOL bDupUsed = FALSE;
            if ( nDupSource >= 0 )
            {
                //  add function bit to previous entry

                short nCompCol;
                if ( bDataLayout )
                    nCompCol = PIVOT_DATA_FIELD;
                else
                    nCompCol = (short)(nDupSource+nColAdd);     //! seek source column from name

                for (USHORT nOld=0; nOld<nOutCount; nOld++)
                    if ( pFields[nOld].nCol == nCompCol )
                    {
                        pFields[nOld].nFuncMask |= nMask;
                        pFields[nOld].nFuncCount = lcl_CountBits( pFields[nOld].nFuncMask );
                        bDupUsed = TRUE;
                    }
            }

            if ( !bDupUsed )        // also for duplicated dim if original has different orientation
            {
                if ( bDataLayout )
                {
                    pFields[nOutCount].nCol = PIVOT_DATA_FIELD;
                    bDataFound = TRUE;
                }
                else if ( nDupSource >= 0 )     // if source was not found (different orientation)
                    pFields[nOutCount].nCol = (short)(nDupSource+nColAdd);      //! seek from name
                else
                    pFields[nOutCount].nCol = (short)(nDim+nColAdd);    //! seek source column from name

                pFields[nOutCount].nFuncMask = nMask;
                pFields[nOutCount].nFuncCount = lcl_CountBits( nMask );
                nPos[nOutCount] = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                    rtl::OUString::createFromAscii(DP_PROP_POSITION) );
                ++nOutCount;
            }
        }
    }

    //  sort by getPosition() value

    for (long i=0; i+1<nOutCount; i++)
    {
        for (long j=0; j+i+1<nOutCount; j++)
            if ( nPos[j+1] < nPos[j] )
            {
                long nTemp = nPos[j+1];
                nPos[j+1] = nPos[j];
                nPos[j] = nTemp;
                PivotField aField = pFields[j+1];
                pFields[j+1] = pFields[j];
                pFields[j] = aField;
            }
    }

    if ( bAddData && !bDataFound )
    {
        if ( nOutCount >= PIVOT_MAXFIELD )              //  space for data field?
            --nOutCount;                                //! error?
        pFields[nOutCount].nCol = PIVOT_DATA_FIELD;
        pFields[nOutCount].nFuncMask = 0;
        pFields[nOutCount].nFuncCount = 0;
        ++nOutCount;
    }

    return nOutCount;
}

void lcl_SaveOldFieldArr( SvStream& rStream,
                            const uno::Reference<sheet::XDimensionsSupplier>& xSource,
                            USHORT nOrient, USHORT nColAdd, BOOL bAddData )
{
    // PIVOT_MAXFIELD = max. number in old files
    PivotField aFields[PIVOT_MAXFIELD];
    USHORT nOutCount = lcl_FillOldFields( aFields, xSource, nOrient, nColAdd, bAddData );

    rStream << nOutCount;
    for (USHORT i=0; i<nOutCount; i++)
    {
        rStream << (BYTE) 0x00
                << aFields[i].nCol
                << aFields[i].nFuncMask
                << aFields[i].nFuncCount;
    }
}

BOOL ScDPObject::StoreNew( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
{
    //  save all data

    rHdr.StartEntry();

    if ( pImpDesc )
    {
        rStream << (BYTE) SC_DP_SOURCE_DATABASE;
        rStream.WriteByteString( pImpDesc->aDBName, rStream.GetStreamCharSet() );
        rStream.WriteByteString( pImpDesc->aObject, rStream.GetStreamCharSet() );
        rStream << pImpDesc->nType;     // USHORT
        rStream << pImpDesc->bNative;
    }
    else if ( pServDesc )
    {
        rStream << (BYTE) SC_DP_SOURCE_SERVICE;
        rStream.WriteByteString( pServDesc->aServiceName, rStream.GetStreamCharSet() );
        rStream.WriteByteString( pServDesc->aParSource,   rStream.GetStreamCharSet() );
        rStream.WriteByteString( pServDesc->aParName,     rStream.GetStreamCharSet() );
        rStream.WriteByteString( pServDesc->aParUser,     rStream.GetStreamCharSet() );
        rStream.WriteByteString( pServDesc->aParPass,     rStream.GetStreamCharSet() );
    }
    else
    {
        if (!pSheetDesc)
        {
            DBG_ERROR("no source descriptor");
            ((ScDPObject*)this)->pSheetDesc = new ScSheetSourceDesc;        // dummy defaults
        }

        rStream << (BYTE) SC_DP_SOURCE_SHEET;
        rStream << pSheetDesc->aSourceRange;
        pSheetDesc->aQueryParam.Store( rStream );
    }

    rStream << aOutRange;

    DBG_ASSERT(pSaveData, "ScDPObject::StoreNew no SaveData");
    pSaveData->Store( rStream );

    //  additional data starting from 561b
    rStream.WriteByteString( aTableName, rStream.GetStreamCharSet() );
    rStream.WriteByteString( aTableTag,  rStream.GetStreamCharSet() );

    rHdr.EndEntry();
    return TRUE;
}

BOOL ScDPObject::LoadNew(SvStream& rStream, ScMultipleReadHeader& rHdr )
{
    rHdr.StartEntry();

    DELETEZ( pImpDesc );
    DELETEZ( pSheetDesc );
    DELETEZ( pServDesc );

    BYTE nType;
    rStream >> nType;
    switch (nType)
    {
        case SC_DP_SOURCE_DATABASE:
            pImpDesc = new ScImportSourceDesc;
            rStream.ReadByteString( pImpDesc->aDBName, rStream.GetStreamCharSet() );
            rStream.ReadByteString( pImpDesc->aObject, rStream.GetStreamCharSet() );
            rStream >> pImpDesc->nType;     // USHORT
            rStream >> pImpDesc->bNative;
            break;

        case SC_DP_SOURCE_SHEET:
            pSheetDesc = new ScSheetSourceDesc;
            rStream >> pSheetDesc->aSourceRange;
            pSheetDesc->aQueryParam.Load( rStream );
            break;

        case SC_DP_SOURCE_SERVICE:
            {
                String aServiceName, aParSource, aParName, aParUser, aParPass;
                rStream.ReadByteString( aServiceName, rStream.GetStreamCharSet() );
                rStream.ReadByteString( aParSource,   rStream.GetStreamCharSet() );
                rStream.ReadByteString( aParName,     rStream.GetStreamCharSet() );
                rStream.ReadByteString( aParUser,     rStream.GetStreamCharSet() );
                rStream.ReadByteString( aParPass,     rStream.GetStreamCharSet() );
                pServDesc = new ScDPServiceDesc( aServiceName,
                                        aParSource, aParName, aParUser, aParPass );
            }
            break;

        default:
            DBG_ERROR("unknown source type");
    }

    rStream >> aOutRange;

    SetSaveData(ScDPSaveData());
    pSaveData->Load( rStream );

    if (rHdr.BytesLeft())       //  additional data starting from 561b
    {
        rStream.ReadByteString( aTableName, rStream.GetStreamCharSet() );
        rStream.ReadByteString( aTableTag,  rStream.GetStreamCharSet() );
    }

    rHdr.EndEntry();
    return TRUE;
}

BOOL ScDPObject::StoreOld( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
{
    //  write compatible data for office 5.1 and below

    DBG_ASSERT( pSheetDesc, "StoreOld: !pSheetDesc" );
    ScRange aStoreRange;
    ScQueryParam aStoreQuery;
    if (pSheetDesc)
    {
        aStoreRange = pSheetDesc->aSourceRange;
        aStoreQuery = pSheetDesc->aQueryParam;
    }

    ((ScDPObject*)this)->CreateObjects();       // xSource is needed for field numbers

    rHdr.StartEntry();

    rStream << (BOOL) TRUE;         // bHasHeader

    rStream << aStoreRange.aStart.Col();
    rStream << aStoreRange.aStart.Row();
    rStream << aStoreRange.aEnd.Col();
    rStream << aStoreRange.aEnd.Row();
    rStream << aStoreRange.aStart.Tab();

    //! make sure aOutRange is initialized

    rStream << aOutRange.aStart.Col();
    rStream << aOutRange.aStart.Row();
    rStream << aOutRange.aEnd.Col();
    rStream << aOutRange.aEnd.Row();
    rStream << aOutRange.aStart.Tab();

    BOOL bAddData = ( lcl_GetDataGetOrientation( xSource ) == sheet::DataPilotFieldOrientation_HIDDEN );

    lcl_SaveOldFieldArr( rStream, xSource, sheet::DataPilotFieldOrientation_ROW,    aStoreRange.aStart.Col(), bAddData );
    lcl_SaveOldFieldArr( rStream, xSource, sheet::DataPilotFieldOrientation_COLUMN, aStoreRange.aStart.Col(), FALSE );
    lcl_SaveOldFieldArr( rStream, xSource, sheet::DataPilotFieldOrientation_DATA,   aStoreRange.aStart.Col(), FALSE );

    aStoreQuery.Store( rStream );

    BOOL bColumnGrand   = TRUE;
    BOOL bRowGrand      = TRUE;
    BOOL bIgnoreEmpty   = FALSE;
    BOOL bRepeatIfEmpty = FALSE;

    uno::Reference<beans::XPropertySet> xProp( xSource, uno::UNO_QUERY );
    if (xProp.is())
    {
        bColumnGrand = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_COLUMNGRAND), TRUE );
        bRowGrand = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_ROWGRAND), TRUE );

        // following properties may be missing for external sources
        bIgnoreEmpty = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_IGNOREEMPTY) );
        bRepeatIfEmpty = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_REPEATIFEMPTY) );
    }

    rStream << bIgnoreEmpty;        // bIgnoreEmpty
    rStream << bRepeatIfEmpty;      // bDetectCat

    rStream << bColumnGrand;        // bMakeTotalCol
    rStream << bRowGrand;           // bMakeTotalRow

    if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )
    {
        rStream.WriteByteString( aTableName, rStream.GetStreamCharSet() );
        rStream.WriteByteString( aTableTag,  rStream.GetStreamCharSet() );
        rStream << (USHORT)0;       // nColNameCount
    }

    rHdr.EndEntry();
    return TRUE;
}

BOOL ScDPObject::FillOldParam(ScPivotParam& rParam, BOOL bForFile) const
{
    ((ScDPObject*)this)->CreateObjects();       // xSource is needed for field numbers

    rParam.nCol = aOutRange.aStart.Col();
    rParam.nRow = aOutRange.aStart.Row();
    rParam.nTab = aOutRange.aStart.Tab();
    // ppLabelArr / nLabels is not changed

    USHORT nColAdd = 0;
    if ( bForFile )
    {
        // in old file format, columns are within document, not within source range

        DBG_ASSERT( pSheetDesc, "FillOldParam: bForFile, !pSheetDesc" );
        nColAdd = pSheetDesc->aSourceRange.aStart.Col();
    }

    BOOL bAddData = ( lcl_GetDataGetOrientation( xSource ) == sheet::DataPilotFieldOrientation_HIDDEN );
    rParam.nColCount  = lcl_FillOldFields( rParam.aColArr,
                            xSource, sheet::DataPilotFieldOrientation_COLUMN, nColAdd, bAddData );
    rParam.nRowCount  = lcl_FillOldFields( rParam.aRowArr,
                            xSource, sheet::DataPilotFieldOrientation_ROW,    nColAdd, FALSE );
    rParam.nDataCount = lcl_FillOldFields( rParam.aDataArr,
                            xSource, sheet::DataPilotFieldOrientation_DATA,   nColAdd, FALSE );

    uno::Reference<beans::XPropertySet> xProp( xSource, uno::UNO_QUERY );
    if (xProp.is())
    {
        try
        {
            rParam.bMakeTotalCol = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_COLUMNGRAND), TRUE );
            rParam.bMakeTotalRow = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_ROWGRAND), TRUE );

            // following properties may be missing for external sources
            rParam.bIgnoreEmptyRows = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_IGNOREEMPTY) );
            rParam.bDetectCategories = ScUnoHelpFunctions::GetBoolProperty( xProp,
                        rtl::OUString::createFromAscii(DP_PROP_REPEATIFEMPTY) );
        }
        catch(uno::Exception&)
        {
            // no error
        }
    }
    return TRUE;
}

BOOL lcl_ShowEmptyFromDim( const uno::Reference<beans::XPropertySet>& xDimProp )
{
    BOOL bRet = FALSE;

    uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimSupp.is() )
    {
        uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
        long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY) );
        if ( nHierarchy >= xHiers->getCount() )
            nHierarchy = 0;

        uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                    xHiers->getByIndex(nHierarchy) );
        uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
        if ( xHierSupp.is() )
        {
            uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
            uno::Reference<uno::XInterface> xLevel =
                ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( 0 ) );
            uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
            if ( xLevProp.is() )
                bRet = ScUnoHelpFunctions::GetBoolProperty( xLevProp,
                                    rtl::OUString::createFromAscii(DP_PROP_SHOWEMPTY) );
        }
    }

    return bRet;
}

BOOL ScDPObject::FillLabelData(ScPivotParam& rParam, BOOL* pShowAll, USHORT nShowAllMax) const
{
    ((ScDPObject*)this)->CreateObjects();

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
    long nDimCount = xDims->getCount();
    if ( nDimCount > MAX_LABELS )
        nDimCount = MAX_LABELS;
    if (!nDimCount)
        return FALSE;

    USHORT nOutCount = 0;
    LabelData** aLabelArr = new LabelData*[nDimCount];
    for (long nDim=0; nDim < nDimCount; nDim++)
    {
        String aFieldName;
        uno::Reference<uno::XInterface> xIntDim =
            ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
        uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
        uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );

        if ( xDimName.is() && xDimProp.is() )
        {
            BOOL bDuplicated = FALSE;
            BOOL bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                            rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
            //! error checking -- is "IsDataLayoutDimension" property required??

            try
            {
                aFieldName = String( xDimName->getName() );

                uno::Any aOrigAny = xDimProp->getPropertyValue(
                            rtl::OUString::createFromAscii(DP_PROP_ORIGINAL) );
                uno::Reference<uno::XInterface> xIntOrig;
                if ( (aOrigAny >>= xIntOrig) && xIntOrig.is() )
                    bDuplicated = TRUE;
            }
            catch(uno::Exception&)
            {
            }

            if ( aFieldName.Len() && !bData && !bDuplicated )
            {
                BOOL bIsValue = TRUE;       //! check
                USHORT nCol = nDim;         //! ???

                aLabelArr[nOutCount] = new LabelData( aFieldName, nCol, bIsValue );
                if ( pShowAll && nOutCount < nShowAllMax )
                    pShowAll[nOutCount] = lcl_ShowEmptyFromDim( xDimProp );

                ++nOutCount;
            }
        }
    }

    rParam.SetLabelData( aLabelArr, nOutCount );

    for (USHORT i=0; i<nOutCount; i++)
        delete aLabelArr[i];
    delete[] aLabelArr;

    return TRUE;
}


//------------------------------------------------------------------------
//  convert old pivot tables into new datapilot tables

String lcl_GetDimName( const uno::Reference<sheet::XDimensionsSupplier>& xSource, long nDim )
{
    rtl::OUString aName;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
            if (xDimName.is())
            {
                try
                {
                    aName = xDimName->getName();
                }
                catch(uno::Exception&)
                {
                }
            }
        }
    }
    return aName;
}

// static
void ScDPObject::ConvertOrientation( ScDPSaveData& rSaveData,
                            PivotField* pFields, USHORT nCount, USHORT nOrient,
                            ScDocument* pDoc, USHORT nRow, USHORT nTab,
                            const uno::Reference<sheet::XDimensionsSupplier>& xSource,
                            BOOL bOldDefaults,
                            PivotField* pRefColFields, USHORT nRefColCount,
                            PivotField* pRefRowFields, USHORT nRefRowCount )
{
    //  pDoc or xSource must be set
    DBG_ASSERT( pDoc || xSource.is(), "missing string source" );

    String aDocStr;
    ScDPSaveDimension* pDim;

    for (USHORT i=0; i<nCount; i++)
    {
        USHORT nCol = pFields[i].nCol;
        USHORT nFuncs = pFields[i].nFuncMask;
        if ( nCol == PIVOT_DATA_FIELD )
            pDim = rSaveData.GetDataLayoutDimension();
        else
        {
            if ( pDoc )
                pDoc->GetString( nCol, nRow, nTab, aDocStr );
            else
                aDocStr = lcl_GetDimName( xSource, nCol );  // cols must start at 0
            //! handle empty strings etc.
            pDim = rSaveData.GetDimensionByName(aDocStr);
        }

        if ( nOrient == sheet::DataPilotFieldOrientation_DATA )     // set summary function
        {
            //  generate an individual entry for each function
            BOOL bFirst = TRUE;

            //  if a dimension is used for column or row and data,
            //  use duplicated dimensions for all data occurrences
            if (pRefColFields)
                for (USHORT nRefCol=0; nRefCol<nRefColCount; nRefCol++)
                    if (pRefColFields[nRefCol].nCol == nCol)
                        bFirst = FALSE;
            if (pRefRowFields)
                for (USHORT nRefRow=0; nRefRow<nRefRowCount; nRefRow++)
                    if (pRefRowFields[nRefRow].nCol == nCol)
                        bFirst = FALSE;

            USHORT nMask = 1;
            for (USHORT nBit=0; nBit<16; nBit++)
            {
                if ( nFuncs & nMask )
                {
                    sheet::GeneralFunction eFunc = ScDataPilotConversion::FirstFunc( nMask );
                    if (bFirst)
                    {
                        pDim->SetOrientation( nOrient );
                        pDim->SetFunction( eFunc );
                        bFirst = FALSE;
                    }
                    else
                    {
                        ScDPSaveDimension* pDup = rSaveData.DuplicateDimension(pDim->GetName());
                        pDup->SetOrientation( nOrient );
                        pDup->SetFunction( eFunc );
                    }
                }
                nMask *= 2;
            }
        }
        else                                            // set SubTotals
        {
            pDim->SetOrientation( nOrient );

            USHORT nFuncArray[16];
            USHORT nFuncCount = 0;
            USHORT nMask = 1;
            for (USHORT nBit=0; nBit<16; nBit++)
            {
                if ( nFuncs & nMask )
                    nFuncArray[nFuncCount++] = ScDataPilotConversion::FirstFunc( nMask );
                nMask *= 2;
            }
            pDim->SetSubTotals( nFuncCount, nFuncArray );

            //  ShowEmpty was implicit in old tables,
            //  must be set for data layout dimension (not accessible in dialog)
            if ( bOldDefaults || nCol == PIVOT_DATA_FIELD )
                pDim->SetShowEmpty( TRUE );
        }
    }
}

void ScDPObject::InitFromOldPivot( const ScPivot& rOld, ScDocument* pDoc, BOOL bSetSource )
{
    ScDPSaveData aSaveData;

    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aArea;
    rOld.GetParam( aParam, aQuery, aArea );

    ConvertOrientation( aSaveData, aParam.aColArr, aParam.nColCount,
                            sheet::DataPilotFieldOrientation_COLUMN, pDoc, aArea.nRowStart, aArea.nTab,
                            uno::Reference<sheet::XDimensionsSupplier>(), TRUE );
    ConvertOrientation( aSaveData, aParam.aRowArr, aParam.nRowCount,
                            sheet::DataPilotFieldOrientation_ROW, pDoc, aArea.nRowStart, aArea.nTab,
                            uno::Reference<sheet::XDimensionsSupplier>(), TRUE );
    ConvertOrientation( aSaveData, aParam.aDataArr, aParam.nDataCount,
                            sheet::DataPilotFieldOrientation_DATA, pDoc, aArea.nRowStart, aArea.nTab,
                            uno::Reference<sheet::XDimensionsSupplier>(), TRUE,
                            aParam.aColArr, aParam.nColCount, aParam.aRowArr, aParam.nRowCount );

    aSaveData.SetIgnoreEmptyRows( rOld.GetIgnoreEmpty() );
    aSaveData.SetRepeatIfEmpty( rOld.GetDetectCat() );
    aSaveData.SetColumnGrand( rOld.GetMakeTotalCol() );
    aSaveData.SetRowGrand( rOld.GetMakeTotalRow() );

    SetSaveData( aSaveData );
    if (bSetSource)
    {
        ScSheetSourceDesc aDesc;
        aDesc.aSourceRange = rOld.GetSrcArea();
        rOld.GetQuery( aDesc.aQueryParam );
        SetSheetDesc( aDesc );
    }
    SetOutRange( rOld.GetDestArea() );

    aTableName = rOld.GetName();
    aTableTag  = rOld.GetTag();
}

// -----------------------------------------------------------------------

//  static
BOOL ScDPObject::HasRegisteredSources()
{
    BOOL bFound = FALSE;

    uno::Reference<lang::XMultiServiceFactory> xManager = utl::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        rtl::OUString::createFromAscii( SCDPSOURCE_SERVICE ) );
        if ( xEnum.is() && xEnum->hasMoreElements() )
            bFound = TRUE;
    }

    return bFound;
}

//  static
uno::Sequence<rtl::OUString> ScDPObject::GetRegisteredSources()
{
    long nCount = 0;
    uno::Sequence<rtl::OUString> aSeq(0);

    //  use implementation names...

    uno::Reference<lang::XMultiServiceFactory> xManager = utl::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        rtl::OUString::createFromAscii( SCDPSOURCE_SERVICE ) );
        if ( xEnum.is() )
        {
            while ( xEnum->hasMoreElements() )
            {
                uno::Any aAddInAny = xEnum->nextElement();
//              if ( aAddInAny.getReflection()->getTypeClass() == TypeClass_INTERFACE )
                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                        if ( xInfo.is() )
                        {
                            rtl::OUString sName = xInfo->getImplementationName();

                            aSeq.realloc( nCount+1 );
                            aSeq.getArray()[nCount] = sName;
                            ++nCount;
                        }
                    }
                }
            }
        }
    }

    return aSeq;
}

//  static
uno::Reference<sheet::XDimensionsSupplier> ScDPObject::CreateSource( const ScDPServiceDesc& rDesc )
{
    rtl::OUString aImplName = rDesc.aServiceName;
    uno::Reference<sheet::XDimensionsSupplier> xRet = NULL;

    uno::Reference<lang::XMultiServiceFactory> xManager = utl::getProcessServiceFactory();
    uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
    if ( xEnAc.is() )
    {
        uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
                                        rtl::OUString::createFromAscii( SCDPSOURCE_SERVICE ) );
        if ( xEnum.is() )
        {
            while ( xEnum->hasMoreElements() && !xRet.is() )
            {
                uno::Any aAddInAny = xEnum->nextElement();
//              if ( aAddInAny.getReflection()->getTypeClass() == TypeClass_INTERFACE )
                {
                    uno::Reference<uno::XInterface> xIntFac;
                    aAddInAny >>= xIntFac;
                    if ( xIntFac.is() )
                    {
                        uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
                        uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
                        if ( xFac.is() && xInfo.is() && xInfo->getImplementationName() == aImplName )
                        {
                            try
                            {
                                uno::Reference<uno::XInterface> xInterface = xFac->createInstance();
                                uno::Reference<lang::XInitialization> xInit( xInterface, uno::UNO_QUERY );
                                if (xInit.is())
                                {
                                    //  initialize
                                    uno::Sequence<uno::Any> aSeq(4);
                                    uno::Any* pArray = aSeq.getArray();
                                    pArray[0] <<= rtl::OUString( rDesc.aParSource );
                                    pArray[1] <<= rtl::OUString( rDesc.aParName );
                                    pArray[2] <<= rtl::OUString( rDesc.aParUser );
                                    pArray[3] <<= rtl::OUString( rDesc.aParPass );
                                    xInit->initialize( aSeq );
                                }
                                xRet = uno::Reference<sheet::XDimensionsSupplier>( xInterface, uno::UNO_QUERY );
                            }
                            catch(uno::Exception&)
                            {
                            }
                        }
                    }
                }
            }
        }
    }

    return xRet;
}

// -----------------------------------------------------------------------

ScDPCollection::ScDPCollection(ScDocument* pDocument) :
    pDoc( pDocument )
{
}

ScDPCollection::ScDPCollection(const ScDPCollection& r) :
    Collection(r),
    pDoc(r.pDoc)
{
}

ScDPCollection::~ScDPCollection()
{
}

DataObject* ScDPCollection::Clone() const
{
    return new ScDPCollection(*this);
}

BOOL ScDPCollection::StoreOld( SvStream& rStream ) const
{
    BOOL bSuccess = TRUE;

    USHORT nSheetCount = 0;
    USHORT i;
    for (i=0; i<nCount; i++)
        if ( ((const ScDPObject*)At(i))->IsSheetData() )
            ++nSheetCount;

    ScMultipleWriteHeader aHdr( rStream );

    rStream << nSheetCount;         // only tables from sheet data

    for (i=0; i<nCount && bSuccess; i++)
    {
        const ScDPObject* pObj = (const ScDPObject*)At(i);
        if ( pObj->IsSheetData() )
            bSuccess = pObj->StoreOld( rStream, aHdr );
    }

    return bSuccess;
}

BOOL ScDPCollection::StoreNew( SvStream& rStream ) const
{
    BOOL bSuccess = TRUE;

    ScMultipleWriteHeader aHdr( rStream );

    rStream << (long)SC_DP_VERSION_CURRENT;
    rStream << (long)nCount;

    for (USHORT i=0; i<nCount && bSuccess; i++)
        bSuccess = ((const ScDPObject*)At(i))->StoreNew( rStream, aHdr );

    return bSuccess;
}

BOOL ScDPCollection::LoadNew( SvStream& rStream )
{
    BOOL bSuccess = TRUE;

    FreeAll();
    ScMultipleReadHeader aHdr( rStream );

    long nVer;
    rStream >> nVer;

    //  check for all supported versions here..

    if ( nVer != SC_DP_VERSION_CURRENT )
    {
        DBG_ERROR("skipping unknown version of data pilot obejct");
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
        return FALSE;
    }

    long nNewCount;
    rStream >> nNewCount;
    for (long i=0; i<nNewCount; i++)
    {
        ScDPObject* pObj = new ScDPObject( pDoc );
        if ( pObj->LoadNew(rStream, aHdr) )
        {
            pObj->SetAlive( TRUE );
            Insert( pObj );
        }
        else
            delete pObj;
    }

    return bSuccess;
}

void ScDPCollection::UpdateReference( UpdateRefMode eUpdateRefMode,
                                         const ScRange& r, short nDx, short nDy, short nDz )
{
    for (USHORT i=0; i<nCount; i++)
        ((ScDPObject*)At(i))->UpdateReference( eUpdateRefMode, r, nDx, nDy, nDz );
}

BOOL ScDPCollection::RefsEqual( const ScDPCollection& r ) const
{
    if ( nCount != r.nCount )
        return FALSE;

    for (USHORT i=0; i<nCount; i++)
        if ( ! ((const ScDPObject*)At(i))->RefsEqual( *((const ScDPObject*)r.At(i)) ) )
            return FALSE;

    return TRUE;    // all equal
}

void ScDPCollection::WriteRefsTo( ScDPCollection& r ) const
{
    if ( nCount == r.nCount )
    {
        //! assert equal names?
        for (USHORT i=0; i<nCount; i++)
            ((const ScDPObject*)At(i))->WriteRefsTo( *((ScDPObject*)r.At(i)) );
    }
    else
        DBG_ERROR("WriteRefsTo: different count");
}

String ScDPCollection::CreateNewName( USHORT nMin ) const
{
    String aBase = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("DataPilot"));
    //! from Resource?

    for (USHORT nAdd=0; nAdd<=nCount; nAdd++)   //  nCount+1 tries
    {
        String aNewName = aBase;
        aNewName += String::CreateFromInt32( nMin + nAdd );
        BOOL bFound = FALSE;
        for (USHORT i=0; i<nCount && !bFound; i++)
            if (((const ScDPObject*)pItems[i])->GetName() == aNewName)
                bFound = TRUE;
        if (!bFound)
            return aNewName;            // found unused Name
    }
    return String();                    // should not happen
}

void ScDPCollection::EnsureNames()
{
    for (USHORT i=0; i<nCount; i++)
        if (!((const ScDPObject*)At(i))->GetName().Len())
            ((ScDPObject*)At(i))->SetName( CreateNewName() );
}

//------------------------------------------------------------------------
//  convert old pivot tables into new datapilot tables

void ScDPCollection::ConvertOldTables( ScPivotCollection& rOldColl )
{
    //  convert old pivot tables into new datapilot tables

    USHORT nOldCount = rOldColl.GetCount();
    for (USHORT i=0; i<nOldCount; i++)
    {
        ScDPObject* pNewObj = new ScDPObject(pDoc);
        pNewObj->InitFromOldPivot( *(rOldColl)[i], pDoc, TRUE );
        pNewObj->SetAlive( TRUE );
        Insert( pNewObj );
    }
    rOldColl.FreeAll();
}





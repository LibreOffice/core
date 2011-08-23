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

#include "ChXChartData.hxx"

// header for SvxServiceInfoHelper
#include <bf_svx/unoprov.hxx>
// header for class OGuard
// header for rtl_createUuid
#include <rtl/uuid.h>
// header for class Application
#include <vcl/svapp.hxx>

#include "chtmodel.hxx"
#include "memchrt.hxx"
namespace binfilter {

#ifndef SCH_ASCII_TO_OU
#define SCH_ASCII_TO_OU( s )  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( s ) )
#endif

using namespace ::com::sun::star;

#define SCH_BIN_MIN(a,b) ( ((a)<(b))? (a) : (b) )

// ----------------------------------------
// class ChXChartData
// ----------------------------------------

ChXChartData::ChXChartData( uno::Reference< lang::XComponent > xModel,
                            ChartModel* pModel ) :
        maListeners( GetMutex() ),
        mpModel( pModel )
{
    if( xModel.is())
    {
        uno::Reference< lang::XEventListener > xListener = this;
        xModel->addEventListener( xListener );
    }

    DBG_WARNING( "Parameter pModel is NULL." );
}

ChXChartData::~ChXChartData()
{}

void ChXChartData::DataModified( chart::ChartDataChangeEvent& aEvent )
{
    if( !maListeners.getLength() ) return;

    uno::Reference< uno::XInterface > xSrc( SAL_STATIC_CAST( cppu::OWeakObject*, this ), uno::UNO_QUERY );
    if( xSrc.is() )
        aEvent.Source = xSrc;

    cppu::OInterfaceIteratorHelper aIter( maListeners );

    while( aIter.hasMoreElements() )
    {
        uno::Reference< chart::XChartDataChangeEventListener > xListener( aIter.next(), uno::UNO_QUERY );
        xListener->chartDataChanged( aEvent );
    }
}

// generate a uniqueId
const uno::Sequence< sal_Int8 > & ChXChartData::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

ChXChartData* ChXChartData::getImplementation( uno::Reference< uno::XInterface > xData ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xData, uno::UNO_QUERY );
    if( xUT.is() )
        return reinterpret_cast< ChXChartData * >( xUT->getSomething( ChXChartData::getUnoTunnelId() ));
    else
        return NULL;
}

// XChartData
void SAL_CALL ChXChartData::addChartDataChangeEventListener(
    const uno::Reference< chart::XChartDataChangeEventListener >& aListener )
    throw( uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xIntf( aListener, uno::UNO_QUERY );
    if( xIntf.is() )
        maListeners.addInterface( xIntf );
}
    
void SAL_CALL ChXChartData::removeChartDataChangeEventListener(
    const uno::Reference< chart::XChartDataChangeEventListener >& aListener )
    throw( uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xIntf( aListener, uno::UNO_QUERY );
    if( xIntf.is() )
        maListeners.removeInterface( xIntf );
}

double SAL_CALL ChXChartData::getNotANumber() throw( ::com::sun::star::uno::RuntimeException )
{
    return DBL_MIN;
}

sal_Bool SAL_CALL ChXChartData::isNotANumber( double nNumber ) throw( ::com::sun::star::uno::RuntimeException )
{
    return (nNumber == getNotANumber());
}


// XServiceInfo
::rtl::OUString SAL_CALL ChXChartData::getImplementationName()
    throw( uno::RuntimeException )
{
    return SCH_ASCII_TO_OU( "ChXChartData" );
}

sal_Bool SAL_CALL ChXChartData::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartData::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq;
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.chart.ChartData");
    return aSeq;
}

// XUnoTunnel
sal_Int64 SAL_CALL ChXChartData::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( uno::RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                                 aIdentifier.getConstArray(), 16 ) )
    {
        return reinterpret_cast< sal_Int64 >( this );
    }
    return 0;
}

// XEventListener listens to disposing of XModel
void SAL_CALL ChXChartData::disposing( const lang::EventObject& Source ) throw( uno::RuntimeException )
{
    // XModel is disposed -> mpModel is invalid now
    mpModel = NULL;
}

// ----------------------------------------
// class ChXChartDataArray
// ----------------------------------------
uno::Sequence< uno::Type > ChXChartDataArray::maTypeSequence;

ChXChartDataArray::ChXChartDataArray( uno::Reference< lang::XComponent > xModel,
                                      ChartModel* pModel ) :
        ChXChartData( xModel, pModel )
{}

ChXChartDataArray::~ChXChartDataArray()
{}

// XInterface
uno::Any SAL_CALL ChXChartDataArray::queryInterface( const uno::Type& aType )
    throw( uno::RuntimeException )
{
    if( aType == ::getCppuType((const uno::Reference< chart::XChartDataArray >*)0) )
        return uno::makeAny( uno::Reference< chart::XChartDataArray >(this));
    else
        return ChXChartData::queryInterface( aType );
}

void SAL_CALL ChXChartDataArray::acquire() throw()
{
    ChXChartData::acquire();
}

void SAL_CALL ChXChartDataArray::release() throw()
{
    ChXChartData::release();
}


// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL ChXChartDataArray::getTypes() throw( uno::RuntimeException )
{
    if( maTypeSequence.getLength() == 0 )
    {
        uno::Sequence< uno::Type > aBaseTypes( ChXChartData::getTypes() );
        uno::Type* pBaseTypes = aBaseTypes.getArray();

        maTypeSequence.realloc( aBaseTypes.getLength() + 1 );	// keep updated!!
        uno::Type* pTypes = maTypeSequence.getArray();

        // additional interfaces (1)
        *pTypes++ = ::getCppuType((const uno::Reference< chart::XChartDataArray >*)0);

        for( sal_Int32 nType = aBaseTypes.getLength(); nType; nType-- )
            *pTypes++ = *pBaseTypes++;
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL ChXChartDataArray::getImplementationId() throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;	
}


// XChartDataArray
uno::Sequence< uno::Sequence< double > > SAL_CALL ChXChartDataArray::getData() throw( uno::RuntimeException )
{
    DBG_ASSERT( mpModel, "No Model" );
    if( ! mpModel )
        return uno::Sequence< uno::Sequence< double > >();

    SolarMutexGuard aGuard;

    double *pData;

    // get MemChart:
    SchMemChart* pDocData = mpModel->GetChartData();

    // number of rows and columns
    sal_Int32 nCol, nColCnt = pDocData->GetColCount();
    sal_Int32 nRow, nRowCnt = pDocData->GetRowCount();

    // create array
    uno::Sequence< uno::Sequence< double > > aSequence( nRowCnt );
    uno::Sequence< double >* pSeqArray = aSequence.getArray();
    for( nRow = 0; nRow < nRowCnt; nRow++ )
    {
        pSeqArray[ nRow ].realloc( nColCnt );
    }

    // copy data from MemChart to the array:
    for( nRow = 0; nRow < nRowCnt; nRow++ )
    {
        pData = pSeqArray[ nRow ].getArray(); // get row
        for( nCol = 0; nCol < nColCnt; nCol++ )
        {
            pData[ nCol ] = pDocData->GetData( (short)nCol, (short)nRow ); // copy data to column
        }
    }
    return aSequence;
}

void SAL_CALL ChXChartDataArray::setData( const uno::Sequence< uno::Sequence< double > >& aData )
    throw( uno::RuntimeException )
{
    DBG_ASSERT( mpModel, "No Model" );
    if( ! mpModel )
        return;

    SolarMutexGuard aGuard;

    // get number of rows/columns
    const uno::Sequence< double >* pSequence = aData.getConstArray();
    sal_Int32 nRow, nRowCnt = aData.getLength();
    sal_Int32 nCol, nColCnt = 0;

    const double* pData = NULL;
    if( nRowCnt )
    {
        pData = pSequence[ 0 ].getConstArray();
        nColCnt = pSequence->getLength();
    }
    
    // compare size to the size of the existing MemChart
    SchMemChart* pDocData = mpModel->GetChartData();

    if( nColCnt != (sal_Int32)pDocData->GetColCount() ||
        nRowCnt != (sal_Int32)pDocData->GetRowCount() )
    {   // change size, i.e. create new MemChart
        pDocData = new SchMemChart( (short)nColCnt, (short)nRowCnt );
        // save as much as possible (title, ...)
        pDocData->SetNonNumericData( *(mpModel->GetChartData()) );
    }

    // copy data
    for( nRow = 0; nRow < nRowCnt; nRow++ )
    {
        pData = pSequence[ nRow ].getConstArray();
        for( nCol = 0; nCol < nColCnt; nCol++ )
        {
            pDocData->SetData( (short)nCol, (short)nRow,pData[ nCol ] );
        }
    }
    
    // data is only changed if new MemChart was created (otherwise everything is already in the chart)
    // (Wird nur gewechselt, wenn neues erzeugt wurde (sonst ist schon alles im Chart))
    mpModel->ChangeChartData( *pDocData, FALSE );

    // call listeners
    chart::ChartDataChangeEvent aEvent;
    aEvent.Type = chart::ChartDataChangeType_ALL;
    aEvent.StartColumn = 0;
    aEvent.EndColumn = 0;
    aEvent.StartRow = 0;
    aEvent.EndRow = 0;

    DataModified( aEvent );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartDataArray::getRowDescriptions() throw( uno::RuntimeException )
{
    DBG_ASSERT( mpModel, "No Model" );
    if( ! mpModel )
        return uno::Sequence< ::rtl::OUString >();

    SolarMutexGuard aGuard;

    SchMemChart* pDocData = mpModel->GetChartData();
    sal_Int32 nRow, nRowCnt = pDocData->GetRowCount();

    uno::Sequence< ::rtl::OUString > aResult( nRowCnt );

    ::rtl::OUString* pResultArray = aResult.getArray();
    for( nRow = 0; nRow < nRowCnt; nRow++ )
    {
        pResultArray[ nRow ] = pDocData->GetRowText( (short)nRow );
    }

    return aResult;
}

void SAL_CALL ChXChartDataArray::setRowDescriptions( const uno::Sequence< ::rtl::OUString >& aRowDescriptions )
    throw( uno::RuntimeException )
{
    DBG_ASSERT( mpModel, "No Model" );
    if( ! mpModel )
        return;

    SolarMutexGuard aGuard;

    SchMemChart* pDocData = mpModel->GetChartData();
    sal_Int32 nRow, nRowCnt = SCH_BIN_MIN( (sal_Int32)pDocData->GetRowCount(), aRowDescriptions.getLength() );
    ::rtl::OUString* pString = ( (uno::Sequence< ::rtl::OUString >&)aRowDescriptions ).getArray();

    for( nRow = 0; nRow < nRowCnt; nRow++ )
    {
       pDocData->SetRowText( (short)nRow, pString[ nRow ] );
    }
    mpModel->BuildChart( FALSE );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartDataArray::getColumnDescriptions() throw( uno::RuntimeException )
{
    DBG_ASSERT( mpModel, "No Model" );
    if( ! mpModel )
        return uno::Sequence< ::rtl::OUString >();

    SolarMutexGuard aGuard;

    SchMemChart* pDocData = mpModel->GetChartData();
    sal_Int32 nCol, nColCnt = pDocData->GetColCount();
    uno::Sequence< ::rtl::OUString > aResult( nColCnt );

    ::rtl::OUString *pResultArray = aResult.getArray();
    for( nCol = 0; nCol < nColCnt; nCol++ )
    {
        pResultArray[ nCol ] = pDocData->GetColText( (short)nCol);
    }
    return aResult;
}

 void SAL_CALL ChXChartDataArray::setColumnDescriptions( const uno::Sequence< ::rtl::OUString >& aColumnDescriptions )
     throw( uno::RuntimeException )
{
    DBG_ASSERT( mpModel, "No Model" );
    if( ! mpModel )
        return;

    SolarMutexGuard aGuard;

    SchMemChart* pDocData = mpModel->GetChartData();
    sal_Int32 nCol, nColCnt = SCH_BIN_MIN( (sal_Int32)pDocData->GetColCount(), aColumnDescriptions.getLength() );

    ::rtl::OUString* pString = ( (uno::Sequence< ::rtl::OUString >&)aColumnDescriptions ).getArray();

    for( nCol = 0; nCol < nColCnt; nCol++ )
    {
       pDocData->SetColText( (short)nCol, pString[ nCol ] );
    }
    mpModel->BuildChart( FALSE );
}


// XChartData
 void SAL_CALL ChXChartDataArray::addChartDataChangeEventListener(
     const uno::Reference< chart::XChartDataChangeEventListener >& aListener )
     throw( uno::RuntimeException )
{
    ChXChartData::addChartDataChangeEventListener( aListener );
}

void SAL_CALL ChXChartDataArray::removeChartDataChangeEventListener(
    const uno::Reference< chart::XChartDataChangeEventListener >& aListener )
    throw( uno::RuntimeException )
{
    ChXChartData::removeChartDataChangeEventListener( aListener );
}

double SAL_CALL ChXChartDataArray::getNotANumber() throw( uno::RuntimeException )
{
    return ChXChartData::getNotANumber();
}

sal_Bool SAL_CALL ChXChartDataArray::isNotANumber( double nNumber ) throw( uno::RuntimeException )
{
    return ChXChartData::isNotANumber( nNumber );
}


// XServiceInfo
::rtl::OUString SAL_CALL ChXChartDataArray::getImplementationName()
    throw( uno::RuntimeException )
{
    return SCH_ASCII_TO_OU( "ChXChartDataArray" );
}

sal_Bool SAL_CALL ChXChartDataArray::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return ChXChartData::supportsService( ServiceName );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartDataArray::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq( ChXChartData::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.chart.ChartDataArray" );
    return aSeq;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

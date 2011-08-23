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

#include "ChXChartDocument.hxx"
#include "ChXDiagram.hxx"
#include "ChXChartData.hxx"
#include "ChXChartDataChangeEventListener.hxx"
#include "ChXChartDrawPage.hxx"

#include "ChartLegend.hxx"
#include "ChartTitle.hxx"
#include "ChartArea.hxx"


#include <bf_svx/UnoNamespaceMap.hxx>
#include <bf_svx/xmlgrhlp.hxx>

#include "mapprov.hxx"
// header for SvxChartLegendPosItem
#ifndef _SVX_CHRTITEM_HXX
#include "schattr.hxx"
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS

#include <bf_svtools/eitem.hxx>

#endif
#include "docshell.hxx"
#include "objid.hxx"

#include "schgroup.hxx"

// for access to table addresses
#include "memchrt.hxx"

// header for SvNumberFormatsSupplierObj
#include <bf_svtools/numuno.hxx>

// header for class OGuard
// header for class Application
#include <vcl/svapp.hxx>
#include <bf_svx/unofill.hxx>
#include <bf_svx/unoshcol.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "unonames.hxx"
#include <rtl/uuid.h>

namespace binfilter {

extern SchUnoPropertyMapProvider aSchMapProvider;

using namespace ::com::sun::star;
using ::rtl::OUString;

sal_Int32              ChXChartDocument::mnInstanceCounter = 0;
SchAddInCollection *   ChXChartDocument::mpAddInCollection = NULL;

SchAddInCollection& ChXChartDocument::GetAddInCollection()
{
    if( mpAddInCollection == NULL )
        mpAddInCollection = new SchAddInCollection();

    return *mpAddInCollection;
}

ChXChartDocument::ChXChartDocument( SchChartDocShell* pShell ) :
        SfxBaseModel( pShell ),
        m_pModel( NULL ),
        m_aPropSet( aSchMapProvider.GetMap( CHMAP_DOC, NULL ))
{
    if( pShell )
    {
        SolarMutexGuard aSolarGuard;

        m_pDocShell = pShell;
        if( m_pDocShell->GetModelPtr())
            SetChartModel( m_pDocShell->GetModelPtr());
    }
    else
        DBG_WARNING( "Parameter pShell is NULL." );

    ++mnInstanceCounter;
}

ChXChartDocument::~ChXChartDocument()
{
    if( m_rXDiagram.is() )
    {
        osl::Guard< osl::Mutex > aGuard( GetMutex());
        ChXDiagram* pDiagram = ChXDiagram::getImplementation( m_rXDiagram );
        if( pDiagram )
            pDiagram->SetDocShell( NULL );
        uno::Reference<lang::XComponent> xComponent (m_rXDiagram,uno::UNO_QUERY);
        if (xComponent.is())
        {
            try
            {
                xComponent->removeEventListener (this);
                xComponent->dispose();
            }
            catch( uno::RuntimeException & aEx )
            {
                DBG_ERROR1( "Exception caught in DTOR: %s",
                            ::rtl::OUStringToOString( aEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            }
        }
    }
    if( --mnInstanceCounter == 0 )
    {
        delete mpAddInCollection;
        mpAddInCollection = NULL;
    }
}

// public methods
void ChXChartDocument::setDiagramType( const ::rtl::OUString& aType,
                                       sal_Bool bKeepAddin /* = sal_False */ ) throw()
{
    SolarMutexGuard aGuard;

    if( ! m_pModel )
        return;

    // never show sorting dialog
    m_pModel->SetChartStatusFlag( CHS_USER_NOQUERY );

    // disable AddIn if requested
    if( bKeepAddin )
        m_pModel->SetChartStatusFlag( CHS_KEEP_ADDIN );
    else
        m_pModel->ResetChartStatusFlag( CHS_KEEP_ADDIN );

    // all chart types start with the prefix com.sun.star.chart
    if( 0 == aType.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_NAMESPACE )))
    {
        // get postfix using copy( beginIndex )
        ::rtl::OUString aPostfix = aType.copy( RTL_CONSTASCII_LENGTH( SCH_X_STR_CHTYPE_NAMESPACE ));

        if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_LINE )) == 0 )
            m_pModel->SetBaseType( CHTYPE_LINE );
        else if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_AREA )) == 0 )
            m_pModel->SetBaseType( CHTYPE_AREA );
        else if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_PIE )) == 0 )
            m_pModel->SetBaseType( CHTYPE_CIRCLE);
        else if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_BAR )) == 0 )
            m_pModel->SetBaseType( CHTYPE_BAR);
        else if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_XY )) == 0 )
            m_pModel->SetBaseType( CHTYPE_XY );
        else if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_NET )) == 0 )
            m_pModel->SetBaseType( CHTYPE_NET );
        else if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_DONUT )) == 0 )
            m_pModel->SetBaseType( CHTYPE_DONUT );
        else if( aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_STOCK )) == 0 )
            m_pModel->SetBaseType( CHTYPE_STOCK );
    }
}

sal_Bool ChXChartDocument::setBaseDiagramType( sal_Bool bSet ) throw()
{
    if( bSet )
    {
        if( maBaseDiagramType.getLength())
        {
            // 2nd param: keep reference to AddIn in model
            setDiagramType( maBaseDiagramType, sal_True );
            return sal_True;
        }
    }
    else		// unset
    {
        if( m_pModel )
            m_pModel->SetBaseType( CHTYPE_ADDIN );
        return sal_True;
    }

    return sal_False;
}

void ChXChartDocument::SetChartModel( ChartModel* pModel ) throw()
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());

    m_pModel = pModel;
    m_aPropSet = SvxItemPropertySet( aSchMapProvider.GetMap( CHMAP_DOC, m_pModel ));
}

void ChXChartDocument::RefreshData( const chart::ChartDataChangeEvent& aEvent ) throw()
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());

    // copy new data ( in member m_xChartData )
    if( m_xChartData.is() )
    {
        // get XChartDataArray representation of m_xChartData
        uno::Reference< chart::XChartDataArray > xDataArray( m_xChartData, uno::UNO_QUERY );

        if( xDataArray.is() )
        {
            // new data object using own doc shell
            uno::Reference< lang::XComponent > xComp( SAL_STATIC_CAST( SfxBaseModel*, this ));
            ChXChartDataArray* pNewDataArray = new ChXChartDataArray( xComp, m_pModel );
            chart::XChartDataArray* pSrcArray = xDataArray.get();

            // the get-methods re-retrieve the (changed) data from the SchMemChart
            pNewDataArray->setData( pSrcArray->getData() );
            pNewDataArray->setRowDescriptions( pSrcArray->getRowDescriptions() );
            pNewDataArray->setColumnDescriptions( pSrcArray->getColumnDescriptions() );

            // remember the new data as member
            m_xChartData = SAL_STATIC_CAST( ChXChartData*, pNewDataArray );
        }
    }
}

// generate a uniqueId
const uno::Sequence< sal_Int8 > & ChXChartDocument::getUnoTunnelId() throw()
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

ChXChartDocument* ChXChartDocument::getImplementation( uno::Reference< uno::XInterface > xData ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xData, uno::UNO_QUERY );
    if( xUT.is() )
        return (ChXChartDocument*)xUT->getSomething( ChXChartDocument::getUnoTunnelId() );
    else
        return NULL;
}


// XInterface
uno::Any SAL_CALL ChXChartDocument::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aAny = SfxBaseModel::queryInterface( rType );
    if( aAny.hasValue() )
        return aAny;

    return ::cppu::queryInterface(
        rType,
        ( lang::XMultiServiceFactory* )this,
        ( beans::XPropertySet* )this,
        ( chart::XChartDocument* )this,
        ( lang::XServiceInfo* )this,
        ( util::XNumberFormatsSupplier* )this,
        ( drawing::XDrawPageSupplier* )this,
        ( lang::XUnoTunnel* )this );
}

void SAL_CALL ChXChartDocument::acquire() throw()
{
    SfxBaseModel::acquire();
}

void SAL_CALL ChXChartDocument::release() throw()
{
    SfxBaseModel::release();
}

// XMultiServiceFactory
uno::Reference< uno::XInterface > SAL_CALL ChXChartDocument::createInstance( const ::rtl::OUString& aServiceSpecifier )
    throw( uno::Exception, uno::RuntimeException )
{
    bool bServiceFound = false;
    uno::Reference< uno::XInterface > xResult;

    // 1. create a new built-in diagram type
    if( 0 == aServiceSpecifier.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_NAMESPACE )))
    {
        ::rtl::OUString aPostfix = aServiceSpecifier.copy( RTL_CONSTASCII_LENGTH( SCH_X_STR_CHTYPE_NAMESPACE ));

        if( 0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_LINE )) ||
            0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_AREA )) ||
            0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_BAR )) ||
            0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_PIE )) ||
            0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_XY )) ||
            0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_NET )) ||
            0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_DONUT )) ||
            0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_STOCK )))
        {
            ChXDiagram* pDiagram = new ChXDiagram( m_pDocShell, FALSE );
            pDiagram->SetServiceName( aServiceSpecifier );

            xResult.set( static_cast< cppu::OWeakObject * >( pDiagram ) );
            bServiceFound = true;
        }
    }
    // 2. create drawing services for XML
    else if( 0 == aServiceSpecifier.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing." )))
    {
        ::rtl::OUString aPostfix = aServiceSpecifier.copy( RTL_CONSTASCII_LENGTH( "com.sun.star.drawing." ));

        bServiceFound = true;

        // get a table for XML ex-/import
        if( 0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "DashTable" )))
        {
            if( ! xDashTable.is())
                xDashTable = SvxUnoDashTable_createInstance( m_pModel );
            xResult = xDashTable;
        }
        else if( 0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "GradientTable" )))
        {
            if( ! xGradientTable.is())
                xGradientTable = SvxUnoGradientTable_createInstance( m_pModel );
            xResult = xGradientTable;
        }
        else if( 0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "HatchTable" )))
        {
            if( ! xHatchTable.is())
                xHatchTable = SvxUnoHatchTable_createInstance( m_pModel );
            xResult = xHatchTable;
        }
        else if( 0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "BitmapTable" )))
        {
            if( ! xBitmapTable.is())
                xBitmapTable = SvxUnoBitmapTable_createInstance( m_pModel );
            xResult = xBitmapTable;
        }
        else if( 0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "TransparencyGradientTable" )))
        {
            if( ! xTransparencyGradientTable.is())
                xTransparencyGradientTable = SvxUnoTransGradientTable_createInstance( m_pModel );
            xResult = xTransparencyGradientTable;
        }
        else if( 0 == aPostfix.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "MarkerTable" )))
        {
            if( ! xMarkerTable.is())
                xMarkerTable = SvxUnoMarkerTable_createInstance( m_pModel );
            xResult = xMarkerTable;
        }
        else
            bServiceFound = false;
    }

    // 3. XML namespace-map for alien attributes
    if( ! bServiceFound &&
        0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.xml.NamespaceMap") ) )
    {
        static sal_uInt16 aWhichIds[] = { SCHATTR_USER_DEFINED_ATTR, 0 };
        xResult = svx::NamespaceMap_createInstance( aWhichIds, &m_pModel->GetPool() );
        bServiceFound = true;
    }

    // 4. graphics resolver
    if( ! bServiceFound &&
        0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.document.ExportGraphicObjectResolver") ) )
    {
        xResult = static_cast< ::cppu::OWeakObject * >( new SvXMLGraphicHelper( GRAPHICHELPER_MODE_WRITE ));
        bServiceFound = true;
    }
    if( ! bServiceFound &&
        0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.document.ImportGraphicObjectResolver") ) )
    {
        xResult = static_cast< ::cppu::OWeakObject * >( new SvXMLGraphicHelper( GRAPHICHELPER_MODE_READ ));
        bServiceFound = true;
    }

    // 5. embedded object resolver
    if( ! bServiceFound &&
        ( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.document.ExportEmbeddedObjectResolver" ))
          || 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.document.ImportEmbeddedObjectResolver" ))))
    {
        // Charts do not support EmbeddedObjectResolvers, so they return an empty
        // reference, but as the request is well known, no assertion is printed
        bServiceFound = true;
    }

    // 6. create a shape
    if( ! bServiceFound )
    {
        // try to get a shape
        try
        {
            xResult = SvxUnoDrawMSFactory::createInstance( aServiceSpecifier );
            bServiceFound = true;
        }
        catch( const uno::Exception& aEx )
        {
            // couldn't create shape
        }
    }

    // 7. try to create an AddIn
    if( ! bServiceFound )
    {
        uno::Reference< util::XRefreshable > xAddIn = GetAddInCollection().GetAddInByName( aServiceSpecifier );
        if( xAddIn.is())
        {
            xResult = xAddIn;
            bServiceFound = true;
        }
    }

    OSL_ENSURE( bServiceFound,
                ::rtl::OUStringToOString(
                    ::rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "createInstance() failed in creating the service " )) +
                    aServiceSpecifier,
                    RTL_TEXTENCODING_ASCII_US ).getStr() );

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL ChXChartDocument::createInstanceWithArguments(
    const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
    throw( uno::Exception, uno::RuntimeException )
{
    if( 0 == ServiceSpecifier.compareToAscii( RTL_CONSTASCII_STRINGPARAM( SCH_X_STR_CHTYPE_NAMESPACE )))
    {
        if( Arguments.getLength() )
            throw lang::IllegalArgumentException();
        else
            return createInstance( ServiceSpecifier );
    }

    return SvxUnoDrawMSFactory::createInstanceWithArguments( ServiceSpecifier, Arguments );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartDocument::getAvailableServiceNames()
    throw( uno::RuntimeException )
{
    ::std::vector< OUString > aServices;

    // chart types
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_LINE ));
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_AREA ));
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_BAR ));
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_PIE ));
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_XY ));
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_NET ));
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_DONUT ));
    aServices.push_back( OUString::createFromAscii( SCH_X_STR_CHTYPE_NAMESPACE SCH_X_STR_CHTYPE_STOCK ));

    // style tables (for XML)
    aServices.push_back( OUString::createFromAscii( "com.sun.star.drawing.DashTable" ));
    aServices.push_back( OUString::createFromAscii( "com.sun.star.drawing.GradientTable" ));
    aServices.push_back( OUString::createFromAscii( "com.sun.star.drawing.HatchTable" ));
    aServices.push_back( OUString::createFromAscii( "com.sun.star.drawing.BitmapTable" ));
    aServices.push_back( OUString::createFromAscii( "com.sun.star.drawing.TransparencyGradientTable" ));
    aServices.push_back( OUString::createFromAscii( "com.sun.star.drawing.MarkerTable" ));

    // XML namespacemap / object resolver
    aServices.push_back( OUString::createFromAscii( "com.sun.star.xml.NamespaceMap" ));
    aServices.push_back( OUString::createFromAscii( "com.sun.star.document.ExportGraphicObjectResolver" ));
    aServices.push_back( OUString::createFromAscii( "com.sun.star.document.ImportGraphicObjectResolver" ));

    // shapes
    uno::Sequence< OUString > aDrawServices( SvxUnoDrawMSFactory::getAvailableServiceNames() );
    const OUString * pArr = aDrawServices.getConstArray();
    aServices.insert( aServices.end(), pArr, pArr + aDrawServices.getLength() );

    // add-ins
    uno::Sequence< OUString > aAddIns( GetAddInCollection().GetAddInNames() );
    pArr = aAddIns.getConstArray();
    aServices.insert( aServices.end(), pArr, pArr + aAddIns.getLength() );

    return uno::Sequence< OUString >( &(* aServices.begin()), aServices.size() );
}


// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ChXChartDocument::getPropertySetInfo()
    throw( uno::RuntimeException )
{
    return m_aPropSet.getPropertySetInfo();
}

void SAL_CALL ChXChartDocument::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( m_pModel )
    {
        const SfxItemPropertyMap* pMap = m_aPropSet.getPropertyMapEntry( aPropertyName );
        if( pMap && pMap->nWID )
        {
            if( pMap->nFlags & beans::PropertyAttribute::READONLY )
                throw beans::PropertyVetoException();

            USHORT nWID = pMap->nWID;

            switch( nWID )
            {
                case CHATTR_DIAGRAM_START:		// dummy id for "HasLegend"
                    {
                        sal_Bool bVal;
                        aValue >>= bVal;
                        m_pModel->SetShowLegend( bVal );
                        m_pModel->SetLegendHasBeenMoved( FALSE );
                        m_pModel->BuildChart( FALSE );
                    }
                    break;

                case CHATTR_ADDR_CATEGORIES:
                case CHATTR_ADDR_SERIES:
                case CHATTR_ADDR_CHART:
                case CHATTR_TABLE_NUMBER_LIST:
                case CHATTR_EXPORT_TABLE:
                case CHATTR_FIRST_COL_LABELS:
                case CHATTR_FIRST_ROW_LABELS:
                    {
                        SchMemChart* pData = m_pModel->GetChartData();
                        if( pData )
                        {
                            switch( nWID )
                            {
                                case CHATTR_ADDR_CATEGORIES:
                                    {
                                        ::rtl::OUString aCatAddr;
                                        aValue >>= aCatAddr;
                                        pData->SetCategoriesRangeAddress( aCatAddr );
                                    }
                                    break;
                                case CHATTR_ADDR_SERIES:
                                    {
                                        uno::Sequence< chart::ChartSeriesAddress > aSeriesAddresses;
                                        aValue >>= aSeriesAddresses;
                                        pData->SetSeriesAddresses( aSeriesAddresses );
                                    }
                                    break;
                                case CHATTR_ADDR_CHART:
                                    {
                                        ::rtl::OUString aAddr;
                                        aValue >>= aAddr;
                                        pData->getChartRangeForXMLString( aAddr );
                                    }
                                    break;
                                case CHATTR_TABLE_NUMBER_LIST:
                                    {
                                        ::rtl::OUString aList;
                                        aValue >>= aList;
                                        pData->parseTableNumberList( aList );
                                    }
                                    break;
                                case CHATTR_EXPORT_TABLE:
                                    {
                                        sal_Bool bValueToSet;
                                        aValue >>= bValueToSet;

                                        SchChartRange aRange = pData->GetChartRange();
                                        if( aRange.mbKeepCopyOfData != bValueToSet )
                                        {
                                            aRange.mbKeepCopyOfData = bValueToSet;
                                            pData->SetChartRange( aRange );
                                        }
                                    }
                                    break;
                                case CHATTR_FIRST_COL_LABELS:
                                    {
                                        sal_Bool bValueToSet;
                                        aValue >>= bValueToSet;

                                        SchChartRange aRange = pData->GetChartRange();
                                        if( aRange.mbFirstColumnContainsLabels != bValueToSet )
                                        {
                                            aRange.mbFirstColumnContainsLabels = bValueToSet;
                                            pData->SetChartRange( aRange );
                                        }
                                    }
                                    break;
                                case CHATTR_FIRST_ROW_LABELS:
                                    {
                                        sal_Bool bValueToSet;
                                        aValue >>= bValueToSet;

                                        SchChartRange aRange = pData->GetChartRange();
                                        if( aRange.mbFirstRowContainsLabels != bValueToSet )
                                        {
                                            aRange.mbFirstRowContainsLabels = bValueToSet;
                                            pData->SetChartRange( aRange );
                                        }
                                    }
                                    break;
                            }
                        }
                        else
                            DBG_ERROR( "invalid SchMemChart" );
                    }
                    break;

                case CHATTR_TRANSLATED_COLS:
                case CHATTR_TRANSLATED_ROWS:
                    {
                        SchMemChart* pData = m_pModel->GetChartData();
                        if( pData )
                        {
                            uno::Sequence< sal_Int32 > aSeq;
                            aValue >>= aSeq;

                            if( ! SetTransSequence( pData, CHATTR_TRANSLATED_COLS == nWID, aSeq ))
                            {
                                ::rtl::OUString aMsg(
                                    RTL_CONSTASCII_STRINGPARAM( "Conflict on setting row-/col translation" ),
                                    RTL_TEXTENCODING_ASCII_US );
                                uno::RuntimeException aExcept( aMsg, static_cast< ::cppu::OWeakObject* >( this ));
                                throw aExcept;
                            }
                        }
                        else
                            DBG_ERROR( "invalid SchMemChart" );
                    }
                    break;

                default:
                    {
                        SfxItemSet aSet( m_pModel->GetPool(), nWID, nWID );
                        m_pModel->GetAttr( aSet );

                        if( ! aSet.Count() )
                        {
                            // get default from ItemPool
                            if( m_pModel->GetItemPool().IsWhich( nWID ) )
                            {
                                aSet.Put( m_pModel->GetItemPool().GetDefaultItem( pMap->nWID ) );
                            }
                            else
                            {
#ifdef DBG_UTIL
                                String aTmpString( aPropertyName );
                                ByteString aProp( aTmpString, RTL_TEXTENCODING_ASCII_US );
                                DBG_ERROR2( "Diagram: Property %s has an invalid ID (%d)", aProp.GetBuffer(), nWID );
#endif
                            }
                        }
                        if( aSet.Count())
                        {
                            m_aPropSet.setPropertyValue( pMap, aValue, aSet );
                            m_pModel->PutAttr( aSet );
                            m_pModel->BuildChart( FALSE );
                        }
                        else
                        {
                            throw beans::UnknownPropertyException();
                        }
                    }
                    break;
            }
        }
        else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BaseDiagram" ) ))
        {
            aValue >>= maBaseDiagramType;
            setBaseDiagramType( sal_True );
        }
        else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ExportForClipboard" )))
        {
            sal_Bool bBool;
            aValue >>= bBool;
            m_pDocShell->SetClipboardExport( bBool );
        }
        else
        {
            throw beans::UnknownPropertyException();
        }
    }
    else
        DBG_WARNING( "No Model" );
}

uno::Any SAL_CALL ChXChartDocument::getPropertyValue( const ::rtl::OUString& aPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    uno::Any aAny;
    if( m_pModel )
    {
        const SfxItemPropertyMap* pMap = m_aPropSet.getPropertyMapEntry( aPropertyName );
        if( pMap && pMap->nWID )
        {
            USHORT nWID = pMap->nWID;

            switch( nWID )
            {
                case CHATTR_ADDITIONAL_SHAPES:
                    aAny <<= GetAdditionalShapes();
                    return aAny;        // RETURN

                case CHATTR_DIAGRAM_START:		// dummy id for "HasLegend"
                    {
                        SfxItemSet rSet( m_pModel->GetFullLegendAttr() );
                        const SfxPoolItem *pPoolItem = NULL;
                        SvxChartLegendPos ePos = CHLEGEND_RIGHT;
                        if( rSet.GetItemState( SCHATTR_LEGEND_POS, TRUE, &pPoolItem ) == SFX_ITEM_SET )
                        {
                            ePos = ((const SvxChartLegendPosItem*)pPoolItem)->GetValue();
                        }
                        aAny <<= sal_Bool( ePos != CHLEGEND_NONE );
                    }
                    break;

                case CHATTR_ADDR_CATEGORIES:
                case CHATTR_ADDR_SERIES:
                case CHATTR_ADDR_CHART:
                case CHATTR_TABLE_NUMBER_LIST:
                case CHATTR_EXPORT_TABLE:
                case CHATTR_FIRST_COL_LABELS:
                case CHATTR_FIRST_ROW_LABELS:
                    {
                        SchMemChart* pData = m_pModel->GetChartData();
                        if( pData )
                        {
                            switch( nWID )
                            {
                                case CHATTR_ADDR_CATEGORIES:
                                    aAny <<= pData->GetCategoriesRangeAddress();
                                    break;
                                case CHATTR_ADDR_SERIES:
                                    aAny <<= pData->GetSeriesAddresses();
                                    break;
                                case CHATTR_ADDR_CHART:
                                    aAny <<= pData->getXMLStringForChartRange();
                                    break;
                                case CHATTR_TABLE_NUMBER_LIST:
                                    aAny <<= pData->createTableNumberList();
                                    break;
                                case CHATTR_EXPORT_TABLE:
                                    {
                                        // data table is always exported for standalone charts
                                        // if container (Calc/Writer) has set the property not
                                        // to export the data then do so
                                        DBG_ASSERT( pData, "Invalid MemChart" );
                                        const SchChartRange& rRange = pData->GetChartRange();
                                        sal_Bool bResult = rRange.mbKeepCopyOfData;

                                        // if export was set to false it might nontheless be
                                        // necessary to export data for inserting the chart into
                                        // a different container from clipboard
                                        if( !bResult && m_pDocShell )
                                            aAny <<= m_pDocShell->GetClipboardExport();

                                        aAny <<= bResult;
                                    }
                                    break;
                                case CHATTR_FIRST_COL_LABELS:
                                    {
                                        const SchChartRange& rRange = pData->GetChartRange();
                                        aAny <<= (sal_Bool)(rRange.mbFirstColumnContainsLabels);
                                    }
                                    break;
                                case CHATTR_FIRST_ROW_LABELS:
                                    {
                                        const SchChartRange& rRange = pData->GetChartRange();
                                        aAny <<= (sal_Bool)(rRange.mbFirstRowContainsLabels);
                                    }
                                    break;
                            }
                        }
                        else
                            DBG_ERROR( "invalid SchMemChart" );
                    }
                    break;

                case CHATTR_HAS_TRANSLATED_COLS:
                case CHATTR_HAS_TRANSLATED_ROWS:
                case CHATTR_TRANSLATED_COLS:
                case CHATTR_TRANSLATED_ROWS:
                    {
                        SchMemChart* pData = m_pModel->GetChartData();
                        if( pData )
                        {
                            long nTranslation = pData->GetTranslation();
                            switch( nWID )
                            {
                                case CHATTR_HAS_TRANSLATED_COLS:
                                    aAny <<= (sal_Bool)( nTranslation == TRANS_COL );
                                    break;
                                case CHATTR_HAS_TRANSLATED_ROWS:
                                    aAny <<= (sal_Bool)( nTranslation == TRANS_ROW );
                                    break;
                                case CHATTR_TRANSLATED_COLS:
                                case CHATTR_TRANSLATED_ROWS:
                                    aAny <<= GetTransSequence( pData, (CHATTR_TRANSLATED_COLS == nWID));
                                    break;
                            }
                        }
                        else
                            DBG_ERROR( "invalid SchMemChart" );
                    }
                    break;

                default:
                    {
                        SfxItemSet aSet( m_pModel->GetPool(), nWID, nWID );
                        m_pModel->GetAttr( aSet );

                        if( ! aSet.Count() )
                        {
                            // get default from ItemPool
                            if( m_pModel->GetItemPool().IsWhich( nWID ) )
                            {
                                aSet.Put( m_pModel->GetItemPool().GetDefaultItem( pMap->nWID ) );
                            }
                            else
                            {
#ifdef DBG_UTIL
                                String aTmpString( aPropertyName );
                                ByteString aProp( aTmpString, RTL_TEXTENCODING_ASCII_US );
                                DBG_ERROR2( "Diagram: Property %s has an invalid ID (%d)", aProp.GetBuffer(), nWID );
#endif
                            }
                        }
                        if( aSet.Count() )
                            aAny = m_aPropSet.getPropertyValue( pMap, aSet );
                        else
                            throw beans::UnknownPropertyException();

                        if( *pMap->pType != aAny.getValueType() )
                        {
                            // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                            if( ( *pMap->pType == ::getCppuType((const sal_Int16*)0)) && aAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
                            {
                                sal_Int32 nValue;
                                aAny >>= nValue;
                                aAny <<= static_cast< sal_Int16 >( nValue );
                            }
                            else
                            {
                                DBG_ERROR( "getPropertyValue(): wrong Type!" );
                            }
                        }
                    }
            }
        }
        else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BaseDiagram" ) ))
        {
            aAny <<= maBaseDiagramType;
        }
        else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "AddIn" ) ))
        {
            if( m_pModel )
            {
                uno::Reference< util::XRefreshable > xAddIn = m_pModel->GetChartAddIn();
                aAny <<= xAddIn;
            }
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        DBG_WARNING( "No Model" );

    return aAny;
}

void SAL_CALL ChXChartDocument::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                           const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartDocument::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                              const uno::Reference< beans::XPropertyChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartDocument::addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                           const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

void SAL_CALL ChXChartDocument::removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                                              const uno::Reference< beans::XVetoableChangeListener >& aListener )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{}

// XChartDocument

uno::Reference< drawing::XShape > SAL_CALL ChXChartDocument::getTitle() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());
    if( ! mxMainTitle.is())
    {
        mxMainTitle = new ChartTitle( m_pModel, CHOBJID_TITLE_MAIN );
        uno::Reference<lang::XComponent> xComponent (mxMainTitle, uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener (this);
    }

    return mxMainTitle;
}

uno::Reference< drawing::XShape > SAL_CALL ChXChartDocument::getSubTitle() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());
    if( ! mxSubTitle.is())
    {
        mxSubTitle = new ChartTitle( m_pModel, CHOBJID_TITLE_SUB );
        uno::Reference<lang::XComponent> xComponent (mxSubTitle, uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener (this);
    }

    return mxSubTitle;
}

uno::Reference< drawing::XShape > SAL_CALL ChXChartDocument::getLegend() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());
    if( ! mxLegend.is())
    {
        mxLegend = new ChartLegend( m_pModel );
        uno::Reference<lang::XComponent> xComponent (mxLegend, uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener (this);
    }

    return mxLegend;
}

uno::Reference< beans::XPropertySet > SAL_CALL ChXChartDocument::getArea() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());
    if( ! mxArea.is())
    {
        mxArea = new ChartArea( m_pModel, CHOBJID_DIAGRAM_AREA );
        uno::Reference<lang::XComponent> xComponent (mxArea, uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener (this);
    }

    return mxArea;
}

uno::Reference< chart::XDiagram > SAL_CALL ChXChartDocument::getDiagram() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());
    if( ! m_rXDiagram.is() )
    {
        m_rXDiagram = new ChXDiagram( m_pDocShell );
        uno::Reference<lang::XComponent> xComponent (m_rXDiagram, uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener (this);
    }

    return m_rXDiagram;
}

void SAL_CALL ChXChartDocument::setDiagram( const uno::Reference< chart::XDiagram >& _NewDiagram )
    throw( uno::RuntimeException )
{
    if( _NewDiagram == m_rXDiagram )
        return;

    osl::Guard< osl::Mutex > aGuard( GetMutex());

    uno::Reference< util::XRefreshable > xAddIn( _NewDiagram, uno::UNO_QUERY );
    if( xAddIn.is())
    {
        // use AddIn
        if( m_pModel )
        {
            SolarMutexGuard aGuard;
            m_pModel->SetChartAddIn( xAddIn );

            // initialize AddIn with this as chart document
            uno::Reference< lang::XInitialization > xInit( xAddIn, uno::UNO_QUERY );
            if( xInit.is())
            {
                uno::Any aParam;
                uno::Reference< chart::XChartDocument > xDoc( (chart::XChartDocument*)this, uno::UNO_QUERY );
                aParam <<= xDoc;
                uno::Sequence< uno::Any > aSeq( &aParam, 1 );
                xInit->initialize( aSeq );
            }

            // remember service name in member ChXDiagram instance
            uno::Reference< lang::XServiceName > xServiceName( xAddIn, uno::UNO_QUERY );
            if( xServiceName.is())
            {
                uno::Reference<lang::XComponent> xComponent (m_rXDiagram,uno::UNO_QUERY);
                if (xComponent.is())
                {
                    xComponent->removeEventListener (this);
                    xComponent->dispose();
                }

                ChXDiagram* pDiagram = new ChXDiagram( m_pDocShell, TRUE );
                pDiagram->SetServiceName( xServiceName->getServiceName() );
                m_rXDiagram = pDiagram;
                xComponent = uno::Reference<lang::XComponent>(m_rXDiagram, uno::UNO_QUERY);
                if (xComponent.is())
                    xComponent->addEventListener (this);
            }
        }
        else
        {
            DBG_ERROR( "No Model - Couldn't attach AddIn" );
        }
    }
    else
    {
        // the new diagram is only set here
        // when you have an addin the old diagram is kept as
        // 'parent diagram'

        // clear addin
        if( m_pModel )
        {
            SolarMutexGuard aGuard;
            uno::Reference< util::XRefreshable > xRefreshable;
            m_pModel->SetChartAddIn( xRefreshable );
        }

        // invalidate and release old diagram
        if( m_rXDiagram.is())
        {
            uno::Reference<lang::XComponent> xComponent (m_rXDiagram,uno::UNO_QUERY);
            if (xComponent.is())
            {
                xComponent->removeEventListener (this);
                xComponent->dispose();
            }

            ChXDiagram* pDiagram = ChXDiagram::getImplementation( m_rXDiagram );
            if( pDiagram )
                pDiagram->SetDocShell( SchChartDocShellRef() );
        }

        // set new diagram
        if( _NewDiagram.is() )
        {
            ChXDiagram* pDiagram = ChXDiagram::getImplementation( _NewDiagram );
            if( pDiagram )
            {
                if( m_pDocShell )
                {
                    if( pDiagram->SetDocShell( m_pDocShell, sal_True ))		// keep model (is copied)
                    {
                        // update local model
                        if( m_pModel )
                        {
                            SolarMutexGuard aGuard;
                            m_pModel = m_pDocShell->GetModelPtr();
                        }
                    }
                    else
                    {
                        // model was invalid
                        // so set correct chart type to current model
                        setDiagramType( pDiagram->getServiceName());
                    }
                }
            }
            uno::Reference<lang::XComponent> xComponent (m_rXDiagram, uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->removeEventListener (this);
            m_rXDiagram = _NewDiagram;
            xComponent = uno::Reference<lang::XComponent>(m_rXDiagram, uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->addEventListener (this);
        }
    }
}

uno::Reference< chart::XChartData > SAL_CALL ChXChartDocument::getData() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());
    if( ! m_xChartData.is())
    {
        uno::Reference< lang::XComponent > xComp( SAL_STATIC_CAST( SfxBaseModel*, this ));
        // XChartData is inherited twice
        m_xChartData = SAL_STATIC_CAST( chart::XChartDataArray*,
                                        new ChXChartDataArray( xComp, m_pModel ));
    }

    return m_xChartData;
}

void SAL_CALL ChXChartDocument::attachData( const uno::Reference< chart::XChartData >& _Data )
    throw( uno::RuntimeException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( GetMutex());

    if( ! _Data.is() )
        return;

    if( ! m_xEventListener.is() )
    {
        ChXChartDataChangeEventListener *pEvtListener = new ChXChartDataChangeEventListener;
        if( pEvtListener )
        {
            pEvtListener->Reset();				// Reset not implemented yet
            pEvtListener->SetOwner( this ); 	// become owner of event listener
            m_xEventListener = pEvtListener;	// remember Listener as member
        }
    }

    // add listener to XChartDataArray
    uno::Reference< chart::XChartDataArray > xChDataArray( _Data, uno::UNO_QUERY );

    if( xChDataArray.is() )
        xChDataArray->addChartDataChangeEventListener( m_xEventListener );

    // copy new Data
    m_xChartData = _Data;
    aGuard.clear();

    chart::ChartDataChangeEvent aEvent;

    aEvent.Type			 = chart::ChartDataChangeType_ALL;
    aEvent.StartColumn	 = 0;
    aEvent.EndColumn	 = 0;
    aEvent.StartRow		 = 0;
    aEvent.EndRow		 = 0;

    RefreshData( aEvent ); // set initial data
}


// XModel ( ::SfxBaseModel )
sal_Bool SAL_CALL ChXChartDocument::attachResource( const ::rtl::OUString& aURL,
                                                    const uno::Sequence< beans::PropertyValue >& aArgs )
    throw( uno::RuntimeException )
{
    return SfxBaseModel::attachResource( aURL, aArgs );
}

::rtl::OUString SAL_CALL ChXChartDocument::getURL() throw( uno::RuntimeException )
{
    return SfxBaseModel::getURL();
}

uno::Sequence< beans::PropertyValue > SAL_CALL ChXChartDocument::getArgs() throw( uno::RuntimeException )
{
    return SfxBaseModel::getArgs();
}

void SAL_CALL ChXChartDocument::connectController( const uno::Reference< frame::XController >& xController )
    throw( uno::RuntimeException )
{
    SfxBaseModel::connectController( xController );
}

void SAL_CALL ChXChartDocument::disconnectController( const uno::Reference< frame::XController >& xController )
    throw( uno::RuntimeException )
{
    SfxBaseModel::disconnectController( xController );
}

uno::Reference< frame::XController > SAL_CALL ChXChartDocument::getCurrentController() throw( uno::RuntimeException )
{
    return SfxBaseModel::getCurrentController();
}

void SAL_CALL ChXChartDocument::setCurrentController( const uno::Reference< frame::XController >& xController )
    throw( container::NoSuchElementException, uno::RuntimeException )
{
    SfxBaseModel::setCurrentController( xController );
}

uno::Reference< uno::XInterface > SAL_CALL ChXChartDocument::getCurrentSelection() throw( uno::RuntimeException )
{
    return SfxBaseModel::getCurrentSelection();
}

void SAL_CALL ChXChartDocument::lockControllers() throw( uno::RuntimeException )
{
    if( m_pModel )
    {
        SolarMutexGuard aGuard;
        m_pModel->LockBuild();
    }
    else
         throw uno::RuntimeException();
}

void SAL_CALL ChXChartDocument::unlockControllers() throw( uno::RuntimeException )
{
    if( m_pModel )
    {
        SolarMutexGuard aGuard;
        m_pModel->UnlockBuild();
    }
    else
         throw uno::RuntimeException();
}

sal_Bool SAL_CALL ChXChartDocument::hasControllersLocked() throw( uno::RuntimeException )
{
    if( m_pModel )
    {
        SolarMutexGuard aGuard;
        return m_pModel->IsLockedBuild();
    }
    else
        return sal_False;
}

// XTypeProvider ( ::SfxBaseModel )
uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ChXChartDocument::getTypes()
    throw( uno::RuntimeException )
{
    if( maTypeSequence.getLength() == 0 )
    {
        osl::Guard< osl::Mutex > aGuard( GetMutex());

        const uno::Sequence< uno::Type > aBaseTypes( SfxBaseModel::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        maTypeSequence.realloc( nBaseTypes + 7 );		// Note: Keep the size updated !!
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< lang::XMultiServiceFactory >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< chart::XChartDocument >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< util::XNumberFormatsSupplier >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XDrawPageSupplier >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL ChXChartDocument::getImplementationId()
    throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}




// XComponent ( ::XModel ::SfxModel )
void SAL_CALL ChXChartDocument::dispose() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());

    uno::Reference<lang::XComponent> xComponent;
    if (mxMainTitle.is())
    {
           xComponent = uno::Reference<lang::XComponent>(mxMainTitle,uno::UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->removeEventListener (this);
            xComponent->dispose ();
            mxMainTitle = NULL;
        }
    }
    if (mxSubTitle.is())
    {
           xComponent = uno::Reference<lang::XComponent>(mxSubTitle,uno::UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->removeEventListener (this);
            xComponent->dispose ();
            mxSubTitle = NULL;
        }
    }
    if (mxLegend.is())
    {
           xComponent = uno::Reference<lang::XComponent>(mxLegend,uno::UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->removeEventListener (this);
            xComponent->dispose ();
            mxLegend = NULL;
        }
    }
    if (mxArea.is())
    {
           xComponent = uno::Reference<lang::XComponent>(mxArea,uno::UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->removeEventListener (this);
            xComponent->dispose ();
            mxArea = NULL;
        }
    }

    if( m_rXDiagram.is())
    {
        ChXDiagram* pDiagram = ChXDiagram::getImplementation( m_rXDiagram );
        if( pDiagram )
            pDiagram->SetDocShell( NULL );
        xComponent = uno::Reference<lang::XComponent>(m_rXDiagram,uno::UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->removeEventListener (this);
            xComponent->dispose();
            m_rXDiagram = NULL;
        }
    }
    m_pModel = NULL;

    SfxBaseModel::dispose();
}




void SAL_CALL ChXChartDocument::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw( uno::RuntimeException )
{
    SfxBaseModel::addEventListener( xListener );
}

void SAL_CALL ChXChartDocument::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw( uno::RuntimeException )
{
    SfxBaseModel::removeEventListener( aListener );
}

// XNumberFormatsSupplier
void ChXChartDocument::InitNumberFormatter() throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());
    if( ! mrNumberFormatter.is())
    {
        if( m_pModel )
        {
            SolarMutexGuard aGuard;
            mrNumberFormatter = new SvNumberFormatsSupplierObj( m_pModel->GetNumFormatter() );
        }
        else
            mrNumberFormatter = new SvNumberFormatsSupplierObj();
    }

    if( ! mrNumberFormatter.is())
         throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySet > SAL_CALL ChXChartDocument::getNumberFormatSettings() throw( uno::RuntimeException )
{
    if( ! mrNumberFormatter.is())
        InitNumberFormatter();

    return mrNumberFormatter.get()->getNumberFormatSettings();
}

uno::Reference< util::XNumberFormats > SAL_CALL ChXChartDocument::getNumberFormats() throw( uno::RuntimeException )
{
    if( ! mrNumberFormatter.is())
        InitNumberFormatter();

    return mrNumberFormatter.get()->getNumberFormats();
}

// XServiceInfo
::rtl::OUString SAL_CALL ChXChartDocument::getImplementationName()
    throw( uno::RuntimeException )
{
    return SCH_ASCII_TO_OU( "ChXChartDocument" );
}

sal_Bool SAL_CALL ChXChartDocument::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChXChartDocument::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq( 4 );
    ::rtl::OUString* pStr = aSeq.getArray();
    pStr[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.OfficeDocument" ));
    pStr[ 1 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.ChartDocument" ));
    pStr[ 2 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.ChartTableAddressSupplier" ));
    pStr[ 3 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.UserDefinedAttributeSupplier" ));

    return aSeq;
}

// XDrawPageSupplier
uno::Reference< drawing::XDrawPage > SAL_CALL ChXChartDocument::getDrawPage()
    throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( GetMutex());

    if( ! mrDrawPage.is() && m_pModel )
    {
        mrDrawPage = new ChXChartDrawPage( m_pModel );
        uno::WeakReference< uno::XInterface > xPage( mrDrawPage );
        // set page wrapper (member can be used as this class is declared as friend)
        m_pModel->GetPage( 0 )->mxUnoPage = xPage;
    }

    return mrDrawPage;
}

// XUnoTunnel
sal_Int64 SAL_CALL ChXChartDocument::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( uno::RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                                 aIdentifier.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }

    // in XML the implementation of the XNumberFormatsSupplier (SvNumberFormatsSupplierObj) is queried
    // so return this member here

    if( ! mrNumberFormatter.is())
        InitNumberFormatter();

    uno::Reference< lang::XUnoTunnel > xUnoTunnel( mrNumberFormatter, uno::UNO_QUERY );
    if( xUnoTunnel.is())
        return xUnoTunnel->getSomething( aIdentifier );

    return 0;
}



//	XEeventListener
//virtual
void SAL_CALL ChXChartDocument::disposing (const lang::EventObject & Source)
    throw (uno::RuntimeException)
{
    sal_Bool bChangeProperty = sal_False;
    ::rtl::OUString aProperty;

    if (Source.Source == m_rXDiagram)
        m_rXDiagram = NULL;
    else if (Source.Source == mxMainTitle)
    {
        mxMainTitle = NULL;
        aProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UNONAME_TITLE_SHOW_MAIN ));
        bChangeProperty = sal_True;
    }
    else if (Source.Source == mxSubTitle)
    {
        mxSubTitle = NULL;
        aProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UNONAME_TITLE_SHOW_SUB ));
        bChangeProperty = sal_True;
    }
    else if (Source.Source == mxLegend)
    {
        mxLegend = NULL;
        aProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UNONAME_HASLEGEND ));
        bChangeProperty = sal_True;
    }
    else if (Source.Source == mxArea)
    {
        mxArea = NULL;
    }

    if ( ! bChangeProperty)
        return;

    //	Inform the document shell of the disposing of Source.Source.
    SolarMutexGuard aGuard;

    SchChartDocShell* pDocSh = (SchChartDocShell*)GetObjectShell();
    if (pDocSh == NULL)
        return;
    uno::Reference< beans::XPropertySet > xDocProp( pDocSh->GetBaseModel(), uno::UNO_QUERY );
    if( xDocProp.is())
    {
        uno::Any aFalseBool;
        aFalseBool <<= (sal_Bool)(sal_False);
        try
        {
            xDocProp->setPropertyValue( aProperty, aFalseBool );
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_WARNING( "cannot set property to false" );
        }
    }

    SfxBaseModel::disposing( Source );
}

// get additional non-chart shapes for XML export
uno::Reference< drawing::XShapes > ChXChartDocument::GetAdditionalShapes()
{
    uno::Reference< drawing::XShapes > xFoundShapes;

    if( m_pModel == NULL )
        return xFoundShapes;

    // iterate 'flat' over all top-level objects
    // and determine all that are no chart objects
    // i.e. which have no SchObjectId set as user data
    ::std::vector< uno::Reference< drawing::XShape > > aShapeVector;
    SdrObjListIter aIter( *(m_pModel->GetPage( 0 )), IM_FLAT );
    SdrObject* pObj;
    while( aIter.IsMore())
    {
        pObj = aIter.Next();

        if( GetObjectId( *pObj ) != NULL )
            continue;

        // we have no chart user data, i.e. a non-chart object

        // check if chart is inventor.  This has to be changed for XML exportability
        if( pObj->GetObjInventor() == SchInventor &&
            pObj->ISA( SchObjGroup ))
        {
            static_cast< SchObjGroup* >( pObj )->SetUseChartInventor( false );
        }

        uno::Reference< drawing::XShape > xShape( pObj->getUnoShape(), uno::UNO_QUERY );
        if( xShape.is() )
            aShapeVector.push_back( xShape );
    }
    if( ! aShapeVector.empty())
    {
        // create a shape collection
        xFoundShapes = uno::Reference< drawing::XShapes >( SvxShapeCollection_NewInstance(), uno::UNO_QUERY );

        DBG_ASSERT( xFoundShapes.is(), "Couldn't create a shape collection!" );
        if( xFoundShapes.is())
        {
            ::std::vector< uno::Reference< drawing::XShape > >::iterator aIter;
            for( aIter = aShapeVector.begin(); aIter != aShapeVector.end(); ++aIter )
                xFoundShapes->add( *aIter );
        }
//          }
    }

    return xFoundShapes;
}

uno::Sequence< sal_Int32 > ChXChartDocument::GetTransSequence( SchMemChart* pData, bool bColumns )
{
    uno::Sequence< sal_Int32 > aResult;

    if( pData )
    {
        long nTranslation = pData->GetTranslation();

        if( bColumns )
        {
            short nColCount = pData->GetColCount();
            aResult.realloc( nColCount );
            sal_Int32* pSeq = aResult.getArray();
            const sal_Int32* pTransArray = pData->GetColTranslation();
            sal_Int32 i;

            if( nTranslation == TRANS_COL &&
                pTransArray != NULL )
            {
                for( i = 0; i < nColCount; ++i )
                    pSeq[ i ] = pTransArray[ i ];
            }
            else
            {
                // identity sequence
                for( i = 0; i < nColCount; ++i )
                    pSeq[ i ] = i;
            }
        }
        else // rows
        {
            short nRowCount = pData->GetRowCount();
            aResult.realloc( nRowCount );
            sal_Int32* pSeq = aResult.getArray();
            const sal_Int32* pTransArray = pData->GetRowTranslation();
            sal_Int32 i;

            if( nTranslation == TRANS_ROW &&
                pTransArray != NULL )
            {
                for( i = 0; i < nRowCount; ++i )
                    pSeq[ i ] = pTransArray[ i ];
            }
            else
            {
                // identity sequence
                for( i = 0; i < nRowCount; ++i )
                    pSeq[ i ] = i;
            }
        }
    }
    else
    {
        DBG_ERROR( "Invalid MemChart" );
    }

    return aResult;
}

bool ChXChartDocument::SetTransSequence( SchMemChart* pData, bool bColumns, const uno::Sequence< sal_Int32 >& rSeq )
{
    bool bResult = false;

    if( pData )
    {
        long nTranslation = pData->GetTranslation();

        if( bColumns )
        {
            if( nTranslation != TRANS_ROW &&
                pData->GetColCount() == rSeq.getLength() )
            {
                pData->SetTranslation( TRANS_COL );
                pData->SetColTranslation( rSeq.getConstArray());
                bResult = true;
            }
        }
        else // rows
        {
            if( nTranslation != TRANS_COL &&
                pData->GetRowCount() == rSeq.getLength() )
            {
                pData->SetTranslation( TRANS_ROW );
                pData->SetRowTranslation( rSeq.getConstArray());
                bResult = true;
            }
        }

    }
    else
    {
        DBG_ERROR( "Invalid MemChart" );
    }

    return bResult;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

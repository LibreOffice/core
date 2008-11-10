/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chart2uno.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "chart2uno.hxx"
#include "miscuno.hxx"
#include "document.hxx"
#include "unoguard.hxx"
#include "cell.hxx"
#include "chartpos.hxx"
#include "unonames.hxx"
#include "globstr.hrc"
#include "convuno.hxx"
#include "rangeutl.hxx"
#include "hints.hxx"
#include "unoreflist.hxx"

#include <sfx2/objsh.hxx>

#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HDL_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_DATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/text/XText.hpp>
#include <comphelper/extract.hxx>

#include <vector>
#include <list>
#include <rtl/math.hxx>

SC_SIMPLE_SERVICE_INFO( ScChart2DataProvider, "ScChart2DataProvider",
        "com.sun.star.chart2.data.DataProvider")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSource, "ScChart2DataSource",
        "com.sun.star.chart2.data.DataSource")
SC_SIMPLE_SERVICE_INFO( ScChart2LabeledDataSequence, "ScChart2LabeledDataSequence",
        "com.sun.star.chart2.data.LabeledDataSequence")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSequence, "ScChart2DataSequence",
        "com.sun.star.chart2.data.DataSequence")
SC_SIMPLE_SERVICE_INFO( ScChart2EmptyDataSequence, "ScChart2EmptyDataSequence",
        "com.sun.star.chart2.data.DataSequence")

using namespace ::com::sun::star;

namespace
{
const SfxItemPropertyMap* lcl_GetDataSequencePropertyMap()
{
    static SfxItemPropertyMap aDataSequencePropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ISHIDDEN), 0,      &getBooleanCppuType(),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROLE), 0, &getCppuType((::com::sun::star::chart2::data::DataSequenceRole*)0),                  0, 0 },
        {0,0,0,0,0,0}
    };
    return aDataSequencePropertyMap_Impl;
}

template< typename T >
::com::sun::star::uno::Sequence< T > lcl_VectorToSequence(
    const ::std::vector< T > & rCont )
{
    ::com::sun::star::uno::Sequence< T > aResult( rCont.size());
    ::std::copy( rCont.begin(), rCont.end(), aResult.getArray());
    return aResult;
}

::std::vector< ::rtl::OUString > lcl_getRangeRepresentationsFromDataSource(
    const uno::Reference< chart2::data::XDataSource > & xDataSource )
{
    ::std::vector< ::rtl::OUString > aResult;
    if( xDataSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(
            xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
        {
            if( aSequences[nIdx].is() )
            {
                // first: label
                uno::Reference< chart2::data::XDataSequence > xSeq( aSequences[nIdx]->getLabel());
                if( xSeq.is())
                    aResult.push_back( xSeq->getSourceRangeRepresentation());
                // then: values
                xSeq.set( aSequences[nIdx]->getValues());
                if( xSeq.is())
                    aResult.push_back( xSeq->getSourceRangeRepresentation());
            }
        }
    }
    return aResult;
}

uno::Reference< chart2::data::XLabeledDataSequence > lcl_getCategoriesFromDataSource(
    const uno::Reference< chart2::data::XDataSource > & xDataSource )
{
    uno::Reference< chart2::data::XLabeledDataSequence > xResult;
    if( xDataSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(
            xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
        {
            if( aSequences[nIdx].is() )
            {
                uno::Reference< beans::XPropertySet > xSeqProp( aSequences[nIdx]->getValues(), uno::UNO_QUERY );
                ::rtl::OUString aRole;
                if( xSeqProp.is() &&
                    (xSeqProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Role"))) >>= aRole) &&
                    aRole.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("categories")) )
                {
                    xResult.set( aSequences[nIdx] );
                    break;
                }
            }
        }
    }
    return xResult;
}

struct lcl_appendTableNumber : public ::std::unary_function< SCTAB, void >
{
    lcl_appendTableNumber( ::rtl::OUStringBuffer & rBuffer ) :
            m_rBuffer( rBuffer )
    {}
    void operator() ( SCTAB nTab )
    {
        // there is no append with SCTAB or sal_Int16
        m_rBuffer.append( static_cast< sal_Int32 >( nTab ));
        m_rBuffer.append( sal_Unicode( ' ' ));
    }
private:
    ::rtl::OUStringBuffer & m_rBuffer;
};

::rtl::OUString lcl_createTableNumberList( const ::std::list< SCTAB > & rTableList )
{
    ::rtl::OUStringBuffer aBuffer;
    ::std::for_each( rTableList.begin(), rTableList.end(), lcl_appendTableNumber( aBuffer ));
    // remove last trailing ' '
    if( aBuffer.getLength() > 0 )
        aBuffer.setLength( aBuffer.getLength() - 1 );
    return aBuffer.makeStringAndClear();
}

uno::Reference< frame::XModel > lcl_GetXModel( ScDocument * pDoc )
{
    uno::Reference< frame::XModel > xModel;
    SfxObjectShell * pObjSh( pDoc ? pDoc->GetDocumentShell() : 0 );
    if( pObjSh )
        xModel.set( pObjSh->GetModel());
    return xModel;
}

uno::Reference< sheet::XSpreadsheetDocument > lcl_GetSpreadSheetDocument( ScDocument * pDoc )
{
    return uno::Reference< sheet::XSpreadsheetDocument >( lcl_GetXModel( pDoc ), uno::UNO_QUERY );
}

} // anonymous namespace

// DataProvider ==============================================================

ScChart2DataProvider::ScChart2DataProvider( ScDocument* pDoc )
    : m_pDocument( pDoc)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}

ScChart2DataProvider::~ScChart2DataProvider()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}


void ScChart2DataProvider::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}

void lcl_SeperateOneColumnRange(ScRange aR, const ScAddress& rPos, ScRangeListRef& xRanges)
{
    if (aR.aStart == rPos)
    {
        aR.aStart.SetRow(aR.aStart.Row() + 1);
        xRanges->Join(aR);
    }
    else if (aR.aEnd == rPos)
    {
        aR.aStart.SetRow(aR.aStart.Row() - 1);
        xRanges->Join(aR);
    }
    else
    {
        xRanges->Join(ScRange(aR.aStart, ScAddress(rPos.Col(), rPos.Row() - 1, rPos.Tab())));
        xRanges->Join(ScRange(ScAddress(rPos.Col(), rPos.Row() + 1, rPos.Tab()), aR.aEnd ));
    }
}

void lcl_SeperateOneRowRange(ScRange aR, const ScAddress& rPos, ScRangeListRef& xRanges)
{
    if (aR.aStart == rPos)
    {
        aR.aStart.SetCol(aR.aStart.Col() + 1);
        xRanges->Join(aR);
    }
    else if (aR.aEnd == rPos)
    {
        aR.aStart.SetCol(aR.aStart.Col() - 1);
        xRanges->Join(aR);
    }
    else
    {
        xRanges->Join(ScRange(aR.aStart, ScAddress(rPos.Col() - 1, rPos.Row(), rPos.Tab())));
        xRanges->Join(ScRange(ScAddress(rPos.Col() + 1, rPos.Row(), rPos.Tab()), aR.aEnd ));
    }
}

::sal_Bool SAL_CALL ScChart2DataProvider::createDataSourcePossible( const uno::Sequence< beans::PropertyValue >& aArguments )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if( ! m_pDocument )
        return false;

    rtl::OUString aRangeRepresentation;
    for(sal_Int32 i = 0; i < aArguments.getLength(); ++i)
    {
        rtl::OUString sName(aArguments[i].Name);
        if (aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CellRangeRepresentation")))
        {
            aArguments[i].Value >>= aRangeRepresentation;
        }
    }

    ScRangeList aRangeList;
    USHORT nResult = aRangeList.Parse( aRangeRepresentation, m_pDocument );
    //! if anything is missing, SCA_VALID shouldn't be set
    const USHORT nNeeded = SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                           SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2;
    bool bValid = ( (nResult & nNeeded ) == nNeeded );

    return bValid;
}

uno::Reference< chart2::data::XDataSource> SAL_CALL
ScChart2DataProvider::createDataSource(
    const uno::Sequence< beans::PropertyValue >& aArguments )
    throw( lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( ! m_pDocument )
        throw uno::RuntimeException();

    uno::Reference< chart2::data::XDataSource> xResult;
    sal_Bool bLabel = sal_True;
//        sal_Bool bCat = sal_True;
    sal_Bool bOrientCol = sal_True;
    ::rtl::OUString aRangeRepresentation;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    for(sal_Int32 i = 0; i < aArguments.getLength(); ++i)
    {
        rtl::OUString sName(aArguments[i].Name);
        if (aArguments[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataRowSource")))
        {
            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            if( ! (aArguments[i].Value >>= eSource))
            {
                sal_Int32 nSource(0);
                if( aArguments[i].Value >>= nSource )
                    eSource = (static_cast< chart::ChartDataRowSource >( nSource ));
            }
            bOrientCol = (eSource == chart::ChartDataRowSource_COLUMNS);
        }
        else if (aArguments[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FirstCellAsLabel")))
        {
            bLabel = ::cppu::any2bool(aArguments[i].Value);
        }
        else if (aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CellRangeRepresentation")))
        {
            aArguments[i].Value >>= aRangeRepresentation;
        }
        else if (aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SequenceMapping")))
        {
            aArguments[i].Value >>= aSequenceMapping;
        }
/*            else if (aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("HasCategories")))
              {
              bCat = ::cppu::any2bool(aArguments[i].Value);
              }*/
    }

    ScRangeListRef aRangeList = new ScRangeList;
    USHORT nValid = aRangeList->Parse( aRangeRepresentation, m_pDocument);
    if ( (nValid & SCA_VALID) == SCA_VALID )
    {
        if(bLabel)
            addUpperLeftCornerIfMissing( aRangeList );

        ScChartPositioner aChartPositioner(m_pDocument, aRangeList);
        BOOL bColHeaders = (bOrientCol ? bLabel : FALSE );
        BOOL bRowHeaders = (bOrientCol ? FALSE : bLabel );
        aChartPositioner.SetHeaders( bColHeaders, bRowHeaders );

        const ScChartPositionMap* pChartMap = aChartPositioner.GetPositionMap();

        ScChart2DataSource* pDS = NULL;

        if (pChartMap)
        {
            std::list < ScChart2LabeledDataSequence* > aSeqs;

            // Fill Categories

            ScChart2LabeledDataSequence* pHeader = NULL;
            if (bOrientCol ? aChartPositioner.HasRowHeaders() : aChartPositioner.HasColHeaders())
            {
                pHeader = new ScChart2LabeledDataSequence(m_pDocument);
                sal_Int32 nCount = static_cast< sal_Int32 >( bOrientCol ? pChartMap->GetRowCount() : pChartMap->GetColCount() );
                ScRangeList* pRanges = new ScRangeList;
                ScAddress aLabel;
                sal_Bool bLabelCell = sal_False;
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    const ScAddress* pPos = bOrientCol ?
                        pChartMap->GetRowHeaderPosition( static_cast< SCROW >( i ) ) :
                        pChartMap->GetColHeaderPosition( static_cast< SCCOL >( i ) );
                    if (pPos)
                    {
                        pRanges->Join(ScRange(*pPos));
                        if (!bLabelCell)
                        {
                            aLabel = *pPos;
                            if (bOrientCol)
                                aLabel.IncRow( -1 );
                            else
                                aLabel.IncCol( -1 );
                            bLabelCell = sal_True;
                        }
                    }
                }
                if (bLabelCell && bLabel)
                {
                    ScRangeList* pLabelRanges = new ScRangeList;
                    pLabelRanges->Join(ScRange(aLabel));
                    uno::Reference < chart2::data::XDataSequence > xLabelSeq(new ScChart2DataSequence(m_pDocument, this, pLabelRanges));
                    uno::Reference< beans::XPropertySet > xLabelProps(xLabelSeq, uno::UNO_QUERY);
                    if (xLabelProps.is())
                        xLabelProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("label"))));
                    pHeader->setLabel(xLabelSeq);
                }
                else if (bLabelCell)
                {
                    pRanges->Join(ScRange(aLabel));
/*                        uno::Reference < chart2::data::XDataSequence > xLabelSeq(new ScChart2EmptyDataSequence(m_pDocument, this, new ScRangeList(*pRanges), bOrientCol));
                          uno::Reference< beans::XPropertySet > xLabelProps(xLabelSeq, uno::UNO_QUERY);
                          if (xLabelProps.is())
                          xLabelProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("label"))));
                          pHeader->setLabel(xLabelSeq);*/
                }
                uno::Reference< chart2::data::XDataSequence > xSeq( new ScChart2DataSequence( m_pDocument, this, pRanges ) );
/*                    if (bCat)
                      {
                      uno::Reference< beans::XPropertySet > xProps(xSeq, uno::UNO_QUERY);
                      if (xProps.is())
                      xProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("categories"))));
                      }*/
                pHeader->setValues(xSeq);
            }
            if (pHeader)
                aSeqs.push_back(pHeader);

            ScRangeListRef xCatRanges = new ScRangeList();

            // Fill Serieses with Labels

            sal_Int32 nCount = bOrientCol ? pChartMap->GetColCount() : pChartMap->GetRowCount();
            for (sal_Int32 i = 0; i < nCount; ++i)
            {
                ScChart2LabeledDataSequence* pLabeled = new ScChart2LabeledDataSequence(m_pDocument);
                uno::Reference < chart2::data::XDataSequence > xLabelSeq;
                ScRangeListRef xRanges( bOrientCol ?
                                        pChartMap->GetColRanges( static_cast< SCCOL >( i ) ) :
                                        pChartMap->GetRowRanges( static_cast< SCROW >( i ) ) );
                const ScAddress* pPos = bOrientCol ?
                    pChartMap->GetColHeaderPosition( static_cast< SCCOL >( i ) ) :
                    pChartMap->GetRowHeaderPosition( static_cast< SCROW >( i ) );

                if (bLabel/* && (bOrientCol ? aChartPositioner.HasColHeaders() : aChartPositioner.HasRowHeaders())*/)
                {
                    if (!pPos)
                    {
                        ScRangePtr pR = xRanges->First();
                        if (pR)
                        {
                            pR->Justify();
                            pPos = &(pR->aStart);
                        }
                    }
                    if (pPos)
                    {
                        ScRangeList* pRanges = new ScRangeList;
                        ScRange aPosRange(*pPos);
                        pRanges->Join(aPosRange);
                        xLabelSeq.set(new ScChart2DataSequence(m_pDocument, this, pRanges));
                        uno::Reference< beans::XPropertySet > xLabelProps(xLabelSeq, uno::UNO_QUERY);
                        if (xLabelProps.is())
                            xLabelProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("label"))));

                        // remove Header from Ranges
                        ScRangePtr pR;
                        ScRangeListRef xRangeList = new ScRangeList;
                        for ( pR = xRanges->First(); pR; pR = xRanges->Next() )
                        {
                            if (pR->In(*pPos))
                            {
                                pR->Justify();
                                if (pR->aStart != pR->aEnd)
                                {
                                    DBG_ASSERT(pR->aStart.Tab() == pR->aEnd.Tab(), "a 3D range should not happen in this case");
                                        if(bOrientCol)
                                        {
                                            if (pR->aStart.Col() == pR->aEnd.Col())
                                            {
                                                lcl_SeperateOneColumnRange(*pR, *pPos, xRangeList);
                                            }
                                            else
                                            {
                                                if (pR->aStart.Col() == pPos->Col())
                                                {
                                                    lcl_SeperateOneColumnRange(ScRange(pR->aStart, ScAddress(pR->aStart.Col(), pR->aEnd.Row(), pR->aStart.Tab())), *pPos, xRangeList);
                                                    xRangeList->Join(ScRange(ScAddress(pR->aStart.Col() + 1, pR->aStart.Row(), pR->aStart.Tab()), pR->aEnd));
                                                }
                                                else if (pR->aEnd.Col() == pPos->Col())
                                                {
                                                    lcl_SeperateOneColumnRange(ScRange(ScAddress(pR->aEnd.Col(), pR->aStart.Row(), pR->aStart.Tab()), pR->aEnd), *pPos, xRangeList);
                                                    xRangeList->Join(ScRange(pR->aStart, ScAddress(pR->aEnd.Col() - 1, pR->aEnd.Row(), pR->aEnd.Tab())));
                                                }
                                                else
                                                {
                                                    xRangeList->Join(ScRange(pR->aStart, ScAddress(pPos->Col() - 1, pR->aEnd.Row(), pPos->Tab())));
                                                    lcl_SeperateOneColumnRange(ScRange(ScAddress(pPos->Col(), pR->aStart.Row(), pR->aStart.Tab()), ScAddress(pPos->Col(), pR->aEnd.Row(), pR->aEnd.Tab())), *pPos, xRangeList);
                                                    xRangeList->Join(ScRange(ScAddress(pPos->Col() + 1, pR->aStart.Row(), pPos->Tab()), pR->aEnd));
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (pR->aStart.Row() == pR->aEnd.Row())
                                            {
                                                lcl_SeperateOneRowRange(*pR, *pPos, xRangeList);
                                            }
                                            else
                                            {
                                                if (pR->aStart.Row() == pPos->Row())
                                                {
                                                    lcl_SeperateOneRowRange(ScRange(pR->aStart, ScAddress(pR->aEnd.Col(), pR->aStart.Row(), pR->aStart.Tab())), *pPos, xRangeList);
                                                    xRangeList->Join(ScRange(ScAddress(pR->aStart.Col(), pR->aStart.Row() + 1, pR->aStart.Tab()), pR->aEnd));
                                                }
                                                else if (pR->aEnd.Row() == pPos->Row())
                                                {
                                                    lcl_SeperateOneRowRange(ScRange(ScAddress(pR->aStart.Col(), pR->aEnd.Row(), pR->aStart.Tab()), pR->aEnd), *pPos, xRangeList);
                                                    xRangeList->Join(ScRange(pR->aStart, ScAddress(pR->aEnd.Col(), pR->aEnd.Row() - 1, pR->aEnd.Tab())));
                                                }
                                                else
                                                {
                                                    xRangeList->Join(ScRange(pR->aStart, ScAddress(pR->aEnd.Col(), pPos->Row() - 1, pPos->Tab())));
                                                    lcl_SeperateOneRowRange(ScRange(ScAddress(pR->aStart.Col(), pPos->Row(), pR->aStart.Tab()), ScAddress(pR->aEnd.Col(), pPos->Row(), pR->aEnd.Tab())), *pPos, xRangeList);
                                                    xRangeList->Join(ScRange(ScAddress(pR->aStart.Col(), pPos->Row() + 1, pPos->Tab()), pR->aEnd));
                                                }
                                            }
                                        }
                                }
                            }
                            else
                                xRangeList->Join(*pR);
                        }
                        xRanges = xRangeList;
                    }
                }
                else
                {
                    if (pPos)
                        xRanges->Join(ScRange(*pPos));
/*                        xLabelSeq.set(new ScChart2EmptyDataSequence(m_pDocument, this, xRanges, bOrientCol));
                          uno::Reference< beans::XPropertySet > xLabelProps(xLabelSeq, uno::UNO_QUERY);
                          if (xLabelProps.is())
                          xLabelProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("label"))));*/
                }

                // FIXME: if there are no labels the column or row name should be taken
/*                    else if (bLabel)
                      {
                      if (bOrientCol)
                      {
                      String aString = ScGlobal::GetRscString(STR_COLUMN);
                      aString += ' ';
                      //                aString += String::CreateFromInt32( pCols[nCol]+1 );
                      ScAddress aPos( nCol, 0, 0 );
                      String aColStr;
                      aPos.Format( aColStr, SCA_VALID_COL, NULL );
                      aString += aColStr;
                      }
                      else
                      {
                      String aString = ScGlobal::GetRscString(STR_ROW);
                      aString += ' ';
                      aString += String::CreateFromInt32( nRow+1 );
                      }
                      }*/

/*                  ScRangePtr pR;
                    for ( pR = xRanges->First(); pR; pR = xRanges->Next() )
                    xCatRanges->Join(*pR);*/

                uno::Reference < chart2::data::XDataSequence > xSeq(new ScChart2DataSequence(m_pDocument, this, new ScRangeList(*xRanges)));
/*                    if (bCat && !pHeader && (i == 0))
                      {
                      uno::Reference< beans::XPropertySet > xProps(xSeq, uno::UNO_QUERY);
                      if (xProps.is())
                      xProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("categories"))));
                      }*/
                pLabeled->setValues(xSeq);
                pLabeled->setLabel(xLabelSeq);

                aSeqs.push_back(pLabeled);
            }

/*                if (!bCat)
                  {
                  ScChart2LabeledDataSequence* pCat = new ScChart2LabeledDataSequence(m_pDocument);

                  uno::Reference< chart2::data::XDataSequence > xSeq( new ScChart2EmptyDataSequence( m_pDocument, this, new ScRangeList(*xCatRanges), !bOrientCol ) );
                  uno::Reference< beans::XPropertySet > xProps(xSeq, uno::UNO_QUERY);
                  if (xProps.is())
                  xProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("categories"))));
                  pCat->setValues(xSeq);

                  aSeqs.push_front(pCat);
                  }*/

            pDS = new ScChart2DataSource(m_pDocument);
            std::list < ScChart2LabeledDataSequence* >::iterator aItr(aSeqs.begin());
            std::list < ScChart2LabeledDataSequence* >::iterator aEndItr(aSeqs.end());

            //reorder labeled sequences according to aSequenceMapping
            std::vector< ScChart2LabeledDataSequence* >  aSeqVector;
            while(aItr != aEndItr)
            {
                aSeqVector.push_back(*aItr);
                ++aItr;
            }

            std::map< sal_Int32, ScChart2LabeledDataSequence* > aSequenceMap;
            for( sal_Int32 nNewIndex = 0; nNewIndex < aSequenceMapping.getLength(); nNewIndex++ )
            {
                // note: assuming that the values in the sequence mapping are always non-negative
                std::vector< ScChart2LabeledDataSequence* >::size_type nOldIndex( static_cast< sal_uInt32 >( aSequenceMapping[nNewIndex] ));
                if( nOldIndex < aSeqVector.size() )
                {
                    pDS->AddLabeledSequence( aSeqVector[nOldIndex] );
                    aSeqVector[nOldIndex] = 0;
                }

            }

            std::vector< ScChart2LabeledDataSequence* >::iterator aVectorItr(aSeqVector.begin());
            std::vector< ScChart2LabeledDataSequence* >::iterator aVectorEndItr(aSeqVector.end());
            while(aVectorItr != aVectorEndItr)
            {
                if(*aVectorItr)
                    pDS->AddLabeledSequence(*aVectorItr);
                ++aVectorItr;
            }
        }

        xResult.set( pDS );
    }
    else
    {
        throw lang::IllegalArgumentException();
    }

    return xResult;
}

namespace
{
void lcl_detectRanges(
    ScRangeListRef & rOutRanges,
    chart::ChartDataRowSource & rOutRowSource,
    bool & rOutRowSourceDetected,
    const uno::Reference< chart2::data::XDataSource >& xDataSource,
    ScDocument * pDoc )
{
    if(!pDoc)
        return;

    ScUnoGuard aGuard;

    sal_Int32 nDataInRows = 0;
    sal_Int32 nDataInColumns = 0;
    bool bRowSourceAmbiguous = false;

    ::std::vector< ::rtl::OUString > aRangeRepresentations(
        lcl_getRangeRepresentationsFromDataSource( xDataSource ));
    for (::std::vector< ::rtl::OUString >::const_iterator aIt( aRangeRepresentations.begin());
         aIt != aRangeRepresentations.end(); ++aIt)
    {
        ScRangePtr pR;
        ScRangeListRef aRangeList = new ScRangeList;
        aRangeList->Parse( (*aIt), pDoc);
        for ( pR = aRangeList->First(); pR; pR = aRangeList->Next() )
        {
            bRowSourceAmbiguous = bRowSourceAmbiguous || (pR->aStart.Tab() != pR->aEnd.Tab());
            if( ! bRowSourceAmbiguous )
            {
                bool bColDiff( ( pR->aEnd.Col() - pR->aStart.Col() ) != 0 );
                bool bRowDiff( ( pR->aEnd.Row() - pR->aStart.Row() ) != 0 );

                if( bColDiff && ! bRowDiff )
                    ++nDataInRows;
                else if( bRowDiff && ! bColDiff )
                    ++nDataInColumns;
                else if( bRowDiff && bColDiff )
                    bRowSourceAmbiguous = true;

                if( nDataInRows > 0 && nDataInColumns > 0 )
                    bRowSourceAmbiguous = true;
            }
            rOutRanges->Join(*pR);
        }
    }

    if( ! bRowSourceAmbiguous )
    {
        rOutRowSourceDetected = true;
        rOutRowSource = ( nDataInRows > 0
                       ? chart::ChartDataRowSource_ROWS
                       : chart::ChartDataRowSource_COLUMNS );
    }
    else
    {
            // set DataRowSource to the better of the two ambiguities
            rOutRowSource = ( nDataInRows > nDataInColumns
                           ? chart::ChartDataRowSource_ROWS
                           : chart::ChartDataRowSource_COLUMNS );
    }
}

bool lcl_HasCategories(
    const uno::Reference< chart2::data::XDataSource >& xDataSource,
    bool & rOutHasCategories )
{
    bool bResult = false;
    uno::Reference< chart2::data::XLabeledDataSequence > xCategories(
        lcl_getCategoriesFromDataSource( xDataSource ));
    if( xCategories.is())
    {
        uno::Reference< lang::XServiceInfo > xValues( xCategories->getValues(), uno::UNO_QUERY );
        if (xValues.is())
        {
            rOutHasCategories = xValues->getImplementationName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ScChart2DataSequence"));
            bResult = true;
        }
    }
    return bResult;
}

bool lcl_HasFirstCellAsLabel(
    const uno::Reference< chart2::data::XDataSource >& xDataSource,
    bool & rOutHasFirstCellAsLabel )
{
    bool bResult = false;
    if( xDataSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(
            xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        if (nCount > 0 && aSequences[nCount - 1].is() )
        {
            uno::Reference< lang::XServiceInfo > xLabel( aSequences[nCount - 1]->getLabel(), uno::UNO_QUERY ); // take the last sequence, because the first has no label if it is also created
            if (xLabel.is())
            {
                rOutHasFirstCellAsLabel = xLabel->getImplementationName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ScChart2DataSequence"));
                bResult = true;
            }
        }
    }
    return bResult;
}

} // anonymous namespace

bool ScChart2DataProvider::addUpperLeftCornerIfMissing( ScRangeListRef& xRanges )
{
    bool bChanged=false;

    //returns true if the corner was added
    ScRangePtr pR = xRanges->First();
    if( !pR )
        return bChanged;

    SCCOL nMinColumn = MAXCOLCOUNT;
    SCROW nMinRow = MAXROWCOUNT;
    SCCOL nMaxColumn = ::std::max( pR->aStart.Col(), pR->aEnd.Col());
    SCROW nMaxRow = ::std::max( pR->aStart.Row(), pR->aEnd.Row());
    SCTAB nTable = pR->aStart.Tab();
    bool bUniqueTable = true;
    for(; pR; pR=xRanges->Next() )
    {
        if( bUniqueTable && pR->aStart.Tab() != nTable )
        {
            bUniqueTable = false;
            break;
        }

        nMinColumn = ::std::min( nMinColumn, pR->aStart.Col());
        nMinColumn = ::std::min( nMinColumn, pR->aEnd.Col());
        nMinRow = ::std::min( nMinRow, pR->aStart.Row());
        nMinRow = ::std::min( nMinRow, pR->aEnd.Row());

        nMaxColumn = ::std::max( nMaxColumn, pR->aStart.Col());
        nMaxColumn = ::std::max( nMaxColumn, pR->aEnd.Col());
        nMaxRow = ::std::max( nMaxRow, pR->aStart.Row());
        nMaxRow = ::std::max( nMaxRow, pR->aEnd.Row());
    }

    if( bUniqueTable
        && nMinRow < nMaxRow && nMinColumn < nMaxColumn
        && nMinRow < MAXROWCOUNT && nMinColumn < MAXCOLCOUNT
        && nMaxRow < MAXROWCOUNT && nMaxColumn < MAXCOLCOUNT )
    {
        ScRange aCorner( nMinColumn, nMinRow, nTable );

        //check whether the corner is contained
        bool bCornerMissing = !xRanges->In( aCorner );

        //and check whether the cells around the corner are contained thus the corner would be a natural part of the range
        bCornerMissing = bCornerMissing && xRanges->In( ScRange( nMinColumn, nMinRow+1, nTable ) );
        bCornerMissing = bCornerMissing && xRanges->In( ScRange( nMinColumn+1, nMinRow, nTable ) );
        bCornerMissing = bCornerMissing && xRanges->In( ScRange( nMinColumn+1, nMinRow+1, nTable ) );

        if( bCornerMissing )
        {
            //make a simple rectangular range if possible
            bool bSimpleRange = false;
            if( xRanges->Count()==2 )
            {
                ScRange aRightPart( ScAddress( nMinColumn+1, nMinRow, nTable),  ScAddress( nMaxColumn, nMaxRow, nTable) );
                ScRange aBottomPart( ScAddress( nMinColumn, nMinRow+1, nTable),  ScAddress( nMaxColumn, nMaxRow, nTable) );
                if( xRanges->In(aRightPart) && xRanges->In(aBottomPart)  )
                {
                    xRanges->RemoveAll();
                    xRanges->Append( ScRange( ScAddress( nMinColumn, nMinRow, nTable),  ScAddress( nMaxColumn, nMaxRow, nTable) ) );
                    bChanged=true;
                    bSimpleRange = true;
                }
            }

            if( !bSimpleRange )
            {
                xRanges->Join( aCorner );
                bChanged=true;
            }
        }
    }
    return bChanged;
}

uno::Sequence< beans::PropertyValue > SAL_CALL ScChart2DataProvider::detectArguments(
    const uno::Reference< chart2::data::XDataSource >& xDataSource )
    throw (uno::RuntimeException)
{
    ::std::vector< beans::PropertyValue > aResult;
    bool bRowSourceDetected = false;
    bool bFirstCellAsLabel = false;
    bool bHasCategories = true;
    ::rtl::OUString sRangeRep;

    chart::ChartDataRowSource eRowSource = chart::ChartDataRowSource_COLUMNS;

    ScRangeListRef xRanges = new ScRangeList;

    // CellRangeRepresentation
    {
        ScUnoGuard aGuard;
        DBG_ASSERT( m_pDocument, "No Document -> no detectArguments" );
        if(!m_pDocument)
            return lcl_VectorToSequence( aResult );

        lcl_detectRanges( xRanges, eRowSource, bRowSourceDetected, xDataSource, m_pDocument );

        //get range string
        String sRet;
        xRanges->Format(sRet, SCR_ABS_3D, m_pDocument);
        sRangeRep = ::rtl::OUString( sRet );
    }

    // TableNumberList
    {
        ::std::list< SCTAB > aTableNumList;
        ScRangePtr pR=xRanges->First();
        if( pR )
        {
            for (; pR; pR=xRanges->Next())
                aTableNumList.push_back( pR->aStart.Tab());
        }
        aResult.push_back(
            beans::PropertyValue( ::rtl::OUString::createFromAscii("TableNumberList"), -1,
                                  uno::makeAny( lcl_createTableNumberList( aTableNumList ) ),
                                  beans::PropertyState_DIRECT_VALUE ));
    }

    // DataRowSource (calculated before)
    if( bRowSourceDetected )
    {
        aResult.push_back(
            beans::PropertyValue( ::rtl::OUString::createFromAscii("DataRowSource"), -1,
                                  uno::makeAny( eRowSource ), beans::PropertyState_DIRECT_VALUE ));
    }

    // HasCategories
    if( bRowSourceDetected )
    {
        if( lcl_HasCategories( xDataSource, bHasCategories ))
        {
            aResult.push_back(
                beans::PropertyValue( ::rtl::OUString::createFromAscii("HasCategories"), -1,
                                      uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE ));
        }
    }

    // FirstCellAsLabel
    if( bRowSourceDetected )
    {
        lcl_HasFirstCellAsLabel( xDataSource, bFirstCellAsLabel );
        aResult.push_back(
            beans::PropertyValue( ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
                                  uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE ));
    }

    //add the left upper corner to the range when it is missing
    if( bRowSourceDetected && bFirstCellAsLabel && bHasCategories  )
    {
        if( addUpperLeftCornerIfMissing( xRanges ) )
        {
            String sRange;
            xRanges->Format( sRange, SCR_ABS_3D, m_pDocument );
            sRangeRep = ::rtl::OUString( sRange );
        }
    }

    // add cell range property
    aResult.push_back(
        beans::PropertyValue( ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
                              uno::makeAny( sRangeRep ), beans::PropertyState_DIRECT_VALUE ));

    //Sequence Mapping
    bool bSequencesReordered = true;//todo detect this above or detect this sequence mapping cheaper ...
    if( bSequencesReordered && bRowSourceDetected )
    {
        bool bDifferentIndexes = false;

        std::vector< sal_Int32 > aSequenceMappingVector;

        uno::Reference< chart2::data::XDataSource > xCompareDataSource;
        try
        {
            xCompareDataSource.set( this->createDataSource( lcl_VectorToSequence( aResult ) ) );
        }
        catch( const lang::IllegalArgumentException & )
        {
            // creation of data source to compare didn't work, so we cannot
            // create a sequence mapping
        }

        if( xDataSource.is() && xCompareDataSource.is() )
        {
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > aOldSequences(
                xCompareDataSource->getDataSequences() );
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aNewSequences(
                xDataSource->getDataSequences());

            rtl::OUString aOldLabel;
            rtl::OUString aNewLabel;
            rtl::OUString aOldValues;
            rtl::OUString aNewValues;
            rtl::OUString aEmpty;

            for( sal_Int32 nNewIndex = 0; nNewIndex < aNewSequences.getLength(); nNewIndex++ )
            {
                uno::Reference< chart2::data::XLabeledDataSequence> xNew( aNewSequences[nNewIndex] );
                for( sal_Int32 nOldIndex = 0; nOldIndex < aOldSequences.getLength(); nOldIndex++ )
                {
                    uno::Reference< chart2::data::XLabeledDataSequence> xOld( aOldSequences[nOldIndex] );

                    if( xOld.is() && xNew.is() )
                    {
                        aOldLabel = aNewLabel = aOldValues = aNewValues = aEmpty;
                        if( xOld.is() && xOld->getLabel().is() )
                            aOldLabel = xOld->getLabel()->getSourceRangeRepresentation();
                        if( xNew.is() && xNew->getLabel().is() )
                            aNewLabel = xNew->getLabel()->getSourceRangeRepresentation();
                        if( xOld.is() && xOld->getValues().is() )
                            aOldValues = xOld->getValues()->getSourceRangeRepresentation();
                        if( xNew.is() && xNew->getValues().is() )
                            aNewValues = xNew->getValues()->getSourceRangeRepresentation();

                        if( aOldLabel.equals(aNewLabel)
                            && ( aOldValues.equals(aNewValues) ) )
                        {
                            if( nOldIndex!=nNewIndex )
                                bDifferentIndexes = true;
                            aSequenceMappingVector.push_back(nOldIndex);
                            break;
                        }
                    }
                }
            }
        }

        if( bDifferentIndexes && aSequenceMappingVector.size() )
        {
            aResult.push_back(
                beans::PropertyValue( ::rtl::OUString::createFromAscii("SequenceMapping"), -1,
                    uno::makeAny( lcl_VectorToSequence(aSequenceMappingVector) )
                    , beans::PropertyState_DIRECT_VALUE ));
        }
    }

    return lcl_VectorToSequence( aResult );
}

::sal_Bool SAL_CALL ScChart2DataProvider::createDataSequenceByRangeRepresentationPossible( const ::rtl::OUString& aRangeRepresentation )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if( ! m_pDocument )
        return false;

    ScRangeList aRangeList;
    USHORT nResult = aRangeList.Parse( aRangeRepresentation, m_pDocument );
    //! if anything is missing, SCA_VALID shouldn't be set
    const USHORT nNeeded = SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                           SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2;
    bool bValid = ( (nResult & nNeeded ) == nNeeded );

    return bValid;
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL
    ScChart2DataProvider::createDataSequenceByRangeRepresentation(
    const ::rtl::OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference< chart2::data::XDataSequence > xResult;

    DBG_ASSERT( m_pDocument, "No Document -> no createDataSequenceByRangeRepresentation" );
    if(!m_pDocument || (aRangeRepresentation.getLength() == 0))
        return xResult;

    ScRangeListRef aRangeList = new ScRangeList();
    if (ScRangeStringConverter::GetRangeListFromString( *aRangeList, aRangeRepresentation, m_pDocument, ';' ))
        xResult.set(new ScChart2DataSequence(m_pDocument, this, aRangeList));

    return xResult;
}

uno::Reference< sheet::XRangeSelection > SAL_CALL ScChart2DataProvider::getRangeSelection()
    throw (uno::RuntimeException)
{
    uno::Reference< sheet::XRangeSelection > xResult;

    uno::Reference< frame::XModel > xModel( lcl_GetXModel( m_pDocument ));
    if( xModel.is())
        xResult.set( xModel->getCurrentController(), uno::UNO_QUERY );

    return xResult;
}

/*uno::Reference< util::XNumberFormatsSupplier > SAL_CALL ScChart2DataProvider::getNumberFormatsSupplier()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XNumberFormatsSupplier >( lcl_GetXModel( m_pDocument ), uno::UNO_QUERY );
}*/

// XRangeXMLConversion ---------------------------------------------------

rtl::OUString SAL_CALL ScChart2DataProvider::convertRangeToXML( const rtl::OUString& sRangeRepresentation )
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    rtl::OUString sRet;

    if( m_pDocument )
    {
        ScRangeList aList;
        if( !ScRangeStringConverter::GetRangeListFromString(aList, sRangeRepresentation, m_pDocument, ';'))
            throw lang::IllegalArgumentException();
        ScRangeStringConverter::GetStringFromRangeList(sRet, &aList, m_pDocument);
    }

    return sRet;
}

rtl::OUString SAL_CALL ScChart2DataProvider::convertRangeFromXML( const rtl::OUString& sXMLRange )
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    // #i74062# When loading flat XML, this is called before the referenced sheets are in the document,
    // so the conversion has to take place directly with the strings, without looking up the sheets.

    const sal_Unicode cSep = ' ';
    const sal_Unicode cQuote = '\'';

    rtl::OUStringBuffer sRet;
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        rtl::OUString sToken;
        ScRangeStringConverter::GetTokenByOffset( sToken, sXMLRange, nOffset, cSep, cQuote );
        if( nOffset >= 0 )
        {
            // convert one address (remove dots)

            String aUIString(sToken);

            sal_Int32 nIndex = ScRangeStringConverter::IndexOf( sToken, ':', 0, cQuote );
            if ( nIndex >= 0 && nIndex < aUIString.Len() - 1 &&
                    aUIString.GetChar((xub_StrLen)nIndex + 1) == (sal_Unicode) '.' )
                aUIString.Erase( (xub_StrLen)nIndex + 1, 1 );

            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                aUIString.Erase( 0, 1 );

            if( sRet.getLength() )
                sRet.append( (sal_Unicode) ';' );
            sRet.append( aUIString );
        }
    }

    return sRet.makeStringAndClear();
}

// DataSource ================================================================

ScChart2DataSource::ScChart2DataSource( ScDocument* pDoc)
    : m_pDocument( pDoc)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}


ScChart2DataSource::~ScChart2DataSource()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}


void ScChart2DataSource::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}


uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > SAL_CALL
ScChart2DataSource::getDataSequences() throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;

    LabeledList::const_iterator aItr(m_aLabeledSequences.begin());
    LabeledList::const_iterator aEndItr(m_aLabeledSequences.end());

    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aRet(m_aLabeledSequences.size());

    sal_Int32 i = 0;
    while (aItr != aEndItr)
    {
        aRet[i] = *aItr;
        ++i;
        ++aItr;
    }

    return aRet;

/*    typedef ::std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > tVec;
    tVec aVec;
    bool bSeries = false;
    // split into columns - FIXME: different if GlueState() is used
    for ( ScRangePtr p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        for ( SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
        {
            uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
                new ScChart2LabeledDataSequence( m_pDocument));
            if( xLabeledSeq.is())
            {
                aVec.push_back( xLabeledSeq );
                if( bSeries )
                {
                    ScRangeListRef aColRanges = new ScRangeList;
                    // one single sheet selected assumed for now
                    aColRanges->Append( ScRange( nCol, p->aStart.Row(),
                                                 p->aStart.Tab(), nCol, p->aStart.Row(),
                                                 p->aStart.Tab()));
                    // TEST: add range two times, once as label, once as data
                    // TODO: create pure Numerical and Text sequences if possible
                    uno::Reference< chart2::data::XDataSequence > xLabel(
                        new ScChart2DataSequence( m_pDocument, aColRanges));

                    // set role
                    uno::Reference< beans::XPropertySet > xProp( xLabel, uno::UNO_QUERY );
                    if( xProp.is())
                        xProp->setPropertyValue(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )),
                            ::uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "label" ))));

                    xLabeledSeq->setLabel( xLabel );
                }

                ScRangeListRef aColRanges = new ScRangeList;

                // one single sheet selected assumed for now
                aColRanges->Append( ScRange( nCol, p->aStart.Row() + 1,
                                             p->aStart.Tab(), nCol, p->aEnd.Row(),
                                             p->aStart.Tab()));
                uno::Reference< chart2::data::XDataSequence > xData(
                    new ScChart2DataSequence( m_pDocument, aColRanges));

                // set role
                uno::Reference< beans::XPropertySet > xProp( xData, uno::UNO_QUERY );
                if( xProp.is())
                    xProp->setPropertyValue(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )),
                        ::uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "values" ))));

                xLabeledSeq->setValues( xData );

                bSeries = true;
            }
        }
    }
    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > aSequences(
            aVec.size());
    uno::Reference< chart2::data::XLabeledDataSequence> * pArr = aSequences.getArray();
    sal_Int32 j = 0;
    for ( tVec::const_iterator iSeq = aVec.begin(); iSeq != aVec.end();
            ++iSeq, ++j)
    {
        pArr[j] = *iSeq;
    }
    return aSequences;*/
}

void ScChart2DataSource::AddLabeledSequence(const uno::Reference < chart2::data::XLabeledDataSequence >& xNew)
{
    m_aLabeledSequences.push_back(xNew);
}

// LabeledDataSequence =======================================================

ScChart2LabeledDataSequence::ScChart2LabeledDataSequence(
    ScDocument* pDoc ) :
        m_pDocument( pDoc )
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}

ScChart2LabeledDataSequence::~ScChart2LabeledDataSequence()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}

// SfxListener -----------------------------------------------------------

void ScChart2LabeledDataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        ScUnoGuard aGuard;
        m_pDocument = NULL;
    }
}

// XLabeledDataSequence --------------------------------------------------

uno::Reference< chart2::data::XDataSequence > SAL_CALL ScChart2LabeledDataSequence::getValues()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return m_aData;
}

void SAL_CALL ScChart2LabeledDataSequence::setValues(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    m_aData = xSequence;
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL ScChart2LabeledDataSequence::getLabel()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return m_aLabel;
}

void SAL_CALL ScChart2LabeledDataSequence::setLabel(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    m_aLabel = xSequence;
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2LabeledDataSequence::createClone()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference< util::XCloneable > xToClone(m_aData, uno::UNO_QUERY);
    if (xToClone.is())
    {
        ScChart2LabeledDataSequence* pRet = new ScChart2LabeledDataSequence(m_pDocument);
        uno::Reference< chart2::data::XDataSequence > xSequence(xToClone->createClone(), uno::UNO_QUERY);
        pRet->setValues(xSequence);
        xToClone.set(m_aLabel, uno::UNO_QUERY);
        if(xToClone.is())
        {
            xSequence.set(xToClone->createClone(), uno::UNO_QUERY);
            pRet->setLabel(xSequence);
        }
        return pRet;
    }
    return NULL;
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2LabeledDataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    // quick'n dirty: just add the listener to each DataSequence

    uno::Reference<util::XModifyBroadcaster> xDataBroadcaster( m_aData, uno::UNO_QUERY );
    if ( xDataBroadcaster.is() )
        xDataBroadcaster->addModifyListener( aListener );
    uno::Reference<util::XModifyBroadcaster> xLabelBroadcaster( m_aLabel, uno::UNO_QUERY );
    if ( xLabelBroadcaster.is() )
        xLabelBroadcaster->addModifyListener( aListener );
}

void SAL_CALL ScChart2LabeledDataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    uno::Reference<util::XModifyBroadcaster> xDataBroadcaster( m_aData, uno::UNO_QUERY );
    if ( xDataBroadcaster.is() )
        xDataBroadcaster->removeModifyListener( aListener );
    uno::Reference<util::XModifyBroadcaster> xLabelBroadcaster( m_aLabel, uno::UNO_QUERY );
    if ( xLabelBroadcaster.is() )
        xLabelBroadcaster->removeModifyListener( aListener );
}

// DataSequence ==============================================================

ScChart2DataSequence::ScChart2DataSequence( ScDocument* pDoc,
        const uno::Reference < chart2::data::XDataProvider >& xDP,
        const ScRangeListRef& rRangeList)
    : m_bHidden( sal_False)
    , m_xRanges( rRangeList)
    , m_nObjectId( 0 )
    , m_pDocument( pDoc)
    , m_xDataProvider( xDP)
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
    , m_pValueListener( NULL )
    , m_bGotDataChangedHint( FALSE )
{
    if ( m_pDocument )
    {
        m_pDocument->AddUnoObject( *this);
        m_nObjectId = m_pDocument->GetNewUnoId();
    }
    // FIXME: real implementation of identifier and it's mapping to ranges.
    // Reuse ScChartListener?

    // BM: don't use names of named ranges but the UI range strings
//  String  aStr;
//  rRangeList->Format( aStr, SCR_ABS_3D, m_pDocument );
//    m_aIdentifier = ::rtl::OUString( aStr );

//      m_aIdentifier = ::rtl::OUString::createFromAscii( "ID_");
//      static sal_Int32 nID = 0;
//      m_aIdentifier += ::rtl::OUString::valueOf( ++nID);
}


ScChart2DataSequence::~ScChart2DataSequence()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);

    delete m_pValueListener;
}


void ScChart2DataSequence::RefChanged()
{
    if( m_pValueListener && m_aValueListeners.Count() != 0 )
    {
        m_pValueListener->EndListeningAll();

        if( m_pDocument )
        {
            ULONG nCount = m_xRanges->Count();
            for (ULONG i=0; i<nCount; i++)
                m_pDocument->StartListeningArea( *m_xRanges->GetObject(i), m_pValueListener );
        }
    }
}


void ScChart2DataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = static_cast<const SfxSimpleHint&>(rHint).GetId();
        if ( nId ==SFX_HINT_DYING )
        {
            m_pDocument = NULL;
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            // delayed broadcast as in ScCellRangesBase

            if ( m_bGotDataChangedHint && m_pDocument )
            {
                lang::EventObject aEvent;
                aEvent.Source.set((cppu::OWeakObject*)this);

                if( m_pDocument )
                {
                    for ( USHORT n=0; n<m_aValueListeners.Count(); n++ )
                        m_pDocument->AddUnoListenerCall( *m_aValueListeners[n], aEvent );
                }

                m_bGotDataChangedHint = FALSE;
            }
        }
    }
    else if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        ScRangeList* pUndoRanges = NULL;
        if ( m_pDocument->HasUnoRefUndo() )
            pUndoRanges = new ScRangeList( *m_xRanges );

        if ( m_xRanges->UpdateReference( rRef.GetMode(), m_pDocument, rRef.GetRange(),
                                    rRef.GetDx(), rRef.GetDy(), rRef.GetDz() ) )
        {
            RefChanged();

            // any change of the range address is broadcast to value (modify) listeners
            if ( m_aValueListeners.Count() )
                m_bGotDataChangedHint = TRUE;

            if ( pUndoRanges )
                m_pDocument->AddUnoRefChange( m_nObjectId, *pUndoRanges );
        }

        delete pUndoRanges;
    }
    else if ( rHint.ISA( ScUnoRefUndoHint ) )
    {
        const ScUnoRefUndoHint& rUndoHint = static_cast<const ScUnoRefUndoHint&>(rHint);
        if ( rUndoHint.GetObjectId() == m_nObjectId )
        {
            // restore ranges from hint

            m_xRanges = new ScRangeList( rUndoHint.GetRanges() );

            RefChanged();
            if ( m_aValueListeners.Count() )
                m_bGotDataChangedHint = TRUE;     // need to broadcast the undo, too
        }
    }
}


IMPL_LINK( ScChart2DataSequence, ValueListenerHdl, SfxHint*, pHint )
{
    if ( m_pDocument && pHint && pHint->ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint*)pHint)->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING) )
    {
        //  This may be called several times for a single change, if several formulas
        //  in the range are notified. So only a flag is set that is checked when
        //  SFX_HINT_DATACHANGED is received.

        m_bGotDataChangedHint = TRUE;
    }
    return 0;
}


uno::Sequence< uno::Any> SAL_CALL ScChart2DataSequence::getData()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    const ScDocument* pDoc = m_pDocument;
    sal_Int32 nCount = 0;
    ScRangePtr p;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        nCount += sal_Int32(p->aEnd.Col() - p->aStart.Col() + 1) *
            (p->aEnd.Row() - p->aStart.Row() + 1) * (p->aEnd.Tab() -
                                                     p->aStart.Tab() + 1);
    }
    uno::Sequence< uno::Any> aSeq( nCount);
    uno::Any * pArr = aSeq.getArray();
    nCount = 0;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        // TODO: use DocIter?
        ScAddress aAdr( p->aStart);
        for ( SCTAB nTab = p->aStart.Tab(); nTab <= p->aEnd.Tab(); ++nTab)
        {
            aAdr.SetTab( nTab);
            for ( SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
            {
                aAdr.SetCol( nCol);
                for ( SCROW nRow = p->aStart.Row(); nRow <= p->aEnd.Row();
                        ++nRow)
                {
                    aAdr.SetRow( nRow);
                    ScBaseCell* pCell = pDoc->GetCell( aAdr);
                    if ( pCell)
                    {
                        switch ( pCell->GetCellType())
                        {
                            case CELLTYPE_VALUE:
                                pArr[nCount] <<= static_cast< ScValueCell*>(
                                        pCell)->GetValue();
                            break;
                            case CELLTYPE_FORMULA:
                            {
                                ScFormulaCell* pFCell = static_cast<
                                    ScFormulaCell*>( pCell);
                                USHORT nErr = pFCell->GetErrCode();
                                if  ( !nErr)
                                {
                                    if ( pFCell->HasValueData())
                                        pArr[nCount] <<= pFCell->GetValue();
                                    else
                                    {
                                        String aStr;
                                        pFCell->GetString( aStr);
                                        pArr[nCount] <<= ::rtl::OUString(
                                                aStr);
                                    }
                                }
                            }
                            default:
                            {
                                if ( pCell->HasStringData())
                                    pArr[nCount] <<= ::rtl::OUString(pCell->GetStringData());

                            }
                        }
                    }
                    ++nCount;
                }
            }
        }
    }
    return aSeq;
}

// XNumericalDataSequence --------------------------------------------------

uno::Sequence< double > SAL_CALL ScChart2DataSequence::getNumericalData()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    double fNAN;
    ::rtl::math::setNan( & fNAN );

    const ScDocument* pDoc = m_pDocument;
    sal_Int32 nCount = 0;
    ScRangePtr p;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        nCount += sal_Int32(p->aEnd.Col() - p->aStart.Col() + 1) *
            (p->aEnd.Row() - p->aStart.Row() + 1) * (p->aEnd.Tab() -
                                                     p->aStart.Tab() + 1);
    }
    uno::Sequence< double > aSeq( nCount);
    double* pArr = aSeq.getArray();
    nCount = 0;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        // TODO: use DocIter?
        ScAddress aAdr( p->aStart);
        for ( SCTAB nTab = p->aStart.Tab(); nTab <= p->aEnd.Tab(); ++nTab)
        {
            aAdr.SetTab( nTab);
            for ( SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
            {
                aAdr.SetCol( nCol);
                for ( SCROW nRow = p->aStart.Row(); nRow <= p->aEnd.Row();
                        ++nRow)
                {
                    pArr[nCount] = fNAN;

                    aAdr.SetRow( nRow);
                    ScBaseCell* pCell = pDoc->GetCell( aAdr);
                    if ( pCell)
                    {
                        switch ( pCell->GetCellType())
                        {
                            case CELLTYPE_VALUE:
                                pArr[nCount] = static_cast< ScValueCell*>(
                                        pCell)->GetValue();
                            break;
                            case CELLTYPE_FORMULA:
                            {
                                ScFormulaCell* pFCell = static_cast<
                                    ScFormulaCell*>( pCell);
                                USHORT nErr = pFCell->GetErrCode();
                                if  ( !nErr)
                                {
                                    if ( pFCell->HasValueData())
                                        pArr[nCount] = pFCell->GetValue();
                                }
                            }
                            default:
                                // nothing
                                break;
                        }
                    }
                    ++nCount;
                }
            }
        }
    }
    return aSeq;
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL ScChart2DataSequence::getTextualData(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    sal_Int32 nCount = 0;
    ScRangePtr p;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        nCount += sal_Int32(p->aEnd.Col() - p->aStart.Col() + 1) *
            (p->aEnd.Row() - p->aStart.Row() + 1) * (p->aEnd.Tab() -
                                                     p->aStart.Tab() + 1);
    }
    uno::Sequence< rtl::OUString > aSeq( nCount);
    rtl::OUString* pArr = aSeq.getArray();
    nCount = 0;
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( lcl_GetSpreadSheetDocument( m_pDocument ));
    if ( xSpreadDoc.is() )
    {
        uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
        if ( xIndex.is() )
        {
            uno::Reference< table::XCellRange > xSheet;
            for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
            {
                // TODO: use DocIter?
                table::CellAddress aStart, aEnd;
                ScUnoConversion::FillApiAddress( aStart, p->aStart );
                ScUnoConversion::FillApiAddress( aEnd, p->aEnd );
                for ( sal_Int16 nSheet = aStart.Sheet; nSheet <= aEnd.Sheet; ++nSheet)
                {
                    xSheet.set(xIndex->getByIndex(nSheet), uno::UNO_QUERY);
                    for ( sal_Int32 nCol = aStart.Column; nCol <= aEnd.Column; ++nCol)
                    {
                        for ( sal_Int32 nRow = aStart.Row; nRow <= aEnd.Row; ++nRow)
                        {
                            uno::Reference< text::XText > xText(xSheet->getCellByPosition(nCol, nRow), uno::UNO_QUERY);
                            if (xText.is())
                            {
                                pArr[nCount] = xText->getString();
                                ++nCount;
                            }
                        }
                    }
                }
            }
        }
    }
    return aSeq;
}

::rtl::OUString SAL_CALL ScChart2DataSequence::getSourceRangeRepresentation()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String  aStr;
    DBG_ASSERT( m_pDocument, "No Document -> no SourceRangeRepresentation" );
    if( m_pDocument )
        m_xRanges->Format( aStr, SCR_ABS_3D, m_pDocument );
    return aStr;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScChart2DataSequence::generateLabel(chart2::data::LabelOrigin eOrigin)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    sal_Int32 nCount = 0;
    sal_Bool bColumn = sal_True;

    ScRangePtr p;

//  DR: no idea about this assertion
//    DBG_ASSERT(m_xRanges->Count() == 1, "not handled count of ranges");

    SCCOL nCols = 0;
    SCROW nRows = 0;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        p->Justify();
        nCols += p->aEnd.Col() - p->aStart.Col() + 1;
        nRows += p->aEnd.Row() - p->aStart.Row() + 1;
    }

    if ((eOrigin == chart2::data::LabelOrigin_SHORT_SIDE) ||
        (eOrigin == chart2::data::LabelOrigin_LONG_SIDE))
    {
        if (nRows > nCols)
        {
            if (eOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
                bColumn = sal_True;
            else
                bColumn = sal_False;
        }
        else if (nCols > nRows)
        {
            if (eOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
                bColumn = sal_False;
            else
                bColumn = sal_True;
        }
        else
            return uno::Sequence< rtl::OUString >();
    }

    nCount = bColumn ? nCols : nRows;

    uno::Sequence< rtl::OUString > aSeq( nCount);
    rtl::OUString* pArr = aSeq.getArray();
    nCount = 0;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        if (bColumn)
        {
            for (SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
            {
                if( eOrigin != chart2::data::LabelOrigin_LONG_SIDE )
                {
                    String aString = ScGlobal::GetRscString(STR_COLUMN);
                    aString += ' ';
                    ScAddress aPos( nCol, 0, 0 );
                    String aColStr;
                    aPos.Format( aColStr, SCA_VALID_COL, NULL );
                    aString += aColStr;
                    pArr[nCount] = aString;
                }
                else //only indices for categories
                    pArr[nCount] = String::CreateFromInt32( nCount+1 );
                ++nCount;
            }
        }
        else
        {
            for (sal_Int32 nRow = p->aStart.Row(); nRow <= p->aEnd.Row(); ++nRow)
            {
                if( eOrigin != chart2::data::LabelOrigin_LONG_SIDE )
                {
                    String aString = ScGlobal::GetRscString(STR_ROW);
                    aString += ' ';
                    aString += String::CreateFromInt32( nRow+1 );
                    pArr[nCount] = aString;
                }
                else //only indices for categories
                    pArr[nCount] = String::CreateFromInt32( nCount+1 );
                ++nCount;
            }
        }
    }
    return aSeq;
}

::sal_Int32 SAL_CALL ScChart2DataSequence::getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    // index -1 means a heuristic value for the entire sequence
    bool bGetSeriesFormat = (nIndex == -1);
    sal_Int32 nResult = 0;

    ScUnoGuard aGuard;
    if ( !m_pDocument)
        return nResult;

    sal_Int32 nCount = 0;
    bool bFound = false;
    ScRangePtr p;

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( lcl_GetSpreadSheetDocument( m_pDocument ));
    if ( xSpreadDoc.is() )
    {
        uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
        if ( xIndex.is() )
        {
            uno::Reference< table::XCellRange > xSheet;
            for ( p = m_xRanges->First(); p && !bFound; p = m_xRanges->Next())
            {
                // TODO: use DocIter?
                table::CellAddress aStart, aEnd;
                ScUnoConversion::FillApiAddress( aStart, p->aStart );
                ScUnoConversion::FillApiAddress( aEnd, p->aEnd );
                for ( sal_Int16 nSheet = aStart.Sheet; nSheet <= aEnd.Sheet && !bFound; ++nSheet)
                {
                    xSheet.set(xIndex->getByIndex(nSheet), uno::UNO_QUERY);
                    for ( sal_Int32 nCol = aStart.Column; nCol <= aEnd.Column && !bFound; ++nCol)
                    {
                        for ( sal_Int32 nRow = aStart.Row; nRow <= aEnd.Row && !bFound; ++nRow)
                        {
                            if( bGetSeriesFormat )
                            {
                                // TODO: use nicer heuristic
                                // return format of first non-empty cell
                                uno::Reference< text::XText > xText(
                                    xSheet->getCellByPosition(nCol, nRow), uno::UNO_QUERY);
                                if (xText.is() && xText->getString().getLength())
                                {
                                    uno::Reference< beans::XPropertySet > xProp(xText, uno::UNO_QUERY);
                                    if( xProp.is())
                                        xProp->getPropertyValue(
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"))) >>= nResult;
                                    bFound = true;
                                    break;
                                }
                            }
                            else if( nCount == nIndex )
                            {
                                uno::Reference< beans::XPropertySet > xProp(
                                    xSheet->getCellByPosition(nCol, nRow), uno::UNO_QUERY);
                                if( xProp.is())
                                    xProp->getPropertyValue(
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"))) >>= nResult;
                                bFound = true;
                                break;
                            }
                            ++nCount;
                        }
                    }
                }
            }
        }
    }

    return nResult;
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2DataSequence::createClone()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < util::XCloneable > xClone;
    if (m_xDataProvider.is())
    {
        xClone.set(m_xDataProvider->createDataSequenceByRangeRepresentation(getSourceRangeRepresentation()), uno::UNO_QUERY);

        // copy properties
        uno::Reference< beans::XPropertySet > xProp( xClone, uno::UNO_QUERY );
        if( xProp.is())
        {
            xProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ROLE )),
                                     uno::makeAny( m_aRole ));
            xProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ISHIDDEN )),
                                     uno::makeAny( m_bHidden ));
        }
    }

    return xClone;
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2DataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    // like ScCellRangesBase::addModifyListener

    ScUnoGuard aGuard;
     if ( m_xRanges->Count() == 0 )
        return;
//      throw uno::RuntimeException();

    uno::Reference<util::XModifyListener> *pObj =
            new uno::Reference<util::XModifyListener>( aListener );
    m_aValueListeners.Insert( pObj, m_aValueListeners.Count() );

    if ( m_aValueListeners.Count() == 1 )
    {
        if (!m_pValueListener)
            m_pValueListener = new ScLinkListener( LINK( this, ScChart2DataSequence, ValueListenerHdl ) );

        if( m_pDocument )
        {
            ULONG nCount = m_xRanges->Count();
            for (ULONG i=0; i<nCount; i++)
                m_pDocument->StartListeningArea( *m_xRanges->GetObject(i), m_pValueListener );
        }

        acquire();  // don't lose this object (one ref for all listeners)
    }
}

void SAL_CALL ScChart2DataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    // like ScCellRangesBase::removeModifyListener

    ScUnoGuard aGuard;
     if ( m_xRanges->Count() == 0 )
        return;
//      throw uno::RuntimeException();

    acquire();      // in case the listeners have the last ref - released below

    USHORT nCount = m_aValueListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<util::XModifyListener> *pObj = m_aValueListeners[n];
        if ( *pObj == aListener )
        {
            m_aValueListeners.DeleteAndDestroy( n );

            if ( m_aValueListeners.Count() == 0 )
            {
                if (m_pValueListener)
                    m_pValueListener->EndListeningAll();

                release();      // release the ref for the listeners
            }

            break;
        }
    }

    release();      // might delete this object
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2DataSequence::getPropertySetInfo() throw( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}


void SAL_CALL ScChart2DataSequence::setPropertyValue(
        const ::rtl::OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
    {
        if ( !(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ISHIDDEN)))
    {
        if ( !(rValue >>= m_bHidden))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
}


uno::Any SAL_CALL ScChart2DataSequence::getPropertyValue(
        const ::rtl::OUString& rPropertyName)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
        aRet <<= m_aRole;
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ISHIDDEN)))
        aRet <<= m_bHidden;
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
    return aRet;
}


void SAL_CALL ScChart2DataSequence::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataSequence::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataSequence::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataSequence::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}

// XUnoTunnel

// sal_Int64 SAL_CALL ScChart2DataSequence::getSomething(
//              const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
// {
//  if ( rId.getLength() == 16 &&
//           0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
//                                  rId.getConstArray(), 16 ) )
//  {
//      return (sal_Int64)this;
//  }
//  return 0;
// }

// // static
// const uno::Sequence<sal_Int8>& ScChart2DataSequence::getUnoTunnelId()
// {
//  static uno::Sequence<sal_Int8> * pSeq = 0;
//  if( !pSeq )
//  {
//      osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
//      if( !pSeq )
//      {
//          static uno::Sequence< sal_Int8 > aSeq( 16 );
//          rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
//          pSeq = &aSeq;
//      }
//  }
//  return *pSeq;
// }

// // static
// ScChart2DataSequence* ScChart2DataSequence::getImplementation( const uno::Reference<uno::XInterface> xObj )
// {
//  ScChart2DataSequence* pRet = NULL;
//  uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
//  if (xUT.is())
//      pRet = (ScChart2DataSequence*) xUT->getSomething( getUnoTunnelId() );
//  return pRet;
// }

// DataSequence ==============================================================

ScChart2EmptyDataSequence::ScChart2EmptyDataSequence( ScDocument* pDoc,
        const uno::Reference < chart2::data::XDataProvider >& xDP,
        const ScRangeListRef& rRangeList,
        sal_Bool bColumn)
    : m_bHidden( sal_False)
    , m_xRanges( rRangeList)
    , m_pDocument( pDoc)
    , m_xDataProvider( xDP)
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
    , m_bColumn(bColumn)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
    // FIXME: real implementation of identifier and it's mapping to ranges.
    // Reuse ScChartListener?

    // BM: don't use names of named ranges but the UI range strings
//  String  aStr;
//  rRangeList->Format( aStr, SCR_ABS_3D, m_pDocument );
//    m_aIdentifier = ::rtl::OUString( aStr );

//      m_aIdentifier = ::rtl::OUString::createFromAscii( "ID_");
//      static sal_Int32 nID = 0;
//      m_aIdentifier += ::rtl::OUString::valueOf( ++nID);
}


ScChart2EmptyDataSequence::~ScChart2EmptyDataSequence()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}


void ScChart2EmptyDataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}


uno::Sequence< uno::Any> SAL_CALL ScChart2EmptyDataSequence::getData()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();
    return uno::Sequence< uno::Any>();
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL ScChart2EmptyDataSequence::getTextualData(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    sal_Int32 nCount = 0;
    ScRangePtr p;

    DBG_ASSERT(m_xRanges->Count() == 1, "not handled count of ranges");

    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        p->Justify();
        // TODO: handle overlaping ranges?
        nCount += m_bColumn ? p->aEnd.Col() - p->aStart.Col() + 1 : p->aEnd.Row() - p->aStart.Row() + 1;
    }
    uno::Sequence< rtl::OUString > aSeq( nCount);
    rtl::OUString* pArr = aSeq.getArray();
    nCount = 0;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        if (m_bColumn)
        {
            for (SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
            {
                String aString = ScGlobal::GetRscString(STR_COLUMN);
                aString += ' ';
                ScAddress aPos( nCol, 0, 0 );
                String aColStr;
                aPos.Format( aColStr, SCA_VALID_COL, NULL );
                aString += aColStr;
                pArr[nCount] = aString;
                ++nCount;
            }
        }
        else
        {
            for (sal_Int32 nRow = p->aStart.Row(); nRow <= p->aEnd.Row(); ++nRow)
            {
                String aString = ScGlobal::GetRscString(STR_ROW);
                aString += ' ';
                aString += String::CreateFromInt32( nRow+1 );
                pArr[nCount] = aString;
                ++nCount;
            }
        }
    }
    return aSeq;
}

::rtl::OUString SAL_CALL ScChart2EmptyDataSequence::getSourceRangeRepresentation()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String  aStr;
    DBG_ASSERT( m_pDocument, "No Document -> no SourceRangeRepresentation" );
    if( m_pDocument )
        m_xRanges->Format( aStr, SCR_ABS_3D, m_pDocument );
    return aStr;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScChart2EmptyDataSequence::generateLabel(chart2::data::LabelOrigin /*nOrigin*/)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Sequence< ::rtl::OUString > aRet;
    return aRet;
}

::sal_Int32 SAL_CALL ScChart2EmptyDataSequence::getNumberFormatKeyByIndex( ::sal_Int32 /*nIndex*/ )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    sal_Int32 nResult = 0;

    ScUnoGuard aGuard;
    if ( !m_pDocument)
        return nResult;

    return nResult;
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2EmptyDataSequence::createClone()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (m_xDataProvider.is())
    {
        // copy properties
        uno::Reference < util::XCloneable > xClone(new ScChart2EmptyDataSequence(m_pDocument, m_xDataProvider, new ScRangeList(*m_xRanges), m_bColumn));
        uno::Reference< beans::XPropertySet > xProp( xClone, uno::UNO_QUERY );
        if( xProp.is())
        {
            xProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ROLE )),
                                     uno::makeAny( m_aRole ));
            xProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ISHIDDEN )),
                                     uno::makeAny( m_bHidden ));
        }
        return xClone;
    }
    return uno::Reference< util::XCloneable >();
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2EmptyDataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& /*aListener*/ )
    throw (uno::RuntimeException)
{
    // TODO: Implement
}

void SAL_CALL ScChart2EmptyDataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& /*aListener*/ )
    throw (uno::RuntimeException)
{
    // TODO: Implement
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2EmptyDataSequence::getPropertySetInfo() throw( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}


void SAL_CALL ScChart2EmptyDataSequence::setPropertyValue(
        const ::rtl::OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
    {
        if ( !(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ISHIDDEN)))
    {
        if ( !(rValue >>= m_bHidden))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
}


uno::Any SAL_CALL ScChart2EmptyDataSequence::getPropertyValue(
        const ::rtl::OUString& rPropertyName)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
        aRet <<= m_aRole;
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ISHIDDEN)))
        aRet <<= m_bHidden;
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
    return aRet;
}


void SAL_CALL ScChart2EmptyDataSequence::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2EmptyDataSequence::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2EmptyDataSequence::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2EmptyDataSequence::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/ )
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}

// XUnoTunnel

// sal_Int64 SAL_CALL ScChart2EmptyDataSequence::getSomething(
//              const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
// {
//  if ( rId.getLength() == 16 &&
//           0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
//                                  rId.getConstArray(), 16 ) )
//  {
//      return (sal_Int64)this;
//  }
//  return 0;
// }

// // static
// const uno::Sequence<sal_Int8>& ScChart2EmptyDataSequence::getUnoTunnelId()
// {
//  static uno::Sequence<sal_Int8> * pSeq = 0;
//  if( !pSeq )
//  {
//      osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
//      if( !pSeq )
//      {
//          static uno::Sequence< sal_Int8 > aSeq( 16 );
//          rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
//          pSeq = &aSeq;
//      }
//  }
//  return *pSeq;
// }

// // static
// ScChart2DataSequence* ScChart2EmptyDataSequence::getImplementation( const uno::Reference<uno::XInterface> xObj )
// {
//  ScChart2DataSequence* pRet = NULL;
//  uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
//  if (xUT.is())
//      pRet = (ScChart2EmptyDataSequence*) xUT->getSomething( getUnoTunnelId() );
//  return pRet;
// }


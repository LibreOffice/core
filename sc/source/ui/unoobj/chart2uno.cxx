/*************************************************************************
 *
 *  $RCSfile: chart2uno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $  $Date: 2004-06-04 11:53:27 $
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

#include "chart2uno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "unoguard.hxx"
#include "cell.hxx"

#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HDL_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif

SC_SIMPLE_SERVICE_INFO( ScChart2DataProvider, "ScChart2DataProvider",
        "com.sun.star.chart2.DataProvider")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSource, "ScChart2DataSource",
        "com.sun.star.chart2.DataSource")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSequence, "ScChart2DataSequence",
        "com.sun.star.chart2.DataSequence")

using namespace ::com::sun::star;


// DataProvider ==============================================================

ScChart2DataProvider::ScChart2DataProvider( ScDocShell* pDocSh)
    : pDocShell( pDocSh)
{
    if ( pDocShell )
        pDocShell->GetDocument()->AddUnoObject( *this);
}


ScChart2DataProvider::~ScChart2DataProvider()
{
    if ( pDocShell )
        pDocShell->GetDocument()->RemoveUnoObject( *this);
}


void ScChart2DataProvider::Notify( SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;
    }
}


uno::Reference< chart2::XDataSource> SAL_CALL
ScChart2DataProvider::getDataByRangeRepresentation(
        const ::rtl::OUString& rRangeRepresentation)
            throw( lang::IllegalArgumentException, uno::RuntimeException)
{
    // FIXME: every call a new object?!? Or create hash of rRangeRepresentation?
    if ( pDocShell )
    {
        ScUnoGuard aGuard;
        ScRangeList aRangeList;
        USHORT nValid = aRangeList.Parse( rRangeRepresentation,
                pDocShell->GetDocument());
        if ( (nValid & SCA_VALID) == SCA_VALID )
        {
            // FIXME: add glue mechanism similar to ScChartArray::GlueState(),
            // for now this is a simple join
            ScRangeListRef xRanges = new ScRangeList;
            for ( ScRangePtr p = aRangeList.First(); p; p = aRangeList.Next())
            {
                xRanges->Join( *p );
            }
            return new ScChart2DataSource( pDocShell, xRanges);
        }
        throw lang::IllegalArgumentException();
    }
    throw uno::RuntimeException();
    return 0;
}


uno::Reference< chart2::XDataSequence> SAL_CALL
ScChart2DataProvider::getDataSequenceByRangeIdentifier(
        const ::rtl::OUString& rRangeIdentifier)
            throw( lang::IllegalArgumentException, uno::RuntimeException)
{
    // FIXME: find and return data sequence that matches rRangeIdentifier
    throw uno::RuntimeException();
    return 0;
}


uno::Reference< chart2::XDataSequence> SAL_CALL
ScChart2DataProvider::replaceRange(
        const uno::Reference< chart2::XDataSequence>& rSeq)
            throw( lang::IllegalArgumentException, uno::RuntimeException)
{
    // FIXME: real implementation
    throw uno::RuntimeException();
    return 0;
}


void SAL_CALL ScChart2DataProvider::addDataChangeListener(
        const uno::Reference< chart2::XDataChangeListener>& rListener,
        const uno::Reference< chart2::XDataSource>& rData)
            throw( uno::RuntimeException)
{
    // FIXME: real implementation, reuse ScChartListener
    throw uno::RuntimeException();
}


void SAL_CALL ScChart2DataProvider::removeDataChangeListener(
        const uno::Reference< chart2::XDataChangeListener>& rListener,
        const uno::Reference< chart2::XDataSource>& rData)
            throw( lang::IllegalArgumentException, uno::RuntimeException)
{
    // FIXME: real implementation, reuse ScChartListener
    throw uno::RuntimeException();
}


// DataSource ================================================================

ScChart2DataSource::ScChart2DataSource( ScDocShell* pDocSh,
        const ScRangeListRef& rRangeList)
    : pDocShell( pDocSh)
    , xRanges( rRangeList)
{
    if ( pDocShell )
        pDocShell->GetDocument()->AddUnoObject( *this);
}


ScChart2DataSource::~ScChart2DataSource()
{
    if ( pDocShell )
        pDocShell->GetDocument()->RemoveUnoObject( *this);
}


void ScChart2DataSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;
    }
}


uno::Sequence< uno::Reference< chart2::XDataSequence> > SAL_CALL
ScChart2DataSource::getDataSequences() throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    typedef ::std::vector< chart2::XDataSequence *> tVec;
    tVec aVec;
    // split into columns - FIXME: different if GlueState() is used
    for ( ScRangePtr p = xRanges->First(); p; p = xRanges->Next())
    {
        for ( SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
        {
            ScRangeListRef aColRanges = new ScRangeList;
            // one single sheet selected assumed for now
            aColRanges->Append( ScRange( nCol, p->aStart.Row(),
                        p->aStart.Tab(), nCol, p->aEnd.Row(),
                        p->aStart.Tab()));
            // TODO: create pure Numerical and Text sequences if possible
            aVec.push_back( new ScChart2DataSequence( pDocShell,
                        aColRanges));
        }
    }
    uno::Sequence< uno::Reference< chart2::XDataSequence> > aSequences(
            aVec.size());
    uno::Reference< chart2::XDataSequence> * pArr = aSequences.getArray();
    sal_Int32 j = 0;
    for ( tVec::const_iterator iSeq = aVec.begin(); iSeq != aVec.end();
            ++iSeq, ++j)
    {
        pArr[j] = *iSeq;
    }
    return aSequences;
}


// DataSequence ==============================================================

ScChart2DataSequence::ScChart2DataSequence( ScDocShell* pDocSh,
        const ScRangeListRef& rRangeList)
    : bHidden( sal_False)
    , xRanges( rRangeList)
    , pDocShell( pDocSh)
{
    if ( pDocShell )
        pDocShell->GetDocument()->AddUnoObject( *this);
    // FIXME: real implementation of identifier and it's mapping to ranges.
    // Reuse ScChartListener?
    aIdentifier = ::rtl::OUString::createFromAscii( "ScChart2DataSequence_dummy_ID_");
    static sal_Int32 nID = 0;
    aIdentifier += ::rtl::OUString::valueOf( ++nID);
}


ScChart2DataSequence::~ScChart2DataSequence()
{
    if ( pDocShell )
        pDocShell->GetDocument()->RemoveUnoObject( *this);
}


void ScChart2DataSequence::Notify( SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;
    }
}


uno::Sequence< uno::Any> SAL_CALL ScChart2DataSequence::getData()
            throw ( uno::RuntimeException)
{
    if ( !pDocShell)
        throw uno::RuntimeException();

    ScUnoGuard aGuard;
    const ScDocument* pDoc = pDocShell->GetDocument();
    sal_Int32 nCount = 0;
    ScRangePtr p;
    for ( p = xRanges->First(); p; p = xRanges->Next())
    {
        nCount += sal_Int32(p->aEnd.Col() - p->aStart.Col() + 1) *
            (p->aEnd.Row() - p->aStart.Row() + 1) * (p->aEnd.Tab() -
                                                     p->aStart.Tab() + 1);
    }
    uno::Sequence< uno::Any> aSeq( nCount);
    uno::Any * pArr = aSeq.getArray();
    nCount = 0;
    for ( p = xRanges->First(); p; p = xRanges->Next())
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
                                    pArr[nCount] <<= ::rtl::OUString(
                                            pCell->GetStringData());
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


::rtl::OUString SAL_CALL ScChart2DataSequence::getSourceIdentifier()
            throw ( uno::RuntimeException)
{
    return aIdentifier;
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2DataSequence::getPropertySetInfo() throw( uno::RuntimeException)
{
    // FIXME: real implementation
    throw uno::RuntimeException();
    return 0;
}


void SAL_CALL ScChart2DataSequence::setPropertyValue(
        const ::rtl::OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Role")))
    {
        if ( !(rValue >>= aRole))
            throw lang::IllegalArgumentException();
    }
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Hidden")))
    {
        if ( !(rValue >>= bHidden))
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
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Role")))
        aRet <<= aRole;
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Hidden")))
        aRet <<= bHidden;
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
    return aRet;
}


void SAL_CALL ScChart2DataSequence::addPropertyChangeListener(
        const ::rtl::OUString& rPropertyName,
        const uno::Reference< beans::XPropertyChangeListener>& xListener)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
    throw uno::RuntimeException();
}


void SAL_CALL ScChart2DataSequence::removePropertyChangeListener(
        const ::rtl::OUString& rPropertyName,
        const uno::Reference< beans::XPropertyChangeListener>& rListener)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
    throw uno::RuntimeException();
}


void SAL_CALL ScChart2DataSequence::addVetoableChangeListener(
        const ::rtl::OUString& rPropertyName,
        const uno::Reference< beans::XVetoableChangeListener>& rListener)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
    throw uno::RuntimeException();
}


void SAL_CALL ScChart2DataSequence::removeVetoableChangeListener(
        const ::rtl::OUString& rPropertyName,
        const uno::Reference< beans::XVetoableChangeListener>& rListener)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
    throw uno::RuntimeException();
}


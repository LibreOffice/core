/*************************************************************************
 *
 *  $RCSfile: funcuno.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-13 19:53:27 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <sfx2/app.hxx>
#include <svtools/itemprop.hxx>

#include "funcuno.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "unoguard.hxx"
#include "scdll.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "rangeseq.hxx"
#include "cell.hxx"
#include "docoptio.hxx"
#include "optuno.hxx"

// for lcl_CopyData:
#include "markdata.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  registered as implementation for service FunctionAccess,
//  also supports service SpreadsheetDocumentSettings (to set null date etc.)

#define SCFUNCTIONACCESS_SERVICE    "com.sun.star.sheet.FunctionAccess"
#define SCDOCSETTINGS_SERVICE       "com.sun.star.sheet.SpreadsheetDocumentSettings"

//------------------------------------------------------------------------

// helper to use cached document if not in use, temporary document otherwise

class ScTempDocSource
{
private:
    ScTempDocCache& rCache;
    ScDocument*     pTempDoc;

    static ScDocument*  CreateDocument();       // create and initialize doc

public:
                ScTempDocSource( ScTempDocCache& rDocCache );
                ~ScTempDocSource();

    ScDocument*     GetDocument();
};

//------------------------------------------------------------------------

// static
ScDocument* ScTempDocSource::CreateDocument()
{
    ScDocument* pDoc = new ScDocument;                  // SCDOCMODE_DOCUMENT
    pDoc->MakeTable( 0 );
    return pDoc;
}

ScTempDocSource::ScTempDocSource( ScTempDocCache& rDocCache ) :
    rCache( rDocCache ),
    pTempDoc( NULL )
{
    if ( rCache.IsInUse() )
        pTempDoc = CreateDocument();
    else
    {
        rCache.SetInUse( TRUE );
        if ( !rCache.GetDocument() )
            rCache.SetDocument( CreateDocument() );
    }
}

ScTempDocSource::~ScTempDocSource()
{
    if ( pTempDoc )
        delete pTempDoc;
    else
        rCache.SetInUse( FALSE );
}

ScDocument* ScTempDocSource::GetDocument()
{
    if ( pTempDoc )
        return pTempDoc;
    else
        return rCache.GetDocument();
}

//------------------------------------------------------------------------

ScTempDocCache::ScTempDocCache() :
    pDoc( NULL ),
    bInUse( FALSE )
{
}

ScTempDocCache::~ScTempDocCache()
{
    DBG_ASSERT( !bInUse, "ScTempDocCache dtor: bInUse" );
    delete pDoc;
}

void ScTempDocCache::SetDocument( ScDocument* pNew )
{
    DBG_ASSERT( !pDoc, "ScTempDocCache::SetDocument: already set" );
    pDoc = pNew;
}

void ScTempDocCache::Clear()
{
    DBG_ASSERT( !bInUse, "ScTempDocCache::Clear: bInUse" );
    delete pDoc;
    pDoc = NULL;
}

//------------------------------------------------------------------------

//  copy results from one document into another
//! merge this with ScAreaLink::Refresh
//! copy directly without a clipboard document?

BOOL lcl_CopyData( ScDocument* pSrcDoc, const ScRange& rSrcRange,
                    ScDocument* pDestDoc, const ScAddress& rDestPos )
{
    USHORT nSrcTab = rSrcRange.aStart.Tab();
    USHORT nDestTab = rDestPos.Tab();

    ScRange aNewRange( rDestPos, ScAddress(
                rSrcRange.aEnd.Col() - rSrcRange.aStart.Col() + rDestPos.Col(),
                rSrcRange.aEnd.Row() - rSrcRange.aStart.Row() + rDestPos.Row(),
                nDestTab ) );

    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
    ScMarkData aSourceMark;
    aSourceMark.SelectOneTable( nSrcTab );      // for CopyToClip
    aSourceMark.SetMarkArea( rSrcRange );
    pSrcDoc->CopyToClip( rSrcRange.aStart.Col(),rSrcRange.aStart.Row(),
                         rSrcRange.aEnd.Col(),rSrcRange.aEnd.Row(),
                         FALSE, pClipDoc, FALSE, &aSourceMark );

    if ( pClipDoc->HasAttrib( 0,0,nSrcTab, MAXCOL,MAXROW,nSrcTab,
                                HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {
        ScPatternAttr aPattern( pSrcDoc->GetPool() );
        aPattern.GetItemSet().Put( ScMergeAttr() );             // Defaults
        aPattern.GetItemSet().Put( ScMergeFlagAttr() );
        pClipDoc->ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nSrcTab, aPattern );
    }

    ScMarkData aDestMark;
    aDestMark.SelectOneTable( nDestTab );
    aDestMark.SetMarkArea( aNewRange );
    pDestDoc->CopyFromClip( aNewRange, aDestMark, IDF_ALL & ~IDF_FORMULA, NULL, pClipDoc, FALSE );

    delete pClipDoc;
    return TRUE;
}

//------------------------------------------------------------------------

ScFunctionAccess::ScFunctionAccess() :
    bInvalid( FALSE ),
    pOptions( NULL )
{
    StartListening( *SFX_APP() );       // for SFX_HINT_DEINITIALIZING
}

ScFunctionAccess::~ScFunctionAccess()
{
    delete pOptions;
}

void ScFunctionAccess::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA(SfxSimpleHint) &&
        ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DEINITIALIZING )
    {
        //  document must not be used anymore
        aDocCache.Clear();
        bInvalid = TRUE;
    }
}

// stuff for exService_...

uno::Reference<uno::XInterface> SAL_CALL ScFunctionAccess_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& )
{
    ScUnoGuard aGuard;
    SC_DLL()->Load();       // load module
    static uno::Reference< uno::XInterface > xInst = (::cppu::OWeakObject*) new ScFunctionAccess;
    return xInst;
}

rtl::OUString ScFunctionAccess::getImplementationName_Static()
{
    return rtl::OUString::createFromAscii( "stardiv.StarCalc.ScFunctionAccess" );
}

uno::Sequence<rtl::OUString> ScFunctionAccess::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCFUNCTIONACCESS_SERVICE );
    return aRet;
}

// XServiceInfo

rtl::OUString SAL_CALL ScFunctionAccess::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScFunctionAccess" );
}

sal_Bool SAL_CALL ScFunctionAccess::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr = rServiceName;
    return aServiceStr.EqualsAscii( SCFUNCTIONACCESS_SERVICE ) ||
           aServiceStr.EqualsAscii( SCDOCSETTINGS_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScFunctionAccess::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCFUNCTIONACCESS_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCDOCSETTINGS_SERVICE );
    return aRet;
}

// XPropertySet (document settings)

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScFunctionAccess::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( ScDocOptionsHelper::GetPropertyMap() );
    return aRef;
}

void SAL_CALL ScFunctionAccess::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !pOptions )
        pOptions = new ScDocOptions();

    // options aren't initialized from configuration - always get the same default behaviour

    BOOL bDone = ScDocOptionsHelper::setPropertyValue( *pOptions, aPropertyName, aValue );
    if (!bDone)
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScFunctionAccess::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( !pOptions )
        pOptions = new ScDocOptions();

    // options aren't initialized from configuration - always get the same default behaviour

    return ScDocOptionsHelper::getPropertyValue( *pOptions, aPropertyName );
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScFunctionAccess )

// XFunctionAccess

BOOL lcl_AddFunctionToken( ScTokenArray& rArray, const rtl::OUString& rName )
{
    // function names are always case-insensitive
    String aUpper( rName );
    ScGlobal::pCharClass->toUpper( aUpper );

    // same options as in ScCompiler::IsOpCode:
    // 1. built-in function name

    DBG_ASSERT( ScCompiler::pSymbolTableEnglish, "no symbol table" );
    if (!ScCompiler::pSymbolTableEnglish)
        return FALSE;

    for ( USHORT nPos=0; nPos<ScCompiler::nAnzStrings; nPos++ )
        if ( ScCompiler::pSymbolTableEnglish[nPos] == aUpper )
        {
            rArray.AddOpCode( (OpCode) nPos );
            return TRUE;
        }

    // 2. old add in functions

    USHORT nIndex;
    if ( ScGlobal::GetFuncCollection()->SearchFunc( aUpper, nIndex ) )
    {
        rArray.AddExternal( aUpper.GetBuffer() );
        return TRUE;
    }

    // 3. new (uno) add in functions

    String aIntName = ScGlobal::GetAddInCollection()->FindFunction( aUpper, FALSE );
    if (aIntName.Len())
    {
        rArray.AddExternal( aIntName.GetBuffer() );     // international name
        return TRUE;
    }

    return FALSE;       // no valid function name
}

void lcl_AddRef( ScTokenArray& rArray, long nStartRow, long nColCount, long nRowCount )
{
    ComplRefData aRef;
    aRef.InitFlags();
    aRef.Ref1.nTab = 0;
    aRef.Ref2.nTab = 0;
    aRef.Ref1.nCol = 0;
    aRef.Ref1.nRow = (USHORT) nStartRow;
    aRef.Ref2.nCol = (USHORT) (nColCount - 1);
    aRef.Ref2.nRow = (USHORT) (nStartRow + nRowCount - 1);
    rArray.AddDoubleReference(aRef);
}

uno::Any SAL_CALL ScFunctionAccess::callFunction( const rtl::OUString& aName,
                            const uno::Sequence<uno::Any>& aArguments )
                throw(container::NoSuchElementException, lang::IllegalArgumentException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (bInvalid)
        throw uno::RuntimeException();

    // use cached document if not in use, temporary document otherwise
    //  (deleted in ScTempDocSource dtor)
    ScTempDocSource aSource( aDocCache );
    ScDocument* pDoc = aSource.GetDocument();

    if (!ScCompiler::pSymbolTableEnglish)
    {
        ScCompiler aComp( pDoc, ScAddress() );
        aComp.SetCompileEnglish( TRUE );        // setup english symbol table
    }

    //
    //  find function
    //

    ScTokenArray aTokenArr;
    if ( !lcl_AddFunctionToken( aTokenArr, aName ) )
    {
        // function not found
        throw container::NoSuchElementException();
    }

    //
    //  set options (null date, etc.)
    //

    if ( pOptions )
        pDoc->SetDocOptions( *pOptions );

    //
    //  add arguments to token array
    //

    BOOL bArgErr = FALSE;
    BOOL bOverflow = FALSE;
    long nDocRow = 0;
    long nArgCount = aArguments.getLength();
    const uno::Any* pArgArr = aArguments.getConstArray();

    aTokenArr.AddOpCode(ocOpen);
    for (long nPos=0; nPos<nArgCount; nPos++)
    {
        if ( nPos > 0 )
            aTokenArr.AddOpCode(ocSep);

        const uno::Any& rArg = pArgArr[nPos];

        uno::TypeClass eClass = rArg.getValueTypeClass();
        uno::Type aType = rArg.getValueType();
        if ( eClass == uno::TypeClass_SHORT ||
                eClass == uno::TypeClass_LONG ||
                eClass == uno::TypeClass_DOUBLE )
        {
            double fVal;
            rArg >>= fVal;
            aTokenArr.AddDouble( fVal );
        }
        else if ( eClass == uno::TypeClass_STRING )
        {
            rtl::OUString aUStr;
            rArg >>= aUStr;
            String aStr( aUStr );
            aTokenArr.AddString( aStr.GetBuffer() );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<sal_Int16> > *)0 ) ) )
        {
            uno::Sequence< uno::Sequence<sal_Int16> > aRowSeq;
            rArg >>= aRowSeq;

            long nStartRow = nDocRow;
            long nMaxColCount = 0;
            long nRowCount = aRowSeq.getLength();
            const uno::Sequence<sal_Int16>* pRowArr = aRowSeq.getConstArray();
            for (long nRow=0; nRow<nRowCount; nRow++)
            {
                long nColCount = pRowArr[nRow].getLength();
                if ( nColCount > nMaxColCount )
                    nMaxColCount = nColCount;
                const sal_Int16* pColArr = pRowArr[nRow].getConstArray();
                for (long nCol=0; nCol<nColCount; nCol++)
                    if ( nCol <= MAXCOL && nDocRow <= MAXROW )
                        pDoc->SetValue( (USHORT) nCol, (USHORT) nDocRow, 0, pColArr[nCol] );
                    else
                        bOverflow = TRUE;
                ++nDocRow;
            }

            if ( nRowCount && nMaxColCount && !bOverflow )
                lcl_AddRef( aTokenArr, nStartRow, nMaxColCount, nRowCount );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<sal_Int32> > *)0 ) ) )
        {
            uno::Sequence< uno::Sequence<sal_Int32> > aRowSeq;
            rArg >>= aRowSeq;

            long nStartRow = nDocRow;
            long nMaxColCount = 0;
            long nRowCount = aRowSeq.getLength();
            const uno::Sequence<sal_Int32>* pRowArr = aRowSeq.getConstArray();
            for (long nRow=0; nRow<nRowCount; nRow++)
            {
                long nColCount = pRowArr[nRow].getLength();
                if ( nColCount > nMaxColCount )
                    nMaxColCount = nColCount;
                const sal_Int32* pColArr = pRowArr[nRow].getConstArray();
                for (long nCol=0; nCol<nColCount; nCol++)
                    if ( nCol <= MAXCOL && nDocRow <= MAXROW )
                        pDoc->SetValue( (USHORT) nCol, (USHORT) nDocRow, 0, pColArr[nCol] );
                    else
                        bOverflow = TRUE;
                ++nDocRow;
            }

            if ( nRowCount && nMaxColCount && !bOverflow )
                lcl_AddRef( aTokenArr, nStartRow, nMaxColCount, nRowCount );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<double> > *)0 ) ) )
        {
            uno::Sequence< uno::Sequence<double> > aRowSeq;
            rArg >>= aRowSeq;

            long nStartRow = nDocRow;
            long nMaxColCount = 0;
            long nRowCount = aRowSeq.getLength();
            const uno::Sequence<double>* pRowArr = aRowSeq.getConstArray();
            for (long nRow=0; nRow<nRowCount; nRow++)
            {
                long nColCount = pRowArr[nRow].getLength();
                if ( nColCount > nMaxColCount )
                    nMaxColCount = nColCount;
                const double* pColArr = pRowArr[nRow].getConstArray();
                for (long nCol=0; nCol<nColCount; nCol++)
                    if ( nCol <= MAXCOL && nDocRow <= MAXROW )
                        pDoc->SetValue( (USHORT) nCol, (USHORT) nDocRow, 0, pColArr[nCol] );
                    else
                        bOverflow = TRUE;
                ++nDocRow;
            }

            if ( nRowCount && nMaxColCount && !bOverflow )
                lcl_AddRef( aTokenArr, nStartRow, nMaxColCount, nRowCount );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<rtl::OUString> > *)0 ) ) )
        {
            uno::Sequence< uno::Sequence<rtl::OUString> > aRowSeq;
            rArg >>= aRowSeq;

            long nStartRow = nDocRow;
            long nMaxColCount = 0;
            long nRowCount = aRowSeq.getLength();
            const uno::Sequence<rtl::OUString>* pRowArr = aRowSeq.getConstArray();
            for (long nRow=0; nRow<nRowCount; nRow++)
            {
                long nColCount = pRowArr[nRow].getLength();
                if ( nColCount > nMaxColCount )
                    nMaxColCount = nColCount;
                const rtl::OUString* pColArr = pRowArr[nRow].getConstArray();
                for (long nCol=0; nCol<nColCount; nCol++)
                    if ( nCol <= MAXCOL && nDocRow <= MAXROW )
                    {
                        if ( pColArr[nCol].getLength() )
                            pDoc->PutCell( (USHORT) nCol, (USHORT) nDocRow, 0,
                                            new ScStringCell( pColArr[nCol] ) );
                    }
                    else
                        bOverflow = TRUE;
                ++nDocRow;
            }

            if ( nRowCount && nMaxColCount && !bOverflow )
                lcl_AddRef( aTokenArr, nStartRow, nMaxColCount, nRowCount );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<uno::Any> > *)0 ) ) )
        {
            uno::Sequence< uno::Sequence<uno::Any> > aRowSeq;
            rArg >>= aRowSeq;

            long nStartRow = nDocRow;
            long nMaxColCount = 0;
            long nRowCount = aRowSeq.getLength();
            const uno::Sequence<uno::Any>* pRowArr = aRowSeq.getConstArray();
            for (long nRow=0; nRow<nRowCount; nRow++)
            {
                long nColCount = pRowArr[nRow].getLength();
                if ( nColCount > nMaxColCount )
                    nMaxColCount = nColCount;
                const uno::Any* pColArr = pRowArr[nRow].getConstArray();
                for (long nCol=0; nCol<nColCount; nCol++)
                    if ( nCol <= MAXCOL && nDocRow <= MAXROW )
                    {
                        const uno::Any& rElement = pColArr[nCol];
                        uno::TypeClass eElemClass = rElement.getValueTypeClass();
                        if ( eElemClass == uno::TypeClass_VOID )
                        {
                            // leave empty
                        }
                        else if ( eElemClass == uno::TypeClass_SHORT ||
                                    eElemClass == uno::TypeClass_LONG ||
                                    eElemClass == uno::TypeClass_DOUBLE )
                        {
                            double fVal;
                            rElement >>= fVal;
                            pDoc->SetValue( (USHORT) nCol, (USHORT) nDocRow, 0, fVal );
                        }
                        else if ( eElemClass == uno::TypeClass_STRING )
                        {
                            rtl::OUString aUStr;
                            rElement >>= aUStr;
                            if ( aUStr.getLength() )
                                pDoc->PutCell( (USHORT) nCol, (USHORT) nDocRow, 0,
                                                            new ScStringCell( aUStr ) );
                        }
                        else
                            bArgErr = TRUE;     // invalid type
                    }
                    else
                        bOverflow = TRUE;
                ++nDocRow;
            }

            if ( nRowCount && nMaxColCount && !bOverflow )
                lcl_AddRef( aTokenArr, nStartRow, nMaxColCount, nRowCount );
        }
        else if ( aType.equals( getCppuType( (uno::Reference<table::XCellRange>*)0 ) ) )
        {
            // currently, only our own cell ranges are supported

            uno::Reference<table::XCellRange> xRange;
            rArg >>= xRange;
            ScCellRangesBase* pImpl = ScCellRangesBase::getImplementation( xRange );
            if ( pImpl )
            {
                ScDocument* pSrcDoc = pImpl->GetDocument();
                const ScRangeList& rRanges = pImpl->GetRangeList();
                if ( pSrcDoc && rRanges.Count() == 1 )
                {
                    ScRange aSrcRange = *rRanges.GetObject(0);

                    long nStartRow = nDocRow;
                    long nColCount = aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1;
                    long nRowCount = aSrcRange.aEnd.Row() - aSrcRange.aStart.Row() + 1;

                    if ( nStartRow + nRowCount > MAXROW )
                        bOverflow = TRUE;
                    else
                    {
                        // copy data
                        if ( !lcl_CopyData( pSrcDoc, aSrcRange, pDoc, ScAddress( 0, nDocRow, 0 ) ) )
                            bOverflow = TRUE;
                    }

                    nDocRow += nRowCount;
                    if ( !bOverflow )
                        lcl_AddRef( aTokenArr, nStartRow, nColCount, nRowCount );
                }
                else
                    bArgErr = TRUE;
            }
            else
                bArgErr = TRUE;
        }
        else
            bArgErr = TRUE;                 // invalid type
    }
    aTokenArr.AddOpCode(ocClose);
    aTokenArr.AddOpCode(ocStop);

    //
    //  execute formula
    //

    uno::Any aRet;
    if ( !bArgErr && !bOverflow && nDocRow <= MAXROW )
    {
        ScAddress aFormulaPos( 0, nDocRow, 0 );
        ScFormulaCell* pFormula = new ScFormulaCell( pDoc, aFormulaPos, &aTokenArr, MM_FORMULA );
        pDoc->PutCell( aFormulaPos, pFormula );     //! necessary?

        //  call GetMatrix before GetErrCode because GetMatrix always recalculates
        //  if there is no matrix result

        ScMatrix* pMat = NULL;
        pFormula->GetMatrix(&pMat);
        USHORT nErrCode = pFormula->GetErrCode();
        if ( nErrCode == 0 )
        {
            if ( pMat )
            {
                // array result
                ScRangeToSequence::FillMixedArray( aRet, pMat );
            }
            else if ( pFormula->IsValue() )
            {
                // numeric value
                aRet <<= (double) pFormula->GetValue();
            }
            else
            {
                // string result
                String aStrVal;
                pFormula->GetString( aStrVal );
                aRet <<= rtl::OUString( aStrVal );
            }
        }
        else if ( nErrCode == NOVALUE )
        {
            // #N/A: leave result empty, no exception
        }
        else
        {
            //  any other error: IllegalArgumentException
            bArgErr = TRUE;
        }

        pDoc->DeleteAreaTab( 0, 0, MAXCOL, MAXROW, 0, IDF_ALL );
    }

    if (bOverflow)
        throw uno::RuntimeException();

    if (bArgErr)
        throw lang::IllegalArgumentException();

    return aRet;
}



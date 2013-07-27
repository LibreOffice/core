/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/app.hxx>
#include <svl/itemprop.hxx>

#include "scitems.hxx"
#include "funcuno.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "scdll.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "formula/errorcodes.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "rangeseq.hxx"
#include "formulacell.hxx"
#include "docoptio.hxx"
#include "optuno.hxx"
#include <docuno.hxx>
// for lcl_CopyData:
#include "markdata.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "clipparam.hxx"
#include "dociter.hxx"
#include "stringutil.hxx"
#include "tokenarray.hxx"

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
        rCache.SetInUse( sal_True );
        if ( !rCache.GetDocument() )
            rCache.SetDocument( CreateDocument() );
    }
}

ScTempDocSource::~ScTempDocSource()
{
    if ( pTempDoc )
        delete pTempDoc;
    else
        rCache.SetInUse( false );
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
    bInUse( false )
{
}

ScTempDocCache::~ScTempDocCache()
{
    OSL_ENSURE( !bInUse, "ScTempDocCache dtor: bInUse" );
    delete pDoc;
}

void ScTempDocCache::SetDocument( ScDocument* pNew )
{
    OSL_ENSURE( !pDoc, "ScTempDocCache::SetDocument: already set" );
    pDoc = pNew;
}

void ScTempDocCache::Clear()
{
    OSL_ENSURE( !bInUse, "ScTempDocCache::Clear: bInUse" );
    delete pDoc;
    pDoc = NULL;
}

//------------------------------------------------------------------------

//  copy results from one document into another
//! merge this with ScAreaLink::Refresh
//! copy directly without a clipboard document?

static sal_Bool lcl_CopyData( ScDocument* pSrcDoc, const ScRange& rSrcRange,
                    ScDocument* pDestDoc, const ScAddress& rDestPos )
{
    SCTAB nSrcTab = rSrcRange.aStart.Tab();
    SCTAB nDestTab = rDestPos.Tab();

    ScRange aNewRange( rDestPos, ScAddress(
                rSrcRange.aEnd.Col() - rSrcRange.aStart.Col() + rDestPos.Col(),
                rSrcRange.aEnd.Row() - rSrcRange.aStart.Row() + rDestPos.Row(),
                nDestTab ) );

    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
    ScMarkData aSourceMark;
    aSourceMark.SelectOneTable( nSrcTab );      // for CopyToClip
    aSourceMark.SetMarkArea( rSrcRange );
    ScClipParam aClipParam(rSrcRange, false);
    pSrcDoc->CopyToClip(aClipParam, pClipDoc, &aSourceMark, false);

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
    pDestDoc->CopyFromClip( aNewRange, aDestMark, IDF_ALL & ~IDF_FORMULA, NULL, pClipDoc, false );

    delete pClipDoc;
    return sal_True;
}

//------------------------------------------------------------------------

ScFunctionAccess::ScFunctionAccess() :
    pOptions( NULL ),
    aPropertyMap( ScDocOptionsHelper::GetPropertyMap() ),
    mbArray( true ),    // default according to behaviour of older Office versions
    mbValid( true )
{
    StartListening( *SFX_APP() );       // for SFX_HINT_DEINITIALIZING
}

ScFunctionAccess::~ScFunctionAccess()
{
    delete pOptions;
}

void ScFunctionAccess::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA(SfxSimpleHint) &&
        ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DEINITIALIZING )
    {
        //  document must not be used anymore
        aDocCache.Clear();
        mbValid = false;
    }
}

// stuff for exService_...

uno::Reference<uno::XInterface> SAL_CALL ScFunctionAccess_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& )
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    static uno::Reference< uno::XInterface > xInst((::cppu::OWeakObject*) new ScFunctionAccess);
    return xInst;
}

OUString ScFunctionAccess::getImplementationName_Static()
{
    return OUString( "stardiv.StarCalc.ScFunctionAccess" );
}

uno::Sequence<OUString> ScFunctionAccess::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( SCFUNCTIONACCESS_SERVICE );
    return aRet;
}

// XServiceInfo

OUString SAL_CALL ScFunctionAccess::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( "ScFunctionAccess");
}

sal_Bool SAL_CALL ScFunctionAccess::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr(rServiceName);
    return aServiceStr.EqualsAscii( SCFUNCTIONACCESS_SERVICE ) ||
           aServiceStr.EqualsAscii( SCDOCSETTINGS_SERVICE );
}

uno::Sequence<OUString> SAL_CALL ScFunctionAccess::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( SCFUNCTIONACCESS_SERVICE );
    pArray[1] = OUString( SCDOCSETTINGS_SERVICE );
    return aRet;
}

// XPropertySet (document settings)

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScFunctionAccess::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropertyMap ));
    return aRef;
}

void SAL_CALL ScFunctionAccess::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( aPropertyName == "IsArrayFunction" )
    {
        if( !(aValue >>= mbArray) )
            throw lang::IllegalArgumentException();
    }
    else
    {
        if ( !pOptions )
            pOptions = new ScDocOptions();

        // options aren't initialized from configuration - always get the same default behaviour

        sal_Bool bDone = ScDocOptionsHelper::setPropertyValue( *pOptions, aPropertyMap, aPropertyName, aValue );
        if (!bDone)
            throw beans::UnknownPropertyException();
    }
}

uno::Any SAL_CALL ScFunctionAccess::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( aPropertyName == "IsArrayFunction" )
        return uno::Any( mbArray );

    if ( !pOptions )
        pOptions = new ScDocOptions();

    // options aren't initialized from configuration - always get the same default behaviour

    return ScDocOptionsHelper::getPropertyValue( *pOptions, aPropertyMap, aPropertyName );
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScFunctionAccess )

// XFunctionAccess

static sal_Bool lcl_AddFunctionToken( ScTokenArray& rArray, const OUString& rName,const ScCompiler& rCompiler )
{
    // function names are always case-insensitive
    OUString aUpper = ScGlobal::pCharClass->uppercase(rName);

    // same options as in ScCompiler::IsOpCode:
    // 1. built-in function name

    OpCode eOp = rCompiler.GetEnglishOpCode( aUpper );
    if ( eOp != ocNone )
    {
        rArray.AddOpCode( eOp );
        return true;
    }

    // 2. old add in functions

    if (ScGlobal::GetFuncCollection()->findByName(aUpper))
    {
        rArray.AddExternal(aUpper.getStr());
        return true;
    }

    // 3. new (uno) add in functions

    OUString aIntName =
        ScGlobal::GetAddInCollection()->FindFunction(aUpper, false);
    if (!aIntName.isEmpty())
    {
        rArray.AddExternal(aIntName.getStr());     // international name
        return true;
    }

    return false;       // no valid function name
}

static void lcl_AddRef( ScTokenArray& rArray, long nStartRow, long nColCount, long nRowCount )
{
    ScComplexRefData aRef;
    aRef.InitRange(ScRange(0,nStartRow,0,nColCount-1,nStartRow+nRowCount-1,0));
    rArray.AddDoubleReference(aRef);
}

class SimpleVisitor
{
protected:
    bool mbArgError;
    ScDocument* mpDoc;
public:
    SimpleVisitor( ScDocument* pDoc ) : mbArgError( false ), mpDoc( pDoc ) {}
    // could possibly just get away with JUST the following overload
    // 1) virtual void visitElem( long& nCol, long& nRow, const double& elem )
    // 2) virtual void visitElem( long& nCol, long& nRow, const OUString& elem )
    // 3) virtual void visitElem( long& nCol, long& nRow, const uno::Any& elem )
    // the other types methods are here just to reflect the orig code and for
    // completeness.

    void visitElem( long nCol, long nRow, const sal_Int16& elem )
    {
        mpDoc->SetValue( (SCCOL) nCol, (SCROW) nRow, 0, elem );
    }
    void visitElem( long nCol, long nRow, const sal_Int32& elem )
    {
        mpDoc->SetValue( (SCCOL) nCol, (SCROW) nRow, 0, elem );
    }
    void visitElem( long nCol, long nRow, const double& elem )
    {
        mpDoc->SetValue( (SCCOL) nCol, (SCROW) nRow, 0, elem );
    }
    void visitElem( long nCol, long nRow, const OUString& elem )
    {
        if (!elem.isEmpty())
        {
            ScSetStringParam aParam;
            aParam.setTextInput();
            mpDoc->SetString(ScAddress(nCol,nRow,0), elem, &aParam);
        }
    }
    void visitElem( long nCol, long nRow, const uno::Any& rElement )
    {
        uno::TypeClass eElemClass = rElement.getValueTypeClass();
        if ( eElemClass == uno::TypeClass_VOID )
        {
            // leave empty
        }
        else if ( eElemClass == uno::TypeClass_BYTE ||
                    eElemClass == uno::TypeClass_SHORT ||
                    eElemClass == uno::TypeClass_UNSIGNED_SHORT ||
                    eElemClass == uno::TypeClass_LONG ||
                    eElemClass == uno::TypeClass_UNSIGNED_LONG ||
                    eElemClass == uno::TypeClass_FLOAT ||
                    eElemClass == uno::TypeClass_DOUBLE )
        {
            //  accept integer types because Basic passes a floating point
            //  variable as byte, short or long if it's an integer number.
            double fVal(0.0);
            rElement >>= fVal;
            visitElem( nCol, nRow, fVal );
        }
        else if ( eElemClass == uno::TypeClass_STRING )
        {
            OUString aUStr;
            rElement >>= aUStr;
            visitElem( nCol, nRow, aUStr );
        }
        else
            mbArgError = true;
    }
    bool hasArgError() const { return mbArgError; }
};

template< class seq >
class SequencesContainer
{
    uno::Sequence< uno::Sequence< seq > > maSeq;

    long& mrDocRow;
    bool mbOverflow;
    bool mbArgError;
    ScDocument* mpDoc;
    ScTokenArray& mrTokenArr;

public:
    SequencesContainer( const uno::Any& rArg, ScTokenArray& rTokenArr, long& rDocRow, ScDocument* pDoc ) :
        mrDocRow( rDocRow ), mbOverflow(false), mbArgError(false), mpDoc( pDoc ), mrTokenArr( rTokenArr )
    {
        rArg >>= maSeq;
    }

    void process()
    {
        SimpleVisitor aVisitor(mpDoc);
        long nStartRow = mrDocRow;
        long nRowCount = maSeq.getLength();
        long nMaxColCount = 0;
        const uno::Sequence< seq >* pRowArr = maSeq.getConstArray();
        for ( long nRow=0; nRow<nRowCount; nRow++ )
        {
            long nColCount = pRowArr[nRow].getLength();
            if ( nColCount > nMaxColCount )
                nMaxColCount = nColCount;
            const seq* pColArr = pRowArr[nRow].getConstArray();
            for (long nCol=0; nCol<nColCount; nCol++)
                if ( nCol <= MAXCOL && mrDocRow <= MAXROW )
                    aVisitor.visitElem( nCol, mrDocRow, pColArr[ nCol ] );
                else
                    mbOverflow=true;
            mrDocRow++;
        }
        mbArgError = aVisitor.hasArgError();
        if ( nRowCount && nMaxColCount && !mbOverflow )
            lcl_AddRef( mrTokenArr, nStartRow, nMaxColCount, nRowCount );
    }
    bool getOverflow() const { return mbOverflow; }
    bool getArgError() const { return mbArgError; }
};

template <class T>
class ArrayOfArrayProc
{
public:
static void processSequences( ScDocument* pDoc, const uno::Any& rArg, ScTokenArray& rTokenArr,
                                long& rDocRow, sal_Bool& rArgErr, sal_Bool& rOverflow )
{
    SequencesContainer< T > aContainer( rArg, rTokenArr, rDocRow, pDoc );
    aContainer.process();
    rArgErr = aContainer.getArgError();
    rOverflow = aContainer.getOverflow();
}
};

uno::Any SAL_CALL ScFunctionAccess::callFunction( const OUString& aName,
                            const uno::Sequence<uno::Any>& aArguments )
                throw(container::NoSuchElementException, lang::IllegalArgumentException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!mbValid)
        throw uno::RuntimeException();

    // use cached document if not in use, temporary document otherwise
    //  (deleted in ScTempDocSource dtor)
    ScTempDocSource aSource( aDocCache );
    ScDocument* pDoc = aSource.GetDocument();
    const static SCTAB nTempSheet = 1;
    // Create an extra tab to contain the Function Cell
    // this will allow full rows to be used.
    if ( !pDoc->HasTable( nTempSheet ) )
        pDoc->MakeTable( nTempSheet );

    /// TODO: check
    ScAddress aAdr;
    ScCompiler aCompiler(pDoc,aAdr);
    aCompiler.SetGrammar(pDoc->GetGrammar());

    //
    //  find function
    //

    ScTokenArray aTokenArr;
    if ( !lcl_AddFunctionToken( aTokenArr, aName,aCompiler ) )
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

    sal_Bool bArgErr = false;
    sal_Bool bOverflow = false;
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
        if ( eClass == uno::TypeClass_BYTE ||
             eClass == uno::TypeClass_BOOLEAN ||
             eClass == uno::TypeClass_SHORT ||
             eClass == uno::TypeClass_UNSIGNED_SHORT ||
             eClass == uno::TypeClass_LONG ||
             eClass == uno::TypeClass_UNSIGNED_LONG ||
             eClass == uno::TypeClass_FLOAT ||
             eClass == uno::TypeClass_DOUBLE )
        {
            //  accept integer types because Basic passes a floating point
            //  variable as byte, short or long if it's an integer number.
            double fVal = 0;
            rArg >>= fVal;
            aTokenArr.AddDouble( fVal );
        }
        else if ( eClass == uno::TypeClass_STRING )
        {
            OUString aUStr;
            rArg >>= aUStr;
            String aStr( aUStr );
            aTokenArr.AddString( aStr.GetBuffer() );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<sal_Int16> > *)0 ) ) )
        {
            ArrayOfArrayProc<sal_Int16>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<sal_Int32> > *)0 ) ) )
        {
            ArrayOfArrayProc<sal_Int32>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<double> > *)0 ) ) )
        {
            ArrayOfArrayProc<double>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<OUString> > *)0 ) ) )
        {
            ArrayOfArrayProc<OUString>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( getCppuType( (uno::Sequence< uno::Sequence<uno::Any> > *)0 ) ) )
        {
            ArrayOfArrayProc<uno::Any>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( getCppuType( (uno::Reference<table::XCellRange>*)0 ) ) )
        {
            // currently, only our own cell ranges are supported

            uno::Reference<table::XCellRange> xRange(rArg, uno::UNO_QUERY);
            ScCellRangesBase* pImpl = ScCellRangesBase::getImplementation( xRange );
            if ( pImpl )
            {
                ScDocument* pSrcDoc = pImpl->GetDocument();
                const ScRangeList& rRanges = pImpl->GetRangeList();
                if ( pSrcDoc && rRanges.size() == 1 )
                {
                    ScRange aSrcRange = *rRanges[ 0 ];

                    long nStartRow = nDocRow;
                    long nColCount = aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1;
                    long nRowCount = aSrcRange.aEnd.Row() - aSrcRange.aStart.Row() + 1;

                    if ( nStartRow + nRowCount > MAXROWCOUNT )
                        bOverflow = sal_True;
                    else
                    {
                        // copy data
                        if ( !lcl_CopyData( pSrcDoc, aSrcRange, pDoc, ScAddress( 0, (SCROW)nDocRow, 0 ) ) )
                            bOverflow = sal_True;
                    }

                    nDocRow += nRowCount;
                    if ( !bOverflow )
                        lcl_AddRef( aTokenArr, nStartRow, nColCount, nRowCount );
                }
                else
                    bArgErr = sal_True;
            }
            else
                bArgErr = sal_True;
        }
        else
            bArgErr = sal_True;                 // invalid type
    }
    aTokenArr.AddOpCode(ocClose);
    aTokenArr.AddOpCode(ocStop);

    //
    //  execute formula
    //

    uno::Any aRet;
    if ( !bArgErr && !bOverflow && nDocRow <= MAXROWCOUNT )
    {
        ScAddress aFormulaPos( 0, 0, nTempSheet );
        // GRAM_PODF_A1 doesn't really matter for the token array but fits with
        // other API compatibility grammars.
        ScFormulaCell* pFormula = new ScFormulaCell( pDoc, aFormulaPos,
                &aTokenArr, formula::FormulaGrammar::GRAM_PODF_A1, (sal_uInt8)(mbArray ? MM_FORMULA : MM_NONE) );
        pFormula = pDoc->SetFormulaCell(aFormulaPos, pFormula);

        //  call GetMatrix before GetErrCode because GetMatrix always recalculates
        //  if there is no matrix result

        const ScMatrix* pMat = (mbArray && pFormula) ? pFormula->GetMatrix() : 0;
        sal_uInt16 nErrCode = pFormula ? pFormula->GetErrCode() : errIllegalArgument;
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
                OUString aStrVal = pFormula->GetString();
                aRet <<= aStrVal;
            }
        }
        else if ( nErrCode == NOTAVAILABLE )
        {
            // #N/A: leave result empty, no exception
        }
        else
        {
            //  any other error: IllegalArgumentException
            bArgErr = sal_True;
        }

        pDoc->DeleteAreaTab( 0, 0, MAXCOL, MAXROW, 0, IDF_ALL );
        pDoc->DeleteAreaTab( 0, 0, 0, 0, nTempSheet, IDF_ALL );
    }

    if (bOverflow)
        throw uno::RuntimeException();

    if (bArgErr)
        throw lang::IllegalArgumentException();

    return aRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

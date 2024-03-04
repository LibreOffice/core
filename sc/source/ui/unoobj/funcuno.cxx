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

#include <cppuhelper/supportsservice.hxx>
#include <sfx2/app.hxx>
#include <svl/itemprop.hxx>
#include <svl/sharedstringpool.hxx>
#include <unotools/charclass.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>

#include <funcuno.hxx>
#include <miscuno.hxx>
#include <cellsuno.hxx>
#include <scdll.hxx>
#include <document.hxx>
#include <compiler.hxx>
#include <formula/errorcodes.hxx>
#include <callform.hxx>
#include <addincol.hxx>
#include <rangeseq.hxx>
#include <formulacell.hxx>
#include <docoptio.hxx>
#include <optuno.hxx>
#include <markdata.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <attrib.hxx>
#include <clipparam.hxx>
#include <stringutil.hxx>
#include <tokenarray.hxx>
#include <unonames.hxx>
#include <memory>

using namespace com::sun::star;

//  registered as implementation for service FunctionAccess,
//  also supports service SpreadsheetDocumentSettings (to set null date etc.)

constexpr OUString SCFUNCTIONACCESS_SERVICE = u"com.sun.star.sheet.FunctionAccess"_ustr;
constexpr OUString SCDOCSETTINGS_SERVICE = u"com.sun.star.sheet.SpreadsheetDocumentSettings"_ustr;

// helper to use cached document if not in use, temporary document otherwise

namespace {

class ScTempDocSource
{
private:
    ScTempDocCache& rCache;
    ScDocumentUniquePtr pTempDoc;

    static ScDocument*  CreateDocument();       // create and initialize doc

public:
    explicit ScTempDocSource( ScTempDocCache& rDocCache );
    ~ScTempDocSource() COVERITY_NOEXCEPT_FALSE;

    ScDocument*     GetDocument();
};

}

ScDocument* ScTempDocSource::CreateDocument()
{
    ScDocument* pDoc = new ScDocument( SCDOCMODE_FUNCTIONACCESS );
    pDoc->MakeTable( 0 );
    return pDoc;
}

ScTempDocSource::ScTempDocSource( ScTempDocCache& rDocCache ) :
    rCache( rDocCache )
{
    if ( rCache.IsInUse() )
        pTempDoc.reset(CreateDocument());
    else
    {
        rCache.SetInUse( true );
        if ( !rCache.GetDocument() )
            rCache.SetDocument( CreateDocument() );
    }
}

ScTempDocSource::~ScTempDocSource() COVERITY_NOEXCEPT_FALSE
{
    if ( !pTempDoc )
        rCache.SetInUse( false );
}

ScDocument* ScTempDocSource::GetDocument()
{
    if ( pTempDoc )
        return pTempDoc.get();
    else
        return rCache.GetDocument();
}

ScTempDocCache::ScTempDocCache()
    : bInUse(false)
{
}

void ScTempDocCache::SetDocument( ScDocument* pNew )
{
    OSL_ENSURE(!xDoc, "ScTempDocCache::SetDocument: already set");
    xDoc.reset(pNew);
}

void ScTempDocCache::Clear()
{
    OSL_ENSURE( !bInUse, "ScTempDocCache::Clear: bInUse" );
    xDoc.reset();
}

//  copy results from one document into another
//! merge this with ScAreaLink::Refresh
//! copy directly without a clipboard document?

static bool lcl_CopyData( ScDocument* pSrcDoc, const ScRange& rSrcRange,
                    ScDocument* pDestDoc, const ScAddress& rDestPos )
{
    SCTAB nSrcTab = rSrcRange.aStart.Tab();
    SCTAB nDestTab = rDestPos.Tab();

    ScRange aNewRange( rDestPos, ScAddress(
                rSrcRange.aEnd.Col() - rSrcRange.aStart.Col() + rDestPos.Col(),
                rSrcRange.aEnd.Row() - rSrcRange.aStart.Row() + rDestPos.Row(),
                nDestTab ) );

    ScDocumentUniquePtr pClipDoc(new ScDocument( SCDOCMODE_CLIP ));
    ScMarkData aSourceMark(pSrcDoc->GetSheetLimits());
    aSourceMark.SelectOneTable( nSrcTab );      // for CopyToClip
    aSourceMark.SetMarkArea( rSrcRange );
    ScClipParam aClipParam(rSrcRange, false);
    pSrcDoc->CopyToClip(aClipParam, pClipDoc.get(), &aSourceMark, false, false);

    if ( pClipDoc->HasAttrib( 0,0,nSrcTab, pClipDoc->MaxCol(), pClipDoc->MaxRow(),nSrcTab,
                                HasAttrFlags::Merged | HasAttrFlags::Overlapped ) )
    {
        ScPatternAttr aPattern(pSrcDoc->getCellAttributeHelper());
        aPattern.GetItemSet().Put( ScMergeAttr() );             // Defaults
        aPattern.GetItemSet().Put( ScMergeFlagAttr() );
        pClipDoc->ApplyPatternAreaTab( 0,0, pClipDoc->MaxCol(), pClipDoc->MaxRow(), nSrcTab, aPattern );
    }

    ScMarkData aDestMark(pDestDoc->GetSheetLimits());
    aDestMark.SelectOneTable( nDestTab );
    aDestMark.SetMarkArea( aNewRange );
    pDestDoc->CopyFromClip( aNewRange, aDestMark, InsertDeleteFlags::ALL & ~InsertDeleteFlags::FORMULA, nullptr, pClipDoc.get(), false );

    return true;
}

namespace
{
    std::span<const SfxItemPropertyMapEntry> GetPropertyMap()
    {
        static const SfxItemPropertyMapEntry aMap[] =
        {
            { SC_UNO_CALCASSHOWN,  PROP_UNO_CALCASSHOWN ,  cppu::UnoType<bool>::get(),          0, 0},
            { SC_UNO_DEFTABSTOP,   PROP_UNO_DEFTABSTOP  ,  cppu::UnoType<sal_Int16>::get(),    0, 0},
            { SC_UNO_IGNORECASE,   PROP_UNO_IGNORECASE  ,  cppu::UnoType<bool>::get(),          0, 0},
            { SC_UNO_ITERENABLED,  PROP_UNO_ITERENABLED ,  cppu::UnoType<bool>::get(),          0, 0},
            { SC_UNO_ITERCOUNT,    PROP_UNO_ITERCOUNT   ,  cppu::UnoType<sal_Int32>::get(),    0, 0},
            { SC_UNO_ITEREPSILON,  PROP_UNO_ITEREPSILON ,  cppu::UnoType<double>::get(),       0, 0},
            { SC_UNO_LOOKUPLABELS, PROP_UNO_LOOKUPLABELS,  cppu::UnoType<bool>::get(),          0, 0},
            { SC_UNO_MATCHWHOLE,   PROP_UNO_MATCHWHOLE  ,  cppu::UnoType<bool>::get(),          0, 0},
            { SC_UNO_NULLDATE,     PROP_UNO_NULLDATE    ,  cppu::UnoType<util::Date>::get(),   0, 0},
            // SpreadsheetDocumentSettings supports "SpellOnline" so we must claim to support this here too
            { SC_UNO_SPELLONLINE,  0 ,                     cppu::UnoType<bool>::get(),          0, 0},
            { SC_UNO_STANDARDDEC,  PROP_UNO_STANDARDDEC ,  cppu::UnoType<sal_Int16>::get(),    0, 0},
            { SC_UNO_REGEXENABLED, PROP_UNO_REGEXENABLED,  cppu::UnoType<bool>::get(),          0, 0},
            { SC_UNO_WILDCARDSENABLED, PROP_UNO_WILDCARDSENABLED, cppu::UnoType<bool>::get(),  0, 0},
        };
        return aMap;
    }
}

ScFunctionAccess::ScFunctionAccess() :
    aPropertyMap( GetPropertyMap() ),
    mbArray( true ),    // default according to behaviour of older Office versions
    mbValid( true ),
    mbSpellOnline( false )
{
    StartListening( *SfxGetpApp() );       // for SfxHintId::Deinitializing
}

ScFunctionAccess::~ScFunctionAccess()
{
    pOptions.reset();
    {
        // SfxBroadcaster::RemoveListener checks DBG_TESTSOLARMUTEX():
        SolarMutexGuard g;
        EndListeningAll();
    }
}

void ScFunctionAccess::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Deinitializing )
    {
        //  document must not be used anymore
        aDocCache.Clear();
        mbValid = false;
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ScFunctionAccess_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return cppu::acquire(new ScFunctionAccess);
}

// XServiceInfo
OUString SAL_CALL ScFunctionAccess::getImplementationName()
{
    return "stardiv.StarCalc.ScFunctionAccess";
}

sal_Bool SAL_CALL ScFunctionAccess::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScFunctionAccess::getSupportedServiceNames()
{
    return {SCFUNCTIONACCESS_SERVICE, SCDOCSETTINGS_SERVICE};
}

// XPropertySet (document settings)

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScFunctionAccess::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropertyMap ));
    return aRef;
}

void SAL_CALL ScFunctionAccess::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if ( aPropertyName == "IsArrayFunction" )
    {
        if( !(aValue >>= mbArray) )
            throw lang::IllegalArgumentException();
    }
    else if (aPropertyName == SC_UNO_SPELLONLINE)
    {
        // Allow this property because SpreadsheetDocumentSettings lists it
        if( !(aValue >>= mbSpellOnline) )
            throw lang::IllegalArgumentException();
    }
    else
    {
        if ( !pOptions )
            pOptions.reset( new ScDocOptions() );

        // options aren't initialized from configuration - always get the same default behaviour
        bool bDone = ScDocOptionsHelper::setPropertyValue( *pOptions, aPropertyMap, aPropertyName, aValue );
        if (!bDone)
            throw beans::UnknownPropertyException(aPropertyName);
    }
}

uno::Any SAL_CALL ScFunctionAccess::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    if ( aPropertyName == "IsArrayFunction" )
        return uno::Any( mbArray );

    // Allow this property because SpreadsheetDocumentSettings lists it
    if (aPropertyName == SC_UNO_SPELLONLINE)
        return uno::Any(mbSpellOnline);

    if ( !pOptions )
        pOptions.reset( new ScDocOptions() );

    // options aren't initialized from configuration - always get the same default behaviour

    return ScDocOptionsHelper::getPropertyValue( *pOptions, aPropertyMap, aPropertyName );
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScFunctionAccess )

// XFunctionAccess

static bool lcl_AddFunctionToken( ScTokenArray& rArray, const OUString& rName,const ScCompiler& rCompiler )
{
    // function names are always case-insensitive
    OUString aUpper = ScGlobal::getCharClass().uppercase(rName);

    // same options as in ScCompiler::IsOpCode:
    // 1. built-in function name

    OpCode eOp = rCompiler.GetEnglishOpCode( aUpper );
    if ( eOp != ocNone )
    {
        rArray.AddOpCode( eOp );
        return true;
    }

    // 2. old add in functions

    if (ScGlobal::GetLegacyFuncCollection()->findByName(aUpper))
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

static void lcl_AddRef( ScTokenArray& rArray, sal_Int32 nStartRow, sal_Int32 nColCount, sal_Int32 nRowCount )
{
    ScComplexRefData aRef;
    aRef.InitRange(ScRange(0,nStartRow,0,nColCount-1,nStartRow+nRowCount-1,0));
    rArray.AddDoubleReference(aRef);
}

namespace {

class SimpleVisitor
{
protected:
    bool mbArgError;
    ScDocument* mpDoc;
public:
    explicit SimpleVisitor( ScDocument* pDoc ) : mbArgError( false ), mpDoc( pDoc ) {}
    // could possibly just get away with JUST the following overload
    // 1) virtual void visitElem( long& nCol, long& nRow, const double& elem )
    // 2) virtual void visitElem( long& nCol, long& nRow, const OUString& elem )
    // 3) virtual void visitElem( long& nCol, long& nRow, const uno::Any& elem )
    // the other types methods are here just to reflect the orig code and for
    // completeness.

    void visitElem( sal_Int32 nCol, sal_Int32 nRow, sal_Int16 elem )
    {
        mpDoc->SetValue( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), 0, elem );
    }
    void visitElem( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 elem )
    {
        mpDoc->SetValue( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), 0, elem );
    }
    void visitElem( sal_Int32 nCol, sal_Int32 nRow, const double& elem )
    {
        mpDoc->SetValue( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), 0, elem );
    }
    void visitElem( sal_Int32 nCol, sal_Int32 nRow, const OUString& elem )
    {
        if (!elem.isEmpty())
        {
            ScSetStringParam aParam;
            aParam.setTextInput();
            mpDoc->SetString(ScAddress(nCol,nRow,0), elem, &aParam);
        }
    }
    void visitElem( sal_Int32 nCol, sal_Int32 nRow, const uno::Any& rElement )
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

    sal_Int32& mrDocRow;
    bool mbOverflow;
    bool mbArgError;
    ScDocument* mpDoc;
    ScTokenArray& mrTokenArr;

public:
    SequencesContainer( const uno::Any& rArg, ScTokenArray& rTokenArr, sal_Int32& rDocRow, ScDocument* pDoc ) :
        mrDocRow( rDocRow ), mbOverflow(false), mbArgError(false), mpDoc( pDoc ), mrTokenArr( rTokenArr )
    {
        rArg >>= maSeq;
    }

    void process()
    {
        SimpleVisitor aVisitor(mpDoc);
        sal_Int32 nStartRow = mrDocRow;
        sal_Int32 nRowCount = maSeq.getLength();
        sal_Int32 nMaxColCount = 0;
        for (const uno::Sequence<seq>& rRow : maSeq)
        {
            sal_Int32 nColCount = rRow.getLength();
            if ( nColCount > nMaxColCount )
                nMaxColCount = nColCount;
            for (sal_Int32 nCol=0; nCol<nColCount; nCol++)
                if ( nCol <= mpDoc->MaxCol() && mrDocRow <= mpDoc->MaxRow() )
                    aVisitor.visitElem( nCol, mrDocRow, rRow[ nCol ] );
                else
                    mbOverflow=true;
            mrDocRow++;
        }
        mbArgError = aVisitor.hasArgError();
        if (!mbOverflow)
        {
            if (nRowCount && nMaxColCount)
                lcl_AddRef( mrTokenArr, nStartRow, nMaxColCount, nRowCount );
            else if (nRowCount == 1 && !nMaxColCount)
                // Empty Sequence<Sequence<Any>> is omitted argument.
                mrTokenArr.AddOpCode( ocMissing);
        }
    }
    bool getOverflow() const { return mbOverflow; }
    bool getArgError() const { return mbArgError; }
};

template <class T>
class ArrayOfArrayProc
{
public:
static void processSequences( ScDocument* pDoc, const uno::Any& rArg, ScTokenArray& rTokenArr,
                                sal_Int32& rDocRow, bool& rArgErr, bool& rOverflow )
{
    SequencesContainer< T > aContainer( rArg, rTokenArr, rDocRow, pDoc );
    aContainer.process();
    rArgErr = aContainer.getArgError();
    rOverflow = aContainer.getOverflow();
}
};

}

uno::Any SAL_CALL ScFunctionAccess::callFunction( const OUString& aName,
                            const uno::Sequence<uno::Any>& aArguments )
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
    ScCompiler aCompiler(*pDoc, aAdr, pDoc->GetGrammar());

    //  find function

    ScTokenArray aTokenArr(*pDoc);
    if ( !lcl_AddFunctionToken( aTokenArr, aName,aCompiler ) )
    {
        // function not found
        throw container::NoSuchElementException();
    }

    //  set options (null date, etc.)

    if ( pOptions )
        pDoc->SetDocOptions( *pOptions );

    //  add arguments to token array

    bool bArgErr = false;
    bool bOverflow = false;
    sal_Int32 nDocRow = 0;
    tools::Long nArgCount = aArguments.getLength();
    const uno::Any* pArgArr = aArguments.getConstArray();

    svl::SharedStringPool& rSPool = pDoc->GetSharedStringPool();
    aTokenArr.AddOpCode(ocOpen);
    for (tools::Long nPos=0; nPos<nArgCount; nPos++)
    {
        if ( nPos > 0 )
            aTokenArr.AddOpCode(ocSep);

        const uno::Any& rArg = pArgArr[nPos];

        uno::TypeClass eClass = rArg.getValueTypeClass();
        const uno::Type& aType = rArg.getValueType();
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
            aTokenArr.AddString(rSPool.intern(aUStr));
        }
        else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<sal_Int16> >>::get() ) )
        {
            ArrayOfArrayProc<sal_Int16>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<sal_Int32> >>::get() ) )
        {
            ArrayOfArrayProc<sal_Int32>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<double> >>::get() ) )
        {
            ArrayOfArrayProc<double>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<OUString> >>::get() ) )
        {
            ArrayOfArrayProc<OUString>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( cppu::UnoType<uno::Sequence< uno::Sequence<uno::Any> >>::get() ) )
        {
            ArrayOfArrayProc<uno::Any>::processSequences( pDoc, rArg, aTokenArr, nDocRow, bArgErr, bOverflow );
        }
        else if ( aType.equals( cppu::UnoType<table::XCellRange>::get()) )
        {
            // currently, only our own cell ranges are supported

            uno::Reference<table::XCellRange> xRange(rArg, uno::UNO_QUERY);
            ScCellRangesBase* pImpl = dynamic_cast<ScCellRangesBase*>( xRange.get() );
            if ( pImpl )
            {
                ScDocument* pSrcDoc = pImpl->GetDocument();
                const ScRangeList& rRanges = pImpl->GetRangeList();
                if ( pSrcDoc && rRanges.size() == 1 )
                {
                    ScRange const & rSrcRange = rRanges[ 0 ];

                    sal_Int32 nStartRow = nDocRow;
                    sal_Int32 nColCount = rSrcRange.aEnd.Col() - rSrcRange.aStart.Col() + 1;
                    sal_Int32 nRowCount = rSrcRange.aEnd.Row() - rSrcRange.aStart.Row() + 1;

                    if ( nStartRow + nRowCount > pDoc->GetSheetLimits().GetMaxRowCount() )
                        bOverflow = true;
                    else
                    {
                        // copy data
                        if ( !lcl_CopyData( pSrcDoc, rSrcRange, pDoc, ScAddress( 0, static_cast<SCROW>(nDocRow), 0 ) ) )
                            bOverflow = true;
                    }

                    nDocRow += nRowCount;
                    if ( !bOverflow )
                        lcl_AddRef( aTokenArr, nStartRow, nColCount, nRowCount );
                }
                else
                    bArgErr = true;
            }
            else
                bArgErr = true;
        }
        else
            bArgErr = true;                 // invalid type
    }
    aTokenArr.AddOpCode(ocClose);
    aTokenArr.AddOpCode(ocStop);

    //  execute formula

    uno::Any aRet;
    if ( !bArgErr && !bOverflow && nDocRow <= pDoc->GetSheetLimits().GetMaxRowCount() )
    {
        ScAddress aFormulaPos( 0, 0, nTempSheet );
        // GRAM_API doesn't really matter for the token array but fits with
        // other API compatibility grammars.
        ScFormulaCell* pFormula = new ScFormulaCell(
            *pDoc, aFormulaPos, aTokenArr, formula::FormulaGrammar::GRAM_API,
            mbArray ? ScMatrixMode::Formula : ScMatrixMode::NONE );
        pFormula = pDoc->SetFormulaCell(aFormulaPos, pFormula);
        if (mbArray && pFormula)
            pFormula->SetMatColsRows(1,1);  // the cell dimensions (only one cell)

        //  call GetMatrix before GetErrCode because GetMatrix always recalculates
        //  if there is no matrix result

        const ScMatrix* pMat = (mbArray && pFormula) ? pFormula->GetMatrix() : nullptr;
        FormulaError nErrCode = pFormula ? pFormula->GetErrCode() : FormulaError::IllegalArgument;
        if ( nErrCode == FormulaError::NONE )
        {
            if ( pMat )
            {
                // array result
                ScRangeToSequence::FillMixedArray( aRet, pMat );
            }
            else if ( pFormula->IsValue() )
            {
                // numeric value
                aRet <<= pFormula->GetValue();
            }
            else
            {
                // string result
                OUString aStrVal = pFormula->GetString().getString();
                aRet <<= aStrVal;
            }
        }
        else if ( nErrCode == FormulaError::NotAvailable )
        {
            // #N/A: leave result empty, no exception
        }
        else
        {
            //  any other error: IllegalArgumentException
            bArgErr = true;
        }

        pDoc->DeleteAreaTab( 0, 0, pDoc->MaxCol(), pDoc->MaxRow(), 0, InsertDeleteFlags::ALL );
        pDoc->DeleteAreaTab( 0, 0, 0, 0, nTempSheet, InsertDeleteFlags::ALL );
    }

    if (bOverflow)
        throw uno::RuntimeException();

    if (bArgErr)
        throw lang::IllegalArgumentException();

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

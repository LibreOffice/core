/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "validat.hxx"

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>

#include <basic/sbx.hxx>
#include <svl/zforlist.hxx>
#include "svl/sharedstringpool.hxx"
#include <vcl/msgbox.hxx>
#include <rtl/math.hxx>

#include "scitems.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "typedstrdata.hxx"
#include "dociter.hxx"
#include "editutil.hxx"
#include "tokenarray.hxx"
#include "scmatrix.hxx"

#include <math.h>
#include <memory>

using namespace formula;

//

//

ScValidationData::ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                    const OUString& rExpr1, const OUString& rExpr2,
                                    ScDocument* pDocument, const ScAddress& rPos,
                                    const OUString& rExprNmsp1, const OUString& rExprNmsp2,
                                    FormulaGrammar::Grammar eGrammar1,
                                    FormulaGrammar::Grammar eGrammar2 )
    : ScConditionEntry( eOper, rExpr1, rExpr2, pDocument, rPos, rExprNmsp1,
                        rExprNmsp2, eGrammar1, eGrammar2 )
    , nKey( 0 )
    , eDataMode( eMode )
    , bShowInput(false)
    , bShowError(false)
    , eErrorStyle( SC_VALERR_STOP )
    , mnListType( ValidListType::UNSORTED )
{
}

ScValidationData::ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                    const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                    ScDocument* pDocument, const ScAddress& rPos )
    : ScConditionEntry( eOper, pArr1, pArr2, pDocument, rPos )
    , nKey( 0 )
    , eDataMode( eMode )
    , bShowInput(false)
    , bShowError(false)
    , eErrorStyle( SC_VALERR_STOP )
    , mnListType( ValidListType::UNSORTED )
{
}

ScValidationData::ScValidationData( const ScValidationData& r )
    : ScConditionEntry( r )
    , nKey( r.nKey )
    , eDataMode( r.eDataMode )
    , bShowInput( r.bShowInput )
    , bShowError( r.bShowError )
    , eErrorStyle( r.eErrorStyle )
    , mnListType( r.mnListType )
    , aInputTitle( r.aInputTitle )
    , aInputMessage( r.aInputMessage )
    , aErrorTitle( r.aErrorTitle )
    , aErrorMessage( r.aErrorMessage )
{
    
}

ScValidationData::ScValidationData( ScDocument* pDocument, const ScValidationData& r )
    : ScConditionEntry( pDocument, r )
    , nKey( r.nKey )
    , eDataMode( r.eDataMode )
    , bShowInput( r.bShowInput )
    , bShowError( r.bShowError )
    , eErrorStyle( r.eErrorStyle )
    , mnListType( r.mnListType )
    , aInputTitle( r.aInputTitle )
    , aInputMessage( r.aInputMessage )
    , aErrorTitle( r.aErrorTitle )
    , aErrorMessage( r.aErrorMessage )
{
    
}

ScValidationData::~ScValidationData()
{
}

bool ScValidationData::IsEmpty() const
{
    OUString aEmpty;
    ScValidationData aDefault( SC_VALID_ANY, SC_COND_EQUAL, aEmpty, aEmpty, GetDocument(), ScAddress() );
    return EqualEntries( aDefault );
}

bool ScValidationData::EqualEntries( const ScValidationData& r ) const
{
        

    return ScConditionEntry::operator==(r) &&
            eDataMode       == r.eDataMode &&
            bShowInput      == r.bShowInput &&
            bShowError      == r.bShowError &&
            eErrorStyle     == r.eErrorStyle &&
            mnListType      == r.mnListType &&
            aInputTitle     == r.aInputTitle &&
            aInputMessage   == r.aInputMessage &&
            aErrorTitle     == r.aErrorTitle &&
            aErrorMessage   == r.aErrorMessage;
}

void ScValidationData::ResetInput()
{
    bShowInput = false;
}

void ScValidationData::ResetError()
{
    bShowError = false;
}

void ScValidationData::SetInput( const OUString& rTitle, const OUString& rMsg )
{
    bShowInput = true;
    aInputTitle = rTitle;
    aInputMessage = rMsg;
}

void ScValidationData::SetError( const OUString& rTitle, const OUString& rMsg,
                                    ScValidErrorStyle eStyle )
{
    bShowError = true;
    eErrorStyle = eStyle;
    aErrorTitle = rTitle;
    aErrorMessage = rMsg;
}

bool ScValidationData::GetErrMsg( OUString& rTitle, OUString& rMsg,
                                    ScValidErrorStyle& rStyle ) const
{
    rTitle = aErrorTitle;
    rMsg   = aErrorMessage;
    rStyle = eErrorStyle;
    return bShowError;
}

bool ScValidationData::DoScript( const ScAddress& rPos, const OUString& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const
{
    ScDocument* pDocument = GetDocument();
    SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
    if ( !pDocSh || !pDocument->CheckMacroWarn() )
        return false;

    bool bScriptReturnedFalse = false;  

    
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aParams(2);

    
    OUString aValStr = rInput;
    double nValue;
    bool bIsValue = false;
    if ( pCell )                
    {
        bIsValue = pCell->IsValue();
        if ( bIsValue )
            nValue  = pCell->GetValue();
        else
            aValStr = pCell->GetString().getString();
    }
    if ( bIsValue )
        aParams[0] = ::com::sun::star::uno::makeAny( nValue );
    else
        aParams[0] = ::com::sun::star::uno::makeAny( OUString( aValStr ) );

    
    OUString aPosStr(rPos.Format(SCA_VALID | SCA_TAB_3D, pDocument, pDocument->GetAddressConvention()));
    aParams[1] = ::com::sun::star::uno::makeAny(aPosStr);

    
    
    bool bWasInLinkUpdate = pDocument->IsInLinkUpdate();
    if ( !bWasInLinkUpdate )
        pDocument->SetInLinkUpdate( true );

    if ( pCell )
        pDocument->LockTable( rPos.Tab() );

    ::com::sun::star::uno::Any aRet;
    ::com::sun::star::uno::Sequence< sal_Int16 > aOutArgsIndex;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aOutArgs;

    ErrCode eRet = pDocSh->CallXScript(
        aErrorTitle, aParams, aRet, aOutArgsIndex, aOutArgs );

    if ( pCell )
        pDocument->UnlockTable( rPos.Tab() );

    if ( !bWasInLinkUpdate )
        pDocument->SetInLinkUpdate( false );

    
    
    bool bTmp = false;
    if ( eRet == ERRCODE_NONE &&
             aRet.getValueType() == getCppuBooleanType() &&
             ( aRet >>= bTmp ) &&
             !bTmp )
    {
        bScriptReturnedFalse =  true;
    }

    if ( eRet == ERRCODE_BASIC_METHOD_NOT_FOUND && !pCell )
    
    {
        

        ErrorBox aBox( pParent, WinBits(WB_OK),
                        ScGlobal::GetRscString( STR_VALID_MACRONOTFOUND ) );
        aBox.Execute();
    }

    return bScriptReturnedFalse;
}

    

bool ScValidationData::DoMacro( const ScAddress& rPos, const OUString& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const
{
    if ( SfxApplication::IsXScriptURL( aErrorTitle ) )
    {
        return DoScript( rPos, rInput, pCell, pParent );
    }

    ScDocument* pDocument = GetDocument();
    SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
    if ( !pDocSh || !pDocument->CheckMacroWarn() )
        return false;

    bool bDone = false;
    bool bRet = false;                      

    
    


#ifndef DISABLE_SCRIPTING
    

    
    

    StarBASIC* pRoot = pDocSh->GetBasic();
    SbxVariable* pVar = pRoot->Find( aErrorTitle, SbxCLASS_METHOD );
    if ( pVar && pVar->ISA(SbMethod) )
    {
        SbMethod* pMethod = (SbMethod*)pVar;
        SbModule* pModule = pMethod->GetModule();
        SbxObject* pObject = pModule->GetParent();
        OUStringBuffer aMacroStr = pObject->GetName();
        aMacroStr.append('.').append(pModule->GetName()).append('.').append(pMethod->GetName());
        OUString aBasicStr;

        
        
        
        

        if ( pObject->GetParent() )
            aBasicStr = pObject->GetParent()->GetName();    
        else
            aBasicStr = SFX_APP()->GetName();               

        
        SbxArrayRef refPar = new SbxArray;

        
        OUString aValStr = rInput;
        double nValue = 0.0;
        bool bIsValue = false;
        if ( pCell )                
        {
            bIsValue = pCell->IsValue();
            if ( bIsValue )
                nValue  = pCell->GetValue();
            else
                aValStr = pCell->GetString().getString();
        }
        if ( bIsValue )
            refPar->Get(1)->PutDouble( nValue );
        else
            refPar->Get(1)->PutString( aValStr );

        
        OUString aPosStr(rPos.Format(SCA_VALID | SCA_TAB_3D, pDocument, pDocument->GetAddressConvention()));
        refPar->Get(2)->PutString( aPosStr );

        
        
        bool bWasInLinkUpdate = pDocument->IsInLinkUpdate();
        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( true );

        if ( pCell )
            pDocument->LockTable( rPos.Tab() );
        SbxVariableRef refRes = new SbxVariable;
        ErrCode eRet = pDocSh->CallBasic( aMacroStr.makeStringAndClear(), aBasicStr, refPar, refRes );
        if ( pCell )
            pDocument->UnlockTable( rPos.Tab() );

        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( false );

        
        if ( eRet == ERRCODE_NONE && refRes->GetType() == SbxBOOL && !refRes->GetBool() )
            bRet = true;
        bDone = true;
    }
#endif
    if ( !bDone && !pCell )         
    {
        

        ErrorBox aBox( pParent, WinBits(WB_OK),
                        ScGlobal::GetRscString( STR_VALID_MACRONOTFOUND ) );
        aBox.Execute();
    }

    return bRet;
}

void ScValidationData::DoCalcError( ScFormulaCell* pCell ) const
{
    if ( eErrorStyle == SC_VALERR_MACRO )
        DoMacro( pCell->aPos, EMPTY_OUSTRING, pCell, NULL );
}

    

bool ScValidationData::DoError( Window* pParent, const OUString& rInput,
                                const ScAddress& rPos ) const
{
    if ( eErrorStyle == SC_VALERR_MACRO )
        return DoMacro( rPos, rInput, NULL, pParent );

    

    OUString aTitle = aErrorTitle;
    if (aTitle.isEmpty())
        aTitle = ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 );  
    OUString aMessage = aErrorMessage;
    if (aMessage.isEmpty())
        aMessage = ScGlobal::GetRscString( STR_VALID_DEFERROR );

    
    

    WinBits nStyle = 0;
    switch (eErrorStyle)
    {
        case SC_VALERR_STOP:
            nStyle = WB_OK | WB_DEF_OK;
            break;
        case SC_VALERR_WARNING:
            nStyle = WB_OK_CANCEL | WB_DEF_CANCEL;
            break;
        case SC_VALERR_INFO:
            nStyle = WB_OK_CANCEL | WB_DEF_OK;
            break;
        default:
        {
            
        }
    }

    MessBox aBox( pParent, WinBits(nStyle), aTitle, aMessage );
    sal_uInt16 nRet = aBox.Execute();

    return ( eErrorStyle == SC_VALERR_STOP || nRet == RET_CANCEL );
}


bool ScValidationData::IsDataValid(
    const OUString& rTest, const ScPatternAttr& rPattern, const ScAddress& rPos ) const
{
    if ( eDataMode == SC_VALID_ANY ) 
        return true;

    if (rTest.isEmpty())              
        return IsIgnoreBlank();

    if (rTest[0] == '=')   
        return false;


    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    
    sal_uInt32 nFormat = rPattern.GetNumberFormat( pFormatter );
    double nVal;
    bool bIsVal = pFormatter->IsNumberFormat( rTest, nFormat, nVal );

    bool bRet;
    if (SC_VALID_TEXTLEN == eDataMode)
    {
        double nLenVal;
        if (!bIsVal)
            nLenVal = static_cast<double>(rTest.getLength());
        else
        {
            
            
            
            
            OUString aStr;
            pFormatter->GetInputLineString( nVal, nFormat, aStr);
            nLenVal = static_cast<double>( aStr.getLength() );
        }
        ScRefCellValue aTmpCell(nLenVal);
        bRet = IsCellValid(aTmpCell, rPos);
    }
    else
    {
        if (bIsVal)
        {
            ScRefCellValue aTmpCell(nVal);
            bRet = IsDataValid(aTmpCell, rPos);
        }
        else
        {
            svl::SharedString aSS = mpDoc->GetSharedStringPool().intern(rTest);
            ScRefCellValue aTmpCell(&aSS);
            bRet = IsDataValid(aTmpCell, rPos);
        }
    }

    return bRet;
}

bool ScValidationData::IsDataValid( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    if( eDataMode == SC_VALID_LIST )
        return IsListValid(rCell, rPos);

    double nVal = 0.0;
    OUString aString;
    bool bIsVal = true;

    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
            nVal = rCell.mfValue;
        break;
        case CELLTYPE_STRING:
            aString = rCell.mpString->getString();
            bIsVal = false;
        break;
        case CELLTYPE_EDIT:
            if (rCell.mpEditText)
                aString = ScEditUtil::GetString(*rCell.mpEditText, GetDocument());
            bIsVal = false;
        break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = rCell.mpFormula;
            bIsVal = pFCell->IsValue();
            if ( bIsVal )
                nVal  = pFCell->GetValue();
            else
                aString = pFCell->GetString().getString();
        }
        break;
        default:                        
            return IsIgnoreBlank();     
    }

    bool bOk = true;
    switch (eDataMode)
    {
        

        case SC_VALID_WHOLE:
        case SC_VALID_DECIMAL:
        case SC_VALID_DATE:         
        case SC_VALID_TIME:
            bOk = bIsVal;
            if ( bOk && eDataMode == SC_VALID_WHOLE )
                bOk = ::rtl::math::approxEqual( nVal, floor(nVal+0.5) );        
            if ( bOk )
                bOk = IsCellValid(rCell, rPos);
            break;

        case SC_VALID_CUSTOM:
            
            
            bOk = IsCellValid(rCell, rPos);
            break;

        case SC_VALID_TEXTLEN:
            bOk = !bIsVal;          
            if ( bOk )
            {
                double nLenVal = (double) aString.getLength();
                ScRefCellValue aTmpCell(nLenVal);
                bOk = IsCellValid(aTmpCell, rPos);
            }
            break;

        default:
            OSL_FAIL("not yet done");
            break;
    }

    return bOk;
}

namespace {

/** Token array helper. Iterates over all string tokens.
    @descr  The token array must contain separated string tokens only.
    @param bSkipEmpty  true = Ignores string tokens with empty strings. */
class ScStringTokenIterator
{
public:
    inline explicit             ScStringTokenIterator( ScTokenArray& rTokArr, bool bSkipEmpty = true ) :
                                    mrTokArr( rTokArr ), mbSkipEmpty( bSkipEmpty ), mbOk( true ) {}

    /** Returns the string of the first string token or NULL on error or empty token array. */
    rtl_uString* First();
    /** Returns the string of the next string token or NULL on error or end of token array. */
    rtl_uString* Next();

    /** Returns false, if a wrong token has been found. Does NOT return false on end of token array. */
    inline bool                 Ok() const { return mbOk; }

private:
    svl::SharedString maCurString; 
    ScTokenArray&               mrTokArr;       
    bool                        mbSkipEmpty;    
    bool                        mbOk;           
};

rtl_uString* ScStringTokenIterator::First()
{
    mrTokArr.Reset();
    mbOk = true;
    return Next();
}

rtl_uString* ScStringTokenIterator::Next()
{
    if( !mbOk )
        return NULL;

    
    const FormulaToken* pToken = mrTokArr.NextNoSpaces();
    while( pToken && (pToken->GetOpCode() == ocSep) )
        pToken = mrTokArr.NextNoSpaces();

    mbOk = !pToken || (pToken->GetType() == formula::svString);

    maCurString = svl::SharedString(); 
    if (mbOk && pToken)
        maCurString = pToken->GetString();

    
    return (mbSkipEmpty && maCurString.isValid() && maCurString.isEmpty()) ? Next() : maCurString.getData();
}



/** Returns the number format of the passed cell, or the standard format. */
sal_uLong lclGetCellFormat( ScDocument& rDoc, const ScAddress& rPos )
{
    const ScPatternAttr* pPattern = rDoc.GetPattern( rPos.Col(), rPos.Row(), rPos.Tab() );
    if( !pPattern )
        pPattern = rDoc.GetDefPattern();
    return pPattern->GetNumberFormat( rDoc.GetFormatTable() );
}

} 



bool ScValidationData::HasSelectionList() const
{
    return (eDataMode == SC_VALID_LIST) && (mnListType != ValidListType::INVISIBLE);
}

bool ScValidationData::GetSelectionFromFormula(
    std::vector<ScTypedStrData>* pStrings, ScRefCellValue& rCell, const ScAddress& rPos,
    const ScTokenArray& rTokArr, int& rMatch) const
{
    bool bOk = true;

    
    ScDocument* pDocument = GetDocument();
    if( NULL == pDocument )
        return false;

    ScFormulaCell aValidationSrc(
        pDocument, rPos, rTokArr, formula::FormulaGrammar::GRAM_DEFAULT, MM_FORMULA);

    
    
    aValidationSrc.Interpret();

    ScMatrixRef xMatRef;
    const ScMatrix *pValues = aValidationSrc.GetMatrix();
    if (!pValues)
    {
        
        
        

        
        xMatRef = new ScMatrix(1, 1, 0.0);

        sal_uInt16 nErrCode = aValidationSrc.GetErrCode();
        if (nErrCode)
        {
            /* TODO : to use later in an alert box?
             * OUString rStrResult = "...";
             * rStrResult += ScGlobal::GetLongErrorString(nErrCode);
             */

            xMatRef->PutError( nErrCode, 0, 0);
            bOk = false;
        }
        else if (aValidationSrc.IsValue())
            xMatRef->PutDouble( aValidationSrc.GetValue(), 0);
        else
        {
            svl::SharedString aStr = aValidationSrc.GetString();
            xMatRef->PutString(aStr, 0);
        }

        pValues = xMatRef.get();
    }

    
    rMatch = -1;

    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    SCSIZE  nCol, nRow, nCols, nRows, n = 0;
    pValues->GetDimensions( nCols, nRows );

    bool bRef = false;
    ScRange aRange;

    ScTokenArray* pArr = (ScTokenArray*) &rTokArr;
    pArr->Reset();
    ScToken* t = NULL;
    if (pArr->GetLen() == 1 && (t = static_cast<ScToken*>(pArr->GetNextReferenceOrName())) != NULL)
    {
        if (t->GetOpCode() == ocDBArea)
        {
            if (const ScDBData* pDBData = pDocument->GetDBCollection()->getNamedDBs().findByIndex(t->GetIndex()))
            {
                pDBData->GetArea(aRange);
                bRef = true;
            }
        }
        else if (t->GetOpCode() == ocName)
        {
            ScRangeData* pName = pDocument->GetRangeName()->findByIndex( t->GetIndex() );
            if (pName && pName->IsReference(aRange))
            {
                bRef = true;
            }
        }
        else if (t->GetType() != svIndex)
        {
            if (pArr->IsValidReference(aRange, rPos))
            {
                bRef = true;
            }
        }
    }

    /* XL artificially limits things to a single col or row in the UI but does
     * not list the constraint in MOOXml. If a defined name or INDIRECT
     * resulting in 1D is entered in the UI and the definition later modified
     * to 2D, it is evaluated fine and also stored and loaded.  Lets get ahead
     * of the curve and support 2d. In XL, values are listed row-wise, do the
     * same. */
    for( nRow = 0; nRow < nRows ; nRow++ )
    {
        for( nCol = 0; nCol < nCols ; nCol++ )
        {
            ScTokenArray         aCondTokArr;
            ScTypedStrData*        pEntry = NULL;
            OUString               aValStr;
            ScMatrixValue nMatVal = pValues->Get( nCol, nRow);

            
            if( ScMatrix::IsNonValueType( nMatVal.nType ) )
            {
                aValStr = nMatVal.GetString().getString();

                if( NULL != pStrings )
                    pEntry = new ScTypedStrData( aValStr, 0.0, ScTypedStrData::Standard);

                if (!rCell.isEmpty() && rMatch < 0)
                    aCondTokArr.AddString( aValStr );
            }
            else
            {
                sal_uInt16 nErr = nMatVal.GetError();

                if( 0 != nErr )
                {
                    aValStr = ScGlobal::GetErrorString( nErr );
                }
                else
                {
                    
                    
                    
                    
                    if ( bRef )
                    {
                        pDocument->GetInputString((SCCOL)(nCol+aRange.aStart.Col()),
                            (SCROW)(nRow+aRange.aStart.Row()), aRange.aStart.Tab() , aValStr);
                    }
                    else
                    {
                        OUString sTmp(aValStr);
                        pFormatter->GetInputLineString( nMatVal.fVal, 0, sTmp );
                        aValStr = sTmp;
                    }
                }

                if (!rCell.isEmpty() && rMatch < 0)
                {
                    
                    
                    aCondTokArr.AddDouble( nMatVal.fVal );
                }
                if( NULL != pStrings )
                    pEntry = new ScTypedStrData( aValStr, nMatVal.fVal, ScTypedStrData::Value);
            }

            if (rMatch < 0 && !rCell.isEmpty() && IsEqualToTokenArray(rCell, rPos, aCondTokArr))
            {
                rMatch = n;
                
                if( NULL == pStrings )
                    return true;
            }

            if( NULL != pEntry )
            {
                pStrings->push_back(*pEntry);
                delete pEntry;
                n++;
            }
        }
    }

    
    
    return bOk || rCell.isEmpty();
}

bool ScValidationData::FillSelectionList(std::vector<ScTypedStrData>& rStrColl, const ScAddress& rPos) const
{
    bool bOk = false;

    if( HasSelectionList() )
    {
        boost::scoped_ptr<ScTokenArray> pTokArr( CreateTokenArry(0) );

        

        sal_uInt32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
        ScStringTokenIterator aIt( *pTokArr );
        for (rtl_uString* pString = aIt.First(); pString && aIt.Ok(); pString = aIt.Next())
        {
            double fValue;
            OUString aStr(pString);
            bool bIsValue = GetDocument()->GetFormatTable()->IsNumberFormat(aStr, nFormat, fValue);
            rStrColl.push_back(
                ScTypedStrData(
                    aStr, fValue, bIsValue ? ScTypedStrData::Value : ScTypedStrData::Standard));
        }
        bOk = aIt.Ok();

        
        

        if (!bOk)
        {
            int nMatch;
            ScRefCellValue aEmptyCell;
            bOk = GetSelectionFromFormula(&rStrColl, aEmptyCell, rPos, *pTokArr, nMatch);
        }
    }

    return bOk;
}



bool ScValidationData::IsEqualToTokenArray( ScRefCellValue& rCell, const ScAddress& rPos, const ScTokenArray& rTokArr ) const
{
    
    ScConditionEntry aCondEntry( SC_COND_EQUAL, &rTokArr, NULL, GetDocument(), rPos );
    return aCondEntry.IsCellValid(rCell, rPos);
}

bool ScValidationData::IsListValid( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    bool bIsValid = false;

    /*  Compare input cell with all supported tokens from the formula.
        Currently a formula may contain:
        1)  A list of strings (at least one string).
        2)  A single cell or range reference.
        3)  A single defined name (must contain a cell/range reference, another
            name, or DB range, or a formula resulting in a cell/range reference
            or matrix/array).
        4)  A single database range.
        5)  A formula resulting in a cell/range reference or matrix/array.
    */

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ScTokenArray > pTokArr( CreateTokenArry( 0 ) );
    SAL_WNODEPRECATED_DECLARATIONS_POP

    

    sal_uInt32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
    ScStringTokenIterator aIt( *pTokArr );
    for (rtl_uString* pString = aIt.First(); pString && aIt.Ok(); pString = aIt.Next())
    {
        /*  Do not break the loop, if a valid string has been found.
            This is to find invalid tokens following in the formula. */
        if( !bIsValid )
        {
            
            ScTokenArray aCondTokArr;
            double fValue;
            OUString aStr(pString);
            if (GetDocument()->GetFormatTable()->IsNumberFormat(aStr, nFormat, fValue))
                aCondTokArr.AddDouble( fValue );
            else
                aCondTokArr.AddString(aStr);

            bIsValid = IsEqualToTokenArray(rCell, rPos, aCondTokArr);
        }
    }

    if( !aIt.Ok() )
        bIsValid = false;

    
    

    if (!bIsValid)
    {
        int nMatch;
        bIsValid = GetSelectionFromFormula(NULL, rCell, rPos, *pTokArr, nMatch);
        bIsValid = bIsValid && nMatch >= 0;
    }

    return bIsValid;
}




ScValidationDataList::ScValidationDataList(const ScValidationDataList& rList)
{
    

    for (const_iterator it = rList.begin(); it != rList.end(); ++it)
    {
        InsertNew( (*it)->Clone() );
    }

    
}

ScValidationDataList::ScValidationDataList(ScDocument* pNewDoc,
                                            const ScValidationDataList& rList)
{
    

    for (const_iterator it = rList.begin(); it != rList.end(); ++it)
    {
        InsertNew( (*it)->Clone(pNewDoc) );
    }

    
}

ScValidationData* ScValidationDataList::GetData( sal_uInt32 nKey )
{
    

    for( iterator it = begin(); it != end(); ++it )
        if( (*it)->GetKey() == nKey )
            return *it;

    OSL_FAIL("ScValidationDataList: Entry not found");
    return NULL;
}

void ScValidationDataList::CompileXML()
{
    for( iterator it = begin(); it != end(); ++it )
        (*it)->CompileXML();
}

void ScValidationDataList::UpdateReference( sc::RefUpdateContext& rCxt )
{
    for( iterator it = begin(); it != end(); ++it )
        (*it)->UpdateReference(rCxt);
}

void ScValidationDataList::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    for (iterator it = begin(); it != end(); ++it)
        (*it)->UpdateInsertTab(rCxt);
}

void ScValidationDataList::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    for (iterator it = begin(); it != end(); ++it)
        (*it)->UpdateDeleteTab(rCxt);
}

void ScValidationDataList::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    for (iterator it = begin(); it != end(); ++it)
        (*it)->UpdateMoveTab(rCxt);
}

bool ScValidationDataList::operator==( const ScValidationDataList& r ) const
{
    

    size_t nCount = maData.size();
    bool bEqual = ( nCount == r.maData.size() );
    for( const_iterator it1 = begin(), it2 = r.begin(); it1 != end() && bEqual; ++it1, ++it2 ) 
        if ( !(*it1)->EqualEntries(**it2) )         
            bEqual = false;

    return bEqual;
}

ScValidationDataList::iterator ScValidationDataList::begin()
{
    return maData.begin();
}

ScValidationDataList::const_iterator ScValidationDataList::begin() const
{
    return maData.begin();
}

ScValidationDataList::iterator ScValidationDataList::end()
{
    return maData.end();
}

ScValidationDataList::const_iterator ScValidationDataList::end() const
{
    return maData.end();
}

void ScValidationDataList::clear()
{
    maData.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

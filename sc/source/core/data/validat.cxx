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
//  Eintrag fuer Gueltigkeit (es gibt nur eine Bedingung)
//

ScValidationData::ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                            const String& rExpr1, const String& rExpr2,
                            ScDocument* pDocument, const ScAddress& rPos,
                            const String& rExprNmsp1, const String& rExprNmsp2,
                            FormulaGrammar::Grammar eGrammar1, FormulaGrammar::Grammar eGrammar2 ) :
    ScConditionEntry( eOper, rExpr1, rExpr2, pDocument, rPos, rExprNmsp1, rExprNmsp2, eGrammar1, eGrammar2 ),
    nKey( 0 ),
    eDataMode( eMode ),
    eErrorStyle( SC_VALERR_STOP ),
    mnListType( ValidListType::UNSORTED )
{
    bShowInput = bShowError = false;
}

ScValidationData::ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                            const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                            ScDocument* pDocument, const ScAddress& rPos ) :
    ScConditionEntry( eOper, pArr1, pArr2, pDocument, rPos ),
    nKey( 0 ),
    eDataMode( eMode ),
    eErrorStyle( SC_VALERR_STOP ),
    mnListType( ValidListType::UNSORTED )
{
    bShowInput = bShowError = false;
}

ScValidationData::ScValidationData( const ScValidationData& r ) :
    ScConditionEntry( r ),
    nKey( r.nKey ),
    eDataMode( r.eDataMode ),
    bShowInput( r.bShowInput ),
    bShowError( r.bShowError ),
    eErrorStyle( r.eErrorStyle ),
    mnListType( r.mnListType ),
    aInputTitle( r.aInputTitle ),
    aInputMessage( r.aInputMessage ),
    aErrorTitle( r.aErrorTitle ),
    aErrorMessage( r.aErrorMessage )
{
    //  Formeln per RefCount kopiert
}

ScValidationData::ScValidationData( ScDocument* pDocument, const ScValidationData& r ) :
    ScConditionEntry( pDocument, r ),
    nKey( r.nKey ),
    eDataMode( r.eDataMode ),
    bShowInput( r.bShowInput ),
    bShowError( r.bShowError ),
    eErrorStyle( r.eErrorStyle ),
    mnListType( r.mnListType ),
    aInputTitle( r.aInputTitle ),
    aInputMessage( r.aInputMessage ),
    aErrorTitle( r.aErrorTitle ),
    aErrorMessage( r.aErrorMessage )
{
    //  Formeln wirklich kopiert
}

ScValidationData::~ScValidationData()
{
}

sal_Bool ScValidationData::IsEmpty() const
{
    String aEmpty;
    ScValidationData aDefault( SC_VALID_ANY, SC_COND_EQUAL, aEmpty, aEmpty, GetDocument(), ScAddress() );
    return EqualEntries( aDefault );
}

sal_Bool ScValidationData::EqualEntries( const ScValidationData& r ) const
{
        //  gleiche Parameter eingestellt (ohne Key)

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

void ScValidationData::SetInput( const String& rTitle, const String& rMsg )
{
    bShowInput = sal_True;
    aInputTitle = rTitle;
    aInputMessage = rMsg;
}

void ScValidationData::SetError( const String& rTitle, const String& rMsg,
                                    ScValidErrorStyle eStyle )
{
    bShowError = sal_True;
    eErrorStyle = eStyle;
    aErrorTitle = rTitle;
    aErrorMessage = rMsg;
}

sal_Bool ScValidationData::GetErrMsg( String& rTitle, String& rMsg,
                                    ScValidErrorStyle& rStyle ) const
{
    rTitle = aErrorTitle;
    rMsg   = aErrorMessage;
    rStyle = eErrorStyle;
    return bShowError;
}

sal_Bool ScValidationData::DoScript( const ScAddress& rPos, const String& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const
{
    ScDocument* pDocument = GetDocument();
    SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
    if ( !pDocSh || !pDocument->CheckMacroWarn() )
        return false;

    sal_Bool bScriptReturnedFalse = false;  // Standard: kein Abbruch

    // Set up parameters
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aParams(2);

    //  1) eingegebener / berechneter Wert
    String aValStr = rInput;
    double nValue;
    sal_Bool bIsValue = false;
    if ( pCell )                // wenn Zelle gesetzt, aus Interpret gerufen
    {
        bIsValue = pCell->IsValue();
        if ( bIsValue )
            nValue  = pCell->GetValue();
        else
            aValStr = pCell->GetString();
    }
    if ( bIsValue )
        aParams[0] = ::com::sun::star::uno::makeAny( nValue );
    else
        aParams[0] = ::com::sun::star::uno::makeAny( OUString( aValStr ) );

    //  2) Position der Zelle
    String aPosStr;
    rPos.Format( aPosStr, SCA_VALID | SCA_TAB_3D, pDocument, pDocument->GetAddressConvention() );
    aParams[1] = ::com::sun::star::uno::makeAny( OUString( aPosStr ) );

    //  use link-update flag to prevent closing the document
    //  while the macro is running
    sal_Bool bWasInLinkUpdate = pDocument->IsInLinkUpdate();
    if ( !bWasInLinkUpdate )
        pDocument->SetInLinkUpdate( sal_True );

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

    // Check the return value from the script
    // The contents of the cell get reset if the script returns false
    sal_Bool bTmp = false;
    if ( eRet == ERRCODE_NONE &&
             aRet.getValueType() == getCppuBooleanType() &&
             sal_True == ( aRet >>= bTmp ) &&
             bTmp == false )
    {
        bScriptReturnedFalse = sal_True;
    }

    if ( eRet == ERRCODE_BASIC_METHOD_NOT_FOUND && !pCell )
    // Makro nicht gefunden (nur bei Eingabe)
    {
        //! andere Fehlermeldung, wenn gefunden, aber nicht bAllowed ??

        ErrorBox aBox( pParent, WinBits(WB_OK),
                        ScGlobal::GetRscString( STR_VALID_MACRONOTFOUND ) );
        aBox.Execute();
    }

    return bScriptReturnedFalse;
}

    // sal_True -> Abbruch

sal_Bool ScValidationData::DoMacro( const ScAddress& rPos, const String& rInput,
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

    sal_Bool bDone = false;
    sal_Bool bRet = false;                      // Standard: kein Abbruch

    //  Wenn das Dok waehrend eines Basic-Calls geladen wurde,
    //  ist das Sbx-Objekt evtl. nicht angelegt (?)
//  pDocSh->GetSbxObject();

#ifndef DISABLE_SCRIPTING
    //  keine Sicherheitsabfrage mehr vorneweg (nur CheckMacroWarn), das passiert im CallBasic

    //  Funktion ueber den einfachen Namen suchen,
    //  dann aBasicStr, aMacroStr fuer SfxObjectShell::CallBasic zusammenbauen

    StarBASIC* pRoot = pDocSh->GetBasic();
    SbxVariable* pVar = pRoot->Find( aErrorTitle, SbxCLASS_METHOD );
    if ( pVar && pVar->ISA(SbMethod) )
    {
        SbMethod* pMethod = (SbMethod*)pVar;
        SbModule* pModule = pMethod->GetModule();
        SbxObject* pObject = pModule->GetParent();
        String aMacroStr = pObject->GetName();
        aMacroStr += '.';
        aMacroStr += pModule->GetName();
        aMacroStr += '.';
        aMacroStr += pMethod->GetName();
        String aBasicStr;

        //  the distinction between document- and app-basic has to be done
        //  by checking the parent (as in ScInterpreter::ScMacro), not by looping
        //  over all open documents, because this may be called from within loading,
        //  when SfxObjectShell::GetFirst/GetNext won't find the document.

        if ( pObject->GetParent() )
            aBasicStr = pObject->GetParent()->GetName();    // Dokumentenbasic
        else
            aBasicStr = SFX_APP()->GetName();               // Applikationsbasic

        //  Parameter fuer Makro
        SbxArrayRef refPar = new SbxArray;

        //  1) eingegebener / berechneter Wert
        String aValStr = rInput;
        double nValue = 0.0;
        sal_Bool bIsValue = false;
        if ( pCell )                // wenn Zelle gesetzt, aus Interpret gerufen
        {
            bIsValue = pCell->IsValue();
            if ( bIsValue )
                nValue  = pCell->GetValue();
            else
                aValStr = pCell->GetString();
        }
        if ( bIsValue )
            refPar->Get(1)->PutDouble( nValue );
        else
            refPar->Get(1)->PutString( aValStr );

        //  2) Position der Zelle
        String aPosStr;
        rPos.Format( aPosStr, SCA_VALID | SCA_TAB_3D, pDocument, pDocument->GetAddressConvention() );
        refPar->Get(2)->PutString( aPosStr );

        //  use link-update flag to prevent closing the document
        //  while the macro is running
        sal_Bool bWasInLinkUpdate = pDocument->IsInLinkUpdate();
        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( sal_True );

        if ( pCell )
            pDocument->LockTable( rPos.Tab() );
        SbxVariableRef refRes = new SbxVariable;
        ErrCode eRet = pDocSh->CallBasic( aMacroStr, aBasicStr, refPar, refRes );
        if ( pCell )
            pDocument->UnlockTable( rPos.Tab() );

        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( false );

        //  Eingabe abbrechen, wenn Basic-Makro sal_False zurueckgibt
        if ( eRet == ERRCODE_NONE && refRes->GetType() == SbxBOOL && refRes->GetBool() == false )
            bRet = sal_True;
        bDone = sal_True;
    }
#endif
    if ( !bDone && !pCell )         // Makro nicht gefunden (nur bei Eingabe)
    {
        //! andere Fehlermeldung, wenn gefunden, aber nicht bAllowed ??

        ErrorBox aBox( pParent, WinBits(WB_OK),
                        ScGlobal::GetRscString( STR_VALID_MACRONOTFOUND ) );
        aBox.Execute();
    }

    return bRet;
}

void ScValidationData::DoCalcError( ScFormulaCell* pCell ) const
{
    if ( eErrorStyle == SC_VALERR_MACRO )
        DoMacro( pCell->aPos, EMPTY_STRING, pCell, NULL );
}

    // sal_True -> Abbruch

sal_Bool ScValidationData::DoError( Window* pParent, const String& rInput,
                                const ScAddress& rPos ) const
{
    if ( eErrorStyle == SC_VALERR_MACRO )
        return DoMacro( rPos, rInput, NULL, pParent );

    //  Fehlermeldung ausgeben

    String aTitle = aErrorTitle;
    if (!aTitle.Len())
        aTitle = ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 );  // application title
    String aMessage = aErrorMessage;
    if (!aMessage.Len())
        aMessage = ScGlobal::GetRscString( STR_VALID_DEFERROR );

    //! ErrorBox / WarningBox / InfoBox ?
    //! (bei InfoBox immer nur OK-Button)

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
            // added to avoid warnings
        }
    }

    MessBox aBox( pParent, WinBits(nStyle), aTitle, aMessage );
    sal_uInt16 nRet = aBox.Execute();

    return ( eErrorStyle == SC_VALERR_STOP || nRet == RET_CANCEL );
}


bool ScValidationData::IsDataValid(
    const OUString& rTest, const ScPatternAttr& rPattern, const ScAddress& rPos ) const
{
    if ( eDataMode == SC_VALID_ANY ) // check if any cell content is allowed
        return sal_True;

    if (rTest.isEmpty())              // check whether empty cells are allowed
        return IsIgnoreBlank();

    if (rTest[0] == '=')   // formulas do not pass the validity test
        return false;


    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    // get the value if any
    sal_uInt32 nFormat = rPattern.GetNumberFormat( pFormatter );
    double nVal;
    sal_Bool bIsVal = pFormatter->IsNumberFormat( rTest, nFormat, nVal );

    sal_Bool bRet;
    if (SC_VALID_TEXTLEN == eDataMode)
    {
        double nLenVal;
        if (!bIsVal)
            nLenVal = static_cast<double>(rTest.getLength());
        else
        {
            // For numeric values use the resulting input line string to
            // determine length, otherwise a once accepted value maybe could
            // not be edited again, for example abbreviated dates or leading
            // zeros or trailing zeros after decimal separator change length.
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
            ScRefCellValue aTmpCell(&rTest);
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
            aString = *rCell.mpString;
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
                aString = pFCell->GetString();
        }
        break;
        default:                        // Notizen, Broadcaster
            return IsIgnoreBlank();     // wie eingestellt
    }

    bool bOk = true;
    switch (eDataMode)
    {
        // SC_VALID_ANY schon oben

        case SC_VALID_WHOLE:
        case SC_VALID_DECIMAL:
        case SC_VALID_DATE:         // Date/Time ist nur Formatierung
        case SC_VALID_TIME:
            bOk = bIsVal;
            if ( bOk && eDataMode == SC_VALID_WHOLE )
                bOk = ::rtl::math::approxEqual( nVal, floor(nVal+0.5) );        // ganze Zahlen
            if ( bOk )
                bOk = IsCellValid(rCell, rPos);
            break;

        case SC_VALID_CUSTOM:
            //  fuer Custom muss eOp == SC_COND_DIRECT sein
            //! der Wert muss im Dokument stehen !!!!!!!!!!!!!!!!!!!!
            bOk = IsCellValid(rCell, rPos);
            break;

        case SC_VALID_TEXTLEN:
            bOk = !bIsVal;          // nur Text
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
    const String*               First();
    /** Returns the string of the next string token or NULL on error or end of token array. */
    const String*               Next();

    /** Returns false, if a wrong token has been found. Does NOT return false on end of token array. */
    inline bool                 Ok() const { return mbOk; }

private:
    ScTokenArray&               mrTokArr;       /// The token array for iteration.
    bool                        mbSkipEmpty;    /// Ignore empty strings.
    bool                        mbOk;           /// true = correct token or end of token array.
};

const String* ScStringTokenIterator::First()
{
    mrTokArr.Reset();
    mbOk = true;
    return Next();
}

const String* ScStringTokenIterator::Next()
{
    if( !mbOk )
        return NULL;

    // seek to next non-separator token
    const FormulaToken* pToken = mrTokArr.NextNoSpaces();
    while( pToken && (pToken->GetOpCode() == ocSep) )
        pToken = mrTokArr.NextNoSpaces();

    mbOk = !pToken || (pToken->GetType() == formula::svString);
    const String* pString = (mbOk && pToken) ? &pToken->GetString() : NULL;
    // string found but empty -> get next token; otherwise return it
    return (mbSkipEmpty && pString && !pString->Len()) ? Next() : pString;
}

// ----------------------------------------------------------------------------

/** Returns the number format of the passed cell, or the standard format. */
sal_uLong lclGetCellFormat( ScDocument& rDoc, const ScAddress& rPos )
{
    const ScPatternAttr* pPattern = rDoc.GetPattern( rPos.Col(), rPos.Row(), rPos.Tab() );
    if( !pPattern )
        pPattern = rDoc.GetDefPattern();
    return pPattern->GetNumberFormat( rDoc.GetFormatTable() );
}

} // namespace

// ----------------------------------------------------------------------------

bool ScValidationData::HasSelectionList() const
{
    return (eDataMode == SC_VALID_LIST) && (mnListType != ValidListType::INVISIBLE);
}

bool ScValidationData::GetSelectionFromFormula(
    std::vector<ScTypedStrData>* pStrings, ScRefCellValue& rCell, const ScAddress& rPos,
    const ScTokenArray& rTokArr, int& rMatch) const
{
    bool bOk = true;

    // pDoc is private in condition, use an accessor and a long winded name.
    ScDocument* pDocument = GetDocument();
    if( NULL == pDocument )
        return false;

    ScFormulaCell aValidationSrc( pDocument, rPos, &rTokArr,
           formula::FormulaGrammar::GRAM_DEFAULT, MM_FORMULA);

    // Make sure the formula gets interpreted and a result is delivered,
    // regardless of the AutoCalc setting.
    aValidationSrc.Interpret();

    ScMatrixRef xMatRef;
    const ScMatrix *pValues = aValidationSrc.GetMatrix();
    if (!pValues)
    {
        // The somewhat nasty case of either an error occurred, or the
        // dereferenced value of a single cell reference or an immediate result
        // is stored as a single value.

        // Use an interim matrix to create the TypedStrData below.
        xMatRef = new ScMatrix(1, 1, 0.0);

        sal_uInt16 nErrCode = aValidationSrc.GetErrCode();
        if (nErrCode)
        {
            /* TODO : to use later in an alert box?
             * String rStrResult = "...";
             * rStrResult += ScGlobal::GetLongErrorString(nErrCode);
             */

            xMatRef->PutError( nErrCode, 0, 0);
            bOk = false;
        }
        else if (aValidationSrc.IsValue())
            xMatRef->PutDouble( aValidationSrc.GetValue(), 0);
        else
        {
            String aStr = aValidationSrc.GetString();
            xMatRef->PutString( aStr, 0);
        }

        pValues = xMatRef.get();
    }

    // which index matched.  We will want it eventually to pre-select that item.
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
            String               aValStr;
            ScMatrixValue nMatVal = pValues->Get( nCol, nRow);

            // strings and empties
            if( ScMatrix::IsNonValueType( nMatVal.nType ) )
            {
                aValStr = nMatVal.GetString();

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
                    // FIXME FIXME FIXME
                    // Feature regression.  Date formats are lost passing through the matrix
                    //pFormatter->GetInputLineString( pMatVal->fVal, 0, aValStr );
                    //For external reference and a formula that results in an area or array, date formats are still lost.
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
                    // I am not sure errors will work here, but a user can no
                    // manually enter an error yet so the point is somewhat moot.
                    aCondTokArr.AddDouble( nMatVal.fVal );
                }
                if( NULL != pStrings )
                    pEntry = new ScTypedStrData( aValStr, nMatVal.fVal, ScTypedStrData::Value);
            }

            if (rMatch < 0 && !rCell.isEmpty() && IsEqualToTokenArray(rCell, rPos, aCondTokArr))
            {
                rMatch = n;
                // short circuit on the first match if not filling the list
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

    // In case of no match needed and an error occurred, return that error
    // entry as valid instead of silently failing.
    return bOk || rCell.isEmpty();
}

bool ScValidationData::FillSelectionList(std::vector<ScTypedStrData>& rStrColl, const ScAddress& rPos) const
{
    bool bOk = false;

    if( HasSelectionList() )
    {
        boost::scoped_ptr<ScTokenArray> pTokArr( CreateTokenArry(0) );

        // *** try if formula is a string list ***

        sal_uInt32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
        ScStringTokenIterator aIt( *pTokArr );
        for( const String* pString = aIt.First(); pString && aIt.Ok(); pString = aIt.Next() )
        {
            double fValue;
            bool bIsValue = GetDocument()->GetFormatTable()->IsNumberFormat( *pString, nFormat, fValue );
            rStrColl.push_back(
                ScTypedStrData(
                    *pString, fValue, bIsValue ? ScTypedStrData::Value : ScTypedStrData::Standard));
        }
        bOk = aIt.Ok();

        // *** if not a string list, try if formula results in a cell range or
        // anything else we recognize as valid ***

        if (!bOk)
        {
            int nMatch;
            ScRefCellValue aEmptyCell;
            bOk = GetSelectionFromFormula(&rStrColl, aEmptyCell, rPos, *pTokArr, nMatch);
        }
    }

    return bOk;
}

// ----------------------------------------------------------------------------

bool ScValidationData::IsEqualToTokenArray( ScRefCellValue& rCell, const ScAddress& rPos, const ScTokenArray& rTokArr ) const
{
    // create a condition entry that tests on equality and set the passed token array
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

    // *** try if formula is a string list ***

    sal_uInt32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
    ScStringTokenIterator aIt( *pTokArr );
    for( const String* pString = aIt.First(); pString && aIt.Ok(); pString = aIt.Next() )
    {
        /*  Do not break the loop, if a valid string has been found.
            This is to find invalid tokens following in the formula. */
        if( !bIsValid )
        {
            // create a formula containing a single string or number
            ScTokenArray aCondTokArr;
            double fValue;
            if( GetDocument()->GetFormatTable()->IsNumberFormat( *pString, nFormat, fValue ) )
                aCondTokArr.AddDouble( fValue );
            else
                aCondTokArr.AddString( *pString );

            bIsValid = IsEqualToTokenArray(rCell, rPos, aCondTokArr);
        }
    }

    if( !aIt.Ok() )
        bIsValid = false;

    // *** if not a string list, try if formula results in a cell range or
    // anything else we recognize as valid ***

    if (!bIsValid)
    {
        int nMatch;
        bIsValid = GetSelectionFromFormula(NULL, rCell, rPos, *pTokArr, nMatch);
        bIsValid = bIsValid && nMatch >= 0;
    }

    return bIsValid;
}

// ============================================================================
// ============================================================================

ScValidationDataList::ScValidationDataList(const ScValidationDataList& rList) :
    std::set<ScValidationData*, CompareScValidationDataPtr>()
{
    //  fuer Ref-Undo - echte Kopie mit neuen Tokens!

    for (const_iterator it = rList.begin(); it != rList.end(); ++it)
    {
        InsertNew( (*it)->Clone() );
    }

    //!     sortierte Eintraege aus rList schneller einfuegen ???
}

ScValidationDataList::ScValidationDataList(ScDocument* pNewDoc,
                                            const ScValidationDataList& rList)
{
    //  fuer neues Dokument - echte Kopie mit neuen Tokens!

    for (const_iterator it = rList.begin(); it != rList.end(); ++it)
    {
        InsertNew( (*it)->Clone(pNewDoc) );
    }

    //!     sortierte Eintraege aus rList schneller einfuegen ???
}

ScValidationData* ScValidationDataList::GetData( sal_uInt32 nKey )
{
    //! binaer suchen

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

sal_Bool ScValidationDataList::operator==( const ScValidationDataList& r ) const
{
    // fuer Ref-Undo - interne Variablen werden nicht verglichen

    sal_uInt16 nCount = size();
    sal_Bool bEqual = ( nCount == r.size() );
    for( const_iterator it1 = begin(), it2 = r.begin(); it1 != end() && bEqual; ++it1, ++it2 ) // Eintraege sind sortiert
        if ( !(*it1)->EqualEntries(**it2) )         // Eintraege unterschiedlich ?
            bEqual = sal_False;

    return bEqual;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

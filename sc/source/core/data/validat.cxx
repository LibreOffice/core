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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
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
#include <tools/urlobj.hxx>
#include <rtl/math.hxx>

#include "validat.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "rangenam.hxx"
#include "dbcolect.hxx"

#include <math.h>
#include <memory>

using namespace formula;
//------------------------------------------------------------------------

SV_IMPL_OP_PTRARR_SORT( ScValidationEntries_Impl, ScValidationDataPtr );

//------------------------------------------------------------------------

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
    bShowInput = bShowError = FALSE;
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
    bShowInput = bShowError = FALSE;
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

BOOL ScValidationData::IsEmpty() const
{
    String aEmpty;
    ScValidationData aDefault( SC_VALID_ANY, SC_COND_EQUAL, aEmpty, aEmpty, GetDocument(), ScAddress() );
    return EqualEntries( aDefault );
}

BOOL ScValidationData::EqualEntries( const ScValidationData& r ) const
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
    bShowInput = FALSE;
}

void ScValidationData::ResetError()
{
    bShowError = FALSE;
}

void ScValidationData::SetInput( const String& rTitle, const String& rMsg )
{
    bShowInput = TRUE;
    aInputTitle = rTitle;
    aInputMessage = rMsg;
}

void ScValidationData::SetError( const String& rTitle, const String& rMsg,
                                    ScValidErrorStyle eStyle )
{
    bShowError = TRUE;
    eErrorStyle = eStyle;
    aErrorTitle = rTitle;
    aErrorMessage = rMsg;
}

BOOL ScValidationData::GetErrMsg( String& rTitle, String& rMsg,
                                    ScValidErrorStyle& rStyle ) const
{
    rTitle = aErrorTitle;
    rMsg   = aErrorMessage;
    rStyle = eErrorStyle;
    return bShowError;
}

BOOL ScValidationData::DoScript( const ScAddress& rPos, const String& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const
{
    ScDocument* pDocument = GetDocument();
    SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
    if ( !pDocSh || !pDocument->CheckMacroWarn() )
        return FALSE;

    BOOL bScriptReturnedFalse = FALSE;  // Standard: kein Abbruch

    // Set up parameters
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aParams(2);

    //  1) eingegebener / berechneter Wert
    String aValStr = rInput;
    double nValue;
    BOOL bIsValue = FALSE;
    if ( pCell )                // wenn Zelle gesetzt, aus Interpret gerufen
    {
        bIsValue = pCell->IsValue();
        if ( bIsValue )
            nValue  = pCell->GetValue();
        else
            pCell->GetString( aValStr );
    }
    if ( bIsValue )
        aParams[0] = ::com::sun::star::uno::makeAny( nValue );
    else
        aParams[0] = ::com::sun::star::uno::makeAny( ::rtl::OUString( aValStr ) );

    //  2) Position der Zelle
    String aPosStr;
    rPos.Format( aPosStr, SCA_VALID | SCA_TAB_3D, pDocument, pDocument->GetAddressConvention() );
    aParams[1] = ::com::sun::star::uno::makeAny( ::rtl::OUString( aPosStr ) );

    //  use link-update flag to prevent closing the document
    //  while the macro is running
    BOOL bWasInLinkUpdate = pDocument->IsInLinkUpdate();
    if ( !bWasInLinkUpdate )
        pDocument->SetInLinkUpdate( TRUE );

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
        pDocument->SetInLinkUpdate( FALSE );

    // Check the return value from the script
    // The contents of the cell get reset if the script returns false
    BOOL bTmp = FALSE;
    if ( eRet == ERRCODE_NONE &&
             aRet.getValueType() == getCppuBooleanType() &&
             sal_True == ( aRet >>= bTmp ) &&
             bTmp == FALSE )
    {
        bScriptReturnedFalse = TRUE;
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

    // TRUE -> Abbruch

BOOL ScValidationData::DoMacro( const ScAddress& rPos, const String& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const
{
    if ( SfxApplication::IsXScriptURL( aErrorTitle ) )
    {
        return DoScript( rPos, rInput, pCell, pParent );
    }

    ScDocument* pDocument = GetDocument();
    SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
    if ( !pDocSh || !pDocument->CheckMacroWarn() )
        return FALSE;

    BOOL bDone = FALSE;
    BOOL bRet = FALSE;                      // Standard: kein Abbruch
    SfxApplication* pSfxApp = SFX_APP();
    pSfxApp->EnterBasicCall();              // Dok-Basic anlegen etc.

    //  Wenn das Dok waehrend eines Basic-Calls geladen wurde,
    //  ist das Sbx-Objekt evtl. nicht angelegt (?)
//  pDocSh->GetSbxObject();

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
        BOOL bIsValue = FALSE;
        if ( pCell )                // wenn Zelle gesetzt, aus Interpret gerufen
        {
            bIsValue = pCell->IsValue();
            if ( bIsValue )
                nValue  = pCell->GetValue();
            else
                pCell->GetString( aValStr );
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
        BOOL bWasInLinkUpdate = pDocument->IsInLinkUpdate();
        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( TRUE );

        if ( pCell )
            pDocument->LockTable( rPos.Tab() );
        SbxVariableRef refRes = new SbxVariable;
        ErrCode eRet = pDocSh->CallBasic( aMacroStr, aBasicStr, NULL, refPar, refRes );
        if ( pCell )
            pDocument->UnlockTable( rPos.Tab() );

        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( FALSE );

        //  Eingabe abbrechen, wenn Basic-Makro FALSE zurueckgibt
        if ( eRet == ERRCODE_NONE && refRes->GetType() == SbxBOOL && refRes->GetBool() == FALSE )
            bRet = TRUE;
        bDone = TRUE;
    }
    pSfxApp->LeaveBasicCall();

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

    // TRUE -> Abbruch

BOOL ScValidationData::DoError( Window* pParent, const String& rInput,
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
    USHORT nRet = aBox.Execute();

    return ( eErrorStyle == SC_VALERR_STOP || nRet == RET_CANCEL );
}


BOOL ScValidationData::IsDataValid( const String& rTest, const ScPatternAttr& rPattern,
                                    const ScAddress& rPos ) const
{
    if ( eDataMode == SC_VALID_ANY )
        return TRUE;                        // alles erlaubt

    if ( rTest.GetChar(0) == '=' )
        return FALSE;                       // Formeln sind sonst immer ungueltig

    if ( !rTest.Len() )
        return IsIgnoreBlank();             // leer: wie eingestellt

    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    //  Test, was es denn ist - wie in ScColumn::SetString

    sal_uInt32 nFormat = rPattern.GetNumberFormat( pFormatter );

    double nVal;
    BOOL bIsVal = pFormatter->IsNumberFormat( rTest, nFormat, nVal );
    ScBaseCell* pCell;
    if (bIsVal)
        pCell = new ScValueCell( nVal );
    else
        pCell = new ScStringCell( rTest );

    BOOL bRet = IsDataValid( pCell, rPos );

    pCell->Delete();
    return bRet;
}

BOOL ScValidationData::IsDataValid( ScBaseCell* pCell, const ScAddress& rPos ) const
{
    if( eDataMode == SC_VALID_LIST )
        return IsListValid( pCell, rPos );

    double nVal = 0.0;
    String aString;
    BOOL bIsVal = TRUE;

    switch (pCell->GetCellType())
    {
        case CELLTYPE_VALUE:
            nVal = ((ScValueCell*)pCell)->GetValue();
            break;
        case CELLTYPE_STRING:
            ((ScStringCell*)pCell)->GetString( aString );
            bIsVal = FALSE;
            break;
        case CELLTYPE_EDIT:
            ((ScEditCell*)pCell)->GetString( aString );
            bIsVal = FALSE;
            break;
        case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                bIsVal = pFCell->IsValue();
                if ( bIsVal )
                    nVal  = pFCell->GetValue();
                else
                    pFCell->GetString( aString );
            }
            break;
        default:                        // Notizen, Broadcaster
            return IsIgnoreBlank();     // wie eingestellt
    }

    BOOL bOk = TRUE;
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
                bOk = IsCellValid( pCell, rPos );
            break;

        case SC_VALID_CUSTOM:
            //  fuer Custom muss eOp == SC_COND_DIRECT sein
            //! der Wert muss im Dokument stehen !!!!!!!!!!!!!!!!!!!!
            bOk = IsCellValid( pCell, rPos );
            break;

        case SC_VALID_TEXTLEN:
            bOk = !bIsVal;          // nur Text
            if ( bOk )
            {
                double nLenVal = (double) aString.Len();
                ScValueCell aTmpCell( nLenVal );
                bOk = IsCellValid( &aTmpCell, rPos );
            }
            break;

        default:
            OSL_FAIL("hammanochnich");
            break;
    }

    return bOk;
}

// ----------------------------------------------------------------------------

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
ULONG lclGetCellFormat( ScDocument& rDoc, const ScAddress& rPos )
{
    const ScPatternAttr* pPattern = rDoc.GetPattern( rPos.Col(), rPos.Row(), rPos.Tab() );
    if( !pPattern )
        pPattern = rDoc.GetDefPattern();
    return pPattern->GetNumberFormat( rDoc.GetFormatTable() );
}

/** Inserts the passed string object. Always takes ownership. pData is invalid after this call! */
void lclInsertStringToCollection( TypedScStrCollection& rStrColl, TypedStrData* pData, bool bSorted )
{
    if( !(bSorted ? rStrColl.Insert( pData ) : rStrColl.AtInsert( rStrColl.GetCount(), pData )) )
        delete pData;
}

} // namespace

// ----------------------------------------------------------------------------

bool ScValidationData::HasSelectionList() const
{
    return (eDataMode == SC_VALID_LIST) && (mnListType != ValidListType::INVISIBLE);
}

bool ScValidationData::GetSelectionFromFormula( TypedScStrCollection* pStrings,
                                                ScBaseCell* pCell,
                                                const ScAddress& rPos,
                                                const ScTokenArray& rTokArr,
                                                int& rMatch ) const
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
        xMatRef = new ScMatrix(1,1);

        USHORT nErrCode = aValidationSrc.GetErrCode();
        if (nErrCode)
        {
            /* TODO : to use later in an alert box?
             * String rStrResult = "...";
             * rStrResult += ScGlobal::GetLongErrorString(nErrCode);
             */

            xMatRef->PutError( nErrCode, 0, 0);
            bOk = false;
        }
        else if (aValidationSrc.HasValueData())
            xMatRef->PutDouble( aValidationSrc.GetValue(), 0);
        else
        {
            String aStr;
            aValidationSrc.GetString( aStr);
            xMatRef->PutString( aStr, 0);
        }

        pValues = xMatRef.get();
    }

    // which index matched.  We will want it eventually to pre-select that item.
    rMatch = -1;

    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    bool    bSortList = (mnListType == ValidListType::SORTEDASCENDING);
    SCSIZE  nCol, nRow, nCols, nRows, n = 0;
    pValues->GetDimensions( nCols, nRows );

    BOOL bRef = FALSE;
    ScRange aRange;

    ScTokenArray* pArr = (ScTokenArray*) &rTokArr;
    pArr->Reset();
    ScToken* t = NULL;
    if (pArr->GetLen() == 1 && (t = static_cast<ScToken*>(pArr->GetNextReferenceOrName())) != NULL)
    {
        if (t->GetOpCode() == ocDBArea)
        {
            if( ScDBData* pDBData = pDocument->GetDBCollection()->FindIndex( t->GetIndex() ) )
            {
                pDBData->GetArea(aRange);
                bRef = TRUE;
            }
        }
        else if (t->GetOpCode() == ocName)
        {
            ScRangeData* pName = pDocument->GetRangeName()->FindIndex( t->GetIndex() );
            if (pName && pName->IsReference(aRange))
            {
                bRef = TRUE;
            }
        }
        else if (t->GetType() != svIndex)
        {
            t->CalcAbsIfRel(rPos);
            if (pArr->IsValidReference(aRange))
            {
                bRef = TRUE;
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
            TypedStrData*        pEntry = NULL;
            String               aValStr;
            ScMatrixValue nMatVal = pValues->Get( nCol, nRow);

            // strings and empties
            if( ScMatrix::IsNonValueType( nMatVal.nType ) )
            {
                aValStr = nMatVal.GetString();

                if( NULL != pStrings )
                    pEntry = new TypedStrData( aValStr, 0.0, SC_STRTYPE_STANDARD);

                if( pCell && rMatch < 0 )
                    aCondTokArr.AddString( aValStr );
            }
            else
            {
                USHORT nErr = nMatVal.GetError();

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
                        pFormatter->GetInputLineString( nMatVal.fVal, 0, aValStr );
                }

                if( pCell && rMatch < 0 )
                {
                    // I am not sure errors will work here, but a user can no
                    // manually enter an error yet so the point is somewhat moot.
                    aCondTokArr.AddDouble( nMatVal.fVal );
                }
                if( NULL != pStrings )
                    pEntry = new TypedStrData( aValStr, nMatVal.fVal, SC_STRTYPE_VALUE);
            }

            if( rMatch < 0 && NULL != pCell && IsEqualToTokenArray( pCell, rPos, aCondTokArr ) )
            {
                rMatch = n;
                // short circuit on the first match if not filling the list
                if( NULL == pStrings )
                    return true;
            }

            if( NULL != pEntry )
            {
                lclInsertStringToCollection( *pStrings, pEntry, bSortList );
                n++;
            }
        }
    }

    // In case of no match needed and an error occurred, return that error
    // entry as valid instead of silently failing.
    return bOk || NULL == pCell;
}

bool ScValidationData::FillSelectionList( TypedScStrCollection& rStrColl, const ScAddress& rPos ) const
{
    bool bOk = false;

    if( HasSelectionList() )
    {
        ::std::auto_ptr< ScTokenArray > pTokArr( CreateTokenArry( 0 ) );

        // *** try if formula is a string list ***

        bool bSortList = (mnListType == ValidListType::SORTEDASCENDING);
        UINT32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
        ScStringTokenIterator aIt( *pTokArr );
        for( const String* pString = aIt.First(); pString && aIt.Ok(); pString = aIt.Next() )
        {
            double fValue;
            bool bIsValue = GetDocument()->GetFormatTable()->IsNumberFormat( *pString, nFormat, fValue );
            TypedStrData* pData = new TypedStrData( *pString, fValue, bIsValue ? SC_STRTYPE_VALUE : SC_STRTYPE_STANDARD );
            lclInsertStringToCollection( rStrColl, pData, bSortList );
        }
        bOk = aIt.Ok();

        // *** if not a string list, try if formula results in a cell range or
        // anything else we recognize as valid ***

        if (!bOk)
        {
            int nMatch;
            bOk = GetSelectionFromFormula( &rStrColl, NULL, rPos, *pTokArr, nMatch );
        }
    }

    return bOk;
}

// ----------------------------------------------------------------------------

bool ScValidationData::IsEqualToTokenArray( ScBaseCell* pCell, const ScAddress& rPos, const ScTokenArray& rTokArr ) const
{
    // create a condition entry that tests on equality and set the passed token array
    ScConditionEntry aCondEntry( SC_COND_EQUAL, &rTokArr, NULL, GetDocument(), rPos );
    return aCondEntry.IsCellValid( pCell, rPos );
}

bool ScValidationData::IsListValid( ScBaseCell* pCell, const ScAddress& rPos ) const
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

    ::std::auto_ptr< ScTokenArray > pTokArr( CreateTokenArry( 0 ) );

    // *** try if formula is a string list ***

    UINT32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
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

            bIsValid = IsEqualToTokenArray( pCell, rPos, aCondTokArr );
        }
    }

    if( !aIt.Ok() )
        bIsValid = false;

    // *** if not a string list, try if formula results in a cell range or
    // anything else we recognize as valid ***

    if (!bIsValid)
    {
        int nMatch;
        bIsValid = GetSelectionFromFormula( NULL, pCell, rPos, *pTokArr, nMatch );
        bIsValid = bIsValid && nMatch >= 0;
    }

    return bIsValid;
}

// ============================================================================
// ============================================================================

ScValidationDataList::ScValidationDataList(const ScValidationDataList& rList) :
    ScValidationEntries_Impl()
{
    //  fuer Ref-Undo - echte Kopie mit neuen Tokens!

    USHORT nCount = rList.Count();

    for (USHORT i=0; i<nCount; i++)
        InsertNew( rList[i]->Clone() );

    //!     sortierte Eintraege aus rList schneller einfuegen ???
}

ScValidationDataList::ScValidationDataList(ScDocument* pNewDoc,
                                            const ScValidationDataList& rList)
{
    //  fuer neues Dokument - echte Kopie mit neuen Tokens!

    USHORT nCount = rList.Count();

    for (USHORT i=0; i<nCount; i++)
        InsertNew( rList[i]->Clone(pNewDoc) );

    //!     sortierte Eintraege aus rList schneller einfuegen ???
}

ScValidationData* ScValidationDataList::GetData( sal_uInt32 nKey )
{
    //! binaer suchen

    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
        if ((*this)[i]->GetKey() == nKey)
            return (*this)[i];

    OSL_FAIL("ScValidationDataList: Eintrag nicht gefunden");
    return NULL;
}

void ScValidationDataList::CompileXML()
{
    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
        (*this)[i]->CompileXML();
}

void ScValidationDataList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
        (*this)[i]->UpdateReference( eUpdateRefMode, rRange, nDx, nDy, nDz);
}

void ScValidationDataList::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
        (*this)[i]->UpdateMoveTab( nOldPos, nNewPos );
}

BOOL ScValidationDataList::operator==( const ScValidationDataList& r ) const
{
    // fuer Ref-Undo - interne Variablen werden nicht verglichen

    USHORT nCount = Count();
    BOOL bEqual = ( nCount == r.Count() );
    for (USHORT i=0; i<nCount && bEqual; i++)           // Eintraege sind sortiert
        if ( !(*this)[i]->EqualEntries(*r[i]) )         // Eintraege unterschiedlich ?
            bEqual = FALSE;

    return bEqual;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

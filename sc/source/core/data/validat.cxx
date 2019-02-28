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

#include <config_features.h>

#include <validat.hxx>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <basic/sberrors.hxx>

#include <basic/sbx.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
#include <vcl/weld.hxx>
#include <rtl/math.hxx>
#include <osl/diagnose.h>

#include <document.hxx>
#include <formulacell.hxx>
#include <patattr.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <typedstrdata.hxx>
#include <editutil.hxx>
#include <tokenarray.hxx>
#include <scmatrix.hxx>
#include <cellvalue.hxx>

#include <math.h>
#include <memory>

using namespace formula;

//  Entries for validation (with only one condition)

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
    , mnListType( css::sheet::TableValidationVisibility::UNSORTED )
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
    , mnListType( css::sheet::TableValidationVisibility::UNSORTED )
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
    //  Formulae copied by RefCount
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
    //  Formulae really copied
}

ScValidationData::~ScValidationData()
{
}

bool ScValidationData::IsEmpty() const
{
    ScValidationData aDefault( SC_VALID_ANY, ScConditionMode::Equal, "", "", GetDocument(), ScAddress() );
    return EqualEntries( aDefault );
}

bool ScValidationData::EqualEntries( const ScValidationData& r ) const
{
        //  test same parameters (excluding Key)

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
                                ScFormulaCell* pCell, weld::Window* pParent ) const
{
    ScDocument* pDocument = GetDocument();
    SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
    if ( !pDocSh )
        return false;

    bool bScriptReturnedFalse = false;  // default: do not abort

    // Set up parameters
    css::uno::Sequence< css::uno::Any > aParams(2);

    //  1) entered or calculated value
    OUString aValStr = rInput;
    double nValue;
    bool bIsValue = false;
    if ( pCell )                // if cell exists, call interpret
    {
        bIsValue = pCell->IsValue();
        if ( bIsValue )
            nValue  = pCell->GetValue();
        else
            aValStr = pCell->GetString().getString();
    }
    if ( bIsValue )
        aParams[0] <<= nValue;
    else
        aParams[0] <<= aValStr;

    //  2) Position of the cell
    OUString aPosStr(rPos.Format(ScRefFlags::VALID | ScRefFlags::TAB_3D, pDocument, pDocument->GetAddressConvention()));
    aParams[1] <<= aPosStr;

    //  use link-update flag to prevent closing the document
    //  while the macro is running
    bool bWasInLinkUpdate = pDocument->IsInLinkUpdate();
    if ( !bWasInLinkUpdate )
        pDocument->SetInLinkUpdate( true );

    if ( pCell )
        pDocument->LockTable( rPos.Tab() );

    css::uno::Any aRet;
    css::uno::Sequence< sal_Int16 > aOutArgsIndex;
    css::uno::Sequence< css::uno::Any > aOutArgs;

    ErrCode eRet = pDocSh->CallXScript(
        aErrorTitle, aParams, aRet, aOutArgsIndex, aOutArgs );

    if ( pCell )
        pDocument->UnlockTable( rPos.Tab() );

    if ( !bWasInLinkUpdate )
        pDocument->SetInLinkUpdate( false );

    // Check the return value from the script
    // The contents of the cell get reset if the script returns false
    bool bTmp = false;
    if ( eRet == ERRCODE_NONE &&
             aRet.getValueType() == cppu::UnoType<bool>::get() &&
             ( aRet >>= bTmp ) &&
             !bTmp )
    {
        bScriptReturnedFalse =  true;
    }

    if ( eRet == ERRCODE_BASIC_METHOD_NOT_FOUND && !pCell )
    // Macro not found (only with input)
    {
        //TODO: different error message, if found, but not bAllowed ??
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  ScResId(STR_VALID_MACRONOTFOUND)));
        xBox->run();
    }

    return bScriptReturnedFalse;
}

    // true -> abort

bool ScValidationData::DoMacro( const ScAddress& rPos, const OUString& rInput,
                                ScFormulaCell* pCell, weld::Window* pParent ) const
{
    if ( SfxApplication::IsXScriptURL( aErrorTitle ) )
    {
        return DoScript( rPos, rInput, pCell, pParent );
    }

    ScDocument* pDocument = GetDocument();
    SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
    if ( !pDocSh )
        return false;

    bool bDone = false;
    bool bRet = false;                      // default: do not abort

    //  If the Doc was loaded during a Basic-Calls,
    //  the Sbx-object may not be created (?)
//  pDocSh->GetSbxObject();

#if HAVE_FEATURE_SCRIPTING
    //  no security check ahead (only CheckMacroWarn), that happens in CallBasic

    //  Function search by their simple name,
    //  then assemble aBasicStr, aMacroStr for SfxObjectShell::CallBasic

    StarBASIC* pRoot = pDocSh->GetBasic();
    SbxVariable* pVar = pRoot->Find( aErrorTitle, SbxClassType::Method );
    if (SbMethod* pMethod = dynamic_cast<SbMethod*>(pVar))
    {
        SbModule* pModule = pMethod->GetModule();
        SbxObject* pObject = pModule->GetParent();
        OUStringBuffer aMacroStr = pObject->GetName();
        aMacroStr.append('.').append(pModule->GetName()).append('.').append(pMethod->GetName());
        OUString aBasicStr;

        //  the distinction between document- and app-basic has to be done
        //  by checking the parent (as in ScInterpreter::ScMacro), not by looping
        //  over all open documents, because this may be called from within loading,
        //  when SfxObjectShell::GetFirst/GetNext won't find the document.

        if ( pObject->GetParent() )
            aBasicStr = pObject->GetParent()->GetName();    // Basic of document
        else
            aBasicStr = SfxGetpApp()->GetName();            // Basic of application

        //  Parameter for Macro
        SbxArrayRef refPar = new SbxArray;

        //  1) entered or calculated value
        OUString aValStr = rInput;
        double nValue = 0.0;
        bool bIsValue = false;
        if ( pCell )                // if cell set, called from interpret
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

        //  2) Position of the cell
        OUString aPosStr(rPos.Format(ScRefFlags::VALID | ScRefFlags::TAB_3D, pDocument, pDocument->GetAddressConvention()));
        refPar->Get(2)->PutString( aPosStr );

        //  use link-update flag to prevent closing the document
        //  while the macro is running
        bool bWasInLinkUpdate = pDocument->IsInLinkUpdate();
        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( true );

        if ( pCell )
            pDocument->LockTable( rPos.Tab() );
        SbxVariableRef refRes = new SbxVariable;
        ErrCode eRet = pDocSh->CallBasic( aMacroStr.makeStringAndClear(), aBasicStr, refPar.get(), refRes.get() );
        if ( pCell )
            pDocument->UnlockTable( rPos.Tab() );

        if ( !bWasInLinkUpdate )
            pDocument->SetInLinkUpdate( false );

        //  Interrupt input if Basic macro returns false
        if ( eRet == ERRCODE_NONE && refRes->GetType() == SbxBOOL && !refRes->GetBool() )
            bRet = true;
        bDone = true;
    }
#endif
    if ( !bDone && !pCell )         // Macro not found (only with input)
    {
        //TODO: different error message, if found, but not bAllowed ??
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  ScResId(STR_VALID_MACRONOTFOUND)));
        xBox->run();
    }

    return bRet;
}

void ScValidationData::DoCalcError( ScFormulaCell* pCell ) const
{
    if ( eErrorStyle == SC_VALERR_MACRO )
        DoMacro( pCell->aPos, EMPTY_OUSTRING, pCell, nullptr );
}

    // true -> abort

bool ScValidationData::DoError(weld::Window* pParent, const OUString& rInput,
                               const ScAddress& rPos) const
{
    if ( eErrorStyle == SC_VALERR_MACRO )
        return DoMacro(rPos, rInput, nullptr, pParent);

    //  Output error message

    OUString aTitle = aErrorTitle;
    if (aTitle.isEmpty())
        aTitle = ScResId( STR_MSSG_DOSUBTOTALS_0 );  // application title
    OUString aMessage = aErrorMessage;
    if (aMessage.isEmpty())
        aMessage = ScResId( STR_VALID_DEFERROR );

    VclButtonsType eStyle = VclButtonsType::Ok;
    VclMessageType eType = VclMessageType::Error;
    switch (eErrorStyle)
    {
        case SC_VALERR_INFO:
            eType = VclMessageType::Info;
            eStyle = VclButtonsType::OkCancel;
            break;
        case SC_VALERR_WARNING:
            eType = VclMessageType::Warning;
            eStyle = VclButtonsType::OkCancel;
            break;
        default:
            break;
    }

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent, eType,
                                              eStyle, aMessage));
    xBox->set_title(aTitle);

    switch (eErrorStyle)
    {
        case SC_VALERR_INFO:
            xBox->set_default_response(RET_OK);
            break;
        case SC_VALERR_WARNING:
            xBox->set_default_response(RET_CANCEL);
            break;
        default:
            break;
    }

    short nRet = xBox->run();

    return ( eErrorStyle == SC_VALERR_STOP || nRet == RET_CANCEL );
}

bool ScValidationData::IsDataValidCustom(
        const OUString& rTest,
        const ScPatternAttr& rPattern,
        const ScAddress& rPos,
        const CustomValidationPrivateAccess& ) const
{
    OSL_ENSURE(GetDataMode() == SC_VALID_CUSTOM,
            "ScValidationData::IsDataValidCustom invoked for a non-custom validation");

    if (rTest.isEmpty())              // check whether empty cells are allowed
        return IsIgnoreBlank();

    if (rTest[0] == '=')   // formulas do not pass the validity test
        return false;

    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    // get the value if any
    sal_uInt32 nFormat = rPattern.GetNumberFormat( pFormatter );
    double nVal;
    bool bIsVal = pFormatter->IsNumberFormat( rTest, nFormat, nVal );

    ScRefCellValue aTmpCell;
    svl::SharedString aSS;
    if (bIsVal)
    {
        aTmpCell.meType = CELLTYPE_VALUE;
        aTmpCell.mfValue = nVal;
    }
    else
    {
        aTmpCell.meType = CELLTYPE_STRING;
        aSS = mpDoc->GetSharedStringPool().intern(rTest);
        aTmpCell.mpString = &aSS;
    }

    ScCellValue aOriginalCellValue(ScRefCellValue(*GetDocument(), rPos));

    aTmpCell.commit(*GetDocument(), rPos);
    bool bRet = IsCellValid(aTmpCell, rPos);
    aOriginalCellValue.commit(*GetDocument(), rPos);

    return bRet;
}

bool ScValidationData::IsDataValid(
    const OUString& rTest, const ScPatternAttr& rPattern, const ScAddress& rPos ) const
{
    if ( eDataMode == SC_VALID_ANY ) // check if any cell content is allowed
        return true;

    if (rTest.isEmpty())              // check whether empty cells are allowed
        return IsIgnoreBlank();

    if (rTest[0] == '=')   // formulas do not pass the validity test
        return false;

    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    // get the value if any
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

    if ( eDataMode == SC_VALID_CUSTOM )
        return IsCellValid(rCell, rPos);

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
        default:                        // Notes, Broadcaster
            return IsIgnoreBlank();     // as set
    }

    bool bOk = true;
    switch (eDataMode)
    {
        // SC_VALID_ANY already above

        case SC_VALID_WHOLE:
        case SC_VALID_DECIMAL:
        case SC_VALID_DATE:         // Date/Time is only formatting
        case SC_VALID_TIME:
            bOk = bIsVal;
            if ( bOk && eDataMode == SC_VALID_WHOLE )
                bOk = ::rtl::math::approxEqual( nVal, floor(nVal+0.5) );        // integers
            if ( bOk )
                bOk = IsCellValid(rCell, rPos);
            break;

        case SC_VALID_TEXTLEN:
            bOk = !bIsVal;          // only Text
            if ( bOk )
            {
                double nLenVal = static_cast<double>(aString.getLength());
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
    explicit             ScStringTokenIterator( const ScTokenArray& rTokArr ) :
        maIter( rTokArr ), mbOk( true ) {}

    /** Returns the string of the first string token or NULL on error or empty token array. */
    rtl_uString* First();
    /** Returns the string of the next string token or NULL on error or end of token array. */
    rtl_uString* Next();

    /** Returns false, if a wrong token has been found. Does NOT return false on end of token array. */
    bool                 Ok() const { return mbOk; }

private:
    svl::SharedString maCurString; /// Current string.
    FormulaTokenArrayPlainIterator maIter;
    bool                        mbOk;           /// true = correct token or end of token array.
};

rtl_uString* ScStringTokenIterator::First()
{
    maIter.Reset();
    mbOk = true;
    return Next();
}

rtl_uString* ScStringTokenIterator::Next()
{
    if( !mbOk )
        return nullptr;

    // seek to next non-separator token
    const FormulaToken* pToken = maIter.NextNoSpaces();
    while( pToken && (pToken->GetOpCode() == ocSep) )
        pToken = maIter.NextNoSpaces();

    mbOk = !pToken || (pToken->GetType() == formula::svString);

    maCurString = svl::SharedString(); // start with invalid string.
    if (mbOk && pToken)
        maCurString = pToken->GetString();

    // string found but empty -> get next token; otherwise return it
    return (maCurString.isValid() && maCurString.isEmpty()) ? Next() : maCurString.getData();
}

/** Returns the number format of the passed cell, or the standard format. */
sal_uLong lclGetCellFormat( const ScDocument& rDoc, const ScAddress& rPos )
{
    const ScPatternAttr* pPattern = rDoc.GetPattern( rPos.Col(), rPos.Row(), rPos.Tab() );
    if( !pPattern )
        pPattern = rDoc.GetDefPattern();
    return pPattern->GetNumberFormat( rDoc.GetFormatTable() );
}

} // namespace

bool ScValidationData::HasSelectionList() const
{
    return (eDataMode == SC_VALID_LIST) && (mnListType != css::sheet::TableValidationVisibility::INVISIBLE);
}

bool ScValidationData::GetSelectionFromFormula(
    std::vector<ScTypedStrData>* pStrings, ScRefCellValue& rCell, const ScAddress& rPos,
    const ScTokenArray& rTokArr, int& rMatch) const
{
    bool bOk = true;

    // pDoc is private in condition, use an accessor and a long winded name.
    ScDocument* pDocument = GetDocument();
    if( nullptr == pDocument )
        return false;

    ScFormulaCell aValidationSrc(
        pDocument, rPos, rTokArr, formula::FormulaGrammar::GRAM_DEFAULT, ScMatrixMode::Formula);

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

        FormulaError nErrCode = aValidationSrc.GetErrCode();
        if (nErrCode != FormulaError::NONE)
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

    // which index matched.  We will want it eventually to pre-select that item.
    rMatch = -1;

    SvNumberFormatter* pFormatter = GetDocument()->GetFormatTable();

    SCSIZE  nCol, nRow, nCols, nRows, n = 0;
    pValues->GetDimensions( nCols, nRows );

    bool bRef = false;
    ScRange aRange;

    ScTokenArray* pArr = const_cast<ScTokenArray*>(&rTokArr);
    formula::FormulaTokenArrayPlainIterator aIter(*pArr);
    formula::FormulaToken* t = nullptr;
    if (pArr->GetLen() == 1 && (t = aIter.GetNextReferenceOrName()) != nullptr)
    {
        OpCode eOpCode = t->GetOpCode();
        if (eOpCode == ocDBArea || eOpCode == ocTableRef)
        {
            if (const ScDBData* pDBData = pDocument->GetDBCollection()->getNamedDBs().findByIndex(t->GetIndex()))
            {
                pDBData->GetArea(aRange);
                bRef = true;
            }
        }
        else if (eOpCode == ocName)
        {
            const ScRangeData* pName = pDocument->FindRangeNameBySheetAndIndex( t->GetSheet(), t->GetIndex());
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

    bool bHaveEmpty = false;
    svl::SharedStringPool& rSPool = pDocument->GetSharedStringPool();

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
            std::unique_ptr<ScTypedStrData> pEntry;
            OUString               aValStr;
            ScMatrixValue nMatVal = pValues->Get( nCol, nRow);

            // strings and empties
            if( ScMatrix::IsNonValueType( nMatVal.nType ) )
            {
                aValStr = nMatVal.GetString().getString();

                // Do not add multiple empty strings to the validation list,
                // especially not if they'd bloat the tail with a million empty
                // entries for a column range, fdo#61520
                if (aValStr.isEmpty())
                {
                    if (bHaveEmpty)
                        continue;
                    bHaveEmpty = true;
                }

                if( nullptr != pStrings )
                    pEntry.reset(new ScTypedStrData( aValStr, 0.0, ScTypedStrData::Standard));

                if (!rCell.isEmpty() && rMatch < 0)
                    aCondTokArr.AddString(rSPool.intern(aValStr));
            }
            else
            {
                FormulaError nErr = nMatVal.GetError();

                if( FormulaError::NONE != nErr )
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
                        pDocument->GetInputString(static_cast<SCCOL>(nCol+aRange.aStart.Col()),
                            static_cast<SCROW>(nRow+aRange.aStart.Row()), aRange.aStart.Tab() , aValStr);
                    }
                    else
                    {
                        pFormatter->GetInputLineString( nMatVal.fVal, 0, aValStr );
                    }
                }

                if (!rCell.isEmpty() && rMatch < 0)
                {
                    // I am not sure errors will work here, but a user can no
                    // manually enter an error yet so the point is somewhat moot.
                    aCondTokArr.AddDouble( nMatVal.fVal );
                }
                if( nullptr != pStrings )
                    pEntry.reset(new ScTypedStrData( aValStr, nMatVal.fVal, ScTypedStrData::Value));
            }

            if (rMatch < 0 && !rCell.isEmpty() && IsEqualToTokenArray(rCell, rPos, aCondTokArr))
            {
                rMatch = n;
                // short circuit on the first match if not filling the list
                if( nullptr == pStrings )
                    return true;
            }

            if( pEntry )
            {
                pStrings->push_back(*pEntry);
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
        std::unique_ptr<ScTokenArray> pTokArr( CreateFlatCopiedTokenArray(0) );

        // *** try if formula is a string list ***

        sal_uInt32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
        ScStringTokenIterator aIt( *pTokArr );
        for (rtl_uString* pString = aIt.First(); pString && aIt.Ok(); pString = aIt.Next())
        {
            double fValue;
            OUString aStr(pString);
            bool bIsValue = GetDocument()->GetFormatTable()->IsNumberFormat(aStr, nFormat, fValue);
            rStrColl.emplace_back(
                    aStr, fValue, bIsValue ? ScTypedStrData::Value : ScTypedStrData::Standard);
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

bool ScValidationData::IsEqualToTokenArray( ScRefCellValue& rCell, const ScAddress& rPos, const ScTokenArray& rTokArr ) const
{
    // create a condition entry that tests on equality and set the passed token array
    ScConditionEntry aCondEntry( ScConditionMode::Equal, &rTokArr, nullptr, GetDocument(), rPos );
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

    std::unique_ptr< ScTokenArray > pTokArr( CreateFlatCopiedTokenArray( 0 ) );

    // *** try if formula is a string list ***

    svl::SharedStringPool& rSPool = GetDocument()->GetSharedStringPool();
    sal_uInt32 nFormat = lclGetCellFormat( *GetDocument(), rPos );
    ScStringTokenIterator aIt( *pTokArr );
    for (rtl_uString* pString = aIt.First(); pString && aIt.Ok(); pString = aIt.Next())
    {
        /*  Do not break the loop, if a valid string has been found.
            This is to find invalid tokens following in the formula. */
        if( !bIsValid )
        {
            // create a formula containing a single string or number
            ScTokenArray aCondTokArr;
            double fValue;
            OUString aStr(pString);
            if (GetDocument()->GetFormatTable()->IsNumberFormat(aStr, nFormat, fValue))
                aCondTokArr.AddDouble( fValue );
            else
                aCondTokArr.AddString(rSPool.intern(aStr));

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
        bIsValid = GetSelectionFromFormula(nullptr, rCell, rPos, *pTokArr, nMatch);
        bIsValid = bIsValid && nMatch >= 0;
    }

    return bIsValid;
}

ScValidationDataList::ScValidationDataList(const ScValidationDataList& rList)
{
    //  for Ref-Undo - real copy with new tokens!

    for (const auto& rxItem : rList)
    {
        InsertNew( std::unique_ptr<ScValidationData>(rxItem->Clone()) );
    }

    //TODO: faster insert for sorted entries from rList ???
}

ScValidationDataList::ScValidationDataList(ScDocument* pNewDoc,
                                            const ScValidationDataList& rList)
{
    //  for new documents - real copy with new tokens!

    for (const auto& rxItem : rList)
    {
        InsertNew( std::unique_ptr<ScValidationData>(rxItem->Clone(pNewDoc)) );
    }

    //TODO: faster insert for sorted entries from rList ???
}

ScValidationData* ScValidationDataList::GetData( sal_uInt32 nKey )
{
    //TODO: binary search

    for( iterator it = begin(); it != end(); ++it )
        if( (*it)->GetKey() == nKey )
            return it->get();

    OSL_FAIL("ScValidationDataList: Entry not found");
    return nullptr;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: validat.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:35:08 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>

#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif

#include <svtools/sbx.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <rtl/math.hxx>
#include <math.h>
#include <memory>

#include "validat.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "rangenam.hxx"
#include "dbcolect.hxx"

//------------------------------------------------------------------------

SV_IMPL_OP_PTRARR_SORT( ScValidationEntries_Impl, ScValidationDataPtr );

//------------------------------------------------------------------------

//
//  Eintrag fuer Gueltigkeit (es gibt nur eine Bedingung)
//

ScValidationData::ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                            const String& rExpr1, const String& rExpr2,
                            ScDocument* pDocument, const ScAddress& rPos,
                            BOOL bCompileEnglish, BOOL bCompileXML ) :
    ScConditionEntry( eOper, rExpr1, rExpr2, pDocument, rPos, bCompileEnglish, bCompileXML ),
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

ScValidationData::ScValidationData( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                    ScDocument* pDocument ) :
    ScConditionEntry( rStream, rHdr, pDocument ),
    mnListType( ValidListType::UNSORTED )   // not imported/exported
{
    //  im Datei-Header sind getrennte Eintraege fuer ScConditionEntry und ScValidationData

    rHdr.StartEntry();

    //  1) Key
    //  2) eDataMode
    //  3) bShowInput
    //  4) aInputTitle
    //  5) aInputMessage
    //  6) bShowError
    //  7) aErrorTitle
    //  8) aErrorMessage
    //  9) eErrorStyle

    USHORT nDummy;
    rStream >> nKey;
    rStream >> nDummy;
    eDataMode = (ScValidationMode) nDummy;
    rStream >> bShowInput;
    rStream.ReadByteString( aInputTitle, rStream.GetStreamCharSet() );
    rStream.ReadByteString( aInputMessage, rStream.GetStreamCharSet() );
    rStream >> bShowError;
    rStream.ReadByteString( aErrorTitle, rStream.GetStreamCharSet() );
    rStream.ReadByteString( aErrorMessage, rStream.GetStreamCharSet() );
    rStream >> nDummy;
    eErrorStyle = (ScValidErrorStyle) nDummy;

    rHdr.EndEntry();
}

ScValidationData::~ScValidationData()
{
}

void ScValidationData::Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const
{
    //  im Datei-Header sind getrennte Eintraege fuer ScConditionEntry und ScValidationData

    StoreCondition( rStream, rHdr );

    rHdr.StartEntry();

    //  1) Key
    //  2) eDataMode
    //  3) bShowInput
    //  4) aInputTitle
    //  5) aInputMessage
    //  6) bShowError
    //  7) aErrorTitle
    //  8) aErrorMessage
    //  9) eErrorStyle

    rStream << nKey;
    rStream << (USHORT) eDataMode;
    rStream << bShowInput;
    rStream.WriteByteString( aInputTitle, rStream.GetStreamCharSet() );
    rStream.WriteByteString( aInputMessage, rStream.GetStreamCharSet() );
    rStream << bShowError;
    rStream.WriteByteString( aErrorTitle, rStream.GetStreamCharSet() );
    rStream.WriteByteString( aErrorMessage, rStream.GetStreamCharSet() );
    rStream << (USHORT) eErrorStyle;

    rHdr.EndEntry();
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

    // TRUE -> Abbruch

BOOL ScValidationData::DoMacro( const ScAddress& rPos, const String& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const
{
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

#if 0
    // Makro-Name liegt in folgender Form vor:
    // "Macroname.Modulname.Libname.Dokumentname" oder
    // "Macroname.Modulname.Libname.Applikationsname"
    String aMacroName = aErrorTitle.GetToken(0, '.');
    String aModulName = aErrorTitle.GetToken(1, '.');
    String aLibName   = aErrorTitle.GetToken(2, '.');
    String aDocName   = aErrorTitle.GetToken(3, '.');
#endif

    //  Funktion ueber den einfachen Namen suchen,
    //  dann aBasicStr, aMacroStr fuer SfxObjectShell::CallBasic zusammenbauen

    StarBASIC* pRoot = pDocSh->GetBasic();
    SbxVariable* pVar = pRoot->Find( aErrorTitle, SbxCLASS_METHOD );
    if ( pVar && pVar->ISA(SbMethod) )
    {
        SbMethod* pMethod = (SbMethod*)pVar;
        SbModule* pModule = pMethod->GetModule();
        SbxObject* pObject = pModule->GetParent();
        StarBASIC* pBasic = PTR_CAST(StarBASIC,pObject);
        DBG_ASSERT(pBasic, "Kein Basic gefunden!");
        String aMacroStr = pObject->GetName();
        aMacroStr += '.';
        aMacroStr += pModule->GetName();
        aMacroStr += '.';
        aMacroStr += pMethod->GetName();
        String aBasicStr;

        //  #95867# the distinction between document- and app-basic has to be done
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
            refPar->Get(1)->PutDouble( nValue );
        else
            refPar->Get(1)->PutString( aValStr );

        //  2) Position der Zelle
        String aPosStr;
        rPos.Format( aPosStr, SCA_VALID | SCA_TAB_3D, pDocument );
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

    ULONG nFormat = rPattern.GetNumberFormat( pFormatter );

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
            DBG_ERROR("hammanochnich");
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
    const ScToken* pToken = mrTokArr.NextNoSpaces();
    while( pToken && (pToken->GetOpCode() == ocSep) )
        pToken = mrTokArr.NextNoSpaces();

    mbOk = !pToken || (pToken->GetType() == svString);
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
void lclInsertStringToCollection( TypedStrCollection& rStrColl, TypedStrData* pData, bool bSorted )
{
    if( !(bSorted ? rStrColl.Insert( pData ) : rStrColl.AtInsert( rStrColl.GetCount(), pData )) )
        delete pData;
}

} // namespace

// ----------------------------------------------------------------------------

bool ScValidationData::GetRangeFromFormula( ScRange& rRange, const ScAddress& rBaseAddr, ScTokenArray& rTokArr, int nRecCount ) const
{
    if( nRecCount >= 42 )
        return false;

    bool bIsRange = false;          // range found from recursive call
    bool bRangeFound = false;       // range found in current run

    const ScToken* pToken = rTokArr.FirstNoSpaces();
    /*  The test !rTokArr.NextNoSpaces() would result in returning an error, if any
        other token follows. But specification says, just ignore the rest of the formula. */
    if( pToken /*&& !rTokArr.NextNoSpaces()*/ )
    {
        switch( pToken->GetType() )
        {
            case svSingleRef:
            case svDoubleRef:
            {
                // Single or double reference. Convert to absolute cell range in the document.
                ComplRefData aRef;
                if( pToken->GetType() == svSingleRef )
                    aRef.Ref1 = aRef.Ref2 = pToken->GetSingleRef();
                else
                    aRef = pToken->GetDoubleRef();
                aRef.CalcAbsIfRel( rBaseAddr );

                if( aRef.Valid() && !aRef.IsDeleted() )
                {
                    rRange.aStart.Set( aRef.Ref1.nCol, aRef.Ref1.nRow, aRef.Ref1.nTab );
                    rRange.aEnd.Set( aRef.Ref2.nCol, aRef.Ref2.nRow, aRef.Ref2.nTab );
                    bRangeFound = true;
                }
            }
            break;

            case svIndex:
            {
                switch( pToken->GetOpCode() )
                {
                    case ocName:
                    {
                        // Defined name. Calls this function recursively with name definition.
                        if( ScRangeData* pRangeData = GetDocument()->GetRangeName()->FindIndex( pToken->GetIndex() ) )
                            if( ScTokenArray* pNameTokArr = pRangeData->GetCode() )
                                bIsRange = GetRangeFromFormula( rRange, rBaseAddr, *pNameTokArr, nRecCount + 1 );
                    }
                    break;

                    case ocDBArea:
                    {
                        // Database range. Gets cell range and evaluates the cells.
                        if( ScDBCollection* pDBColl = GetDocument()->GetDBCollection() )
                        {
                            if( const ScDBData* pDBData = pDBColl->FindIndex( pToken->GetIndex() ) )
                            {
                                pDBData->GetArea( rRange );
                                bRangeFound = true;
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    if( bRangeFound )
    {
        rRange.Justify();
        // range is restricted to one table
        rRange.aEnd.SetTab( rRange.aStart.Tab() );
        // if the range has multiple rows, it is restricted to one column
        if( rRange.aStart.Row() < rRange.aEnd.Row() )
            rRange.aEnd.SetCol( rRange.aStart.Col() );
        bIsRange = true;
    }

    return bIsRange;
}

// ----------------------------------------------------------------------------

bool ScValidationData::HasSelectionList() const
{
    return (eDataMode == SC_VALID_LIST) && (mnListType != ValidListType::INVISIBLE);
}

bool ScValidationData::FillSelectionList( TypedStrCollection& rStrColl, const ScAddress rPos ) const
{
    bool bOk = false;

    if( HasSelectionList() )
    {
        ::std::auto_ptr< ScTokenArray > pTokArr( CreateTokenArry( 0 ) );
        bool bSortList = (mnListType == ValidListType::SORTEDASCENDING);

        ScRange aSource;
        if( GetRangeFromFormula( aSource, rPos, *pTokArr ) )
        {
            // *** formula results in a cell range ***

            USHORT nTab = aSource.aStart.Tab();
            for( USHORT nCol = aSource.aStart.Col(), nEndCol = aSource.aEnd.Col(); nCol <= nEndCol; ++nCol )
            {
                for( USHORT nRow = aSource.aStart.Row(), nEndRow = aSource.aEnd.Row(); nRow <= nEndRow; ++nRow )
                {
                    TypedStrData* pData = new TypedStrData( GetDocument(), nCol, nRow, nTab, TRUE );
                    lclInsertStringToCollection( rStrColl, pData, bSortList );
                }
            }

            bOk = true;
        }
        else
        {
            // *** try if formula is a string list ***

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
        3)  A single defined name (must contain a cell/range reference, another name, or DB range).
        4)  A single database range.
        If the range consists of one row or one column, all cells of a cell range are used.
        Otherwise only the first column of a range will be evaluated. */

    ::std::auto_ptr< ScTokenArray > pTokArr( CreateTokenArry( 0 ) );

    ScRange aSource;
    if( GetRangeFromFormula( aSource, rPos, *pTokArr ) )
    {
        // *** formula results in a cell range ***

        SingleRefData aStartRef, aEndRef;
        aStartRef.InitAddress( aSource.aStart );
        aEndRef.InitAddress( aSource.aEnd );

        for( SingleRefData aRef = aStartRef; !bIsValid && (aRef.nCol <= aEndRef.nCol); ++aRef.nCol )
        {
            for( aRef.nRow = aStartRef.nRow; !bIsValid && (aRef.nRow <= aEndRef.nRow); ++aRef.nRow )
            {
                // create a formula containing a single reference to the current cell
                ScTokenArray aCondTokArr;
                aCondTokArr.AddSingleReference( aRef );

                bIsValid = IsEqualToTokenArray( pCell, rPos, aCondTokArr );
            }
        }
    }
    else
    {
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
    }

    return bIsValid;
}

// ============================================================================
// ============================================================================

ScValidationDataList::ScValidationDataList(const ScValidationDataList& rList)
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

ScValidationData* ScValidationDataList::GetData( ULONG nKey )
{
    //! binaer suchen

    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
        if ((*this)[i]->GetKey() == nKey)
            return (*this)[i];

    DBG_ERROR("ScValidationDataList: Eintrag nicht gefunden");
    return NULL;
}

void ScValidationDataList::Load( SvStream& rStream, ScDocument* pDocument )
{
    ScMultipleReadHeader aHdr( rStream );

    USHORT nNewCount;
    rStream >> nNewCount;

    for (USHORT i=0; i<nNewCount; i++)
    {
        ScValidationData* pNew = new ScValidationData( rStream, aHdr, pDocument );
        InsertNew( pNew );
    }
}

void ScValidationDataList::Store( SvStream& rStream ) const
{
    USHORT i;
    ScMultipleWriteHeader aHdr( rStream );

    USHORT nCount = Count();
    USHORT nUsed = 0;
    for (i=0; i<nCount; i++)
        if ((*this)[i]->IsUsed())
            ++nUsed;

    rStream << nUsed;       // Anzahl der gespeicherten

    for (i=0; i<nCount; i++)
    {
        const ScValidationData* pForm = (*this)[i];
        if (pForm->IsUsed())
            pForm->Store( rStream, aHdr );
    }
}

void ScValidationDataList::ResetUsed()
{
    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
        (*this)[i]->SetUsed(FALSE);
}

void ScValidationDataList::CompileXML()
{
    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
        (*this)[i]->CompileXML();
}

void ScValidationDataList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz )
{
    DBG_ERROR("ScValidationDataList::UpdateReference");
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


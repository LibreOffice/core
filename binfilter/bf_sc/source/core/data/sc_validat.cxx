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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------



#include <math.h>


#include "validat.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
namespace binfilter {

//------------------------------------------------------------------------

/*N*/ SV_IMPL_OP_PTRARR_SORT( ScValidationEntries_Impl, ScValidationDataPtr );

//------------------------------------------------------------------------

//
//	Eintrag fuer Gueltigkeit (es gibt nur eine Bedingung)
//

/*N*/ ScValidationData::ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
/*N*/ 							const String& rExpr1, const String& rExpr2,
/*N*/ 							ScDocument* pDocument, const ScAddress& rPos,
/*N*/ 							BOOL bCompileEnglish, BOOL bCompileXML ) :
/*N*/ 	ScConditionEntry( eOper, rExpr1, rExpr2, pDocument, rPos, bCompileEnglish, bCompileXML ),
/*N*/ 	nKey( 0 ),
/*N*/ 	eDataMode( eMode )
/*N*/ {
/*N*/ 	bShowInput = bShowError = FALSE;
/*N*/ 	eErrorStyle = SC_VALERR_STOP;
/*N*/ }

/*N*/ ScValidationData::ScValidationData( const ScValidationData& r ) :
/*N*/ 	ScConditionEntry( r ),
/*N*/ 	nKey( r.nKey ),
/*N*/ 	eDataMode( r.eDataMode ),
/*N*/ 	bShowInput( r.bShowInput ),
/*N*/ 	bShowError( r.bShowError ),
/*N*/ 	eErrorStyle( r.eErrorStyle ),
/*N*/ 	aInputTitle( r.aInputTitle ),
/*N*/ 	aInputMessage( r.aInputMessage ),
/*N*/ 	aErrorTitle( r.aErrorTitle ),
/*N*/ 	aErrorMessage( r.aErrorMessage )
/*N*/ {
/*N*/ 	//	Formeln per RefCount kopiert
/*N*/ }

/*N*/ ScValidationData::ScValidationData( ScDocument* pDocument, const ScValidationData& r ) :
/*N*/ 	ScConditionEntry( pDocument, r ),
/*N*/ 	nKey( r.nKey ),
/*N*/ 	eDataMode( r.eDataMode ),
/*N*/ 	bShowInput( r.bShowInput ),
/*N*/ 	bShowError( r.bShowError ),
/*N*/ 	eErrorStyle( r.eErrorStyle ),
/*N*/ 	aInputTitle( r.aInputTitle ),
/*N*/ 	aInputMessage( r.aInputMessage ),
/*N*/ 	aErrorTitle( r.aErrorTitle ),
/*N*/ 	aErrorMessage( r.aErrorMessage )
/*N*/ {
/*N*/ 	//	Formeln wirklich kopiert
/*N*/ }

/*N*/ ScValidationData::ScValidationData( SvStream& rStream, ScMultipleReadHeader& rHdr,
/*N*/ 									ScDocument* pDocument ) :
/*N*/ 	ScConditionEntry( rStream, rHdr, pDocument )
/*N*/ {
/*N*/ 	//	im Datei-Header sind getrennte Eintraege fuer ScConditionEntry und ScValidationData
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	//	1) Key
/*N*/ 	//	2) eDataMode
/*N*/ 	//	3) bShowInput
/*N*/ 	//	4) aInputTitle
/*N*/ 	//	5) aInputMessage
/*N*/ 	//	6) bShowError
/*N*/ 	//	7) aErrorTitle
/*N*/ 	//	8) aErrorMessage
/*N*/ 	//	9) eErrorStyle
/*N*/ 
/*N*/ 	USHORT nDummy;
/*N*/ 	rStream >> nKey;
/*N*/ 	rStream >> nDummy;
/*N*/ 	eDataMode = (ScValidationMode) nDummy;
/*N*/ 	rStream >> bShowInput;
/*N*/ 	rStream.ReadByteString( aInputTitle, rStream.GetStreamCharSet() );
/*N*/ 	rStream.ReadByteString( aInputMessage, rStream.GetStreamCharSet() );
/*N*/ 	rStream >> bShowError;
/*N*/ 	rStream.ReadByteString( aErrorTitle, rStream.GetStreamCharSet() );
/*N*/ 	rStream.ReadByteString( aErrorMessage, rStream.GetStreamCharSet() );
/*N*/ 	rStream >> nDummy;
/*N*/ 	eErrorStyle = (ScValidErrorStyle) nDummy;
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ }

/*N*/ ScValidationData::~ScValidationData()
/*N*/ {
/*N*/ }

/*N*/ void ScValidationData::Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const
/*N*/ {
/*N*/ 	//	im Datei-Header sind getrennte Eintraege fuer ScConditionEntry und ScValidationData
/*N*/ 
/*N*/ 	StoreCondition( rStream, rHdr );
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	//	1) Key
/*N*/ 	//	2) eDataMode
/*N*/ 	//	3) bShowInput
/*N*/ 	//	4) aInputTitle
/*N*/ 	//	5) aInputMessage
/*N*/ 	//	6) bShowError
/*N*/ 	//	7) aErrorTitle
/*N*/ 	//	8) aErrorMessage
/*N*/ 	//	9) eErrorStyle
/*N*/ 
/*N*/ 	rStream << nKey;
/*N*/ 	rStream << (USHORT) eDataMode;
/*N*/ 	rStream << bShowInput;
/*N*/ 	rStream.WriteByteString( aInputTitle, rStream.GetStreamCharSet() );
/*N*/ 	rStream.WriteByteString( aInputMessage, rStream.GetStreamCharSet() );
/*N*/ 	rStream << bShowError;
/*N*/ 	rStream.WriteByteString( aErrorTitle, rStream.GetStreamCharSet() );
/*N*/ 	rStream.WriteByteString( aErrorMessage, rStream.GetStreamCharSet() );
/*N*/ 	rStream << (USHORT) eErrorStyle;
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ }

/*N*/ BOOL ScValidationData::IsEmpty() const
/*N*/ {
/*N*/ 	String aEmpty;
/*N*/ 	ScValidationData aDefault( SC_VALID_ANY, SC_COND_EQUAL, aEmpty, aEmpty, GetDocument(), ScAddress() );
/*N*/ 	return EqualEntries( aDefault );
/*N*/ }

/*N*/ BOOL ScValidationData::EqualEntries( const ScValidationData& r ) const
/*N*/ {
/*N*/ 		//	gleiche Parameter eingestellt (ohne Key)
/*N*/ 
/*N*/ 	return ScConditionEntry::operator==(r) &&
/*N*/ 			eDataMode		== r.eDataMode &&
/*N*/ 			bShowInput		== r.bShowInput &&
/*N*/ 			bShowError		== r.bShowError &&
/*N*/ 			eErrorStyle		== r.eErrorStyle &&
/*N*/ 			aInputTitle		== r.aInputTitle &&
/*N*/ 			aInputMessage	== r.aInputMessage &&
/*N*/ 			aErrorTitle		== r.aErrorTitle &&
/*N*/ 			aErrorMessage	== r.aErrorMessage;
/*N*/ }

/*N*/ void ScValidationData::ResetInput()
/*N*/ {
/*N*/ 	bShowInput = FALSE;
/*N*/ }

/*N*/ void ScValidationData::ResetError()
/*N*/ {
/*N*/ 	bShowError = FALSE;
/*N*/ }

/*N*/ void ScValidationData::SetInput( const String& rTitle, const String& rMsg )
/*N*/ {
/*N*/ 	bShowInput = TRUE;
/*N*/ 	aInputTitle = rTitle;
/*N*/ 	aInputMessage = rMsg;
/*N*/ }

/*N*/ void ScValidationData::SetError( const String& rTitle, const String& rMsg,
/*N*/ 									ScValidErrorStyle eStyle )
/*N*/ {
/*N*/ 	bShowError = TRUE;
/*N*/ 	eErrorStyle = eStyle;
/*N*/ 	aErrorTitle = rTitle;
/*N*/ 	aErrorMessage = rMsg;
/*N*/ }

/*N*/ BOOL ScValidationData::GetErrMsg( String& rTitle, String& rMsg,
/*N*/ 									ScValidErrorStyle& rStyle ) const
/*N*/ {
/*N*/ 	rTitle = aErrorTitle;
/*N*/ 	rMsg   = aErrorMessage;
/*N*/ 	rStyle = eErrorStyle;
/*N*/ 	return bShowError;
/*N*/ }

/*N*/ BOOL ScValidationData::IsDataValid( ScBaseCell* pCell, const ScAddress& rPos ) const
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 double nVal = 0.0;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScValidationDataList::ScValidationDataList(const ScValidationDataList& rList)
/*N*/ {
/*N*/ 	//	fuer Ref-Undo - echte Kopie mit neuen Tokens!
/*N*/ 
/*N*/ 	USHORT nCount = rList.Count();
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		InsertNew( rList[i]->Clone() );
/*N*/ 
/*N*/ 	//!		sortierte Eintraege aus rList schneller einfuegen ???
/*N*/ }

/*N*/ ScValidationData* ScValidationDataList::GetData( sal_uInt32 nKey )
/*N*/ {
/*N*/ 	//!	binaer suchen
/*N*/ 
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		if ((*this)[i]->GetKey() == nKey)
/*N*/ 			return (*this)[i];
/*N*/ 
/*N*/ 	DBG_ERROR("ScValidationDataList: Eintrag nicht gefunden");
/*N*/ 	return NULL;
/*N*/ }

/*N*/ void ScValidationDataList::Load( SvStream& rStream, ScDocument* pDocument )
/*N*/ {
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nNewCount;
/*N*/ 	rStream >> nNewCount;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nNewCount; i++)
/*N*/ 	{
/*N*/ 		ScValidationData* pNew = new ScValidationData( rStream, aHdr, pDocument );
/*N*/ 		InsertNew( pNew );
/*N*/ 	}
/*N*/ }

/*N*/ void ScValidationDataList::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nCount = Count();
/*N*/ 	USHORT nUsed = 0;
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 		if ((*this)[i]->IsUsed())
/*N*/ 			++nUsed;
/*N*/ 
/*N*/ 	rStream << nUsed;		// Anzahl der gespeicherten
/*N*/ 
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		const ScValidationData* pForm = (*this)[i];
/*N*/ 		if (pForm->IsUsed())
/*N*/ 			pForm->Store( rStream, aHdr );
/*N*/ 	}
/*N*/ }

/*N*/ void ScValidationDataList::ResetUsed()
/*N*/ {
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		(*this)[i]->SetUsed(FALSE);
/*N*/ }

/*N*/ void ScValidationDataList::CompileXML()
/*N*/ {
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		(*this)[i]->CompileXML();
/*N*/ }






}

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


#ifdef _MSC_VER
#pragma hdrstop
#endif


#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif
#ifndef _EEITEMID_HXX
#include "eeitemid.hxx"
#endif


#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif



#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#include <impedit.hxx>


#include "kernitem.hxx"

namespace binfilter {

/*N*/ DBG_NAME( EE_ParaPortion )

/*N*/ SV_IMPL_VARARR( CharPosArray, sal_Int32 );

/*

BOOL EditStyleSheet::HasStyleAsAnyParent( SfxStyleSheet& rStyle )
{
    if ( GetParent() == rStyle.GetName() )
        return TRUE;

    if ( GetParent().Len() && ( GetParent() != GetName() ) )
    {
        EditStyleSheet* pS = (EditStyleSheet*)GetPool().Find( GetParent(), rStyle.GetFamily() );
        if ( pS )
            return pS->HasStyleAsAnyParent( rStyle );
    }
    return FALSE;
}

*/

// -------------------------------------------------------------------------
// class TextPortionList
// -------------------------------------------------------------------------
/*N*/ TextPortionList::TextPortionList()
/*N*/ {
/*N*/ }

/*N*/ TextPortionList::~TextPortionList()
/*N*/ {
/*N*/ 	Reset();
/*N*/ }

/*N*/ void TextPortionList::Reset()
/*N*/ {
/*N*/ 	for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
/*N*/ 		delete GetObject( nPortion );
/*N*/ 	Remove( 0, Count() );
/*N*/ }

/*N*/ void TextPortionList::DeleteFromPortion( USHORT nDelFrom )
/*N*/ {
/*N*/ 	DBG_ASSERT( ( nDelFrom < Count() ) || ( (nDelFrom == 0) && (Count() == 0) ), "DeleteFromPortion: Out of range" );
/*N*/ 	for ( USHORT nP = nDelFrom; nP < Count(); nP++ )
/*N*/ 		delete GetObject( nP );
/*N*/ 	Remove( nDelFrom, Count()-nDelFrom );
/*N*/ }

/*N*/ USHORT TextPortionList::FindPortion( USHORT nCharPos, USHORT& nPortionStart, BOOL bPreferStartingPortion )
/*N*/ {
/*N*/ 	// Bei nCharPos an Portion-Grenze wird die linke Portion gefunden
/*N*/ 	USHORT nTmpPos = 0;
/*N*/ 	for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
/*N*/ 	{
/*N*/ 		TextPortion* pPortion = GetObject( nPortion );
/*N*/ 		nTmpPos += pPortion->GetLen();
/*N*/ 		if ( nTmpPos >= nCharPos )
/*N*/ 		{
/*N*/             // take this one if we don't prefer the starting portion, or if it's the last one
/*N*/             if ( ( nTmpPos != nCharPos ) || !bPreferStartingPortion || ( nPortion == Count() - 1 ) )
/*N*/             {
/*N*/ 			    nPortionStart = nTmpPos - pPortion->GetLen();
/*N*/ 			    return nPortion;
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*?*/ 	DBG_ERROR( "FindPortion: Nicht gefunden!" );
/*?*/ 	return ( Count() - 1 );
/*N*/ }

/*N*/ ExtraPortionInfo::~ExtraPortionInfo()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// -------------------------------------------------------------------------
// class ParaPortion
// -------------------------------------------------------------------------
/*N*/ ParaPortion::ParaPortion( ContentNode* pN )
/*N*/ {
/*N*/ 	DBG_CTOR( EE_ParaPortion, 0 );
/*N*/ 
/*N*/ 	pNode 				= pN;
/*N*/ 	bInvalid 			= TRUE;
/*N*/ 	bVisible 			= TRUE;
/*N*/ 	bSimple 			= FALSE;
/*N*/ 	bForceRepaint 		= FALSE;
/*N*/ 	nInvalidPosStart	= 0;
/*N*/ 	nInvalidDiff 		= 0;
/*N*/ 	nHeight 			= 0;
/*N*/ 	nFirstLineOffset 	= 0;
/*N*/ 	nBulletX			= 0;
/*N*/ }

/*N*/ ParaPortion::~ParaPortion()
/*N*/ {
/*N*/ 	DBG_DTOR( EE_ParaPortion, 0 );
/*N*/ }

/*N*/ void ParaPortion::MarkInvalid( USHORT nStart, short nDiff )
/*N*/ {
/*N*/ 	if ( bInvalid == FALSE )
/*N*/ 	{
/*N*/ //		nInvalidPosEnd = nStart;	// ??? => CreateLines
/*N*/ 		nInvalidPosStart = ( nDiff >= 0 ) ? nStart : ( nStart + nDiff );
/*N*/ 		nInvalidDiff = nDiff;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Einfaches hintereinander tippen
/*N*/ 		if ( ( nDiff > 0 ) && ( nInvalidDiff > 0 ) &&
/*N*/ 			 ( ( nInvalidPosStart+nInvalidDiff ) == nStart ) )
/*N*/ 		{
/*N*/ 			nInvalidDiff += nDiff;
/*N*/ 		}
/*N*/ 		// Einfaches hintereinander loeschen
/*N*/ 		else if ( ( nDiff < 0 ) && ( nInvalidDiff < 0 ) && ( nInvalidPosStart == nStart ) )
/*N*/ 		{
/*?*/ 			nInvalidPosStart += nDiff;
/*?*/ 			nInvalidDiff += nDiff;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ //			nInvalidPosEnd = pNode->Len();
/*N*/ 			DBG_ASSERT( ( nDiff >= 0 ) || ( (nStart+nDiff) >= 0 ), "MarkInvalid: Diff out of Range" );
/*N*/ 			nInvalidPosStart = Min( nInvalidPosStart, (USHORT) ( nDiff < 0 ? nStart+nDiff : nDiff ) );
/*N*/ 			nInvalidDiff = 0;
/*N*/ 			bSimple = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	bInvalid = TRUE;
/*N*/ 	aScriptInfos.Remove( 0, aScriptInfos.Count() );
/*N*/ 	aWritingDirectionInfos.Remove( 0, aWritingDirectionInfos.Count() );
/*N*/ //	aExtraCharInfos.Remove( 0, aExtraCharInfos.Count() );
/*N*/ }

/*N*/ void ParaPortion::MarkSelectionInvalid( USHORT nStart, USHORT nEnd )
/*N*/ {
/*N*/ 	if ( bInvalid == FALSE )
/*N*/ 	{
/*N*/ 		nInvalidPosStart = nStart;
/*N*/ //		nInvalidPosEnd = nEnd;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nInvalidPosStart = Min( nInvalidPosStart, nStart );
/*N*/ //		nInvalidPosEnd = pNode->Len();
/*N*/ 	}
/*N*/ 	nInvalidDiff = 0;
/*N*/ 	bInvalid = TRUE;
/*N*/ 	bSimple = FALSE;
/*N*/ 	aScriptInfos.Remove( 0, aScriptInfos.Count() );
/*N*/ 	aWritingDirectionInfos.Remove( 0, aWritingDirectionInfos.Count() );
/*N*/ //	aExtraCharInfos.Remove( 0, aExtraCharInfos.Count() );
/*N*/ }

/*N*/ USHORT ParaPortion::GetLineNumber( USHORT nIndex )
/*N*/ {
/*N*/ 	DBG_ASSERTWARNING( aLineList.Count(), "Leere ParaPortion in GetLine!" );
/*N*/ 	DBG_ASSERT( bVisible, "Wozu GetLine() bei einem unsichtbaren Absatz?" );
/*N*/ 
/*N*/ 	for ( USHORT nLine = 0; nLine < aLineList.Count(); nLine++ )
/*N*/ 	{
/*N*/ 		if ( aLineList[nLine]->IsIn( nIndex ) )
/*N*/ 			return nLine;
/*N*/ 	}
/*N*/ 
/*?*/ 	// Dann sollte es am Ende der letzten Zeile sein!
/*?*/ 	DBG_ASSERT( nIndex == aLineList[ aLineList.Count() - 1 ]->GetEnd(), "Index voll daneben!" );
/*?*/ 	return (aLineList.Count()-1);
/*N*/ }


/*N*/ void ParaPortion::CorrectValuesBehindLastFormattedLine( USHORT nLastFormattedLine )
/*N*/ {
/*N*/ 	USHORT nLines = aLineList.Count();
/*N*/ 	DBG_ASSERT( nLines, "CorrectPortionNumbersFromLine: Leere Portion?" );
/*N*/ 	if ( nLastFormattedLine < ( nLines - 1 ) )
/*N*/ 	{
/*?*/ 		const EditLine* pLastFormatted = aLineList[ nLastFormattedLine ];
/*?*/ 		const EditLine* pUnformatted = aLineList[ nLastFormattedLine+1 ];
/*?*/ 		short nPortionDiff = pUnformatted->GetStartPortion() - pLastFormatted->GetEndPortion();
/*?*/ 		short nTextDiff = pUnformatted->GetStart() - pLastFormatted->GetEnd();
/*?*/ 		nTextDiff++;	// LastFormatted->GetEnd() war incl. => 1 zuviel abgezogen!
/*?*/ 
/*?*/ 		// Die erste unformatierte muss genau eine Portion hinter der letzten der
/*?*/ 		// formatierten beginnen:
/*?*/ 		// Wenn in der geaenderten Zeile eine Portion gesplittet wurde,
/*?*/ 		// kann nLastEnd > nNextStart sein!
/*?*/ 		short nPDiff = -( nPortionDiff-1 );
/*?*/ 		short nTDiff = -( nTextDiff-1 );
/*?*/ 		if ( nPDiff || nTDiff )
/*?*/ 		{
/*?*/ 			for ( USHORT nL = nLastFormattedLine+1; nL < nLines; nL++ )
/*?*/ 			{
/*?*/ 				EditLine* pLine = aLineList[ nL ];
/*?*/ 
/*?*/ 				pLine->GetStartPortion() += nPDiff;
/*?*/ 				pLine->GetEndPortion() += nPDiff;
/*?*/ 
/*?*/ 				pLine->GetStart() += nTDiff;
/*?*/ 				pLine->GetEnd() += nTDiff;
/*?*/ 
/*?*/ 				pLine->SetValid();
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	DBG_ASSERT( aLineList[ aLineList.Count()-1 ]->GetEnd() == pNode->Len(), "CorrectLines: Ende stimmt nicht!" );
/*N*/ }

// -------------------------------------------------------------------------
// class ParaPortionList
// -------------------------------------------------------------------------
/*N*/ ParaPortionList::ParaPortionList()
/*N*/ {
/*N*/ }

/*N*/ ParaPortionList::~ParaPortionList()
/*N*/ {
/*N*/ 	Reset();
/*N*/ }

/*N*/ void ParaPortionList::Reset()
/*N*/ {
/*N*/ 	for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
/*N*/ 		delete GetObject( nPortion );
/*N*/ 	Remove( 0, Count() );
/*N*/ }

/*N*/ long ParaPortionList::GetYOffset( ParaPortion* pPPortion )
/*N*/ {
/*N*/ 	long nHeight = 0;
/*N*/ 	for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
/*N*/ 	{
/*N*/ 		ParaPortion* pTmpPortion = GetObject(nPortion);
/*N*/ 		if ( pTmpPortion == pPPortion )
/*N*/ 			return nHeight;
/*?*/ 		nHeight += pTmpPortion->GetHeight();
/*?*/ 	}
/*?*/ 	DBG_ERROR( "GetYOffset: Portion nicht gefunden" );
/*?*/ 	return nHeight;
/*N*/ }

/*N*/ USHORT ParaPortionList::FindParagraph( long nYOffset )
/*N*/ {
/*N*/ 	long nY = 0;
/*N*/ 	for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
/*N*/ 	{
/*N*/ 		nY += GetObject(nPortion)->GetHeight(); // sollte auch bei !bVisible richtig sein!
/*N*/ 		if ( nY > nYOffset )
/*N*/ 			return nPortion;
/*N*/ 	}
/*N*/ 	return 0xFFFF;	// solte mal ueber EE_PARA_NOT_FOUND erreicht werden!
/*N*/ }





/*N*/ void ConvertItem( SfxPoolItem& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit )
/*N*/ {
/*N*/ 	DBG_ASSERT( eSourceUnit != eDestUnit, "ConvertItem - Why?!" );
/*N*/ 
/*N*/ 	switch ( rPoolItem.Which() )
/*N*/ 	{
/*N*/ 		case EE_PARA_LRSPACE:
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_PARA_ULSPACE:
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_PARA_SBL:
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_PARA_TABS:
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTHEIGHT:
/*N*/ 		case EE_CHAR_FONTHEIGHT_CJK:
/*N*/ 		case EE_CHAR_FONTHEIGHT_CTL:
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ }


}

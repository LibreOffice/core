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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <bf_svx/twolinesitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <bf_svx/charrotateitem.hxx>
#endif

#ifdef BIDI
#endif

#ifndef _CHARFMT_HXX	// SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _TXTINET_HXX	// SwTxtINetFmt
#include <txtinet.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _PORMULTI_HXX
#include <pormulti.hxx> 	// SwMultiPortion
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx>		// SwTxtFormatter
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _PORFLD_HXX
#include <porfld.hxx>		// SwFldPortion
#endif
namespace binfilter {

using namespace ::com::sun::star;
extern sal_Bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt );
extern BYTE WhichFont( xub_StrLen nIdx, const String* pTxt,
                       const SwScriptInfo* pSI );

/*-----------------10.10.00 15:23-------------------
 *  class SwMultiPortion
 *
 * A SwMultiPortion is not a simple portion,
 * it's a container, which contains almost a SwLineLayoutPortion.
 * This SwLineLayout could be followed by other textportions via pPortion
 * and by another SwLineLayout via pNext to realize a doubleline portion.
 * --------------------------------------------------*/



/*-----------------13.10.00 16:21-------------------
 * Summarize the internal lines to calculate the (external) size.
 * The internal line has to calculate first.
 * --------------------------------------------------*/



#ifdef BIDI
#endif

/*************************************************************************
 *              virtual SwMultiPortion::HandlePortion()
 *************************************************************************/


/*-----------------01.11.00 14:21-------------------
 * SwMultiPortion::ActualizeTabulator()
 * sets the tabulator-flag, if there's any tabulator-portion inside.
 * --------------------------------------------------*/


/*-----------------16.02.01 12:07-------------------
 * SwRotatedPortion::SwRotatedPortion(..)
 * --------------------------------------------------*/



/*-----------------01.11.00 14:22-------------------
 * SwDoubleLinePortion::SwDoubleLinePortion(..)
 * This constructor is for the continuation of a doubleline portion
 * in the next line.
 * It takes the same brackets and if the original has no content except
 * brackets, these will be deleted.
 * --------------------------------------------------*/


/*-----------------01.11.00 14:22-------------------
 * SwDoubleLinePortion::SwDoubleLinePortion(..)
 * This constructor uses the textattribut to get the right brackets.
 * The textattribut could be a 2-line-attribute or a character- or
 * internetstyle, which contains the 2-line-attribute.
 * --------------------------------------------------*/



/*-----------------25.10.00 09:51-------------------
 * SwMultiPortion::PaintBracket paints the wished bracket,
 * if the multiportion has surrounding brackets.
 * The X-position of the SwTxtPaintInfo will be modified:
 * the open bracket sets position behind itself,
 * the close bracket in front of itself.
 * --------------------------------------------------*/


/*-----------------25.10.00 16:26-------------------
 * SwDoubleLinePortion::SetBrackets creates the bracket-structur
 * and fills it, if not both characters are 0x00.
 * --------------------------------------------------*/


/*-----------------25.10.00 16:29-------------------
 * SwDoubleLinePortion::FormatBrackets
 * calculates the size of the brackets => pBracket,
 * reduces the nMaxWidth-parameter ( minus bracket-width )
 * and moves the rInf-x-position behind the opening bracket.
 * --------------------------------------------------*/


/*-----------------26.10.00 10:36-------------------
 * SwDoubleLinePortion::CalcBlanks
 * calculates the number of blanks in each line and
 * the difference of the width of the two lines.
 * These results are used from the text adjustment.
 * --------------------------------------------------*/



/*-----------------01.11.00 14:29-------------------
 * SwDoubleLinePortion::ChangeSpaceAdd(..)
 * merges the spaces for text adjustment from the inner and outer part.
 * Inside the doubleline portion the wider line has no spaceadd-array, the
 * smaller line has such an array to reach width of the wider line.
 * If the surrounding line has text adjustment and the doubleline portion
 * contains no tabulator, it is necessary to create/manipulate the inner
 * space arrays.
 * --------------------------------------------------*/

/*-----------------01.11.00 14:29-------------------
 * SwDoubleLinePortion::ResetSpaceAdd(..)
 * cancels the manipulation from SwDoubleLinePortion::ChangeSpaceAdd(..)
 * --------------------------------------------------*/


#ifdef BIDI
/*-----------------13.11.00 14:50-------------------
 * SwRubyPortion::SwRubyPortion(..)
 * constructs a ruby portion, i.e. an additional text is displayed
 * beside the main text, e.g. phonetic characters.
 * --------------------------------------------------*/


#endif

/*-----------------13.11.00 14:50-------------------
 * SwRubyPortion::SwRubyPortion(..)
 * constructs a ruby portion, i.e. an additional text is displayed
 * beside the main text, e.g. phonetic characters.
 * --------------------------------------------------*/


/*-----------------13.11.00 14:56-------------------
 * SwRubyPortion::_Adjust(..)
 * In ruby portion there are different alignments for
 * the ruby text and the main text.
 * Left, right, centered and two possibilities of block adjustment
 * The block adjustment is realized by spacing between the characteres,
 * either with a half space or no space in front of the first letter and
 * a half space at the end of the last letter.
 * Notice: the smaller line will be manipulated, normally it's the ruby line,
 * but it could be the main text, too.
 * If there is a tabulator in smaller line, no adjustment is possible.
 * --------------------------------------------------*/


/*-----------------08.11.00 14:14-------------------
 * CalcRubyOffset()
 * has to change the nRubyOffset, if there's a fieldportion
 * in the phonetic line.
 * The nRubyOffset is the position in the rubystring, where the
 * next SwRubyPortion has start the displaying of the phonetics.
 * --------------------------------------------------*/


/*-----------------13.10.00 16:22-------------------
 * SwTxtSizeInfo::GetMultiCreator(..)
 * If we (e.g. the position rPos) are inside a two-line-attribute or
 * a ruby-attribute, the attribute will be returned in a SwMultiCreator-struct,
 * otherwise the function returns zero.
 * The rPos parameter is set to the end of the multiportion,
 * normally this is the end of the attribute,
 * but sometimes it is the start of another attribute, which finished or
 * interrupts the first attribute.
 * E.g. a ruby portion interrupts a 2-line-attribute, a 2-line-attribute
 * with different brackets interrupts another 2-line-attribute.
 * --------------------------------------------------*/

/*-----------------13.11.00 15:38-------------------
 * lcl_Has2Lines(..)
 * is a little help function for GetMultiCreator(..)
 * It extracts the 2-line-format from a 2-line-attribute or a character style.
 * The rValue is set to TRUE, if the 2-line-attribute's value is set and
 * no 2-line-format reference is passed. If there is a 2-line-format reference,
 * then the rValue is set only, if the 2-line-attribute's value is set _and_
 * the 2-line-formats has the same brackets.
 * --------------------------------------------------*/

/*N*/ sal_Bool lcl_Has2Lines( const SwTxtAttr& rAttr, const SvxTwoLinesItem* &rpRef,
/*N*/ 	sal_Bool &rValue )
/*N*/ {
/*N*/ 	if( RES_CHRATR_TWO_LINES == rAttr.Which() )
/*N*/ 	{
/*?*/ 		rValue = rAttr.Get2Lines().GetValue();
/*?*/ 		if( !rpRef )
/*?*/ 			rpRef = &rAttr.Get2Lines();
/*?*/ 		else if( rAttr.Get2Lines().GetEndBracket() != rpRef->GetEndBracket() ||
/*?*/ 			rAttr.Get2Lines().GetStartBracket() != rpRef->GetStartBracket() )
/*?*/ 			rValue = sal_False;
/*?*/ 		return sal_True;
/*N*/ 	}
/*N*/ 	SwCharFmt* pFmt = NULL;
/*N*/ 	if( RES_TXTATR_INETFMT == rAttr.Which() )
/*N*/ 		pFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
/*N*/ 	else if( RES_TXTATR_CHARFMT == rAttr.Which() )
/*N*/ 		pFmt = rAttr.GetCharFmt().GetCharFmt();
/*N*/ 	if ( pFmt )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		if( SFX_ITEM_SET == pFmt->GetAttrSet().
/*N*/ 			GetItemState( RES_CHRATR_TWO_LINES, TRUE, &pItem ) )
/*N*/ 		{
/*?*/ 			rValue = ((SvxTwoLinesItem*)pItem)->GetValue();
/*?*/ 			if( !rpRef )
/*?*/ 				rpRef = (SvxTwoLinesItem*)pItem;
/*?*/ 			else if( ((SvxTwoLinesItem*)pItem)->GetEndBracket() !=
/*?*/ 						rpRef->GetEndBracket() ||
/*?*/ 						((SvxTwoLinesItem*)pItem)->GetStartBracket() !=
/*?*/ 						rpRef->GetStartBracket() )
/*?*/ 				rValue = sal_False;
/*?*/ 			return sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*-----------------16.02.01 16:39-------------------
 * lcl_HasRotation(..)
 * is a little help function for GetMultiCreator(..)
 * It extracts the charrotation from a charrotate-attribute or a character style.
 * The rValue is set to TRUE, if the charrotate-attribute's value is set and
 * no charrotate-format reference is passed.
 * If there is a charrotate-format reference, then the rValue is set only,
 * if the charrotate-attribute's value is set _and_ identical
 * to the charrotate-format's value.
 * --------------------------------------------------*/

/*N*/ sal_Bool lcl_HasRotation( const SwTxtAttr& rAttr,
/*N*/ 	const SvxCharRotateItem* &rpRef, sal_Bool &rValue )
/*N*/ {
/*N*/ 	if( RES_CHRATR_ROTATE == rAttr.Which() )
/*N*/ 	{
/*?*/ 		rValue = 0 != rAttr.GetCharRotate().GetValue();
/*?*/ 		if( !rpRef )
/*?*/ 			rpRef = &rAttr.GetCharRotate();
/*?*/ 		else if( rAttr.GetCharRotate().GetValue() != rpRef->GetValue() )
/*?*/ 			rValue = sal_False;
/*?*/ 		return sal_True;
/*N*/ 	}
/*N*/ 	SwCharFmt* pFmt = NULL;
/*N*/ 	if( RES_TXTATR_INETFMT == rAttr.Which() )
/*N*/ 		pFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
/*N*/ 	else if( RES_TXTATR_CHARFMT == rAttr.Which() )
/*N*/ 		pFmt = rAttr.GetCharFmt().GetCharFmt();
/*N*/ 	if ( pFmt )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		if( SFX_ITEM_SET == pFmt->GetAttrSet().
/*N*/ 			GetItemState( RES_CHRATR_ROTATE, TRUE, &pItem ) )
/*N*/ 		{
/*?*/ 			rValue = 0 != ((SvxCharRotateItem*)pItem)->GetValue();
/*?*/ 			if( !rpRef )
/*?*/ 				rpRef = (SvxCharRotateItem*)pItem;
/*?*/ 			else if( ((SvxCharRotateItem*)pItem)->GetValue() !=
/*?*/ 						rpRef->GetValue() )
/*?*/ 				rValue = sal_False;
/*?*/ 			return sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*N*/ #ifdef BIDI
/*N*/ SwMultiCreator* SwTxtSizeInfo::GetMultiCreator( xub_StrLen &rPos,
/*N*/                                                 SwMultiPortion* pMulti ) const
/*N*/ #else
/*N*/ SwMultiCreator* SwTxtSizeInfo::GetMultiCreator( xub_StrLen &rPos ) const
/*N*/ #endif
/*N*/ {
/*N*/ #ifdef BIDI
/*N*/     SwScriptInfo& rSI = ((SwParaPortion*)GetParaPortion())->GetScriptInfo();
/*N*/ 
/*N*/     // get the last embedding level
/*N*/     BYTE nCurrLevel;
/*N*/     if ( pMulti )
/*N*/     {
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         ASSERT( pMulti->IsBidi(), "Nested MultiPortion is not BidiPortion" )
/*N*/     }
/*N*/     else
/*N*/         // no nested bidi portion required
/*N*/         nCurrLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;
/*N*/ 
/*N*/     // check if there is a field at rPos:
/*N*/     BYTE nNextLevel = nCurrLevel;
/*N*/     sal_Bool bFldBidi = sal_False;
/*N*/ 
/*N*/     if ( CH_TXTATR_BREAKWORD == GetChar( rPos ) )
/*N*/     {
                    bFldBidi = sal_True;
/*		
         // examining the script of the field text should be sufficient
        // for 99% of all cases
         XubString aTxt = GetTxtFrm()->GetTxtNode()->GetExpandTxt( rPos, 1 );
 
         if ( pBreakIt->xBreak.is() && aTxt.Len() )
         {
             sal_Bool bFldDir = ( ::com::sun::star::i18n::ScriptType::COMPLEX ==
                                  pBreakIt->GetRealScriptOfText( aTxt, 0 ) );
             sal_Bool bCurrDir = ( 0 != ( nCurrLevel % 2 ) );
             if ( bFldDir != bCurrDir )
             {
                 nNextLevel = nCurrLevel + 1;
                 bFldBidi = sal_True;
             }
        }*/
/*N*/     }
/*N*/     else
/*N*/         nNextLevel = rSI.DirType( rPos );
/*N*/ 
/*N*/     if ( GetTxt().Len() != rPos && nNextLevel > nCurrLevel )
/*N*/     {
/*?*/         rPos = bFldBidi ? rPos + 1 : rSI.NextDirChg( rPos, &nCurrLevel );
/*?*/         if ( STRING_LEN == rPos )
/*?*/             return NULL;
/*?*/         SwMultiCreator *pRet = new SwMultiCreator;
/*?*/ 		pRet->pItem = NULL;
/*?*/         pRet->pAttr = NULL;
/*?*/         pRet->nId = SW_MC_BIDI;
/*?*/         pRet->nLevel = nCurrLevel + 1;
/*?*/ 		return pRet;
/*N*/     }
/*N*/ 
/*N*/     // a bidi portion can only contain other bidi portions
/*N*/     if ( pMulti )
/*N*/         return NULL;
/*N*/ #endif
/*N*/ 
/*N*/ 	const SvxCharRotateItem* pRotate = NULL;
/*N*/ 	const SfxPoolItem* pRotItem;
/*N*/ 	if( SFX_ITEM_SET == pFrm->GetTxtNode()->GetSwAttrSet().
/*N*/ 		GetItemState( RES_CHRATR_ROTATE, TRUE, &pRotItem ) &&
/*N*/ 		((SvxCharRotateItem*)pRotItem)->GetValue() )
/*?*/ 		pRotate = (SvxCharRotateItem*)pRotItem;
/*N*/ 	else
/*N*/ 		pRotItem = NULL;
/*N*/ 	const SvxTwoLinesItem* p2Lines = NULL;
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	if( SFX_ITEM_SET == pFrm->GetTxtNode()->GetSwAttrSet().
/*N*/ 		GetItemState( RES_CHRATR_TWO_LINES, TRUE, &pItem ) &&
/*N*/ 		((SvxTwoLinesItem*)pItem)->GetValue() )
/*N*/ 		p2Lines = (SvxTwoLinesItem*)pItem;
/*N*/ 	else
/*N*/ 		pItem = NULL;
/*N*/ 
/*N*/ 	const SwpHints *pHints = pFrm->GetTxtNode()->GetpSwpHints();
/*N*/ 	if( !pHints && !p2Lines && !pRotate )
/*N*/ 		return NULL;
/*N*/ 	const SwTxtAttr *pRuby = NULL;
/*N*/ 	sal_Bool bTwo = sal_False;
/*N*/ 	sal_Bool bRot = sal_False;
/*N*/ 	USHORT n2Lines = USHRT_MAX;
/*N*/ 	USHORT nRotate = USHRT_MAX;
/*N*/ 	USHORT nCount = pHints ? pHints->Count() : 0;
/*N*/ 	USHORT i;
/*N*/ 	for( i = 0; i < nCount; ++i )
/*N*/ 	{
/*N*/ 		const SwTxtAttr *pTmp = (*pHints)[i];
/*N*/ 		xub_StrLen nStart = *pTmp->GetStart();
/*N*/ 		if( rPos < nStart )
/*N*/ 			break;
/*N*/ 		if( *pTmp->GetAnyEnd() > rPos )
/*N*/ 		{
/*N*/ 			if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
/*N*/ 				pRuby = pTmp;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				const SvxCharRotateItem* pRoTmp = NULL;
/*N*/ 				if( lcl_HasRotation( *pTmp, pRoTmp, bRot ) )
/*N*/ 				{
/*N*/ 					nRotate = bRot ? i : nCount;
/*N*/ 					pRotate = pRoTmp;
/*N*/ 				}
/*N*/ 				const SvxTwoLinesItem* p2Tmp = NULL;
/*N*/ 				if( lcl_Has2Lines( *pTmp, p2Tmp, bTwo ) )
/*N*/ 				{
/*N*/ 					n2Lines = bTwo ? i : nCount;
/*N*/ 					p2Lines = p2Tmp;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( pRuby )
/*N*/ 	{	// The winner is ... a ruby attribute and so
/*N*/ 		// the end of the multiportion is the end of the ruby attribute.
/*?*/ 		rPos = *pRuby->GetEnd();
/*?*/ 		SwMultiCreator *pRet = new SwMultiCreator;
/*?*/ 		pRet->pItem = NULL;
/*?*/ 		pRet->pAttr = pRuby;
/*?*/ 		pRet->nId = SW_MC_RUBY;
/*?*/ #ifdef BIDI
/*?*/         pRet->nLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;
/*?*/ #endif
/*?*/ 		return pRet;
/*N*/ 	}
/*N*/ 	if( n2Lines < nCount || ( pItem && pItem == p2Lines &&
/*N*/ 		rPos < GetTxt().Len() ) )
/*N*/ 	{	// The winner is a 2-line-attribute,
/*?*/ 		// the end of the multiportion depends on the following attributes...
/*?*/ 		SwMultiCreator *pRet = new SwMultiCreator;
/*?*/ 
/*?*/ 		// We note the endpositions of the 2-line attributes in aEnd as stack
/*?*/ 		SvXub_StrLens aEnd;
/*?*/ 
/*?*/ 		// The bOn flag signs the state of the last 2-line attribute in the
/*?*/ 		// aEnd-stack, it is compatible with the winner-attribute or
/*?*/ 		// it interrupts the other attribute.
/*?*/ 		sal_Bool bOn = sal_True;
/*?*/ 
/*?*/ 		if( n2Lines < nCount )
/*?*/ 		{
/*?*/ 			pRet->pItem = NULL;
/*?*/ 			pRet->pAttr = (*pHints)[n2Lines];
/*?*/ 			aEnd.Insert( *pRet->pAttr->GetEnd(), 0 );
/*?*/ 			if( pItem )
/*?*/ 			{
/*?*/ 				aEnd[ 0 ] = GetTxt().Len();
/*?*/ 				bOn = ((SvxTwoLinesItem*)pItem)->GetEndBracket() ==
/*?*/ 						p2Lines->GetEndBracket() &&
/*?*/ 					  ((SvxTwoLinesItem*)pItem)->GetStartBracket() ==
/*?*/ 						p2Lines->GetStartBracket();
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pRet->pItem = pItem;
/*?*/ 			pRet->pAttr = NULL;
/*?*/ 			aEnd.Insert( GetTxt().Len(), 0 );
/*?*/ 		}
/*?*/ 		pRet->nId = SW_MC_DOUBLE;
/*?*/ #ifdef BIDI
/*?*/         pRet->nLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;
/*?*/ #endif
/*?*/ 
/*?*/ 		// n2Lines is the index of the last 2-line-attribute, which contains
/*?*/ 		// the actual position.
/*?*/ 		i = 0;
/*?*/ 		// At this moment we know that at position rPos the "winner"-attribute
/*?*/ 		// causes a 2-line-portion. The end of the attribute is the end of the
/*?*/ 		// portion, if there's no interrupting attribute.
/*?*/ 		// There are two kinds of interruptors:
/*?*/ 		// - ruby attributes stops the 2-line-attribute, the end of the
/*?*/ 		//	 multiline is the start of the ruby attribute
/*?*/ 		// - 2-line-attributes with value "Off" or with different brackets,
/*?*/ 		//   these attributes may interrupt the winner, but they could be
/*?*/ 		//	 neutralized by another 2-line-attribute starting at the same
/*?*/ 		//	 position with the same brackets as the winner-attribute.
/*?*/ 
/*?*/ 		// In the following loop rPos is the critical position and it will be
/*?*/ 		// evaluated, if at rPos starts a interrupting or a maintaining
/*?*/ 		// continuity attribute.
/*?*/ 		while( i < nCount )
/*?*/ 		{
/*?*/ 			const SwTxtAttr *pTmp = (*pHints)[i++];
/*?*/ 			if( *pTmp->GetAnyEnd() <= rPos )
/*?*/ 				continue;
/*?*/ 			if( rPos < *pTmp->GetStart() )
/*?*/ 			{
/*?*/ 				// If bOn is FALSE and the next attribute starts later than rPos
/*?*/ 				// the winner attribute is interrupted at rPos.
/*?*/ 				// If the start of the next atribute is behind the end of
/*?*/ 				// the last attribute on the aEnd-stack, this is the endposition
/*?*/ 				// on the stack is the end of the 2-line portion.
/*?*/ 				if( !bOn || aEnd[ aEnd.Count()-1 ] < *pTmp->GetStart() )
/*?*/ 					break;
/*?*/ 				// At this moment, bOn is TRUE and the next attribute starts
/*?*/ 				// behind rPos, so we could move rPos to the next startpoint
/*?*/ 				rPos = *pTmp->GetStart();
/*?*/ 				// We clean up the aEnd-stack, endpositions equal to rPos are
/*?*/ 				// superfluous.
/*?*/ 				while( aEnd.Count() && aEnd[ aEnd.Count()-1 ] <= rPos )
/*?*/ 				{
/*?*/ 					bOn = !bOn;
/*?*/ 					aEnd.Remove( aEnd.Count()-1, 1 );
/*?*/ 				}
/*?*/ 				// If the endstack is empty, we simulate an attribute with
/*?*/ 				// state TRUE and endposition rPos
/*?*/ 				if( !aEnd.Count() )
/*?*/ 				{
/*?*/ 					aEnd.Insert( rPos, 0 );
/*?*/ 					bOn = sal_True;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			// A ruby attribute stops the 2-line immediately
/*?*/ 			if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
/*?*/ 				return pRet;
/*?*/ 			if( lcl_Has2Lines( *pTmp, p2Lines, bTwo ) )
/*?*/ 			{   // We have an interesting attribute..
/*?*/ 				if( bTwo == bOn )
/*?*/ 				{   // .. with the same state, so the last attribute could
/*?*/ 					// be continued.
/*?*/ 					if( aEnd[ aEnd.Count()-1 ] < *pTmp->GetEnd() )
/*?*/ 						aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{   // .. with a different state.
/*?*/ 					bOn = bTwo;
/*?*/ 					// If this is smaller than the last on the stack, we put
/*?*/ 					// it on the stack. If it has the same endposition, the last
/*?*/ 					// could be removed.
/*?*/ 					if( aEnd[ aEnd.Count()-1 ] > *pTmp->GetEnd() )
/*?*/ 						aEnd.Insert( *pTmp->GetEnd(), aEnd.Count() );
/*?*/ 					else if( aEnd.Count() > 1 )
/*?*/ 						aEnd.Remove( aEnd.Count()-1, 1 );
/*?*/ 					else
/*?*/ 						aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if( bOn && aEnd.Count() )
/*?*/ 			rPos = aEnd[ aEnd.Count()-1 ];
/*?*/ 		return pRet;
/*N*/ 	}
/*N*/ 	if( nRotate < nCount || ( pRotItem && pRotItem == pRotate &&
/*N*/ 		rPos < GetTxt().Len() ) )
/*N*/ 	{	// The winner is a rotate-attribute,
/*?*/ 		// the end of the multiportion depends on the following attributes...
/*?*/ 		SwMultiCreator *pRet = new SwMultiCreator;
/*?*/ 		pRet->nId = SW_MC_ROTATE;
/*?*/ 
/*?*/ 		// We note the endpositions of the 2-line attributes in aEnd as stack
/*?*/ 		SvXub_StrLens aEnd;
/*?*/ 
/*?*/ 		// The bOn flag signs the state of the last 2-line attribute in the
/*?*/ 		// aEnd-stack, which could interrupts the winning rotation attribute.
/*?*/ 		sal_Bool bOn = pItem ? sal_True : sal_False;
/*?*/ 		aEnd.Insert( GetTxt().Len(), 0 );
/*?*/ 		// n2Lines is the index of the last 2-line-attribute, which contains
/*?*/ 		// the actual position.
/*?*/ 		i = 0;
/*?*/ 		xub_StrLen n2Start = rPos;
/*?*/ 		while( i < nCount )
/*?*/ 		{
/*?*/ 			const SwTxtAttr *pTmp = (*pHints)[i++];
/*?*/ 			if( *pTmp->GetAnyEnd() <= n2Start )
/*?*/ 				continue;
/*?*/ 			if( n2Start < *pTmp->GetStart() )
/*?*/ 			{
/*?*/ 				if( bOn || aEnd[ aEnd.Count()-1 ] < *pTmp->GetStart() )
/*?*/ 					break;
/*?*/ 				n2Start = *pTmp->GetStart();
/*?*/ 				while( aEnd.Count() && aEnd[ aEnd.Count()-1 ] <= n2Start )
/*?*/ 				{
/*?*/ 					bOn = !bOn;
/*?*/ 					aEnd.Remove( aEnd.Count()-1, 1 );
/*?*/ 				}
/*?*/ 				if( !aEnd.Count() )
/*?*/ 				{
/*?*/ 					aEnd.Insert( n2Start, 0 );
/*?*/ 					bOn = sal_False;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			// A ruby attribute stops immediately
/*?*/ 			if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
/*?*/ 			{
/*?*/ 				bOn = sal_True;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			p2Lines = NULL;
/*?*/ 			if( lcl_Has2Lines( *pTmp, p2Lines, bTwo ) )
/*?*/ 			{
/*?*/ 				if( bTwo == bOn )
/*?*/ 				{
/*?*/ 					if( aEnd[ aEnd.Count()-1 ] < *pTmp->GetEnd() )
/*?*/ 						aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					bOn = bTwo;
/*?*/ 					if( aEnd[ aEnd.Count()-1 ] > *pTmp->GetEnd() )
/*?*/ 						aEnd.Insert( *pTmp->GetEnd(), aEnd.Count() );
/*?*/ 					else if( aEnd.Count() > 1 )
/*?*/ 						aEnd.Remove( aEnd.Count()-1, 1 );
/*?*/ 					else
/*?*/ 						aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if( !bOn && aEnd.Count() )
/*?*/ 			n2Start = aEnd[ aEnd.Count()-1 ];
/*?*/ 
/*?*/ 		if( aEnd.Count() )
/*?*/ 			aEnd.Remove( 0, aEnd.Count() );
/*?*/ 
/*?*/ 		bOn = sal_True;
/*?*/ 		if( nRotate < nCount )
/*?*/ 		{
/*?*/ 			pRet->pItem = NULL;
/*?*/ 			pRet->pAttr = (*pHints)[nRotate];
/*?*/ 			aEnd.Insert( *pRet->pAttr->GetEnd(), 0 );
/*?*/ 			if( pRotItem )
/*?*/ 			{
/*?*/ 				aEnd[ 0 ] = GetTxt().Len();
/*?*/ 				bOn = ((SvxCharRotateItem*)pRotItem)->GetValue() ==
/*?*/ 						pRotate->GetValue();
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pRet->pItem = pRotItem;
/*?*/ 			pRet->pAttr = NULL;
/*?*/ 			aEnd.Insert( GetTxt().Len(), 0 );
/*?*/ 		}
/*?*/ 		i = 0;
/*?*/ 		while( i < nCount )
/*?*/ 		{
/*?*/ 			const SwTxtAttr *pTmp = (*pHints)[i++];
/*?*/ 			if( *pTmp->GetAnyEnd() <= rPos )
/*?*/ 				continue;
/*?*/ 			if( rPos < *pTmp->GetStart() )
/*?*/ 			{
/*?*/ 				if( !bOn || aEnd[ aEnd.Count()-1 ] < *pTmp->GetStart() )
/*?*/ 					break;
/*?*/ 				rPos = *pTmp->GetStart();
/*?*/ 				while( aEnd.Count() && aEnd[ aEnd.Count()-1 ] <= rPos )
/*?*/ 				{
/*?*/ 					bOn = !bOn;
/*?*/ 					aEnd.Remove( aEnd.Count()-1, 1 );
/*?*/ 				}
/*?*/ 				if( !aEnd.Count() )
/*?*/ 				{
/*?*/ 					aEnd.Insert( rPos, 0 );
/*?*/ 					bOn = sal_True;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
/*?*/ 			{
/*?*/ 				bOn = sal_False;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			if( lcl_HasRotation( *pTmp, pRotate, bTwo ) )
/*?*/ 			{
/*?*/ 				if( bTwo == bOn )
/*?*/ 				{
/*?*/ 					if( aEnd[ aEnd.Count()-1 ] < *pTmp->GetEnd() )
/*?*/ 						aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					bOn = bTwo;
/*?*/ 					if( aEnd[ aEnd.Count()-1 ] > *pTmp->GetEnd() )
/*?*/ 						aEnd.Insert( *pTmp->GetEnd(), aEnd.Count() );
/*?*/ 					else if( aEnd.Count() > 1 )
/*?*/ 						aEnd.Remove( aEnd.Count()-1, 1 );
/*?*/ 					else
/*?*/ 						aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if( bOn && aEnd.Count() )
/*?*/ 			rPos = aEnd[ aEnd.Count()-1 ];
/*?*/ 		if( rPos > n2Start )
/*?*/ 			rPos = n2Start;
/*?*/ 		return pRet;
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*-----------------01.11.00 14:52-------------------
 * SwSpaceManipulator
 * is a little helper class to manage the spaceadd-arrays of the text adjustment
 * during a PaintMultiPortion.
 * The constructor prepares the array for the first line of multiportion,
 * the SecondLine-function restores the values for the first line and prepares
 * the second line.
 * The destructor restores the values of the last manipulation.
 * --------------------------------------------------*/





/*-----------------13.10.00 16:24-------------------
 * SwTxtPainter::PaintMultiPortion manages the paint for a SwMultiPortion.
 * External, for the calling function, it seems to be a normal Paint-function,
 * internal it is like a SwTxtFrm::Paint with multiple DrawTextLines
 * --------------------------------------------------*/



/*----------------------------------------------------
 *              lcl_TruncateMultiPortion
 * If a multi portion completely has to go to the
 * next line, this function is called to trunctate
 * the rest of the remaining multi portion
 * --------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *              SwTxtFormatter::BuildMultiPortion
 * manages the formatting of a SwMultiPortion. External, for the calling
 * function, it seems to be a normal Format-function, internal it is like a
 * SwTxtFrm::_Format with multiple BuildPortions
 *---------------------------------------------------------------------------*/


/*-----------------08.11.00 09:29-------------------
 * SwTxtFormatter::MakeRestPortion(..)
 * When a fieldportion at the end of line breaks and needs a following
 * fieldportion in the next line, then the "restportion" of the formatinfo
 * has to be set. Normally this happens during the formatting of the first
 * part of the fieldportion.
 * But sometimes the formatting starts at the line with the following part,
 * exspecally when the following part is on the next page.
 * In this case the MakeRestPortion-function has to create the following part.
 * The first parameter is the line that contains possibly a first part
 * of a field. When the function finds such field part, it creates the right
 * restportion. This may be a multiportion, e.g. if the field is surrounded by
 * a doubleline- or ruby-portion.
 * The second parameter is the start index of the line.
 * --------------------------------------------------*/

/*N*/ SwLinePortion* SwTxtFormatter::MakeRestPortion( const SwLineLayout* pLine,
/*N*/ 	xub_StrLen nPos )
/*N*/ {
/*N*/ 	if( !nPos )
/*N*/ 		return NULL;
/*N*/ 	xub_StrLen nMultiPos = nPos - pLine->GetLen();
/*N*/ 	const SwMultiPortion *pTmpMulti = NULL;
/*N*/ 	const SwMultiPortion *pMulti = NULL;
/*N*/ 	const SwLinePortion* pPor = pLine->GetFirstPortion();
/*N*/ 	SwFldPortion *pFld = NULL;
/*N*/ 	while( pPor )
/*N*/ 	{
/*N*/ 		if( pPor->GetLen() )
/*N*/ 		{
/*N*/ 			if( !pMulti )
/*N*/ 			{
/*N*/ 				nMultiPos += pPor->GetLen();
/*N*/ 				pTmpMulti = NULL;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( pPor->InFldGrp() )
/*N*/ 		{
/*?*/ 			if( !pMulti )
/*?*/ 				pTmpMulti = NULL;
/*?*/ 			pFld = (SwFldPortion*)pPor;
/*N*/ 		}
/*N*/ 		else if( pPor->IsMultiPortion() )
/*N*/ 		{
/*?*/ #ifdef BIDI
/*?*/             ASSERT( !pMulti || pMulti->IsBidi(),
/*?*/                     "Nested multiportions are forbidden." );
/*?*/ #else
/*?*/ 			ASSERT( !pMulti, "Nested multiportions are forbidden." );
/*?*/ #endif
/*?*/ 
/*?*/ 			pFld = NULL;
/*?*/ 			pTmpMulti = (SwMultiPortion*)pPor;
/*N*/ 		}
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 		// If the last portion is a multi-portion, we enter it
/*N*/ 		// and look for a field portion inside.
/*N*/ 		// If we are already in a multiportion, we could change to the
/*N*/ 		// next line
/*N*/ 		if( !pPor && pTmpMulti )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*N*/ 		{
/*N*/ 	}
/*N*/ 	if( pFld && !pFld->HasFollow() )
/*N*/ 		pFld = NULL;
/*N*/ 
/*N*/ 	SwLinePortion *pRest = NULL;
/*N*/ 	if( pFld )
/*N*/ 	{
/*?*/ 		const SwTxtAttr *pHint = GetAttr( nPos - 1 );
/*?*/ 		if( pHint && pHint->Which() == RES_TXTATR_FIELD )
/*?*/ 		{
/*?*/ 			pRest = NewFldPortion( GetInfo(), pHint );
/*?*/ 			if( pRest->InFldGrp() )
/*?*/ 				((SwFldPortion*)pRest)->TakeNextOffset( pFld );
/*?*/ 			else
/*?*/ 			{
/*?*/ 				delete pRest;
/*?*/ 				pRest = NULL;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	if( !pMulti )
/*N*/ 		return pRest;
/*N*/ {DBG_BF_ASSERT(0, "STRIP");} 
/*?*/ 	return pRest;
/*N*/ }



/*-----------------23.10.00 10:47-------------------
 * SwTxtCursorSave notes the start and current line of a SwTxtCursor,
 * sets them to the values for GetCrsrOfst inside a multiportion
 * and restores them in the destructor.
 * --------------------------------------------------*/

}

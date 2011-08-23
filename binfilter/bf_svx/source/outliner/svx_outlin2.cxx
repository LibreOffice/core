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




#include <forbiddencharacterstable.hxx>

#define _OUTLINER_CXX

#include <outleeng.hxx>
namespace binfilter {

DBG_NAMEEX(Outliner)//STRIP008

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

// =====================================================================
// ======================   Einfache Durchreicher =======================
// ======================================================================

/*N*/ void Outliner::SetUpdateMode( BOOL bUpdate )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetUpdateMode( bUpdate );
/*N*/ }


/*N*/ BOOL Outliner::GetUpdateMode() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetUpdateMode();
/*N*/ }

/*N*/ const SfxItemSet& Outliner::GetEmptyItemSet() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetEmptyItemSet();
/*N*/ }

/*N*/ void Outliner::EnableUndo( BOOL bEnable )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->EnableUndo( bEnable );
/*N*/ }

/*N*/ BOOL Outliner::IsUndoEnabled() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->IsUndoEnabled();
/*N*/ }

/*N*/ MapMode	Outliner::GetRefMapMode() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetRefMapMode();
/*N*/ }

/*N*/ void Outliner::SetRefMapMode( const MapMode& rMMode )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetRefMapMode( rMMode );
/*N*/ }

/*N*/ void Outliner::SetBackgroundColor( const Color& rColor )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetBackgroundColor( rColor );
/*N*/ }





/*N*/ ULONG Outliner::GetTextHeight() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetTextHeight();
/*N*/ }



/*N*/ void Outliner::SetNotifyHdl( const Link& rLink )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/     pEditEngine->aOutlinerNotifyHdl = rLink;
/*N*/ 
/*N*/     if ( rLink.IsSet() )
/*?*/ 	    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pEditEngine->SetNotifyHdl( LINK( this, Outliner, EditEngineNotifyHdl ) );
/*N*/     else
/*N*/ 	    pEditEngine->SetNotifyHdl( Link() );
/*N*/ 
/*N*/ }




/*N*/ void Outliner::SetDefTab( USHORT nTab )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetDefTab( nTab );
/*N*/ }

    

/*N*/ BOOL Outliner::UpdateFields()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->UpdateFields();
/*N*/ }









/*N*/ void Outliner::SetPaperSize( const Size& rSize )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetPaperSize( rSize );
/*N*/ }

/*N*/ const Size& Outliner::GetPaperSize() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetPaperSize();
/*N*/ }



/*N*/ void Outliner::ClearPolygon()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( Outliner, 0 );
/*N*/ 	pEditEngine->ClearPolygon();
/*N*/ }



/*N*/ void Outliner::SetMinAutoPaperSize( const Size& rSz )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetMinAutoPaperSize( rSz );
/*N*/ }


/*N*/ void Outliner::SetMaxAutoPaperSize( const Size& rSz )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetMaxAutoPaperSize( rSz );
/*N*/ }




/*N*/ Size Outliner::CalcTextSize()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return Size(pEditEngine->CalcTextWidth(),pEditEngine->GetTextHeight());
/*N*/ }


/*N*/ void Outliner::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetStyleSheetPool( pSPool );
/*N*/ }

/*N*/ SfxStyleSheetPool* Outliner::GetStyleSheetPool()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetStyleSheetPool();
/*N*/ }

/*N*/ SfxStyleSheet* Outliner::GetStyleSheet( ULONG nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetStyleSheet( (USHORT)nPara );
/*N*/ }

/*N*/ void Outliner::SetControlWord( ULONG nWord )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetControlWord( nWord );
/*N*/ }

/*N*/ ULONG Outliner::GetControlWord() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetControlWord();
/*N*/ }

/*N*/ void Outliner::SetAsianCompressionMode( USHORT n )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetAsianCompressionMode( n );
/*N*/ }


/*N*/ void Outliner::SetKernAsianPunctuation( BOOL b )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetKernAsianPunctuation( b );
/*N*/ }


/*N*/ void Outliner::UndoActionStart( USHORT nId )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->UndoActionStart( nId );
/*N*/ }

/*N*/ void Outliner::UndoActionEnd( USHORT nId )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->UndoActionEnd( nId );
/*N*/ }


/*N*/ BOOL Outliner::IsInUndo()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->IsInUndo();
/*N*/ }

/*N*/ ULONG Outliner::GetLineCount( ULONG nParagraph ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetLineCount( (USHORT)nParagraph );
/*N*/ }

/*N*/ USHORT Outliner::GetLineLen( ULONG nParagraph, USHORT nLine ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetLineLen( (USHORT)nParagraph, nLine );
/*N*/ }

/*?*/ ULONG Outliner::GetLineHeight( ULONG nParagraph, ULONG nLine )
/*?*/ {
/*?*/ 	DBG_CHKTHIS(Outliner,0);
/*?*/ 	return pEditEngine->GetLineHeight( (USHORT)nParagraph, (USHORT)nLine );
/*?*/ }

/*N*/ void Outliner::QuickRemoveCharAttribs( USHORT nPara, USHORT nWhich )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->QuickRemoveCharAttribs( nPara, nWhich );
/*N*/ }



/*N*/ void Outliner::SetDefaultLanguage( LanguageType eLang )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetDefaultLanguage( eLang );
/*N*/ }





/*N*/ void Outliner::SetEditTextObjectPool( SfxItemPool* pPool )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetEditTextObjectPool( pPool );
/*N*/ }

/*N*/ SfxItemPool* Outliner::GetEditTextObjectPool() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetEditTextObjectPool();
/*N*/ }

/*N*/ void Outliner::SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetForbiddenCharsTable( xForbiddenChars );
/*N*/ }

/*N*/ OutputDevice* Outliner::GetRefDevice() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetRefDevice();
/*N*/ }


/*N*/ ULONG Outliner::GetTextHeight( ULONG nParagraph ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetTextHeight((USHORT)nParagraph );
/*N*/ }

/*N*/ Point Outliner::GetDocPos( const Point& rPaperPos ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetDocPos( rPaperPos );
/*N*/ }

/*N*/ Point Outliner::GetDocPosTopLeft( ULONG nParagraph )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetDocPosTopLeft( (USHORT)nParagraph );
/*N*/ }

/*N*/ BOOL Outliner::IsTextPos( const Point& rPaperPos, USHORT nBorder )
/*N*/ {
/*N*/     return IsTextPos( rPaperPos, nBorder, NULL );
/*N*/ }

/*N*/ BOOL Outliner::IsTextPos( const Point& rPaperPos, USHORT nBorder, BOOL* pbBullet )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/     if ( pbBullet)
/*N*/         *pbBullet = FALSE;
/*N*/ 	BOOL bTextPos = pEditEngine->IsTextPos( rPaperPos, nBorder );
/*N*/ 	if ( !bTextPos )
/*N*/ 	{
/*N*/ 		Point aDocPos = GetDocPos( rPaperPos );
/*N*/ 		USHORT nPara = pEditEngine->FindParagraph( aDocPos.Y() );
/*N*/ 		if ( ( nPara != EE_PARA_NOT_FOUND ) && ImplHasBullet( nPara ) ) 
/*N*/ 		{
/*?*/ 			Rectangle aBulArea = ImpCalcBulletArea( nPara, TRUE, TRUE );
/*?*/ 			if ( aBulArea.IsInside( rPaperPos ) )
/*?*/ 			{
/*?*/ 				bTextPos = TRUE;
/*?*/                 if ( pbBullet)
/*?*/                     *pbBullet = TRUE;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bTextPos;
/*N*/ }

/*N*/ void Outliner::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->QuickSetAttribs( rSet, rSel );
/*N*/ }

/*N*/ void Outliner::QuickInsertText( const XubString& rText, const ESelection& rSel )
/*N*/ {
/*N*/ 	bFirstParaIsEmpty = FALSE;
/*N*/ 	pEditEngine->QuickInsertText( rText, rSel );
/*N*/ }

/*N*/ void Outliner::QuickDelete( const ESelection& rSel )
/*N*/ {
/*N*/ 	bFirstParaIsEmpty = FALSE;
/*N*/ 	pEditEngine->QuickDelete( rSel );
/*N*/ }

/*N*/ void Outliner::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	bFirstParaIsEmpty = FALSE;
/*N*/ 	pEditEngine->QuickInsertField( rFld, rSel );
/*N*/ }

/*N*/ void Outliner::QuickInsertLineBreak( const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	bFirstParaIsEmpty = FALSE;
/*N*/ 	pEditEngine->QuickInsertLineBreak( rSel );
/*N*/ }

/*N*/ void Outliner::QuickFormatDoc( BOOL bFull )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->QuickFormatDoc( bFull );
/*N*/ }

/*N*/ void Outliner::SetGlobalCharStretching( USHORT nX, USHORT nY )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetGlobalCharStretching( nX, nY );
/*N*/ }

/*N*/ void Outliner::EraseVirtualDevice()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->EraseVirtualDevice();
/*N*/ }

/*N*/ BOOL Outliner::ShouldCreateBigTextObject() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->ShouldCreateBigTextObject();
/*N*/ }

/*N*/ void Outliner::SetVertical( BOOL b )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetVertical( b );
/*N*/ }

/*N*/ BOOL Outliner::IsVertical() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->IsVertical();
/*N*/ }



/*N*/ USHORT Outliner::GetScriptType( const ESelection& rSelection ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetScriptType( rSelection );
/*N*/ }

/*N*/ LanguageType Outliner::GetLanguage( USHORT nPara, USHORT nPos ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetLanguage( nPara, nPos );
/*N*/ }





/*N*/ BOOL Outliner::IsForceAutoColor() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->IsForceAutoColor();
/*N*/ }
}

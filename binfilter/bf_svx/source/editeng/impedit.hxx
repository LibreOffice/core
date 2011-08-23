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

#ifndef _IMPEDIT_HXX
#define _IMPEDIT_HXX

#include <editdoc.hxx>
#include <editsel.hxx>
#include <editundo.hxx>
#include <editobj2.hxx>
#include <editstt2.hxx>
#include <editdata.hxx>

#include <vcl/virdev.hxx>
#include <vcl/cursor.hxx>
#include <vcl/dndhelp.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>

#include <bf_svtools/colorcfg.hxx>
#include <rtl/ref.hxx>

namespace binfilter {
class SvtCTLOptions;
class SvKeyValueIterator;
class SvUShorts;

DBG_NAMEEX( EditView )//STRIP008 
DBG_NAMEEX( EditEngine )//STRIP008 

#define PIMPEE		pImpEditView->pEditEngine->pImpEditEngine

#define DEL_LEFT	1
#define DEL_RIGHT	2
#define TRAVEL_X_DONTKNOW	0xFFFFFFFF
#define CURSOR_BIDILEVEL_DONTKNOW	0xFFFF
#define MAXCHARSINPARA		0x3FFF-CHARPOSGROW	// Max 16K, because WYSIWYG array

#define ATTRSPECIAL_WHOLEWORD	1
#define ATTRSPECIAL_EDGE		2

#define GETCRSR_TXTONLY		        0x0001
#define GETCRSR_STARTOFLINE	        0x0002
#define GETCRSR_ENDOFLINE	        0x0004
#define GETCRSR_PREFERPORTIONSTART  0x0008

#define LINE_SEP	0x0A

typedef EENotify* EENotifyPtr;
SV_DECL_PTRARR_DEL( NotifyList, EENotifyPtr, 1, 1 )//STRIP008 ;    // IMPL is in outliner.cxx, move to EE later and share declaration, or use BlockNotifications from EE directly


class EditView;
class EditEngine;
class EditUndoRemoveChars;
class SvxFontTable;
class SvxColorList;

class SvxLRSpaceItem;
class TextRanger;
class SvxForbiddenCharactersTable;


}//end of namespace binfilter
namespace com {
namespace sun {
namespace star {
namespace datatransfer {
namespace clipboard {
    class XClipboard;
}}}}}

namespace binfilter {

struct ImplIMEInfos
{
    String      aOldTextAfterStartPos;
    sal_uInt16* pAttribs;
    EditPaM		aPos;
    sal_uInt16	nLen;
    sal_Bool	bCursor;
    sal_Bool	bWasCursorOverwrite;
};


struct FormatterFontMetric
{
    sal_uInt16 nMaxAscent;
    sal_uInt16 nMaxDescent;

    FormatterFontMetric()				{ nMaxAscent = 0; nMaxDescent = 0; /* nMinLeading = 0xFFFF; */ }
    sal_uInt16	GetHeight() const		{ return nMaxAscent+nMaxDescent; }
};

//	----------------------------------------------------------------------
//	class ImpEditView
//	----------------------------------------------------------------------
class ImpEditView : public vcl::unohelper::DragAndDropClient
{
    friend class EditView;
    friend class EditEngine;
    friend class ImpEditEngine;

private:
    EditView*           pEditView;
    Cursor*				pCursor;
    Color*				pBackgroundColor;
    EditEngine*			pEditEngine;
    Window*				pOutWin;
    Pointer*			pPointer;

    long				nInvMore;
    ULONG				nControl;
    sal_uInt32			nTravelXPos;
    sal_uInt16          nExtraCursorFlags;
    sal_uInt16          nCursorBidiLevel;
    sal_uInt16			nScrollDiffX;
    sal_Bool			bReadOnly;
    sal_Bool            bClickedInSelection;
    sal_Bool            bActiveDragAndDropListener;

    Point				aAnchorPoint;
    Rectangle			aOutArea;
    Point				aVisDocStartPos;
    EESelectionMode		eSelectionMode;
    EditSelection		aEditSelection;
    EVAnchorMode		eAnchorMode;

protected:

    // DragAndDropClient



public:
                    ~ImpEditView();

    EditView*       GetEditViewPtr() { return pEditView; }

    sal_uInt16		GetScrollDiffX() const 			{ return nScrollDiffX; }
    void			SetScrollDiffX( sal_uInt16 n ) 	{ nScrollDiffX = n; }

    sal_uInt16      GetCursorBidiLevel() const      { return nCursorBidiLevel; }
    void            SetCursorBidiLevel( sal_uInt16 n ) { nCursorBidiLevel = n; }

    Point			GetDocPos( const Point& rWindowPos ) const;
    Point			GetWindowPos( const Point& rDocPos ) const;
    Rectangle 		GetWindowPos( const Rectangle& rDocPos ) const;

    const Rectangle&	GetOutputArea() const 	{ return aOutArea; }

    BOOL			IsVertical() const;

    void			SetVisDocStartPos( const Point& rPos ) { aVisDocStartPos = rPos; }
    const Point&	GetVisDocStartPos() const { return aVisDocStartPos; }

    long			GetVisDocLeft() const { return aVisDocStartPos.X(); }
    long			GetVisDocTop() const { return aVisDocStartPos.Y(); }
    long			GetVisDocRight() const { return aVisDocStartPos.X() + ( !IsVertical() ? aOutArea.GetWidth() : aOutArea.GetHeight() ); }
    long			GetVisDocBottom() const { return aVisDocStartPos.Y() + ( !IsVertical() ? aOutArea.GetHeight() : aOutArea.GetWidth() ); }
    Rectangle		GetVisDocArea() const;

    EditSelection&	GetEditSelection() 			{ return aEditSelection; }
    void			SetEditSelection( const EditSelection& rEditSelection );
    sal_Bool		HasSelection() const { return aEditSelection.HasRange(); }

    void			DrawSelection() { DrawSelection( aEditSelection ); }
    void			DrawSelection( EditSelection, Region* pRegion = NULL );

    Window*			GetWindow()	const 			{ return pOutWin; }

    EESelectionMode	GetSelectionMode() const 	{ return eSelectionMode; }

    inline void		SetPointer( const Pointer& rPointer );
    inline const Pointer&	GetPointer();

    inline void		SetCursor( const Cursor& rCursor );
    inline Cursor*	GetCursor();


    EVAnchorMode	GetAnchorMode() const 			{ return eAnchorMode; }
    void			ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, BOOL test );
    void			ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, USHORT nShowCursorFlags = 0 );

    sal_Bool			IsInsertMode() const 			{ return ( ( nControl & EV_CNTRL_OVERWRITE ) == 0 ); }

    void			    EnablePaste( sal_Bool bEnable ) 	{ SetFlags( nControl, EV_CNTRL_ENABLEPASTE, bEnable ); }
    sal_Bool			IsPasteEnabled() const			{ return ( ( nControl & EV_CNTRL_ENABLEPASTE ) != 0 ); }

    sal_Bool			DoSingleLinePaste() const		{ return ( ( nControl & EV_CNTRL_SINGLELINEPASTE ) != 0 ); }
    sal_Bool			DoAutoScroll() const			{ return ( ( nControl & EV_CNTRL_AUTOSCROLL ) != 0 ); }
    sal_Bool			DoBigScroll() const				{ return ( ( nControl & EV_CNTRL_BIGSCROLL ) != 0 ); }
    sal_Bool			DoAutoSize() const				{ return ( ( nControl & EV_CNTRL_AUTOSIZE ) != 0 ); }
    sal_Bool			DoAutoWidth() const				{ return ( ( nControl & EV_CNTRL_AUTOSIZEX) != 0 ); }
    sal_Bool			DoAutoHeight() const			{ return ( ( nControl & EV_CNTRL_AUTOSIZEY) != 0 ); }
    sal_Bool			DoInvalidateMore() const		{ return ( ( nControl & EV_CNTRL_INVONEMORE ) != 0 ); }

    const Color&	GetBackgroundColor() const {
                        return ( pBackgroundColor ? *pBackgroundColor : pOutWin->GetBackground().GetColor() ); }

    // Ggf. mehr als OutputArea invalidieren, fuer den DrawingEngine-Textrahmen...
    void			SetInvalidateMore( sal_uInt16 nPixel ) { nInvMore = nPixel; }
    sal_uInt16		GetInvalidateMore() const { return (sal_uInt16)nInvMore; }
};

//	----------------------------------------------------------------------
//	ImpEditEngine
//	----------------------------------------------------------------------

typedef EditView* EditViewPtr;
SV_DECL_PTRARR( EditViews, EditViewPtr, 0, 1 )//STRIP008 ;

class ImpEditEngine : public SfxListener
{
    // Die Undos muessen direkt manipulieren ( private-Methoden ),
    // damit keine neues Undos eingefuegt werden!
    friend class EditUndoInsertChars;
    friend class EditUndoRemoveChars;
    friend class EditUndoDelContent;
    friend class EditUndoConnectParas;
    friend class EditUndoInsertFeature;
    friend class EditUndoMoveParagraphs;

    friend class EditView;
    friend class ImpEditView;
    friend class EditEngine;		// Fuer Zugriff auf Imp-Methoden
    friend class EditRTFParser;		// Fuer Zugriff auf Imp-Methoden
    friend class EditHTMLParser;	// Fuer Zugriff auf Imp-Methoden
    friend class EdtAutoCorrDoc;	// Fuer Zugriff auf Imp-Methoden
    friend class EditDbg;			// DebugRoutinen

private:

    // ================================================================
    // Daten...
    // ================================================================

    // Dokument-Spezifische Daten...
    ParaPortionList		aParaPortionList;		// Formatierung
    Size				aPaperSize;				// Layout
    Size				aMinAutoPaperSize;		// Layout ?
    Size				aMaxAutoPaperSize;		// Layout ?
    EditDoc				aEditDoc;				// Dokumenteninhalt

    // Engine-Spezifische Daten....
    EditEngine*			pEditEngine;
    EditViews			aEditViews;
    EditView*			pActiveView;
    TextRanger*			pTextRanger;

    SfxStyleSheetPool*	pStylePool;
    SfxItemPool*		pTextObjectPool;

    VirtualDevice*		pVirtDev;
    OutputDevice*		pRefDev;

    ColorConfig*   pColorConfig;
    SvtCTLOptions*      pCTLOptions;

    SfxItemSet*			pEmptyItemSet;
    EditUndoManager*	pUndoManager;
    ESelection*			pUndoMarkSelection;

    ImplIMEInfos*		mpIMEInfos;

    NotifyList          aNotifyCache;

    XubString			aWordDelimiters;
    XubString			aGroupChars;

    EditSelFunctionSet	aSelFuncSet;
    EditSelectionEngine	aSelEngine;

    Color				maBackgroundColor;

    sal_uInt32          nBlockNotifications;
    sal_uInt16			nStretchX;
    sal_uInt16			nStretchY;

    USHORT              nAsianCompressionMode;
    BOOL				bKernAsianPunctuation;

    EEHorizontalTextDirection eDefaultHorizontalTextDirection;

    sal_uInt16			nBigTextObjectStart;
    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > xBI;

    XubString			aAutoCompleteText;

    InternalEditStatus	aStatus;

    LanguageType		eDefLanguage;

    sal_Bool			bIsFormatting;
    sal_Bool			bFormatted;
    sal_Bool			bInSelection;
    sal_Bool			bIsInUndo;
    sal_Bool			bUpdate;
    sal_Bool			bUndoEnabled;
    sal_Bool			bOwnerOfRefDev;
    sal_Bool			bDowning;
    sal_Bool            bUseAutoColor;
    sal_Bool            bForceAutoColor;
    sal_Bool			bCallParaInsertedOrDeleted;

    // Fuer Formatierung / Update....
    DeletedNodesList	aDeletedNodes;
    Rectangle			aInvalidRec;
    sal_uInt32			nCurTextHeight;
    sal_uInt16			nOnePixelInRef;

    // Wenn an einer Stelle erkannt wird, dass der StatusHdl gerufen werden
    // muss, dies aber nicht sofort geschehen darf (kritischer Abschnitt):
    Link				aNotifyHdl;
    Link				aImportHdl;
    Link                aBeginMovingParagraphsHdl;
    Link                aEndMovingParagraphsHdl;
    Link                aBeginPasteOrDropHdl;
    Link                aEndPasteOrDropHdl;
    Link                aModifyHdl;

    rtl::Reference<SvxForbiddenCharactersTable>	xForbiddenCharsTable;


    // ================================================================
    // Methoden...
    // ================================================================

    void				CursorMoved( ContentNode* pPrevNode );
    void				ParaAttribsChanged( ContentNode* pNode );
    void			    TextModified();
    void				CalcHeight( ParaPortion* pPortion );

    // ggf. lieber inline, aber so einiges...
    void				InsertUndo( EditUndo* pUndo, sal_Bool bTryMerge = sal_False );
    void				ResetUndoManager();
    sal_Bool			HasUndoManager() const	{ return pUndoManager ? sal_True : sal_False; }


    EditPaM				GetPaM( Point aDocPos, sal_Bool bSmart = sal_True );
    EditPaM				GetPaM( ParaPortion* pPortion, Point aPos, sal_Bool bSmart = sal_True );
    long                GetPortionXOffset( ParaPortion* pParaPortion, EditLine* pLine, USHORT nTextPortion );
    USHORT				GetChar( ParaPortion* pParaPortion, EditLine* pLine, long nX, BOOL bSmart = TRUE );
    Range				GetInvalidYOffsets( ParaPortion* pPortion );
    Range               GetLineXPosStartEnd( ParaPortion* pParaPortion, EditLine* pLine );

    void				SetParaAttrib( BYTE nFunc, EditSelection aSel, sal_uInt16 nValue );
    sal_uInt16			GetParaAttrib( BYTE nFunc, EditSelection aSel );
    void				SetCharAttrib( EditSelection aSel, const SfxPoolItem& rItem );

    EditTextObject*		CreateBinTextObject( EditSelection aSelection, SfxItemPool*, sal_Bool bAllowBigObjects = sal_False, sal_uInt16 nBigObjStart = 0 ) const;
    void 				StoreBinTextObject( SvStream& rOStream, BinTextObject& rTextObject );
    EditSelection		InsertBinTextObject( BinTextObject&, EditPaM aPaM );

    EditPaM				Clear();
    EditPaM				RemoveText();
    EditPaM				RemoveText( EditSelection aEditSelection );
    sal_Bool			CreateLines( USHORT nPara, sal_uInt32 nStartPosY );
    void				CreateAndInsertEmptyLine( ParaPortion* pParaPortion, sal_uInt32 nStartPosY );
    sal_Bool			FinishCreateLines( ParaPortion* pParaPortion );
    void				CalcCharPositions( ParaPortion* pParaPortion );
    void				CreateTextPortions( ParaPortion* pParaPortion, sal_uInt16& rStartPos /*, sal_Bool bCreateBlockPortions */ );
    void				RecalcTextPortion( ParaPortion* pParaPortion, sal_uInt16 nStartPos, short nNewChars );
    sal_uInt16			SplitTextPortion( ParaPortion* pParaPortion, sal_uInt16 nPos,  EditLine* pCurLine = 0 );
    void				SeekCursor( ContentNode* pNode, sal_uInt16 nPos, SvxFont& rFont, OutputDevice* pOut = NULL, sal_uInt16 nIgnoreWhich = 0 );
    void				RecalcFormatterFontMetrics( FormatterFontMetric& rCurMetrics, SvxFont& rFont );
    void				CheckAutoPageSize();

    void				ImpBreakLine( ParaPortion* pParaPortion, EditLine* pLine, TextPortion* pPortion, sal_uInt16 nPortionStart, long nRemainingWidth, sal_Bool bCanHyphenate );
    void				ImpAdjustBlocks( ParaPortion* pParaPortion, EditLine* pLine, long nRemainingSpace );
    EditPaM				ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, sal_Bool bBackward = sal_False );
    EditPaM				ImpDeleteSelection( EditSelection aEditSelection);
    EditPaM				ImpInsertParaBreak( const EditPaM& rPaM, sal_Bool bKeepEndingAttribs = sal_True );
    EditPaM				ImpInsertText( EditSelection aCurEditSelection, const String& rStr );
    EditPaM				ImpInsertFeature( EditSelection aEditSelection, const SfxPoolItem& rItem );
    void				ImpRemoveChars( const EditPaM& rPaM, sal_uInt16 nChars, EditUndoRemoveChars* pCurUndo = 0 );

    EditPaM				ImpFastInsertText( EditPaM aPaM, const String& rStr );
    EditPaM 			ImpFastInsertParagraph( sal_uInt16 nPara );

    sal_Bool			ImpCheckRefMapMode();

    BOOL                ImplHasText() const;

    void                ImpFindKashidas( ContentNode* pNode, USHORT nStart, USHORT nEnd, SvUShorts& rArray );



    EditPaM				WordRight( const EditPaM& rPaM, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    EditSelection		SelectWord( const EditSelection& rCurSelection, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, BOOL bAcceptStartOfWord = TRUE );
    EditPaM             CursorVisualLeftRight( EditView* pEditView, const EditPaM& rPaM, USHORT nCharacterIteratorMode, BOOL bToLeft );
    EditPaM             CursorVisualStartEnd( EditView* pEditView, const EditPaM& rPaM, BOOL bStart );


    void				InitScriptTypes( USHORT nPara );
    USHORT				GetScriptType( const EditPaM& rPaM, USHORT* pEndPos = NULL ) const;
    USHORT				GetScriptType( const EditSelection& rSel ) const;
    BOOL				IsScriptChange( const EditPaM& rPaM ) const;
    BOOL                HasScriptType( USHORT nPara, USHORT nType ) const;


    void                ImplInitLayoutMode( OutputDevice* pOutDev, USHORT nPara, USHORT nIndex );



    inline short		GetXValue( short nXValue ) const;
    inline sal_uInt16	GetXValue( sal_uInt16 nXValue ) const;
    inline long			GetXValue( long nXValue ) const;

    inline short		GetYValue( short nYValue ) const;
    inline sal_uInt16	GetYValue( sal_uInt16 nYValue ) const;



    void				SetBackgroundColor( const Color& rColor ) { maBackgroundColor = rColor; }
    Color				GetBackgroundColor() const { return maBackgroundColor; }

    Color				GetAutoColor() const;
    void                EnableAutoColor( BOOL b ) { bUseAutoColor = b; }
    BOOL                IsAutoColorEnabled() const { return bUseAutoColor; }
    void                ForceAutoColor( BOOL b ) { bForceAutoColor = b; }
    BOOL                IsForceAutoColor() const { return bForceAutoColor; }

    inline VirtualDevice*	GetVirtualDevice( const MapMode& rMapMode );
    inline void				EraseVirtualDevice();

    void				CheckIdleFormatter();

    inline ParaPortion*	FindParaPortion( ContentNode* pNode ) const;

    void				SetValidPaperSize( const Size& rSz );

    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > ImplGetBreakIterator();



protected:

public:
                            ImpEditEngine( EditEngine* pEditEngine, SfxItemPool* pPool );
                            ~ImpEditEngine();

    void					InitDoc( BOOL bKeepParaAttribs );
    EditDoc&				GetEditDoc()			{ return aEditDoc; }
    const EditDoc&			GetEditDoc() const		{ return aEditDoc; }

    inline EditUndoManager&	GetUndoManager();

    void					SetUpdateMode( sal_Bool bUp, EditView* pCurView = 0, sal_Bool bForceUpdate = sal_False );
    sal_Bool				GetUpdateMode()	const	{ return bUpdate; }

    const ParaPortionList&	GetParaPortions() const	{ return aParaPortionList; }
    ParaPortionList&		GetParaPortions() 		{ return aParaPortionList; }
    EditViews&				GetEditViews() 			{ return aEditViews; }

    const Size&				GetPaperSize() const					{ return aPaperSize; }
    void					SetPaperSize( const Size& rSz ) 		{ aPaperSize = rSz; }

    void					SetVertical( BOOL bVertical );
    BOOL					IsVertical() const 						{ return GetEditDoc().IsVertical(); }

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir ) { eDefaultHorizontalTextDirection = eHTextDir; }
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const { return eDefaultHorizontalTextDirection; }


    void				    InitWritingDirections( USHORT nPara );
    BOOL                    IsRightToLeft( USHORT nPara ) const;
    BYTE                    GetRightToLeft( USHORT nPara, USHORT nChar, USHORT* pStart = NULL, USHORT* pEnd = NULL );
    BOOL                    HasDifferentRTLLevels( const ContentNode* pNode );

    void					SetTextRanger( TextRanger* pRanger );
    TextRanger*				GetTextRanger() const { return pTextRanger; }

    const Size&				GetMinAutoPaperSize() const				{ return aMinAutoPaperSize; }
    void					SetMinAutoPaperSize( const Size& rSz )	{ aMinAutoPaperSize = rSz; }

    const Size&				GetMaxAutoPaperSize() const				{ return aMaxAutoPaperSize; }
    void					SetMaxAutoPaperSize( const Size& rSz )	{ aMaxAutoPaperSize = rSz; }

    void					FormatDoc();
    void					FormatFullDoc();
    void					UpdateViews( EditView* pCurView = 0 );
    void					Paint( ImpEditView* pView, const Rectangle& rRect, sal_Bool bUseVirtDev = sal_False );
    void					Paint( OutputDevice* pOutDev, Rectangle aClipRec, Point aStartPos, sal_Bool bStripOnly = sal_False, short nOrientation = 0 );


    EditSelectionEngine& 	GetSelEngine() { return aSelEngine; }
    XubString				GetSelected( const EditSelection& rSel, const LineEnd eParaSep = LINEEND_LF ) const;

    const SfxItemSet&		GetEmptyItemSet();

    void					UpdateSelections();

    void					EnableUndo( sal_Bool bEnable );
    sal_Bool				IsUndoEnabled()			{ return bUndoEnabled; }
    void					SetUndoMode( sal_Bool b ) 	{ bIsInUndo = b; }
    sal_Bool				IsInUndo()				{ return bIsInUndo; }

    void					SetCallParaInsertedOrDeleted( sal_Bool b ) { bCallParaInsertedOrDeleted = b; }
    sal_Bool				IsCallParaInsertedOrDeleted() const { return bCallParaInsertedOrDeleted; }

    sal_Bool				IsFormatted() const { return bFormatted; }
    sal_Bool				IsFormatting() const { return bIsFormatting; }

    void 			SetText( const String& rText );
    EditPaM			InsertLineBreak( EditSelection aEditSelection );
    EditPaM			InsertField( EditSelection aEditSelection, const SvxFieldItem& rFld );
    sal_Bool		UpdateFields();


    EditTextObject*	CreateTextObject();
    EditTextObject*	CreateTextObject( EditSelection aSel );
    void			SetText( const EditTextObject& rTextObject );
    EditSelection	InsertText( const EditTextObject& rTextObject, EditSelection aSel );



    sal_uInt32		CalcTextHeight();
    sal_uInt32		GetTextHeight() const;
    sal_uInt32		CalcTextWidth( BOOL bIgnoreExtraSpace );
    sal_uInt32 		CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, BOOL bIgnoreExtraSpace );
    sal_uInt16		GetLineCount( sal_uInt16 nParagraph ) const;

    SfxItemSet      GetAttribs( USHORT nPara, USHORT nStart, USHORT nEnd, sal_uInt8 nFlags = 0xFF ) const;
    SfxItemSet		GetAttribs( EditSelection aSel, BOOL bOnlyHardAttrib = FALSE  );
    void			SetAttribs( EditSelection aSel, const SfxItemSet& rSet, BYTE nSpecial = 0 );
    void			RemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich = 0, sal_Bool bRemoveFeatures = sal_False );

    void				SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet );
    const SfxItemSet& 	GetParaAttribs( sal_uInt16 nPara ) const;

    sal_Bool			HasParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&	GetParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich ) const;


    sal_Bool		IsModified() const 		{ return aEditDoc.IsModified(); }
    void			SetModifyFlag( sal_Bool b ) { aEditDoc.SetModified( b ); }
    void            SetModifyHdl( const Link& rLink ) { aModifyHdl = rLink; }
    Link            GetModifyHdl() const { return aModifyHdl; }


    sal_Bool		IsInSelectionMode() { return bInSelection; }


//	Fuer Undo/Redo

//	OV-Special
    EditPaM			InsertParagraph( sal_uInt16 nPara );
    EditSelection*	SelectParagraph( sal_uInt16 nPara );

    void			SetNotifyHdl( const Link& rLink )	    { aNotifyHdl = rLink; }
    Link			GetNotifyHdl() const   			{ return aNotifyHdl; }

    void			FormatAndUpdate( EditView* pCurView = 0 );
    inline void		IdleFormatAndUpdate( EditView* pCurView = 0 );

    ColorConfig& GetColorConfig();
    BOOL            IsVisualCursorTravelingEnabled();
    BOOL            DoVisualCursorTraveling( const ContentNode* pNode );

    EditSelection			ConvertSelection( sal_uInt16 nStartPara, sal_uInt16 nStartPos, sal_uInt16 nEndPara, sal_uInt16 nEndPos ) const;
    inline EPaM				CreateEPaM( const EditPaM& rPaM );
    inline EditPaM			CreateEditPaM( const EPaM& rEPaM );
    inline ESelection		CreateESel( const EditSelection& rSel );
    inline EditSelection	CreateSel( const ESelection& rSel );


    void				SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*	GetStyleSheetPool() const { return pStylePool; }

    void				SetStyleSheet( sal_uInt16 nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*		GetStyleSheet( sal_uInt16 nPara ) const;


    OutputDevice*		GetRefDevice() const { return pRefDev; }
    void				SetRefDevice( OutputDevice* pRefDef );

    const MapMode&		GetRefMapMode() { return pRefDev->GetMapMode(); }
    void				SetRefMapMode( const MapMode& rMapMode );

    InternalEditStatus&	GetStatus()	{ return aStatus; }
    void				CallStatusHdl();

    void                EnterBlockNotifications();
    void                LeaveBlockNotifications();

    void				UndoActionStart( sal_uInt16 nId );
    void				UndoActionEnd( sal_uInt16 nId );

    EditView*			GetActiveView()	const	{ return pActiveView; }
    void				SetActiveView( EditView* pView );

    void				SetDefaultLanguage( LanguageType eLang ) { eDefLanguage = eLang; }
    LanguageType		GetDefaultLanguage() const { return eDefLanguage; }

    LanguageType        GetLanguage( const EditSelection rSelection ) const;
    LanguageType		GetLanguage( const EditPaM& rPaM, USHORT* pEndPos = NULL ) const;
    ::com::sun::star::lang::Locale GetLocale( const EditPaM& rPaM ) const;



    void				SetEditTextObjectPool( SfxItemPool* pP )	{ pTextObjectPool = pP; }
    SfxItemPool*		GetEditTextObjectPool() const				{ return pTextObjectPool; }

    const SvxLRSpaceItem&	GetLRSpaceItem( ContentNode* pNode );
    SvxAdjust               GetJustification( USHORT nPara ) const;

    void				SetCharStretching( sal_uInt16 nX, sal_uInt16 nY );
    inline void			GetCharStretching( sal_uInt16& rX, sal_uInt16& rY );

    void				SetBigTextObjectStart( sal_uInt16 nStartAtPortionCount )	{ nBigTextObjectStart = nStartAtPortionCount; }
    sal_uInt16			GetBigTextObjectStart() const								{ return nBigTextObjectStart; }

    inline EditEngine*	GetEditEnginePtr() const	{ return pEditEngine; }

    void                SetAsianCompressionMode( USHORT n );
    USHORT              GetAsianCompressionMode() const { return nAsianCompressionMode; }

    void                SetKernAsianPunctuation( BOOL b );
    BOOL				IsKernAsianPunctuation() const { return bKernAsianPunctuation; }

    rtl::Reference<SvxForbiddenCharactersTable>	GetForbiddenCharsTable( BOOL bGetInternal = TRUE ) const;
    void				SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );
};

inline EPaM ImpEditEngine::CreateEPaM( const EditPaM& rPaM )
{
    ContentNode* pNode = rPaM.GetNode();
    return EPaM( aEditDoc.GetPos( pNode ), rPaM.GetIndex() );
}

inline EditPaM ImpEditEngine::CreateEditPaM( const EPaM& rEPaM )
{
    DBG_ASSERT( rEPaM.nPara < aEditDoc.Count(), "CreateEditPaM: Ungueltiger Absatz" );
    DBG_ASSERT( aEditDoc[ rEPaM.nPara ]->Len() >= rEPaM.nIndex, "CreateEditPaM: Ungueltiger Index" );
    return EditPaM( aEditDoc[ rEPaM.nPara], rEPaM.nIndex );
}

inline ESelection ImpEditEngine::CreateESel( const EditSelection& rSel )
{
    ContentNode* pStartNode = rSel.Min().GetNode();
    ContentNode* pEndNode = rSel.Max().GetNode();
    ESelection aESel;
    aESel.nStartPara = aEditDoc.GetPos( pStartNode );
    aESel.nStartPos = rSel.Min().GetIndex();
    aESel.nEndPara = aEditDoc.GetPos( pEndNode );
    aESel.nEndPos = rSel.Max().GetIndex();
    return aESel;
}

inline EditSelection ImpEditEngine::CreateSel( const ESelection& rSel )
{
    DBG_ASSERT( rSel.nStartPara < aEditDoc.Count(), "CreateSel: Ungueltiger Start-Absatz" );
    DBG_ASSERT( rSel.nEndPara < aEditDoc.Count(), "CreateSel: Ungueltiger End-Absatz" );
    EditSelection aSel;
    aSel.Min().SetNode( aEditDoc[ rSel.nStartPara ] );
    aSel.Min().SetIndex( rSel.nStartPos );
    aSel.Max().SetNode( aEditDoc[ rSel.nEndPara ] );
    aSel.Max().SetIndex( rSel.nEndPos );
    return aSel;
}

inline VirtualDevice* ImpEditEngine::GetVirtualDevice( const MapMode& rMapMode )
{
    if ( !pVirtDev )
        pVirtDev = new VirtualDevice;

    if ( ( pVirtDev->GetMapMode().GetMapUnit() != rMapMode.GetMapUnit() ) ||
         ( pVirtDev->GetMapMode().GetScaleX() != rMapMode.GetScaleX() ) ||
         ( pVirtDev->GetMapMode().GetScaleY() != rMapMode.GetScaleY() ) )
    {
        MapMode aMapMode( rMapMode );
        aMapMode.SetOrigin( Point( 0, 0 ) );
        pVirtDev->SetMapMode( aMapMode );
    }

    return pVirtDev;
}

inline void ImpEditEngine::EraseVirtualDevice()
{
    delete pVirtDev;
    pVirtDev = 0;
}

#ifndef SVX_LIGHT
inline EditUndoManager&	ImpEditEngine::GetUndoManager()
{
    if ( !pUndoManager )
        pUndoManager = new EditUndoManager( this );
    return *pUndoManager;
}
#endif

inline ParaPortion*	ImpEditEngine::FindParaPortion( ContentNode* pNode ) const
{
    sal_uInt16 nPos = aEditDoc.GetPos( pNode );
    DBG_ASSERT( nPos < GetParaPortions().Count(), "Portionloser Node?" );
    return GetParaPortions()[ nPos ];
}

inline void	ImpEditEngine::GetCharStretching( sal_uInt16& rX, sal_uInt16& rY )
{
    rX = nStretchX;
    rY = nStretchY;
}

inline short ImpEditEngine::GetXValue( short nXValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchX == 100 ) )
        return nXValue;

    return (short) ((long)nXValue*nStretchX/100);
}

inline sal_uInt16 ImpEditEngine::GetXValue( sal_uInt16 nXValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchX == 100 ) )
        return nXValue;

    return (sal_uInt16) ((long)nXValue*nStretchX/100);
}

inline long ImpEditEngine::GetXValue( long nXValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchX == 100 ) )
        return nXValue;

    return nXValue*nStretchX/100;
}

inline short ImpEditEngine::GetYValue( short nYValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchY == 100 ) )
        return nYValue;

    return (short) ((long)nYValue*nStretchY/100);
}

inline sal_uInt16 ImpEditEngine::GetYValue( sal_uInt16 nYValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchY == 100 ) )
        return nYValue;

    return (sal_uInt16) ((long)nYValue*nStretchY/100);
}

inline void ImpEditView::SetPointer( const Pointer& rPointer )
{
    delete pPointer;
    pPointer = new Pointer( rPointer );
}

inline const Pointer& ImpEditView::GetPointer()
{
    if ( !pPointer )
    {
        pPointer = new Pointer( IsVertical() ? POINTER_TEXT_VERTICAL : POINTER_TEXT );
        return *pPointer;
    }

    if(POINTER_TEXT == pPointer->GetStyle() && IsVertical())
    {
        delete pPointer;
        pPointer = new Pointer(POINTER_TEXT_VERTICAL);
    }
    else if(POINTER_TEXT_VERTICAL == pPointer->GetStyle() && !IsVertical())
    {
        delete pPointer;
        pPointer = new Pointer(POINTER_TEXT);
    }

    return *pPointer;
}

inline void ImpEditView::SetCursor( const Cursor& rCursor )
{
    delete pCursor;
    pCursor = new Cursor( rCursor );
}

inline Cursor* ImpEditView::GetCursor()
{
    if ( !pCursor )
        pCursor = new Cursor;
    return pCursor;
}

void ConvertItem( SfxPoolItem& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit );
BYTE GetCharTypeForCompression( xub_Unicode cChar );
Point Rotate( const Point& rPoint, short nOrientation, const Point& rOrigin );

}//end of namespace binfilter
#endif // _IMPEDIT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

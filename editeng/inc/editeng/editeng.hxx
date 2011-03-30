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
// MyEDITENG, due to exported EditEng
#ifndef _MyEDITENG_HXX
#define _MyEDITENG_HXX

class ImpEditEngine;
class EditView;
class OutputDevice;
class EditUndo;
class SvxFont;
class SfxItemPool;
class SfxStyleSheet;
class String;
class SfxStyleSheetPool;
class SvxSearchItem;
class SvxFieldItem;
class SvxCharSetColorItem;
class SfxUndoAction;
class MapMode;
class Color;
class Font;
class KeyEvent;
class PolyPolygon;
class Size;
class Point;
class Rectangle;
class SvStream;
class Link;
class OutputDevice;
class Window;
class SvUShorts;
class SfxPoolItem;
class SvxNumBulletItem;
class SvxBulletItem;
class SvxLRSpaceItem;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class SvxNumberFormat;
class FontList;

#include <rtl/ref.hxx>
#include <vector>
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
  namespace linguistic2 {
    class XSpellChecker1;
    class XHyphenator;
  }
  namespace datatransfer {
    class XTransferable;
  }
  namespace lang {
    struct Locale;
  }
}}}
namespace svx{
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}
namespace svl{
class IUndoManager;
}

namespace basegfx { class B2DPolyPolygon; }
#include <rsc/rscsfx.hxx>
#include <editeng/editdata.hxx>
#include <i18npool/lang.h>
#include "editeng/editengdllapi.h"

#include <tools/rtti.hxx>   // due to typedef TypeId

#include <editeng/eedata.hxx>
class SvxFieldData;

//////////////////////////////////////////////////////////////////////////////

/** values for GetAttribs
*/
const sal_uInt8 EditEngineAttribs_All   = 0;        /// returns all attributes even when theire not set
const sal_uInt8 EditEngineAttribs_HardAndPara = 1;  /// returns all attributes set on paragraph and on portions
const sal_uInt8 EditEngineAttribs_OnlyHard = 2;     /// returns only attributes hard set on portions

#define GETATTRIBS_STYLESHEET   (sal_uInt8)0x01
#define GETATTRIBS_PARAATTRIBS  (sal_uInt8)0x02
#define GETATTRIBS_CHARATTRIBS  (sal_uInt8)0x04
#define GETATTRIBS_ALL          (sal_uInt8)0xFF

class EDITENG_DLLPUBLIC EditEngine
{
    friend class EditView;
    friend class ImpEditView;
    friend class EditDbg;
    friend class Outliner;

private:
    ImpEditEngine*  pImpEditEngine;

                    EDITENG_DLLPRIVATE EditEngine( const EditEngine& );
    EDITENG_DLLPRIVATE EditEngine&      operator=( const EditEngine& );
    EDITENG_DLLPRIVATE sal_uInt8        PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pView, Window* pFrameWin = NULL );

protected:


public:
                    EditEngine( SfxItemPool* pItemPool );
    virtual         ~EditEngine();

    const SfxItemSet&   GetEmptyItemSet();

    void            SetDefTab( sal_uInt16 nDefTab );
    sal_uInt16          GetDefTab() const;

    void            SetRefDevice( OutputDevice* pRefDef );
    OutputDevice*   GetRefDevice() const;

    void            SetRefMapMode( const MapMode& rMapMode );
    MapMode         GetRefMapMode();

    void            SetUpdateMode( sal_Bool bUpdate );
    sal_Bool            GetUpdateMode() const;

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;
    Color           GetAutoColor() const;
    void            EnableAutoColor( sal_Bool b );
    sal_Bool            IsAutoColorEnabled() const;
    void            ForceAutoColor( sal_Bool b );
    sal_Bool            IsForceAutoColor() const;

    void            InsertView( EditView* pEditView, sal_uInt16 nIndex = EE_APPEND );
    EditView*       RemoveView( EditView* pEditView );
    EditView*       RemoveView( sal_uInt16 nIndex = EE_APPEND );
    EditView*       GetView( sal_uInt16 nIndex = 0 ) const;
    sal_uInt16          GetViewCount() const;
    sal_Bool            HasView( EditView* pView ) const;
    EditView*       GetActiveView() const;
    void            SetActiveView( EditView* pView );

    void            SetPaperSize( const Size& rSize );
    const Size&     GetPaperSize() const;

    void            SetVertical( sal_Bool bVertical );
    sal_Bool            IsVertical() const;

    void            SetFixedCellHeight( sal_Bool bUseFixedCellHeight );
    sal_Bool            IsFixedCellHeight() const;

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    sal_uInt16          GetScriptType( const ESelection& rSelection ) const;
    LanguageType    GetLanguage( sal_uInt16 nPara, sal_uInt16 nPos ) const;

    void            TransliterateText( const ESelection& rSelection, sal_Int32 nTransliterationMode );

    void            SetAsianCompressionMode( sal_uInt16 nCompression );
    sal_uInt16          GetAsianCompressionMode() const;

    void            SetKernAsianPunctuation( sal_Bool bEnabled );
    sal_Bool            IsKernAsianPunctuation() const;

    void            SetAddExtLeading( sal_Bool b );
    sal_Bool            IsAddExtLeading() const;

    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    void                ClearPolygon();
    const PolyPolygon*  GetPolygon();

    const Size&     GetMinAutoPaperSize() const;
    void            SetMinAutoPaperSize( const Size& rSz );

    const Size&     GetMaxAutoPaperSize() const;
    void            SetMaxAutoPaperSize( const Size& rSz );

    String          GetText( LineEnd eEnd = LINEEND_LF ) const;
    String          GetText( const ESelection& rSelection, const LineEnd eEnd = LINEEND_LF ) const;
    sal_uInt32      GetTextLen() const;
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth();

    String          GetText( sal_uInt16 nParagraph ) const;
    xub_StrLen      GetTextLen( sal_uInt16 nParagraph ) const;
    sal_uInt32      GetTextHeight( sal_uInt16 nParagraph ) const;

    sal_uInt16          GetParagraphCount() const;

    sal_uInt16          GetLineCount( sal_uInt16 nParagraph ) const;
    xub_StrLen      GetLineLen( sal_uInt16 nParagraph, sal_uInt16 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_uInt16 nParagraph, sal_uInt16 nLine ) const;
    sal_uInt16          GetLineNumberAtIndex( sal_uInt16 nPara, sal_uInt16 nIndex ) const;
    sal_uInt32      GetLineHeight( sal_uInt16 nParagraph, sal_uInt16 nLine = 0 );
    sal_uInt16          GetFirstLineOffset( sal_uInt16 nParagraph );
    ParagraphInfos  GetParagraphInfos( sal_uInt16 nPara );
    sal_uInt16          FindParagraph( long nDocPosY );
    EPosition       FindDocPosition( const Point& rDocPos ) const;
    Rectangle       GetCharacterBounds( const EPosition& rPos ) const;

    String          GetWord( sal_uInt16 nPara, xub_StrLen nIndex );

    ESelection      GetWord( const ESelection& rSelection, sal_uInt16 nWordType ) const;
    ESelection      WordLeft( const ESelection& rSelection, sal_uInt16 nWordType  ) const;
    ESelection      WordRight( const ESelection& rSelection, sal_uInt16 nWordType  ) const;
    ESelection      CursorLeft( const ESelection& rSelection, sal_uInt16 nCharacterIteratorMode ) const;
    ESelection      CursorRight( const ESelection& rSelection, sal_uInt16 nCharacterIteratorMode ) const;
    ESelection      SelectSentence( const ESelection& rCurSel ) const;

    void            Clear();
    void            SetText( const String& rStr );

    EditTextObject* CreateTextObject();
    EditTextObject* CreateTextObject( sal_uInt16 nPara, sal_uInt16 nParas = 1 );
    EditTextObject* CreateTextObject( const ESelection& rESelection );
    void            SetText( const EditTextObject& rTextObject );

    void            RemoveParagraph( sal_uInt16 nPara );
    void            InsertParagraph( sal_uInt16 nPara, const EditTextObject& rTxtObj );
    void            InsertParagraph( sal_uInt16 nPara, const String& rText);

    void            SetText( sal_uInt16 nPara, const EditTextObject& rTxtObj );
    void            SetText( sal_uInt16 nPara, const String& rText);

    virtual void                SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet );
    virtual const SfxItemSet&   GetParaAttribs( sal_uInt16 nPara ) const;

    void                GetCharAttribs( sal_uInt16 nPara, EECharAttribArray& rLst ) const;

    SfxItemSet          GetAttribs( sal_uInt16 nPara, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt8 nFlags = 0xFF ) const;
    SfxItemSet          GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib = EditEngineAttribs_All );

    sal_Bool                HasParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich );

    Font            GetStandardFont( sal_uInt16 nPara );
    SvxFont         GetStandardSvxFont( sal_uInt16 nPara );

    void            RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich );

    void            ShowParagraph( sal_uInt16 nParagraph, sal_Bool bShow = sal_True );
    sal_Bool            IsParagraphVisible( sal_uInt16 nParagraph );

    ::svl::IUndoManager&
                    GetUndoManager();
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionEnd( sal_uInt16 nId );
    sal_Bool        IsInUndo();

    void            EnableUndo( sal_Bool bEnable );
    sal_Bool            IsUndoEnabled();

    /** returns the value last used for bTryMerge while calling ImpEditEngine::InsertUndo
        This is currently used in a bad but needed hack to get undo actions merged in the
        OutlineView in impress. Do not use it unless you want to sell your soul too! */
    sal_Bool            HasTriedMergeOnLastAddUndo() const;

    void            ClearModifyFlag();
    void            SetModified();
    sal_Bool            IsModified() const;

    void            SetModifyHdl( const Link& rLink );
    Link            GetModifyHdl() const;

    sal_Bool            IsInSelectionMode() const;
    void            StopSelectionMode();

    void            StripPortions();
    void            GetPortions( sal_uInt16 nPara, SvUShorts& rList );

    long            GetFirstLineStartX( sal_uInt16 nParagraph );
    Point           GetDocPosTopLeft( sal_uInt16 nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    sal_Bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder = 0 );

    // StartDocPos corrresponds to VisArea.TopLeft().
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos, sal_Bool bClip );
    void            Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation = 0 );

//  sal_uInt32: Error code of the stream.
        sal_uLong               Read( SvStream& rInput, const String& rBaseURL, EETextFormat, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );
    sal_uLong       Write( SvStream& rOutput, EETextFormat );

    void            SetStatusEventHdl( const Link& rLink );
    Link            GetStatusEventHdl() const;

    void            SetNotifyHdl( const Link& rLink );
    Link            GetNotifyHdl() const;

    void            SetImportHdl( const Link& rLink );
    Link            GetImportHdl() const;

    // Do not evaluate font formatting => For Outliner
    sal_Bool            IsFlatMode() const;
    void            SetFlatMode( sal_Bool bFlat );

    void            SetControlWord( sal_uInt32 nWord );
    sal_uInt32      GetControlWord() const;

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickRemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich = 0 );
    void            QuickMarkInvalid( const ESelection& rSel );
    void            QuickFormatDoc( sal_Bool bFull = sal_False );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );
    void            QuickInsertText( const String& rText, const ESelection& rSel );
    void            QuickDelete( const ESelection& rSel );
    void            QuickMarkToBeRepainted( sal_uInt16 nPara );

    void            SetGlobalCharStretching( sal_uInt16 nX = 100, sal_uInt16 nY = 100 );
    void            GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY );
    void            DoStretchChars( sal_uInt16 nX, sal_uInt16 nY );

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    void                SetStyleSheet( sal_uInt16 nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*      GetStyleSheet( sal_uInt16 nPara ) const;

    void            SetWordDelimiters( const String& rDelimiters );
    String          GetWordDelimiters() const;

    void            SetGroupChars( const String& rChars );
    String          GetGroupChars() const;

    void            EnablePasteSpecial( sal_Bool bEnable );
    sal_Bool            IsPasteSpecialEnabled() const;

    void            EnableIdleFormatter( sal_Bool bEnable );
    sal_Bool            IsIdleFormatterEnabled() const;

    void            EraseVirtualDevice();

    void            SetSpeller( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XSpellChecker1 > &xSpeller );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                    GetSpeller();
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >
                    GetHyphenator() const;
    void            SetHyphenator( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XHyphenator >& xHyph );

    void            SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );
    rtl::Reference<SvxForbiddenCharactersTable> GetForbiddenCharsTable() const;

    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    sal_Bool            HasOnlineSpellErrors() const;
    void            CompleteOnlineSpelling();

    void            SetBigTextObjectStart( sal_uInt16 nStartAtPortionCount );
    sal_uInt16          GetBigTextObjectStart() const;
    sal_Bool            ShouldCreateBigTextObject() const;

    // For fast Pre-Test without view:
    EESpellState    HasSpellErrors();
    sal_Bool            HasText( const SvxSearchItem& rSearchItem );

    //initialize sentence spelling
    void            StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc);
    //spell and return a sentence
    bool            SpellSentence(EditView& rEditView, ::svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    // put spell position to start of current sentence
    void            PutSpellingToSentenceStart( EditView& rEditView );
    //applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions, bool bRecheck );
    //deinitialize sentence spelling
    void            EndSpelling();

    // for text conversion (see also HasSpellErrors)
    sal_Bool        HasConvertibleTextPortion( LanguageType nLang );
    virtual sal_Bool    ConvertNextDocument();

    sal_Bool            UpdateFields();
    void            RemoveFields( sal_Bool bKeepFieldText, TypeId aType = NULL );

    sal_uInt16          GetFieldCount( sal_uInt16 nPara ) const;
    EFieldInfo      GetFieldInfo( sal_uInt16 nPara, sal_uInt16 nField ) const;

    sal_Bool            IsRightToLeft( sal_uInt16 nPara ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >
                    CreateTransferable( const ESelection& rSelection ) const;

    // MT: Can't create new virtual functions like for ParagraphInserted/Deleted, musst be compatible in SRC638, change later...
    void            SetBeginMovingParagraphsHdl( const Link& rLink );
    void            SetEndMovingParagraphsHdl( const Link& rLink );
    void            SetBeginPasteOrDropHdl( const Link& rLink );
    void            SetEndPasteOrDropHdl( const Link& rLink );

    virtual void    PaintingFirstLine( sal_uInt16 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );
    virtual void    ParagraphInserted( sal_uInt16 nNewParagraph );
    virtual void    ParagraphDeleted( sal_uInt16 nDeletedParagraph );
    virtual void    ParagraphConnected( sal_uInt16 nLeftParagraph, sal_uInt16 nRightParagraph );
    virtual void    ParaAttribsChanged( sal_uInt16 nParagraph );
    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );
    virtual void    ParagraphHeightChanged( sal_uInt16 nPara );

    virtual void DrawingText(
        const Point& rStartPos, const String& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen, const sal_Int32* pDXArray,
        const SvxFont& rFont, sal_uInt16 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
        const EEngineData::WrongSpellVector* pWrongSpellVector,
        const SvxFieldData* pFieldData,
        bool bEndOfLine,
        bool bEndOfParagraph,
        bool bEndOfBullet,
        const ::com::sun::star::lang::Locale* pLocale,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    virtual void DrawingTab(
        const Point& rStartPos, long nWidth, const String& rChar,
        const SvxFont& rFont, sal_uInt16 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor);
    virtual String  GetUndoComment( sal_uInt16 nUndoId ) const;
    virtual sal_Bool    FormattingParagraph( sal_uInt16 nPara );
    virtual sal_Bool    SpellNextDocument();
    virtual void    FieldClicked( const SvxFieldItem& rField, sal_uInt16 nPara, xub_StrLen nPos );
    virtual void    FieldSelected( const SvxFieldItem& rField, sal_uInt16 nPara, xub_StrLen nPos );
    virtual String  CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, xub_StrLen nPos, Color*& rTxtColor, Color*& rFldColor );

    // to be overloaded if access to bullet information needs to be provided
    virtual const SvxNumberFormat * GetNumberFormat( sal_uInt16 nPara ) const;

    virtual Rectangle GetBulletArea( sal_uInt16 nPara );

    static SfxItemPool* CreatePool( sal_Bool bLoadRefCounts = sal_True );
    static SfxItemPool& GetGlobalItemPool();
    static sal_uInt32   RegisterClipboardFormatName();
    static sal_Bool     DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static sal_Bool     DoesKeyMoveCursor( const KeyEvent& rKeyEvent );
    static sal_Bool     IsSimpleCharInput( const KeyEvent& rKeyEvent );
    static sal_uInt16   GetAvailableSearchOptions();
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const Font& rFont );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const SvxFont& rFont );
    static Font     CreateFontFromItemSet( const SfxItemSet& rItemSet );
    static Font     CreateFontFromItemSet( const SfxItemSet& rItemSet, sal_uInt16 nScriptType );
    static SvxFont  CreateSvxFontFromItemSet( const SfxItemSet& rItemSet );
    static void     ImportBulletItem( SvxNumBulletItem& rNumBullet, sal_uInt16 nLevel, const SvxBulletItem* pOldBullet, const SvxLRSpaceItem* pOldLRSpace );
    static sal_Bool     IsPrintable( sal_Unicode c ) { return ( ( c >= 32 ) && ( c != 127 ) ); }
    static sal_Bool     HasValidData( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rTransferable );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link& rLink );
    Link            GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link& rLink );
    Link            GetEndDropHdl() const;

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void            SetFirstWordCapitalization( sal_Bool bCapitalize );
    sal_Bool            IsFirstWordCapitalization() const;
};

#endif // _MyEDITENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

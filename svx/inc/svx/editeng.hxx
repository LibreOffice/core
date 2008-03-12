/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editeng.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:25:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
// MyEDITENG, wegen exportiertem EditEng
#ifndef _MyEDITENG_HXX
#define _MyEDITENG_HXX

class ImpEditEngine;
class EditView;
class OutputDevice;
class EditUndo;
class SvxFont;
class SfxUndoManager;
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
class SvUShorts;
class SfxPoolItem;
class SvxNumBulletItem;
class SvxBulletItem;
class SvxLRSpaceItem;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;

#include <vos/ref.hxx>
#include <vector>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com { namespace sun { namespace star {
  namespace linguistic2 {
    class XSpellChecker1;
    class XHyphenator;
}
  namespace datatransfer {
    class XTransferable;
}
}}}
namespace svx{
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}

namespace basegfx { class B2DPolyPolygon; }

#ifndef _RSCSFX_HXX //autogen
#include <rsc/rscsfx.hxx>
#endif
#ifndef _EDITDATA_HXX
#include <svx/editdata.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <tools/rtti.hxx>   // wegen typedef TypeId

/** values for GetAttribs
*/
const sal_uInt8 EditEngineAttribs_All   = 0;        /// returns all attributes even when theire not set
const sal_uInt8 EditEngineAttribs_HardAndPara = 1;  /// returns all attributes set on paragraph and on portions
const sal_uInt8 EditEngineAttribs_OnlyHard = 2;     /// returns only attributes hard set on portions

#define GETATTRIBS_STYLESHEET   (sal_uInt8)0x01
#define GETATTRIBS_PARAATTRIBS  (sal_uInt8)0x02
#define GETATTRIBS_CHARATTRIBS  (sal_uInt8)0x04
#define GETATTRIBS_ALL          (sal_uInt8)0xFF

class SVX_DLLPUBLIC EditEngine
{
    friend class EditView;
    friend class ImpEditView;
    friend class EditDbg;
    friend class Outliner;

private:
    ImpEditEngine*  pImpEditEngine;

                    SVX_DLLPRIVATE EditEngine( const EditEngine& );
    SVX_DLLPRIVATE EditEngine&      operator=( const EditEngine& );

//#if 0 // _SOLAR__PRIVATE
    SVX_DLLPRIVATE BOOL             PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pView );
//#endif

public:
                    EditEngine( SfxItemPool* pItemPool );
    virtual         ~EditEngine();

    const SfxItemSet&   GetEmptyItemSet();

    void            SetDefTab( USHORT nDefTab );
    USHORT          GetDefTab() const;

    void            SetRefDevice( OutputDevice* pRefDef );
    OutputDevice*   GetRefDevice() const;

    void            SetRefMapMode( const MapMode& rMapMode );
    MapMode         GetRefMapMode();

    void            SetUpdateMode( BOOL bUpdate );
    BOOL            GetUpdateMode() const;

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;
    Color           GetAutoColor() const;
    void            EnableAutoColor( BOOL b );
    BOOL            IsAutoColorEnabled() const;
    void            ForceAutoColor( BOOL b );
    BOOL            IsForceAutoColor() const;

    void            InsertView( EditView* pEditView, USHORT nIndex = EE_APPEND );
    EditView*       RemoveView( EditView* pEditView );
    EditView*       RemoveView( USHORT nIndex = EE_APPEND );
    EditView*       GetView( USHORT nIndex = 0 ) const;
    USHORT          GetViewCount() const;
    BOOL            HasView( EditView* pView ) const;
    EditView*       GetActiveView() const;
    void            SetActiveView( EditView* pView );

    void            SetPaperSize( const Size& rSize );
    const Size&     GetPaperSize() const;

    void            SetVertical( BOOL bVertical );
    BOOL            IsVertical() const;

    void            SetFixedCellHeight( BOOL bUseFixedCellHeight );
    BOOL            IsFixedCellHeight() const;

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    USHORT          GetScriptType( const ESelection& rSelection ) const;
    LanguageType    GetLanguage( USHORT nPara, USHORT nPos ) const;

    void            TransliterateText( const ESelection& rSelection, sal_Int32 nTransliterationMode );

    void            SetAsianCompressionMode( USHORT nCompression );
    USHORT          GetAsianCompressionMode() const;

    void            SetKernAsianPunctuation( BOOL bEnabled );
    BOOL            IsKernAsianPunctuation() const;

    void            SetAddExtLeading( BOOL b );
    BOOL            IsAddExtLeading() const;

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
    sal_uInt32      CalcTextWidth();

    String          GetText( USHORT nParagraph ) const;
    xub_StrLen      GetTextLen( USHORT nParagraph ) const;
    sal_uInt32      GetTextHeight( USHORT nParagraph ) const;

    USHORT          GetParagraphCount() const;

    USHORT          GetLineCount( USHORT nParagraph ) const;
    xub_StrLen      GetLineLen( USHORT nParagraph, USHORT nLine ) const;
    sal_uInt32      GetLineHeight( USHORT nParagraph, USHORT nLine = 0 );
    USHORT          GetFirstLineOffset( USHORT nParagraph );
    ParagraphInfos  GetParagraphInfos( USHORT nPara );
    USHORT          FindParagraph( long nDocPosY );
    EPosition       FindDocPosition( const Point& rDocPos ) const;
    Rectangle       GetCharacterBounds( const EPosition& rPos ) const;

    String          GetWord( USHORT nPara, xub_StrLen nIndex );

    ESelection      GetWord( const ESelection& rSelection, USHORT nWordType ) const;
    ESelection      WordLeft( const ESelection& rSelection, USHORT nWordType  ) const;
    ESelection      WordRight( const ESelection& rSelection, USHORT nWordType  ) const;
    ESelection      CursorLeft( const ESelection& rSelection, USHORT nCharacterIteratorMode ) const;
    ESelection      CursorRight( const ESelection& rSelection, USHORT nCharacterIteratorMode ) const;

    void            Clear();
    void            SetText( const String& rStr );

    EditTextObject* CreateTextObject();
    EditTextObject* CreateTextObject( USHORT nPara, USHORT nParas = 1 );
    EditTextObject* CreateTextObject( const ESelection& rESelection );
    void            SetText( const EditTextObject& rTextObject );

    void            RemoveParagraph( USHORT nPara );
    void            InsertParagraph( USHORT nPara, const EditTextObject& rTxtObj );
    void            InsertParagraph( USHORT nPara, const String& rText);

    void            SetText( USHORT nPara, const EditTextObject& rTxtObj );
    void            SetText( USHORT nPara, const String& rText);

    void                SetParaAttribs( USHORT nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( USHORT nPara ) const;

    void                GetCharAttribs( USHORT nPara, EECharAttribArray& rLst ) const;

    SfxItemSet          GetAttribs( USHORT nPara, USHORT nStart, USHORT nEnd, sal_uInt8 nFlags = 0xFF ) const;
    SfxItemSet          GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib = EditEngineAttribs_All );

    BOOL                HasParaAttrib( USHORT nPara, USHORT nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( USHORT nPara, USHORT nWhich );

    Font            GetStandardFont( USHORT nPara );
    SvxFont         GetStandardSvxFont( USHORT nPara );

    void            RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich );

    void            ShowParagraph( USHORT nParagraph, BOOL bShow = TRUE );
    BOOL            IsParagraphVisible( USHORT nParagraph );

    SfxUndoManager& GetUndoManager();
    void            UndoActionStart( USHORT nId );
    void            UndoActionEnd( USHORT nId );
    BOOL            IsInUndo();

    void            EnableUndo( BOOL bEnable );
    BOOL            IsUndoEnabled();

    /** returns the value last used for bTryMerge while calling ImpEditEngine::InsertUndo
        This is currently used in a bad but needed hack to get undo actions merged in the
        OutlineView in impress. Do not use it unless you want to sell your soul too! */
    BOOL            HasTriedMergeOnLastAddUndo() const;

    void            ClearModifyFlag();
    void            SetModified();
    BOOL            IsModified() const;

    void            SetModifyHdl( const Link& rLink );
    Link            GetModifyHdl() const;

    BOOL            IsInSelectionMode() const;
    void            StopSelectionMode();

    void            StripPortions();
    void            GetPortions( USHORT nPara, SvUShorts& rList );

    long            GetFirstLineStartX( USHORT nParagraph );
    Point           GetDocPosTopLeft( USHORT nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    BOOL            IsTextPos( const Point& rPaperPos, USHORT nBorder = 0 );

    // StartDocPos entspr. VisArea.TopLeft().
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos, BOOL bClip );
    void            Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation = 0 );

//  ULONG: Fehlercode des Streams.
    ULONG           Read( SvStream& rInput, const String& rBaseURL, EETextFormat, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );
    ULONG           Write( SvStream& rOutput, EETextFormat );

    void            SetStatusEventHdl( const Link& rLink );
    Link            GetStatusEventHdl() const;

    void            SetNotifyHdl( const Link& rLink );
    Link            GetNotifyHdl() const;

    void            SetImportHdl( const Link& rLink );
    Link            GetImportHdl() const;

    // Flat-Mode: Keine Zeichenformatierung auswerten => Fuer Outliner
    BOOL            IsFlatMode() const;
    void            SetFlatMode( BOOL bFlat );

    void            SetControlWord( sal_uInt32 nWord );
    sal_uInt32      GetControlWord() const;

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickRemoveCharAttribs( USHORT nPara, USHORT nWhich = 0 );
    void            QuickMarkInvalid( const ESelection& rSel );
    void            QuickFormatDoc( BOOL bFull = FALSE );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );
    void            QuickInsertText( const String& rText, const ESelection& rSel );
    void            QuickDelete( const ESelection& rSel );
    void            QuickMarkToBeRepainted( USHORT nPara );

    void            SetGlobalCharStretching( USHORT nX = 100, USHORT nY = 100 );
    void            GetGlobalCharStretching( USHORT& rX, USHORT& rY );
    void            DoStretchChars( USHORT nX, USHORT nY );

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    void                SetStyleSheet( USHORT nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*      GetStyleSheet( USHORT nPara ) const;

    void            SetWordDelimiters( const String& rDelimiters );
    String          GetWordDelimiters() const;

    void            SetGroupChars( const String& rChars );
    String          GetGroupChars() const;

    void            EnablePasteSpecial( BOOL bEnable );
    BOOL            IsPasteSpecialEnabled() const;

    void            EnableIdleFormatter( BOOL bEnable );
    BOOL            IsIdleFormatterEnabled() const;

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

    void            SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars );
    vos::ORef<SvxForbiddenCharactersTable>  GetForbiddenCharsTable() const;

    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    BOOL            HasOnlineSpellErrors() const;
    void            CompleteOnlineSpelling();

    void            SetBigTextObjectStart( USHORT nStartAtPortionCount );
    USHORT          GetBigTextObjectStart() const;
    BOOL            ShouldCreateBigTextObject() const;

    // Zum schnellen Vorab-Pruefen ohne View:
    EESpellState    HasSpellErrors();
    BOOL            HasText( const SvxSearchItem& rSearchItem );

    //initialize sentence spelling
    void            StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc);
    //spell and return a sentence
    bool            SpellSentence(EditView& rEditView, ::svx::SpellPortions& rToFill);
    //applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions);
    //deinitialize sentence spelling
    void            EndSpelling();

    // for text conversion (see also HasSpellErrors)
    sal_Bool        HasConvertibleTextPortion( LanguageType nLang );
    virtual BOOL    ConvertNextDocument();

    BOOL            UpdateFields();
    void            RemoveFields( BOOL bKeepFieldText, TypeId aType = NULL );

    USHORT          GetFieldCount( USHORT nPara ) const;
    EFieldInfo      GetFieldInfo( USHORT nPara, USHORT nField ) const;

    BOOL            IsRightToLeft( USHORT nPara ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >
                    CreateTransferable( const ESelection& rSelection ) const;

    // #110496#
    /** Enable/disable verbose comments of recorded metafiles

        This method decorates recorded metafiles with XTEXT_EOx
        comments, to transport the logical text structure like
        paragraph, line, sentence, word and cell breaks.
     */
    void            EnableVerboseTextComments( BOOL bEnable = TRUE );
    BOOL            IsVerboseTextComments() const;

    // MT: Can't create new virtual functions like for ParagraphInserted/Deleted, musst be compatible in SRC638, change later...
    void            SetBeginMovingParagraphsHdl( const Link& rLink );
    void            SetEndMovingParagraphsHdl( const Link& rLink );
    void            SetBeginPasteOrDropHdl( const Link& rLink );
    void            SetEndPasteOrDropHdl( const Link& rLink );

    virtual void    PaintingFirstLine( USHORT nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );
    virtual void    ParagraphInserted( USHORT nNewParagraph );
    virtual void    ParagraphDeleted( USHORT nDeletedParagraph );
    virtual void    ParaAttribsChanged( USHORT nParagraph );
    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );
    virtual void    ParagraphHeightChanged( USHORT nPara );

    // #101498#
    virtual void    DrawingText( const Point& rStartPos, const String& rText, USHORT nTextStart, USHORT nTextLen, const sal_Int32* pDXArray, const SvxFont& rFont, USHORT nPara, xub_StrLen nIndex, BYTE nRightToLeft);

    virtual String  GetUndoComment( USHORT nUndoId ) const;
    virtual BOOL    FormattingParagraph( USHORT nPara );
    virtual BOOL    SpellNextDocument();
    virtual void    FieldClicked( const SvxFieldItem& rField, USHORT nPara, xub_StrLen nPos );
    virtual void    FieldSelected( const SvxFieldItem& rField, USHORT nPara, xub_StrLen nPos );
    virtual String  CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, xub_StrLen nPos, Color*& rTxtColor, Color*& rFldColor );
    virtual Rectangle GetBulletArea( USHORT nPara );

    static SfxItemPool* CreatePool( BOOL bLoadRefCounts = TRUE );
    static sal_uInt32   RegisterClipboardFormatName();
    static BOOL     DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static BOOL     DoesKeyMoveCursor( const KeyEvent& rKeyEvent );
    static BOOL     IsSimpleCharInput( const KeyEvent& rKeyEvent );
    static USHORT   GetAvailableSearchOptions();
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const Font& rFont );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const SvxFont& rFont );
    static Font     CreateFontFromItemSet( const SfxItemSet& rItemSet );
    static Font     CreateFontFromItemSet( const SfxItemSet& rItemSet, USHORT nScriptType );
    static SvxFont  CreateSvxFontFromItemSet( const SfxItemSet& rItemSet );
    static void     ImportBulletItem( SvxNumBulletItem& rNumBullet, USHORT nLevel, const SvxBulletItem* pOldBullet, const SvxLRSpaceItem* pOldLRSpace );
    static BOOL     IsPrintable( sal_Unicode c ) { return ( ( c >= 32 ) && ( c != 127 ) ); }
    static BOOL     HasValidData( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rTransferable );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link& rLink );
    Link            GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link& rLink );
    Link            GetEndDropHdl() const;

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void            SetFirstWordCapitalization( BOOL bCapitalize );
    BOOL            IsFirstWordCapitalization() const;
};

#endif // _MyEDITENG_HXX

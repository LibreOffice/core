/*************************************************************************
 *
 *  $RCSfile: editdoc.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-21 12:02:04 $
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

#ifndef _EDITDOC_HXX
#define _EDITDOC_HXX

#include <editattr.hxx>
#include <edtspell.hxx>
#include <svxfont.hxx>

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _TOOLS_TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

class ImpEditEngine;
class SvxTabStop;

DBG_NAMEEX( EE_TextPortion );

#define CHARPOSGROW     16
#define DEFTAB          720

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, BOOL bSearchInParent = TRUE );
USHORT GetScriptItemId( USHORT nItemId, short nScriptType );
BOOL IsScriptItemValid( USHORT nItemId, short nScriptType );

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, USHORT nS, USHORT nE );

class ContentNode;
class EditDoc;

struct EPaM
{
    USHORT nPara;
    USHORT nIndex;

    EPaM()                              { nPara = 0; nIndex = 0; }
    EPaM( USHORT nP, USHORT nI )        { nPara = nP; nIndex = nI; }
    EPaM( const EPaM& r)                { nPara = r.nPara; nIndex = r.nIndex; }
    EPaM& operator = ( const EPaM& r )  { nPara = r.nPara; nIndex = r.nIndex; return *this; }
    inline BOOL operator == ( const EPaM& r ) const;
    inline BOOL operator < ( const EPaM& r ) const;
};

inline BOOL EPaM::operator < ( const EPaM& r ) const
{
    return ( ( nPara < r.nPara ) ||
             ( ( nPara == r.nPara ) && nIndex < r.nIndex ) ) ? TRUE : FALSE;
}

inline BOOL EPaM::operator == ( const EPaM& r ) const
{
    return ( ( nPara == r.nPara ) && ( nIndex == r.nIndex ) ) ? TRUE : FALSE;
}

struct ScriptTypePosInfo
{
    short   nScriptType;
    USHORT  nStartPos;
    USHORT  nEndPos;

    ScriptTypePosInfo( short _Type, USHORT _Start, USHORT _End )
    {
        nScriptType = _Type;
        nStartPos = _Start;
        nEndPos = _End;
    }
};

SV_DECL_VARARR( ScriptTypePosInfos, ScriptTypePosInfo, 0, 4 );

typedef EditCharAttrib* EditCharAttribPtr;
SV_DECL_PTRARR( CharAttribArray, EditCharAttribPtr, 0, 4 );

class ContentAttribsInfo
{
private:
    SfxItemSet          aPrevParaAttribs;
    CharAttribArray     aPrevCharAttribs;

public:
                        ContentAttribsInfo( const SfxItemSet& rParaAttribs );

    const SfxItemSet&       GetPrevParaAttribs() const  { return aPrevParaAttribs; }
    const CharAttribArray&  GetPrevCharAttribs() const  { return aPrevCharAttribs; }

    CharAttribArray&        GetPrevCharAttribs()        { return aPrevCharAttribs; }
};

typedef ContentAttribsInfo* ContentAttribsInfoPtr;
SV_DECL_PTRARR( ContentInfoArray, ContentAttribsInfoPtr, 1, 1 );

// ----------------------------------------------------------------------
//  class SvxFontTable
//  ----------------------------------------------------------------------
DECLARE_TABLE( DummyFontTable, SvxFontItem* );
class SvxFontTable : public DummyFontTable
{
public:
            SvxFontTable();
            ~SvxFontTable();

    ULONG   GetId( const SvxFontItem& rFont );
};

// ----------------------------------------------------------------------
//  class SvxColorList
//  ----------------------------------------------------------------------
typedef ContentNode* ContentNodePtr;
DECLARE_LIST( DummyColorList, SvxColorItem* );
class SvxColorList : public DummyColorList
{
public:
            SvxColorList();
            ~SvxColorList();

    ULONG   GetId( const SvxColorItem& rColor );
};

// ----------------------------------------------------------------------
//  class ItemList
//  ----------------------------------------------------------------------
typedef const SfxPoolItem* ConstPoolItemPtr;
DECLARE_LIST( DummyItemList, ConstPoolItemPtr );
class ItemList : public DummyItemList
{
public:
    const SfxPoolItem*  FindAttrib( USHORT nWhich );
};

// -------------------------------------------------------------------------
// class ContentAttribs
// -------------------------------------------------------------------------
class ContentAttribs
{
private:
    SfxStyleSheet*  pStyle;
    SfxItemSet      aAttribSet;

public:
                    ContentAttribs( SfxItemPool& rItemPool );
                    ContentAttribs( const ContentAttribs& );
                    ~ContentAttribs();  // erst bei umfangreicheren Tabs

    SvxTabStop      FindTabStop( long nCurPos, USHORT nDefTab );
    SfxItemSet&     GetItems()                          { return aAttribSet; }
    SfxStyleSheet*  GetStyleSheet() const               { return pStyle; }
    void            SetStyleSheet( SfxStyleSheet* pS );

    const SfxPoolItem&  GetItem( USHORT nWhich );
    BOOL                HasItem( USHORT nWhich );
};

// -------------------------------------------------------------------------
// class CharAttribList
// -------------------------------------------------------------------------
class CharAttribList
{
private:
    CharAttribArray aAttribs;
    SvxFont         aDefFont;               // schneller, als jedesmal vom Pool!
    BOOL            bHasEmptyAttribs;

                    CharAttribList( const CharAttribList& ) {;}

public:
                    CharAttribList();
                    ~CharAttribList();

    void            DeleteEmptyAttribs(  SfxItemPool& rItemPool );
    void            RemoveItemsFromPool( SfxItemPool* pItemPool );

    EditCharAttrib* FindAttrib( USHORT nWhich, USHORT nPos );
    EditCharAttrib* FindNextAttrib( USHORT nWhich, USHORT nFromPos ) const;
    EditCharAttrib* FindEmptyAttrib( USHORT nWhich, USHORT nPos );
    EditCharAttrib* FindFeature( USHORT nPos ) const;


    void            ResortAttribs();

    USHORT          Count()                 { return aAttribs.Count(); }
    void            Clear()                 { aAttribs.Remove( 0, aAttribs.Count()); }
    void            InsertAttrib( EditCharAttrib* pAttrib );

    SvxFont&        GetDefFont()            { return aDefFont; }

    BOOL            HasEmptyAttribs() const { return bHasEmptyAttribs; }
    BOOL&           HasEmptyAttribs()       { return bHasEmptyAttribs; }
    BOOL            HasBoundingAttrib( USHORT nBound );
    BOOL            HasAttrib( USHORT nWhich ) const;
    BOOL            HasAttrib( USHORT nStartPos, USHORT nEndPos ) const;

    CharAttribArray&        GetAttribs()        { return aAttribs; }
    const CharAttribArray&  GetAttribs() const  { return aAttribs; }

    // Debug:
    BOOL            DbgCheckAttribs();
};

// -------------------------------------------------------------------------
// class ContentNode
// -------------------------------------------------------------------------
class ContentNode : public XubString
{
private:
    ContentAttribs  aContentAttribs;
    CharAttribList  aCharAttribList;
    WrongList*      pWrongList;

public:
                    ContentNode( SfxItemPool& rItemPool );
                    ContentNode( const XubString& rStr, const ContentAttribs& rContentAttribs );
                    ~ContentNode();

    ContentAttribs& GetContentAttribs()     { return aContentAttribs; }
    CharAttribList& GetCharAttribs()        { return aCharAttribList; }

    void            ExpandAttribs( USHORT nIndex, USHORT nNewChars, SfxItemPool& rItemPool );
    void            CollapsAttribs( USHORT nIndex, USHORT nDelChars, SfxItemPool& rItemPool );
    void            AppendAttribs( ContentNode* pNextNode );
    void            CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, BOOL bKeepEndingAttribs );

    void            SetStyleSheet( SfxStyleSheet* pS, BOOL bRecalcFont = TRUE );
    void            SetStyleSheet( SfxStyleSheet* pS, const SvxFont& rFontFromStyle );
    SfxStyleSheet*  GetStyleSheet() { return aContentAttribs.GetStyleSheet(); }

    void            CreateDefFont();

    WrongList*      GetWrongList()                  { return pWrongList; }
    void            SetWrongList( WrongList* p )    { pWrongList = p; }

    void            CreateWrongList();
    void            DestroyWrongList();

    BOOL            IsFeature( USHORT nPos ) const { return ( GetChar( nPos ) == CH_FEATURE ); }
};

inline void ContentNode::DestroyWrongList()
{
#ifndef SVX_LIGHT
    delete pWrongList;
#endif
    pWrongList = NULL;
}

inline void ContentNode::CreateWrongList()
{
    DBG_ASSERT( !pWrongList, "WrongList existiert schon!" );
#ifndef SVX_LIGHT
    pWrongList = new WrongList;
#endif
}

typedef ContentNode* ContentNodePtr;
SV_DECL_PTRARR( ContentList, ContentNodePtr, 0, 4 );

// -------------------------------------------------------------------------
// class EditPaM
// -------------------------------------------------------------------------
class EditPaM
{
private:
    ContentNode*    pNode;
    USHORT          nIndex;

public:
                    EditPaM()                           { pNode = NULL; nIndex = 0; }
                    EditPaM( ContentNode* p, USHORT n ) { pNode = p; nIndex = n; }

    ContentNode*    GetNode() const                 { return pNode; }
    void            SetNode( ContentNode* p)        { pNode = p; }

    USHORT          GetIndex() const                { return nIndex; }
    USHORT&         GetIndex()                      { return nIndex; }
    void            SetIndex( USHORT n )            { nIndex = n; }

    BOOL            IsParaStart() const             { return nIndex == 0; }
    BOOL            IsParaEnd() const               { return nIndex == pNode->Len(); }

    BOOL            DbgIsBuggy( EditDoc& rDoc );

    EditPaM&    operator = ( const EditPaM& rPaM );
    friend BOOL operator == ( const EditPaM& r1,  const EditPaM& r2  );
    friend BOOL operator != ( const EditPaM& r1,  const EditPaM& r2  );
};

#define PORTIONKIND_TEXT        0
#define PORTIONKIND_TAB         1
#define PORTIONKIND_LINEBREAK   2
#define PORTIONKIND_FIELD       3
#define PORTIONKIND_HYPHENATOR  4
#define PORTIONKIND_EXTRASPACE  5

#define DELMODE_SIMPLE          0
#define DELMODE_RESTOFWORD      1
#define DELMODE_RESTOFCONTENT   2

// -------------------------------------------------------------------------
// class TextPortion
// -------------------------------------------------------------------------
class TextPortion
{
private:
    USHORT      nLen;
    Size        aOutSz;
    BYTE        nKind;
    sal_Unicode nExtraValue;

                TextPortion()               { nLen = 0; nKind = PORTIONKIND_TEXT; nExtraValue = 0;}

public:
                TextPortion( USHORT nL ) : aOutSz( -1, -1 )
                                            {   DBG_CTOR( EE_TextPortion, 0 );
                                                nLen = nL; nKind = PORTIONKIND_TEXT; nExtraValue = 0; }
                TextPortion( const TextPortion& r ) : aOutSz( r.aOutSz )
                                            { DBG_CTOR( EE_TextPortion, 0 );
                                                nLen = r.nLen; nKind = r.nKind; nExtraValue = r.nExtraValue; }
                ~TextPortion()              {   DBG_DTOR( EE_TextPortion, 0 ); }

    USHORT      GetLen() const              { return nLen; }
    USHORT&     GetLen()                    { return nLen; }
    void        SetLen( USHORT nL )         { nLen = nL; }

    Size&       GetSize()                   { return aOutSz; }
    Size        GetSize() const             { return aOutSz; }

    BYTE&       GetKind()                   { return nKind; }
    BYTE        GetKind() const             { return nKind; }

    sal_Unicode GetExtraValue() const       { return nExtraValue; }
    void        SetExtraValue( sal_Unicode n )  { nExtraValue = n; }

    BOOL        HasValidSize() const        { return aOutSz.Width() != (-1); }
};


// -------------------------------------------------------------------------
// class TextPortionList
// -------------------------------------------------------------------------
typedef TextPortion* TextPortionPtr;
SV_DECL_PTRARR( TextPortionArray, TextPortionPtr, 0, 8 );

class TextPortionList : public TextPortionArray
{
public:
            TextPortionList();
            ~TextPortionList();

    void    Reset();
    USHORT  FindPortion( USHORT nCharPos, USHORT& rPortionStart );
    void    DeleteFromPortion( USHORT nDelFrom );
};

class ParaPortion;

SV_DECL_VARARR( CharPosArray, long, 0, CHARPOSGROW );

// ------------------------------------------------------------------------
// class EditLine
// -------------------------------------------------------------------------
class EditLine
{
private:
    CharPosArray    aPositions;
    USHORT          nStartPosX;
    USHORT          nStart;     // koennte durch nStartPortion ersetzt werden
    USHORT          nEnd;       // koennte durch nEndPortion ersetzt werden
    USHORT          nStartPortion;
    USHORT          nEndPortion;
    USHORT          nHeight;    // Gesamthoehe der Zeile
    USHORT          nTxtHeight; // Reine Texthoehe
    USHORT          nCrsrHeight;    // Bei Konturfluss hohe Zeilen => Cursor zu groá.
    USHORT          nMaxAscent;
    BOOL            bInvalid;   // fuer geschickte Formatierung

public:
                    EditLine();
                    EditLine( const EditLine& );
                    ~EditLine();

    BOOL            IsIn( USHORT nIndex ) const
                        { return ( (nIndex >= nStart ) && ( nIndex < nEnd ) ); }

    BOOL            IsIn( USHORT nIndex, BOOL bInclEnd ) const
                        { return ( ( nIndex >= nStart ) && ( bInclEnd ? ( nIndex <= nEnd ) : ( nIndex < nEnd ) ) ); }

    void            SetStart( USHORT n )            { nStart = n; }
    USHORT          GetStart() const                { return nStart; }
    USHORT&         GetStart()                      { return nStart; }

    void            SetEnd( USHORT n )              { nEnd = n; }
    USHORT          GetEnd() const                  { return nEnd; }
    USHORT&         GetEnd()                        { return nEnd; }

    void            SetStartPortion( USHORT n )     { nStartPortion = n; }
    USHORT          GetStartPortion() const         { return nStartPortion; }
    USHORT&         GetStartPortion()               { return nStartPortion; }

    void            SetEndPortion( USHORT n )       { nEndPortion = n; }
    USHORT          GetEndPortion() const           { return nEndPortion; }
    USHORT&         GetEndPortion()                 { return nEndPortion; }

    void            SetHeight( USHORT nH, USHORT nTxtH = 0, USHORT nCrsrH = 0 )
                    {   nHeight = nH;
                        nTxtHeight = ( nTxtH ? nTxtH : nH );
                        nCrsrHeight = ( nCrsrH ? nCrsrH : nTxtHeight );
                    }
    USHORT          GetHeight() const                           { return nHeight; }
    USHORT          GetTxtHeight() const                        { return nTxtHeight; }
    USHORT          GetCrsrHeight() const                       { return nCrsrHeight; }

    void            SetMaxAscent( USHORT n )        { nMaxAscent = n; }
    USHORT          GetMaxAscent() const            { return nMaxAscent; }

    USHORT          GetLen() const                  { return nEnd - nStart; }

    USHORT          GetStartPosX() const            { return nStartPosX; }
    void            SetStartPosX( USHORT nStart )   { nStartPosX = nStart; }

    Size            CalcTextSize( ParaPortion& rParaPortion );

    BOOL            IsInvalid() const               { return bInvalid; }
    BOOL            IsValid() const                 { return !bInvalid; }
    void            SetInvalid()                    { bInvalid = TRUE; }
    void            SetValid()                      { bInvalid = FALSE; }

    BOOL            IsEmpty() const                 { return (nEnd > nStart) ? FALSE : TRUE; }

    CharPosArray&   GetCharPosArray()               { return aPositions; }

    EditLine*       Clone() const;

    EditLine&   operator = ( const EditLine& rLine );
    friend BOOL operator == ( const EditLine& r1,  const EditLine& r2  );
    friend BOOL operator != ( const EditLine& r1,  const EditLine& r2  );
};


// -------------------------------------------------------------------------
// class LineList
// -------------------------------------------------------------------------
typedef EditLine* EditLinePtr;
SV_DECL_PTRARR( LineArray, EditLinePtr, 0, 4 );

class EditLineList : public LineArray
{
public:
            EditLineList();
            ~EditLineList();

    void    Reset();
    void    DeleteFromLine( USHORT nDelFrom );
    USHORT  FindLine( USHORT nChar, BOOL bInclEnd );
};

// -------------------------------------------------------------------------
// class ParaPortion
// -------------------------------------------------------------------------
class ParaPortion
{
    friend class ImpEditEngine; // zum Einstellen der Hoehe
private:
    EditLineList        aLineList;
    TextPortionList     aTextPortionList;
    ContentNode*        pNode;
    long                nHeight;

    ScriptTypePosInfos  aScriptInfos;

    USHORT              nInvalidPosStart;
    USHORT              nFirstLineOffset;   // Fuer Writer-LineSpacing-Interpretation
    USHORT              nBulletX;
    short               nInvalidDiff;

    BOOL                bInvalid            : 1;
    BOOL                bSimple             : 1;    // nur lineares Tippen
    BOOL                bVisible            : 1;    // MT 05/00: Gehoert an den Node!!!
    BOOL                bForceRepaint       : 1;

                        ParaPortion( const ParaPortion& );

public:
                        ParaPortion( ContentNode* pNode );
                        ~ParaPortion();

    USHORT              GetLineNumber( USHORT nIndex );
    long                GetXPos( EditLine* pLine, USHORT nIndex );
    USHORT              GetChar( EditLine* pLine, long nX, BOOL bSmart = TRUE );

    EditLineList&       GetLines()                  { return aLineList; }

    BOOL                IsInvalid() const           { return bInvalid; }
    BOOL                IsSimpleInvalid()   const   { return bSimple; }
    void                SetValid()                  { bInvalid = FALSE; bSimple = TRUE;}

    BOOL                MustRepaint() const         { return bForceRepaint; }
    void                SetMustRepaint( BOOL bRP )  { bForceRepaint = bRP; }

    USHORT              GetBulletX() const          { return nBulletX; }
    void                SetBulletX( USHORT n )      { nBulletX = n; }

    void                MarkInvalid( USHORT nStart, short nDiff);
    void                MarkSelectionInvalid( USHORT nStart, USHORT nEnd );

    void                SetVisible( BOOL bVisible );
    BOOL                IsVisible()                 { return bVisible; }

    long                GetHeight() const           { return ( bVisible ? nHeight : 0 ); }
    USHORT              GetFirstLineOffset() const  { return ( bVisible ? nFirstLineOffset : 0 ); }
    void                ResetHeight()   { nHeight = 0; nFirstLineOffset = 0; }

    ContentNode*        GetNode() const             { return pNode; }
    TextPortionList&    GetTextPortions()           { return aTextPortionList; }
    void                AdjustBlocks( EditLine* pLine, long nRemainingSpace );

    USHORT              GetInvalidPosStart() const  { return nInvalidPosStart; }
    short               GetInvalidDiff() const      { return nInvalidDiff; }

    void                CorrectValuesBehindLastFormattedLine( USHORT nLastFormattedLine );

    BOOL                DbgCheckTextPortions();
};

typedef ParaPortion* ParaPortionPtr;
SV_DECL_PTRARR( DummyParaPortionList, ParaPortionPtr, 0, 4 );

// -------------------------------------------------------------------------
// class ParaPortionList
// -------------------------------------------------------------------------
class ParaPortionList : public DummyParaPortionList
{
public:
                    ParaPortionList();
                    ~ParaPortionList();

    void            Reset();
    long            GetYOffset( ParaPortion* pPPortion );
    USHORT          FindParagraph( long nYOffset );

    inline ParaPortion* SaveGetObject( USHORT nPos ) const
        { return ( nPos < Count() ) ? GetObject( nPos ) : 0; }

    // temporaer:
    void            DbgCheck( EditDoc& rDoc );
};

// -------------------------------------------------------------------------
// class EditSelection
// -------------------------------------------------------------------------
class EditSelection
{
private:
    EditPaM         aStartPaM;
    EditPaM         aEndPaM;

public:
                    EditSelection();    // kein CCTOR und DTOR, geht autom. richtig!
                    EditSelection( const EditPaM& rStartAndAnd );
                    EditSelection( const EditPaM& rStart, const EditPaM& rEnd );

    EditPaM&        Min()               { return aStartPaM; }
    EditPaM&        Max()               { return aEndPaM; }

    const EditPaM&  Min() const         { return aStartPaM; }
    const EditPaM&  Max() const         { return aEndPaM; }

    BOOL            HasRange() const    { return aStartPaM != aEndPaM; }
    BOOL            IsInvalid();
    BOOL            DbgIsBuggy( EditDoc& rDoc );

    BOOL            Adjust( const ContentList& rNodes );

    EditSelection&  operator = ( const EditPaM& r );
    BOOL            operator == ( const EditSelection& r ) const
                    { return ( ( aStartPaM == r.aStartPaM ) && ( aEndPaM == r.aEndPaM ) )
                            ? TRUE : FALSE; }
};

// -------------------------------------------------------------------------
// class DeletedNodeInfo
// -------------------------------------------------------------------------
class DeletedNodeInfo
{
private:
    ULONG   nInvalidAdressPtr;
    USHORT  nInvalidParagraph;

public:
            DeletedNodeInfo( ULONG nInvAdr, USHORT nPos )
                                            {   nInvalidAdressPtr = nInvAdr;
                                                nInvalidParagraph = nPos; }

    ULONG   GetInvalidAdress()              {   return nInvalidAdressPtr; }
    USHORT  GetPosition()                   {   return nInvalidParagraph; }
};

typedef DeletedNodeInfo* DeletedNodeInfoPtr;
SV_DECL_PTRARR( DeletedNodesList, DeletedNodeInfoPtr, 0, 4 );

// -------------------------------------------------------------------------
// class EditDoc
// -------------------------------------------------------------------------
class EditDoc : public ContentList
{
private:
    SfxItemPool*    pItemPool;

    SvxFont         aDefFont;           //schneller, als jedesmal vom Pool!
    USHORT          nDefTab;
    BOOL            bIsVertical;

    BOOL            bOwnerOfPool;
    BOOL            bModified;

protected:
    void            ImplDestroyContents();

public:
                    EditDoc( SfxItemPool* pItemPool );
                    ~EditDoc();

    BOOL            IsModified() const      { return bModified; }
    void            SetModified( BOOL b )   { bModified = b; }

    void            CreateDefFont( BOOL bUseStyles );
    const SvxFont&  GetDefFont() { return aDefFont; }

    void            SetDefTab( USHORT nTab )    { nDefTab = nTab ? nTab : DEFTAB; }
    USHORT          GetDefTab() const           { return nDefTab; }

    void            SetVertical( BOOL bVertical )   { bIsVertical = bVertical; }
    BOOL            IsVertical() const              { return bIsVertical; }

    EditPaM         Clear();
    EditPaM         RemoveText();
    EditPaM         RemoveChars( EditPaM aPaM, USHORT nChars );
    void            InsertText( const EditPaM& rPaM, xub_Unicode c );
    EditPaM         InsertText( EditPaM aPaM, const XubString& rStr );
    EditPaM         InsertParaBreak( EditPaM aPaM, BOOL bKeepEndingAttribs );
    EditPaM         InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem );
    EditPaM         ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight );

    String          GetText( LineEnd eEnd ) const;
    ULONG           GetTextLen() const;

    XubString       GetParaAsString( USHORT nNode ) const;
    XubString       GetParaAsString( ContentNode* pNode, USHORT nStartPos = 0, USHORT nEndPos = 0xFFFF, BOOL bResolveFields = TRUE ) const;

    inline EditPaM  GetStartPaM() const;
    inline EditPaM  GetEndPaM() const;

    SfxItemPool&        GetItemPool()                   { return *pItemPool; }
    const SfxItemPool&  GetItemPool() const             { return *pItemPool; }

    void            RemoveItemsFromPool( ContentNode* pNode );

    void            InsertAttrib( const SfxPoolItem& rItem, ContentNode* pNode, USHORT nStart, USHORT nEnd );
    void            InsertAttrib( ContentNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem );
    void            InsertAttribInSelection( ContentNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem );
    BOOL            RemoveAttribs( ContentNode* pNode, USHORT nStart, USHORT nEnd, USHORT nWhich = 0 );
    BOOL            RemoveAttribs( ContentNode* pNode, USHORT nStart, USHORT nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, USHORT nWhich = 0 );
    void            FindAttribs( ContentNode* pNode, USHORT nStartPos, USHORT nEndPos, SfxItemSet& rCurSet );

    USHORT          GetPos( ContentNode* pNode ) const { return ContentList::GetPos(pNode); }
    ContentNode*    SaveGetObject( USHORT nPos ) const { return ( nPos < Count() ) ? GetObject( nPos ) : 0; }

    static XubString    GetSepStr( LineEnd eEnd );
};

inline EditPaM EditDoc::GetStartPaM() const
{
    return EditPaM( GetObject( 0 ), 0 );
}

inline EditPaM EditDoc::GetEndPaM() const
{
    ContentNode* pLastNode = GetObject( Count()-1 );
    return EditPaM( pLastNode, pLastNode->Len() );
}

inline EditCharAttrib* GetAttrib( const CharAttribArray& rAttribs, USHORT nAttr )
{
    return ( nAttr < rAttribs.Count() ) ? rAttribs[nAttr] : 0;
}

BOOL CheckOrderedList( CharAttribArray& rAttribs, BOOL bStart );

// -------------------------------------------------------------------------
// class EditEngineItemPool
// -------------------------------------------------------------------------
class EditEngineItemPool : public SfxItemPool
{
public:
                        EditEngineItemPool( BOOL bPersistenRefCounts );
                        ~EditEngineItemPool();

    virtual SvStream&   Store( SvStream& rStream ) const;
};

#endif // _EDITDOC_HXX

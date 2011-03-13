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

#ifndef _EDITDOC_HXX
#define _EDITDOC_HXX

#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>

#include <editattr.hxx>
#include <edtspell.hxx>
#include <editeng/svxfont.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <tools/table.hxx>
#include <vector>

#include <deque>

class ImpEditEngine;
class SvxTabStop;
class SvtCTLOptions;

DBG_NAMEEX( EE_TextPortion )

#define CHARPOSGROW     16
#define DEFTAB          720

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent = true, short nScriptType = 0 );
sal_uInt16 GetScriptItemId( sal_uInt16 nItemId, short nScriptType );
sal_Bool IsScriptItemValid( sal_uInt16 nItemId, short nScriptType );

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_uInt16 nS, sal_uInt16 nE );

class ContentNode;
class EditDoc;

struct EPaM
{
    sal_uInt16 nPara;
    sal_uInt16 nIndex;

    EPaM()                              { nPara = 0; nIndex = 0; }
    EPaM( sal_uInt16 nP, sal_uInt16 nI )        { nPara = nP; nIndex = nI; }
    EPaM( const EPaM& r)                { nPara = r.nPara; nIndex = r.nIndex; }
    EPaM& operator = ( const EPaM& r )  { nPara = r.nPara; nIndex = r.nIndex; return *this; }
    inline sal_Bool operator == ( const EPaM& r ) const;
    inline sal_Bool operator < ( const EPaM& r ) const;
};

inline sal_Bool EPaM::operator < ( const EPaM& r ) const
{
    return ( ( nPara < r.nPara ) ||
             ( ( nPara == r.nPara ) && nIndex < r.nIndex ) ) ? sal_True : sal_False;
}

inline sal_Bool EPaM::operator == ( const EPaM& r ) const
{
    return ( ( nPara == r.nPara ) && ( nIndex == r.nIndex ) ) ? sal_True : sal_False;
}

struct ScriptTypePosInfo
{
    short   nScriptType;
    sal_uInt16  nStartPos;
    sal_uInt16  nEndPos;

    ScriptTypePosInfo( short _Type, sal_uInt16 _Start, sal_uInt16 _End )
    {
        nScriptType = _Type;
        nStartPos = _Start;
        nEndPos = _End;
    }
};

typedef std::deque< ScriptTypePosInfo > ScriptTypePosInfos;

struct WritingDirectionInfo
{
    sal_uInt8   nType;
    sal_uInt16  nStartPos;
    sal_uInt16  nEndPos;

    WritingDirectionInfo( sal_uInt8 _Type, sal_uInt16 _Start, sal_uInt16 _End )
    {
        nType = _Type;
        nStartPos = _Start;
        nEndPos = _End;
    }
};


typedef std::deque< WritingDirectionInfo > WritingDirectionInfos;

typedef EditCharAttrib* EditCharAttribPtr;
SV_DECL_PTRARR( CharAttribArray, EditCharAttribPtr, 0, 4 )

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
SV_DECL_PTRARR( ContentInfoArray, ContentAttribsInfoPtr, 1, 1 )

//  ----------------------------------------------------------------------
//  class SvxFontTable
//  ----------------------------------------------------------------------
DECLARE_TABLE( DummyFontTable, SvxFontItem* )

class SvxFontTable : public DummyFontTable
{
public:
            SvxFontTable();
            ~SvxFontTable();

    sal_uLong   GetId( const SvxFontItem& rFont );
};

//  ----------------------------------------------------------------------
//  class SvxColorList
//  ----------------------------------------------------------------------
typedef ContentNode* ContentNodePtr;
typedef ::std::vector< SvxColorItem* > DummyColorList;

class SvxColorList
{
private:
    DummyColorList aColorList;

public:
            SvxColorList();
            ~SvxColorList();

    size_t  GetId( const SvxColorItem& rColor );
    size_t  Count() { return aColorList.size(); };
    void    Insert( SvxColorItem* pItem, size_t nIndex );
    SvxColorItem* GetObject( size_t nIndex );
};

//  ----------------------------------------------------------------------
//  class ItemList
//  ----------------------------------------------------------------------
typedef const SfxPoolItem* ConstPoolItemPtr;
typedef ::std::vector< ConstPoolItemPtr > DummyItemList;

class ItemList
{
private:
    DummyItemList aItemPool;
    size_t  CurrentItem;

public:
    ItemList();
    const SfxPoolItem*  FindAttrib( sal_uInt16 nWhich );
    const SfxPoolItem*  First();
    const SfxPoolItem*  Next();
    size_t              Count() { return aItemPool.size(); };
    void                Insert( const SfxPoolItem* pItem );
    void                Clear() { aItemPool.clear(); };
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
                    ~ContentAttribs();  // only for larger Tabs

    SvxTabStop      FindTabStop( long nCurPos, sal_uInt16 nDefTab );
    SfxItemSet&     GetItems()                          { return aAttribSet; }
    SfxStyleSheet*  GetStyleSheet() const               { return pStyle; }
    void            SetStyleSheet( SfxStyleSheet* pS );

    const SfxPoolItem&  GetItem( sal_uInt16 nWhich );
    sal_Bool                HasItem( sal_uInt16 nWhich );
};

// -------------------------------------------------------------------------
// class CharAttribList
// -------------------------------------------------------------------------
class CharAttribList
{
private:
    CharAttribArray aAttribs;
    SvxFont         aDefFont;          // faster than ever from the pool!
    sal_Bool        bHasEmptyAttribs;

                    CharAttribList( const CharAttribList& ) {;}

public:
                    CharAttribList();
                    ~CharAttribList();

    void            DeleteEmptyAttribs(  SfxItemPool& rItemPool );
    void            RemoveItemsFromPool( SfxItemPool* pItemPool );

    EditCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    EditCharAttrib* FindNextAttrib( sal_uInt16 nWhich, sal_uInt16 nFromPos ) const;
    EditCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    EditCharAttrib* FindFeature( sal_uInt16 nPos ) const;


    void            ResortAttribs();
    void            OptimizeRanges( SfxItemPool& rItemPool );

    sal_uInt16          Count()                 { return aAttribs.Count(); }
    void            Clear()                 { aAttribs.Remove( 0, aAttribs.Count()); }
    void            InsertAttrib( EditCharAttrib* pAttrib );

    SvxFont&        GetDefFont()            { return aDefFont; }

    sal_Bool            HasEmptyAttribs() const { return bHasEmptyAttribs; }
    sal_Bool&           HasEmptyAttribs()       { return bHasEmptyAttribs; }
    sal_Bool            HasBoundingAttrib( sal_uInt16 nBound );
    sal_Bool            HasAttrib( sal_uInt16 nWhich ) const;
    sal_Bool            HasAttrib( sal_uInt16 nStartPos, sal_uInt16 nEndPos ) const;

    CharAttribArray&        GetAttribs()        { return aAttribs; }
    const CharAttribArray&  GetAttribs() const  { return aAttribs; }

    // Debug:
    sal_Bool            DbgCheckAttribs();
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

    void            ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNewChars, SfxItemPool& rItemPool );
    void            CollapsAttribs( sal_uInt16 nIndex, sal_uInt16 nDelChars, SfxItemPool& rItemPool );
    void            AppendAttribs( ContentNode* pNextNode );
    void            CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, sal_Bool bKeepEndingAttribs );

    void            SetStyleSheet( SfxStyleSheet* pS, sal_Bool bRecalcFont = sal_True );
    void            SetStyleSheet( SfxStyleSheet* pS, const SvxFont& rFontFromStyle );
    SfxStyleSheet*  GetStyleSheet() { return aContentAttribs.GetStyleSheet(); }

    void            CreateDefFont();

    WrongList*      GetWrongList()          { return pWrongList; }
    void            SetWrongList( WrongList* p );

    void            CreateWrongList();
    void            DestroyWrongList();

    sal_Bool            IsFeature( sal_uInt16 nPos ) const { return ( GetChar( nPos ) == CH_FEATURE ); }
};

typedef ContentNode* ContentNodePtr;
SV_DECL_PTRARR( DummyContentList, ContentNodePtr, 0, 4 )

class ContentList : public DummyContentList
{
  sal_uInt16 nLastCache;
public:
  ContentList() : DummyContentList( 0, 4 ), nLastCache(0) {}
  sal_uInt16 GetPos( const ContentNodePtr &rPtr ) const;
};

// -------------------------------------------------------------------------
// class EditPaM
// -------------------------------------------------------------------------
class EditPaM
{
private:
    ContentNode*    pNode;
    sal_uInt16          nIndex;

public:
                    EditPaM()                           { pNode = NULL; nIndex = 0; }
                    EditPaM( ContentNode* p, sal_uInt16 n ) { pNode = p; nIndex = n; }

    ContentNode*    GetNode() const                 { return pNode; }
    void            SetNode( ContentNode* p)        { pNode = p; }

    sal_uInt16          GetIndex() const                { return nIndex; }
    sal_uInt16&         GetIndex()                      { return nIndex; }
    void            SetIndex( sal_uInt16 n )            { nIndex = n; }

    sal_Bool            IsParaStart() const             { return nIndex == 0; }
    sal_Bool            IsParaEnd() const               { return nIndex == pNode->Len(); }

    sal_Bool            DbgIsBuggy( EditDoc& rDoc );

    EditPaM&    operator = ( const EditPaM& rPaM );
    friend sal_Bool operator == ( const EditPaM& r1,  const EditPaM& r2  );
    friend sal_Bool operator != ( const EditPaM& r1,  const EditPaM& r2  );
};

#define PORTIONKIND_TEXT        0
#define PORTIONKIND_TAB         1
#define PORTIONKIND_LINEBREAK   2
#define PORTIONKIND_FIELD       3
#define PORTIONKIND_HYPHENATOR  4
// #define PORTIONKIND_EXTRASPACE   5

#define DELMODE_SIMPLE          0
#define DELMODE_RESTOFWORD      1
#define DELMODE_RESTOFCONTENT   2

#define CHAR_NORMAL            0x00
#define CHAR_KANA              0x01
#define CHAR_PUNCTUATIONLEFT   0x02
#define CHAR_PUNCTUATIONRIGHT  0x04

// -------------------------------------------------------------------------
// struct ExtraPortionInfos
// -------------------------------------------------------------------------
struct ExtraPortionInfo
{
    long    nOrgWidth;
    long    nWidthFullCompression;

    long    nPortionOffsetX;

    sal_uInt16  nMaxCompression100thPercent;

    sal_uInt8    nAsianCompressionTypes;
    sal_Bool    bFirstCharIsRightPunktuation;
    sal_Bool    bCompressed;

    sal_Int32*    pOrgDXArray;


            ExtraPortionInfo();
            ~ExtraPortionInfo();

    void    SaveOrgDXArray( const sal_Int32* pDXArray, sal_uInt16 nLen );
    void    DestroyOrgDXArray();
};


// -------------------------------------------------------------------------
// class TextPortion
// -------------------------------------------------------------------------
class TextPortion
{
private:
    ExtraPortionInfo*   pExtraInfos;
    sal_uInt16              nLen;
    Size                aOutSz;
    sal_uInt8               nKind;
    sal_uInt8                nRightToLeft;
    sal_Unicode         nExtraValue;


                TextPortion()               { DBG_CTOR( EE_TextPortion, 0 );
                                              pExtraInfos = NULL; nLen = 0; nKind = PORTIONKIND_TEXT; nExtraValue = 0; nRightToLeft = sal_False;}

public:
                TextPortion( sal_uInt16 nL ) : aOutSz( -1, -1 )
                                            {   DBG_CTOR( EE_TextPortion, 0 );
                                                pExtraInfos = NULL; nLen = nL; nKind = PORTIONKIND_TEXT; nExtraValue = 0; nRightToLeft = sal_False;}
                TextPortion( const TextPortion& r ) : aOutSz( r.aOutSz )
                                            { DBG_CTOR( EE_TextPortion, 0 );
                                                pExtraInfos = NULL; nLen = r.nLen; nKind = r.nKind; nExtraValue = r.nExtraValue; nRightToLeft = r.nRightToLeft; }

                ~TextPortion()              {   DBG_DTOR( EE_TextPortion, 0 ); delete pExtraInfos; }

    sal_uInt16      GetLen() const              { return nLen; }
    sal_uInt16&     GetLen()                    { return nLen; }
    void        SetLen( sal_uInt16 nL )         { nLen = nL; }

    Size&       GetSize()                   { return aOutSz; }
    Size        GetSize() const             { return aOutSz; }

    sal_uInt8&      GetKind()                   { return nKind; }
    sal_uInt8       GetKind() const             { return nKind; }

    void        SetRightToLeft( sal_uInt8 b )    { nRightToLeft = b; }
    sal_uInt8        GetRightToLeft() const      { return nRightToLeft; }
    sal_Bool        IsRightToLeft() const       { return (nRightToLeft&1); }

    sal_Unicode GetExtraValue() const       { return nExtraValue; }
    void        SetExtraValue( sal_Unicode n )  { nExtraValue = n; }

    sal_Bool        HasValidSize() const        { return aOutSz.Width() != (-1); }

    ExtraPortionInfo*   GetExtraInfos() const { return pExtraInfos; }
    void                SetExtraInfos( ExtraPortionInfo* p ) { delete pExtraInfos; pExtraInfos = p; }
};

// -------------------------------------------------------------------------
// class TextPortionList
// -------------------------------------------------------------------------
typedef TextPortion* TextPortionPtr;
SV_DECL_PTRARR( TextPortionArray, TextPortionPtr, 0, 8 )

class TextPortionList : public TextPortionArray
{
public:
            TextPortionList();
            ~TextPortionList();

    void    Reset();
    sal_uInt16  FindPortion( sal_uInt16 nCharPos, sal_uInt16& rPortionStart, sal_Bool bPreferStartingPortion = sal_False );
    sal_uInt16  GetStartPos( sal_uInt16 nPortion );
    void    DeleteFromPortion( sal_uInt16 nDelFrom );
};

class ParaPortion;

SV_DECL_VARARR( CharPosArray, sal_Int32, 0, CHARPOSGROW )

// ------------------------------------------------------------------------
// class EditLine
// -------------------------------------------------------------------------
class EditLine
{
private:
    CharPosArray    aPositions;
    long            nTxtWidth;
    sal_uInt16          nStartPosX;
    sal_uInt16          nStart;     // could be replaced by nStartPortion
    sal_uInt16          nEnd;       // could be replaced by nEndPortion
    sal_uInt16          nStartPortion;
    sal_uInt16          nEndPortion;
    sal_uInt16          nHeight;    //  Total height of the line
    sal_uInt16          nTxtHeight; // Pure Text height
    sal_uInt16          nCrsrHeight;    // For contour flow high lines => cursor is large.
    sal_uInt16          nMaxAscent;
    sal_Bool            bHangingPunctuation;
    sal_Bool            bInvalid;   // for skillful formatting

public:
                    EditLine();
                    EditLine( const EditLine& );
                    ~EditLine();

    sal_Bool            IsIn( sal_uInt16 nIndex ) const
                        { return ( (nIndex >= nStart ) && ( nIndex < nEnd ) ); }

    sal_Bool            IsIn( sal_uInt16 nIndex, sal_Bool bInclEnd ) const
                        { return ( ( nIndex >= nStart ) && ( bInclEnd ? ( nIndex <= nEnd ) : ( nIndex < nEnd ) ) ); }

    void            SetStart( sal_uInt16 n )            { nStart = n; }
    sal_uInt16          GetStart() const                { return nStart; }
    sal_uInt16&         GetStart()                      { return nStart; }

    void            SetEnd( sal_uInt16 n )              { nEnd = n; }
    sal_uInt16          GetEnd() const                  { return nEnd; }
    sal_uInt16&         GetEnd()                        { return nEnd; }

    void            SetStartPortion( sal_uInt16 n )     { nStartPortion = n; }
    sal_uInt16          GetStartPortion() const         { return nStartPortion; }
    sal_uInt16&         GetStartPortion()               { return nStartPortion; }

    void            SetEndPortion( sal_uInt16 n )       { nEndPortion = n; }
    sal_uInt16          GetEndPortion() const           { return nEndPortion; }
    sal_uInt16&         GetEndPortion()                 { return nEndPortion; }

    void            SetHeight( sal_uInt16 nH, sal_uInt16 nTxtH = 0, sal_uInt16 nCrsrH = 0 )
                    {   nHeight = nH;
                        nTxtHeight = ( nTxtH ? nTxtH : nH );
                        nCrsrHeight = ( nCrsrH ? nCrsrH : nTxtHeight );
                    }
    sal_uInt16          GetHeight() const               { return nHeight; }
    sal_uInt16          GetTxtHeight() const            { return nTxtHeight; }
    sal_uInt16          GetCrsrHeight() const           { return nCrsrHeight; }

    void            SetTextWidth( long n )          { nTxtWidth = n; }
    long            GetTextWidth() const            { return nTxtWidth; }

    void            SetMaxAscent( sal_uInt16 n )        { nMaxAscent = n; }
    sal_uInt16          GetMaxAscent() const            { return nMaxAscent; }

    void            SetHangingPunctuation( sal_Bool b )     { bHangingPunctuation = b; }
    sal_Bool            IsHangingPunctuation() const        { return bHangingPunctuation; }

    sal_uInt16          GetLen() const                  { return nEnd - nStart; }

    sal_uInt16          GetStartPosX() const            { return nStartPosX; }
    void            SetStartPosX( sal_uInt16 start )    { nStartPosX = start; }

    Size            CalcTextSize( ParaPortion& rParaPortion );

    sal_Bool            IsInvalid() const               { return bInvalid; }
    sal_Bool            IsValid() const                 { return !bInvalid; }
    void            SetInvalid()                    { bInvalid = sal_True; }
    void            SetValid()                      { bInvalid = sal_False; }

    sal_Bool            IsEmpty() const                 { return (nEnd > nStart) ? sal_False : sal_True; }

    CharPosArray&   GetCharPosArray()               { return aPositions; }

    EditLine*       Clone() const;

    EditLine&   operator = ( const EditLine& rLine );
    friend sal_Bool operator == ( const EditLine& r1,  const EditLine& r2  );
    friend sal_Bool operator != ( const EditLine& r1,  const EditLine& r2  );
};


// -------------------------------------------------------------------------
// class LineList
// -------------------------------------------------------------------------
typedef EditLine* EditLinePtr;
SV_DECL_PTRARR( LineArray, EditLinePtr, 0, 4 )

class EditLineList : public LineArray
{
public:
            EditLineList();
            ~EditLineList();

    void    Reset();
    void    DeleteFromLine( sal_uInt16 nDelFrom );
    sal_uInt16  FindLine( sal_uInt16 nChar, sal_Bool bInclEnd );
};

// -------------------------------------------------------------------------
// class ParaPortion
// -------------------------------------------------------------------------
class ParaPortion
{
    friend class ImpEditEngine; // to adjust the height
private:
    EditLineList        aLineList;
    TextPortionList     aTextPortionList;
    ContentNode*        pNode;
    long                nHeight;

    ScriptTypePosInfos      aScriptInfos;
    WritingDirectionInfos   aWritingDirectionInfos;

    sal_uInt16              nInvalidPosStart;
    sal_uInt16              nFirstLineOffset;   // For Writer-LineSpacing-Interpretation
    sal_uInt16              nBulletX;
    short                   nInvalidDiff;

    sal_Bool                bInvalid            : 1;
    sal_Bool                bSimple             : 1;    // only linear Tap
    sal_Bool                bVisible            : 1;    // Belongs to the node!
    sal_Bool                bForceRepaint       : 1;

                        ParaPortion( const ParaPortion& );

public:
                        ParaPortion( ContentNode* pNode );
                        ~ParaPortion();

    sal_uInt16              GetLineNumber( sal_uInt16 nIndex );

    EditLineList&       GetLines()                  { return aLineList; }

    sal_Bool                IsInvalid() const           { return bInvalid; }
    sal_Bool                IsSimpleInvalid()   const   { return bSimple; }
    void                SetValid()                  { bInvalid = sal_False; bSimple = sal_True;}

    sal_Bool                MustRepaint() const         { return bForceRepaint; }
    void                SetMustRepaint( sal_Bool bRP )  { bForceRepaint = bRP; }

    sal_uInt16              GetBulletX() const          { return nBulletX; }
    void                SetBulletX( sal_uInt16 n )      { nBulletX = n; }

    void                MarkInvalid( sal_uInt16 nStart, short nDiff);
    void                MarkSelectionInvalid( sal_uInt16 nStart, sal_uInt16 nEnd );

    void                SetVisible( sal_Bool bVisible );
    sal_Bool                IsVisible()                 { return bVisible; }

    long                GetHeight() const           { return ( bVisible ? nHeight : 0 ); }
    sal_uInt16              GetFirstLineOffset() const  { return ( bVisible ? nFirstLineOffset : 0 ); }
    void                ResetHeight()   { nHeight = 0; nFirstLineOffset = 0; }

    ContentNode*        GetNode() const             { return pNode; }
    TextPortionList&    GetTextPortions()           { return aTextPortionList; }

    sal_uInt16              GetInvalidPosStart() const  { return nInvalidPosStart; }
    short               GetInvalidDiff() const      { return nInvalidDiff; }

    void                CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine );

    sal_Bool                DbgCheckTextPortions();
};

typedef ParaPortion* ParaPortionPtr;
SV_DECL_PTRARR( DummyParaPortionList, ParaPortionPtr, 0, 4 )

// -------------------------------------------------------------------------
// class ParaPortionList
// -------------------------------------------------------------------------
class ParaPortionList : public DummyParaPortionList
{
    sal_uInt16 nLastCache;
public:
                    ParaPortionList();
                    ~ParaPortionList();

    void            Reset();
    long            GetYOffset( ParaPortion* pPPortion );
    sal_uInt16          FindParagraph( long nYOffset );

    inline ParaPortion* SaveGetObject( sal_uInt16 nPos ) const
        { return ( nPos < Count() ) ? GetObject( nPos ) : 0; }

    sal_uInt16                  GetPos( const ParaPortionPtr &rPtr ) const;

    // temporary:
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
                    EditSelection();    // No constructor and destructor
                                        // are automtically excecuted correctly!
                    EditSelection( const EditPaM& rStartAndAnd );
                    EditSelection( const EditPaM& rStart, const EditPaM& rEnd );

    EditPaM&        Min()               { return aStartPaM; }
    EditPaM&        Max()               { return aEndPaM; }

    const EditPaM&  Min() const         { return aStartPaM; }
    const EditPaM&  Max() const         { return aEndPaM; }

    sal_Bool            HasRange() const    { return aStartPaM != aEndPaM; }
    sal_Bool            IsInvalid() const;
    sal_Bool            DbgIsBuggy( EditDoc& rDoc );

    sal_Bool            Adjust( const ContentList& rNodes );

    EditSelection&  operator = ( const EditPaM& r );
    sal_Bool            operator == ( const EditSelection& r ) const
                    { return ( ( aStartPaM == r.aStartPaM ) && ( aEndPaM == r.aEndPaM ) )
                            ? sal_True : sal_False; }
    sal_Bool            operator != ( const EditSelection& r ) const { return !( r == *this ); }
};

// -------------------------------------------------------------------------
// class DeletedNodeInfo
// -------------------------------------------------------------------------
class DeletedNodeInfo
{
private:
    sal_uIntPtr     nInvalidAdressPtr;
    sal_uInt16  nInvalidParagraph;

public:
            DeletedNodeInfo( sal_uIntPtr nInvAdr, sal_uInt16 nPos )
                                            {   nInvalidAdressPtr = nInvAdr;
                                                nInvalidParagraph = nPos; }

    sal_uIntPtr GetInvalidAdress()              {   return nInvalidAdressPtr; }
    sal_uInt16  GetPosition()                   {   return nInvalidParagraph; }
};

typedef DeletedNodeInfo* DeletedNodeInfoPtr;
SV_DECL_PTRARR( DeletedNodesList, DeletedNodeInfoPtr, 0, 4 )

// -------------------------------------------------------------------------
// class EditDoc
// -------------------------------------------------------------------------
class EditDoc : public ContentList
{
private:
    SfxItemPool*    pItemPool;
    Link            aModifyHdl;

    SvxFont         aDefFont;           //faster than ever from the pool!!
    sal_uInt16          nDefTab;
    sal_Bool            bIsVertical;
    sal_Bool            bIsFixedCellHeight;

    sal_Bool            bOwnerOfPool;
    sal_Bool            bModified;

protected:
    void            ImplDestroyContents();

public:
                    EditDoc( SfxItemPool* pItemPool );
                    ~EditDoc();

    sal_Bool            IsModified() const      { return bModified; }
    void            SetModified( sal_Bool b );

    void            SetModifyHdl( const Link& rLink ) { aModifyHdl = rLink; }
    Link            GetModifyHdl() const { return aModifyHdl; }

    void            CreateDefFont( sal_Bool bUseStyles );
    const SvxFont&  GetDefFont() { return aDefFont; }

    void            SetDefTab( sal_uInt16 nTab )    { nDefTab = nTab ? nTab : DEFTAB; }
    sal_uInt16          GetDefTab() const           { return nDefTab; }

    void            SetVertical( sal_Bool bVertical )   { bIsVertical = bVertical; }
    sal_Bool            IsVertical() const              { return bIsVertical; }

    void            SetFixedCellHeight( sal_Bool bUseFixedCellHeight )  { bIsFixedCellHeight = bUseFixedCellHeight; }
    sal_Bool            IsFixedCellHeight() const               { return bIsFixedCellHeight; }

    EditPaM         Clear();
    EditPaM         RemoveText();
    EditPaM         RemoveChars( EditPaM aPaM, sal_uInt16 nChars );
    void            InsertText( const EditPaM& rPaM, xub_Unicode c );
    EditPaM         InsertText( EditPaM aPaM, const XubString& rStr );
    EditPaM         InsertParaBreak( EditPaM aPaM, sal_Bool bKeepEndingAttribs );
    EditPaM         InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem );
    EditPaM         ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight );

    String          GetText( LineEnd eEnd ) const;
    sal_uLong           GetTextLen() const;

    XubString       GetParaAsString( sal_uInt16 nNode ) const;
    XubString       GetParaAsString( ContentNode* pNode, sal_uInt16 nStartPos = 0, sal_uInt16 nEndPos = 0xFFFF, sal_Bool bResolveFields = sal_True ) const;

    inline EditPaM  GetStartPaM() const;
    inline EditPaM  GetEndPaM() const;

    SfxItemPool&        GetItemPool()                   { return *pItemPool; }
    const SfxItemPool&  GetItemPool() const             { return *pItemPool; }

    void            RemoveItemsFromPool( ContentNode* pNode );

    void            InsertAttrib( const SfxPoolItem& rItem, ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd );
    void            InsertAttrib( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem );
    void            InsertAttribInSelection( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem );
    sal_Bool            RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt16 nWhich = 0 );
    sal_Bool            RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, sal_uInt16 nWhich = 0 );
    void            FindAttribs( ContentNode* pNode, sal_uInt16 nStartPos, sal_uInt16 nEndPos, SfxItemSet& rCurSet );

    sal_uInt16          GetPos( ContentNode* pNode ) const { return ContentList::GetPos(pNode); }
    ContentNode*    SaveGetObject( sal_uInt16 nPos ) const { return ( nPos < Count() ) ? GetObject( nPos ) : 0; }

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

inline EditCharAttrib* GetAttrib( const CharAttribArray& rAttribs, sal_uInt16 nAttr )
{
    return ( nAttr < rAttribs.Count() ) ? rAttribs[nAttr] : 0;
}

sal_Bool CheckOrderedList( CharAttribArray& rAttribs, sal_Bool bStart );

// -------------------------------------------------------------------------
// class EditEngineItemPool
// -------------------------------------------------------------------------
class EditEngineItemPool : public SfxItemPool
{
public:
                        EditEngineItemPool( sal_Bool bPersistenRefCounts );
protected:
                        virtual ~EditEngineItemPool();
public:

    virtual SvStream&   Store( SvStream& rStream ) const;
};

#endif // _EDITDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

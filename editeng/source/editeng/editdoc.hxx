/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _EDITDOC_HXX
#define _EDITDOC_HXX

#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>

#include "editattr.hxx"
#include "edtspell.hxx"
#include <editeng/svxfont.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>

#include <vector>
#include <deque>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class ImpEditEngine;
class SvxTabStop;

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
    sal_Int32  nPara;
    sal_uInt16 nIndex;

    EPaM()                              { nPara = 0; nIndex = 0; }
    EPaM( sal_Int32 nP, sal_uInt16 nI ) { nPara = nP; nIndex = nI; }
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

class ContentAttribsInfo
{
private:
    typedef boost::ptr_vector<EditCharAttrib> CharAttribsType;

    SfxItemSet          aPrevParaAttribs;
    CharAttribsType     aPrevCharAttribs;

public:
                        ContentAttribsInfo( const SfxItemSet& rParaAttribs );

    const SfxItemSet&       GetPrevParaAttribs() const  { return aPrevParaAttribs; }
    const CharAttribsType&  GetPrevCharAttribs() const  { return aPrevCharAttribs; }

    void RemoveAllCharAttribsFromPool(SfxItemPool& rPool) const;
    void AppendCharAttrib(EditCharAttrib* pNew);
};

//  ----------------------------------------------------------------------
//  class SvxColorList
//  ----------------------------------------------------------------------

class SvxColorList
{
private:
    typedef std::vector<SvxColorItem*> DummyColorList;
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

class ItemList
{
private:
    typedef std::vector<const SfxPoolItem*> DummyItemList;
    DummyItemList aItemPool;
    size_t  CurrentItem;

public:
    ItemList();
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
    const SfxItemSet& GetItems() const { return aAttribSet; }
    const SfxStyleSheet*  GetStyleSheet() const { return pStyle; }
    SfxStyleSheet*  GetStyleSheet() { return pStyle; }
    void            SetStyleSheet( SfxStyleSheet* pS );

    const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const;
    bool HasItem( sal_uInt16 nWhich ) const;
};

// -------------------------------------------------------------------------
// class CharAttribList
// -------------------------------------------------------------------------
class CharAttribList
{
public:
    typedef boost::ptr_vector<EditCharAttrib> AttribsType;

private:
    AttribsType     aAttribs;
    SvxFont         aDefFont;          // faster than ever from the pool!
    bool            bHasEmptyAttribs;

                    CharAttribList( const CharAttribList& ) {;}

public:
                    CharAttribList();
                    ~CharAttribList();

    void            DeleteEmptyAttribs(  SfxItemPool& rItemPool );
    void            RemoveItemsFromPool( SfxItemPool* pItemPool );

    const EditCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos ) const;
    EditCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    const EditCharAttrib* FindNextAttrib( sal_uInt16 nWhich, sal_uInt16 nFromPos ) const;
    const EditCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos ) const;
    EditCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    const EditCharAttrib* FindFeature( sal_uInt16 nPos ) const;


    void            ResortAttribs();
    void            OptimizeRanges( SfxItemPool& rItemPool );

    size_t Count() const;

    void            InsertAttrib( EditCharAttrib* pAttrib );

    SvxFont&        GetDefFont()            { return aDefFont; }

    bool            HasEmptyAttribs() const { return bHasEmptyAttribs; }
    void SetHasEmptyAttribs(bool b);
    bool HasBoundingAttrib( sal_uInt16 nBound ) const;
    bool HasAttrib( sal_uInt16 nStartPos, sal_uInt16 nEndPos ) const;

    AttribsType& GetAttribs();
    const AttribsType& GetAttribs() const;

    void Remove(const EditCharAttrib* p);
    void Remove(size_t nPos);
    void Release(const EditCharAttrib* p);

#if OSL_DEBUG_LEVEL > 2
    // Debug:
    bool DbgCheckAttribs() const;
#endif
};

// -------------------------------------------------------------------------
// class ContentNode
// -------------------------------------------------------------------------
class ContentNode : boost::noncopyable
{
private:
    XubString maString;
    ContentAttribs  aContentAttribs;
    CharAttribList  aCharAttribList;
    boost::scoped_ptr<WrongList> mpWrongList;

public:
                    ContentNode( SfxItemPool& rItemPool );
                    ContentNode( const XubString& rStr, const ContentAttribs& rContentAttribs );
                    ~ContentNode();

    ContentAttribs& GetContentAttribs()     { return aContentAttribs; }
    const ContentAttribs& GetContentAttribs() const { return aContentAttribs; }
    CharAttribList& GetCharAttribs()        { return aCharAttribList; }
    const CharAttribList& GetCharAttribs() const { return aCharAttribList; }

    void            ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNewChars, SfxItemPool& rItemPool );
    void            CollapsAttribs( sal_uInt16 nIndex, sal_uInt16 nDelChars, SfxItemPool& rItemPool );
    void            AppendAttribs( ContentNode* pNextNode );
    void            CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, sal_Bool bKeepEndingAttribs );

    void            SetStyleSheet( SfxStyleSheet* pS, sal_Bool bRecalcFont = sal_True );
    void            SetStyleSheet( SfxStyleSheet* pS, const SvxFont& rFontFromStyle );
    SfxStyleSheet*  GetStyleSheet() { return aContentAttribs.GetStyleSheet(); }
    const SfxStyleSheet* GetStyleSheet() const { return aContentAttribs.GetStyleSheet(); }

    void            CreateDefFont();

    void EnsureWrongList();
    WrongList* GetWrongList();
    const WrongList* GetWrongList() const;
    void SetWrongList( WrongList* p );

    void            CreateWrongList();
    void            DestroyWrongList();

    bool IsFeature( sal_uInt16 nPos ) const;

    sal_uInt16 Len() const;
    const XubString& GetString() const;

    void SetChar(sal_uInt16 nPos, sal_Unicode c);
    void Insert(const XubString& rStr, sal_uInt16 nPos);
    void Append(const XubString& rStr);
    void Erase(sal_uInt16 nPos);
    void Erase(sal_uInt16 nPos, sal_uInt16 nCount);
    XubString Copy(sal_uInt16 nPos) const;
    XubString Copy(sal_uInt16 nPos, sal_uInt16 nCount) const;
    sal_Unicode GetChar(sal_uInt16 nPos) const;
};

// -------------------------------------------------------------------------
// class EditPaM
// -------------------------------------------------------------------------
class EditPaM
{
private:
    ContentNode* pNode;
    sal_uInt16          nIndex;

public:
    EditPaM();
    EditPaM(const EditPaM& r);
    EditPaM(ContentNode* p, sal_uInt16 n);

    const ContentNode* GetNode() const;
    ContentNode* GetNode();
    void SetNode(ContentNode* p);

    sal_uInt16          GetIndex() const                { return nIndex; }
    sal_uInt16&         GetIndex()                      { return nIndex; }
    void            SetIndex( sal_uInt16 n )            { nIndex = n; }

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
    ::std::vector< sal_Int32 > lineBreaksList;


            ExtraPortionInfo();
            ~ExtraPortionInfo();

    void    SaveOrgDXArray( const sal_Int32* pDXArray, sal_uInt16 nLen );
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
    const Size& GetSize() const             { return aOutSz; }

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
class TextPortionList
{
    typedef boost::ptr_vector<TextPortion> PortionsType;
    PortionsType maPortions;

public:
            TextPortionList();
            ~TextPortionList();

    void    Reset();
    size_t FindPortion(
        sal_uInt16 nCharPos, sal_uInt16& rPortionStart, bool bPreferStartingPortion = false) const;
    sal_uInt16 GetStartPos(size_t nPortion);
    void DeleteFromPortion(size_t nDelFrom);
    size_t Count() const;
    const TextPortion* operator[](size_t nPos) const;
    TextPortion* operator[](size_t nPos);

    void Append(TextPortion* p);
    void Insert(size_t nPos, TextPortion* p);
    void Remove(size_t nPos);
    size_t GetPos(const TextPortion* p) const;
};

class ParaPortion;

// ------------------------------------------------------------------------
// class EditLine
// -------------------------------------------------------------------------
class EditLine
{
public:
    typedef std::vector<sal_Int32> CharPosArrayType;

private:
    CharPosArrayType aPositions;
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
    bool            bHangingPunctuation:1;
    bool            bInvalid:1;   // for skillful formatting

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

    void            SetHeight( sal_uInt16 nH, sal_uInt16 nTxtH = 0, sal_uInt16 nCrsrH = 0 );
    sal_uInt16          GetHeight() const               { return nHeight; }
    sal_uInt16          GetTxtHeight() const            { return nTxtHeight; }
    sal_uInt16          GetCrsrHeight() const           { return nCrsrHeight; }

    void            SetTextWidth( long n )          { nTxtWidth = n; }
    long            GetTextWidth() const            { return nTxtWidth; }

    void            SetMaxAscent( sal_uInt16 n )        { nMaxAscent = n; }
    sal_uInt16          GetMaxAscent() const            { return nMaxAscent; }

    void            SetHangingPunctuation( bool b )     { bHangingPunctuation = b; }
    bool            IsHangingPunctuation() const        { return bHangingPunctuation; }

    sal_uInt16          GetLen() const                  { return nEnd - nStart; }

    sal_uInt16          GetStartPosX() const            { return nStartPosX; }
    void            SetStartPosX( long start );
    Size            CalcTextSize( ParaPortion& rParaPortion );

    bool            IsInvalid() const               { return bInvalid; }
    bool            IsValid() const                 { return !bInvalid; }
    void            SetInvalid()                    { bInvalid = true; }
    void            SetValid()                      { bInvalid = false; }

    sal_Bool            IsEmpty() const                 { return (nEnd > nStart) ? sal_False : sal_True; }

    CharPosArrayType& GetCharPosArray();
    const CharPosArrayType& GetCharPosArray() const;

    EditLine*       Clone() const;

    EditLine&   operator = ( const EditLine& rLine );
    friend sal_Bool operator == ( const EditLine& r1,  const EditLine& r2  );
    friend sal_Bool operator != ( const EditLine& r1,  const EditLine& r2  );
};


// -------------------------------------------------------------------------
// class LineList
// -------------------------------------------------------------------------
class EditLineList
{
    typedef boost::ptr_vector<EditLine> LinesType;
    LinesType maLines;

public:
            EditLineList();
            ~EditLineList();

    void Reset();
    void DeleteFromLine(size_t nDelFrom);
    size_t FindLine(sal_uInt16 nChar, bool bInclEnd);
    size_t Count() const;
    const EditLine* operator[](size_t nPos) const;
    EditLine* operator[](size_t nPos);

    void Append(EditLine* p);
    void Insert(size_t nPos, EditLine* p);
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

    sal_uInt16 GetLineNumber( sal_uInt16 nIndex ) const;

    EditLineList&       GetLines()                  { return aLineList; }
    const EditLineList& GetLines() const { return aLineList; }

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
    bool                IsVisible() const { return bVisible; }

    sal_Bool            IsEmpty() { return GetTextPortions().Count() == 1 && GetTextPortions()[0]->GetLen() == 0; }

    long                GetHeight() const           { return ( bVisible ? nHeight : 0 ); }
    sal_uInt16              GetFirstLineOffset() const  { return ( bVisible ? nFirstLineOffset : 0 ); }
    void                ResetHeight()   { nHeight = 0; nFirstLineOffset = 0; }

    ContentNode*        GetNode() const             { return pNode; }
    TextPortionList&    GetTextPortions()           { return aTextPortionList; }
    const TextPortionList& GetTextPortions() const { return aTextPortionList; }

    sal_uInt16              GetInvalidPosStart() const  { return nInvalidPosStart; }
    short               GetInvalidDiff() const      { return nInvalidDiff; }

    void                CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine );
#if OSL_DEBUG_LEVEL > 2
    sal_Bool                DbgCheckTextPortions();
#endif
};

// -------------------------------------------------------------------------
// class ParaPortionList
// -------------------------------------------------------------------------
class ParaPortionList
{
    mutable size_t nLastCache;
    boost::ptr_vector<ParaPortion> maPortions;
public:
                    ParaPortionList();
                    ~ParaPortionList();

    void            Reset();
    long GetYOffset(const ParaPortion* pPPortion) const;
    sal_Int32 FindParagraph(long nYOffset) const;

    const ParaPortion* SafeGetObject(sal_Int32 nPos) const;
    ParaPortion* SafeGetObject(sal_Int32 nPos);

    sal_Int32 GetPos(const ParaPortion* p) const;
    ParaPortion* operator[](sal_Int32 nPos);
    const ParaPortion* operator[](sal_Int32 nPos) const;

    ParaPortion* Release(sal_Int32 nPos);
    void Remove(sal_Int32 nPos);
    void Insert(sal_Int32 nPos, ParaPortion* p);
    void Append(ParaPortion* p);
    sal_Int32 Count() const;

#if OSL_DEBUG_LEVEL > 2
    // temporary:
    void            DbgCheck( EditDoc& rDoc );
#endif
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

    sal_Bool            Adjust( const EditDoc& rNodes );

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
    sal_Int32       nInvalidParagraph;

public:
            DeletedNodeInfo( sal_uIntPtr nInvAdr, sal_Int32 nPos )
                                            {   nInvalidAdressPtr = nInvAdr;
                                                nInvalidParagraph = nPos; }

    sal_uIntPtr GetInvalidAdress() const { return nInvalidAdressPtr; }
    sal_Int32   GetPosition() const { return nInvalidParagraph; }
};

// -------------------------------------------------------------------------
// class EditDoc
// -------------------------------------------------------------------------
class EditDoc
{
private:
    mutable size_t nLastCache;
    boost::ptr_vector<ContentNode> maContents;

    SfxItemPool*    pItemPool;
    Link            aModifyHdl;

    SvxFont         aDefFont;           //faster than ever from the pool!!
    sal_uInt16          nDefTab;
    bool            bIsVertical:1;
    bool            bIsFixedCellHeight:1;

    bool            bOwnerOfPool:1;
    bool            bModified:1;

private:
    void            ImplDestroyContents();

public:
                    EditDoc( SfxItemPool* pItemPool );
                    ~EditDoc();

    void ClearSpellErrors();

    bool            IsModified() const      { return bModified; }
    void            SetModified( bool b );

    void            SetModifyHdl( const Link& rLink ) { aModifyHdl = rLink; }
    Link            GetModifyHdl() const { return aModifyHdl; }

    void            CreateDefFont( sal_Bool bUseStyles );
    const SvxFont&  GetDefFont() { return aDefFont; }

    void            SetDefTab( sal_uInt16 nTab )    { nDefTab = nTab ? nTab : DEFTAB; }
    sal_uInt16          GetDefTab() const           { return nDefTab; }

    void            SetVertical( bool bVertical )   { bIsVertical = bVertical; }
    bool            IsVertical() const              { return bIsVertical; }

    void            SetFixedCellHeight( bool bUseFixedCellHeight )  { bIsFixedCellHeight = bUseFixedCellHeight; }
    bool            IsFixedCellHeight() const               { return bIsFixedCellHeight; }

    EditPaM         Clear();
    EditPaM         RemoveText();
    EditPaM         RemoveChars( EditPaM aPaM, sal_uInt16 nChars );
    EditPaM         InsertText( EditPaM aPaM, const XubString& rStr );
    EditPaM         InsertParaBreak( EditPaM aPaM, sal_Bool bKeepEndingAttribs );
    EditPaM         InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem );
    EditPaM         ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight );

    OUString        GetText( LineEnd eEnd ) const;
    sal_uLong       GetTextLen() const;

    XubString       GetParaAsString( sal_Int32 nNode ) const;
    XubString       GetParaAsString(const ContentNode* pNode, sal_uInt16 nStartPos = 0, sal_uInt16 nEndPos = 0xFFFF, bool bResolveFields = true) const;

    EditPaM GetStartPaM() const;
    EditPaM GetEndPaM() const;

    SfxItemPool&        GetItemPool()                   { return *pItemPool; }
    const SfxItemPool&  GetItemPool() const             { return *pItemPool; }

    void RemoveItemsFromPool(const ContentNode& rNode);

    void            InsertAttrib( const SfxPoolItem& rItem, ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd );
    void            InsertAttrib( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem );
    void            InsertAttribInSelection( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem );
    sal_Bool            RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt16 nWhich = 0 );
    sal_Bool            RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, sal_uInt16 nWhich = 0 );
    void            FindAttribs( ContentNode* pNode, sal_uInt16 nStartPos, sal_uInt16 nEndPos, SfxItemSet& rCurSet );

    sal_Int32 GetPos(const ContentNode* pNode) const;
    const ContentNode* GetObject(sal_Int32 nPos) const;
    ContentNode* GetObject(sal_Int32 nPos);
    sal_Int32 Count() const;
    const ContentNode* operator[](sal_Int32 nPos) const;
    ContentNode* operator[](sal_Int32 nPos);
    void Insert(sal_Int32 nPos, ContentNode* p);
    /// deletes
    void Remove(sal_Int32 nPos);
    /// does not delete
    void Release(sal_Int32 nPos);

    static OUString GetSepStr( LineEnd eEnd );
};

inline EditCharAttrib* GetAttrib(CharAttribList::AttribsType& rAttribs, size_t nAttr)
{
    return (nAttr < rAttribs.size()) ? &rAttribs[nAttr] : NULL;
}

bool CheckOrderedList(const CharAttribList::AttribsType& rAttribs, bool bStart);

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

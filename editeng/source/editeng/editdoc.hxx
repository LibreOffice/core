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

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EDITDOC_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EDITDOC_HXX

#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>

#include "editattr.hxx"
#include "edtspell.hxx"
#include <editeng/svxfont.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <svl/languageoptions.hxx>
#include <tools/lineend.hxx>

#include <deque>
#include <memory>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

class ImpEditEngine;
class SvxTabStop;


#define CHARPOSGROW     16
#define DEFTAB          720

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent = true, SvtScriptType nScriptType = SvtScriptType::NONE );
sal_uInt16 GetScriptItemId( sal_uInt16 nItemId, SvtScriptType nScriptType );
bool IsScriptItemValid( sal_uInt16 nItemId, short nScriptType );

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE );

class ContentNode;
class EditDoc;

struct EPaM
{
    sal_Int32  nPara;
    sal_Int32  nIndex;

    EPaM() : nPara(0), nIndex(0) {}
    EPaM( sal_Int32 nP, sal_Int32 nI ) : nPara(nP), nIndex(nI) {}
    EPaM( const EPaM& r) : nPara(r.nPara), nIndex(r.nIndex) {}
    EPaM& operator = ( const EPaM& r )  { nPara = r.nPara; nIndex = r.nIndex; return *this; }
    inline bool operator == ( const EPaM& r ) const;
    inline bool operator < ( const EPaM& r ) const;
};

inline bool EPaM::operator < ( const EPaM& r ) const
{
    return ( nPara < r.nPara ) || ( ( nPara == r.nPara ) && nIndex < r.nIndex );
}

inline bool EPaM::operator == ( const EPaM& r ) const
{
    return ( nPara == r.nPara ) && ( nIndex == r.nIndex );
}

struct ScriptTypePosInfo
{
    short   nScriptType;
    sal_Int32  nStartPos;
    sal_Int32  nEndPos;

    ScriptTypePosInfo( short _Type, sal_Int32 _Start, sal_Int32 _End )
    : nScriptType(_Type)
    , nStartPos(_Start)
    , nEndPos(_End)
    {
    }
};

typedef std::deque< ScriptTypePosInfo > ScriptTypePosInfos;

struct WritingDirectionInfo
{
    sal_uInt8   nType;
    sal_Int32  nStartPos;
    sal_Int32  nEndPos;

    WritingDirectionInfo( sal_uInt8 _Type, sal_Int32 _Start, sal_Int32 _End )
    : nType(_Type)
    , nStartPos(_Start)
    , nEndPos(_End)
    {
    }
};


typedef std::deque< WritingDirectionInfo > WritingDirectionInfos;

class ContentAttribsInfo
{
private:
    typedef std::vector<std::unique_ptr<EditCharAttrib> > CharAttribsType;

    SfxItemSet          aPrevParaAttribs;
    CharAttribsType     aPrevCharAttribs;

public:
                        ContentAttribsInfo( const SfxItemSet& rParaAttribs );

    const SfxItemSet&       GetPrevParaAttribs() const  { return aPrevParaAttribs; }
    const CharAttribsType&  GetPrevCharAttribs() const  { return aPrevCharAttribs; }

    void RemoveAllCharAttribsFromPool(SfxItemPool& rPool) const;
    void AppendCharAttrib(EditCharAttrib* pNew);
};


//  class SvxColorList

typedef std::vector<Color> SvxColorList;


//  class ItemList


class ItemList
{
private:
    typedef std::vector<const SfxPoolItem*> DummyItemList;
    DummyItemList aItemPool;
    sal_Int32  CurrentItem;

public:
    ItemList();
    const SfxPoolItem*  First();
    const SfxPoolItem*  Next();
    sal_Int32              Count() { return aItemPool.size(); };
    void                Insert( const SfxPoolItem* pItem );
    void                Clear() { aItemPool.clear(); };
};


// class ContentAttribs

class ContentAttribs
{
private:
    SfxStyleSheet*  pStyle;
    SfxItemSet      aAttribSet;

public:
                    ContentAttribs( SfxItemPool& rItemPool );
                    ContentAttribs( const ContentAttribs& );
                    ~ContentAttribs();  // only for larger Tabs

    SvxTabStop      FindTabStop( sal_Int32 nCurPos, sal_uInt16 nDefTab );
    SfxItemSet&     GetItems()                          { return aAttribSet; }
    const SfxItemSet& GetItems() const { return aAttribSet; }
    const SfxStyleSheet*  GetStyleSheet() const { return pStyle; }
    SfxStyleSheet*  GetStyleSheet() { return pStyle; }
    void            SetStyleSheet( SfxStyleSheet* pS );

    const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const;
    bool HasItem( sal_uInt16 nWhich ) const;
};


// class CharAttribList

class CharAttribList
{
public:
    typedef std::vector<std::unique_ptr<EditCharAttrib> > AttribsType;

private:
    AttribsType     aAttribs;
    SvxFont         aDefFont;          // faster than ever from the pool!
    bool            bHasEmptyAttribs;

                    CharAttribList( const CharAttribList& ) {;}

public:
                    CharAttribList();
                    ~CharAttribList();

    void            DeleteEmptyAttribs(  SfxItemPool& rItemPool );

    const EditCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos ) const;
    EditCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos );
    const EditCharAttrib* FindNextAttrib( sal_uInt16 nWhich, sal_Int32 nFromPos ) const;
    const EditCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_Int32 nPos ) const;
    EditCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_Int32 nPos );
    const EditCharAttrib* FindFeature( sal_Int32 nPos ) const;


    void            ResortAttribs();
    void            OptimizeRanges( SfxItemPool& rItemPool );

    sal_Int32 Count() const;

    void            InsertAttrib( EditCharAttrib* pAttrib );

    SvxFont&        GetDefFont()            { return aDefFont; }

    bool            HasEmptyAttribs() const { return bHasEmptyAttribs; }
    void SetHasEmptyAttribs(bool b);
    bool HasBoundingAttrib( sal_Int32 nBound ) const;
    bool HasAttrib( sal_Int32 nStartPos, sal_Int32 nEndPos ) const;

    AttribsType& GetAttribs() { return aAttribs;}
    const AttribsType& GetAttribs() const { return aAttribs;}

    void Remove(const EditCharAttrib* p);
    void Remove(sal_Int32 nPos);
    void Release(const EditCharAttrib* p);

#if OSL_DEBUG_LEVEL > 0
    static void DbgCheckAttribs(CharAttribList const& rAttribs);
#endif
};


// class ContentNode

class ContentNode : boost::noncopyable
{
private:
    OUString maString;
    ContentAttribs  aContentAttribs;
    CharAttribList  aCharAttribList;
    std::unique_ptr<WrongList> mpWrongList;

    void UnExpandPosition( sal_Int32 &rStartPos, bool bBiasStart );

public:
                    ContentNode( SfxItemPool& rItemPool );
                    ContentNode( const OUString& rStr, const ContentAttribs& rContentAttribs );
                    ~ContentNode();

    ContentAttribs& GetContentAttribs()     { return aContentAttribs; }
    const ContentAttribs& GetContentAttribs() const { return aContentAttribs; }
    CharAttribList& GetCharAttribs()        { return aCharAttribList; }
    const CharAttribList& GetCharAttribs() const { return aCharAttribList; }

    void            ExpandAttribs( sal_Int32 nIndex, sal_Int32 nNewChars, SfxItemPool& rItemPool );
    void            CollapsAttribs( sal_Int32 nIndex, sal_Int32 nDelChars, SfxItemPool& rItemPool );
    void            AppendAttribs( ContentNode* pNextNode );
    void            CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, bool bKeepEndingAttribs );

    void            SetStyleSheet( SfxStyleSheet* pS, bool bRecalcFont = true );
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

    bool IsFeature( sal_Int32 nPos ) const;

    sal_Int32 Len() const;
    const OUString& GetString() const { return maString;}

    /// return length including expanded fields
    sal_uLong GetExpandedLen() const;
    /// return content including expanded fields
    OUString  GetExpandedText(sal_Int32 nStartPos = 0, sal_Int32 nEndPos = -1, bool bResolveFields = true) const;
    /// re-write offsets in the expanded text to string offsets
    void      UnExpandPositions( sal_Int32 &rStartPos, sal_Int32 &rEndPos );

    void SetChar(sal_Int32 nPos, sal_Unicode c);
    void Insert(const OUString& rStr, sal_Int32 nPos);
    void Append(const OUString& rStr);
    void Erase(sal_Int32 nPos);
    void Erase(sal_Int32 nPos, sal_Int32 nCount);
    OUString Copy(sal_Int32 nPos) const;
    OUString Copy(sal_Int32 nPos, sal_Int32 nCount) const;
    sal_Unicode GetChar(sal_Int32 nPos) const;
};


// class EditPaM

class EditPaM
{
private:
    ContentNode* pNode;
    sal_Int32    nIndex;

public:
    EditPaM();
    EditPaM(const EditPaM& r);
    EditPaM(ContentNode* p, sal_Int32 n);

    const ContentNode* GetNode() const { return pNode;}
    ContentNode* GetNode() { return pNode;}
    void SetNode(ContentNode* p);

    sal_Int32  GetIndex() const         { return nIndex; }
    void       SetIndex( sal_Int32 n )  { nIndex = n; }

    bool       DbgIsBuggy( EditDoc& rDoc );

    EditPaM&    operator = ( const EditPaM& rPaM );
    friend bool operator == ( const EditPaM& r1, const EditPaM& r2 );
    friend bool operator != ( const EditPaM& r1, const EditPaM& r2 );
    bool operator !() const { return !pNode && !nIndex; }
};

enum class PortionKind
{
    TEXT        = 0,
    TAB         = 1,
    LINEBREAK   = 2,
    FIELD       = 3,
    HYPHENATOR  = 4
};

#define DELMODE_SIMPLE          0
#define DELMODE_RESTOFWORD      1
#define DELMODE_RESTOFCONTENT   2

#define CHAR_NORMAL            0x00
#define CHAR_KANA              0x01
#define CHAR_PUNCTUATIONLEFT   0x02
#define CHAR_PUNCTUATIONRIGHT  0x04


// struct ExtraPortionInfos

struct ExtraPortionInfo
{
    long    nOrgWidth;
    long    nWidthFullCompression;

    long    nPortionOffsetX;

    sal_uInt16  nMaxCompression100thPercent;

    sal_uInt8    nAsianCompressionTypes;
    bool    bFirstCharIsRightPunktuation;
    bool    bCompressed;

    long*    pOrgDXArray;
    ::std::vector< sal_Int32 > lineBreaksList;


            ExtraPortionInfo();
            ~ExtraPortionInfo();

    void    SaveOrgDXArray( const long* pDXArray, sal_Int32 nLen );
};



// class TextPortion

class TextPortion
{
private:
    ExtraPortionInfo*   pExtraInfos;
    sal_Int32           nLen;
    Size                aOutSz;
    PortionKind         nKind;
    sal_uInt8           nRightToLeft;
    sal_Unicode         nExtraValue;


                TextPortion()
                : pExtraInfos( nullptr )
                , nLen( 0 )
                , aOutSz()
                , nKind( PortionKind::TEXT )
                , nRightToLeft( sal_False )
                , nExtraValue( 0 )
                {
                }

public:
                TextPortion( sal_Int32 nL )
                : pExtraInfos( nullptr )
                , nLen( nL )
                , aOutSz( -1, -1 )
                , nKind( PortionKind::TEXT )
                , nRightToLeft( sal_False )
                , nExtraValue( 0 )
                {
                }

                TextPortion( const TextPortion& r )
                : pExtraInfos( nullptr )
                , nLen( r.nLen )
                , aOutSz( r.aOutSz )
                , nKind( r.nKind )
                , nRightToLeft( r.nRightToLeft )
                , nExtraValue( r.nExtraValue )
                {
                }


    sal_Int32      GetLen() const              { return nLen; }
    void           SetLen( sal_Int32 nL )         { nLen = nL; }

    Size&          GetSize()                   { return aOutSz; }
    const Size&    GetSize() const             { return aOutSz; }

    PortionKind&   GetKind()                   { return nKind; }
    PortionKind    GetKind() const             { return nKind; }

    void           SetRightToLeft( sal_uInt8 b )    { nRightToLeft = b; }
    sal_uInt8      GetRightToLeft() const      { return nRightToLeft; }
    bool           IsRightToLeft() const       { return (nRightToLeft&1); }

    sal_Unicode    GetExtraValue() const       { return nExtraValue; }
    void           SetExtraValue( sal_Unicode n )  { nExtraValue = n; }

    bool           HasValidSize() const        { return aOutSz.Width() != (-1); }

    ExtraPortionInfo*   GetExtraInfos() const { return pExtraInfos; }
    void                SetExtraInfos( ExtraPortionInfo* p ) { delete pExtraInfos; pExtraInfos = p; }
};


// class TextPortionList

class TextPortionList
{
    typedef std::vector<std::unique_ptr<TextPortion> > PortionsType;
    PortionsType maPortions;

public:
            TextPortionList();
            ~TextPortionList();

    void    Reset();
    sal_Int32 FindPortion(
        sal_Int32 nCharPos, sal_Int32& rPortionStart, bool bPreferStartingPortion = false) const;
    sal_Int32 GetStartPos(sal_Int32 nPortion);
    void DeleteFromPortion(sal_Int32 nDelFrom);
    sal_Int32 Count() const;
    const TextPortion& operator[](sal_Int32 nPos) const;
    TextPortion& operator[](sal_Int32 nPos);

    void Append(TextPortion* p);
    void Insert(sal_Int32 nPos, TextPortion* p);
    void Remove(sal_Int32 nPos);
    sal_Int32 GetPos(const TextPortion* p) const;
};

class ParaPortion;


// class EditLine

class EditLine
{
public:
    typedef std::vector<long> CharPosArrayType;

private:
    CharPosArrayType aPositions;
    long            nTxtWidth;
    sal_uInt16          nStartPosX;
    sal_Int32          nStart;     // could be replaced by nStartPortion
    sal_Int32          nEnd;       // could be replaced by nEndPortion
    sal_Int32          nStartPortion;
    sal_Int32          nEndPortion;
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

    bool            IsIn( sal_Int32 nIndex ) const
                        { return ( (nIndex >= nStart ) && ( nIndex < nEnd ) ); }

    bool            IsIn( sal_Int32 nIndex, bool bInclEnd ) const
                        { return ( ( nIndex >= nStart ) && ( bInclEnd ? ( nIndex <= nEnd ) : ( nIndex < nEnd ) ) ); }

    void            SetStart( sal_Int32 n )            { nStart = n; }
    sal_Int32       GetStart() const                { return nStart; }
    sal_Int32&      GetStart()                      { return nStart; }

    void            SetEnd( sal_Int32 n )              { nEnd = n; }
    sal_Int32       GetEnd() const                  { return nEnd; }
    sal_Int32&      GetEnd()                        { return nEnd; }

    void            SetStartPortion( sal_Int32 n )     { nStartPortion = n; }
    sal_Int32       GetStartPortion() const         { return nStartPortion; }
    sal_Int32&      GetStartPortion()               { return nStartPortion; }

    void            SetEndPortion( sal_Int32 n )       { nEndPortion = n; }
    sal_Int32       GetEndPortion() const           { return nEndPortion; }
    sal_Int32&      GetEndPortion()                 { return nEndPortion; }

    void            SetHeight( sal_uInt16 nH, sal_uInt16 nTxtH = 0, sal_uInt16 nCrsrH = 0 );
    sal_uInt16      GetHeight() const               { return nHeight; }
    sal_uInt16      GetTxtHeight() const            { return nTxtHeight; }

    void            SetTextWidth( long n )          { nTxtWidth = n; }
    long            GetTextWidth() const            { return nTxtWidth; }

    void            SetMaxAscent( sal_uInt16 n )        { nMaxAscent = n; }
    sal_uInt16      GetMaxAscent() const            { return nMaxAscent; }

    void            SetHangingPunctuation( bool b )     { bHangingPunctuation = b; }
    bool            IsHangingPunctuation() const        { return bHangingPunctuation; }

    sal_Int32       GetLen() const                  { return nEnd - nStart; }

    sal_uInt16      GetStartPosX() const            { return nStartPosX; }
    void            SetStartPosX( long start );
    Size            CalcTextSize( ParaPortion& rParaPortion );

    bool            IsInvalid() const               { return bInvalid; }
    bool            IsValid() const                 { return !bInvalid; }
    void            SetInvalid()                    { bInvalid = true; }
    void            SetValid()                      { bInvalid = false; }

    bool            IsEmpty() const                 { return nEnd <= nStart; }

    CharPosArrayType& GetCharPosArray() { return aPositions;}
    const CharPosArrayType& GetCharPosArray() const { return aPositions;}

    EditLine*       Clone() const;

    EditLine&   operator = ( const EditLine& rLine );
    friend bool operator == ( const EditLine& r1,  const EditLine& r2  );
};



// class LineList

class EditLineList
{
    typedef std::vector<std::unique_ptr<EditLine> > LinesType;
    LinesType maLines;

public:
            EditLineList();
            ~EditLineList();

    void Reset();
    void DeleteFromLine(sal_Int32 nDelFrom);
    sal_Int32 FindLine(sal_Int32 nChar, bool bInclEnd);
    sal_Int32 Count() const;
    const EditLine& operator[](sal_Int32 nPos) const;
    EditLine& operator[](sal_Int32 nPos);

    void Append(EditLine* p);
    void Insert(sal_Int32 nPos, EditLine* p);
};


// class ParaPortion

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

    sal_Int32              nInvalidPosStart;
    sal_Int32              nFirstLineOffset;   // For Writer-LineSpacing-Interpretation
    sal_uInt16             nBulletX;
    sal_Int32              nInvalidDiff;

    bool                bInvalid            : 1;
    bool                bSimple             : 1;    // only linear Tap
    bool                bVisible            : 1;    // Belongs to the node!
    bool                bForceRepaint       : 1;

                        ParaPortion( const ParaPortion& ) = delete;

public:
                        ParaPortion( ContentNode* pNode );
                        ~ParaPortion();

    sal_Int32 GetLineNumber( sal_Int32 nIndex ) const;

    EditLineList&       GetLines()                  { return aLineList; }
    const EditLineList& GetLines() const { return aLineList; }

    bool                IsInvalid() const           { return bInvalid; }
    bool                IsSimpleInvalid()   const   { return bSimple; }
    void                SetValid()                  { bInvalid = false; bSimple = true;}

    bool                MustRepaint() const         { return bForceRepaint; }
    void                SetMustRepaint( bool bRP )  { bForceRepaint = bRP; }

    sal_uInt16          GetBulletX() const          { return nBulletX; }
    void                SetBulletX( sal_uInt16 n )      { nBulletX = n; }

    void                MarkInvalid( sal_Int32 nStart, sal_Int32 nDiff);
    void                MarkSelectionInvalid( sal_Int32 nStart, sal_Int32 nEnd );

    void                SetVisible( bool bVisible );
    bool                IsVisible() const { return bVisible; }

    bool                IsEmpty() { return GetTextPortions().Count() == 1 && GetTextPortions()[0].GetLen() == 0; }

    long                GetHeight() const           { return ( bVisible ? nHeight : 0 ); }
    sal_Int32           GetFirstLineOffset() const  { return ( bVisible ? nFirstLineOffset : 0 ); }
    void                ResetHeight()   { nHeight = 0; nFirstLineOffset = 0; }

    ContentNode*        GetNode() const             { return pNode; }
    TextPortionList&    GetTextPortions()           { return aTextPortionList; }
    const TextPortionList& GetTextPortions() const { return aTextPortionList; }

    sal_Int32           GetInvalidPosStart() const  { return nInvalidPosStart; }
    short               GetInvalidDiff() const      { return nInvalidDiff; }

    void                CorrectValuesBehindLastFormattedLine( sal_Int32 nLastFormattedLine );
#if OSL_DEBUG_LEVEL > 0
    static bool DbgCheckTextPortions(ParaPortion const&);
#endif
};


// class ParaPortionList

class ParaPortionList
{
    mutable sal_Int32 nLastCache;
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

#if OSL_DEBUG_LEVEL > 0
    // temporary:
    static void DbgCheck(ParaPortionList const&, EditDoc const& rDoc);
#endif
};


// class EditSelection

class EditSelection
{
private:
    EditPaM         aStartPaM;
    EditPaM         aEndPaM;

public:
                    EditSelection();    // No constructor and destructor
                                        // are automatically executed correctly!
                    EditSelection( const EditPaM& rStartAndAnd );
                    EditSelection( const EditPaM& rStart, const EditPaM& rEnd );

    EditPaM&        Min()               { return aStartPaM; }
    EditPaM&        Max()               { return aEndPaM; }

    const EditPaM&  Min() const         { return aStartPaM; }
    const EditPaM&  Max() const         { return aEndPaM; }

    bool            HasRange() const    { return aStartPaM != aEndPaM; }
    bool            IsInvalid() const { return !aStartPaM || !aEndPaM; }
    bool            DbgIsBuggy( EditDoc& rDoc );

    void            Adjust( const EditDoc& rNodes );

    EditSelection&  operator = ( const EditPaM& r );
    bool            operator == ( const EditSelection& r ) const
                    { return ( aStartPaM == r.aStartPaM ) && ( aEndPaM == r.aEndPaM ); }
    bool            operator != ( const EditSelection& r ) const { return !( r == *this ); }
};


// class DeletedNodeInfo

class DeletedNodeInfo
{
private:
    ContentNode*    mpInvalidNode;
    sal_Int32       nInvalidParagraph;

public:
            DeletedNodeInfo( ContentNode* pNode, sal_Int32 nPos )
            : mpInvalidNode(pNode)
            , nInvalidParagraph(nPos)
            {
            }

    ContentNode* GetNode() const { return mpInvalidNode; }
    sal_Int32    GetPosition() const { return nInvalidParagraph; }
};


// class EditDoc

class EditDoc
{
private:
    mutable sal_Int32 nLastCache;
    boost::ptr_vector<ContentNode> maContents;

    SfxItemPool*    pItemPool;
    Link<LinkParamNone*,void>      aModifyHdl;

    SvxFont         aDefFont;           //faster than ever from the pool!!
    sal_uInt16      nDefTab;
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

    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink ) { aModifyHdl = rLink; }

    void            CreateDefFont( bool bUseStyles );
    const SvxFont&  GetDefFont() { return aDefFont; }

    void            SetDefTab( sal_uInt16 nTab )    { nDefTab = nTab ? nTab : DEFTAB; }
    sal_uInt16      GetDefTab() const           { return nDefTab; }

    void            SetVertical( bool bVertical )   { bIsVertical = bVertical; }
    bool            IsVertical() const              { return bIsVertical; }

    void            SetFixedCellHeight( bool bUseFixedCellHeight )  { bIsFixedCellHeight = bUseFixedCellHeight; }
    bool            IsFixedCellHeight() const               { return bIsFixedCellHeight; }

    EditPaM         Clear();
    EditPaM         RemoveText();
    EditPaM         RemoveChars( EditPaM aPaM, sal_Int32 nChars );
    EditPaM         InsertText( EditPaM aPaM, const OUString& rStr );
    EditPaM         InsertParaBreak( EditPaM aPaM, bool bKeepEndingAttribs );
    EditPaM         InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem );
    EditPaM         ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight );

    OUString        GetText( LineEnd eEnd ) const;
    sal_uLong       GetTextLen() const;

    OUString       GetParaAsString( sal_Int32 nNode ) const;
    static OUString  GetParaAsString(const ContentNode* pNode, sal_Int32 nStartPos = 0, sal_Int32 nEndPos = -1, bool bResolveFields = true);

    EditPaM GetStartPaM() const;
    EditPaM GetEndPaM() const;

    SfxItemPool&        GetItemPool()                   { return *pItemPool; }
    const SfxItemPool&  GetItemPool() const             { return *pItemPool; }

    void RemoveItemsFromPool(const ContentNode& rNode);

    void            InsertAttrib( const SfxPoolItem& rItem, ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd );
    void            InsertAttrib( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, const SfxPoolItem& rPoolItem );
    void            InsertAttribInSelection( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, const SfxPoolItem& rPoolItem );
    bool            RemoveAttribs( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, sal_uInt16 nWhich = 0 );
    bool            RemoveAttribs( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, sal_uInt16 nWhich = 0 );
    static void     FindAttribs( ContentNode* pNode, sal_Int32 nStartPos, sal_Int32 nEndPos, SfxItemSet& rCurSet );

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

inline EditCharAttrib* GetAttrib(CharAttribList::AttribsType& rAttribs, sal_Int32 nAttr)
{
    return (nAttr < (sal_Int32)rAttribs.size()) ? rAttribs[nAttr].get() : nullptr;
}

void CheckOrderedList(const CharAttribList::AttribsType& rAttribs, bool bStart);


// class EditEngineItemPool

class EditEngineItemPool : public SfxItemPool
{
public:
                        EditEngineItemPool( bool bPersistenRefCounts );
protected:
                        virtual ~EditEngineItemPool();
public:

    virtual SvStream&   Store( SvStream& rStream ) const override;
};

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EDITDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

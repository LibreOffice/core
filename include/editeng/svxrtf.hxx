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

#ifndef INCLUDED_EDITENG_SVXRTF_HXX
#define INCLUDED_EDITENG_SVXRTF_HXX

#include <svl/itemset.hxx>
#include <svtools/parrtf.hxx>
#include <rtl/ustring.hxx>

#include <editeng/editengdllapi.h>

#include <deque>
#include <utility>
#include <vector>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace vcl { class Font; }
class Color;
class Graphic;
class DateTime;
struct SvxRTFStyleType;
class SvxRTFItemStackType;
class SvxRTFItemStackList : public boost::ptr_vector<SvxRTFItemStackType> {};

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
    namespace util {
        struct DateTime;
    }
} } }


// Mapper-Classes for the various requirements on Document positions
//        Swg - NodePosition is a SwIndex, which is used internally
// EditEngine - ULONG to list of paragraphs


class SvxNodeIdx
{
public:
    virtual ~SvxNodeIdx() {}
    virtual sal_Int32   GetIdx() const = 0;
    virtual SvxNodeIdx* Clone() const = 0;  // Cloning itself
};

class SvxPosition
{
public:
    virtual ~SvxPosition() {}

    virtual sal_Int32   GetNodeIdx() const = 0;
    virtual sal_Int32   GetCntIdx() const = 0;

    virtual SvxPosition* Clone() const = 0; // Cloning itself
    virtual SvxNodeIdx* MakeNodeIdx() const = 0; // Cloning NodeIndex
};


typedef Color* ColorPtr;
typedef std::deque< ColorPtr > SvxRTFColorTbl;
typedef boost::ptr_map<short, vcl::Font> SvxRTFFontTbl;
typedef boost::ptr_map<sal_uInt16, SvxRTFStyleType> SvxRTFStyleTbl;

// SvxRTFItemStack can't be "std::stack< SvxRTFItemStackType* >" type, because
// the methods are using operator[] in sw/source/filter/rtf/rtftbl.cxx file
typedef std::deque< SvxRTFItemStackType* > SvxRTFItemStack;

// own helper classes for the RTF Parser
struct SvxRTFStyleType
{
    SfxItemSet aAttrSet;        // the attributes of Style (+ derivate!)
    OUString sName;
    sal_uInt16 nBasedOn, nNext;
    bool bBasedOnIsSet;
    sal_uInt8 nOutlineNo;
    bool bIsCharFmt;

    SvxRTFStyleType( SfxItemPool& rPool, const sal_uInt16* pWhichRange );
};


// Here are the IDs for all character attributes, which can be detected by
// SvxParser and can be set in a SfxItemSet. The IDs are set correctly through
// the SlotIds from POOL.
struct RTFPlainAttrMapIds
{
    sal_uInt16  nCaseMap,
            nBgColor,
            nColor,
            nContour,
            nCrossedOut,
            nEscapement,
            nFont,
            nFontHeight,
            nKering,
            nLanguage,
            nPosture,
            nShadowed,
            nUnderline,
            nOverline,
            nWeight,
            nWordlineMode,
            nAutoKerning,
            nCJKFont,
            nCJKFontHeight,
            nCJKLanguage,
            nCJKPosture,
            nCJKWeight,
            nCTLFont,
            nCTLFontHeight,
            nCTLLanguage,
            nCTLPosture,
            nCTLWeight,
            nEmphasis,
            nTwoLines,
            nCharScaleX,
            nHorzVert,
            nRuby,
            nRelief,
            nHidden
            ;
    RTFPlainAttrMapIds( const SfxItemPool& rPool );
};

// Here are the IDs for all paragraph attributes, which can be detected by
// SvxParser and can be set in a SfxItemSet. The IDs are set correctly through
// the SlotIds from POOL.
struct RTFPardAttrMapIds
{
    sal_uInt16  nLinespacing,
            nAdjust,
            nTabStop,
            nHyphenzone,
            nLRSpace,
            nULSpace,
            nBrush,
            nBox,
            nShadow,
            nOutlineLvl,
            nSplit,
            nKeep,
            nFontAlign,
            nScriptSpace,
            nHangPunct,
            nForbRule,
            nDirection
            ;
    RTFPardAttrMapIds( const SfxItemPool& rPool );
};






class EDITENG_DLLPUBLIC SvxRTFParser : public SvRTFParser
{
    SvStream &rStrm;
    SvxRTFColorTbl  aColorTbl;
    SvxRTFFontTbl   aFontTbl;
    SvxRTFStyleTbl  aStyleTbl;
    SvxRTFItemStack aAttrStack;
    SvxRTFItemStackList aAttrSetList;

    RTFPlainAttrMapIds aPlainMap;
    RTFPardAttrMapIds aPardMap;
    std::vector<sal_uInt16> aWhichMap;
    OUString  sBaseURL;

    SvxPosition* pInsPos;
    SfxItemPool* pAttrPool;
    Color*  pDfltColor;
    vcl::Font*   pDfltFont;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> m_xDocProps;
    SfxItemSet *pRTFDefaults;

    long    nVersionNo;
    int     nDfltFont;

    bool    bNewDoc : 1;            // sal_False - Reading in an existing
    bool    bNewGroup : 1;          // sal_True - there was an opening parenthesis
    bool    bIsSetDfltTab : 1;      // sal_True - DefTab was loaded
    bool    bChkStyleAttr : 1;      // sal_True - StyleSheets are evaluated
    bool    bCalcValue : 1;         // sal_True - Twip values adapt to App
    bool    bPardTokenRead : 1;     // sal_True - Token \pard was detected
    bool    bReadDocInfo : 1;       // sal_True - DocInfo to read
    bool    bIsLeftToRightDef : 1;  // sal_True - in LeftToRight char run def.
                                        // sal_False - in RightToLeft char run def.
    bool    bIsInReadStyleTab : 1;  // sal_True - in ReadStyleTable

    void ClearColorTbl();
    void ClearFontTbl();
    void ClearStyleTbl();
    void ClearAttrStack();

    SvxRTFItemStackType* _GetAttrSet( int bCopyAttr=sal_False );  // Create new ItemStackType:s
    void _ClearStyleAttr( SvxRTFItemStackType& rStkType );

    // Sets all the attributes that are different from the current
    void SetAttrSet( SfxItemSet& rAttrSet, SvxPosition& rSttPos );
    void SetAttrSet( SvxRTFItemStackType &rSet );
    void SetDefault( int nToken, int nValue );

    // Excecute pard / plain
    void RTFPardPlain( int bPard, SfxItemSet** ppSet );

    void BuildWhichTbl();

    enum RTF_CharTypeDef
    {
        NOTDEF_CHARTYPE,
        LOW_CHARTYPE,
        HIGH_CHARTYPE,
        DOUBLEBYTE_CHARTYPE
    };

        // set latin/asian/complex character attributes
    void SetScriptAttr(
        RTF_CharTypeDef eType, SfxItemSet& rSet, SfxPoolItem& rItem );

protected:
    virtual void EnterEnvironment();
    virtual void LeaveEnvironment();
    virtual void ResetPard();
    virtual void InsertPara() = 0;


    OUString& DelCharAtEnd( OUString& rStr, const sal_Unicode cDel );

    // is called for each token that is recognized in CallParser
    virtual void NextToken( int nToken ) SAL_OVERRIDE;

    virtual void ReadBitmapData() SAL_OVERRIDE;
    virtual void ReadOLEData() SAL_OVERRIDE;

    void ReadStyleTable();
    void ReadColorTable();
    void ReadFontTable();
    void ReadAttr( int nToken, SfxItemSet* pSet );
    void ReadTabAttr( int nToken, SfxItemSet& rSet );

    // Read Document-Info
    ::com::sun::star::util::DateTime GetDateTimeStamp( );
    OUString& GetTextToEndGroup( OUString& rStr );
    virtual void ReadInfo( const sal_Char* pChkForVerNo = 0 );

    inline SfxItemSet& GetAttrSet();
    // no text yet inserted? (SttPos from the top stack entry!)
    bool IsAttrSttPos();
    void AttrGroupEnd();        // edit the current, delete from stack
    void SetAllAttrOfStk();     // end all Attr. and set it into doc


    virtual void InsertText() = 0;
    virtual void MovePos( int bForward = sal_True ) = 0;
    virtual void SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                 sal_Int32& rCntPos )=0;
    virtual void SetAttrInDoc( SvxRTFItemStackType &rSet );
    // for Tokens, which are not evaluated in ReadAttr
    virtual void UnknownAttrToken( int nToken, SfxItemSet* pSet );

    // if no-one would like to have any twips
    virtual void CalcValue();

    SvxRTFParser( SfxItemPool& rAttrPool,
                    SvStream& rIn,
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::document::XDocumentProperties> i_xDocProps,
                    int bReadNewDoc = sal_True );
    virtual ~SvxRTFParser();

    bool IsNewDoc() const               { return bNewDoc; }
    void SetNewDoc( bool bFlag )        { bNewDoc = bFlag; }
    bool IsNewGroup() const             { return bNewGroup; }
    void SetNewGroup( bool bFlag )      { bNewGroup = bFlag; }
    bool IsChkStyleAttr() const         { return bChkStyleAttr; }
    void SetChkStyleAttr( bool bFlag )  { bChkStyleAttr = bFlag; }
    bool IsCalcValue() const            { return bCalcValue; }
    void SetCalcValue( bool bFlag )     { bCalcValue = bFlag; }
    bool IsPardTokenRead() const        { return bPardTokenRead; }
    void SetPardTokenRead( bool bFlag ) { bPardTokenRead = bFlag; }
    bool IsReadDocInfo() const          { return bReadDocInfo; }
    void SetReadDocInfo( bool bFlag )   { bReadDocInfo = bFlag; }

    // Query/Set the current insert position
    SvxPosition& GetInsPos() const      { return *pInsPos; }
    void SetInsPos( const SvxPosition& rNew );

    long GetVersionNo() const           { return nVersionNo; }

    SvxRTFStyleTbl& GetStyleTbl()               { return aStyleTbl; }
    SvxRTFItemStack& GetAttrStack()             { return aAttrStack; }
    SvxRTFColorTbl& GetColorTbl()               { return aColorTbl; }
    SvxRTFFontTbl& GetFontTbl()                 { return aFontTbl; }

    const OUString& GetBaseURL() const          { return sBaseURL; }

public:

    virtual SvParserState CallParser() SAL_OVERRIDE;

    inline const Color& GetColor( size_t nId ) const;
    const vcl::Font& GetFont( sal_uInt16 nId );      // Changes the default Font

    virtual bool IsEndPara( SvxNodeIdx* pNd, sal_Int32 nCnt ) const = 0;

    // to det a different attribute pool. May only be done prior to CallParser!
    // The maps are not generated anew!
    void SetAttrPool( SfxItemPool* pNewPool )   { pAttrPool = pNewPool; }
    // to set different WhichIds for a different pool.
    RTFPardAttrMapIds& GetPardMap() { return aPardMap; }
    RTFPlainAttrMapIds& GetPlainMap() { return aPlainMap; }
    // to be able to assign them from the outside as for example table cells
    void ReadBorderAttr( int nToken, SfxItemSet& rSet, int bTableDef=sal_False );
    void ReadBackgroundAttr( int nToken, SfxItemSet& rSet, int bTableDef=sal_False  );

    // for asynchronous read from the SvStream
    virtual void Continue( int nToken ) SAL_OVERRIDE;

    // get RTF default ItemSets. Must be used by pard/plain tokens or in
    // reset of Style-Items
    const SfxItemSet& GetRTFDefaults();
    virtual bool UncompressableStackEntry(const SvxRTFItemStackType &rSet) const;
};

// The stack for the attributes:
// this class may only be used by SvxRTFParser!
class EDITENG_DLLPUBLIC SvxRTFItemStackType
{
    friend class SvxRTFParser;

    SfxItemSet  aAttrSet;
    SvxNodeIdx  *pSttNd, *pEndNd;
    sal_Int32 nSttCnt, nEndCnt;
    SvxRTFItemStackList* pChildList;
    sal_uInt16 nStyleNo;

    SvxRTFItemStackType( SfxItemPool&, const sal_uInt16* pWhichRange,
                            const SvxPosition& );

    void Add( SvxRTFItemStackType* );
    void Compress( const SvxRTFParser& );

public:
    SvxRTFItemStackType( const SvxRTFItemStackType&, const SvxPosition&,
                        int bCopyAttr = sal_False );
    ~SvxRTFItemStackType();
    //cmc, I'm very suspicios about SetStartPos, it doesn't change
    //its children's starting position, and the implementation looks
    //bad, consider this deprecated.
    void SetStartPos( const SvxPosition& rPos );

    void MoveFullNode(const SvxNodeIdx &rOldNode,
        const SvxNodeIdx &rNewNode);

    sal_Int32 GetSttNodeIdx() const { return pSttNd->GetIdx(); }
    sal_Int32 GetEndNodeIdx() const { return pEndNd->GetIdx(); }

    const SvxNodeIdx& GetSttNode() const { return *pSttNd; }
    const SvxNodeIdx& GetEndNode() const { return *pEndNd; }

    sal_Int32 GetSttCnt() const { return nSttCnt; }
    sal_Int32 GetEndCnt() const { return nEndCnt; }

          SfxItemSet& GetAttrSet()          { return aAttrSet; }
    const SfxItemSet& GetAttrSet() const    { return aAttrSet; }

    sal_uInt16 StyleNo() const  { return nStyleNo; }

    void SetRTFDefaults( const SfxItemSet& rDefaults );
};


// ----------- Inline Implementations --------------

inline const Color& SvxRTFParser::GetColor( size_t nId ) const
{
    ColorPtr pColor = (ColorPtr)pDfltColor;
    if( nId < aColorTbl.size() )
        pColor = aColorTbl[ nId ];
    return *pColor;
}

inline SfxItemSet& SvxRTFParser::GetAttrSet()
{
    SvxRTFItemStackType* pTmp;
    if( bNewGroup || 0 == ( pTmp = aAttrStack.empty() ? 0 : aAttrStack.back()) )
        pTmp = _GetAttrSet();
    return pTmp->aAttrSet;
}


#endif
    // INCLUDED_EDITENG_SVXRTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

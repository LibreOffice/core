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

#ifndef _SVXRTF_HXX
#define _SVXRTF_HXX

#include <tools/table.hxx>
#include <tools/string.hxx>
#include <svl/itemset.hxx>
#include <svtools/parrtf.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <editeng/editengdllapi.h>
#include <utility>
#include <vector>
class Font;
class Color;
class Graphic;
class DateTime;
struct SvxRTFStyleType;
class SvxRTFItemStackType;
class SvxRTFItemStackList;

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
// .....

class SvxNodeIdx
{
public:
    virtual ~SvxNodeIdx() {}
    virtual ULONG   GetIdx() const = 0;
    virtual SvxNodeIdx* Clone() const = 0;  // Cloning itself
};

class SvxPosition
{
public:
    virtual ~SvxPosition() {}

    virtual ULONG       GetNodeIdx() const = 0;
    virtual xub_StrLen  GetCntIdx() const = 0;

    virtual SvxPosition* Clone() const = 0; // Cloning itself
    virtual SvxNodeIdx* MakeNodeIdx() const = 0; // Cloning NodeIndex
};


typedef Color* ColorPtr;
SV_DECL_PTRARR( SvxRTFColorTbl, ColorPtr, 16, 4 )
DECLARE_TABLE( SvxRTFFontTbl, Font* )
DECLARE_TABLE( SvxRTFStyleTbl, SvxRTFStyleType* )
typedef SvxRTFItemStackType* SvxRTFItemStackTypePtr;
SV_DECL_PTRARR_DEL( SvxRTFItemStackList, SvxRTFItemStackTypePtr, 1, 1 )
SV_DECL_PTRARR_STACK( SvxRTFItemStack, SvxRTFItemStackTypePtr, 0, 1 )

// own helper classes for the RTF Parser
struct SvxRTFStyleType
{
    SfxItemSet aAttrSet;        // the attributes of Style (+ derivate!)
    String sName;
    USHORT nBasedOn, nNext;
    BOOL bBasedOnIsSet;
    BYTE nOutlineNo;
    BOOL bIsCharFmt;

    SvxRTFStyleType( SfxItemPool& rPool, const USHORT* pWhichRange );
};


// Bitmap - Mode
typedef ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > > PictPropertyNameValuePairs;
struct EDITENG_DLLPUBLIC SvxRTFPictureType
{
    // Bitmap Format
    enum RTF_BMPSTYLE
    {
        RTF_BITMAP,         // Meta data: Bitmap
        WIN_METAFILE,       // Meta data: Window-Metafile
        MAC_QUICKDRAW,      // Meta data: Mac-QuickDraw
        OS2_METAFILE,       // Meta data: OS2-Metafile
        RTF_DI_BMP,         // Meta data: Device Independent Bitmap
        ENHANCED_MF,        // Meta data: Enhanced-Metafile
        RTF_PNG,            // Meta data: PNG file
        RTF_JPG             // Meta data: JPG file
    } eStyle;

    enum RTF_BMPMODE
    {
        BINARY_MODE,
        HEX_MODE
    } nMode;

    USHORT  nType;
    sal_uInt32 uPicLen;
    USHORT  nWidth, nHeight;
    USHORT  nGoalWidth, nGoalHeight;
    USHORT  nBitsPerPixel;
    USHORT  nPlanes;
    USHORT  nWidthBytes;
    USHORT  nScalX, nScalY;
    short   nCropT, nCropB, nCropL, nCropR;
    PictPropertyNameValuePairs aPropertyPairs;
    SvxRTFPictureType() { ResetValues(); }
    // Reset all values to default; is called after loading the Bitmap
    void ResetValues();
};

// Here are the IDs for all character attributes, which can be detected by
// SvxParser and can be set in a SfxItemSet. The IDs are set correctly throught
// the SlotIds from POOL.
struct RTFPlainAttrMapIds
{
    USHORT  nCaseMap,
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
// SvxParser and can be set in a SfxItemSet. The IDs are set correctly throught
// the SlotIds from POOL.
struct RTFPardAttrMapIds
{
    USHORT  nLinespacing,
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



// -----------------------------------------------------------------------


class EDITENG_DLLPUBLIC SvxRTFParser : public SvRTFParser
{
    SvStream &rStrm;
    SvxRTFColorTbl  aColorTbl;
    SvxRTFFontTbl   aFontTbl;
    SvxRTFStyleTbl  aStyleTbl;
    SvxRTFItemStack aAttrStack;
    SvxRTFItemStackList aAttrSetList;

    SvUShorts aPlainMap;
    SvUShorts aPardMap;
    SvUShorts aWhichMap;
    String  sBaseURL;

    SvxPosition* pInsPos;
    SfxItemPool* pAttrPool;
    Color*  pDfltColor;
    Font*   pDfltFont;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> m_xDocProps;
    SfxItemSet *pRTFDefaults;

    long    nVersionNo;
    int     nDfltFont;

    BOOL    bNewDoc : 1;            // FALSE - Reading in an existing
    BOOL    bNewGroup : 1;          // TRUE - there was an opening parenthesis
    BOOL    bIsSetDfltTab : 1;      // TRUE - DefTab was loaded
    BOOL    bChkStyleAttr : 1;      // TRUE - StyleSheets are evaluated
    BOOL    bCalcValue : 1;         // TRUE - Twip values adapt to App
    BOOL    bPardTokenRead : 1;     // TRUE - Token \pard was detected
    BOOL    bReadDocInfo : 1;       // TRUE - DocInfo to read
    BOOL    bIsLeftToRightDef : 1;  // TRUE - in LeftToRight char run def.
                                    // FALSE - in RightToLeft char run def.
    BOOL    bIsInReadStyleTab : 1;  // TRUE - in ReadStyleTable

    void ClearColorTbl();
    void ClearFontTbl();
    void ClearStyleTbl();
    void ClearAttrStack();

    SvxRTFItemStackTypePtr _GetAttrSet( int bCopyAttr=FALSE );  // Create new ItemStackType:s
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


    String& DelCharAtEnd( String& rStr, const sal_Unicode cDel );

    // is called for each token that is recognized in CallParser
    virtual void NextToken( int nToken );

    virtual void ReadBitmapData();
    virtual void ReadOLEData();

    void ReadStyleTable();
    void ReadColorTable();
    void ReadFontTable();
    void ReadAttr( int nToken, SfxItemSet* pSet );
    void ReadTabAttr( int nToken, SfxItemSet& rSet );

    // Read Document-Info
    ::com::sun::star::util::DateTime GetDateTimeStamp( );
    String& GetTextToEndGroup( String& rStr );
    virtual void ReadInfo( const sal_Char* pChkForVerNo = 0 );

    inline SfxItemSet& GetAttrSet();
    // no text yet inserted? (SttPos from the top stack entry!)
    int IsAttrSttPos();
    void AttrGroupEnd();        // edit the current, delete from stack
    void SetAllAttrOfStk();     // end all Attr. and set it into doc


    virtual void InsertText() = 0;
    virtual void MovePos( int bForward = TRUE ) = 0;
    virtual void SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                 xub_StrLen& rCntPos )=0;
    virtual void SetAttrInDoc( SvxRTFItemStackType &rSet );
    // for Tokens, which are not evaluated in ReadAttr
    virtual void UnknownAttrToken( int nToken, SfxItemSet* pSet );

    // if no-one would like to have any twips
    virtual void CalcValue();

    SvxRTFParser( SfxItemPool& rAttrPool,
                    SvStream& rIn,
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::document::XDocumentProperties> i_xDocProps,
                    int bReadNewDoc = TRUE );
    virtual ~SvxRTFParser();

    int IsNewDoc() const                { return bNewDoc; }
    void SetNewDoc( int bFlag )         { bNewDoc = bFlag; }
    int IsNewGroup() const              { return bNewGroup; }
    void SetNewGroup( int bFlag )       { bNewGroup = bFlag; }
    int IsChkStyleAttr() const          { return bChkStyleAttr; }
    void SetChkStyleAttr( int bFlag )   { bChkStyleAttr = bFlag; }
    int IsCalcValue() const             { return bCalcValue; }
    void SetCalcValue( int bFlag )      { bCalcValue = bFlag; }
    int IsPardTokenRead() const         { return bPardTokenRead; }
    void SetPardTokenRead( int bFlag )  { bPardTokenRead = bFlag; }
    int IsReadDocInfo() const           { return bReadDocInfo; }
    void SetReadDocInfo( int bFlag )    { bReadDocInfo = bFlag; }

    // Query/Set the current insert position
    SvxPosition& GetInsPos() const      { return *pInsPos; }
    void SetInsPos( const SvxPosition& rNew );

    long GetVersionNo() const           { return nVersionNo; }

    // Query/Set the mapping IDs for the Pard/Plain attributes
    //(Set: It is noted in the pointers, which thus does not create a copy)
    void AddPardAttr( USHORT nWhich ) { aPardMap.Insert( nWhich, aPardMap.Count() ); }
    void AddPlainAttr( USHORT nWhich ) { aPlainMap.Insert( nWhich, aPlainMap.Count() ); }

    SvxRTFStyleTbl& GetStyleTbl()               { return aStyleTbl; }
    SvxRTFItemStack& GetAttrStack()             { return aAttrStack; }
    SvxRTFColorTbl& GetColorTbl()               { return aColorTbl; }
    SvxRTFFontTbl& GetFontTbl()                 { return aFontTbl; }

    const String& GetBaseURL() const            { return sBaseURL; }

    // Read the graphics data and make up for the graphics and the picture
    // meta data.
    // Return - TRUE: the graphic is valid
    BOOL ReadBmpData( Graphic& rGrf, SvxRTFPictureType& rPicType );
        // Change the ASCII-HexCodes into binary characters. If invalid data is
        // found (strings not 0-9 | a-f | A-F, then USHRT_MAX is returned,
        // otherwise the number of the converted character.
    xub_StrLen HexToBin( String& rToken );

public:

    virtual SvParserState CallParser();

    inline const Color& GetColor( USHORT nId ) const;
    const Font& GetFont( USHORT nId );      // Changes the dflt Font

    virtual int IsEndPara( SvxNodeIdx* pNd, xub_StrLen nCnt ) const = 0;

    // to det a different attribute pool. May only be done prior to CallParser!
    // The maps are not generated anew!
    void SetAttrPool( SfxItemPool* pNewPool )   { pAttrPool = pNewPool; }
    // to set different WhichIds for a different pool.
    RTFPardAttrMapIds& GetPardMap()
                        { return (RTFPardAttrMapIds&)*aPardMap.GetData(); }
    RTFPlainAttrMapIds& GetPlainMap()
                        { return (RTFPlainAttrMapIds&)*aPlainMap.GetData(); }
    // to be able to assign them from the outside as for example table cells
    void ReadBorderAttr( int nToken, SfxItemSet& rSet, int bTableDef=FALSE );
    void ReadBackgroundAttr( int nToken, SfxItemSet& rSet, int bTableDef=FALSE  );

    // for asynchronous read from the SvStream
    virtual void Continue( int nToken );

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
    friend class SvxRTFItemStackList;

    SfxItemSet  aAttrSet;
    SvxNodeIdx  *pSttNd, *pEndNd;
    xub_StrLen nSttCnt, nEndCnt;
    SvxRTFItemStackList* pChildList;
    USHORT nStyleNo;

    SvxRTFItemStackType( SfxItemPool&, const USHORT* pWhichRange,
                            const SvxPosition& );
    ~SvxRTFItemStackType();

    void Add( SvxRTFItemStackTypePtr );
    void Compress( const SvxRTFParser& );

public:
    SvxRTFItemStackType( const SvxRTFItemStackType&, const SvxPosition&,
                        int bCopyAttr = FALSE );
    //cmc, I'm very suspicios about SetStartPos, it doesn't change
    //its children's starting position, and the implementation looks
    //bad, consider this deprecated.
    void SetStartPos( const SvxPosition& rPos );

    void MoveFullNode(const SvxNodeIdx &rOldNode,
        const SvxNodeIdx &rNewNode);

    ULONG GetSttNodeIdx() const { return pSttNd->GetIdx(); }
    ULONG GetEndNodeIdx() const { return pEndNd->GetIdx(); }

    const SvxNodeIdx& GetSttNode() const { return *pSttNd; }
    const SvxNodeIdx& GetEndNode() const { return *pEndNd; }

    xub_StrLen GetSttCnt() const { return nSttCnt; }
    xub_StrLen GetEndCnt() const { return nEndCnt; }

          SfxItemSet& GetAttrSet()          { return aAttrSet; }
    const SfxItemSet& GetAttrSet() const    { return aAttrSet; }

    USHORT StyleNo() const  { return nStyleNo; }

    void SetRTFDefaults( const SfxItemSet& rDefaults );
};


// ----------- Inline Implementations --------------

inline const Color& SvxRTFParser::GetColor( USHORT nId ) const
{
    ColorPtr pColor = (ColorPtr)pDfltColor;
    if( nId < aColorTbl.Count() )
        pColor = aColorTbl[ nId ];
    return *pColor;
}

inline SfxItemSet& SvxRTFParser::GetAttrSet()
{
    SvxRTFItemStackTypePtr pTmp;
    if( bNewGroup || 0 == ( pTmp = aAttrStack.Top()) )
        pTmp = _GetAttrSet();
    return pTmp->aAttrSet;
}


#endif
    //_SVXRTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

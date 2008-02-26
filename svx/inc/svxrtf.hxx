/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svxrtf.hxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:35:12 $
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

#ifndef _SVXRTF_HXX
#define _SVXRTF_HXX

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _PARRTF_HXX //autogen
#include <svtools/parrtf.hxx>
#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


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
        class DateTime;
    }
} } }


// Mapper-Klassen fuer die verschiedenen Anforderungen an Doc-Positionen
//        Swg - NodePosition ist ein SwIndex, der intern veraendert wird
// EditEngine - ULONG auf Liste von Absaetzen
// .....

class SvxNodeIdx
{
public:
    virtual ~SvxNodeIdx() {}
    virtual ULONG   GetIdx() const = 0;
    virtual SvxNodeIdx* Clone() const = 0;  // erzeuge von sich eine Kopie
};

class SvxPosition
{
public:
    virtual ~SvxPosition() {}

    virtual ULONG       GetNodeIdx() const = 0;
    virtual xub_StrLen  GetCntIdx() const = 0;

    virtual SvxPosition* Clone() const = 0; // erzeuge von sich eine Kopie
    virtual SvxNodeIdx* MakeNodeIdx() const = 0; // erzeuge vom NodeIndex eine Kopie
};


typedef Color* ColorPtr;
SV_DECL_PTRARR( SvxRTFColorTbl, ColorPtr, 16, 4 )
DECLARE_TABLE( SvxRTFFontTbl, Font* )
DECLARE_TABLE( SvxRTFStyleTbl, SvxRTFStyleType* )
typedef SvxRTFItemStackType* SvxRTFItemStackTypePtr;
SV_DECL_PTRARR_DEL( SvxRTFItemStackList, SvxRTFItemStackTypePtr, 1, 1 )
SV_DECL_PTRARR_STACK( SvxRTFItemStack, SvxRTFItemStackTypePtr, 0, 1 )

// einige Hilfsklassen fuer den RTF-Parser
struct SvxRTFStyleType
{
    SfxItemSet aAttrSet;        // die Attribute vom Style (+Ableitung!)
    String sName;
    USHORT nBasedOn, nNext;
    BOOL bBasedOnIsSet; //$flr #117411#
    BYTE nOutlineNo;
    BOOL bIsCharFmt;

    SvxRTFStyleType( SfxItemPool& rPool, const USHORT* pWhichRange );
};


// Bitmap - Mode

struct SVX_DLLPUBLIC SvxRTFPictureType
{
    // Format der Bitmap
    enum RTF_BMPSTYLE
    {
        RTF_BITMAP,         // Bitmap, die Planes sind in den Pict.Daten
        WIN_METAFILE,       // in den Pict.Daten steht ein Window-Metafile
        MAC_QUICKDRAW,      // in den Pict.Daten steht ein Mac-QuickDraw
        OS2_METAFILE,       // in den Pict.Daten steht ein OS2-Metafile
        RTF_DI_BMP,         // Device Independent Bitmap
        ENHANCED_MF,        // in den Pict.Daten steht ein Enhanced-Metafile
        RTF_PNG,            // in den Pict.Daten steht ein PNG file
        RTF_JPG             // in den Pict.Daten steht ein JPG file
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

    SvxRTFPictureType() { ResetValues(); }
    // alle Werte auf default; wird nach einlesen der Bitmap aufgerufen !
    void ResetValues();
};

// Hier sind die Ids fuer alle Charakter-Attribute, die vom SvxParser
// erkannt und in einem SfxItemSet gesetzt werden koennen.
// Die Ids werden ueber die SlotIds vom POOL richtig gesetzt.
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

// Hier sind die Ids fuer alle Paragraph-Attribute, die vom SvxParser
// erkannt und in einem SfxItemSet gesetzt werden koennen.
// Die Ids werden ueber die SlotIds vom POOL richtig gesetzt.
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


class SVX_DLLPUBLIC SvxRTFParser : public SvRTFParser
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

    BOOL    bNewDoc : 1;            // FALSE - in ein bestehendes lesen
    BOOL    bNewGroup : 1;          // TRUE - es gab eine oeffnende Klammer
    BOOL    bIsSetDfltTab : 1;      // TRUE - DefTab wurde eingelesen
    BOOL    bChkStyleAttr : 1;      // TRUE - StyleSheets werden ausgewertet
    BOOL    bCalcValue : 1;         // TRUE - Twipwerte an APP anpassen
    BOOL    bPardTokenRead : 1;     // TRUE - Token \pard wurde erkannt
    BOOL    bReadDocInfo : 1;       // TRUE - DocInfo mit einlesen
    BOOL    bIsLeftToRightDef : 1;  // TRUE - in LeftToRight char run def.
                                    // FALSE - in RightToLeft char run def.
    BOOL    bIsInReadStyleTab : 1;  // TRUE - in ReadStyleTable

    void ClearColorTbl();
    void ClearFontTbl();
    void ClearStyleTbl();
    void ClearAttrStack();

    SvxRTFItemStackTypePtr _GetAttrSet( int bCopyAttr=FALSE );  // neue ItemStackType anlegen
    void _ClearStyleAttr( SvxRTFItemStackType& rStkType );

    // setzt alle Attribute, die unterschiedlich zum aktuellen sind
    void SetAttrSet( SfxItemSet& rAttrSet, SvxPosition& rSttPos );
    void SetAttrSet( SvxRTFItemStackType &rSet );
    void SetDefault( int nToken, int nValue );

    // pard / plain abarbeiten
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

    // wird fuer jedes Token gerufen, das in CallParser erkannt wird
    virtual void NextToken( int nToken );

    virtual void ReadBitmapData();
    virtual void ReadOLEData();

    void ReadStyleTable();
    void ReadColorTable();
    void ReadFontTable();
    void ReadAttr( int nToken, SfxItemSet* pSet );
    void ReadTabAttr( int nToken, SfxItemSet& rSet );

    // Dokument-Info lesen
    ::com::sun::star::util::DateTime GetDateTimeStamp( );
    String& GetTextToEndGroup( String& rStr );
    virtual void ReadInfo( const sal_Char* pChkForVerNo = 0 );

    inline SfxItemSet& GetAttrSet();
    // wurde noch kein Text eingefuegt ? (SttPos vom obersten StackEintrag!)
    int IsAttrSttPos();
    void AttrGroupEnd();        // den akt. Bearbeiten, vom Stack loeschen
    void SetAllAttrOfStk();     // end all Attr. and set it into doc


    virtual void InsertText() = 0;
    virtual void MovePos( int bForward = TRUE ) = 0;
    virtual void SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                 xub_StrLen& rCntPos )=0;
    virtual void SetAttrInDoc( SvxRTFItemStackType &rSet );
    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
    virtual void UnknownAttrToken( int nToken, SfxItemSet* pSet );

    // falls jemand keine Twips haben moechte
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

    // erfrage/setze die aktuelle Einfuegeposition
    SvxPosition& GetInsPos() const      { return *pInsPos; }
    void SetInsPos( const SvxPosition& rNew );

    long GetVersionNo() const           { return nVersionNo; }

    // erfrage/setze die Mapping-Ids fuer die Pard/Plain Attribute
    // (Set: es werden sich die Pointer gemerkt, also keine Kopie erzeugt!!! )
    void AddPardAttr( USHORT nWhich ) { aPardMap.Insert( nWhich, aPardMap.Count() ); }
    void AddPlainAttr( USHORT nWhich ) { aPlainMap.Insert( nWhich, aPlainMap.Count() ); }

    SvxRTFStyleTbl& GetStyleTbl()               { return aStyleTbl; }
    SvxRTFItemStack& GetAttrStack()             { return aAttrStack; }
    SvxRTFColorTbl& GetColorTbl()               { return aColorTbl; }
    SvxRTFFontTbl& GetFontTbl()                 { return aFontTbl; }

    const String& GetBaseURL() const            { return sBaseURL; }

        // lesen die GrafikDaten und fuelle damit die Grafik und die
        // die PicDaten.
        // Return - TRUE: die Grafik ist gueltig
    BOOL ReadBmpData( Graphic& rGrf, SvxRTFPictureType& rPicType );
        // wandel die ASCII-HexCodes in binaere Zeichen um. Werden
        // ungueltige Daten gefunden (Zeichen ausser 0-9|a-f|A-F, so
        // wird USHRT_MAX returnt, ansonsten die Anzahl der umgewandelten Ze.
    xub_StrLen HexToBin( String& rToken );

public:

    virtual SvParserState CallParser(); // Aufruf des Parsers

    inline const Color& GetColor( USHORT nId ) const;
    const Font& GetFont( USHORT nId );      // aendert den dflt Font

    virtual int IsEndPara( SvxNodeIdx* pNd, xub_StrLen nCnt ) const = 0;

    // um einen anderen Attribut-Pool setzen zukoennen. Darf nur vorm
    // CallParser erfolgen!! Die Maps werden nicht neu erzeugt!
    void SetAttrPool( SfxItemPool* pNewPool )   { pAttrPool = pNewPool; }
    // um andere WhichIds fuer einen anderen Pool zusetzen.
    RTFPardAttrMapIds& GetPardMap()
                        { return (RTFPardAttrMapIds&)*aPardMap.GetData(); }
    RTFPlainAttrMapIds& GetPlainMap()
                        { return (RTFPlainAttrMapIds&)*aPlainMap.GetData(); }
    // um diese von aussen z.B. Tabellenzellen zuordnen zu koennen
    void ReadBorderAttr( int nToken, SfxItemSet& rSet, int bTableDef=FALSE );
    void ReadBackgroundAttr( int nToken, SfxItemSet& rSet, int bTableDef=FALSE  );

    // fuers asynchrone lesen aus dem SvStream
    virtual void Continue( int nToken );

    // get RTF default ItemSets. Must be used by pard/plain tokens or in
    // reset of Style-Items
    const SfxItemSet& GetRTFDefaults();
    virtual bool UncompressableStackEntry(const SvxRTFItemStackType &rSet) const;
};

// der Stack fuer die Attribute:
// diese Klasse darf nur vom SvxRTFParser benutzt werden!
class SVX_DLLPUBLIC SvxRTFItemStackType
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


// ----------- Inline Implementierungen --------------

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


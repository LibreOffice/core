/*************************************************************************
 *
 *  $RCSfile: excrecds.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-14 11:17:29 $
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

#ifndef _EXCRECDS_HXX
#define _EXCRECDS_HXX

#ifndef SC_OUTLINETAB_HXX
#include "olinetab.hxx"
#endif
#ifndef SC_FILTER_HXX
#include "filter.hxx"
#endif
#ifndef _XFBUFF_HXX
#include "xfbuff.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif
#ifndef _FLTTOOLS_HXX
#include "flttools.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

//------------------------------------------------------------------ Forwards -

class SvStream;
class Font;
class List;
class ScPatternAttr;
class ScTokenArray;
class ScRangeData;
class ScDBData;
class ScEditCell;
class SfxItemSet;
class EditTextObject;
class ScPageHFItem;
class XclRawUnicodeString;

//class ExcRecordList;
class UsedAttrList;

class ScProgress;

//---------------------------------------------------- class ExcETabNumBuffer -
// - stores the correct Excel table number for each Calc table

class ExcETabNumBuffer : private UINT16List
{
private:
    BOOL                        bEnableLog;

    UINT16                      nScCnt;
    UINT16                      nExcCnt;
    UINT16                      nExtCnt;
    UINT32*                     pBuffer;

    void                        ApplyBuffer();

protected:
public:
                                ExcETabNumBuffer( ScDocument& rDoc );
                                ~ExcETabNumBuffer();

    BOOL                        IsExternal( UINT16 nScTab ) const;
    BOOL                        IsExportTable( UINT16 nScTab ) const;
    UINT16                      GetExcTable( UINT16 nScTab ) const;
    inline UINT16               GetScTabCount() const       { return nScCnt; }
    inline UINT16               GetExcTabCount() const      { return nExcCnt; }
    inline UINT16               GetExternTabCount() const   { return nExtCnt; }

// for change tracking:
                                // append table number pair, called by formula compiler
    void                        AppendTabRef( UINT16 nExcFirst, UINT16 nExcLast );
                                // enables logging of excel table nums in every 3D-ref
    inline void                 StartRefLog()       { UINT16List::Clear(); bEnableLog = TRUE; }
                                // disables logging
    inline void                 EndRefLog()         { bEnableLog = FALSE; }
                                // returns the index list
    inline const UINT16List&    GetRefLog() const   { return *this; }
};



//----------------------------------------------------------- class ExcRecord -

class ExcRecord
{
protected:
    static const UINT16     nIgnore;    // fuer Nichtdokumentiertes
    virtual void            SaveCont( SvStream& );
    virtual void            _Save( SvStream& );
                            // fuer ExcRecord-Daten: ACHTUNG: NICHT UEBERLADEN
                            // ... ausser es werden nNumber und nLen SELBST geschrieben!
public:
    virtual                 ~ExcRecord();
    virtual UINT16          GetNum( void ) const = 0;
    virtual UINT16          GetLen( void ) const = 0;
    virtual void            Save( SvStream& );
};


class ExcRecordList : protected List, public ExcRecord
{
private:
protected:
public:
    virtual                 ~ExcRecordList();
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
    virtual void            Save( SvStream& );

    inline ExcRecord*       First( void )               { return ( ExcRecord* ) List::First(); }
    inline ExcRecord*       Next( void )                { return ( ExcRecord* ) List::Next(); }

    inline void             Append( ExcRecord* pNew )   { List::Insert( pNew, LIST_APPEND ); }
    inline const ExcRecord* Get( UINT32 nNum ) const    { return ( ExcRecord* ) List::GetObject( nNum ); }

    List::Count;
};


//--------------------------------------------------------- class ExcEmptyRec -

class ExcEmptyRec : public ExcRecord
{
private:
    virtual void            _Save( SvStream& rStrm );
protected:
public:
    virtual void            Save( SvStream& rStrm );
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//--------------------------------------------------------- class ExcDummyRec -

class ExcDummyRec : public ExcRecord
{
protected:
public:
    void                    Save( SvStream& );
    virtual UINT16          GetNum( void ) const;
    virtual const BYTE*     GetData( void ) const = 0;
};


//--------------------------------------------------------- class ExcBof_Base -

class ExcBof_Base : public ExcRecord
{
private:
protected:
    UINT16                  nDocType;
    UINT16                  nVers;
    UINT16                  nRupBuild;
    UINT16                  nRupYear;
public:
                            ExcBof_Base( void );
};


//------------------------------------------------------------- class ExcCell -

class ExcRKMulRK;

class ExcCell : public ExcRecord
{
friend ExcRKMulRK;
protected:
    ScAddress               aPos;
    UINT16                  nXF;
    static UsedAttrList*    pXFRecs;        // um zu Attributen zu kommen
    static UINT32           nCellCount;     // zaehlt DOPPELT: im Ctor und SaveCont
    static ScProgress*      pPrgrsBar;
#ifdef DBG_UTIL
    friend class ExcDocument;
    static INT32            _nRefCount;
#endif
    // ---------------------------------------------------------------
    void                    SaveCont( SvStream& );
    virtual void            SaveDiff( SvStream& );
                            ExcCell( const ScAddress aPos, const ScPatternAttr* pAttr,
                                    const ULONG nAltNumForm = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                    BOOL bForceAltNumForm = FALSE );
public:
    virtual                 ~ExcCell();
    void                    SetXF( UINT16 nNew )    { nXF = nNew; }
    static void             SetXFRecs( UsedAttrList* pXFRecs );
    static UsedAttrList*    GetXFRecs() { return pXFRecs; }
    inline static void      ResetCellCount( void );
    inline static void      IncCellCount( void );
    inline static UINT32    GetCellCount( void );
    inline static void      SetPrgrsBar( ScProgress& );
    inline static void      ClearPrgrsBar( void );
};


inline void ExcCell::ResetCellCount( void )
{
    nCellCount = 0;
}


inline void ExcCell::IncCellCount( void )
{
    nCellCount++;
}


inline UINT32 ExcCell::GetCellCount( void )
{
    return nCellCount;
}


inline void ExcCell::SetPrgrsBar( ScProgress& rNewBar )
{
    ResetCellCount();       // logisch... oder?
    pPrgrsBar = &rNewBar;
}


inline void ExcCell::ClearPrgrsBar( void )
{
    pPrgrsBar = NULL;
}


//-------------------------------------------------------------- class ExcBof -
// Header Record fuer WORKSHEETS

class ExcBof : public ExcBof_Base
{
private:
    void                    SaveCont( SvStream& );
public:
                            ExcBof( void );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//------------------------------------------------------------- class ExcBofW -
// Header Record fuer WORKBOOKS

class ExcBofW : public ExcBof_Base
{
private:
    void                    SaveCont( SvStream& );
public:
                            ExcBofW( void );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//----------------------------------------------------- class ExcFngroupcount -

class ExcFngroupcount : public ExcRecord
{
private:
    void                    SaveCont( SvStream& );
public:
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//-------------------------------------------------------------- class ExcEof -

class ExcEof : public ExcRecord
{
private:
public:
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//--------------------------------------------------------- class ExcDummy_00 -
// von INTERFACEHDR bis FNGROUPCOUNT (siehe excrecds.cxx)

class ExcDummy_00 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//-------------------------------------------------------- class ExcDummy_04x -
// von WINDOWPROTECT bis BOOKBOOL (siehe excrecds.cxx), 1904 ausgespart

class ExcDummy_040 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


class ExcDummy_041 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


class Exc1904 : public ExcRecord
{
private:
    BOOL                    b1904;
    virtual void            SaveCont( SvStream& );
public:
                            Exc1904( ScDocument& rDoc );
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//------------------------------------------------------------- class ExcFont -

class ExcPalette2;

class ExcFont : public ExcRecord
{
private:
    String                  sName;
    UINT16                  nNameLen;
    UINT16                  nHeight;
    UINT16                  nAttr;      // italic, strikeout, outline, shadow
    UINT32                  nColorSer;
    UINT16                  nWeight;
    UINT8                   nUnder;
    UINT8                   nFamily;
    UINT8                   nCharset;
    BOOL                    bIgnoreCol;
    UINT32                  nSign;      // quick ==
    BiffTyp                 eBiff;
    static ExcPalette2*     pPalette2;
#ifdef DBG_UTIL
    static UINT16           nObjCnt;
#endif
    // ---------------------------------------------------------------
    virtual void            SaveCont( SvStream& rStrm );
public:
                            ExcFont( BiffTyp eBiff );
                            ExcFont( Font* pFont, BiffTyp eBiff );
                            ~ExcFont();

    BOOL                    operator==( const ExcFont& rRef ) const;
    inline BOOL             operator!=( const ExcFont& rRef ) const
                                { return !(*this == rRef); }

    static void             SetPalette( ExcPalette2& rPalette2 );
    void                    SetName( const String& rName );
    void                    SetColor( UINT32 nSerial );
    inline void             SetHeightPt( float fHt )                { nHeight = (UINT16)(fHt * EXC_FONTHGHT_COEFF); }
    inline void             SetHeight( UINT16 nHt )                 { nHeight = nHt; }
    inline void             SetItalic( BOOL bItalic )               { if( bItalic ) nAttr |= EXC_FONTATTR_ITALIC; }
    inline void             SetStrikeout( BOOL bStrikeout )         { if( bStrikeout ) nAttr |= EXC_FONTATTR_STRIKEOUT; }
    inline void             SetOutline( BOOL bOutline )             { if( bOutline ) nAttr |= EXC_FONTATTR_OUTLINE; }
    inline void             SetShadow( BOOL bShadow )               { if( bShadow ) nAttr |= EXC_FONTATTR_SHADOW; }
    inline void             SetWeight( FontWeight eWeight )         { nWeight = GetWeight( eWeight ); }
    inline void             SetUnderline( FontUnderline eUnder )    { nUnder = GetUnderline( eUnder ); }
    inline void             SetFamily( FontFamily eFamily )         { nFamily = GetFamily( eFamily ); }
    inline void             SetCharSet( rtl_TextEncoding eCharset ) { nCharset = GetCharSet( eCharset ); }

    inline BOOL             HasIgnoreCol() const    { return bIgnoreCol; }
    inline UINT32           GetColor() const        { return nColorSer; }

    static UINT16           GetWeight( const FontWeight eWeight );
    static UINT8            GetUnderline( const FontUnderline eUnder );
    static UINT8            GetFamily( const FontFamily eFamily );
    static UINT8            GetCharSet( const rtl_TextEncoding eCharset );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};



//--------------------------------------------------------- class ExcDummy_01 -
// Ersatz fuer Default Font Records

class ExcDummy_01: public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//--------------------------------------------------------- class ExcDummy_Fm -
// Ersatz fuer Default Format Records

class ExcDummy_Fm : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//--------------------------------------------------------- class ExcDummy_XF -
// Ersatz fuer Default XF Records

class ExcDummy_XF : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//------------------------------------------------------ class ExcDummy_Style -
// Ersatz fuer Default Style Records

class ExcDummy_Style : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//------------------------------------------------------ class ExcBundlesheet -

class ExcBundlesheetBase : public ExcRecord
{
protected:
    UINT32                  nStrPos;
    UINT32                  nOwnPos;    // Position NACH # und Len
    UINT16                  nGrbit;

                            ExcBundlesheetBase();

public:
                            ExcBundlesheetBase( RootData& rRootData, UINT16 nTab );

    inline void             SetStreamPos( UINT32 nNewStrPos ) { nStrPos = nNewStrPos; }
    void                    UpdateStreamPos( SvStream& rOut );

    virtual UINT16          GetNum() const;
};


class ExcBundlesheet : public ExcBundlesheetBase
{
private:
    ByteString              aName;

    void                    SaveCont( SvStream& rStrm );

public:
                            ExcBundlesheet( RootData& rRootData, UINT16 nTab );
    virtual UINT16          GetLen() const;
};




//--------------------------------------------------------- class ExcDummy_02 -
// in Tabelle: von CALCMODE bis SETUP

class ExcDummy_02 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//------------------------------------------------------------- class ExcNote -

class ExcNote : public ExcRecord
{
private:
    ByteString*             pText;
    ScAddress               aPos;
    UINT16                  nTextLen;
    void                    SaveCont( SvStream& );
public:
                            ExcNote( const ScAddress, const String& rText, RootData& );
    virtual                 ~ExcNote();

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//----------------------------------------------------------- class ExcNumber -

class ExcNumber : public ExcCell
{
private:
    double                  fVal;
    //----------------------------------------------------------------
    void                    SaveDiff( SvStream& );// statt SaveCont()
public:
                            ExcNumber( const ScAddress, const ScPatternAttr*, const double& rVal );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//---------------------------------------------------------- class ExcBoolerr -

class ExcBoolerr : public ExcCell
{
private:
    UINT8                   nVal;
    UINT8                   bError;
    //----------------------------------------------------------------
    void                    SaveDiff( SvStream& );// statt SaveCont()
public:
                            ExcBoolerr( const ScAddress, const ScPatternAttr*, UINT8 nVal, BOOL bIsError );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//---------------------------------------------------------- class ExcRKMulRK -

class ExcRKMulRK : private List, public ExcRecord
{
private:
    ScAddress               aPos;
    struct CONT
        {
        UINT32              nVal;
        UINT16              nXF;
        };
protected:
    virtual void            SaveCont( SvStream& );
public:
                            ExcRKMulRK( const ScAddress, const ScPatternAttr*, const INT32 nVal );
    virtual                 ~ExcRKMulRK();

    ExcRKMulRK*             Extend( const ScAddress rPos,
                                const ScPatternAttr *pAttr,
                                const INT32 nVal );
                                // passt neuer RK an alten 'ran, so ist return = NULL,
                                // ansonsten wird neuer RK zurueckgeliefert

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;

    inline BOOL             IsRK( void ) const;
    inline BOOL             IsMulRK( void ) const;
};


inline BOOL ExcRKMulRK::IsRK( void ) const
{
    return List::Count() == 1;
}


inline BOOL ExcRKMulRK::IsMulRK( void ) const
{
    return List::Count() > 1;
}


//------------------------------------------------------------ class ExcLabel -

class ExcLabel : public ExcCell
{
private:
    ByteString*             pText;
    UINT16                  nTextLen;
    //----------------------------------------------------------------
    void                    SaveDiff( SvStream& );// statt SaveCont()
public:
                            ExcLabel( const ScAddress, const ScPatternAttr*, const String& rText, RootData& );
    virtual                 ~ExcLabel();

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//---------------------------------------------------------- class ExcRichStr

class XclContinue;

// Helferklasse fuer ExcRString und ExcLabel8/XclRichString
class ExcRichStr
{
private:
    UINT16List              aForms;     // Form und Pos nacheinander
    BiffTyp                 eBiff;
public:
                            ExcRichStr(
                                ExcCell&                rExcCell,
                                String&                 rText,
                                const ScPatternAttr*    pAttr,
                                const ScEditCell&       rEdCell,
                                RootData&               rRoot,
                                xub_StrLen              nMaxChars );
                            ~ExcRichStr();

    inline  UINT16          GetFormCount() const;

                            // number of bytes to be saved
    inline  UINT32          GetByteCount() const;

                            // write list of forms
    void                    Write( XclContinue& rCont );
};


inline UINT16 ExcRichStr::GetFormCount() const
{
    return (UINT16) Min( aForms.Count() / 2, (eBiff < Biff8 ? UINT32(0xff) : UINT32(0xffff)) );
}


inline UINT32 ExcRichStr::GetByteCount() const
{
    return (eBiff < Biff8 ? 2 : 4) * GetFormCount();
}


//---------------------------------------------------------- class ExcRString -

class ExcRString : public ExcCell, ExcRoot
{
private:
    String                  aText;
    ExcRichStr*             pRichStr;
    UINT16                  nTextLen;
    //----------------------------------------------------------------
    void                    SaveDiff( SvStream& );// statt SaveCont()
public:
                            ExcRString(
                                RootData*               pRootData,
                                const ScAddress         aPos,
                                const ScPatternAttr*    pAttr,
                                const ScEditCell&       rEdCell );
    virtual                 ~ExcRString();

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//---------------------------------------------------------- class ExcFormula -

class ExcFormula : public ExcCell
{
private:
//  double                  fCurVal;
    sal_Char*               pData;
    UINT16                  nFormLen;
    //----------------------------------------------------------------
    void                    SaveDiff( SvStream& );// statt SaveCont()
public:
                            ExcFormula( RootData *pRD,
                                const ScAddress rPos,
                                const ScPatternAttr *pAttr,
                                const ULONG nAltNumForm,
                                BOOL bForceAltNumForm,
//                              const double &rCurVal,
                                const ScTokenArray& rCode );
                            ~ExcFormula();

    inline const ScAddress& GetPosition() const { return aPos; }    // from ExcCell

    void                    SetTableOp( USHORT nCol, USHORT nRow ); // for TableOp export

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//---------------------------------------------------- class ExcBlankMulblank -

class ExcBlankMulblank : public ExcCell, private List
{
private:
    UINT16                  nLastCol;
    BOOL                    bMulBlank;
    UINT16                  nLen;

    inline void*            MakeEntry( const UINT16 nXF, const UINT16 nAnz );
    inline UINT16           GetXF( const void* );
    inline UINT16           GetAnz( const void* );
    void                    SaveDiff( SvStream& );// statt SaveCont()
public:
                            ExcBlankMulblank(
                                const ScAddress rPos,
                                const ScPatternAttr* pFirstAttr,
                                UINT16 nFirstAnz );
    void                    Add( const ScPatternAttr* pAttr, const UINT16 nAnz );
    inline UINT16           GetLastCol( void ) const;

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


inline void* ExcBlankMulblank::MakeEntry( const UINT16 nXF, const UINT16 nAddAnz )
{
    return ( void* ) ( ( UINT32 ) nXF + ( ( ( UINT32 ) nAddAnz ) << 16 ) );
}


inline UINT16 ExcBlankMulblank::GetXF( const void *nEntry )
{
    return ( UINT16 ) ( UINT32 ) nEntry;
}


inline UINT16 ExcBlankMulblank::GetAnz( const void* nEntry )
{
    return ( UINT16 ) ( ( ( UINT32 ) nEntry ) >> 16 );
}


inline UINT16 ExcBlankMulblank::GetLastCol( void ) const
{
    return nLastCol;
}




class ExcNameListEntry : public ExcRecord
{
private:
protected:
public:
    virtual                 ~ExcNameListEntry();
    virtual UINT16          GetNum( void ) const;
};




//------------------------------------------------------------- class ExcName -

class ExcName : public ExcNameListEntry, ExcRoot
{
private:
    String                  aName;
    UINT8*                  pData;
    UINT16                  nFormLen;
    UINT16                  nTabNum;
    BOOL                    bHidden;
    BOOL                    bBuiltIn;
    BOOL                    bDummy;
    BiffTyp                 eBiff;
    //----------------------------------------------------------------
    void                    Init( BOOL bHid = FALSE, BOOL bBIn = FALSE );
    void                    BuildFormula( const ScRange& rRange );
    void                    SaveCont( SvStream& );
public:
                            ExcName( RootData* pRD, ScRangeData* pRange );
                            ExcName( RootData* pRD, ScDBData* pArea );
                            ExcName( RootData* pRD, const ScRange& rRange,
                                    UINT8 nBuiltIn, BOOL bHid = FALSE );
                            ~ExcName();

//  virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
    virtual void            Save( SvStream& );

    inline BOOL             IsDummy( void ) const;
};


inline BOOL ExcName::IsDummy( void ) const
{
    return bDummy;
}




//--------------------------------------------------------- class ExcNameList -

class ExcNameList : public ExcRecord, private List
{
private:
    inline ExcNameListEntry*    _First()    { return (ExcNameListEntry*) List::First(); }
    inline ExcNameListEntry*    _Next()     { return (ExcNameListEntry*) List::Next(); }

protected:
    virtual void            _Save( SvStream& rStrm );
    virtual void            SaveCont( SvStream& rStrm );
public:
    virtual                 ~ExcNameList();

    inline void             Append( ExcNameListEntry* pName )
                                { List::Insert( pName, LIST_APPEND ); }

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//--------------------------------------------------------- class ExcDummy_03 -
// in Tabelle: SELECTION

class ExcDummy_03 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const UINT16     nMyLen;
public:
    virtual UINT16          GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//------------------------------------------------------- class ExcDimensions -

class ExcDimensions : public ExcRecord
{
private:
    UINT16                  nRwMic;
    UINT16                  nRwMac;
    UINT16                  nColMic;
    UINT16                  nColMac;
    BiffTyp                 eBiff;
    void                    SaveCont( SvStream& );
public:
                            ExcDimensions( BiffTyp );
                            ExcDimensions( UINT16 nFirstCol, UINT16 nFirstRow,
                                UINT16 nLastCol, UINT16 nLastRow, BiffTyp );
    void                    SetLimits( UINT16 nFirstCol, UINT16 nFirstRow,
                                UINT16 nLastCol, UINT16 nLastRow );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//--------------------------------------------------------- class ExcEOutline -

class ExcEOutline
{
private:
    ScOutlineArray*             pOLArray;
    UINT16                      nCurrExcLevel;
    BOOL                        bIsColl;
    UINT16                      nEnd[ SC_OL_MAXDEPTH ];
    BOOL                        bHidden[ SC_OL_MAXDEPTH ];

protected:
public:
                                ExcEOutline( ScOutlineArray* pArray );

    void                        Update( UINT16 nNum );

    inline BOOL                 IsCollapsed() const     { return bIsColl; }
    inline UINT16               GetLevel() const
                                    { return Min( nCurrExcLevel, (UINT16) EXC_OUTLINE_MAX ); }
};


//------------------------------------------------------------ class ExcEGuts -

class ExcEGuts : public ExcRecord
{
private:
    UINT16                      nRowLevel;
    UINT16                      nColLevel;

    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
                                ExcEGuts( ScOutlineArray* pCol, ScOutlineArray* pRow );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


//-------------------------------------------------------------- class ExcRow -

class ExcRow : public ExcRecord
{
private:
    UINT16                  nNum;
    UINT16                  nFirstCol;
    UINT16                  nLastCol;
    UINT16                  nHeight;
    UINT16                  nOptions;
    UINT16                  nXF;
    BOOL                    bDefHeight;

    void                    SetRange( UINT16 nFCol, UINT16 nLCol );
    void                    SetHeight( UINT16 nNewHeight, BOOL bUser );

    void                    SaveCont( SvStream& rStrm );

protected:
public:
                            ExcRow( UINT16 nNum, UINT16 nTab, UINT16 nFCol, UINT16 nLCol,
                                UINT16 nXF, ScDocument& rDoc, ExcEOutline& rOutline );

    inline BOOL             IsDefault();

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};

inline BOOL ExcRow::IsDefault()
{
    return (TRUEBOOL( nHeight & EXC_ROW_FLAGDEFHEIGHT ) && !nOptions);
}



class ExcRowBlock : public ExcRecord
{
private:
    ExcRow**                ppRows; // 32 Rows en Block
    UINT16                  nNext;

protected:
public:
                            ExcRowBlock();
                            ~ExcRowBlock();

    virtual void            Save( SvStream& rOut );

    ExcRowBlock*            Append( ExcRow* pNewRow );
                                // liefert neuen Block, wenn alter voll, ansonsten NULL
    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};


//------------------------------------------------------ class ExcDefcolwidth -

class ExcDefcolwidth : public ExcRecord
{
private:
    UINT16                  nWidth;
    void                    SaveCont( SvStream& );
public:
                            ExcDefcolwidth( UINT16 nDefColWidth );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//---------------------------------------------------------- class ExcColinfo -

class ExcColinfo : public ExcRecord
{
private:
    UINT16                  nFirstCol;
    UINT16                  nLastCol;
    UINT16                  nColWidth;
    UINT16                  nXF;
    UINT16                  nOptions;
    //----------------------------------------------------------------
    void                    SaveCont( SvStream& );
public:
                            ExcColinfo( UINT16 nCol, UINT16 nTab, UINT16 nXF, RootData&, ExcEOutline& rOutline );
    void                    SetWidth( UINT16 nWidth, double fColScale );

    BOOL                    Expand( ExcColinfo* pExp );
                                // wenn sich this mit pExp erweitern laesst, wird pExp
                                // GELOESCHT, return = TRUE!, ansonsten muss pExp als
                                // Neuer eingefuegt werden wenn return = FALSE

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//--------------------------------------------------------------- class ExcXf -

class ExcXf : public ExcRecord
{
protected:
    static ExcPalette2*     pPalette2;

    UINT16                  nIfnt;
    UINT16                  nIfmt;

    UINT16                  nOffs8;

    UINT32                  nIcvForeSer;
    UINT32                  nIcvBackSer;
    UINT16                  nFls;

    UINT32                  nIcvTopSer;
    UINT32                  nIcvBotSer;
    UINT32                  nIcvLftSer;
    UINT32                  nIcvRigSer;

    UINT16                  nDgTop;
    UINT16                  nDgBottom;
    UINT16                  nDgLeft;
    UINT16                  nDgRight;

    ExcHorizAlign           eAlc;
    ExcVertAlign            eAlcV;
    ExcTextOrient           eOri;
    BOOL                    bFWrap;
    BOOL                    bFSxButton;

    BOOL                    bStyle;

    BOOL                    bAtrNum;
    BOOL                    bAtrFnt;
    BOOL                    bAtrAlc;
    BOOL                    bAtrBdr;
    BOOL                    bAtrPat;
    BOOL                    bAtrProt;

#ifdef DBG_UTIL
    static UINT16           nObjCnt;
#endif
    //----------------------------------------------------------------
    virtual void            SaveCont( SvStream& );
public:
                            ExcXf( UINT16 nFont, UINT16 nForm, const ScPatternAttr*, BOOL& rbLineBreak,
                                    BOOL bStyle = FALSE );
                                // rbLineBreak = TRUE erzwingt Wrap,
                                // return von rbLineBreak enthaelt immer tatsaechliches Wrap
#ifdef DBG_UTIL
    virtual                 ~ExcXf();
#endif

    static void             SetPalette( ExcPalette2& rPalette2 );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;

    static void             ScToExcBorderLine( const SvxBorderLine*, UINT32& rIcvSer, UINT16& rDg );
};


//----------------------------------------------------------- class ExcFormat -

class UsedFormList;
class SvNumberFormatter;

class ExcFormat : public ExcRecord, ExcRoot
{
private:
    friend                      UsedFormList;

    UINT16                      nIndex;     // Excel-Index
    UINT32                      nScIndex;   // ...
    String*                     pForm;
    BYTE                        nFormLen;
    BiffTyp                     eBiff;
    static SvNumberFormatter*   pFormatter;
    static UINT32               nObjCnt;
    //----------------------------------------------------------------
    void                    SaveCont( SvStream& );
public:
                            ExcFormat( RootData*, UINT32 nScIndex );
    virtual                 ~ExcFormat();

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};



//--------------------------------------------------------- class ExcPalette2 -
// a new class for PALETTE record, supports color reduction

class ExcPal2Entry : private Color
{
private:
    UINT32                  nSerial;
    UINT32                  nWeight;

protected:
public:
    inline                  ExcPal2Entry( const Color& rCol, UINT32 nSer );

                            Color::GetRed;
                            Color::GetGreen;
                            Color::GetBlue;
                            Color::GetColor;
    inline UINT32           GetSerial() const       { return nSerial; }
    inline UINT32           GetWeighting() const    { return nWeight; }
    inline BOOL             IsGreater( const Color& rCol ) const;
    inline BOOL             IsEqual( const Color& rCol ) const;

    void                    UpdateEntry( UINT16 nColorType );
    inline void             AddWeighting( const ExcPal2Entry& rEntry );
    void                    AddColor( const ExcPal2Entry& rEntry );

    void                    Save( SvStream& rStrm );
};

inline ExcPal2Entry::ExcPal2Entry( const Color& rCol, UINT32 nSer ) :
        Color( rCol ),
        nSerial( nSer ),
        nWeight( 0 )
{   }

inline BOOL ExcPal2Entry::IsGreater( const Color& rCol ) const
{
    return mnColor > rCol.GetColor();
}

inline BOOL ExcPal2Entry::IsEqual( const Color& rCol ) const
{
    return mnColor == rCol.GetColor();
}

inline void ExcPal2Entry::AddWeighting( const ExcPal2Entry& rEntry )
{
    nWeight += rEntry.GetWeighting();
}



class ExcPalette2 : private List, public ExcRecord
{
private:
    ColorBuffer&            rColBuff;       // defaults
    UINT32                  nLastInd;
    UINT32                  nMaxSerial;
    UINT32*                 pColorIndex;
    Color*                  pColors;

    inline ExcPal2Entry*    _First()    { return (ExcPal2Entry*) List::First(); }
    inline ExcPal2Entry*    _Next()     { return (ExcPal2Entry*) List::Next(); }
    inline ExcPal2Entry*    _Get( UINT32 nIndex ) const;

    void                    SearchEntry( const Color& rCol, UINT32& nIndex, BOOL& bIsEqual ) const;
    ExcPal2Entry*           CreateEntry( const Color& rCol, UINT32 nIndex );
    void                    RecalcColorIndex( UINT32 nKeep, UINT32 nRemove );
    void                    MergeColors( UINT32 nKeep, UINT32 nRemove );
    UINT32                  GetRemoveColor() const;
    INT32                   GetNearColors( const Color& rCol, UINT32& rFirst, UINT32& rSecond ) const;
    UINT32                  GetNearestColor( const Color& rCol, UINT32 nIgnore ) const;
    UINT32                  GetNearestColor( UINT32 nIndex ) const;

    virtual void            SaveCont( SvStream& rStrm );

protected:
public:
                            ExcPalette2( ColorBuffer& rCB );
    virtual                 ~ExcPalette2();

    UINT32                  InsertColor( const Color& rCol, UINT16 nColorType );
    UINT32                  InsertIndex( UINT16 nIndex );
    void                    ReduceColors();
    UINT16                  GetColorIndex( const Color& rCol ) const;
    UINT16                  GetColorIndex( UINT32 nSerial ) const;
    void                    GetMixedColors( UINT32 nForeSer, UINT32 nBackSer,
                                UINT16& rForeInd, UINT16& rBackInd, UINT16& rPatt ) const;
    ColorData               GetRGBValue( UINT16 nIndex ) const;

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};

inline ExcPal2Entry* ExcPalette2::_Get( UINT32 nIndex ) const
{
    return (ExcPal2Entry*) List::GetObject( nIndex );
}



//------------------------------------------------------ class ExcExterncount -

class ExcExterncount : public ExcRecord, ExcRoot
{
private:
    //----------------------------------------------------------------
    void                    SaveCont( SvStream& );
    BOOL                    bTable;
public:
                            ExcExterncount( RootData*, const BOOL bTable );
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//------------------------------------------------------ class ExcExternsheet -

class ExcExternsheet : public ExcRecord, ExcRoot
{
private:
    String                  aTabName;
    //----------------------------------------------------------------
    void                    SaveCont( SvStream& );
public:
                            ExcExternsheet( RootData* pRD, const UINT16 nTabNum );
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//-------------------------------------------------- class ExcExternsheetList -

class ExcExternsheetList : public ExcRecord, protected List
{
private:
protected:
    virtual void            _Save( SvStream& );
public:
    virtual                 ~ExcExternsheetList();
    inline void             Add( ExcExternsheet* pNew ) { List::Insert( pNew, LIST_APPEND ); }

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//-------------------------------------------------------- class ExcExternDup -

class ExcExternDup : public ExcRecord
{
private:
    ExcExterncount&         rExtCnt;
    ExcExternsheetList&     rExtSheetList;
protected:
    virtual void            _Save( SvStream& );
public:
                            ExcExternDup( ExcExterncount&, ExcExternsheetList& );
                            ExcExternDup( const ExcExternDup& );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//---------------------------------------------------------- class ExcWindow2 -

class ExcWindow2 : public ExcRecord
{
private:
    UINT16                  nTable;
    //----------------------------------------------------------------
    void                    SaveCont( SvStream& );
public:
                            ExcWindow2( UINT16 nTable );
    inline  UINT16          GetTable() const    { return nTable; }
    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};


//------------------------------------------------------------ class UsedList -

class UsedList : public List, public ExcRecord
{
private:
    virtual void            SaveCont( SvStream& );
protected:
    void                    _Save( SvStream& );
                            // musste ueberladen werden, da sonst nicht
                            //  ohne Inhalt geschrieben werden kann!
    UINT16                  nBaseIndex;
    UINT16                  nLen;
public:
                            UsedList( void );
    void                    SetBaseIndex( UINT16 nNewVal );
                            // um auf richtigen Excel-Index 'trimmen'
                            //  zu koennen
    virtual UINT16          GetLen( void ) const;
};


//-------------------------------------------------------- class UsedFontList -

class UsedFontList : public UsedList
{
private:
    BiffTyp                 eBiff;

    inline ExcFont*         _First()    { return (ExcFont*) List::First(); }
    inline ExcFont*         _Next()     { return (ExcFont*) List::Next(); }

public:
                            UsedFontList( BiffTyp eBiff );
    virtual                 ~UsedFontList();

    BOOL                    Find( ExcFont* pExcFont, UINT16& rIndex );
    UINT16                  Add( ExcFont* pExcFont );
    UINT16                  Add( Font* pFont );

    virtual UINT16          GetNum( void ) const;
};


//-------------------------------------------------------- class UsedFormList -

class UsedFormList : public UsedList
{
private:
public:
                            ~UsedFormList();
    UINT16                  Add( ExcFormat* pFormat );

    virtual UINT16          GetNum( void ) const;
};


//-------------------------------------------------------- class UsedAttrList -
// entspricht einer Ansammlung XF-Records

class UsedAttrList : public UsedList, ExcRoot
{
private:
    struct ENTRY
    {
        const ScPatternAttr*    pPattAttr;
        ExcXf*              pXfRec;
        BOOL                bLineBreak;
        ULONG               nAltNumForm;

        inline              ENTRY( void ) : nAltNumForm( NUMBERFORMAT_ENTRY_NOT_FOUND ) {}

        inline BOOL         Equal( const ScPatternAttr* p, const ULONG n ) const
                            {
                                return pPattAttr == p && nAltNumForm == n;
                            }
    };

    ExcPalette2&            rPalette2;
    UsedFontList&           rFntLst;
    UsedFormList&           rFrmLst;
    void                    SaveCont( SvStream& );  // ... ueberladen, da List
                                // nicht direkt ExcRecord-Pointer enthaelt!
    void                    AddNewXF( const ScPatternAttr* pAttr,
                                const BOOL bStyle, const BOOL bExplLineBreak,
                                const ULONG nAltNumForm = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                BOOL bForceAltNumForm = FALSE );
public:
                            UsedAttrList( RootData* pRD, ExcPalette2&,
                                UsedFontList&, UsedFormList& );
    virtual                 ~UsedAttrList();
    UINT16                  Find( const ScPatternAttr* pSearch, const BOOL bStyle = FALSE,
                                    const ULONG nAltNumForm = NUMBERFORMAT_ENTRY_NOT_FOUND,
                                    BOOL bForceAltNumForm = FALSE );
    UINT16                  FindWithLineBreak( const ScPatternAttr* pSearch );

    virtual UINT16          GetNum( void ) const;
};




class ExcSetup : public ExcRecord
{
private:
    UINT16                  nPaperSize;
    UINT16                  nScale;
    UINT16                  nPageStart;
    UINT16                  nGrbit;

    void                    SaveCont( SvStream& );
public:
                            ExcSetup( RootData* );

    virtual UINT16          GetNum( void ) const;
    virtual UINT16          GetLen( void ) const;
};




class ExcHeaderFooter : public ExcRecord, ExcRoot
{
protected:
    String                  aFormatString;
    UINT16                  nLen;
    BOOL                    bUnicode;
    virtual void            _Save( SvStream& );
    String                  GetFormatString( USHORT nWhich );
    String                  GetFormatString( const ScPageHFItem& );
    String                  GetFormatString( const EditTextObject& );
public:
                            ExcHeaderFooter( RootData*, const BOOL bUnicode );

    virtual UINT16          GetLen( void ) const;
};




class ExcHeader : public ExcHeaderFooter
{
protected:
public:
                            ExcHeader( RootData*, const BOOL bUnicode );

    virtual UINT16          GetNum( void ) const;
};




class ExcFooter : public ExcHeaderFooter
{
protected:
public:
                            ExcFooter( RootData*, const BOOL bUnicode );

    virtual UINT16          GetNum( void ) const;
};




class ExcBoolRecord : public ExcRecord
{
    // stores BOOL as 16bit val ( 0x0000 | 0x0001 )
private:
    void                    SaveCont( SvStream& );
    BOOL                    bVal;
public:
    inline                  ExcBoolRecord( const BOOL bDefault ) : bVal( bDefault ) {}
                            ExcBoolRecord( SfxItemSet*, USHORT nWhich, BOOL bDefault );

    virtual UINT16          GetLen( void ) const;
};




class ExcPrintheaders : public ExcBoolRecord
{
private:
public:
                            ExcPrintheaders( SfxItemSet* );

    virtual UINT16          GetNum( void ) const;
};




class ExcPrintGridlines : public ExcBoolRecord
{
private:
public:
                            ExcPrintGridlines( SfxItemSet* );

    virtual UINT16          GetNum( void ) const;
};




class ExcHcenter : public ExcBoolRecord
{
private:
public:
                            ExcHcenter( SfxItemSet* );

    virtual UINT16          GetNum( void ) const;
};




class ExcVcenter : public ExcBoolRecord
{
private:
public:
                            ExcVcenter( SfxItemSet* );

    virtual UINT16          GetNum( void ) const;
};



//___________________________________________________________________
// AutoFilter

class ExcFilterMode : public ExcRecord
{
private:
    virtual void            SaveCont( SvStream& rStrm );

protected:
public:
    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};



class ExcAutoFilterInfo : public ExcRecord
{
private:
    UINT16                  nCount;

    virtual void            SaveCont( SvStream& rStrm );

protected:
public:
    inline                  ExcAutoFilterInfo( UINT16 nC )  { nCount = nC; }
    virtual                 ~ExcAutoFilterInfo();

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};



class ExcFilterCondition
{
private:
    UINT8                   nType;
    UINT8                   nOper;
    double                  fVal;
    XclRawUnicodeString*    pText;

protected:
public:
                            ExcFilterCondition();
                            ~ExcFilterCondition();

    inline BOOL             IsEmpty() const     { return (nType == EXC_AFTYPE_NOTUSED); }
    inline BOOL             HasEqual() const    { return (nOper == EXC_AFOPER_EQUAL); }
    UINT16                  GetTextBytes() const;

    void                    SetCondition( UINT8 nTp, UINT8 nOp, double fV, String* pT );

    void                    Save( SvStream& rStrm );
    void                    SaveText( SvStream& rStrm );
};



class ExcAutoFilter : public ExcRecord
{
private:
    UINT16                  nCol;
    UINT16                  nFlags;
    ExcFilterCondition      aCond[ 2 ];

    BOOL                    AddCondition( ScQueryConnect eConn, UINT8 nType,
                                UINT8 nOp, double fVal, String* pText,
                                BOOL bSimple = FALSE );

    virtual void            SaveCont( SvStream& rStrm );

protected:
public:
                            ExcAutoFilter( UINT16 nC );

    inline UINT16           GetCol() const          { return nCol; }
    inline BOOL             HasCondition() const    { return !aCond[ 0 ].IsEmpty(); }
    inline BOOL             HasTop10() const        { return TRUEBOOL( nFlags & EXC_AFFLAG_TOP10 ); }

    BOOL                    AddEntry( RootData& rRoot, const ScQueryEntry& rEntry );

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};



class ExcAutoFilterRecs : private List, public ExcEmptyRec
{
private:
    ExcFilterMode*          pFilterMode;
    ExcAutoFilterInfo*      pFilterInfo;

    inline ExcAutoFilter*   _First()    { return (ExcAutoFilter*) List::First(); }
    inline ExcAutoFilter*   _Next()     { return (ExcAutoFilter*) List::Next(); }
    ExcAutoFilter*          GetByCol( UINT16 nCol );    // always 0-based

    BOOL                    IsFiltered( UINT16 nCol );

    void                    DeleteList();
    inline void             Append( ExcAutoFilter* pFilter )
                                { List::Insert( pFilter, LIST_APPEND ); }
    void                    AddObjRecs( RootData& rRoot, const ScAddress& rPos, UINT16 nCols );
protected:
public:
                            ExcAutoFilterRecs( RootData& rRoot, UINT16 nTab );
    virtual                 ~ExcAutoFilterRecs();

    virtual void            Save( SvStream& rStrm );
};



//___________________________________________________________________
// Margins

class ExcMargin : public ExcRecord
{
private:
    UINT16                  nVal;
    UINT16                  nId;

    virtual void            SaveCont( SvStream& rStrm );
protected:
public:
                            ExcMargin( long nMargin, IMPEXC_MARGINSIDE eSide );

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};


//___________________________________________________________________
// Page breaks

class XclExpPageBreaks : public ExcRecord
{
private:
    UINT16                  nRecNum;

    virtual void            SaveCont( SvStream& rStrm );

protected:
    UINT16List              aPageBreaks;

public:
    enum ExcPBOrientation   { pbHorizontal, pbVertical };

                            XclExpPageBreaks( RootData& rRootData, UINT16 nScTab, ExcPBOrientation eOrient );
    virtual                 ~XclExpPageBreaks();

    virtual void            Save( SvStream& rStrm );
    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};


//___________________________________________________________________
// multiple operations aka table operations (record TABLE)

// a multiple operations record
// stores pointers to all affected formula records
class XclExpTableOp : private List, public ExcRecord
{
private:
    USHORT                  nFirstCol;
    USHORT                  nLastCol;
    USHORT                  nNextCol;           // next column of next row
    USHORT                  nFirstRow;
    USHORT                  nLastRow;
    USHORT                  nMode;
    USHORT                  nColInpCol;
    USHORT                  nColInpRow;
    USHORT                  nRowInpCol;
    USHORT                  nRowInpRow;
    BOOL                    bIsValid;

    inline ExcFormula*      _First()    { return (ExcFormula*) List::First(); }
    inline ExcFormula*      _Next()     { return (ExcFormula*) List::Next(); }

    inline void             Append( ExcFormula* pFmla ) { List::Insert( pFmla, LIST_APPEND ); }

    virtual void            SaveCont( SvStream& rStrm );

public:
                            XclExpTableOp(
                                ExcFormula& rFormula,
                                const ScAddress& rColFirstPos,
                                const ScAddress& rRowFirstPos,
                                USHORT nNewMode );
    virtual                 ~XclExpTableOp();

    BOOL                    IsAppendable( const ScAddress& rPos );

    BOOL                    CheckPosition(
                                const ScAddress& rPos,
                                const ScAddress& rFmlaPos,
                                const ScAddress& rColFirstPos, const ScAddress& rColRelPos,
                                const ScAddress& rRowFirstPos, const ScAddress& rRowRelPos,
                                BOOL bMode2 );

    static BOOL             CheckFirstPosition(
                                const ScAddress& rPos,
                                const ScAddress& rFmlaPos,
                                const ScAddress& rColFirstPos, const ScAddress& rColRelPos,
                                const ScAddress& rRowFirstPos, const ScAddress& rRowRelPos,
                                BOOL bMode2, USHORT& rnMode );

                            // insert pointer to Formula rec and update range data
    void                    InsertCell( ExcFormula& rFormula );
                            // change #NA error values of formula recs to TableOp values if in table op range
    void                    UpdateCells();

    virtual void            Save( SvStream& rStrm );

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};

// stores pointers to ExcTableOp records - insert cells to existing or new ExcTableOp
class XclExpTableOpManager : private List
{
private:
    inline XclExpTableOp*   _First()    { return (XclExpTableOp*) List::First(); }
    inline XclExpTableOp*   _Next()     { return (XclExpTableOp*) List::Next(); }

public:
    inline                  XclExpTableOpManager() : List() {}
    virtual                 ~XclExpTableOpManager();

                            // create & return new TableOp record or insert to an existing
    XclExpTableOp*          InsertCell( const ScTokenArray* pTokenArray, ExcFormula& rFormula );
                            // change #NA error values of formula recs to TableOp values
    void                    UpdateCells();
};

#endif

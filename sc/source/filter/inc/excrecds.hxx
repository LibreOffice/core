/*************************************************************************
 *
 *  $RCSfile: excrecds.hxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:38:12 $
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


#include <vector>

#ifndef SC_OUTLINETAB_HXX
#include "olinetab.hxx"
#endif
#ifndef SC_FILTER_HXX
#include "filter.hxx"
#endif
#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
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
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif

//------------------------------------------------------------------ Forwards -

class SvxBorderLine;

class SvStream;
class XclExpStream;
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
class ScProgress;

class ExcTable;
class UsedAttrList;
class ExcArray;
class ExcArrays;
class ExcShrdFmla;
class ExcUPN;

//----------------------------------------------------------- class ExcRecord -

class ExcRecord : public XclExpRecord
{
public:
    virtual void            Save( XclExpStream& rStrm );

    virtual UINT16          GetNum() const = 0;
    virtual ULONG           GetLen() const = 0;

protected:
    virtual void            SaveCont( XclExpStream& rStrm );

private:
    /** Writes the body of the record. */
    virtual void            WriteBody( XclExpStream& rStrm );
};


//--------------------------------------------------------- class ExcEmptyRec -

class ExcEmptyRec : public ExcRecord
{
private:
protected:
public:
    virtual void            Save( XclExpStream& rStrm );
    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};


//------------------------------------------------------- class ExcRecordList -

class ExcRecordList : protected List, public ExcEmptyRec
{
private:
protected:
public:
    virtual                 ~ExcRecordList();

                            List::Count;

    inline ExcRecord*       First( void )               { return ( ExcRecord* ) List::First(); }
    inline ExcRecord*       Next( void )                { return ( ExcRecord* ) List::Next(); }

    inline void             Append( ExcRecord* pNew )   { if( pNew ) List::Insert( pNew, LIST_APPEND ); }
    inline const ExcRecord* Get( UINT32 nNum ) const    { return ( ExcRecord* ) List::GetObject( nNum ); }

    virtual void            Save( XclExpStream& rStrm );
};


//--------------------------------------------------------- class ExcDummyRec -

class ExcDummyRec : public ExcRecord
{
protected:
public:
    virtual void            Save( XclExpStream& rStrm );
    virtual UINT16          GetNum() const;
    virtual const BYTE*     GetData() const = 0;    // byte data must contain header and body
};


//------------------------------------------------------- class ExcBoolRecord -
// stores BOOL as 16bit val ( 0x0000 | 0x0001 )

class ExcBoolRecord : public ExcRecord
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );

protected:
    BOOL                    bVal;

    inline                  ExcBoolRecord() : bVal( FALSE ) {}

public:
    inline                  ExcBoolRecord( const BOOL bDefault ) : bVal( bDefault ) {}
                            ExcBoolRecord( SfxItemSet*, USHORT nWhich, BOOL bDefault );

    virtual ULONG           GetLen( void ) const;
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


//-------------------------------------------------------------- class ExcBof -
// Header Record fuer WORKSHEETS

class ExcBof : public ExcBof_Base
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcBof( void );

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//------------------------------------------------------------- class ExcBofW -
// Header Record fuer WORKBOOKS

class ExcBofW : public ExcBof_Base
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcBofW( void );

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//-------------------------------------------------------------- class ExcEof -

class ExcEof : public ExcRecord
{
private:
public:
    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//----------------------------------------------------- class ExcFngroupcount -

class ExcFngroupcount : public ExcRecord
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );
public:
    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//--------------------------------------------------------- class ExcDummy_00 -
// INTERFACEHDR to FNGROUPCOUNT (see excrecds.cxx)

class ExcDummy_00 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};

// EXC_ID_WINDOWPROTECTION
class XclExpWindowProtection : public   XclExpBoolRecord
{
    public:
        XclExpWindowProtection(bool bValue);
};

// EXC_ID_PROTECT  Document Protection
class XclExpDocProtection : public  XclExpBoolRecord
{
    public:
        XclExpDocProtection(bool bValue);
};


//-------------------------------------------------------- class ExcDummy_04x -
// PASSWORD to BOOKBOOL (see excrecds.cxx), no 1904

class ExcDummy_040 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};



class ExcDummy_041 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//------------------------------------------------------------- class Exc1904 -

class Exc1904 : public ExcBoolRecord
{
public:
                            Exc1904( ScDocument& rDoc );
    virtual UINT16          GetNum( void ) const;
};


//------------------------------------------------------ class ExcBundlesheet -

class ExcBundlesheetBase : public ExcRecord
{
protected:
    ULONG                   nStrPos;
    ULONG                   nOwnPos;    // Position NACH # und Len
    UINT16                  nGrbit;

                            ExcBundlesheetBase();

public:
                            ExcBundlesheetBase( RootData& rRootData, UINT16 nTab );

    inline void             SetStreamPos( ULONG nNewStrPos ) { nStrPos = nNewStrPos; }
    void                    UpdateStreamPos( XclExpStream& rStrm );

    virtual UINT16          GetNum() const;
};



class ExcBundlesheet : public ExcBundlesheetBase
{
private:
    ByteString              aName;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcBundlesheet( RootData& rRootData, UINT16 nTab );
    virtual ULONG           GetLen() const;
};

//--------------------------------------------------------- class ExcDummy_02 -
// sheet dummies: CALCMODE to SETUP

class ExcDummy_02a : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


// sheet dummies: DEFAULTROWHEIGHT
class ExcDummy_02c : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


// ----------------------------------------------------------------------------

/** This record contains the Windows country IDs for the UI and document language. */
class XclExpCountry : public XclExpRecord
{
public:
    explicit                    XclExpCountry( const XclExpRoot& rRoot );

private:
    sal_uInt16                  mnUICountry;        /// The UI country ID.
    sal_uInt16                  mnDocCountry;       /// The document country ID.

    /** Writes the body of the COUNTRY record. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


//------------------------------------------------------------- class ExcNote -

class ExcNote : public ExcEmptyRec
{
private:
    ByteString*             pText;
    ScAddress               aPos;
    UINT16                  nTextLen;

public:
                            ExcNote( const ScAddress, const String& rText, RootData& );
    virtual                 ~ExcNote();

    virtual void            Save( XclExpStream& rStrm );
};


//------------------------------------------------------------- class ExcCell -

class ExcCell : public ExcRecord
{
protected:
    ScAddress               aPos;
    sal_uInt32              mnXFId;
    static UINT32           nCellCount;     // zaehlt DOPPELT: im Ctor und SaveCont
    static ScProgress*      pPrgrsBar;
#ifdef DBG_UTIL
    friend class ExcDocument;
    static INT32            _nRefCount;
#endif

                            ExcCell(
                                const ScAddress rPos,
                                const ScPatternAttr* pAttr,
                                RootData& rRootData,
                                const ULONG nAltNumForm = NUMBERFORMAT_ENTRY_NOT_FOUND );

    virtual void            SaveCont( XclExpStream& rStrm );
    virtual void            SaveDiff( XclExpStream& rStrm );
    virtual ULONG           GetDiffLen() const = 0;

public:
    virtual                 ~ExcCell();

    inline void             SetXFId( sal_uInt32 nXFId )   { mnXFId = nXFId; }
    virtual sal_uInt32      GetXFId() const;

    inline static void      ResetCellCount()        { nCellCount = 0; }
    inline static void      IncCellCount()          { nCellCount++; }
    inline static UINT32    GetCellCount()          { return nCellCount; }
    inline static void      SetPrgrsBar( ScProgress& rNewBar );
    inline static void      ClearPrgrsBar()         { pPrgrsBar = NULL; }

    virtual ULONG           GetLen() const;
};

inline void ExcCell::SetPrgrsBar( ScProgress& rNewBar )
{
    ResetCellCount();       // logisch... oder?
    pPrgrsBar = &rNewBar;
}


//----------------------------------------------------------- class ExcNumber -

class ExcNumber : public ExcCell
{
private:
    double                  fVal;

    virtual void            SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG           GetDiffLen( void ) const;

public:
                            ExcNumber(
                                const ScAddress,
                                const ScPatternAttr*,
                                RootData& rRootData,
                                const double& rVal );

    virtual UINT16          GetNum( void ) const;
};


//---------------------------------------------------------- class ExcBoolerr -

class ExcBoolerr : public ExcCell
{
private:
    UINT8                   nVal;
    UINT8                   bError;

    virtual void            SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG           GetDiffLen( void ) const;

public:
                            ExcBoolerr(
                                const ScAddress,
                                const ScPatternAttr*,
                                RootData& rRootData,
                                UINT8 nVal,
                                BOOL bIsError );

    virtual UINT16          GetNum( void ) const;
};


//---------------------------------------------------------- class ExcRKMulRK -

class ExcRKMulRK : public ExcCell
{
private:
    struct ExcRKMulRKEntry
    {
        sal_Int32           mnValue;
        sal_uInt32          mnXFId;
    };

    typedef ScfDelList< ExcRKMulRKEntry > ExcRKMulRKEntryList;

    ExcRKMulRKEntryList     maEntryList;

protected:
    virtual void            SaveCont( XclExpStream& rStrm );
    virtual ULONG           GetDiffLen( void ) const;

public:
                            ExcRKMulRK(
                                const ScAddress,
                                const ScPatternAttr*,
                                RootData& rRootData,
                                sal_Int32 nValue );

                            // returns new RK or NULL if an old RK was extendable
    ExcRKMulRK*             Extend( const ScAddress rPos,
                                const ScPatternAttr *pAttr,
                                RootData& rRootData,
                                sal_Int32 nValue );
    virtual sal_uInt32      GetXFId() const;

    virtual UINT16          GetNum( void ) const;

    inline BOOL             IsRK( void ) const      { return maEntryList.Count() == 1; }
    inline BOOL             IsMulRK( void ) const   { return maEntryList.Count() > 1; }
};


//------------------------------------------------------------ class ExcLabel -

class ExcLabel : public ExcCell
{
private:
    ByteString              aText;
    UINT16                  nTextLen;

    virtual void            SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG           GetDiffLen( void ) const;

public:
                            ExcLabel(
                                const ScAddress,
                                const ScPatternAttr*,
                                RootData& rRootData,
                                const String& rText );
    virtual                 ~ExcLabel();

    virtual UINT16          GetNum( void ) const;
};


//---------------------------------------------------------- class ExcRichStr

// helper class for ExcRString and ExcLabel8/XclExpRichString
class ExcRichStr
{
private:
    ScfUInt16List           aForms;     // Form und Pos nacheinander
    BiffTyp                 eBiff;
public:
                            ExcRichStr(
                                ExcCell&                rExcCell,
                                String&                 rText,
                                const ScPatternAttr*    pAttr,
                                const ScEditCell&       rEdCell,
                                RootData&               rRoot,
                                xub_StrLen              nMaxChars );
    inline                  ExcRichStr( const ExcRichStr& rCopy ) :
                                aForms( rCopy.aForms ), eBiff( rCopy.eBiff ) {}

                            ~ExcRichStr();

    inline  UINT16          GetFormCount() const;

    /** returns font idx and removes the List entry for a char index otherwise EXC_FONT_NOTFOUND from the aForms. */
    sal_uInt16                          RemoveFontOfChar(sal_uInt16 nCharIdx);

                            // number of bytes to be saved
    inline  ULONG           GetByteCount() const;

                            // write list of forms
    void                    Write( XclExpStream& rStrm );
};


inline UINT16 ExcRichStr::GetFormCount() const
{
    return (UINT16) Min( aForms.Count() / 2, (eBiff < Biff8 ? ULONG(0xFF) : ULONG(0xFFFF)) );
}


inline ULONG ExcRichStr::GetByteCount() const
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

    virtual void            SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG           GetDiffLen( void ) const;

public:
                            ExcRString(
                                const ScAddress         aPos,
                                const ScPatternAttr*    pAttr,
                                RootData&               rRootData,
                                const ScEditCell&       rEdCell );
    virtual                 ~ExcRString();

    virtual UINT16          GetNum( void ) const;
};

/*----------------------------------------------------------------------*/

class ExcFmlaResultStr : public XclExpRecord
{
private:
    XclExpString maResultText;

public:

    ExcFmlaResultStr(const XclExpString &aFmlaText);
    virtual ~ExcFmlaResultStr();

private:
    virtual void                WriteBody( XclExpStream& rStrm );
};

//---------------------------------------------------------- class ExcFormula -

class ExcFormula : public ExcCell
{
private:
    sal_Char*               pData;
    UINT16                  nFormLen;
    BOOL                    bShrdFmla;
    ScFormulaCell*          pFCell;

    virtual void            SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG           GetDiffLen( void ) const;

public:
                            ExcFormula(
                                const ScAddress rPos,
                                const ScPatternAttr *pAttr,
                                RootData& rRootData,
                                const ULONG nAltNumForm,
                                const ScTokenArray& rCode,
                                ExcArray** ppArray = NULL,
                                ScMatrixMode eMM = MM_NONE,
                                ExcShrdFmla** ppShrdFmla = NULL,
                                ExcArrays* pShrdFmlas = NULL,
                                ScFormulaCell* pFCell = NULL,
                                ExcFmlaResultStr **pFormulaResult = NULL);
                            ~ExcFormula();

    inline const ScAddress& GetPosition() const { return aPos; }    // from ExcCell

    void                    SetTableOp( USHORT nCol, USHORT nRow ); // for TableOp export

    virtual UINT16          GetNum( void ) const;

    static  BYTE ScErrorCodeToExc(UINT16 nErrorCode);
};


//---------------------------------------------------- class ExcBlankMulblank -

class ExcBlankMulblank : public ExcCell
{
protected:
    struct XclExpBlankCell
    {
        sal_uInt32              mnXFId;
        sal_uInt16              mnCount;
        inline explicit         XclExpBlankCell() : mnXFId( 0 ), mnCount( 0 ) {}
        inline explicit         XclExpBlankCell( sal_uInt32 nXFId, sal_uInt16 nCount ) :
                                    mnXFId( nXFId ), mnCount( nCount ) {}
    };

    typedef ::std::vector< XclExpBlankCell > XclExpBlankCellVec;

    XclExpBlankCellVec      maCellList;
    ULONG                   nRecLen;
    UINT16                  nLastCol;
    BOOL                    bMulBlank;
    BOOL                    bDummy;     // not saved, 'cause row contains formatting info

    inline sal_uInt32       GetXFId( UINT32 nEntry ) const    { return (UINT16) nEntry; }
    inline UINT16           GetCount( UINT32 nEntry ) const { return (UINT16)(nEntry >> 16); }

    inline void             Append( sal_uInt32 nXFId, sal_uInt16 nCount );

    void                    AddEntries(
                                const ScAddress rPos,
                                const ScPatternAttr* pAttr,
                                RootData& rRootData,
                                UINT16 nCount,
                                ExcTable& rExcTab );

    virtual void            SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG           GetDiffLen( void ) const;

public:
                            ExcBlankMulblank(
                                const ScAddress rPos,
                                const ScPatternAttr* pFirstAttr,
                                RootData& rRootData,
                                UINT16 nFirstCount,
                                ExcTable& rExcTab );

    void                    Add(
                                const ScAddress rPos,
                                const ScPatternAttr* pAttr,
                                RootData& rRootData,
                                UINT16 nAddCount,
                                ExcTable& rExcTab );

    inline UINT16           GetLastCol() const  { return nLastCol; }
    virtual sal_uInt32      GetXFId() const;                      // returns last used XF

    virtual UINT16          GetNum() const;

    virtual void            Save( XclExpStream& );              // for dummy case
};


inline void ExcBlankMulblank::Append( sal_uInt32 nXFId, sal_uInt16 nCount )
{
    maCellList.push_back( XclExpBlankCell( nXFId, nCount ) );
}




//---------------------------------------------------- class ExcNameListEntry -

class ExcNameListEntry : public ExcRecord
{
protected:
    UINT8*                  pData;
    UINT16                  nFormLen;

    UINT16                  nTabNum;            // Excel index, 1-based, 0==none
    UINT8                   nBuiltInKey;

    BOOL                    bDummy;

    void                    DeleteData();
    void                    SetCode( const ExcUPN& rUPN );

                            // default: save builtin key
    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcNameListEntry();
                            ExcNameListEntry( RootData& rRootData, UINT16 nScTab, UINT8 nKey );
    virtual                 ~ExcNameListEntry();

    inline UINT16           GetTabIndex() const     { return nTabNum; }
    inline UINT8            GetBuiltInKey() const   { return nBuiltInKey; }
    inline BOOL             IsDummy() const         { return bDummy; }

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};


//------------------------------------------------------------- class ExcName -

class ExcName : public ExcNameListEntry, public ExcRoot
{
private:
    String                  aName;
    BiffTyp                 eBiff;
    BOOL                    bHidden;
    BOOL                    bBuiltIn;

    void                    Init( BOOL bHid = FALSE, BOOL bBIn = FALSE );
    void                    BuildFormula( const ScRange& rRange );

    void                    SetName( const String& rRangeName );
    void                    SetUniqueName( const String& rRangeName );
    BOOL                    SetBuiltInName( const String& rName, UINT8 nKey );
    BOOL                    IsBuiltInAFName( const String& rName, UINT8 nKey );

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcName( RootData& rRootData, ScRangeData* pRange );
                            ExcName( RootData& rRootData, ScDBData* pArea );
                            ExcName( RootData& rRootData, const ScRange& rRange,
                                    const String& rName );
                            ExcName( RootData& rRootData, const ScRange& rRange,
                                    UINT8 nKey, BOOL bHid = FALSE );

    inline const String&    GetName() const     { return aName; }

    virtual ULONG           GetLen() const;
};


// ---- class XclBuildInName -----------------------------------------

class XclBuildInName : public ExcNameListEntry
{
private:
    ScRangeList             aRL;

protected:
    inline void             Append( const ScRange& rNew )   { aRL.Append( rNew ); }
    void                    CreateFormula( RootData& rRootData );

public:
                            XclBuildInName( RootData& rRootData, UINT16 nScTab, UINT8 nKey );
};


// ---- class XclPrintRange, class XclTitleRange ---------------------

class XclPrintRange : public XclBuildInName
{
public:
                            XclPrintRange( RootData& rRootData, UINT16 nScTab );
};


class XclPrintTitles : public XclBuildInName
{
public:
                            XclPrintTitles( RootData& rRootData, UINT16 nScTab );
};


//--------------------------------------------------------- class ExcNameList -

class ExcNameList : public ExcEmptyRec, private List
{
private:
    ULONG                   nFirstPrintRangeIx;
    ULONG                   nFirstPrintTitleIx;
    ULONG                   nFirstOtherNameIx;
    ::std::vector< sal_uInt32 > maNextInsVec; /// List positions for next insertion for each sheet.

    inline ExcNameListEntry* _First()       { return (ExcNameListEntry*) List::First(); }
    inline ExcNameListEntry* _Next()        { return (ExcNameListEntry*) List::Next(); }
    inline ExcNameListEntry* _Get( ULONG nIndex ) const
                                            { return (ExcNameListEntry*) List::GetObject( nIndex ); }
    UINT16                  Append( ExcNameListEntry* pName );

public:
                            ExcNameList( RootData& rRootData );
    virtual                 ~ExcNameList();

    UINT16                  GetBuiltInIx( const ExcNameListEntry* pName );

    /** Inserts a named range in table name sort order. */
    void                    InsertSorted( RootData& rRootData, ExcNameListEntry* pName, sal_uInt16 nScTab );


    virtual void            Save( XclExpStream& rStrm );
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

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcDimensions( BiffTyp );
                            ExcDimensions( UINT16 nFirstCol, UINT16 nFirstRow,
                                UINT16 nLastCol, UINT16 nLastRow, BiffTyp );

    void                    SetLimits( UINT16 nFirstCol, UINT16 nFirstRow,
                                UINT16 nLastCol, UINT16 nLastRow );

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
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

    virtual void                SaveCont( XclExpStream& rStrm );

protected:
public:
                                ExcEGuts( ScOutlineArray* pCol, ScOutlineArray* pRow );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


//-------------------------------------------------------------- class ExcRow -

class ExcRow : public ExcRecord
{
private:
    friend class DefRowXFs;
    ExcTable&               rExcTab;
    UINT16                  nNum;
    UINT16                  nFirstCol;
    UINT16                  nLastCol;
    UINT16                  nHeight;
    UINT16                  nOptions;
    sal_uInt32              mnXFId;
    BOOL                    bDefHeight;

    void                    SetRange( UINT16 nFCol, UINT16 nLCol );
    void                    SetHeight( UINT16 nNewHeight, BOOL bUser );

    virtual void            SaveCont( XclExpStream& rStrm );

protected:
public:
                            ExcRow( UINT16 nNum, UINT16 nTab, UINT16 nFCol, UINT16 nLCol,
                                sal_uInt32 nXFId, ScDocument& rDoc, ExcEOutline& rOutline, ExcTable& rExcTab );

    inline BOOL             IsDefault();

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};

inline BOOL ExcRow::IsDefault()
{
    return (TRUEBOOL( nHeight & EXC_ROW_FLAGDEFHEIGHT ) && !nOptions);
}


//--------------------------------------------------------- class ExcRowBlock -

class ExcRowBlock : public ExcEmptyRec
{
private:
    ExcRow**                ppRows; // 32 rows per block
    UINT16                  nNext;

protected:
public:
                            ExcRowBlock();
    virtual                 ~ExcRowBlock();

                            // returns new block or NULL if last block not full
    ExcRowBlock*            Append( ExcRow* pNewRow );

    void                    SetDefXFs( DefRowXFs& rDefRowXFs );

    virtual void            Save( XclExpStream& rStrm );
};


// ============================================================================

/** Contains the column settings for a range of columns. */
class XclExpColinfo : public XclExpRecord, protected XclExpRoot
{
private:
    sal_uInt32              mnXFId;         /// The XF ID for column default format.
    sal_uInt16              mnFirstXclCol;  /// Index to first column.
    sal_uInt16              mnLastXclCol;   /// Index to last column.
    sal_uInt16              mnWidth;        /// Width of the column(s), Excel value.
    sal_uInt16              mnFlags;        /// Additional column flags.

public:
    explicit                XclExpColinfo(
                                const XclExpRoot& rRoot,
                                sal_uInt16 nScCol, sal_uInt16 nScTab, sal_uInt32 nXFId,
                                ExcEOutline& rOutline );

    /** Tries to expand this record with a new column.
        @descr  This can be done, if the new column has the same settings as all other columns.
        @return  true = Expansion was successful, no new COLINFO record is needed. */
    bool                    Expand(
                                sal_uInt16 nScCol, sal_uInt16 nScTab, sal_uInt32 nXFId,
                                ExcEOutline& rOutline );

private:
    /** Returns the Excel width of the passed Calc column. */
    sal_uInt16              GetWidth( sal_uInt16 nScCol, sal_uInt16 nScTab ) const;
    /** Returns the Excel option flags of the passed Calc column. */
    sal_uInt16              GetFlags( sal_uInt16 nScCol, sal_uInt16 nScTab, ExcEOutline& rOutline ) const;

    /** Writes the contents of this COLINFO record. */
    virtual void            WriteBody( XclExpStream& rStrm );
};


// ============================================================================
//------------------------------------------------------ class ExcExterncount -

class ExcExterncount : public ExcRecord, ExcRoot
{
private:
    BOOL                    bTable;

    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcExterncount( RootData*, const BOOL bTable );
    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//------------------------------------------------------ class ExcExternsheet -

class ExcExternsheet : public ExcRecord, public ExcRoot
{
private:
    String                  aTabName;

    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcExternsheet( RootData* pRD, const UINT16 nTabNum );
    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//-------------------------------------------------- class ExcExternsheetList -

class ExcExternsheetList : public ExcEmptyRec, protected List
{
private:
    inline ExcExternsheet*  _First()    { return (ExcExternsheet*) List::First(); }
    inline ExcExternsheet*  _Next()     { return (ExcExternsheet*) List::Next(); }

protected:
public:
    virtual                 ~ExcExternsheetList();
    inline void             Add( ExcExternsheet* pNew ) { List::Insert( pNew, LIST_APPEND ); }

    virtual void            Save( XclExpStream& rStrm );
};


//-------------------------------------------------------- class ExcExternDup -

class ExcExternDup : public ExcEmptyRec
{
private:
    ExcExterncount&         rExtCnt;
    ExcExternsheetList&     rExtSheetList;

protected:
public:
                            ExcExternDup( ExcExterncount&, ExcExternsheetList& );
                            ExcExternDup( const ExcExternDup& );

    virtual void            Save( XclExpStream& rStrm );
};


//---------------------------------------------------------- class ExcWindow2 -

class ExcWindow2 : public ExcRecord
{
private:
    UINT16                  nTable;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcWindow2( UINT16 nTable );
    inline  UINT16          GetTable() const    { return nTable; }

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//-------------------------------------------------------- class ExcSelection -

class ExcSelection : public ExcRecord
{
private:
    UINT16                  nCol;
    UINT16                  nRow;
    UINT8                   nPane;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
    inline                  ExcSelection( UINT16 _nCol, UINT16 _nRow, UINT8 _nPane ) :
                                nCol( _nCol ), nRow( _nRow ), nPane( _nPane ) {}

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


// XclExpWsbool ===============================================================

class XclExpWsbool : public XclExpUInt16Record
{
public:
    explicit                    XclExpWsbool( bool bFitToPages );
};


// ============================================================================
//---------------------------------------------------------------- AutoFilter -
// classes: ExcFilterMode, ExcAutoFilterInfo, ExcFilterCondition,
//          ExcAutoFilter, ExcAutoFilterRecs

class ExcFilterMode : public ExcRecord
{
public:
    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};



class ExcAutoFilterInfo : public ExcRecord
{
private:
    UINT16                  nCount;

    virtual void            SaveCont( XclExpStream& rStrm );

protected:
public:
    inline                  ExcAutoFilterInfo( UINT16 nC )  { nCount = nC; }
    virtual                 ~ExcAutoFilterInfo();

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};



class ExcFilterCondition
{
private:
    UINT8                   nType;
    UINT8                   nOper;
    double                  fVal;
    XclExpUniString*        pText;

protected:
public:
                            ExcFilterCondition();
                            ~ExcFilterCondition();

    inline BOOL             IsEmpty() const     { return (nType == EXC_AFTYPE_NOTUSED); }
    inline BOOL             HasEqual() const    { return (nOper == EXC_AFOPER_EQUAL); }
    ULONG                   GetTextBytes() const;

    void                    SetCondition( UINT8 nTp, UINT8 nOp, double fV, String* pT );

    void                    Save( XclExpStream& rStrm );
    void                    SaveText( XclExpStream& rStrm );
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

    virtual void            SaveCont( XclExpStream& rStrm );

protected:
public:
                            ExcAutoFilter( UINT16 nC );

    inline UINT16           GetCol() const          { return nCol; }
    inline BOOL             HasCondition() const    { return !aCond[ 0 ].IsEmpty(); }
    inline BOOL             HasTop10() const        { return TRUEBOOL( nFlags & EXC_AFFLAG_TOP10 ); }

    BOOL                    AddEntry( RootData& rRoot, const ScQueryEntry& rEntry );

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
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

    virtual void            Save( XclExpStream& rStrm );
};


//------------------------ class ExcArray, class ExcArrays, class ExcShrdFmla -

class ExcArray : public ExcRecord
{
protected:
    UINT32                  nID;
    UINT16                  nFirstRow;
    UINT16                  nLastRow;
    UINT8                   nFirstCol;
    UINT8                   nLastCol;
    sal_Char*               pData;
    UINT16                  nFormLen;

    void                    SetColRow( UINT8 nCol, UINT16 nRow, UINT32 nID = 0xFFFFFFFF );

    virtual void            SaveCont( XclExpStream& rStrm );

                            ExcArray( const sal_Char* pData, UINT16 nLen, UINT8 nCol, UINT16 nRow );
public:
                            ExcArray( const ExcUPN&, UINT8 nCol, UINT16 nRow );
                            ExcArray( UINT8 nCol, UINT16 nRow, UINT32 nID );
    virtual                 ~ExcArray();

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;

    BOOL                    AppendBy( const ExcArray& rExt );
                                // TRUE, if rEXt is touching given range and extend range
    BOOL                    AppendBy( UINT8 nStartCol, UINT16 nStartRow, UINT8 nEndCol, UINT16 nEndRow );
};




class ExcArrays : protected List
{
private:
protected:
public:
                            ExcArrays( void );
    virtual                 ~ExcArrays();

    BOOL                    Insert( ExcArray* pPossibleNewArrayFormula );
                                // insert only, if not already in array
                                // only ref in list, so if return is TRUE, do not delete _before_ using
                                //  Insert() the _last_ time!

    BOOL                    Extend( UINT8 nStartCol, UINT16 nStartRow, UINT8 nEndCol, UINT16 nEndRow );
                                // extend existing range, when start is base

    inline void             Append( ExcArray* );
};


inline void ExcArrays::Append( ExcArray* p )
{
    List::Insert( p, LIST_APPEND );
}




class ExcShrdFmla : public ExcArray
{
private:
//  ScRange                 aPos;
//  sal_Char*               pData;
//  UINT16                  nLen;

    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcShrdFmla( const sal_Char* pData, UINT16 nLen, const ScRange& rPos );
    virtual                 ~ExcShrdFmla();

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};



//--------------------------- class XclExpTableOp, class XclExpTableOpManager -
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

    virtual void            SaveCont( XclExpStream& rStrm );

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

    virtual void            Save( XclExpStream& rStrm );

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
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


/*************************************************************************
 *
 *  $RCSfile: ww8scan.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-24 14:01:34 $
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

#ifndef _WW8SCAN_HXX
#define _WW8SCAN_HXX


#ifndef LONG_MAX
#include <limits.h>
#endif
#ifndef _SAL_TYPES_H
#include <sal/types.h>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>        // UINTXX
#endif
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif


#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#ifndef WW8STRUC_HXX
#include <ww8struc.hxx>     // FIB, STSHI, STD
#endif

#ifndef DELETEZ
#define DELETEZ( p ) ( delete( p ), p = 0 )
#endif


//class SvStream;
class  UShortStk;
class  WW8Fib;
class  WW8ScannerBase;
class  WW8PLCFspecial;
struct WW8PLCFxDesc;
class  WW8PLCFx_PCD;
class  SvUShortsSort;


String WW8ReadPString( SvStream& rStrm, rtl_TextEncoding eEnc,
                       BOOL bAtEndSeekRel1 = TRUE );

// Folgende Methode liest einen 2-byter-UNICODE-String ein:
// - falls bAtEndSeekRel1 gesetzt ist, wird am Ende genau EIN Byte uebersprungen,
// - falls nChars (die Zeichen-Anzahl, NICHT die Byte-Anzahl) nicht gesetzt ist,
//   wird das erste USHORT als Lannengenangabe interpretiert,
//   ansonsten als erstes String-Zeichen genommen.
String WW8Read_xstz(SvStream& rStrm,
                    USHORT nChars,
                    BOOL   bAtEndSeekRel1);


// read array of strings (see MS documentation: STring TaBle stored in File)
// returns NOT the original pascal strings but an array of converted char*
//
// attention: the *extra data* of each string are SKIPPED and ignored

/*  to be optimized like this:    */
void WW8ReadSTTBF(  BOOL bVer8, SvStream& rStrm,
                    UINT32 nStart, INT32 nLen, USHORT nSkip,
                    rtl_TextEncoding eCS,
                    SvStrings &rArray, SvStrings* pExtraArray = 0 );



USHORT WW8GetSprmId(        BYTE nVersion, BYTE* pSp,   BYTE*   pDelta = 0 );
short WW8GetSprmSizeNetto(  BYTE nVersion, BYTE* pSprm, USHORT* pId );
short WW8GetSprmSizeBrutto( BYTE nVersion, BYTE* pSprm, USHORT* pId );
BYTE WW8SprmDataOfs( USHORT nId );

struct WW8FieldDesc
{
    long nLen;              // Gesamtlaenge ( zum Text ueberlesen )
    WW8_CP nSCode;           // Anfang Befehlscode
    long nLCode;            // Laenge
    WW8_CP nSRes;           // Anfang Ergebnis
    long nLRes;             // Laenge ( == 0, falls kein Ergebnis )
    USHORT nId;             // WW-Id fuer Felder
    BYTE nOpt;              // WW-Flags ( z.B.: vom User geaendert )
    BOOL bCodeNest:1;       // Befehl rekursiv verwendet
    BOOL bResNest:1;        // Befehl in Resultat eingefuegt
};

BOOL WW8GetFieldPara( BYTE nVersion, WW8PLCFspecial& rPLCF, WW8FieldDesc& rF );


struct WW8PLCFxSave1
{
    ULONG nPLCFxPos;
    ULONG nPLCFxPos2;       // fuer PLCF_Cp_Fkp: PieceIter-Pos
    long nPLCFxMemOfs;
    WW8_CP nAttrStart;
    WW8_CP nAttrEnd;
    BOOL   bLineEnd;
};


/*
    u.a. fuer Felder, also genausoviele Attr wie Positionen,
    falls Ctor-Param bNoEnd = false
*/
class WW8PLCFspecial        // Iterator fuer PLCFs
{
    INT32* pPLCF_PosArray;  // Pointer auf Pos-Array und auf ganze Struktur
    BYTE*  pPLCF_Contents;  // Pointer auf Inhalts-Array-Teil des Pos-Array
    long nIMax;         // Anzahl der Elemente
    long nIdx;          // Merker, wo wir gerade sind
    long nStru;

public:
    WW8PLCFspecial( SvStream* pSt, long nFilePos, long nPLCF,
          long nStruct, long nStartPos = -1, BOOL bNoEnd = FALSE );
    ~WW8PLCFspecial(){ delete( pPLCF_PosArray ); pPLCF_PosArray = 0; }
    ULONG GetIdx() const { return (ULONG)nIdx; }
    void SetIdx( ULONG nI ) { nIdx = (long)nI; }    // geht ueber Nummer
    long GetIMax(){ return nIMax; }
    BOOL SeekPos( long nPos );          // geht ueber FC- bzw. CP-Wert
                                                                    // bzw. naechste groesseren Wert
    BOOL SeekPosExact( long nPos );
    long Where() { return ( nIdx >= nIMax ) ? LONG_MAX : pPLCF_PosArray[nIdx]; }
    BOOL Get( long& rStart, void*& rpValue );
    BOOL GetData( long nIdx, long& rPos, void*& rpValue );

    const void* GetData( long nIdx ) const {
        return ( nIdx >= nIMax ) ? 0 : (const void*)&pPLCF_Contents[nIdx * nStru]; }
    long GetPos( long nIdx ){
        return ( nIdx >= nIMax ) ? LONG_MAX : pPLCF_PosArray[nIdx]; }

    WW8PLCFspecial& operator ++( int ) { nIdx++; return *this; }
    WW8PLCFspecial& operator --( int ) { nIdx--; return *this; }
};




/*
    simple Iterator for SPRMs
*/
class WW8SprmIter
{
    // these members will be updated
    BYTE*   pSprms;     // remaining part of the SPRMs ( == start of akt. SPRM)
    BYTE*   pAktParams; // start of akt. SPRM's parameters
    USHORT  nAktId;
    short   nAktSizeBrutto;

    // these members will *not* be updated by UpdateMyMembers()
    BYTE    nVersion;
    BYTE    nDelta;
    short   nRemLen;    // length of remaining SPRMs (including akt. SPRM)

    void UpdateMyMembers();
public:
    WW8SprmIter( BYTE* pSprms_, short nLen_, BYTE nVersion_ );
    void   SetSprms( BYTE* pSprms_, short nLen_ );
    BYTE*  operator ++( int );
    BYTE*  GetSprms()     const { return ( pSprms && (0 < nRemLen) )
                                ? pSprms
                                : 0; }
    BYTE*  GetAktParams() const { return pAktParams; }
    USHORT GetAktId()     const { return nAktId; }
};



/*
    u.a. fuer FKPs auf normale Attr., also ein Attr weniger als Positionen
*/
class WW8PLCF                       // Iterator fuer PLCFs
{
    INT32* pPLCF_PosArray;  // Pointer auf Pos-Array und auf ganze Struktur
    BYTE* pPLCF_Contents;       // Pointer auf Inhalts-Array-Teil des Pos-Array
    long nIMax;                         // Anzahl der Elemente
    long nIdx;
    long nStru;

    void ReadPLCF( SvStream* pSt, long nFilePos, long nPLCF );

    /*
        Falls im Dok ein PLC fehlt und die FKPs solo dastehen,
        machen wir uns hiermit einen PLC:
    */
    void GeneratePLCF( SvStream* pSt, long nPN, long ncpN );

public:
    WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF,
        long nStruct, long nStartPos = -1 );

    /*
        folgender Ctor generiert ggfs. einen PLC aus nPN und ncpN
    */
    WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF,
        long nStruct, long nStartPos, long nPN, long ncpN );

    ~WW8PLCF(){ delete( pPLCF_PosArray ); pPLCF_PosArray = 0; }
    ULONG GetIdx() const { return (ULONG)nIdx; }
    void SetIdx( ULONG nI ) { nIdx = (long)nI; }
    long GetIMax(){ return nIMax; }
    BOOL SeekPos( long nPos );
    long Where();
    BOOL Get( long& rStart, long& rEnd, void*& rpValue );
    WW8PLCF& operator ++( int ) { if( nIdx < nIMax ) nIdx++; return *this; }

    const void* GetData( long nIdx ) const {
        return ( nIdx >= nIMax ) ? 0 : (const void*)&pPLCF_Contents[nIdx * nStru]; }
};


DECLARE_TABLE( WW8Pcd_FC_sortArr, INT32 )


/*
    fuer Piece Table (bei Schnellspeicherung)
*/
class WW8PLCFpcd
{
friend class WW8PLCFpcd_Iter;
    WW8Pcd_FC_sortArr aFC_sort; // sorted PCD entries by FC
    INT32* pPLCF_PosArray;  // Pointer auf Pos-Array und auf ganze Struktur
    BYTE*  pPLCF_Contents;  // Pointer auf Inhalts-Array-Teil des Pos-Array
    long nIMax;
    long nStru;

public:
    WW8PLCFpcd( SvStream* pSt, long nFilePos, long nPLCF, long nStruct );
    ~WW8PLCFpcd(){ delete( pPLCF_PosArray ); }
    ULONG FindIdx( WW8_FC nFC ) const;
};

/*
    mehrere WW8PLCFpcd_Iter koennen auf die gleiche WW8PLCFpcd zeigen !!!
*/
class WW8PLCFpcd_Iter
{
    WW8PLCFpcd& rPLCF;
    long nIdx;

public:
    WW8PLCFpcd_Iter( WW8PLCFpcd& rPLCFpcd, long nStartPos = -1 );
//  ~WW8PLCFpcd_Iter() {}
    ULONG GetIdx() const { return (ULONG)nIdx; }
    void SetIdx( ULONG nI ) { nIdx = (long)nI; }
    long GetIMax(){ return rPLCF.nIMax; }
    BOOL SeekPos( long nPos );
//  BOOL SeekMaxMainFC( WW8Fib& rWwF, long& rMaxPosData );
    long Where();
    BOOL Get( long& rStart, long& rEnd, void*& rpValue );
    ULONG FindIdx( WW8_FC nFC ) const { return rPLCF.FindIdx( nFC ); }
    WW8PLCFpcd_Iter& operator ++( int ) { if( nIdx < rPLCF.nIMax ) nIdx++; return *this; }
};

// PLCF-Typ:
enum ePLCFT{ CHP=0, PAP, SEP, /*HED, FNR, ENR,*/ PLCF_END };

/*
    pure virtual:
*/
class WW8PLCFx              // virtueller Iterator fuer Piece Table Exceptions
{
    BYTE nVersion;              // Versionsnummer des FIB
    BOOL bIsSprm;               // PLCF von Sprms oder von anderem ( Footnote, ... )

public:
    WW8PLCFx( BYTE nFibVersion, BOOL bSprm )
        { bIsSprm = bSprm; nVersion = nFibVersion; }

//  virtual ~WW8PLCFx() {}
    BOOL IsSprm() { return bIsSprm; }
    virtual ULONG GetIdx() const = 0;
    virtual void SetIdx( ULONG nIdx ) = 0;
    virtual ULONG GetIdx2() const;
    virtual void SetIdx2( ULONG nIdx );
    virtual BOOL SeekPos( WW8_CP nCpPos ) = 0;
    virtual long Where() = 0;
//  virtual BYTE* GetSprms( long& rStart, long& rEnd, long& rLen );
    virtual void GetSprms( WW8PLCFxDesc* p );
    virtual long GetNoSprms( long& rStart, long&, long& rLen );
    virtual WW8PLCFx& operator ++( int ) = 0;
    virtual USHORT GetIstd() const { return 0xffff; }
    virtual void Save(          WW8PLCFxSave1& rSave ) const;
    virtual void Restore( const WW8PLCFxSave1& rSave );
    BYTE GetVersion() const { return nVersion; }
};

enum eCutT { CUT_NONE = 0, CUT_START, CUT_END, CUT_BOTH };

class WW8PLCFx_PCDAttrs : public WW8PLCFx
{
    WW8PLCFpcd_Iter* pPcdI;
    WW8PLCFx_PCD* pPcd;
    BYTE** pGrpprls;            // Attribute an Piece-Table
    SVBT32 aShortSprm;          // mini storage: can contain ONE sprm with
                                // 1 byte param
    UINT16 nGrpprls;            // Attribut Anzahl davon

public:
    WW8PLCFx_PCDAttrs( BYTE nVersion, WW8PLCFx_PCD* pPLCFx_PCD, WW8ScannerBase* pBase );
    virtual ~WW8PLCFx_PCDAttrs();
    virtual ULONG GetIdx() const;
    virtual void SetIdx( ULONG nI );
    virtual BOOL SeekPos( WW8_CP nCpPos );
    virtual long Where();
    virtual void GetSprms( WW8PLCFxDesc* p );
    virtual WW8PLCFx& operator ++( int );

    WW8PLCFpcd_Iter* GetIter() const { return pPcdI; }
};

class WW8PLCFx_PCD : public WW8PLCFx            // Iterator fuer Piece Table
{
    WW8PLCFpcd_Iter* pPcdI;
    BOOL bVer67;

public:
    WW8PLCFx_PCD( BYTE nVersion, WW8PLCFpcd* pPLCFpcd, WW8_CP nStartCp, BOOL bVer67P );
    virtual ~WW8PLCFx_PCD();
    virtual ULONG GetIMax() const;
    virtual ULONG GetIdx() const;
    virtual void SetIdx( ULONG nI );
    virtual BOOL SeekPos( WW8_CP nCpPos );
    virtual long Where();
//  virtual void GetSprms( WW8PLCFxDesc* p );
    virtual long GetNoSprms( long& rStart, long&, long& rLen );
    virtual WW8PLCFx& operator ++( int );
    WW8_CP AktPieceStartFc2Cp( WW8_FC nStartPos );
    WW8_FC AktPieceStartCp2Fc( WW8_CP nCp );
    eCutT AktPieceFc2Cp( long& rStartPos, long& rEndPos );
    WW8PLCFpcd_Iter* GetPLCFIter() { return pPcdI; }

    static INT32 TransformPieceAddress(long nfc, BOOL& bIsUnicodeAddress)
    {
        bIsUnicodeAddress = 0 == (0x40000000 & nfc);
        return bIsUnicodeAddress
            ?  nfc
            : (nfc & 0x3fffFFFF) / 2;
    }
    BOOL IsVersion67() const { return bVer67; }
};


class WW8PLCFx_Fc_FKP : public WW8PLCFx     // Iterator fuer Piece Table Exceptions of Fkps
                                // arbeitet nur auf FCs, nicht auf CPs !
                                // ( Low-Level )
{


        class WW8Fkp        // Iterator fuer Formatted Disk Page
        {
            struct WW8Grpprl
            {
                BYTE*  pData;
                USHORT nLen;
                USHORT nIStd; // nur bei Fkp.Papx gueltig (aktuelle Style-Nr)
                BOOL   bMustDelete;
            };

            WW8Grpprl* pGrpprl;     // Pointer of Meta Array (pointing

            BYTE* pFkp;         // gesamter Fkp

            long nItemSize;     // entweder 1 Byte oder ein komplettes BX
            long nFilePos;      // Offset in Stream where last read of 52 bytes took place
            short nIdx;         // Pos-Merker
            ePLCFT ePLCF;
            BYTE nIMax;         // Anzahl der Eintraege
            BYTE nVersion;

        public:
            WW8Fkp( BYTE nFibVer, SvStream* pFKPStrm, SvStream* pDataStrm,
                    long _nFilePos, long nItemSiz,
                    ePLCFT ePl, WW8_FC nStartFc = -1 );
            ~WW8Fkp();
            long GetFilePos() const { return nFilePos; }
            ULONG GetIdx() const { return (ULONG)nIdx; }
            void SetIdx( ULONG nI );
            BOOL SeekPos( long nPos );
            WW8_FC Where() { return (  nIdx<nIMax)
                                     ? ((WW8_FC*)pFkp)[nIdx]
                                     : LONG_MAX; }
            WW8Fkp& operator ++( int ) { if( nIdx < nIMax ) nIdx++;
                                         return *this; }
            BYTE* Get( WW8_FC& rStart, WW8_FC& rEnd, short& rLen );
            USHORT GetIstd() const { return pGrpprl[ nIdx ].nIStd; }

            /*
                liefert einen echten Pointer auf das Sprm vom Typ nId,
                falls ein solches im Fkp drin ist.
            */
            BYTE* GetLenAndIStdAndSprms(short& rLen);

            /*
                ruft GetLenAndIStdAndSprms() auf...
            */
            BYTE* HasSprm( USHORT nId );

            ULONG GetParaHeight() const;    // fuer Header/Footer bei Papx-Fkps

            BYTE GetVersion() const { return nVersion; }
        };


    WW8PLCFx_PCDAttrs* pPCDAttrs;
    SvStream* pFKPStrm;         // Input-File
    SvStream* pDataStrm;        // Input-File
    WW8PLCF* pPLCF;
    WW8Fkp* pFkp;
    WW8_FC nStartFc;

    BOOL NewFkp();

protected:
    ePLCFT ePLCF;

public:
    WW8PLCFx_Fc_FKP( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt,
                     WW8Fib& rFib, ePLCFT ePl, WW8_FC nStartFcL,
                     WW8PLCFx_PCDAttrs* pPLCFx_PCD );
    virtual ~WW8PLCFx_Fc_FKP();
    virtual ULONG GetIdx() const;
    virtual void SetIdx( ULONG nIdx );
    virtual BOOL SeekPos( WW8_FC nFcPos );
    virtual WW8_FC Where();
    BYTE* GetSprms( WW8_FC& rStart, WW8_FC& rEnd, long& rLen );
    virtual WW8PLCFx& operator ++( int );
    virtual USHORT GetIstd() const;
    void GetPCDSprms( WW8PLCFxDesc& rDesc );
    BYTE* HasSprm( USHORT nId );
    ULONG GetParaHeight() const;
    BOOL HasFkp() { return (0 != pFkp); }
};

// Iterator fuer Piece Table Exceptions of Fkps arbeitet auf CPs (High-Level)
class WW8PLCFx_Cp_FKP : public WW8PLCFx_Fc_FKP
{
    const WW8ScannerBase& rSBase;
    WW8PLCFx_PCD* pPcd;
    WW8_CP nAttrStart, nAttrEnd;
    BOOL bLineEnd : 1;
    BOOL bComplex : 1;

    void SearchParaEnd( long nOldEndCp );

public:
    WW8PLCFx_Cp_FKP( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt, const WW8ScannerBase&
                        rBase,  ePLCFT ePl );
    virtual ~WW8PLCFx_Cp_FKP();
    void ResetAttrStartEnd();
    ULONG GetPCDIMax() const;
    ULONG GetPCDIdx() const;
    void SetPCDIdx( ULONG nIdx );
    virtual ULONG GetIdx2() const;
    virtual void  SetIdx2( ULONG nIdx );
    virtual BOOL SeekPos( WW8_CP nCpPos );
    virtual WW8_CP Where();
    virtual void GetSprms( WW8PLCFxDesc* p );
    virtual WW8PLCFx& operator ++( int );
    virtual void Save(          WW8PLCFxSave1& rSave ) const;
    virtual void Restore( const WW8PLCFxSave1& rSave );
};



class WW8PLCFx_SEPX : public WW8PLCFx           // Iterator fuer Piece Table Exceptions of Sepx
{
    SvStream* pStrm;
    WW8PLCF* pPLCF;
    BYTE* pSprms;
    USHORT nArrMax;
    UINT16 nSprmSiz;

public:
    WW8PLCFx_SEPX( SvStream* pSt, SvStream* pTblxySt, WW8Fib& rFib, WW8_CP nStartCp );
    virtual ~WW8PLCFx_SEPX();
    virtual ULONG GetIdx() const;
    virtual void SetIdx( ULONG nIdx );
    long GetIMax() const { return ( pPLCF ) ? pPLCF->GetIMax() : 0; }
    virtual BOOL SeekPos( WW8_CP nCpPos );
    virtual long Where();
    virtual void GetSprms( WW8PLCFxDesc* p );
    virtual WW8PLCFx& operator ++( int );
    BYTE* HasSprm( USHORT nId ) const;
    BYTE* HasSprm( USHORT nId, BYTE n2nd ) const;
    BYTE* HasSprm( USHORT nId, BYTE* pOtherSprms, long nOtherSprmSiz ) const;
    BOOL Find4Sprms(USHORT nId1, USHORT nId2, USHORT nId3, USHORT nId4,
                    BYTE*& p1,   BYTE*& p2,   BYTE*& p3,   BYTE*& p4 ) const;
    BOOL CompareSprms( BYTE* pOtherSprms,
                       long nOtherSprmSiz,
                       const SvUShortsSort* pIgnoreSprms = 0 ) const;
};

// Iterator fuer Fuss-/Endnoten und Anmerkungen
class WW8PLCFx_SubDoc : public WW8PLCFx
{
    WW8PLCF* pRef;
    WW8PLCF* pTxt;

public:
    WW8PLCFx_SubDoc( SvStream* pSt, BYTE nVersion, WW8_CP nStartCp,
                    long nFcRef, long nLenRef,
                    long nFcTxt, long nLenTxt, long nStruc = 0 );
    virtual ~WW8PLCFx_SubDoc();
    virtual ULONG GetIdx() const;
    virtual void SetIdx( ULONG nIdx );
    virtual BOOL SeekPos( WW8_CP nCpPos );
    virtual long Where();

    // liefert Reference Descriptoren
    const void* GetData( long nIdx = -1 ) const
        {
            return pRef ? pRef->GetData( -1L == nIdx ? pRef->GetIdx()
                                                     : nIdx ) : 0;
        }

    /*
        liefert Angabe, wo Kopf und Fusszeilen-Text zu finden ist
    */
    BOOL Get( long& rStart, void*& rpValue );
    virtual long GetNoSprms( long& rStart, long&, long& rLen );
    virtual WW8PLCFx& operator ++( int );
    long Count() { return ( pRef ) ? pRef->GetIMax() : 0; }
};

class WW8PLCFx_FLD : public WW8PLCFx            // Iterator fuer Fuss- und Endnoten
{
    WW8PLCFspecial* pPLCF;
    WW8Fib& rFib;

public:
    WW8PLCFx_FLD( SvStream* pSt, WW8Fib& rMyFib, short nType, WW8_CP nStartCp );
    virtual ~WW8PLCFx_FLD();
    virtual ULONG GetIdx() const;
    virtual void SetIdx( ULONG nIdx );
    virtual BOOL SeekPos( WW8_CP nCpPos );
    virtual long Where();
    virtual long GetNoSprms( long& rStart, long&, long& rLen );
    virtual WW8PLCFx& operator ++( int );
    BOOL GetPara( long nIdx, WW8FieldDesc& rF );
};

enum eBookStatus { BOOK_NORMAL = 0, BOOK_IGNORE = 0x1, BOOK_ONLY_REF = 0x2 };

class WW8PLCFx_Book : public WW8PLCFx           // Iterator fuer Booknotes
{
    WW8PLCFspecial* pBook[2];           // Start- und EndPosition
    SvStrings aBookNames;               // Name
    eBookStatus* pStatus;
    long nIMax;                         // Anzahl der Booknotes
    USHORT nIsEnd;
public:
    WW8PLCFx_Book( SvStream* pSt, SvStream* pTblSt, WW8Fib& rFib, WW8_CP nStartCp );
    virtual ~WW8PLCFx_Book();
    long GetIMax(){ return nIMax; }
    virtual ULONG GetIdx() const;
    virtual void SetIdx( ULONG nI );
    virtual ULONG GetIdx2() const;
    virtual void SetIdx2( ULONG nIdx );
    virtual BOOL SeekPos( WW8_CP nCpPos );
    virtual long Where();
    virtual long GetNoSprms( long& rStart, long& rEnd, long& rLen );
    virtual WW8PLCFx& operator ++( int );
    const String* GetName() const
        { return ( !nIsEnd && ( (long)(pBook[0]->GetIdx()) < nIMax ) )
                        ? aBookNames[ pBook[0]->GetIdx() ]
                        : 0;
        }
    WW8_CP GetStartPos() const
        { return ( nIsEnd ) ? LONG_MAX : pBook[0]->Where(); }
    long GetLen() const;
    BOOL GetIsEnd() const { return ( nIsEnd ) ? TRUE : FALSE; }
    long GetHandle() const;
    BOOL SetStatus( WW8_CP nStartRegion, WW8_CP nEndRegion, const String& rName,
                    eBookStatus eStat );
    eBookStatus GetStatus() const;
};

/*
    hiermit arbeiten wir draussen:
*/
struct WW8PLCFManResult
{
    WW8_CP nCpPos;      // Attribut-Anfangsposition
    long nMemLen;       // Laenge dazu
    long nCp2OrIdx;     // footnote-textpos oder Index in PLCF
    WW8_CP nAktCp;      // wird nur vom Aufrufer benutzt
    BYTE* pMemPos;      // Mem-Pos fuer Sprms
    USHORT nSprmId;     // Sprm-Id ( 0 = ungueltige Id -> ueberspringen! )
                                        // (2..255) oder Pseudo-Sprm-Id (256..260)
                                        // bzw. ab Winword-Ver8 die Sprm-Id (800..)
    BYTE nFlags;        // Absatz- oder Section-Anfang
};

#define MAN_ANZ_PLCF 12

#define MAN_MASK_NEW_PAP 1      // neue Zeile
#define MAN_MASK_NEW_SEP 2      // neue Section


#define MAN_MAINTEXT 0          // Defines fuer PLCFMan-ctor
#define MAN_FTN 1
#define MAN_EDN 2
#define MAN_HDFT 3
#define MAN_AND 4
#define MAN_TXBX 5
#define MAN_TXBX_HDFT 6

struct WW8PLCFxSaveAll
{
    WW8PLCFxSave1 aS[MAN_ANZ_PLCF];
};

/*
    hiermit arbeitet der Manager drinnen:
*/
struct WW8PLCFxDesc
{
    WW8PLCFx* pPLCFx;
    UShortStk* pIdStk;// Speicher fuer Attr-Id fuer Attr-Ende(n)
    BYTE* pMemPos;      // wo liegen die Sprm(s)
    long nStartPos;
    long nEndPos;
    long nCp2OrIdx;     // wo liegen die NoSprm(s)
    long nSprmsLen;     // wie viele Bytes fuer weitere Sprms / Laenge Fussnote
    long nCpOfs;        // fuer Offset Header .. Footnote
    BOOL bFirstSprm;    // fuer Erkennung erster Sprm einer Gruppe
    BOOL bRealLineEnd;  // FALSE bei Pap-Piece-Ende
    void Save(          WW8PLCFxSave1& rSave ) const;
    void Restore( const WW8PLCFxSave1& rSave );
};


#ifndef DUMP

class WW8PLCFMan
{
    long nCpO;      // Origin Cp -- the basis for nNewCp

    long nLineEnd;                  // zeigt *hinter* das <CR>
    long nLastWhereIdxCp;           // last result of WhereIdx()
    USHORT nPLCF;                   // so viele PLCFe werden verwaltet
    short nManType;

    WW8PLCFxDesc aD[MAN_ANZ_PLCF];
    WW8PLCFxDesc *pChp, *pPap, *pSep, *pFld, *pFldTxbx, *pFldTxbxHdft,
                 *pFtn, *pEdn,
                 *pBkm, *pPcd, *pPcdA, *pAnd;
    WW8PLCFspecial *pFdoa, *pTxbx, *pTxbxBkd;

    WW8Fib* pWwFib;
    USHORT* pNoAttrScan; // Attribute komplett(!) ignorieren, die ueber n CPs
                        // aufgespannt sind; z.B. bei Char #7 (Zellen-/Zeilenende)

    short WhereIdx( BOOL* pbStart, long* pPos );
    void AdjustEnds(    WW8PLCFxDesc& rDesc );
    void GetNewSprms(   WW8PLCFxDesc& rDesc );
    void GetNewNoSprms( WW8PLCFxDesc& rDesc );
    void GetSprmStart( short nIdx, WW8PLCFManResult* pRes );
    void GetSprmEnd( short nIdx, WW8PLCFManResult* pRes );
    void GetNoSprmStart( short nIdx, WW8PLCFManResult* pRes );
    void GetNoSprmEnd( short nIdx, WW8PLCFManResult* pRes );
    void AdvSprm(   short nIdx, BOOL bStart );
    void AdvNoSprm( short nIdx, BOOL bStart );

public:
    WW8PLCFMan( WW8ScannerBase* pBase, short nType, long nStartCp );
    ~WW8PLCFMan();

    /*
        Where fragt, an welcher naechsten Position sich irgendein
        Attr aendert...
    */
    long Where();

    BOOL Get( WW8PLCFManResult* pResult );
    WW8PLCFMan& operator ++( int );
    USHORT GetColl() const; // liefert aktuellen Style
    WW8PLCFx_FLD* GetFld() const;
    WW8PLCFx_SubDoc* GetEdn() const { return (WW8PLCFx_SubDoc*)pEdn->pPLCFx; }
    WW8PLCFx_SubDoc* GetFtn() const { return (WW8PLCFx_SubDoc*)pFtn->pPLCFx; }
    WW8PLCFx_SubDoc* GetAtn() const { return (WW8PLCFx_SubDoc*)pAnd->pPLCFx; }
    WW8PLCFx_Book* GetBook() const { return (WW8PLCFx_Book*)pBkm->pPLCFx; }
    long GetCpOfs() const { return pChp->nCpOfs; }  // fuer Header/Footer...

    /*
        fragt, ob *aktueller Absatz* einen Sprm diesen Typs hat
    */
    BYTE* HasParaSprm( USHORT nId ) const;

    /*
        fragt, ob *aktueller Textrun* einen Sprm diesen Typs hat
    */
    BYTE* HasCharSprm( USHORT nId ) const;

    WW8PLCFx_Cp_FKP* GetChpPLCF(){ return (WW8PLCFx_Cp_FKP*)pChp->pPLCFx; }
    WW8PLCFx_Cp_FKP* GetPapPLCF(){ return (WW8PLCFx_Cp_FKP*)pPap->pPLCFx; }
    WW8PLCFx_SEPX* GetSepPLCF(){   return (WW8PLCFx_SEPX*)pSep->pPLCFx; }
    WW8PLCFxDesc* GetPap(){ return pPap; }
    void SeekPos( long nNewCp );
    void SaveAllPLCFx(                   WW8PLCFxSaveAll& rSave ) const;
    void RestoreAllPLCFx(          const WW8PLCFxSaveAll& rSave );
    WW8PLCFspecial* GetFdoa()   { return pFdoa;     }
    WW8PLCFspecial* GetTxbx()   { return pTxbx;     }
    WW8PLCFspecial* GetTxbxBkd(){ return pTxbxBkd;  }
    short GetManType() { return nManType; }
};

#endif // !DUMP

class WW8ScannerBase
{
friend WW8PLCFx_PCDAttrs::WW8PLCFx_PCDAttrs( BYTE nVersion, WW8PLCFx_PCD* pPLCFx_PCD, WW8ScannerBase* pBase );
friend WW8PLCFx_Cp_FKP::WW8PLCFx_Cp_FKP( SvStream*, SvStream*, SvStream*,
                                        const WW8ScannerBase&, ePLCFT );

#ifdef DUMP
friend static void DumpPLCFText( WW8_FC nPos, long nLen, long nOfs, sal_Char* pName,
                                WW8ScannerBase* pBase );
friend static void DumpFtnShort( short nId, long nPos, long nFieldLen );
#else
friend WW8PLCFMan::WW8PLCFMan( WW8ScannerBase*, short, long );
friend class SwWw8ImplReader;
#endif

    WW8Fib* pWw8Fib;
    WW8PLCFx_Cp_FKP*  pChpPLCF;         // Character-Attrs
    WW8PLCFx_Cp_FKP*  pPapPLCF;         // Para-Attrs
    WW8PLCFx_SEPX*    pSepPLCF;         // Section-Attrs
    WW8PLCFx_SubDoc*  pFtnPLCF;         // Footnotes
    WW8PLCFx_SubDoc*  pEdnPLCF;         // EndNotes
    WW8PLCFx_SubDoc*  pAndPLCF;         // Anmerkungen
    WW8PLCFx_FLD*     pFldPLCF;         // Fields in Main Text
    WW8PLCFx_FLD*     pFldHdFtPLCF;     // Fields in Header / Footer
    WW8PLCFx_FLD*     pFldTxbxPLCF;     // Fields in Textboxes in Main Text
    WW8PLCFx_FLD*     pFldTxbxHdFtPLCF; // Fields in Textboxes in Header / Footer
    WW8PLCFx_FLD*     pFldFtnPLCF;      // Fields in Footnotes
    WW8PLCFx_FLD*     pFldEdnPLCF;      // Fields in Endnotes
    WW8PLCFx_FLD*     pFldAndPLCF;      // Fields in Anmerkungen
    WW8PLCFspecial*   pMainFdoa;        // Graphic Primitives in Main Text
    WW8PLCFspecial*   pHdFtFdoa;        // Graphic Primitives in Header / Footer
    WW8PLCFspecial*   pMainTxbx;        // Textboxen in Main Text
    WW8PLCFspecial*   pMainTxbxBkd;     // Break-Deskriptoren fuer diese
    WW8PLCFspecial*   pHdFtTxbx;        // TextBoxen in Header / Footer
    WW8PLCFspecial*   pHdFtTxbxBkd;     // Break-Deskriptoren fuer diese
    WW8PLCFx_Book*    pBook;            // Bookmarks

    WW8PLCFpcd*         pPiecePLCF; // fuer FastSave ( Basis-PLCF ohne Iterator )
    WW8PLCFpcd_Iter*    pPieceIter; // fuer FastSave ( Iterator dazu )
    WW8PLCFx_PCD*       pPLCFx_PCD;     // dito
    WW8PLCFx_PCDAttrs*  pPLCFx_PCDAttrs;
    BYTE**              pPieceGrpprls;  // Attribute an Piece-Table
    UINT16              nPieceGrpprls;  // Anzahl davon
    USHORT              nNoAttrScan;    // Attribute komplett(!) ignorieren, die ueber n CPs
                                        // aufgespannt sind; z.B. bei Char #7 (Zellen-/Zeilenende)

    WW8PLCFpcd* OpenPieceTable( SvStream* pStr, WW8Fib* pWwF );
    void DeletePieceTable();

public:
    WW8ScannerBase( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt,
                    WW8Fib* pWwF );
    ~WW8ScannerBase();
    BOOL AreThereFootnotes() { return pFtnPLCF->Count() > 0; };
    BOOL AreThereEndnotes()  { return pEdnPLCF->Count() > 0; };
    static DateTime WW8DTTM2DateTime(long lDTTM);

    WW8_CP WW8Fc2Cp( WW8_FC nFcPos ) const ;
    WW8_FC WW8Cp2Fc( WW8_CP nCpPos, BOOL* pIsUnicode = 0,
                     WW8_CP* pNextPieceCp = 0, BOOL* pTestFlag = 0 ) const;
    void SetNoAttrScan( USHORT nValue ) { nNoAttrScan = nValue; };

    USHORT WW8ReadString( SvStream& rStrm, String& rStr,
                            WW8_CP nAktStartCp, long nTotalLen,
                            rtl_TextEncoding eEnc ) const;
};




class WW8Fib
{
public:
    /*
        von uns verlangte Programm-Version:
        in Ctor wird geprueft, ob sie zu nFib passt
    */
    BYTE nVersion;  // 6 steht fuer "WinWord 6 oder WinWord 95",
                                    // 7 steht fuer "nur WinWord 95"
                                    // 8 steht fuer "nur WinWord 97"
    /*
        Fehlerstatus
    */
    ULONG nFibError;
    /*
        vom Ctor aus dem FIB gelesene Daten
        (entspricht nur ungefaehr der tatsaechlichen Struktur
         des Winword-FIB)
    */
    UINT16 wIdent;      // 0x0 int magic number
    UINT16 nFib;        // 0x2 FIB version written
    UINT16 nProduct;    // 0x4 product version written by
    INT16 lid;          // 0x6 language stamp---localized version;
    WW8_PN pnNext;          // 0x8

    UINT16 fDot :1;     // 0xa 0001
    UINT16 fGlsy :1;
    UINT16 fComplex :1; // 0004 when 1, file is in complex, fast-saved format.
    UINT16 fHasPic :1;  // 0008 file contains 1 or more pictures
    UINT16 cQuickSaves :4; // 00F0 count of times file was quicksaved
    UINT16 fEncrypted :1; //0100 1 if file is encrypted, 0 if not
    UINT16 fWhichTblStm :1; //0200 When 0, this fib refers to the table stream
                                                    // named "0Table", when 1, this fib refers to the
                                                    // table stream named "1Table". Normally, a file
                                                    // will have only one table stream, but under unusual
                                                    // circumstances a file may have table streams with
                                                    // both names. In that case, this flag must be used
                                                    // to decide which table stream is valid.
    // UINT16 u1 :2;
    UINT16 fExtChar :1; // 1000 =1, when using extended character set in file
    // UINT16 u2 :3;

    UINT16 nFibBack;    // 0xc
    INT16 lKey1;            // 0xe  file encrypted key, only valid if fEncrypted.
    INT16 lKey2;            // 0x10  key in 2 Portionen wg. Misalignment
    UINT8 envr;         // 0x12 environment in which file was created
                                    //      0 created by Win Word / 1 created by Mac Word
    BYTE fMac              :1;          // 0x13 when 1, this file was last saved in the Mac environment
    BYTE fEmptySpecial     :1;
    BYTE fLoadOverridePage :1;
    BYTE fFuturesavedUndo  :1;
    BYTE fWord97Saved      :1;
    BYTE :3;

    UINT16 chse;        // 0x14 default extended character set id for text in document stream. (overidden by chp.chse)
                        //      0 = ANSI  / 256 Macintosh character set.
    UINT16 chseTables;  // 0x16 default extended character set id for text in
                        //      internal data structures: 0 = ANSI, 256 = Macintosh
    WW8_FC fcMin;           // 0x18 file offset of first character of text
    WW8_FC fcMac;           // 0x1c file offset of last character of text + 1

    // Einschub fuer WW8 *****************************************************
    UINT16 csw;             // Count of fields in the array of "shorts"

    // Marke: "rgsw" Beginning of the array of shorts
    UINT16 wMagicCreated;                   // unique number Identifying the File's creator
                                                                // 0x6A62 is the creator ID for Word and is reserved.
                                                                // Other creators should choose a different value.
    UINT16 wMagicRevised;                   // identifies the File's last modifier
  UINT16 wMagicCreatedPrivate;  // private data
    UINT16 wMagicRevisedPrivate;    // private data
    /*
    INT16  pnFbpChpFirst_W6;            // not used
    INT16  pnChpFirst_W6;                   // not used
    INT16  cpnBteChp_W6;                    // not used
    INT16  pnFbpPapFirst_W6;            // not used
    INT16  pnPapFirst_W6;                   // not used
    INT16  cpnBtePap_W6;                    // not used
    INT16  pnFbpLvcFirst_W6;            // not used
    INT16  pnLvcFirst_W6;                   // not used
    INT16  cpnBteLvc_W6;                    // not used
    */
    INT16  lidFE;                                   // Language id if document was written by Far East version
                                                                // of Word (i.e. FIB.fFarEast is on)
    UINT16 clw;                                     // Number of fields in the array of longs

    // Ende des Einschubs fuer WW8 *******************************************

    // Marke: "rglw" Beginning of the array of longs
    WW8_FC cbMac;           // 0x20 file offset of last byte written to file + 1.

    // WW8_FC u4[4];            // 0x24
    WW8_CP ccpText;         // 0x34 length of main document text stream
    WW8_CP ccpFtn;          // 0x38 length of footnote subdocument text stream
    WW8_CP ccpHdr;          // 0x3c length of header subdocument text stream
    WW8_CP ccpMcr;          // 0x40 length of macro subdocument text stream
    WW8_CP ccpAtn;          // 0x44 length of annotation subdocument text stream
    WW8_CP ccpEdn;          // 0x48 length of endnote subdocument text stream
    WW8_CP ccpTxbx;         // 0x4c length of textbox subdocument text stream
    WW8_CP ccpHdrTxbx;      // 0x50 length of header textbox subdocument text stream

    // Einschub fuer WW8 *****************************************************
    INT32  pnFbpChpFirst;   // when there was insufficient memory for Word to expand
                                                // the PLCFbte at save time, the PLCFbte is written
                                                // to the file in a linked list of 512-byte pieces
                                                // starting with this pn.
    /*
    // folgende Felder existieren zwar so in der Datei,
    // wir benutzen jedoch unten deklarierte General-Variablen
    // fuer Ver67 und Ver8 gemeinsam.
    INT32  pnChpFirst;      // the page number of the lowest numbered page in the
                                                        // document that records CHPX FKP information
    INT32  cpnBteChp;           // count of CHPX FKPs recorded in file. In non-complex
                                                        // files if the number of entries in the PLCFbteChpx
                                                        // is less than this, the PLCFbteChpx is incomplete.
    */
    INT32  pnFbpPapFirst;   // when there was insufficient memory for Word to expand
                                                // the PLCFbte at save time, the PLCFbte is written to
                                                // the file in a linked list of 512-byte pieces
                                                // starting with this pn
    /*
    // folgende Felder existieren zwar so in der Datei,
    // wir benutzen jedoch unten deklarierte General-Variablen
    // fuer Ver67 und Ver8 gemeinsam.
    INT32  pnPapFirst;      // the page number of the lowest numbered page in the
                                                        // document that records PAPX FKP information
    INT32  cpnBtePap;       // count of PAPX FKPs recorded in file. In non-complex
                                                        // files if the number of entries in the PLCFbtePapx is
                                                        // less than this, the PLCFbtePapx is incomplete.
    */
    INT32  pnFbpLvcFirst;   // when there was insufficient memory for Word to expand
                                                // the PLCFbte at save time, the PLCFbte is written to
                                                // the file in a linked list of 512-byte pieces
                                                // starting with this pn
    INT32  pnLvcFirst;          // the page number of the lowest numbered page in the
                                                // document that records LVC FKP information
    INT32  cpnBteLvc;           // count of LVC FKPs recorded in file. In non-complex
                                                // files if the number of entries in the PLCFbtePapx is
                                                // less than this, the PLCFbtePapx is incomplete.
    INT32  fcIslandFirst;   // ?
    INT32  fcIslandLim;     // ?
    UINT16 cfclcb;              // Number of fields in the array of FC/LCB pairs.

    // Ende des Einschubs fuer WW8 *******************************************

    // Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
    WW8_FC fcStshfOrig;     // file offset of original allocation for STSH in table
                                                // stream. During fast save Word will attempt to reuse
                                                // this allocation if STSH is small enough to fit.
    INT32 lcbStshfOrig; // 0x5c count of bytes of original STSH allocation
    WW8_FC fcStshf;         // 0x60 file offset of STSH in file.
    INT32 lcbStshf;     // 0x64 count of bytes of current STSH allocation
    WW8_FC fcPlcffndRef;    // 0x68 file offset of footnote reference PLCF.
    INT32 lcbPlcffndRef;    // 0x6c count of bytes of footnote reference PLCF
                        //      == 0 if no footnotes defined in document.

    WW8_FC fcPlcffndTxt;    // 0x70 file offset of footnote text PLCF.
    INT32 lcbPlcffndTxt;    // 0x74 count of bytes of footnote text PLCF.
                        //      == 0 if no footnotes defined in document

    WW8_FC fcPlcfandRef;    // 0x78 file offset of annotation reference PLCF.
    INT32 lcbPlcfandRef;    // 0x7c count of bytes of annotation reference PLCF.

    WW8_FC fcPlcfandTxt;    // 0x80 file offset of annotation text PLCF.
    INT32 lcbPlcfandTxt;    // 0x84 count of bytes of the annotation text PLCF

    WW8_FC fcPlcfsed;       // 8x88 file offset of section descriptor PLCF.
    INT32 lcbPlcfsed;   // 0x8c count of bytes of section descriptor PLCF.

    WW8_FC fcPlcfpad;       // 0x90 file offset of paragraph descriptor PLCF
    INT32 lcbPlcfpad;   // 0x94 count of bytes of paragraph descriptor PLCF.
                        // ==0 if file was never viewed in Outline view.
                        // Should not be written by third party creators

    WW8_FC fcPlcfphe;       // 0x98 file offset of PLCF of paragraph heights.
    INT32 lcbPlcfphe;   // 0x9c count of bytes of paragraph height PLCF.
                        // ==0 when file is non-complex.

    WW8_FC fcSttbfglsy;     // 0xa0 file offset of glossary string table.
    INT32 lcbSttbfglsy; // 0xa4 count of bytes of glossary string table.
                        //      == 0 for non-glossary documents.
                        //      !=0 for glossary documents.

    WW8_FC fcPlcfglsy;      // 0xa8 file offset of glossary PLCF.
    INT32 lcbPlcfglsy;  // 0xac count of bytes of glossary PLCF.
                        //      == 0 for non-glossary documents.
                        //      !=0 for glossary documents.

    WW8_FC fcPlcfhdd;       // 0xb0 byte offset of header PLCF.
    INT32 lcbPlcfhdd;   // 0xb4 count of bytes of header PLCF.
                        //      == 0 if document contains no headers

    WW8_FC fcPlcfbteChpx;   // 0xb8 file offset of character property bin table.PLCF.
    INT32 lcbPlcfbteChpx;// 0xbc count of bytes of character property bin table PLCF.

    WW8_FC fcPlcfbtePapx;   // 0xc0 file offset of paragraph property bin table.PLCF.
    INT32 lcbPlcfbtePapx;// 0xc4 count of bytes of paragraph  property bin table PLCF.

    WW8_FC fcPlcfsea;       // 0xc8 file offset of PLCF reserved for private use. The SEA is 6 bytes long.
    INT32 lcbPlcfsea;   // 0xcc count of bytes of private use PLCF.

    WW8_FC fcSttbfffn;      // 0xd0 file offset of font information STTBF. See the FFN file structure definition.
    INT32 lcbSttbfffn;  // 0xd4 count of bytes in sttbfffn.

    WW8_FC fcPlcffldMom;    // 0xd8 offset in doc stream to the PLCF of field positions in the main document.
    INT32 lcbPlcffldMom;    // 0xdc

    WW8_FC fcPlcffldHdr;    // 0xe0 offset in doc stream to the PLCF of field positions in the header subdocument.
    INT32 lcbPlcffldHdr;    // 0xe4

    WW8_FC fcPlcffldFtn;    // 0xe8 offset in doc stream to the PLCF of field positions in the footnote subdocument.
    INT32 lcbPlcffldFtn;    // 0xec

    WW8_FC fcPlcffldAtn;    // 0xf0 offset in doc stream to the PLCF of field positions in the annotation subdocument.
    INT32 lcbPlcffldAtn;    // 0xf4

    WW8_FC fcPlcffldMcr;    // 0xf8 offset in doc stream to the PLCF of field positions in the macro subdocument.
    INT32 lcbPlcffldMcr;    // 9xfc

    WW8_FC fcSttbfbkmk; // 0x100 offset in document stream of the STTBF that records bookmark names in the main document
    INT32 lcbSttbfbkmk; // 0x104

    WW8_FC fcPlcfbkf;   // 0x108 offset in document stream of the PLCF that records the beginning CP offsets of bookmarks in the main document. See BKF
    INT32 lcbPlcfbkf;   // 0x10c

    WW8_FC fcPlcfbkl;   // 0x110 offset in document stream of the PLCF that records the ending CP offsets of bookmarks recorded in the main document. See the BKL structure definition.
    INT32 lcbPlcfbkl;   // 0x114 INT32

    WW8_FC fcCmds;      // 0x118 FC
    INT32 lcbCmds;      // 0x11c

    WW8_FC fcPlcfmcr;       // 0x120 FC
    INT32 lcbPlcfmcr;       // 0x124

    WW8_FC fcSttbfmcr;  // 0x128 FC
    INT32 lcbSttbfmcr;  // 0x12c

    WW8_FC fcPrDrvr;        // 0x130 file offset of the printer driver information (names of drivers, port etc...)
    INT32 lcbPrDrvr;        // 0x134 count of bytes of the printer driver information (names of drivers, port etc...)

    WW8_FC fcPrEnvPort; // 0x138 file offset of the print environment in portrait mode.
    INT32 lcbPrEnvPort; // 0x13c count of bytes of the print environment in portrait mode.

    WW8_FC fcPrEnvLand; // 0x140 file offset of the print environment in landscape mode.
    INT32 lcbPrEnvLand; // 0x144 count of bytes of the print environment in landscape mode.

    WW8_FC fcWss;       // 0x148 file offset of Window Save State data structure. See WSS.
    INT32 lcbWss;       // 0x14c count of bytes of WSS. ==0 if unable to store the window state.

    WW8_FC fcDop;       // 0x150 file offset of document property data structure.
    INT32 lcbDop;       // 0x154 count of bytes of document properties.
        // cbDOP is 84 when nFib < 103


    WW8_FC fcSttbfAssoc;    // 0x158 offset to STTBF of associated strings. See STTBFASSOC.
    INT32 cbSttbfAssoc; // 0x15C

    WW8_FC fcClx;           // 0x160 file  offset of beginning of information for complex files.
    INT32 lcbClx;       // 0x164 count of bytes of complex file information. 0 if file is non-complex.

    WW8_FC fcPlcfpgdFtn;    // 0x168 file offset of page descriptor PLCF for footnote subdocument.
    INT32 lcbPlcfpgdFtn;    // 0x16C count of bytes of page descriptor PLCF for footnote subdocument.
                        //  ==0 if document has not been paginated. The length of the PGD is 8 bytes.

    WW8_FC fcAutosaveSource;    // 0x170 file offset of the name of the original file.
    INT32 lcbAutosaveSource;    // 0x174 count of bytes of the name of the original file.

    WW8_FC fcGrpStAtnOwners;    // 0x178 group of strings recording the names of the owners of annotations
    INT32 lcbGrpStAtnOwners;    // 0x17C count of bytes of the group of strings

    WW8_FC fcSttbfAtnbkmk;  // 0x180 file offset of the sttbf that records names of bookmarks in the annotation subdocument
    INT32 lcbSttbfAtnbkmk;  // 0x184 length in bytes of the sttbf that records names of bookmarks in the annotation subdocument

    // Einschubs fuer WW67 ***************************************************

    // INT16 wSpare4Fib;    // Reserve, muss hier nicht deklariert werden

    /*
    // folgende Felder existieren zwar so in der Datei,
    // wir benutzen jedoch unten deklarierte General-Variablen
    // fuer Ver67 und Ver8 gemeinsam.
    WW8_PN pnChpFirst;  // the page number of the lowest numbered page in
                                                        // the document that records CHPX FKP information
    WW8_PN pnPapFirst;  // the page number of the lowest numbered page in
                                                        // the document that records PAPX FKP information

    WW8_PN cpnBteChp;       // count of CHPX FKPs recorded in file. In non-complex
                                                        // files if the number of entries in the PLCFbteChpx is
                                                        // less than this, the PLCFbteChpx  is incomplete.
    WW8_PN cpnBtePap;       // count of PAPX FKPs recorded in file. In non-complex
                                                        // files if the number of entries in the PLCFbtePapx is
                                                        // less than this, the PLCFbtePapx  is incomplete.
    */

    // Ende des Einschubs fuer WW67 ******************************************

    WW8_FC fcPlcfdoaMom;    // 0x192 file offset of the  FDOA (drawn object) PLCF for main document.
                        //  ==0 if document has no drawn objects. The length of the FDOA is 6 bytes.
                        // ab Ver8 unused
    INT32 lcbPlcfdoaMom;    // 0x196 length in bytes of the FDOA PLCF of the main document
                                                // ab Ver8 unused
    WW8_FC fcPlcfdoaHdr;    // 0x19A file offset of the  FDOA (drawn object) PLCF for the header document.
                        //  ==0 if document has no drawn objects. The length of the FDOA is 6 bytes.
                        // ab Ver8 unused
    INT32 lcbPlcfdoaHdr;    // 0x19E length in bytes of the FDOA PLCF of the header document
                                                // ab Ver8 unused

    WW8_FC fcPlcfspaMom;        // offset in table stream of the FSPA PLCF for main document.
                                                // == 0 if document has no office art objects
                                                        // war in Ver67 nur leere Reserve
    INT32 lcbPlcfspaMom;        // length in bytes of the FSPA PLCF of the main document
                                                        // war in Ver67 nur leere Reserve
    WW8_FC fcPlcfspaHdr;        // offset in table stream of the FSPA PLCF for header document.
                                                // == 0 if document has no office art objects
                                                        // war in Ver67 nur leere Reserve
    INT32 lcbPlcfspaHdr;        // length in bytes of the FSPA PLCF of the header document
                                                        // war in Ver67 nur leere Reserve

    WW8_FC fcPlcfAtnbkf;    // 0x1B2 file offset of BKF (bookmark first) PLCF of the annotation subdocument
    INT32 lcbPlcfAtnbkf;    // 0x1B6 length in bytes of BKF (bookmark first) PLCF of the annotation subdocument

    WW8_FC fcPlcfAtnbkl;    // 0x1BA file offset of BKL (bookmark last) PLCF of the annotation subdocument
    INT32 lcbPlcfAtnbkl;    // 0x1BE length in bytes of BKL (bookmark first) PLCF of the annotation subdocument

    WW8_FC fcPms;       // 0x1C2 file offset of PMS (Print Merge State) information block
    INT32 lcbPMS;       // 0x1C6 length in bytes of PMS

    WW8_FC fcFormFldSttbf;  // 0x1CA file offset of form field Sttbf which contains strings used in form field dropdown controls
    INT32 lcbFormFldSttbf;  // 0x1CE length in bytes of form field Sttbf

    WW8_FC fcPlcfendRef;    // 0x1D2 file offset of PLCFendRef which points to endnote references in the main document stream
    INT32 lcbPlcfendRef;    // 0x1D6

    WW8_FC fcPlcfendTxt;    // 0x1DA file offset of PLCFendRef which points to endnote text  in the endnote document
                        //       stream which corresponds with the PLCFendRef
    INT32 lcbPlcfendTxt;    // 0x1DE

    WW8_FC fcPlcffldEdn;    // 0x1E2 offset to PLCF of field positions in the endnote subdoc
    INT32 lcbPlcffldEdn;    // 0x1E6

    WW8_FC  fcPlcfpgdEdn;   // 0x1EA offset to PLCF of page boundaries in the endnote subdoc.
    INT32 lcbPlcfpgdEdn;        // 0x1EE


    WW8_FC fcDggInfo;           // offset in table stream of the office art object table data.
                                                // The format of office art object table data is found in a separate document.
                                                        // war in Ver67 nur leere Reserve
    INT32 lcbDggInfo;           // length in bytes of the office art object table data
                                                        // war in Ver67 nur leere Reserve

    WW8_FC fcSttbfRMark;        // 0x1fa offset to STTBF that records the author abbreviations...
    INT32 lcbSttbfRMark;        // 0x1fe
    WW8_FC fcSttbfCaption;  // 0x202 offset to STTBF that records caption titles...
    INT32 lcbSttbfCaption;  // 0x206
    WW8_FC fcSttbAutoCaption;   // offset in table stream to the STTBF that records the object names and
                                                        // indices into the caption STTBF for objects which get auto captions.
    INT32 lcbSttbAutoCaption;   // 0x20e

    WW8_FC fcPlcfwkb;       // 0x212 offset to PLCF that describes the boundaries of contributing documents...
    INT32 lcbPlcfwkb;       // 0x216

    WW8_FC fcPlcfspl;       // offset in table stream of PLCF (of SPLS structures) that records spell check state
                                                        // war in Ver67 nur leere Reserve
    INT32 lcbPlcfspl;                   // war in Ver67 nur leere Reserve

    WW8_FC fcPlcftxbxTxt;   // 0x222 ...PLCF of beginning CP in the text box subdoc
    INT32 lcbPlcftxbxTxt;   // 0x226
    WW8_FC fcPlcffldTxbx;   // 0x22a ...PLCF of field boundaries recorded in the textbox subdoc.
    INT32 lcbPlcffldTxbx;   // 0x22e
    WW8_FC fcPlcfHdrtxbxTxt;// 0x232 ...PLCF of beginning CP in the header text box subdoc
    INT32 lcbPlcfHdrtxbxTxt;// 0x236
    WW8_FC fcPlcffldHdrTxbx;// 0x23a ...PLCF of field boundaries recorded in the header textbox subdoc.
    INT32 lcbPlcffldHdrTxbx;// 0x23e

    /*
        spezielle Listenverwaltung fuer WW8
    */
    WW8_FC fcPlcfLst;       // 0x02e2 offset in the table stream of list format information.
    INT32 lcbPlcfLst;       // 0x02e6 length
    WW8_FC fcPlfLfo;        // 0x02ea offset in the table stream of list format override information.
    INT32 lcbPlfLfo;        // 0x02ee length
    /*
        spezielle Break-Verwaltung fuer Text-Box-Stories in WW8
    */
    WW8_FC fcPlcftxbxBkd;   // 0x02f2 PLCF fuer TextBox-Break-Deskriptoren im Maintext
    INT32 lcbPlcftxbxBkd;   // 0x02f6
    WW8_FC fcPlcfHdrtxbxBkd;// 0x02fa PLCF fuer TextBox-Break-Deskriptoren im Header-/Footer-Bereich
    INT32 lcbPlcfHdrtxbxBkd;// 0x02fe

    // 0x302 - 372 == ignore
    /*
        ListNames (skip to here!)
    */
    WW8_FC fcSttbListNames;// 0x0372 PLCF for Listname Table
    INT32 lcbSttbListNames;// 0x0376
    /*
        General-Varaiblen, die fuer Ver67 und Ver8 verwendet werden,
        obwohl sie in der jeweiligen DATEI verschiedene Groesse haben:
    */
    INT32 pnChpFirst;
    INT32 pnPapFirst;
    INT32 cpnBteChp;
    INT32 cpnBtePap;
    /*
        nun wird lediglich noch ein Ctor benoetigt
    */
    WW8Fib( SvStream& rStrm, BYTE nWantedVersion,UINT32 nOffset=0 );

    /* leider falsch, man braucht auch noch einen fuer den Export */
    WW8Fib( BYTE nVersion = 6 );
    BOOL Write( SvStream& rStrm );
};


class WW8Style
{
protected:
    WW8Fib& rFib;
    SvStream& rSt;
    long nStyleStart;
    long nStyleLen;
//  WW8_STSHI* pStishi;

    UINT16  cstd;                               // Count of styles in stylesheet
    UINT16  cbSTDBaseInFile;        // Length of STD Base as stored in a file
    UINT16  fStdStylenamesWritten : 1;  // Are built-in stylenames stored?
    UINT16  : 15;                   // Spare flags
    UINT16  stiMaxWhenSaved;        // Max sti known when this file was written
    UINT16  istdMaxFixedWhenSaved;  // How many fixed-index istds are there?
    UINT16  nVerBuiltInNamesWhenSaved;  // Current version of built-in stylenames
    UINT16  ftcStandardChpStsh;     // ftc used by StandardChpStsh for this document

public:
    WW8Style( SvStream& rSt, WW8Fib& rFibPara );
//  ~WW8Style(){ delete( pStishi ); pStishi = 0; }
    WW8_STD* Read1STDFixed( short& rSkip, short* pcbStd );
    WW8_STD* Read1Style( short& rSkip, String* pString, short* pcbStd );
    const UINT16 GetCount() const { return cstd; }
};


class WW8Fonts
{
protected:
    WW8_FFN* pFontA;        // Array of Pointers to Font Description
    // Hilfs-Vari
    USHORT nMax;        // Array-Groesse
public:
    WW8Fonts( SvStream& rSt, WW8Fib& rFib );
    ~WW8Fonts(){ if( pFontA ) __DELETE( nMax )pFontA; }
    const WW8_FFN* GetFont( USHORT nNum ) const;
    USHORT GetMax() const { return nMax; }
};


#define WW8_HEADER_EVEN 0x1
#define WW8_HEADER_ODD 0x2
#define WW8_FOOTER_EVEN 0x4
#define WW8_FOOTER_ODD 0x8
#define WW8_HEADER_FIRST 0x10
#define WW8_FOOTER_FIRST 0x20

class WW8Dop            // Document Properties
{
public:
    /*
        Fehlerstatus
    */
    ULONG nDopError;
    /*
        vom Ctor aus dem FIB gelesene Daten
        (entspricht nur ungefaehr der tatsaechlichen Struktur
         des Winword-FIB)
    */
    // Initialisier-Dummy:
    BYTE    nDataStart;
    //-------------------------
    UINT16  fFacingPages : 1;   // 1 when facing pages should be printed
    UINT16  fWidowControl : 1;  // 1 when widow control is in effect. 0 when widow control disabled.
    UINT16  fPMHMainDoc : 1;    // 1 when doc is a main doc for Print Merge Helper, 0 when not; default=0
    UINT16  grfSuppression : 2; // 0 Default line suppression storage; 0= form letter line suppression; 1= no line suppression; default=0
    UINT16  fpc : 2;            // 1 footnote position code: 0 as endnotes, 1 at bottom of page, 2 immediately beneath text
    UINT16  : 1;                // 0 unused
    //-------------------------
    UINT16  grpfIhdt : 8;           // 0 specification of document headers and footers. See explanation under Headers and Footers topic.
    //-------------------------
    UINT16  rncFtn : 2;         // 0 restart index for footnotes, 0 don't restart note numbering, 1 section, 2 page
    UINT16  nFtn : 14;          // 1 initial footnote number for document
    UINT16  fOutlineDirtySave : 1; // when 1, indicates that information in the hPLCFpad should be refreshed since outline has been dirtied
    UINT16  : 7;                //   reserved
    UINT16  fOnlyMacPics : 1;   //   when 1, Word believes all pictures recorded in the document were created on a Macintosh
    UINT16  fOnlyWinPics : 1;   //   when 1, Word believes all pictures recorded in the document were created in Windows
    UINT16  fLabelDoc : 1;      //   when 1, document was created as a print merge labels document
    UINT16  fHyphCapitals : 1;  //   when 1, Word is allowed to hyphenate words that are capitalized. When 0, capitalized may not be hyphenated
    UINT16  fAutoHyphen : 1;    //   when 1, Word will hyphenate newly typed text as a background task
    UINT16  fFormNoFields : 1;
    UINT16  fLinkStyles : 1;    //   when 1, Word will merge styles from its template
    UINT16  fRevMarking : 1;    //   when 1, Word will mark revisions as the document is edited
    UINT16  fBackup : 1;        //   always make backup when document saved when 1.
    UINT16  fExactCWords : 1;
    UINT16  fPagHidden : 1;     //
    UINT16  fPagResults : 1;
    UINT16  fLockAtn : 1;       //   when 1, annotations are locked for editing
    UINT16  fMirrorMargins : 1; //   swap margins on left/right pages when 1.
    UINT16  fReadOnlyRecommended : 1;// user has recommended that this doc be opened read-only when 1
    UINT16  fDfltTrueType : 1;  //   when 1, use TrueType fonts by default (flag obeyed only when doc was created by WinWord 2.x)
    UINT16  fPagSuppressTopSpacing : 1;//when 1, file created with SUPPRESSTOPSPACING=YES in win.ini. (flag obeyed only when doc was created by WinWord 2.x).
    UINT16  fProtEnabled : 1;   //   when 1, document is protected from edit operations
    UINT16  fDispFormFldSel : 1;//   when 1, restrict selections to occur only within form fields
    UINT16  fRMView : 1;        //   when 1, show revision markings on screen
    UINT16  fRMPrint : 1;       //   when 1, print revision marks when document is printed
    UINT16  fWriteReservation : 1;
    UINT16  fLockRev : 1;       //   when 1, the current revision marking state is locked
    UINT16  fEmbedFonts : 1;    //   when 1, document contains embedded True Type fonts
    //    compatability options
    UINT16 copts_fNoTabForInd : 1;          //    when 1, don’t add automatic tab stops for hanging indent
    UINT16 copts_fNoSpaceRaiseLower : 1;        //    when 1, don’t add extra space for raised or lowered characters
    UINT16 copts_fSupressSpbfAfterPgBrk : 1;    // when 1, supress the paragraph Space Before and Space After options after a page break
    UINT16 copts_fWrapTrailSpaces : 1;      //    when 1, wrap trailing spaces at the end of a line to the next line
    UINT16 copts_fMapPrintTextColor : 1;        //    when 1, print colors as black on non-color printers
    UINT16 copts_fNoColumnBalance : 1;      //    when 1, don’t balance columns for Continuous Section starts
    UINT16 copts_fConvMailMergeEsc : 1;
    UINT16 copts_fSupressTopSpacing : 1;        //    when 1, supress extra line spacing at top of page
    UINT16 copts_fOrigWordTableRules : 1;   //    when 1, combine table borders like Word 5.x for the Macintosh
    UINT16 copts_fTransparentMetafiles : 1; //    when 1, don’t blank area between metafile pictures
    UINT16 copts_fShowBreaksInFrames : 1;   //    when 1, show hard page or column breaks in frames
    UINT16 copts_fSwapBordersFacingPgs : 1; //    when 1, swap left and right pages on odd facing pages

    INT16  dxaTab;              // 720 twips    default tab width
    UINT16 wSpare;              //
    UINT16 dxaHotZ;         //      width of hyphenation hot zone measured in twips
    UINT16 cConsecHypLim;       //      number of lines allowed to have consecutive hyphens
    UINT16 wSpare2;         //      reserved
    INT32   dttmCreated;        // DTTM date and time document was created
    INT32   dttmRevised;        // DTTM date and time document was last revised
    INT32   dttmLastPrint;      // DTTM date and time document was last printed
    INT16   nRevision;          //      number of times document has been revised since its creation
    INT32   tmEdited;           //      time document was last edited
    INT32   cWords;             //      count of words tallied by last Word Count execution
    INT32   cCh;                //      count of characters tallied by last Word Count execution
    INT16   cPg;                //      count of pages tallied by last Word Count execution
    INT32   cParas;             //      count of paragraphs tallied by last Word Count execution
    UINT16 rncEdn : 2;          //      restart endnote number code: 0 don’t restart endnote numbering, 1 section, 2 page
    UINT16 nEdn : 14;           //      beginning endnote number
    UINT16 epc : 2;         //      endnote position code: 0 at end of section, 3 at end of document
    // UINT16 nfcFtnRef : 4;        //      number format code for auto footnotes: 0 Arabic, 1 Upper case Roman, 2 Lower case Roman
                                //      3 Upper case Letter, 4 Lower case Letter
                                // ersetzt durch gleichlautendes Feld unten
    // UINT16 nfcEdnRef : 4;        //      number format code for auto endnotes: 0 Arabic, 1 Upper case Roman, 2 Lower case Roman
                                //      3 Upper case Letter, 4 Lower case Letter
                                // ersetzt durch gleichlautendes Feld unten
    UINT16 fPrintFormData : 1;  //      only print data inside of form fields
    UINT16 fSaveFormData : 1;   //      only save document data that is inside of a form field.
    UINT16 fShadeFormData : 1;  //      shade form fields
    UINT16 : 2;             //      reserved
    UINT16 fWCFtnEdn : 1;       //      when 1, include footnotes and endnotes in word count
    INT32   cLines;             //      count of lines tallied by last Word Count operation
    INT32   cWordsFtnEnd;       //      count of words in footnotes and endnotes tallied by last Word Count operation
    INT32   cChFtnEdn;          //      count of characters in footnotes and endnotes tallied by last Word Count operation
    INT16   cPgFtnEdn;          //      count of pages in footnotes and endnotes tallied by last Word Count operation
    INT32   cParasFtnEdn;       //      count of paragraphs in footnotes and endnotes tallied by last Word Count operation
    INT32   cLinesFtnEdn;       //      count of paragraphs in footnotes and endnotes tallied by last Word Count operation
    INT32   lKeyProtDoc;        //      document protection password key, only valid if dop.fProtEnabled, dop.fLockAtn or dop.fLockRev are 1.
    UINT16  wvkSaved : 3;       //      document view kind: 0 Normal view, 1 Outline view, 2 Page View
    UINT16  wScaleSaved : 9;    //
    UINT16  zkSaved : 2;

    // hier sollte bei nFib < 103   Schluss sein, sonst ist Datei fehlerhaft!

    /*
        bei nFib >= 103 gehts weiter:
    */
    UINT32 fNoTabForInd                             :1; // see above in compatability options
    UINT32 fNoSpaceRaiseLower                   :1; // see above
    UINT32 fSupressSpbfAfterPageBreak   :1; // see above
    UINT32 fWrapTrailSpaces                     :1; // see above
    UINT32 fMapPrintTextColor                   :1; // see above
    UINT32 fNoColumnBalance                     :1; // see above
    UINT32 fConvMailMergeEsc                    :1; // see above
    UINT32 fSupressTopSpacing                   :1; // see above
    UINT32 fOrigWordTableRules              :1; // see above
    UINT32 fTransparentMetafiles            :1; // see above
    UINT32 fShowBreaksInFrames              :1; // see above
    UINT32 fSwapBordersFacingPgs            :1; // see above
    UINT32                                                      :4; // reserved
    UINT32 fSuppressTopSpacingMac5      :1; // Suppress extra line spacing at top
                                                                                // of page like MacWord 5.x
    UINT32 fTruncDxaExpand                      :1; // Expand/Condense by whole number of points
    UINT32 fPrintBodyBeforeHdr              :1; // Print body text before header/footer
    UINT32 fNoLeading                                   :1; // Don't add extra spacebetween rows of text
    UINT32                                                      :1; // reserved
    UINT32 fMWSmallCaps                             :1; // Use larger small caps like MacWord 5.x
    UINT32                                                   :10;// reserved

    // hier sollte bei nFib <= 105  Schluss sein, sonst ist Datei fehlerhaft!

    /*
        bei nFib > 105 gehts weiter:
    */
    INT16   adt;                            // Autoformat Document Type:
                                                    // 0 for normal. 1 for letter, and 2 for email.
    WW8_DOPTYPOGRAPHY doptypography;    // siehe WW8STRUC.HXX
    WW8_DOGRID        dogrid;                   // siehe WW8STRUC.HXX
    UINT16                      :1; // reserved
    UINT16 lvl                  :4; // Which outline levels are showing in outline view
    UINT16                      :4; // reserved
    UINT16 fHtmlDoc             :1; // This file is based upon an HTML file
    UINT16                      :1; // reserved
    UINT16 fSnapBorder          :1; // Snap table and page borders to page border
    UINT16 fIncludeHeader       :1; // Place header inside page border
    UINT16 fIncludeFooter       :1; // Place footer inside page border
    UINT16 fForcePageSizePag    :1; // Are we in online view
    UINT16 fMinFontSizePag      :1; // Are we auto-promoting
                                                                // fonts to >= hpsZoonFontPag?
    UINT16 fHaveVersions            :1; // versioning is turned on
    UINT16 fAutoVersion             :1; // autoversioning is enabled
    UINT16                                   :14;   // reserved
    // hier 12 Byte ueberspringen: ASUMI
    INT32 cChWS;
    INT32 cChWSFtnEdn;
    INT32 grfDocEvents;
    // hier 4+30+8 Bytes ueberspringen
    INT32 cDBC;
    INT32 cDBCFtnEdn;
    // hier 4 Bytes ueberspringen
    INT16 nfcFtnRef;
    INT16 nfcEdnRef;
    INT16 hpsZoonFontPag;
    INT16 dywDispPag;

    // 2. Initialisier-Dummy:
    BYTE    nDataEnd;

    /*
        nun wird lediglich noch ein Ctor benoetigt,
        dem die FIB-Nummer uebergeben werden muss
    */
    WW8Dop( SvStream& rSt, INT16 nFib, INT32 nPos, INT32 nSize );

    /* leider falsch, man braucht auch noch einen fuer den Export */
    WW8Dop();
    BOOL Write( SvStream& rStrm, WW8Fib& rFib );
};


inline short WW8SkipOdd(SvStream* pSt )
{
#if defined HP9000 || defined SINIX
    short bRet = pSt->Tell() & 0x1;
    if (bRet) {
        UINT8 c;
        pSt->Read( &c, 1 );
    }
    return bRet;
#else
    if ( pSt->Tell() & 0x1 ){
        UINT8 c;
        pSt->Read( &c, 1 );
        return 1;
    }
    return 0;
#endif
}


class WW8PLCF_HdFt
{
    WW8PLCF aPLCF;
    long nTextOfs;
    short nIdxOffset;

public:
    WW8PLCF_HdFt( SvStream* pSt, WW8Fib& rFib, WW8Dop& rDop );
//  ~WW8PLCF_HdFt() {}
    BOOL GetTextPos( BYTE grpfIhdt, BYTE nWhich, WW8_CP& rStart, long& rLen );
    BOOL GetTextPosExact( short nIdx, WW8_CP& rStart, long& rLen );
    void UpdateIndex( BYTE grpfIhdt );
};


/*************************************************************************
      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/ww8scan.hxx,v 1.2 2000-10-24 14:01:34 jp Exp $


      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.58  2000/09/18 16:05:02  willem.vandorp
      OpenOffice header added.

      Revision 1.57  2000/08/24 06:43:01  khz
      #75708# advance StartPos of attributes starting on a Table-Row-End by 1

      Revision 1.56  2000/07/12 12:24:54  khz
      #76503# use WW8PLCFx_Cp_FKP::?etIdx2() to save/restore nIdx of pPcd

      Revision 1.55  2000/05/31 12:23:05  khz
      Changes for Unicode

      Revision 1.54  2000/05/25 08:06:37  khz
      Piece Table optimization, Unicode changes, Bugfixes

      Revision 1.53  2000/05/18 10:58:50  jp
      Changes for Unicode

      Revision 1.52  2000/05/16 12:13:13  jp
      ASS_FALSE define removed

      Revision 1.51  2000/05/16 11:07:47  khz
      Unicode code-conversion

      Revision 1.50  2000/05/12 08:29:03  khz
      Changes for Unicode

      Revision 1.49  2000/05/04 07:32:05  khz
      Task #75379# added Save() and Restore() to WW8PLCFx, WW8PLCFx_Cp_FKP, WW8PLCFxDesc

      Revision 1.48  2000/02/22 16:23:14  khz
      Task #72987# Ignore Sprms covering ONLY a Tab-Row-End Char #7

      Revision 1.47  2000/02/09 08:57:36  khz
      Task #72647# Read SPRMs that are stored in piece table grpprls (2)

      Revision 1.46  2000/02/03 10:08:31  cmc
      #72268# Allow create fib from offset, added data to STTBF

      Revision 1.45  2000/02/02 18:07:51  khz
      Task #69885# Read SPRMs that are stored in piece table grpprls

      Revision 1.44  1999/12/07 14:29:04  khz
      Task #69508# import sprmPHugePapx by reading DATA stream

      Revision 1.43  1999/11/19 15:09:24  khz
      Task #69910# Use extra PLCF structure for field!

      Revision 1.42  1999/11/03 17:18:02  khz
      import new TOX_CONTENT and TOX_INDEX features (2)

      Revision 1.41  1999/10/21 15:36:21  khz
      Import Redlining (4)

      Revision 1.40  1999/10/13 21:06:47  khz
      Import Redlining (3)

      Revision 1.39  1999/10/08 09:25:29  khz
      Import Redlining

      Revision 1.38  1999/09/10 15:36:39  khz
      CharSet matching made by TENCINFO.H::rtl_getTextEncodingFromWindowsCharset()

      Revision 1.37  1999/09/09 18:16:13  khz
      CharSet matching now done in central methode WW8SCAN.HXX::WW8GetCharSet()

      Revision 1.36  1999/09/08 13:26:17  khz
      Better performance by reducing use of SWAP..() and SVBT..To..() methods

      Revision 1.35  1999/08/30 19:53:04  JP
      Bug #68219#: no static members - be reentrant


      Rev 1.34   30 Aug 1999 21:53:04   JP
   Bug #68219#: no static members - be reentrant

      Rev 1.33   09 Aug 1999 18:35:10   KHZ
   Task #67543# Import of Property Modifier(variant 1) (PRM) in WW8 docs

      Rev 1.32   15 Jun 1999 14:34:58   JP
   for Export: FIB - read/write SttbListNames

      Rev 1.31   02 Jun 1999 09:32:44   KHZ
   Task #66227# a) kein Unicode bei Ver67 ;-)  b) Grafik in grupp. Textbox

      Rev 1.30   19 May 1999 11:12:56   JP
   WinWord97-ExportFilter

      Rev 1.29   28 Apr 1999 23:03:28   KHZ
   Task #65245# Breite eines Rahmens bei WW 'automatisch' muss PRTAREA sein

      Rev 1.28   26 Feb 1999 14:43:36   KHZ
   Task #59715# Behandlung von Section breaks

      Rev 1.27   25 Jan 1999 10:22:38   KHZ
   Task #60715# in Textobjekt verankerte Grafik als Grafik importieren

      Rev 1.26   18 Jan 1999 08:53:08   KHZ
   Task #60878# WW8Read_xstz nur dann Assertion, wenn kein pTestFlag

      Rev 1.25   10 Dec 1998 21:29:02   JP
   Bug #59643#: benutzerdefinierte Zeichen von Fuss-/EndNoten verarbeiten

      Rev 1.24   05 Dec 1998 17:10:26   KHZ
   Task #59580# Unicode (3)

      Rev 1.23   04 Dec 1998 20:19:48   KHZ
   Task #58766# Textboxen mit Unicode-Inhalt

      Rev 1.22   03 Dec 1998 19:10:00   KHZ
   Task #58766# Unicode-Import (2)

      Rev 1.20   30 Nov 1998 17:46:52   JP
   Task #59822#: OLE-Objecte importieren

      Rev 1.21   02 Dec 1998 15:34:32   JP
   Task #60063#: Kommentare als PostIts einlesen

      Rev 1.20   30 Nov 1998 17:46:52   JP
   Task #59822#: OLE-Objecte importieren

      Rev 1.19   30 Nov 1998 17:30:36   KHZ
   Task #54828# Unicode-Import

      Rev 1.18   03 Nov 1998 18:29:52   KHZ
   Task #57243# Performance-Gewinn durch Vermeiden ueberfluessiger WW8GetSprmId()

      Rev 1.17   02 Nov 1998 17:58:32   KHZ
   Task #57017# Textmarken-Namen als UNICODE-Strings

      Rev 1.16   16 Oct 1998 16:12:34   KHZ
   Task #53520# ueberpruefe Feld DRUCKDATUM in DocInfo anhand von WW-internem Feld

      Rev 1.15   22 Sep 1998 17:40:22   KHZ
   Bug #56310# Kopf-/Fusszeilen in Unicode-Dokumenten jetzt korrekt

      Rev 1.14   03 Sep 1998 22:15:40   KHZ
   Task #55189# Textboxen

      Rev 1.13   11 Aug 1998 12:25:16   KHZ
   Task #52607# Optimierung in AdjustEnds(), GetNewSprms(), GetNoNewSprms()

      Rev 1.12   30 Jul 1998 00:02:54   KHZ
   Task #53614# Grafiken, die NICHT ueber dem Text liegen (MSDFF-Quick-Hack)

      Rev 1.11   28 Jul 1998 11:05:46   KHZ
   Task #52607# nummerierte Listen (Teil 1)

      Rev 1.10   22 Jul 1998 15:39:32   KHZ
   Task #52607#

      Rev 1.9   22 Jul 1998 15:33:28   KHZ
   Task #52607#

      Rev 1.8   21 Jul 1998 14:52:30   KHZ
   Task #52607# (WW 97 Import)

      Rev 1.7   21 Jul 1998 12:32:54   KHZ
   als MSDrawingObject eingebettete Grafik (Teil 1)

      Rev 1.6   09 Jul 1998 20:14:56   KHZ
   Tabellen: verbundene Zellen und Zellen-Hintergrundfarbe jetzt Ok.

      Rev 1.5   30 Jun 1998 21:33:22   KHZ
   Header/Footer/Footnotes weitgehend ok

      Rev 1.4   26 Jun 1998 20:50:20   KHZ
   Absatz-Attribute jetzt weitestgehend ok

      Rev 1.3   23 Jun 1998 20:49:24   KHZ
   verarbeitet jetzt auch mehrere FKPs

      Rev 1.2   23 Jun 1998 11:24:20   KHZ
   Zwischenstand: die meisten Zeichenattribute Ok!

      Rev 1.1   16 Jun 1998 18:37:14   KHZ
   DaSi-Stand

      Rev 1.0   16 Jun 1998 11:12:10   KHZ
   Initial revision.

      Rev 1.1   10 Jun 1998 17:22:34   KHZ
   Zwischenstand-Sicherung Dumper

      Rev 1.0   27 May 1998 15:29:16   KHZ
   Initial revision.


*************************************************************************/

#endif


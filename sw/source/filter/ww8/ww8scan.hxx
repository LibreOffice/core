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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8SCAN_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8SCAN_HXX

#include <cassert>
#include <cstddef>
#include <deque>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

#include <osl/endian.h>
#include <tools/solar.h>
#include <tools/stream.hxx>
#include <rtl/ustring.hxx>

#include "ww8struc.hxx"
#include "types.hxx"

class SvStream;

//Commonly used string literals for stream and storage names in word docs
namespace SL
{
    const char aObjectPool[]    = "ObjectPool";
    const char a1Table[]        = "1Table";
    const char a0Table[]        = "0Table";
    const char aData[]          = "Data";
    const char aCheckBox[]      = "CheckBox";
    const char aListBox[]       = "ListBox";
    const char aTextField[]     = "TextField";
    const char aMSMacroCmds[]   = "MSMacroCmds";
}

struct SprmInfo
{
    unsigned int nLen : 6;
    unsigned int nVari : 2;
};

struct SprmInfoRow {
    sal_uInt16 nId;         ///< A ww8 sprm is hardcoded as 16bits
    SprmInfo info;
};

class wwSprmSearcher {
public:
    //see Read_AmbiguousSPRM for the bPatchCJK oddity
    wwSprmSearcher(SprmInfoRow const * rows, std::size_t size, bool bPatchCJK = false) {
        for (std::size_t i = 0; i != size; ++i) {
            bool ins = map_.emplace(rows[i].nId, rows[i].info).second;
            assert(ins); (void) ins;
        }
        if (bPatchCJK)
            patchCJKVariant();
    }

    SprmInfo const * search(sal_uInt16 id) const {
        Map::const_iterator i(map_.find(id));
        return i == map_.end() ? nullptr : &i->second;
    }

private:
    typedef std::unordered_map<sal_uInt16, SprmInfo> Map;

    Map map_;

    void patchCJKVariant();
};

class WW8Fib;

struct SprmResult
{
    const sal_uInt8* pSprm;
    sal_Int32 nRemainingData;
    SprmResult()
        : pSprm(nullptr)
        , nRemainingData(0)
    {
    }
    SprmResult(const sal_uInt8* pInSprm, sal_Int32 nInRemainingData)
        : pSprm(pInSprm)
        , nRemainingData(nInRemainingData)
    {
    }
};

/**
    wwSprmParser knows how to take a sequence of bytes and split it up into
    sprms and their arguments
*/
class wwSprmParser
{
private:
    ww::WordVersion meVersion;
    sal_uInt8 mnDelta;
    const wwSprmSearcher *mpKnownSprms;
    static const wwSprmSearcher* GetWW8SprmSearcher();
    static const wwSprmSearcher* GetWW6SprmSearcher(const WW8Fib& rFib);
    static const wwSprmSearcher* GetWW2SprmSearcher();

    SprmInfo GetSprmInfo(sal_uInt16 nId) const;

    sal_uInt8 SprmDataOfs(sal_uInt16 nId) const;

public:
    enum SprmType {L_FIX=0, L_VAR=1, L_VAR2=2};

    //7- ids are very different to 8+ ones
    explicit wwSprmParser(const WW8Fib& rFib);
    /// Return the SPRM id at the beginning of this byte sequence
    sal_uInt16 GetSprmId(const sal_uInt8* pSp) const;

    sal_Int32 GetSprmSize(sal_uInt16 nId, const sal_uInt8* pSprm, sal_Int32 nRemLen) const;

    /// Get known len of a sprms head, the bytes of the sprm id + any bytes
    /// reserved to hold a variable length
    sal_Int32 DistanceToData(sal_uInt16 nId) const;

    /// Get len of a sprms data area, ignoring the bytes of the sprm id and
    /// ignoring any len bytes. Reports the remaining data after those bytes
    sal_uInt16 GetSprmTailLen(sal_uInt16 nId, const sal_uInt8* pSprm, sal_Int32 nRemLen) const;

    /// The minimum acceptable sprm len possible for this type of parser
    int MinSprmLen() const { return (IsSevenMinus(meVersion)) ? 2 : 3; }

    /// Returns the offset to data of the first sprm of id nId, 0
    //  if not found. nLen must be the <= length of pSprms
    SprmResult findSprmData(sal_uInt16 nId, sal_uInt8* pSprms, sal_Int32 nLen) const;
};

//Read a Pascal-style, i.e. single byte string length followed
//by string contents
inline OUString read_uInt8_PascalString(SvStream& rStrm, rtl_TextEncoding eEnc)
{
    return read_uInt8_lenPrefixed_uInt8s_ToOUString(rStrm, eEnc);
}

inline OUString read_uInt16_PascalString(SvStream& rStrm)
{
    return read_uInt16_lenPrefixed_uInt16s_ToOUString(rStrm);
}

//Belt and Braces strings, i.e. Pascal-style strings followed by
//null termination, Spolsky calls them "fucked strings" FWIW
//http://www.joelonsoftware.com/articles/fog0000000319.html
OUString read_uInt8_BeltAndBracesString(SvStream& rStrm, rtl_TextEncoding eEnc);
OUString read_uInt16_BeltAndBracesString(SvStream& rStrm);

//--Line above which the code has meaningful comments

class  WW8ScannerBase;
class  WW8PLCFspecial;
struct WW8PLCFxDesc;
class  WW8PLCFx_PCD;

/**
 reads array of strings (see MS documentation: String Table stored in File)
 returns NOT the original pascal strings but an array of converted char*

 attention: the *extra data* of each string are SKIPPED and ignored
 */
void WW8ReadSTTBF(bool bVer8, SvStream& rStrm, sal_uInt32 nStart, sal_Int32 nLen,
    sal_uInt16 nExtraLen, rtl_TextEncoding eCS, std::vector<OUString> &rArray,
    std::vector<ww::bytes>* pExtraArray = nullptr, std::vector<OUString>* pValueArray = nullptr);

struct WW8FieldDesc
{
    WW8_CP nLen;            ///< total length (to skip over text)
    WW8_CP nSCode;          ///< start of instructions code
    WW8_CP nLCode;          ///< length
    WW8_CP nSRes;           ///< start of result
    WW8_CP nLRes;           ///< length ( == 0, if no result )
    sal_uInt16 nId;         ///< WW-id for fields
    sal_uInt8 nOpt;         ///< WW-Flags ( e.g.: changed by user )
    bool bCodeNest:1;       ///< instruction used recursively
    bool bResNest:1;        ///< instruction inserted into result
};

struct WW8PLCFxSave1
{
    sal_uInt32 nPLCFxPos;
    sal_uInt32 nPLCFxPos2;       ///< for PLCF_Cp_Fkp: PieceIter-Pos
    long nPLCFxMemOfs;
    WW8_CP nStartCp;        ///< for cp based iterator like PAP and CHP
    long nCpOfs;
    WW8_FC nStartFC;
    WW8_CP nAttrStart;
    WW8_CP nAttrEnd;
    bool bLineEnd;
};

/**
    among others for fields, that is, the same number of attr as positions,
    if Ctor-Param bNoEnd = false
*/
class WW8PLCFspecial        // iterator for PLCFs
{
private:
    std::unique_ptr<sal_Int32[]> pPLCF_PosArray;  ///< pointer to Pos-array and to the whole structure
    sal_uInt8*  pPLCF_Contents;  ///< pointer to content-array-part of Pos-array
    long nIMax;             ///< number of elements
    long nIdx;              ///< marker where we currently are
    sal_uInt32 nStru;

    WW8PLCFspecial(const WW8PLCFspecial&) = delete;
    WW8PLCFspecial& operator=(const WW8PLCFspecial&) = delete;

public:
    WW8PLCFspecial(SvStream* pSt, sal_uInt32 nFilePos, sal_uInt32 nPLCF,
        sal_uInt32 nStruct);
    long GetIdx() const { return nIdx; }
    void SetIdx( long nI ) { nIdx = nI; }
    long GetIMax() const { return nIMax; }
    bool SeekPos(long nPos);            // walks over FC- or CP-value
                                        // resp. next biggest value
    bool SeekPosExact(long nPos);
    sal_Int32 Where() const
        { return ( nIdx >= nIMax ) ? SAL_MAX_INT32 : pPLCF_PosArray[nIdx]; }
    bool Get(WW8_CP& rStart, void*& rpValue) const;
    bool GetData(long nIdx, WW8_CP& rPos, void*& rpValue) const;

    const void* GetData( long nInIdx ) const
    {
        return ( nInIdx >= nIMax ) ? nullptr
            : static_cast<const void*>(&pPLCF_Contents[nInIdx * nStru]);
    }
    sal_Int32 GetPos( long nInIdx ) const
        { return ( nInIdx >= nIMax ) ? SAL_MAX_INT32 : pPLCF_PosArray[nInIdx]; }

    void advance()
    {
        if (nIdx <= nIMax)
            ++nIdx;
    }
};

/** simple Iterator for SPRMs */
class WW8SprmIter
{
private:
    const wwSprmParser &mrSprmParser;
    // these members will be updated
    const sal_uInt8* pSprms; // remaining part of the SPRMs ( == start of current SPRM)
    const sal_uInt8* pCurrentParams; // start of current SPRM's parameters
    sal_uInt16 nCurrentId;
    sal_Int32 nCurrentSize;

    sal_Int32 nRemLen;   // length of remaining SPRMs (including current SPRM)

    void UpdateMyMembers();

public:
    explicit WW8SprmIter(const sal_uInt8* pSprms_, sal_Int32 nLen_,
        const wwSprmParser &rSprmParser);
    void  SetSprms(const sal_uInt8* pSprms_, sal_Int32 nLen_);
    SprmResult FindSprm(sal_uInt16 nId, bool bFindFirst, const sal_uInt8* pNextByteMatch = nullptr);
    void  advance();
    const sal_uInt8* GetSprms() const
        { return ( pSprms && (0 < nRemLen) ) ? pSprms : nullptr; }
    const sal_uInt8* GetCurrentParams() const { return pCurrentParams; }
    sal_uInt16 GetCurrentId() const { return nCurrentId; }
    sal_Int32 GetRemLen() const { return nRemLen; }

private:
    WW8SprmIter(const WW8SprmIter&) = delete;
    WW8SprmIter& operator=(const WW8SprmIter&) = delete;
};

/* among others for FKPs to normal attr., i.e. one less attr than positions */
class WW8PLCF                       // Iterator for PLCFs
{
private:
    std::unique_ptr<WW8_CP[]> pPLCF_PosArray; // pointer to Pos-array and the whole structure
    sal_uInt8* pPLCF_Contents;   // pointer to content-array-part of Pos-array
    sal_Int32 nIMax;            // number of elements
    sal_Int32 nIdx;
    int nStru;

    void ReadPLCF(SvStream& rSt, WW8_FC nFilePos, sal_uInt32 nPLCF);

    /*
        If a PLC is missing in the doc and the FKPs stand alone,
        we create a PLC with this:
    */
    void GeneratePLCF(SvStream& rSt, sal_Int32 nPN, sal_Int32 ncpN);

    void MakeFailedPLCF();

    void TruncToSortedRange();
public:
    WW8PLCF(SvStream& rSt, WW8_FC nFilePos, sal_Int32 nPLCF, int nStruct,
        WW8_CP nStartPos = -1);

    /*
        the following ctor generates a PLC from nPN and ncpN, if necessary
    */
    WW8PLCF(SvStream& rSt, WW8_FC nFilePos, sal_Int32 nPLCF, int nStruct,
        WW8_CP nStartPos, sal_Int32 nPN, sal_Int32 ncpN);

    sal_Int32 GetIdx() const { return nIdx; }
    void SetIdx( sal_Int32 nI ) { nIdx = nI; }
    sal_Int32 GetIMax() const { return nIMax; }
    bool SeekPos(WW8_CP nPos);
    WW8_CP Where() const;
    bool Get(WW8_CP& rStart, WW8_CP& rEnd, void*& rpValue) const;
    void advance() { if( nIdx < nIMax ) ++nIdx; }

    const void* GetData( sal_Int32 nInIdx ) const
    {
        return ( nInIdx >= nIMax ) ? nullptr :
            static_cast<const void*>(&pPLCF_Contents[nInIdx * nStru]);
    }
};

/* for Piece Table (.i.e. FastSave Table) */
class WW8PLCFpcd
{
    friend class WW8PLCFpcd_Iter;

    std::unique_ptr<sal_Int32[]> pPLCF_PosArray;  // pointer to Pos-array and the whole structure
    sal_uInt8*  pPLCF_Contents;  // pointer to content-array-part of Pos-array
    long nIMax;
    sal_uInt32 nStru;

    WW8PLCFpcd(const WW8PLCFpcd&) = delete;
    WW8PLCFpcd& operator=(const WW8PLCFpcd&) = delete;

public:
    WW8PLCFpcd(SvStream* pSt, sal_uInt32 nFilePos, sal_uInt32 nPLCF,
        sal_uInt32 nStruct);
};

/* multiple WW8PLCFpcd_Iter may point to the same WW8PLCFpcd !!!  */
class WW8PLCFpcd_Iter
{
private:
    WW8PLCFpcd& rPLCF;
    long nIdx;

    WW8PLCFpcd_Iter(const WW8PLCFpcd_Iter&) = delete;
    WW8PLCFpcd_Iter& operator=(const WW8PLCFpcd_Iter&) = delete;

public:
    WW8PLCFpcd_Iter( WW8PLCFpcd& rPLCFpcd, long nStartPos = -1 );
    long GetIdx() const { return nIdx; }
    void SetIdx( long nI ) { nIdx = nI; }
    long GetIMax() const { return rPLCF.nIMax; }
    bool SeekPos(long nPos);
    sal_Int32 Where() const;
    bool Get(WW8_CP& rStart, WW8_CP& rEnd, void*& rpValue) const;
    void advance()
    {
        if( nIdx < rPLCF.nIMax )
            ++nIdx;
    }
};

// PLCF-type:
enum ePLCFT{ CHP=0, PAP, SEP, /*HED, FNR, ENR,*/ PLCF_END };

//It's hardcoded that eFTN be the first one: a very poor hack, needs to be fixed
enum eExtSprm { eFTN = 256, eEDN = 257, eFLD = 258, eBKN = 259, eAND = 260, eATNBKN = 261, eFACTOIDBKN = 262 };

/*
    pure virtual:
*/
class WW8PLCFx              // virtual iterator for Piece Table Exceptions
{
private:
    const WW8Fib& mrFib;
    bool bIsSprm;           // PLCF of Sprms or other stuff ( Footnote, ... )
    WW8_FC nStartFc;
    bool bDirty;

    WW8PLCFx(const WW8PLCFx&) = delete;
    WW8PLCFx& operator=(const WW8PLCFx&) = delete;

public:
    WW8PLCFx(const WW8Fib& rFib, bool bSprm)
        : mrFib(rFib)
        , bIsSprm(bSprm)
        , nStartFc(-1)
        , bDirty(false)
    {
    }
    virtual ~WW8PLCFx() {}
    bool IsSprm() const { return bIsSprm; }
    virtual sal_uInt32 GetIdx() const = 0;
    virtual void SetIdx(sal_uInt32 nIdx) = 0;
    virtual sal_uInt32 GetIdx2() const;
    virtual void SetIdx2(sal_uInt32 nIdx);
    virtual bool SeekPos(WW8_CP nCpPos) = 0;
    virtual WW8_FC Where() = 0;
    virtual void GetSprms( WW8PLCFxDesc* p );
    virtual long GetNoSprms( WW8_CP& rStart, WW8_CP&, sal_Int32& rLen );
    virtual void advance() = 0;
    virtual sal_uInt16 GetIstd() const { return 0xffff; }
    virtual void Save( WW8PLCFxSave1& rSave ) const;
    virtual void Restore( const WW8PLCFxSave1& rSave );
    ww::WordVersion GetFIBVersion() const;
    const WW8Fib& GetFIB() const { return mrFib; }
    void SetStartFc( WW8_FC nFc ) { nStartFc = nFc; }
    WW8_FC GetStartFc() const { return nStartFc; }
    void SetDirty(bool bIn) {bDirty=bIn;}
    bool GetDirty() const {return bDirty;}
};

class WW8PLCFx_PCDAttrs : public WW8PLCFx
{
private:
    WW8PLCFpcd_Iter* pPcdI;
    WW8PLCFx_PCD* pPcd;
    std::vector<std::unique_ptr<sal_uInt8[]>> const & mrGrpprls; // attribute of Piece-table
    SVBT32 aShortSprm;          // mini storage: can contain ONE sprm with
                                // 1 byte param

    WW8PLCFx_PCDAttrs(const WW8PLCFx_PCDAttrs&) = delete;
    WW8PLCFx_PCDAttrs& operator=(const WW8PLCFx_PCDAttrs&) = delete;

public:
    WW8PLCFx_PCDAttrs(const WW8Fib& rFib, WW8PLCFx_PCD* pPLCFx_PCD,
        const WW8ScannerBase* pBase );
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nI) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual void GetSprms( WW8PLCFxDesc* p ) override;
    virtual void advance() override;

    WW8PLCFpcd_Iter* GetIter() const { return pPcdI; }
};

class WW8PLCFx_PCD : public WW8PLCFx            // iterator for Piece table
{
private:
    std::unique_ptr<WW8PLCFpcd_Iter> pPcdI;
    bool bVer67;
    WW8_CP nClipStart;

    WW8PLCFx_PCD(const WW8PLCFx_PCD&) = delete;
    WW8PLCFx_PCD& operator=(const WW8PLCFx_PCD&) = delete;

public:
    WW8PLCFx_PCD(const WW8Fib& rFib, WW8PLCFpcd* pPLCFpcd,
        WW8_CP nStartCp, bool bVer67P);
    virtual ~WW8PLCFx_PCD() override;
    sal_uInt32 GetIMax() const;
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nI) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual long GetNoSprms( WW8_CP& rStart, WW8_CP&, sal_Int32& rLen ) override;
    virtual void advance() override;
    WW8_CP CurrentPieceStartFc2Cp( WW8_FC nStartPos );
    WW8_FC CurrentPieceStartCp2Fc( WW8_CP nCp );
    static void CurrentPieceFc2Cp(WW8_CP& rStartPos, WW8_CP& rEndPos,
        const WW8ScannerBase *pSBase);
    WW8PLCFpcd_Iter* GetPLCFIter() { return pPcdI.get(); }
    void SetClipStart(WW8_CP nIn) { nClipStart = nIn; }
    WW8_CP GetClipStart() const { return nClipStart; }

    static sal_Int32 TransformPieceAddress(long nfc, bool& bIsUnicodeAddress)
    {
        bIsUnicodeAddress = 0 == (0x40000000 & nfc);
        return bIsUnicodeAddress ?  nfc : (nfc & 0x3fffFFFF) / 2;
    }
};

/**
 Iterator for Piece Table Exceptions of Fkps
 works only with FCs, not with CPs !  ( Low-Level )
*/
class WW8PLCFx_Fc_FKP : public WW8PLCFx
{
public:
    class WW8Fkp        // Iterator for Formatted Disk Page
    {
    private:
        class Entry
        {
        public:
            WW8_FC mnFC;

            sal_uInt8* mpData;
            sal_uInt16 mnLen;
            sal_uInt16 mnIStd; // only for Fkp.Papx (actually Style-Nr)
            bool mbMustDelete;

            explicit Entry(WW8_FC nFC) : mnFC(nFC), mpData(nullptr), mnLen(0),
                mnIStd(0), mbMustDelete(false) {}
            Entry(const Entry &rEntry);
            ~Entry();
            bool operator<(const Entry& rEntry) const;
            Entry& operator=(const Entry& rEntry);
        };

        sal_uInt8 maRawData[512];
        std::vector<Entry> maEntries;

        long nItemSize;     // either 1 Byte or a complete BX

        // Offset in Stream where last read of 512 bytes took place
        long nFilePos;
        sal_uInt8 mnIdx;         // Pos marker
        ePLCFT ePLCF;
        sal_uInt8 mnIMax;         // number of entries
        int mnMustRemainCached;  // after SaveAllPLCFx, before RestoreAllPLCFx

        wwSprmParser maSprmParser;

        //Fill in an Entry with sanity testing
        void FillEntry(Entry &rEntry, std::size_t nDataOffset, sal_uInt16 nLen);

    public:
        WW8Fkp (const WW8Fib& rFib, SvStream* pFKPStrm,
            SvStream* pDataStrm, long _nFilePos, long nItemSiz, ePLCFT ePl,
            WW8_FC nStartFc);
        void Reset(WW8_FC nPos);
        long GetFilePos() const { return nFilePos; }
        sal_uInt8 GetIdx() const { return mnIdx; }
        void SetIdx(sal_uInt8 nI);
        bool SeekPos(WW8_FC nFc);
        WW8_FC Where() const
        {
            return (mnIdx < mnIMax) ? maEntries[mnIdx].mnFC : WW8_FC_MAX;
        }
        void advance()
        {
            if (mnIdx < mnIMax)
                ++mnIdx;
        }
        sal_uInt8* Get( WW8_FC& rStart, WW8_FC& rEnd, sal_Int32& rLen ) const;
        sal_uInt16 GetIstd() const { return maEntries[mnIdx].mnIStd; }

        /*
            returns a real pointer to the Sprm of type nId,
            if such a thing is in the Fkp.
        */
        sal_uInt8* GetLenAndIStdAndSprms(sal_Int32& rLen) const;

        /*
            calls GetLenAndIStdAndSprms()...
            2020 bFindFirst note: Normally the last SPRM takes effect, so I don't know why HasSprm always returned the first value!
            I don't dare to change the default due to regression potential (and slower in the few cases looking for a boolean result),
            but first thing to try is use FindFirst as false.
        */
        SprmResult HasSprm(sal_uInt16 nId, bool bFindFirst = true);
        void HasSprm(sal_uInt16 nId, std::vector<SprmResult> &rResult);

        const wwSprmParser &GetSprmParser() const { return maSprmParser; }

        void IncMustRemainCache() { ++mnMustRemainCached; }
        bool IsMustRemainCache() const { return mnMustRemainCached > 0; }
        void DecMustRemainCache() { --mnMustRemainCached; }
    };

private:
    SvStream* pFKPStrm;         // input file
    SvStream* pDataStrm;        // input file
    std::unique_ptr<WW8PLCF> pPLCF;
protected:
    WW8Fkp* pFkp;
private:

    /*
        Keep a cache of eMaxCache entries of previously seen pFkps, which
        speeds up considerably table parsing and load save plcfs for what turn
        out to be small text frames, which frames generally are

        size      : cache hits
        cache all : 19168 pap, 48 chp
        == 100    : 19166 pap, 48 chp
        == 50     : 18918 pap, 48 chp
        == 10     : 18549 pap, 47 chp
        == 5      : 18515 pap, 47 chp
    */
    std::deque<std::unique_ptr<WW8Fkp>> maFkpCache;
    enum Limits {eMaxCache = 50000};

    bool NewFkp();

    WW8PLCFx_Fc_FKP(const WW8PLCFx_Fc_FKP&) = delete;
    WW8PLCFx_Fc_FKP& operator=(const WW8PLCFx_Fc_FKP&) = delete;

protected:
    ePLCFT ePLCF;
    std::unique_ptr<WW8PLCFx_PCDAttrs> pPCDAttrs;

public:
    WW8PLCFx_Fc_FKP( SvStream* pSt, SvStream* pTableSt, SvStream* pDataSt,
        const WW8Fib& rFib, ePLCFT ePl, WW8_FC nStartFcL );
    virtual ~WW8PLCFx_Fc_FKP() override;
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_FC nFcPos) override;
    virtual WW8_FC Where() override;
    sal_uInt8* GetSprmsAndPos( WW8_FC& rStart, WW8_FC& rEnd, sal_Int32& rLen );
    virtual void advance() override;
    virtual sal_uInt16 GetIstd() const override;
    void GetPCDSprms( WW8PLCFxDesc& rDesc );
    SprmResult HasSprm(sal_uInt16 nId, bool bFindFirst = true);
    void HasSprm(sal_uInt16 nId, std::vector<SprmResult> &rResult);
    bool HasFkp() const { return (nullptr != pFkp); }
};

/// iterator for Piece Table Exceptions of Fkps works on CPs (high-level)
class WW8PLCFx_Cp_FKP : public WW8PLCFx_Fc_FKP
{
private:
    const WW8ScannerBase& rSBase;
    std::unique_ptr<WW8PLCFx_PCD> pPcd;
    WW8PLCFpcd_Iter *pPieceIter;
    WW8_CP nAttrStart, nAttrEnd;
    bool bLineEnd : 1;
    bool bComplex : 1;

    WW8PLCFx_Cp_FKP(const WW8PLCFx_Cp_FKP&) = delete;
    WW8PLCFx_Cp_FKP& operator=(const WW8PLCFx_Cp_FKP&) = delete;

public:
    WW8PLCFx_Cp_FKP( SvStream* pSt, SvStream* pTableSt, SvStream* pDataSt,
        const WW8ScannerBase& rBase,  ePLCFT ePl );
    virtual ~WW8PLCFx_Cp_FKP() override;
    void ResetAttrStartEnd();
    sal_uInt32 GetPCDIdx() const;
    virtual sal_uInt32 GetIdx2() const override;
    virtual void  SetIdx2(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual void GetSprms( WW8PLCFxDesc* p ) override;
    virtual void advance() override;
    virtual void Save( WW8PLCFxSave1& rSave ) const override;
    virtual void Restore( const WW8PLCFxSave1& rSave ) override;
};

/// Iterator for Piece Table Exceptions of Sepx
class WW8PLCFx_SEPX : public WW8PLCFx
{
private:
    wwSprmParser maSprmParser;
    SvStream* pStrm;
    std::unique_ptr<WW8PLCF> pPLCF;
    std::unique_ptr<sal_uInt8[]> pSprms;
    sal_uInt16 nArrMax;
    sal_uInt16 nSprmSiz;

    WW8PLCFx_SEPX(const WW8PLCFx_SEPX&) = delete;
    WW8PLCFx_SEPX& operator=(const WW8PLCFx_SEPX&) = delete;

public:
    WW8PLCFx_SEPX( SvStream* pSt, SvStream* pTablexySt, const WW8Fib& rFib,
        WW8_CP nStartCp );
    virtual ~WW8PLCFx_SEPX() override;
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual void GetSprms( WW8PLCFxDesc* p ) override;
    virtual void advance() override;
    SprmResult HasSprm( sal_uInt16 nId ) const;
    SprmResult HasSprm( sal_uInt16 nId, sal_uInt8 n2nd ) const;
    SprmResult HasSprm( sal_uInt16 nId, const sal_uInt8* pOtherSprms,
        long nOtherSprmSiz ) const;
    bool Find4Sprms(sal_uInt16 nId1, sal_uInt16 nId2, sal_uInt16 nId3, sal_uInt16 nId4,
                    SprmResult& r1, SprmResult& r2, SprmResult& r3, SprmResult& r4) const;
};

/// iterator for footnotes/endnotes and comments
class WW8PLCFx_SubDoc : public WW8PLCFx
{
private:
    std::unique_ptr<WW8PLCF> pRef;
    std::unique_ptr<WW8PLCF> pText;

    WW8PLCFx_SubDoc(const WW8PLCFx_SubDoc&) = delete;
    WW8PLCFx_SubDoc& operator=(const WW8PLCFx_SubDoc&) = delete;

public:
    WW8PLCFx_SubDoc(SvStream* pSt, const WW8Fib& rFib, WW8_CP nStartCp,
                    long nFcRef, long nLenRef, long nFcText, long nLenText, long nStruc);
    virtual ~WW8PLCFx_SubDoc() override;
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;

    // returns reference descriptors
    const void* GetData() const
    {
        return pRef ? pRef->GetData( pRef->GetIdx() ) : nullptr;
    }

    virtual void GetSprms(WW8PLCFxDesc* p) override;
    virtual void advance() override;
    long Count() const { return pRef ? pRef->GetIMax() : 0; }
};

/// Iterator for fields
class WW8PLCFx_FLD : public WW8PLCFx
{
private:
    std::unique_ptr<WW8PLCFspecial> pPLCF;
    const WW8Fib& rFib;
    WW8PLCFx_FLD(const WW8PLCFx_FLD&) = delete;
    WW8PLCFx_FLD& operator=(const WW8PLCFx_FLD &) = delete;

public:
    WW8PLCFx_FLD(SvStream* pSt, const WW8Fib& rMyFib, short nType);
    virtual ~WW8PLCFx_FLD() override;
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual void GetSprms(WW8PLCFxDesc* p) override;
    virtual void advance() override;
    bool StartPosIsFieldStart();
    bool EndPosIsFieldEnd(WW8_CP&);
    bool GetPara(long nIdx, WW8FieldDesc& rF);
};

enum eBookStatus { BOOK_NORMAL = 0, BOOK_IGNORE = 0x1, BOOK_FIELD = 0x2 };

/// Iterator for Booknotes
class WW8PLCFx_Book : public WW8PLCFx
{
private:
    std::unique_ptr<WW8PLCFspecial> pBook[2];           // Start and End Position
    std::vector<OUString> aBookNames;   // Name
    std::vector<eBookStatus> aStatus;
    long nIMax;                         // Number of Booknotes
    sal_uInt16 nIsEnd;
    sal_Int32 nBookmarkId; // counter incremented by GetUniqueBookmarkName.

    WW8PLCFx_Book(const WW8PLCFx_Book&) = delete;
    WW8PLCFx_Book& operator=(const WW8PLCFx_Book&) = delete;

public:
    WW8PLCFx_Book(SvStream* pTableSt,const WW8Fib& rFib);
    virtual ~WW8PLCFx_Book() override;
    long GetIMax() const { return nIMax; }
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nI) override;
    virtual sal_uInt32 GetIdx2() const override;
    virtual void SetIdx2(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual long GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen ) override;
    virtual void advance() override;
    const OUString* GetName() const;
    WW8_CP GetStartPos() const
        { return nIsEnd ? WW8_CP_MAX : pBook[0]->Where(); }
    long GetLen() const;
    bool GetIsEnd() const { return nIsEnd != 0; }
    long GetHandle() const;
    void SetStatus( sal_uInt16 nIndex, eBookStatus eStat );
    void MapName(OUString& rName);
    OUString GetBookmark(long nStart,long nEnd, sal_uInt16 &nIndex);
    eBookStatus GetStatus() const;
    OUString GetUniqueBookmarkName(const OUString &rSuggestedName);
};

/// Handles the import of PlcfAtnBkf and PlcfAtnBkl: start / end position of annotation marks.
class WW8PLCFx_AtnBook : public WW8PLCFx
{
private:
    /// Start and end positions.
    std::unique_ptr<WW8PLCFspecial> m_pBook[2];
    /// Number of annotation marks
    sal_Int32 nIMax;
    bool m_bIsEnd;

    WW8PLCFx_AtnBook(const WW8PLCFx_AtnBook&) = delete;
    WW8PLCFx_AtnBook& operator=(const WW8PLCFx_AtnBook&) = delete;

public:
    WW8PLCFx_AtnBook(SvStream* pTableSt,const WW8Fib& rFib);
    virtual ~WW8PLCFx_AtnBook() override;
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nI) override;
    virtual sal_uInt32 GetIdx2() const override;
    virtual void SetIdx2(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual long GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen ) override;
    virtual void advance() override;

    /// Handle is the unique ID of an annotation mark.
    long getHandle() const;
    bool getIsEnd() const;
};

/// Handles the import of PlcfBkfFactoid and PlcfBklFactoid: start / end position of factoids.
class WW8PLCFx_FactoidBook : public WW8PLCFx
{
private:
    /// Start and end positions.
    std::unique_ptr<WW8PLCFspecial> m_pBook[2];
    /// Number of factoid marks
    sal_Int32 m_nIMax;
    bool m_bIsEnd;

    WW8PLCFx_FactoidBook(const WW8PLCFx_FactoidBook&) = delete;
    WW8PLCFx_FactoidBook& operator=(const WW8PLCFx_FactoidBook&) = delete;

public:
    WW8PLCFx_FactoidBook(SvStream* pTableSt,const WW8Fib& rFib);
    virtual ~WW8PLCFx_FactoidBook() override;
    virtual sal_uInt32 GetIdx() const override;
    virtual void SetIdx(sal_uInt32 nI) override;
    virtual sal_uInt32 GetIdx2() const override;
    virtual void SetIdx2(sal_uInt32 nIdx) override;
    virtual bool SeekPos(WW8_CP nCpPos) override;
    virtual WW8_CP Where() override;
    virtual long GetNoSprms(WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen) override;
    virtual void advance() override;

    /// Handle is the unique ID of a factoid mark.
    long getHandle() const;
    bool getIsEnd() const;
};

/*
    this is what we use outside:
*/
struct WW8PLCFManResult
{
    WW8_CP nCpPos;      // attribute starting position
    long nMemLen;       // length for previous
    long nCp2OrIdx;     // footnote-textpos or index in PLCF
    WW8_CP nCurrentCp;  // only used by caller
    const sal_uInt8* pMemPos;// Mem-Pos for Sprms
    sal_uInt16 nSprmId;     // Sprm-Id ( 0 = invalid Id -> skip! )
                        // (2..255) or pseudo-Sprm-Id (256..260)
                        // from Winword-Ver8 Sprm-Id (800..) resp.
    sal_uInt8 nFlags;        // start of paragraph or section
};

enum ManMaskTypes
{
    MAN_MASK_NEW_PAP = 1,       // new line
    MAN_MASK_NEW_SEP = 2        // new section
};

enum ManTypes // enums for PLCFMan-ctor
{
    MAN_MAINTEXT = 0, MAN_FTN = 1, MAN_EDN = 2, MAN_HDFT = 3, MAN_AND = 4,
    MAN_TXBX = 5, MAN_TXBX_HDFT = 6
};

/*
    this is what the manager uses inside:
*/
struct WW8PLCFxDesc
{
    WW8PLCFx* pPLCFx;
    std::stack<sal_uInt16>* pIdStack;    // memory for Attr-Id for Attr-end(s)
    const sal_uInt8* pMemPos;// where are the Sprm(s)
    long nOrigSprmsLen;

    WW8_CP nStartPos;
    WW8_CP nEndPos;

    WW8_CP nOrigStartPos;
    WW8_CP nOrigEndPos;   // The ending character position of a paragraph is
                          // always one before the end reported in the FKP,
                          // also a character run that ends on the same location
                          // as the paragraph mark is adjusted to end just before
                          // the paragraph mark so as to handle their close
                          // first. The value being used to determining where the
                          // properties end is in nEndPos, but the original
                          // unadjusted end character position is important as
                          // it can be used as the beginning cp of the next set
                          // of properties

    WW8_CP nCp2OrIdx;     // where are the NoSprm(s)
    sal_Int32 nSprmsLen;  // how many bytes for further Sprms / length of footnote
    long nCpOfs;          // for Offset Header .. Footnote
    bool bFirstSprm;      // for recognizing the first Sprm of a group
    bool bRealLineEnd;    // false for Pap-Piece-end
    sal_Int16 nRelativeJustify;
    void Save( WW8PLCFxSave1& rSave ) const;
    void Restore( const WW8PLCFxSave1& rSave );
    //With nStartPos set to WW8_CP_MAX then in the case of a pap or chp
    //GetSprms will not search for the sprms, but instead take the
    //existing ones.
    WW8PLCFxDesc()
        : pPLCFx(nullptr)
        , pIdStack(nullptr)
        , pMemPos(nullptr)
        , nOrigSprmsLen(0)
        , nStartPos(WW8_CP_MAX)
        , nEndPos(WW8_CP_MAX)
        , nOrigStartPos(WW8_CP_MAX)
        , nOrigEndPos(WW8_CP_MAX)
        , nCp2OrIdx(WW8_CP_MAX)
        , nSprmsLen(0)
        , nCpOfs(0)
        , bFirstSprm(false)
        , bRealLineEnd(false)
        , nRelativeJustify(-1)
    {
    }
    void ReduceByOffset();
};

struct WW8PLCFxSaveAll;
class WW8PLCFMan
{
public:
    enum WW8PLCFManLimits {MAN_PLCF_COUNT = 12};

private:
    wwSprmParser maSprmParser;
    WW8_CP m_nCpO;                  //< Origin Cp -- the basis for nNewCp

    WW8_CP m_nLineEnd;                // points *after* the <CR>
    sal_uInt16 m_nPLCF;                   // this many PLCFs are managed
    ManTypes m_nManType;
    bool mbDoingDrawTextBox;        //Normally we adjust the end of attributes
                                    //so that the end of a paragraph occurs
                                    //before the para end mark, but for
                                    //drawboxes we want the true offsets

    WW8PLCFxDesc m_aD[MAN_PLCF_COUNT];
    WW8PLCFxDesc *m_pChp, *m_pPap, *m_pSep, *m_pField, *m_pFootnote, *m_pEdn, *m_pBkm, *m_pPcd,
        *m_pPcdA, *m_pAnd, *m_pAtnBkm, *m_pFactoidBkm;
    WW8PLCFspecial *m_pFdoa, *m_pTxbx, *m_pTxbxBkd,*m_pMagicTables, *m_pSubdocs;
    sal_uInt8* m_pExtendedAtrds;

    const WW8Fib* m_pWwFib;

    sal_uInt16 WhereIdx(bool* pbStart, WW8_CP * pPos=nullptr) const;
    void AdjustEnds(WW8PLCFxDesc& rDesc);
    void GetNewSprms(WW8PLCFxDesc& rDesc);
    static void GetNewNoSprms(WW8PLCFxDesc& rDesc);
    void GetSprmStart(short nIdx, WW8PLCFManResult* pRes) const;
    void GetSprmEnd(short nIdx, WW8PLCFManResult* pRes) const;
    void GetNoSprmStart(short nIdx, WW8PLCFManResult* pRes) const;
    void GetNoSprmEnd(short nIdx, WW8PLCFManResult* pRes) const;
    void AdvSprm(short nIdx, bool bStart);
    void AdvNoSprm(short nIdx, bool bStart);
    sal_uInt16 GetId(const WW8PLCFxDesc* p ) const;

public:
    WW8PLCFMan(const WW8ScannerBase* pBase, ManTypes nType, long nStartCp,
        bool bDoingDrawTextBox = false);
    ~WW8PLCFMan();

    /*
        Where asks on which following position any Attr changes...
    */
    WW8_CP Where() const;

    bool Get(WW8PLCFManResult* pResult) const;
    void advance();
    sal_uInt16 GetColl() const; // index of actual Style
    WW8PLCFx_FLD* GetField() const;
    WW8PLCFx_SubDoc* GetEdn() const { return static_cast<WW8PLCFx_SubDoc*>(m_pEdn->pPLCFx); }
    WW8PLCFx_SubDoc* GetFootnote() const { return static_cast<WW8PLCFx_SubDoc*>(m_pFootnote->pPLCFx); }
    WW8PLCFx_SubDoc* GetAtn() const { return static_cast<WW8PLCFx_SubDoc*>(m_pAnd->pPLCFx); }
    WW8PLCFx_Book* GetBook() const { return static_cast<WW8PLCFx_Book*>(m_pBkm->pPLCFx); }
    WW8PLCFx_AtnBook* GetAtnBook() const { return static_cast<WW8PLCFx_AtnBook*>(m_pAtnBkm->pPLCFx); }
    WW8PLCFx_FactoidBook* GetFactoidBook() const { return static_cast<WW8PLCFx_FactoidBook*>(m_pFactoidBkm->pPLCFx); }
    long GetCpOfs() const { return m_pChp->nCpOfs; }  // for Header/Footer...

    /* asks, if *current paragraph* has an Sprm of this type */
    SprmResult HasParaSprm(sal_uInt16 nId) const;

    /* asks, if *current textrun* has an Sprm of this type */
    SprmResult HasCharSprm(sal_uInt16 nId) const;
    void HasCharSprm(sal_uInt16 nId, std::vector<SprmResult> &rResult) const;

    WW8PLCFx_Cp_FKP* GetChpPLCF() const
        { return static_cast<WW8PLCFx_Cp_FKP*>(m_pChp->pPLCFx); }
    WW8PLCFx_Cp_FKP* GetPapPLCF() const
        { return static_cast<WW8PLCFx_Cp_FKP*>(m_pPap->pPLCFx); }
    WW8PLCFx_SEPX* GetSepPLCF() const
        { return static_cast<WW8PLCFx_SEPX*>(m_pSep->pPLCFx); }
    WW8PLCFxDesc* GetPap() const { return m_pPap; }
    void TransferOpenSprms(std::stack<sal_uInt16> &rStack);
    void SeekPos( long nNewCp );
    void SaveAllPLCFx( WW8PLCFxSaveAll& rSave ) const;
    void RestoreAllPLCFx( const WW8PLCFxSaveAll& rSave );
    WW8PLCFspecial* GetFdoa() const { return m_pFdoa; }
    WW8PLCFspecial* GetTxbx() const { return m_pTxbx; }
    WW8PLCFspecial* GetTxbxBkd() const { return m_pTxbxBkd; }
    WW8PLCFspecial* GetMagicTables() const { return m_pMagicTables; }
    WW8PLCFspecial* GetWkbPLCF() const { return m_pSubdocs; }
    sal_uInt8* GetExtendedAtrds() const { return m_pExtendedAtrds; }
    ManTypes GetManType() const { return m_nManType; }
    bool GetDoingDrawTextBox() const { return mbDoingDrawTextBox; }
};

struct WW8PLCFxSaveAll
{
    WW8PLCFxSave1 aS[WW8PLCFMan::MAN_PLCF_COUNT] = {};
    WW8PLCFxSaveAll() = default;
};

class WW8ScannerBase
{
friend WW8PLCFx_PCDAttrs::WW8PLCFx_PCDAttrs(const WW8Fib& rFib,
    WW8PLCFx_PCD* pPLCFx_PCD, const WW8ScannerBase* pBase );
friend WW8PLCFx_Cp_FKP::WW8PLCFx_Cp_FKP( SvStream*, SvStream*, SvStream*,
    const WW8ScannerBase&, ePLCFT );

friend WW8PLCFMan::WW8PLCFMan(const WW8ScannerBase*, ManTypes, long, bool);
friend class SwWW8FltControlStack;

private:
    WW8Fib* m_pWw8Fib;
    std::unique_ptr<WW8PLCFx_Cp_FKP>  m_pChpPLCF;         // Character-Attrs
    std::unique_ptr<WW8PLCFx_Cp_FKP>  m_pPapPLCF;         // Paragraph-Attrs
    std::unique_ptr<WW8PLCFx_SEPX>    m_pSepPLCF;         // Section-Attrs
    std::unique_ptr<WW8PLCFx_SubDoc>  m_pFootnotePLCF;         // Footnotes
    std::unique_ptr<WW8PLCFx_SubDoc>  m_pEdnPLCF;         // EndNotes
    std::unique_ptr<WW8PLCFx_SubDoc>  m_pAndPLCF;         // Comments
    std::unique_ptr<WW8PLCFx_FLD>     m_pFieldPLCF;         // Fields in Main Text
    std::unique_ptr<WW8PLCFx_FLD>     m_pFieldHdFtPLCF;     // Fields in Header / Footer
    std::unique_ptr<WW8PLCFx_FLD>     m_pFieldTxbxPLCF;     // Fields in Textboxes in Main Text
    std::unique_ptr<WW8PLCFx_FLD>     m_pFieldTxbxHdFtPLCF; // Fields in Textboxes in Header / Footer
    std::unique_ptr<WW8PLCFx_FLD>     m_pFieldFootnotePLCF;      // Fields in Footnotes
    std::unique_ptr<WW8PLCFx_FLD>     m_pFieldEdnPLCF;      // Fields in Endnotes
    std::unique_ptr<WW8PLCFx_FLD>     m_pFieldAndPLCF;      // Fields in Comments
    std::unique_ptr<WW8PLCFspecial>   m_pMainFdoa;        // Graphic Primitives in Main Text
    std::unique_ptr<WW8PLCFspecial>   m_pHdFtFdoa;        // Graphic Primitives in Header / Footer
    std::unique_ptr<WW8PLCFspecial>   m_pMainTxbx;        // Textboxes in Main Text
    std::unique_ptr<WW8PLCFspecial>   m_pMainTxbxBkd;     // Break-Descriptors for them
    std::unique_ptr<WW8PLCFspecial>   m_pHdFtTxbx;        // TextBoxes in Header / Footer
    std::unique_ptr<WW8PLCFspecial>   m_pHdFtTxbxBkd;     // Break-Descriptors for previous
    std::unique_ptr<WW8PLCFspecial>   m_pMagicTables;     // Break-Descriptors for them
    std::unique_ptr<WW8PLCFspecial>   m_pSubdocs;         // subdoc references in master document
    std::unique_ptr<sal_uInt8[]>
                      m_pExtendedAtrds;   // Extended ATRDs
    std::unique_ptr<WW8PLCFx_Book>    m_pBook;            // Bookmarks
    std::unique_ptr<WW8PLCFx_AtnBook> m_pAtnBook;         // Annotationmarks
    /// Smart tag bookmarks.
    std::unique_ptr<WW8PLCFx_FactoidBook> m_pFactoidBook;

    std::unique_ptr<WW8PLCFpcd>         m_pPiecePLCF; // for FastSave ( Basis-PLCF without iterator )
    std::unique_ptr<WW8PLCFpcd_Iter>    m_pPieceIter; // for FastSave ( iterator for previous )
    std::unique_ptr<WW8PLCFx_PCD>       m_pPLCFx_PCD;     // ditto
    std::unique_ptr<WW8PLCFx_PCDAttrs>  m_pPLCFx_PCDAttrs;
    std::vector<std::unique_ptr<sal_uInt8[]>> m_aPieceGrpprls;  // attributes of Piece-Table

    std::unique_ptr<WW8PLCFpcd> OpenPieceTable( SvStream* pStr, const WW8Fib* pWwF );

    WW8ScannerBase(const WW8ScannerBase&) = delete;
    WW8ScannerBase& operator=(const WW8ScannerBase&) = delete;

public:
    WW8ScannerBase( SvStream* pSt, SvStream* pTableSt, SvStream* pDataSt,
        WW8Fib* pWwF );
    ~WW8ScannerBase();
    bool AreThereFootnotes() const { return m_pFootnotePLCF->Count() > 0; };
    bool AreThereEndnotes()  const { return m_pEdnPLCF->Count() > 0; };

    //If you use WW8Fc2Cp you are almost certainly doing the wrong thing
    //when it comes to fastsaved files, avoid like the plague. For export
    //given that we never write fastsaved files you can use it, otherwise
    //I will beat you with a stick
    WW8_CP WW8Fc2Cp(WW8_FC nFcPos) const ;
    WW8_FC WW8Cp2Fc(WW8_CP nCpPos, bool* pIsUnicode = nullptr,
        WW8_CP* pNextPieceCp = nullptr, bool* pTestFlag = nullptr) const;

    sal_Int32 WW8ReadString(SvStream& rStrm, OUString& rStr, WW8_CP nCurrentStartCp,
        long nTotalLen, rtl_TextEncoding eEnc ) const;

};

/** FIB - the File Information Block

    The FIB contains a "magic word" and pointers to the various other parts of
    the file, as well as information about the length of the file.
    The FIB starts at the beginning of the file.
*/
class WW8Fib
{
private:
    sal_Unicode m_nNumDecimalSep = u'\0';

public:
    /**
        Program-Version asked for by us:
        in Ctor we check if it matches the value of nFib

        6 == "WinWord 6 or WinWord 95",
        7 == "only WinWord 95"
        8 == "WinWord 97 or newer"
    */
    sal_uInt8 m_nVersion = 0;
    /*
        error status
    */
    ErrCode   m_nFibError;
    /*
        data read from FIB by Ctor
        (corresponds only approximately to the real structure
        of the Winword-FIB)
    */
    sal_uInt16 m_wIdent = 0; // 0x0 int magic number
    /*
        File Information Block (FIB) values:
        WinWord 1.0 = 33
        WinWord 2.0 = 45
        WinWord 6.0c for 16bit = 101
        Word 6/32 bit = 104
        Word 95 = 104
        Word 97 = 193
        Word 2000 = 217
        Word 2002 = 257
        Word 2003 = 268
        Word 2007 = 274
    */
    sal_uInt16 m_nFib = 0;        // 0x2 FIB version written
    sal_uInt16 m_nProduct = 0;    // 0x4 product version written by
    LanguageType m_lid;       // 0x6 language stamp---localized version;
    WW8_PN m_pnNext = 0;          // 0x8

    bool m_fDot :1 /*= false*/;     // 0xa 0001
    bool m_fGlsy :1 /*= false*/;
    bool m_fComplex :1 /*= false*/; // 0004 when 1, file is in complex, fast-saved format.
    bool m_fHasPic :1 /*= false*/;  // 0008 file contains 1 or more pictures
    sal_uInt16 m_cQuickSaves :4 /*= 0*/; // 00F0 count of times file was quicksaved
    bool m_fEncrypted :1 /*= false*/; //0100 1 if file is encrypted, 0 if not
    bool m_fWhichTableStm :1 /*= false*/; //0200 When 0, this fib refers to the table stream
    bool m_fReadOnlyRecommended :1 /*= false*/;
    bool m_fWriteReservation :1 /*= false*/;
                                                    // named "0Table", when 1, this fib refers to the
                                                    // table stream named "1Table". Normally, a file
                                                    // will have only one table stream, but under unusual
                                                    // circumstances a file may have table streams with
                                                    // both names. In that case, this flag must be used
                                                    // to decide which table stream is valid.

    bool m_fExtChar :1 /*= false*/; // 1000 =1, when using extended character set in file
    bool m_fFarEast :1 /*= false*/; // 4000 =1, probably, when far-East language variants of Word is used to create a file #i90932#

    bool m_fObfuscated :1 /*= false*/; // 8000=1. specifies whether the document is obfuscated using XOR obfuscation. otherwise this bit MUST be ignored.

    sal_uInt16 m_nFibBack = 0;    // 0xc
    sal_uInt16 m_nHash = 0;       // 0xe  file encrypted hash
    sal_uInt16 m_nKey = 0;        // 0x10  file encrypted key
    sal_uInt8 m_envr = 0;         // 0x12 environment in which file was created
                                    //      0 created by Win Word / 1 created by Mac Word
    bool m_fMac              :1 /*= false*/;          // 0x13 when 1, this file was last saved in the Mac environment
    bool m_fEmptySpecial     :1 /*= false*/;
    bool m_fLoadOverridePage :1 /*= false*/;
    bool m_fFuturesavedUndo  :1 /*= false*/;
    bool m_fWord97Saved      :1 /*= false*/;
    bool m_fWord2000Saved    :1 /*= false*/;
    sal_uInt8 :2;

    sal_uInt16 m_chse = 0;        // 0x14 default extended character set id for text in document stream. (overridden by chp.chse)
                        //      0 = ANSI  / 256 Macintosh character set.
    sal_uInt16 m_chseTables = 0;  // 0x16 default extended character set id for text in
                        //      internal data structures: 0 = ANSI, 256 = Macintosh
    WW8_FC m_fcMin = 0;           // 0x18 file offset of first character of text
    WW8_FC m_fcMac = 0;           // 0x1c file offset of last character of text + 1

    // start of WW8 section
    sal_uInt16 m_csw = 0;             // Count of fields in the array of "shorts"

    // marker: "rgsw" Beginning of the array of shorts
    sal_uInt16 m_wMagicCreated = 0;   // unique number Identifying the File's creator
                                // 0x6A62 is the creator ID for Word and is reserved.
                                // Other creators should choose a different value.
    sal_uInt16 m_wMagicRevised = 0;   // identifies the File's last modifier
    sal_uInt16 m_wMagicCreatedPrivate = 0; // private data
    sal_uInt16 m_wMagicRevisedPrivate = 0; // private data

    LanguageType m_lidFE; // Language id if document was written by Far East version
                          // of Word (i.e. FIB.fFarEast is on)
    sal_uInt16 m_clw = 0; // Number of fields in the array of longs

    // end of WW8 section

    // Marker: "rglw" Beginning of the array of longs
    WW8_FC m_cbMac = 0;           // 0x20 file offset of last byte written to file + 1.

    // WW8_FC u4[4];        // 0x24
    WW8_CP m_ccpText = 0;         // 0x34 length of main document text stream
    WW8_CP m_ccpFootnote = 0;          // 0x38 length of footnote subdocument text stream
    WW8_CP m_ccpHdr = 0;          // 0x3c length of header subdocument text stream
    WW8_CP m_ccpMcr = 0;          // 0x40 length of macro subdocument text stream
    WW8_CP m_ccpAtn = 0;          // 0x44 length of annotation subdocument text stream
    WW8_CP m_ccpEdn = 0;          // 0x48 length of endnote subdocument text stream
    WW8_CP m_ccpTxbx = 0;         // 0x4c length of textbox subdocument text stream
    WW8_CP m_ccpHdrTxbx = 0;      // 0x50 length of header textbox subdocument text stream

    // start of WW8 section
    sal_Int32  m_pnFbpChpFirst = 0; // when there was insufficient memory for Word to expand
                              // the PLCFbte at save time, the PLCFbte is written
                              // to the file in a linked list of 512-byte pieces
                              // starting with this pn.
    sal_Int32  m_pnFbpPapFirst = 0; // when there was insufficient memory for Word to expand
                              // the PLCFbte at save time, the PLCFbte is written to
                              // the file in a linked list of 512-byte pieces
                              // starting with this pn

    sal_Int32  m_pnFbpLvcFirst = 0; // when there was insufficient memory for Word to expand
                              // the PLCFbte at save time, the PLCFbte is written to
                              // the file in a linked list of 512-byte pieces
                              // starting with this pn
    sal_Int32  m_pnLvcFirst = 0; // the page number of the lowest numbered page in the
                           // document that records LVC FKP information
    sal_Int32  m_cpnBteLvc = 0;  // count of LVC FKPs recorded in file. In non-complex
                           // files if the number of entries in the PLCFbtePapx is
                           // less than this, the PLCFbtePapx is incomplete.
    sal_Int32  m_fcIslandFirst = 0;   // ?
    sal_Int32  m_fcIslandLim = 0;     // ?
    sal_uInt16 m_cfclcb = 0; // Number of fields in the array of FC/LCB pairs.
    /// Specifies the count of 16-bit values corresponding to fibRgCswNew that follow.
    sal_uInt16 m_cswNew = 0;

    // end of WW8 section

    // Marker: "rgfclcb" Beginning of array of FC/LCB pairs.
    WW8_FC m_fcStshfOrig = 0;     // file offset of original allocation for STSH in table
                                                // stream. During fast save Word will attempt to reuse
                                                // this allocation if STSH is small enough to fit.
    sal_Int32 m_lcbStshfOrig = 0; // 0x5c count of bytes of original STSH allocation
    WW8_FC m_fcStshf = 0;         // 0x60 file offset of STSH in file.
    sal_Int32 m_lcbStshf = 0;     // 0x64 count of bytes of current STSH allocation
    WW8_FC m_fcPlcffndRef = 0;    // 0x68 file offset of footnote reference PLCF.
    sal_Int32 m_lcbPlcffndRef = 0;    // 0x6c count of bytes of footnote reference PLCF
                        //      == 0 if no footnotes defined in document.

    WW8_FC m_fcPlcffndText = 0;    // 0x70 file offset of footnote text PLCF.
    sal_Int32 m_lcbPlcffndText = 0;    // 0x74 count of bytes of footnote text PLCF.
                        //      == 0 if no footnotes defined in document

    WW8_FC m_fcPlcfandRef = 0;    // 0x78 file offset of annotation reference PLCF.
    sal_Int32 m_lcbPlcfandRef = 0;    // 0x7c count of bytes of annotation reference PLCF.

    WW8_FC m_fcPlcfandText = 0;    // 0x80 file offset of annotation text PLCF.
    sal_Int32 m_lcbPlcfandText = 0;    // 0x84 count of bytes of the annotation text PLCF

    WW8_FC m_fcPlcfsed = 0;       // 8x88 file offset of section descriptor PLCF.
    sal_Int32 m_lcbPlcfsed = 0;   // 0x8c count of bytes of section descriptor PLCF.

    WW8_FC m_fcPlcfpad = 0;       // 0x90 file offset of paragraph descriptor PLCF
    sal_Int32 m_lcbPlcfpad = 0;   // 0x94 count of bytes of paragraph descriptor PLCF.
                        // ==0 if file was never viewed in Outline view.
                        // Should not be written by third party creators

    WW8_FC m_fcPlcfphe = 0;       // 0x98 file offset of PLCF of paragraph heights.
    sal_Int32 m_lcbPlcfphe = 0;   // 0x9c count of bytes of paragraph height PLCF.
                        // ==0 when file is non-complex.

    WW8_FC m_fcSttbfglsy = 0;     // 0xa0 file offset of glossary string table.
    sal_Int32 m_lcbSttbfglsy = 0; // 0xa4 count of bytes of glossary string table.
                        //      == 0 for non-glossary documents.
                        //      !=0 for glossary documents.

    WW8_FC m_fcPlcfglsy = 0;      // 0xa8 file offset of glossary PLCF.
    sal_Int32 m_lcbPlcfglsy = 0;  // 0xac count of bytes of glossary PLCF.
                        //      == 0 for non-glossary documents.
                        //      !=0 for glossary documents.

    WW8_FC m_fcPlcfhdd = 0;       // 0xb0 byte offset of header PLCF.
    sal_Int32 m_lcbPlcfhdd = 0;   // 0xb4 count of bytes of header PLCF.
                        //      == 0 if document contains no headers

    WW8_FC m_fcPlcfbteChpx = 0;   // 0xb8 file offset of character property bin table.PLCF.
    sal_Int32 m_lcbPlcfbteChpx = 0;// 0xbc count of bytes of character property bin table PLCF.

    WW8_FC m_fcPlcfbtePapx = 0;   // 0xc0 file offset of paragraph property bin table.PLCF.
    sal_Int32 m_lcbPlcfbtePapx = 0;// 0xc4 count of bytes of paragraph  property bin table PLCF.

    WW8_FC m_fcPlcfsea = 0;       // 0xc8 file offset of PLCF reserved for private use. The SEA is 6 bytes long.
    sal_Int32 m_lcbPlcfsea = 0;   // 0xcc count of bytes of private use PLCF.

    WW8_FC m_fcSttbfffn = 0;      // 0xd0 file offset of font information STTBF. See the FFN file structure definition.
    sal_Int32 m_lcbSttbfffn = 0;  // 0xd4 count of bytes in sttbfffn.

    WW8_FC m_fcPlcffldMom = 0;    // 0xd8 offset in doc stream to the PLCF of field positions in the main document.
    sal_Int32 m_lcbPlcffldMom = 0;    // 0xdc

    WW8_FC m_fcPlcffldHdr = 0;    // 0xe0 offset in doc stream to the PLCF of field positions in the header subdocument.
    sal_Int32 m_lcbPlcffldHdr = 0;    // 0xe4

    WW8_FC m_fcPlcffldFootnote = 0;    // 0xe8 offset in doc stream to the PLCF of field positions in the footnote subdocument.
    sal_Int32 m_lcbPlcffldFootnote = 0;    // 0xec

    WW8_FC m_fcPlcffldAtn = 0;    // 0xf0 offset in doc stream to the PLCF of field positions in the annotation subdocument.
    sal_Int32 m_lcbPlcffldAtn = 0;    // 0xf4

    WW8_FC m_fcPlcffldMcr = 0;    // 0xf8 offset in doc stream to the PLCF of field positions in the macro subdocument.
    sal_Int32 m_lcbPlcffldMcr = 0;    // 9xfc

    WW8_FC m_fcSttbfbkmk = 0; // 0x100 offset in document stream of the STTBF that records bookmark names in the main document
    sal_Int32 m_lcbSttbfbkmk = 0; // 0x104

    WW8_FC m_fcPlcfbkf = 0;   // 0x108 offset in document stream of the PLCF that records the beginning CP offsets of bookmarks in the main document. See BKF
    sal_Int32 m_lcbPlcfbkf = 0;   // 0x10c

    WW8_FC m_fcPlcfbkl = 0;   // 0x110 offset in document stream of the PLCF that records the ending CP offsets of bookmarks recorded in the main document. See the BKL structure definition.
    sal_Int32 m_lcbPlcfbkl = 0;   // 0x114 sal_Int32

    WW8_FC m_fcCmds = 0;      // 0x118 FC
    sal_uInt32 m_lcbCmds = 0;      // 0x11c

    WW8_FC m_fcPlcfmcr = 0;       // 0x120 FC
    sal_Int32 m_lcbPlcfmcr = 0;       // 0x124

    WW8_FC m_fcSttbfmcr = 0;  // 0x128 FC
    sal_Int32 m_lcbSttbfmcr = 0;  // 0x12c

    WW8_FC m_fcPrDrvr = 0;        // 0x130 file offset of the printer driver information (names of drivers, port etc...)
    sal_Int32 m_lcbPrDrvr = 0;        // 0x134 count of bytes of the printer driver information (names of drivers, port etc...)

    WW8_FC m_fcPrEnvPort = 0; // 0x138 file offset of the print environment in portrait mode.
    sal_Int32 m_lcbPrEnvPort = 0; // 0x13c count of bytes of the print environment in portrait mode.

    WW8_FC m_fcPrEnvLand = 0; // 0x140 file offset of the print environment in landscape mode.
    sal_Int32 m_lcbPrEnvLand = 0; // 0x144 count of bytes of the print environment in landscape mode.

    WW8_FC m_fcWss = 0;       // 0x148 file offset of Window Save State data structure. See WSS.
    sal_Int32 m_lcbWss = 0;       // 0x14c count of bytes of WSS. ==0 if unable to store the window state.

    WW8_FC m_fcDop = 0;       // 0x150 file offset of document property data structure.
    sal_uInt32 m_lcbDop = 0;       // 0x154 count of bytes of document properties.
        // cbDOP is 84 when nFib < 103

    WW8_FC m_fcSttbfAssoc = 0;    // 0x158 offset to STTBF of associated strings. See STTBFASSOC.
    sal_Int32 m_lcbSttbfAssoc = 0; // 0x15C

    WW8_FC m_fcClx = 0;           // 0x160 file  offset of beginning of information for complex files.
    sal_Int32 m_lcbClx = 0;       // 0x164 count of bytes of complex file information. 0 if file is non-complex.

    WW8_FC m_fcPlcfpgdFootnote = 0;    // 0x168 file offset of page descriptor PLCF for footnote subdocument.
    sal_Int32 m_lcbPlcfpgdFootnote = 0;    // 0x16C count of bytes of page descriptor PLCF for footnote subdocument.
                        //  ==0 if document has not been paginated. The length of the PGD is 8 bytes.

    WW8_FC m_fcAutosaveSource = 0;    // 0x170 file offset of the name of the original file.
    sal_Int32 m_lcbAutosaveSource = 0;    // 0x174 count of bytes of the name of the original file.

    WW8_FC m_fcGrpStAtnOwners = 0;    // 0x178 group of strings recording the names of the owners of annotations
    sal_Int32 m_lcbGrpStAtnOwners = 0;    // 0x17C count of bytes of the group of strings

    WW8_FC m_fcSttbfAtnbkmk = 0;  // 0x180 file offset of the sttbf that records names of bookmarks in the annotation subdocument
    sal_Int32 m_lcbSttbfAtnbkmk = 0;  // 0x184 length in bytes of the sttbf that records names of bookmarks in the annotation subdocument

    // end of WW67 section

    WW8_FC m_fcPlcfdoaMom = 0;    // 0x192 file offset of the  FDOA (drawn object) PLCF for main document.
                        //  ==0 if document has no drawn objects. The length of the FDOA is 6 bytes.
                        // unused starting from Ver8
    sal_Int32 m_lcbPlcfdoaMom = 0;    // 0x196 length in bytes of the FDOA PLCF of the main document
                                                // unused starting from Ver8
    WW8_FC m_fcPlcfdoaHdr = 0;    // 0x19A file offset of the  FDOA (drawn object) PLCF for the header document.
                        //  ==0 if document has no drawn objects. The length of the FDOA is 6 bytes.
                        // unused starting from Ver8
    sal_Int32 m_lcbPlcfdoaHdr = 0;    // 0x19E length in bytes of the FDOA PLCF of the header document
                                                // unused starting from Ver8

    WW8_FC m_fcPlcfspaMom = 0;        // offset in table stream of the FSPA PLCF for main document.
                                                // == 0 if document has no office art objects
                                                        // was empty reserve in Ver67
    sal_Int32 m_lcbPlcfspaMom = 0;        // length in bytes of the FSPA PLCF of the main document
                                                        // was empty reserve in Ver67
    WW8_FC m_fcPlcfspaHdr = 0;        // offset in table stream of the FSPA PLCF for header document.
                                                // == 0 if document has no office art objects
                                                        // was empty reserve in Ver67
    sal_Int32 m_lcbPlcfspaHdr = 0;        // length in bytes of the FSPA PLCF of the header document
                                                        // was empty reserve in Ver67

    WW8_FC m_fcPlcfAtnbkf = 0;    // 0x1B2 file offset of BKF (bookmark first) PLCF of the annotation subdocument
    sal_Int32 m_lcbPlcfAtnbkf = 0;    // 0x1B6 length in bytes of BKF (bookmark first) PLCF of the annotation subdocument

    WW8_FC m_fcPlcfAtnbkl = 0;    // 0x1BA file offset of BKL (bookmark last) PLCF of the annotation subdocument
    sal_Int32 m_lcbPlcfAtnbkl = 0;    // 0x1BE length in bytes of BKL (bookmark first) PLCF of the annotation subdocument

    WW8_FC m_fcPms = 0;       // 0x1C2 file offset of PMS (Print Merge State) information block
    sal_Int32 m_lcbPMS = 0;       // 0x1C6 length in bytes of PMS

    WW8_FC m_fcFormFieldSttbf = 0;  // 0x1CA file offset of form field Sttbf which contains strings used in form field dropdown controls
    sal_Int32 m_lcbFormFieldSttbf = 0;  // 0x1CE length in bytes of form field Sttbf

    WW8_FC m_fcPlcfendRef = 0;    // 0x1D2 file offset of PLCFendRef which points to endnote references in the main document stream
    sal_Int32 m_lcbPlcfendRef = 0;    // 0x1D6

    WW8_FC m_fcPlcfendText = 0;    // 0x1DA file offset of PLCFendRef which points to endnote text  in the endnote document
                        //       stream which corresponds with the PLCFendRef
    sal_Int32 m_lcbPlcfendText = 0;    // 0x1DE

    WW8_FC m_fcPlcffldEdn = 0;    // 0x1E2 offset to PLCF of field positions in the endnote subdoc
    sal_Int32 m_lcbPlcffldEdn = 0;    // 0x1E6

    WW8_FC  m_fcPlcfpgdEdn = 0;   // 0x1EA offset to PLCF of page boundaries in the endnote subdoc.
    sal_Int32 m_lcbPlcfpgdEdn = 0;        // 0x1EE

    WW8_FC m_fcDggInfo = 0;           // offset in table stream of the office art object table data.
                                                // The format of office art object table data is found in a separate document.
                                                        // was empty reserve in Ver67
    sal_Int32 m_lcbDggInfo = 0;           // length in bytes of the office art object table data
                                                        // was empty reserve in Ver67

    WW8_FC m_fcSttbfRMark = 0;        // 0x1fa offset to STTBF that records the author abbreviations...
    sal_Int32 m_lcbSttbfRMark = 0;        // 0x1fe
    WW8_FC m_fcSttbfCaption = 0;  // 0x202 offset to STTBF that records caption titles...
    sal_Int32 m_lcbSttbfCaption = 0;  // 0x206
    WW8_FC m_fcSttbAutoCaption = 0;   // offset in table stream to the STTBF that records the object names and
                                                        // indices into the caption STTBF for objects which get auto captions.
    sal_Int32 m_lcbSttbAutoCaption = 0;   // 0x20e

    WW8_FC m_fcPlcfwkb = 0;       // 0x212 offset to PLCF that describes the boundaries of contributing documents...
    sal_Int32 m_lcbPlcfwkb = 0;       // 0x216

    WW8_FC m_fcPlcfspl = 0;       // offset in table stream of PLCF (of SPLS structures) that records spell check state
                                                        // was empty reserve in Ver67
    sal_Int32 m_lcbPlcfspl = 0;                   // was empty reserve in Ver67

    WW8_FC m_fcPlcftxbxText = 0;   // 0x222 ...PLCF of beginning CP in the text box subdoc
    sal_Int32 m_lcbPlcftxbxText = 0;   // 0x226
    WW8_FC m_fcPlcffldTxbx = 0;   // 0x22a ...PLCF of field boundaries recorded in the textbox subdoc.
    sal_Int32 m_lcbPlcffldTxbx = 0;   // 0x22e
    WW8_FC m_fcPlcfHdrtxbxText = 0;// 0x232 ...PLCF of beginning CP in the header text box subdoc
    sal_Int32 m_lcbPlcfHdrtxbxText = 0;// 0x236
    WW8_FC m_fcPlcffldHdrTxbx = 0;// 0x23a ...PLCF of field boundaries recorded in the header textbox subdoc.
    sal_Int32 m_lcbPlcffldHdrTxbx = 0;// 0x23e
    WW8_FC m_fcStwUser = 0;
    sal_uInt32 m_lcbStwUser = 0;
    WW8_FC m_fcSttbttmbd = 0;
    sal_uInt32 m_lcbSttbttmbd = 0;

    WW8_FC m_fcSttbFnm = 0;       // 0x02da offset in the table stream of masters subdocument names
    sal_Int32 m_lcbSttbFnm = 0;       // 0x02de length

    /*
        special list handling for WW8
    */
    WW8_FC m_fcPlcfLst = 0;       // 0x02e2 offset in the table stream of list format information.
    sal_Int32 m_lcbPlcfLst = 0;       // 0x02e6 length
    WW8_FC m_fcPlfLfo = 0;        // 0x02ea offset in the table stream of list format override information.
    sal_Int32 m_lcbPlfLfo = 0;        // 0x02ee length
    /*
        special Break handling for text-box-stories in WW8
    */
    WW8_FC m_fcPlcftxbxBkd = 0;   // 0x02f2 PLCF for TextBox-Break-descriptors in the Maintext
    sal_Int32 m_lcbPlcftxbxBkd = 0;   // 0x02f6
    WW8_FC m_fcPlcfHdrtxbxBkd = 0;// 0x02fa PLCF for TextBox-Break-descriptors in the Header-/Footer- area
    sal_Int32 m_lcbPlcfHdrtxbxBkd = 0;// 0x02fe

    // 0x302 - 372 == ignore
    /*
        ListNames (skip to here!)
    */
    WW8_FC m_fcSttbListNames = 0;// 0x0372 PLCF for Listname Table
    sal_Int32 m_lcbSttbListNames = 0;// 0x0376

    WW8_FC m_fcPlcfTch = 0;
    sal_Int32 m_lcbPlcfTch = 0;

    // 0x38A - 41A == ignore
    WW8_FC m_fcAtrdExtra = 0;
    sal_uInt32 m_lcbAtrdExtra = 0;

    // 0x422 - 0x429 == ignore

    /// 0x42a smart-tag bookmark string table offset.
    WW8_FC m_fcSttbfBkmkFactoid = 0;
    /// 0x42e smart-tag bookmark string table length.
    sal_uInt32 m_lcbSttbfBkmkFactoid = 0;
    /// 0x432 smart-tag bookmark starts offset.
    WW8_FC m_fcPlcfBkfFactoid = 0;
    /// 0x436 smart-tag bookmark ends length.
    sal_uInt32 m_lcbPlcfBkfFactoid = 0;

    // 0x43a - 0x441 == ignore

    /// 0x442 smart-tag bookmark ends offset.
    WW8_FC m_fcPlcfBklFactoid = 0;
    /// 0x446 smart-tag bookmark ends length.
    sal_uInt32 m_lcbPlcfBklFactoid = 0;
    /// 0x44a smart tag data offset.
    WW8_FC m_fcFactoidData = 0;
    /// 0x44e smart tag data length.
    sal_uInt32 m_lcbFactoidData = 0;

    // 0x452 - 0x4b9 == ignore

    /// 0x4ba Plcffactoid offset.
    WW8_FC m_fcPlcffactoid = 0;
    /// 0x4be Plcffactoid offset.
    sal_uInt32 m_lcbPlcffactoid = 0;

    // 0x4bf - 0x4d4 == ignore

    WW8_FC m_fcHplxsdr = 0;    //bizarrely, word xp seems to require this set to shows dates from AtrdExtra
    sal_uInt32 m_lcbHplxsdr = 0;

    /*
        general variables that were used for Ver67 and Ver8,
        even though they had different sizes in the corresponding files:
    */
    sal_Int32 m_pnChpFirst = 0;
    sal_Int32 m_pnPapFirst = 0;
    sal_Int32 m_cpnBteChp = 0;
    sal_Int32 m_cpnBtePap = 0;
    /*
        The actual nFib, moved here because some readers assumed
        they couldn't read any format with nFib > some constant
    */
    sal_uInt16 m_nFib_actual = 0; // 0x05bc #i56856#

    WW8Fib(SvStream& rStrm, sal_uInt8 nWantedVersion,sal_uInt32 nOffset=0);
    explicit WW8Fib(sal_uInt8 nVersion, bool bDot = false);

    void WriteHeader(SvStream& rStrm);
    void Write(SvStream& rStrm);
    static rtl_TextEncoding GetFIBCharset(sal_uInt16 chs, LanguageType nLidLocale);
    ww::WordVersion GetFIBVersion() const;
    bool GetBaseCp(ManTypes nType, WW8_CP * cp) const;
    sal_Unicode getNumDecimalSep() const { return m_nNumDecimalSep;}
};

class WW8Style
{
protected:
    WW8Fib& m_rFib;
    SvStream& m_rStream;

    sal_uInt16  m_cstd;                      // Count of styles in stylesheet
    sal_uInt16  m_cbSTDBaseInFile;           // Length of STD Base as stored in a file
    sal_uInt16  m_fStdStylenamesWritten : 1; // Are built-in stylenames stored?
    sal_uInt16  : 15;                      // Spare flags
    sal_uInt16  m_stiMaxWhenSaved;           // Max sti known when file was written
    sal_uInt16  m_istdMaxFixedWhenSaved;     // How many fixed-index istds are there?
    sal_uInt16  m_nVerBuiltInNamesWhenSaved; // Current version of built-in stylenames
    // ftc used by StandardChpStsh for this document
    sal_uInt16  m_ftcAsci;
    // CJK ftc used by StandardChpStsh for this document
    sal_uInt16  m_ftcFE;
    // CTL/Other ftc used by StandardChpStsh for this document
    sal_uInt16  m_ftcOther;
    // CTL ftc used by StandardChpStsh for this document
    sal_uInt16  m_ftcBi;

    //No copying
    WW8Style(const WW8Style&);
    WW8Style& operator=(const WW8Style&);

public:
    WW8Style( SvStream& rSt, WW8Fib& rFibPara );
    std::unique_ptr<WW8_STD> Read1STDFixed(sal_uInt16& rSkip);
    std::unique_ptr<WW8_STD> Read1Style(sal_uInt16& rSkip, OUString* pString);
    sal_uInt16 GetCount() const { return m_cstd; }
};

class WW8Fonts final
{
private:
    WW8Fonts(const WW8Fonts&) = delete;
    WW8Fonts& operator=(const WW8Fonts&) = delete;

    std::vector<WW8_FFN> m_aFontA; // Array of Pointers to Font Description

public:
    WW8Fonts( SvStream& rSt, WW8Fib const & rFib );
    const WW8_FFN* GetFont( sal_uInt16 nNum ) const;
    sal_uInt16 GetMax() const { return m_aFontA.size(); }
};

typedef sal_uInt8 HdFtFlags;
namespace nsHdFtFlags
{
    const HdFtFlags WW8_HEADER_EVEN     = 0x01;
    const HdFtFlags WW8_HEADER_ODD      = 0x02;
    const HdFtFlags WW8_FOOTER_EVEN     = 0x04;
    const HdFtFlags WW8_FOOTER_ODD      = 0x08;
    const HdFtFlags WW8_HEADER_FIRST    = 0x10;
    const HdFtFlags WW8_FOOTER_FIRST    = 0x20;
}

/// Document Properties
struct WW8Dop
{
public:
    /* Error Status */
    ErrCode     nDopError;
    /*
    Corresponds only roughly to the actual structure of the Winword DOP,
    the winword FIB version matters to what exists.
    */
     bool        fFacingPages : 1 /*= false*/;   // 1 when facing pages should be printed

     bool        fWidowControl : 1 /*= false*/;  //a: orig 97 docs say
                                    //    1 when widow control is in effect. 0 when widow control disabled.
                                    //b: MS-DOC: Word Binary File Format (.doc) Structure Specification 2008 says
                                    //    B - unused1 (1 bit): Undefined and MUST be ignored.

     bool        fPMHMainDoc : 1 /*= false*/;    // 1 when doc is a main doc for Print Merge Helper, 0 when not; default=0
     sal_uInt16  grfSuppression : 2 /*= 0*/; // 0 Default line suppression storage; 0= form letter line suppression; 1= no line suppression; default=0
     sal_uInt16  fpc : 2 /*= 0*/;            // 1 footnote position code: 0 as endnotes, 1 at bottom of page, 2 immediately beneath text
    sal_uInt16  : 1;                // 0 unused

     sal_uInt16  grpfIhdt : 8 /*= 0*/;           // 0 specification of document headers and footers. See explanation under Headers and Footers topic.

     sal_uInt16  rncFootnote : 2 /*= 0*/;         // 0 restart index for footnotes, 0 don't restart note numbering, 1 section, 2 page
     sal_uInt16  nFootnote : 14 /*= 0*/;          // 1 initial footnote number for document
     bool        fOutlineDirtySave : 1 /*= false*/; // when 1, indicates that information in the hPLCFpad should be refreshed since outline has been dirtied
    sal_uInt16  : 7;                //   reserved
     bool        fOnlyMacPics : 1 /*= false*/;   //   when 1, Word believes all pictures recorded in the document were created on a Macintosh
     bool        fOnlyWinPics : 1 /*= false*/;   //   when 1, Word believes all pictures recorded in the document were created in Windows
     bool        fLabelDoc : 1 /*= false*/;      //   when 1, document was created as a print merge labels document
     bool        fHyphCapitals : 1 /*= false*/;  //   when 1, Word is allowed to hyphenate words that are capitalized. When 0, capitalized may not be hyphenated
     bool        fAutoHyphen : 1 /*= false*/;    //   when 1, Word will hyphenate newly typed text as a background task
     bool        fFormNoFields : 1 /*= false*/;
     bool        fLinkStyles : 1 /*= false*/;    //   when 1, Word will merge styles from its template
     bool        fRevMarking : 1 /*= false*/;    //   when 1, Word will mark revisions as the document is edited
     bool        fBackup : 1 /*= false*/;        //   always make backup when document saved when 1.
     bool        fExactCWords : 1 /*= false*/;
     bool        fPagHidden : 1 /*= false*/;
     bool        fPagResults : 1 /*= false*/;
     bool        fLockAtn : 1 /*= false*/;       //   when 1, annotations are locked for editing
     bool        fMirrorMargins : 1 /*= false*/; //   swap margins on left/right pages when 1.
     bool        fReadOnlyRecommended : 1 /*= false*/;// user has recommended that this doc be opened read-only when 1
     bool        fDfltTrueType : 1 /*= false*/;  //   when 1, use TrueType fonts by default (flag obeyed only when doc was created by WinWord 2.x)
     bool        fPagSuppressTopSpacing : 1 /*= false*/;//when 1, file created with SUPPRESSTOPSPACING=YES in win.ini. (flag obeyed only when doc was created by WinWord 2.x).
     bool        fProtEnabled : 1 /*= false*/;   //   when 1, document is protected from edit operations
     bool        fDispFormFieldSel : 1 /*= false*/;//   when 1, restrict selections to occur only within form fields
     bool        fRMView : 1 /*= false*/;        //   when 1, show revision markings on screen
     bool        fRMPrint : 1 /*= false*/;       //   when 1, print revision marks when document is printed
     bool        fWriteReservation : 1 /*= false*/;
     bool        fLockRev : 1 /*= false*/;       //   when 1, the current revision marking state is locked
     bool        fEmbedFonts : 1 /*= false*/;    //   when 1, document contains embedded True Type fonts
    //    compatibility options
     bool       copts_fNoTabForInd : 1 /*= false*/;          //    when 1, don't add automatic tab stops for hanging indent
     bool       copts_fNoSpaceRaiseLower : 1 /*= false*/;        //    when 1, don't add extra space for raised or lowered characters
     bool       copts_fSupressSpbfAfterPgBrk : 1 /*= false*/;    // when 1, suppress the paragraph Space Before and Space After options after a page break
     bool       copts_fWrapTrailSpaces : 1 /*= false*/;      //    when 1, wrap trailing spaces at the end of a line to the next line
     bool       copts_fMapPrintTextColor : 1 /*= false*/;        //    when 1, print colors as black on non-color printers
     bool       copts_fNoColumnBalance : 1 /*= false*/;      //    when 1, don't balance columns for Continuous Section starts
     bool       copts_fConvMailMergeEsc : 1 /*= false*/;
     bool       copts_fSupressTopSpacing : 1 /*= false*/;        //    when 1, suppress extra line spacing at top of page
     bool       copts_fOrigWordTableRules : 1 /*= false*/;   //    when 1, combine table borders like Word 5.x for the Macintosh
     bool       copts_fTransparentMetafiles : 1 /*= false*/; //    when 1, don't blank area between metafile pictures
     bool       copts_fShowBreaksInFrames : 1 /*= false*/;   //    when 1, show hard page or column breaks in frames
     bool       copts_fSwapBordersFacingPgs : 1 /*= false*/; //    when 1, swap left and right pages on odd facing pages
     bool       copts_fExpShRtn : 1 /*= false*/;             //    when 1, expand character spaces on the line ending SHIFT+RETURN  // #i56856#

    sal_Int16  dxaTab = 0;              //      720 twips - default tab width
    sal_uInt16 wSpare = 0;
    sal_uInt16 dxaHotZ = 0;             //      width of hyphenation hot zone measured in twips
    sal_uInt16 cConsecHypLim = 0;       //      number of lines allowed to have consecutive hyphens
    sal_uInt16 wSpare2 = 0;             //      reserved
    sal_Int32  dttmCreated = 0;         //      DTTM date and time document was created
    sal_Int32  dttmRevised = 0;         //      DTTM date and time document was last revised
    sal_Int32  dttmLastPrint = 0;       //      DTTM date and time document was last printed
    sal_Int16  nRevision = 0;           //      number of times document has been revised since its creation
    sal_Int32  tmEdited = 0;            //      time document was last edited
    sal_Int32  cWords = 0;              //      count of words tallied by last Word Count execution
    sal_Int32  cCh = 0;                 //      count of characters tallied by last Word Count execution
    sal_Int16  cPg = 0;                 //      count of pages tallied by last Word Count execution
    sal_Int32  cParas = 0;              //      count of paragraphs tallied by last Word Count execution
     sal_uInt16 rncEdn : 2 /*= 0*/;          //      restart endnote number code: 0 don't restart endnote numbering, 1 section, 2 page
     sal_uInt16 nEdn : 14 /*= 0*/;           //      beginning endnote number
     sal_uInt16 epc : 2 /*= 0*/;             //      endnote position code: 0 at end of section, 3 at end of document

     bool       fPrintFormData : 1 /*= false*/;  //      only print data inside of form fields
     bool       fSaveFormData : 1 /*= false*/;   //      only save document data that is inside of a form field.
     bool       fShadeFormData : 1 /*= false*/;  //      shade form fields
    sal_uInt16 : 2;                 //      reserved
     bool       fWCFootnoteEdn : 1 /*= false*/;       //      when 1, include footnotes and endnotes in word count
    sal_Int32   cLines = 0;             //      count of lines tallied by last Word Count operation
    sal_Int32   cWordsFootnoteEnd = 0;       //      count of words in footnotes and endnotes tallied by last Word Count operation
    sal_Int32   cChFootnoteEdn = 0;          //      count of characters in footnotes and endnotes tallied by last Word Count operation
    sal_Int16   cPgFootnoteEdn = 0;          //      count of pages in footnotes and endnotes tallied by last Word Count operation
    sal_Int32   cParasFootnoteEdn = 0;       //      count of paragraphs in footnotes and endnotes tallied by last Word Count operation
    sal_Int32   cLinesFootnoteEdn = 0;       //      count of paragraphs in footnotes and endnotes tallied by last Word Count operation
    sal_Int32   lKeyProtDoc = 0;        //      document protection password key, only valid if dop.fProtEnabled, dop.fLockAtn or dop.fLockRev are 1.
     sal_uInt16  wvkSaved : 3 /*= 0*/;       //      document view kind: 0 Normal view, 1 Outline view, 2 Page View
     sal_uInt16  wScaleSaved : 9 /*= 0*/;    ///< Specifies the zoom percentage that was in use when the document was saved.
     sal_uInt16  zkSaved : 2 /*= 0*/;        //      document zoom type: 0 percent, 1 whole/entire page, 2 page width, 3 text width/optimal
     bool        fRotateFontW6 : 1 /*= false*/;
     bool        iGutterPos : 1 /*= false*/;

    // this should be the end for nFib < 103, otherwise the file is broken!

    /*
        for nFib >= 103 it continues:
    */
     bool       fNoTabForInd : 1 /*= false*/;                      // see above in compatibility options
     bool       fNoSpaceRaiseLower : 1 /*= false*/;                // see above
     bool       fSupressSpbfAfterPageBreak : 1 /*= false*/;        // see above
     bool       fWrapTrailSpaces : 1 /*= false*/;                  // see above
     bool       fMapPrintTextColor : 1 /*= false*/;                // see above
     bool       fNoColumnBalance : 1 /*= false*/;                  // see above
     bool       fConvMailMergeEsc : 1 /*= false*/;                 // see above
     bool       fSupressTopSpacing : 1 /*= false*/;                // see above
     bool       fOrigWordTableRules : 1 /*= false*/;               // see above
     bool       fTransparentMetafiles : 1 /*= false*/;             // see above
     bool       fShowBreaksInFrames : 1 /*= false*/;               // see above
     bool       fSwapBordersFacingPgs : 1 /*= false*/;             // see above
     bool       fCompatibilityOptions_Unknown1_13 : 1 /*= false*/; // #i78591#
     bool       fExpShRtn : 1 /*= false*/;                         // #i78591# and #i56856#
     bool       fCompatibilityOptions_Unknown1_15 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_16 : 1 /*= false*/; // #i78591#
     bool       fSuppressTopSpacingMac5 : 1 /*= false*/;           // Suppress extra line spacing at top
                                                      // of page like MacWord 5.x
     bool       fTruncDxaExpand : 1 /*= false*/;                    // Expand/Condense by whole number of points
     bool       fPrintBodyBeforeHdr : 1 /*= false*/;               // Print body text before header/footer
     bool       fNoLeading : 1 /*= false*/;                        // Don't add extra spacebetween rows of text
     bool       fCompatibilityOptions_Unknown1_21 : 1 /*= false*/; // #i78591#
     bool       fMWSmallCaps : 1 /*= false*/;                     // Use larger small caps like MacWord 5.x
     bool       fCompatibilityOptions_Unknown1_23 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_24 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_25 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_26 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_27 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_28 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_29 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_30 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown1_31 : 1 /*= false*/; // #i78591#
     bool       fUsePrinterMetrics : 1 /*= false*/;                //The magic option

    // this should be the end for nFib <= 105, otherwise the file is broken!

    /*
        for nFib > 105 it continues:
    */
    sal_Int16   adt = 0;                  // Autoformat Document Type:
                                      // 0 for normal.
                                      // 1 for letter, and
                                      // 2 for email.
    WW8DopTypography doptypography = {};   // see WW8STRUC.HXX
    WW8_DOGRID dogrid = {};                // see WW8STRUC.HXX
    sal_uInt16 : 1;                   // reserved
     sal_uInt16 lvl : 4 /*= 0*/;               // Which outline levels are showing in outline view
    sal_uInt16 : 4;                   // reserved
     bool       fHtmlDoc : 1 /*= false*/;          // This file is based upon an HTML file
    sal_uInt16 : 1;                   // reserved
     bool       fSnapBorder : 1 /*= false*/;       // Snap table and page borders to page border
     bool       fIncludeHeader : 1 /*= false*/;    // Place header inside page border
     bool       fIncludeFooter : 1 /*= false*/;    // Place footer inside page border
     bool       fForcePageSizePag : 1 /*= false*/; // Are we in online view
     bool       fMinFontSizePag : 1 /*= false*/;   // Are we auto-promoting fonts to >= hpsZoomFontPag?
     bool       fHaveVersions : 1 /*= false*/;     // versioning is turned on
     bool       fAutoVersion : 1 /*= false*/;      // autoversioning is enabled
    sal_uInt16 : 14;                  // reserved
    // Skip 12 Bytes here: ASUMI
    sal_Int32 cChWS = 0;
    sal_Int32 cChWSFootnoteEdn = 0;
    sal_Int32 grfDocEvents = 0;
    // Skip 4+30+8 Bytes here
    sal_Int32 cDBC = 0;
    sal_Int32 cDBCFootnoteEdn = 0;
    // Skip 4 Bytes here
    sal_Int16 nfcFootnoteRef = 0;
    sal_Int16 nfcEdnRef = 0;
    sal_Int16 hpsZoomFontPag = 0;
    sal_Int16 dywDispPag = 0;

     bool       fCompatibilityOptions_Unknown2_1 : 1 /*= false*/;  // #i78591#
     bool       fCompatibilityOptions_Unknown2_2 : 1 /*= false*/;  // #i78591#
     bool       fDontUseHTMLAutoSpacing : 1 /*= false*/;
     bool       fCompatibilityOptions_Unknown2_4 : 1 /*= false*/;  // #i78591#
     bool       fCompatibilityOptions_Unknown2_5 : 1 /*= false*/;  // #i78591#
     bool       fCompatibilityOptions_Unknown2_6 : 1 /*= false*/;  // #i78591#
     bool       fCompatibilityOptions_Unknown2_7 : 1 /*= false*/;  // #i78591#
     bool       fCompatibilityOptions_Unknown2_8 : 1 /*= false*/;  // #i78591#
     bool       fCompatibilityOptions_Unknown2_9 : 1 /*= false*/;  // #i78591#
     bool       fCompatibilityOptions_Unknown2_10 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_11 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_12 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_13 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_14 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_15 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_16 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_17 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_18 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_19 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_20 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_21 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_22 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_23 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_24 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_25 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_26 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_27 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_28 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_29 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_30 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_31 : 1 /*= false*/; // #i78591#
     bool       fCompatibilityOptions_Unknown2_32 : 1 /*= false*/; // #i78591#

     sal_uInt16 fUnknown3 : 15 /*= 0*/;
     bool       fUseBackGroundInAllmodes : 1 /*= false*/;

     bool       fDoNotEmbedSystemFont : 1 /*= false*/;
     bool       fWordCompat : 1 /*= false*/;
     bool       fLiveRecover : 1 /*= false*/;
     bool       fEmbedFactoids : 1 /*= false*/;
     bool       fFactoidXML : 1 /*= false*/;
     bool       fFactoidAllDone : 1 /*= false*/;
     bool       fFolioPrint : 1 /*= false*/;
     bool       fReverseFolio : 1 /*= false*/;
     sal_uInt16 iTextLineEnding : 3 /*= 0*/;
     bool       fHideFcc : 1 /*= false*/;
     bool       fAcetateShowMarkup : 1 /*= false*/;
     bool       fAcetateShowAtn : 1 /*= false*/;
     bool       fAcetateShowInsDel : 1 /*= false*/;
     bool       fAcetateShowProps : 1 /*= false*/;

    bool bUseThaiLineBreakingRules = false;

    /* Constructor for importing, needs to know the version of word used */
    WW8Dop(SvStream& rSt, sal_Int16 nFib, sal_Int32 nPos, sal_uInt32 nSize);

    /* Constructs default DOP suitable for exporting */
    WW8Dop();
    void Write(SvStream& rStrm, WW8Fib& rFib) const;

    sal_uInt32 GetCompatibilityOptions() const;
    void SetCompatibilityOptions(sal_uInt32 a32Bit);
    // i#78591#
    sal_uInt32 GetCompatibilityOptions2() const;
    void SetCompatibilityOptions2(sal_uInt32 a32Bit);
};

class WW8PLCF_HdFt
{
private:
    WW8PLCF aPLCF;
    short nIdxOffset;

public:
    WW8PLCF_HdFt( SvStream* pSt, WW8Fib const & rFib, WW8Dop const & rDop );
    bool GetTextPos(sal_uInt8 grpfIhdt, sal_uInt8 nWhich, WW8_CP& rStart, WW8_CP& rLen);
    void GetTextPosExact(short nIdx, WW8_CP& rStart, WW8_CP& rLen);
    void UpdateIndex( sal_uInt8 grpfIhdt );
};

Word2CHPX ReadWord2Chpx(SvStream &rSt, std::size_t nOffset, sal_uInt8 nSize);
std::vector<sal_uInt8> ChpxToSprms(const Word2CHPX &rChpx);

[[nodiscard]] bool checkRead(SvStream &rSt, void *pDest, sal_uInt32 nLength);

//MS has a (slightly) inaccurate view of how many twips
//are in the default letter size of a page
const sal_uInt16 lLetterWidth = 12242;
const sal_uInt16 lLetterHeight = 15842;

#ifdef OSL_BIGENDIAN
void swapEndian(sal_Unicode *pString);
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

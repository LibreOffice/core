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
#ifndef INCLUDED_SW_SOURCE_FILTER_INC_FLTSHELL_HXX
#define INCLUDED_SW_SOURCE_FILTER_INC_FLTSHELL_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <hintids.hxx>
#include <vcl/keycod.hxx>
#include <tools/datetime.hxx>
#include <editeng/formatbreakitem.hxx>
#include <poolfmt.hxx>
#include <fmtornt.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <IDocumentRedlineAccess.hxx>

#include <boost/noncopyable.hpp>

#include <memory>
#include <deque>

class SwTOXBase;
class SwField;
class SwFieldType;
class Graphic;
class SwTableBox;
class SwDoc;
class SwPaM;

inline bool SwFltGetFlag(sal_uLong nFieldFlags, int no)
    { return (nFieldFlags & (sal_uLong(1) << no)) != 0; }

//Subvert the Node/Content system to get positions which don't update as
//content is appended to them
struct SW_DLLPUBLIC SwFltPosition
{
public:
    SwNodeIndex m_nNode;
    sal_Int32 m_nContent;
public:
    SwFltPosition(const SwFltPosition &rOther)
        : m_nNode(rOther.m_nNode)
        , m_nContent(rOther.m_nContent)
    {
    }
    SwFltPosition &operator=(const SwFltPosition &rOther)
    {
        m_nNode = rOther.m_nNode;
        m_nContent = rOther.m_nContent;
        return *this;
    }
    bool operator==(const SwFltPosition &rOther) const
    {
        return (m_nContent == rOther.m_nContent &&
                m_nNode == rOther.m_nNode);
    }
    void SetPos(SwNodeIndex &rNode, sal_uInt16 nIdx)
    {
        m_nNode = rNode;
        m_nContent = nIdx;
    }
    //operators with SwPosition, where the node is hacked to the previous one,
    //and the offset to content is de-dynamic-ified
    SwFltPosition(const SwPosition &rPos)
        : m_nNode(rPos.nNode, -1)
        , m_nContent(rPos.nContent.GetIndex())
    {
    }
    void SetPos(const SwPosition &rPos)
    {
        m_nNode = rPos.nNode.GetIndex()-1;
        m_nContent = rPos.nContent.GetIndex();
    }
};

// Stack-Eintrag fuer die Attribute Es werden immer Pointer auf neue Attribute uebergeben.
class SwFltStackEntry : private ::boost::noncopyable
{
public:
    SwFltPosition m_aMkPos;
    SwFltPosition m_aPtPos;

    SfxPoolItem * pAttr;// Format Attribute

    bool bOld;          // to mark Attributes *before* skipping field results
    bool bOpen;     //Entry open, awaiting being closed
    bool bConsumedByField;

    sal_Int32 mnStartCP;
    sal_Int32 mnEndCP;
    bool bIsParaEnd;

    SW_DLLPUBLIC SwFltStackEntry(const SwPosition & rStartPos, SfxPoolItem* pHt );
    SW_DLLPUBLIC ~SwFltStackEntry();

    SW_DLLPUBLIC void SetEndPos(  const SwPosition & rEndPos);
    SW_DLLPUBLIC bool MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck) const;
    SW_DLLPUBLIC static bool MakeRegion(SwDoc* pDoc, SwPaM& rRegion,
        bool bCheck, const SwFltPosition &rMkPos, const SwFltPosition &rPtPos, bool bIsParaEnd=false,
        sal_uInt16 nWhich=0);

    void SetStartCP(sal_Int32 nCP) {mnStartCP = nCP;}
    void SetEndCP(sal_Int32 nCP) {mnEndCP = nCP;}
    sal_Int32 GetStartCP() const {return mnStartCP;}
    sal_Int32 GetEndCP() const {return mnEndCP;}
    bool IsParaEnd(){ return bIsParaEnd;}
    void SetIsParaEnd(bool bArg){ bIsParaEnd = bArg;}
};

class SW_DLLPUBLIC SwFltControlStack : private ::boost::noncopyable
{
    typedef std::deque<std::unique_ptr<SwFltStackEntry>> Entries;
    typedef Entries::iterator myEIter;
    Entries m_Entries;

    sal_uLong nFieldFlags;
    vcl::KeyCode aEmptyKeyCode; // fuer Bookmarks

private:
    bool bHasSdOD;
    bool bSdODChecked;

protected:
    SwDoc* pDoc;
    bool bIsEndStack;

    virtual void SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry& rEntry);
    virtual sal_Int32 GetCurrAttrCP() const {return -1;}
    virtual bool IsParaEndInCPs(sal_Int32 nStart,sal_Int32 nEnd,bool bSdOD=true) const;

    //Clear the para end position recorded in reader intermittently for the least impact on loading performance
    virtual void ClearParaEndPosition(){};
    virtual bool CheckSdOD(sal_Int32 nStart,sal_Int32 nEnd);
    bool HasSdOD();

public:
    void MoveAttrs( const SwPosition&  rPos );
    enum Flags
    {
        HYPO,
        TAGS_DO_ID,
        TAGS_VISIBLE,
        BOOK_TO_VAR_REF,
        BOOK_AND_REF,
        TAGS_IN_TEXT,
        ALLOW_FLD_CR,
        NO_FLD_CR,
        DONT_HARD_PROTECT
    };

    SwFltControlStack(SwDoc* pDo, sal_uLong nFieldFl);
    virtual ~SwFltControlStack();

    bool IsFlagSet(Flags no) const  { return ::SwFltGetFlag(nFieldFlags, no);}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr );

    virtual SwFltStackEntry* SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId=0, bool bTstEnde=true, long nHand = LONG_MAX, bool consumedByField=false);

    void StealAttr(const SwNodeIndex& rNode, sal_uInt16 nAttrId = 0);
    void MarkAllAttrsOld();
    void KillUnlockedAttrs(const SwPosition& pPos);
    SfxPoolItem* GetFormatStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos = 0);
    const SfxPoolItem* GetOpenStackAttr(const SwPosition& rPos, sal_uInt16 nWhich);
    void Delete(const SwPaM &rPam);

    bool empty() const { return m_Entries.empty(); }
    Entries::size_type size() const { return m_Entries.size(); }
    SwFltStackEntry& operator[](Entries::size_type nIndex)
         { return *m_Entries[nIndex]; }
    void DeleteAndDestroy(Entries::size_type nCnt);
};

class SwFltAnchorClient;

class SW_DLLPUBLIC SwFltAnchor : public SfxPoolItem
{
    SwFrameFormat* pFrameFormat;
    SwFltAnchorClient * pClient;

public:
    SwFltAnchor(SwFrameFormat* pFlyFormat);
    SwFltAnchor(const SwFltAnchor&);
    virtual ~SwFltAnchor();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const override;
    void SetFrameFormat(SwFrameFormat * _pFrameFormat);
    const SwFrameFormat* GetFrameFormat() const { return pFrameFormat;}
          SwFrameFormat* GetFrameFormat() { return pFrameFormat;}
};

class SwFltAnchorClient : public SwClient
{
    SwFltAnchor * m_pFltAnchor;

public:
    SwFltAnchorClient(SwFltAnchor * pFltAnchor);

    virtual void Modify (const SfxPoolItem *pOld, const SfxPoolItem *pNew) override;
};

class SW_DLLPUBLIC SwFltRedline : public SfxPoolItem
{
public:
    DateTime        aStamp;
    DateTime        aStampPrev;
    RedlineType_t   eType;
    RedlineType_t   eTypePrev;
    sal_uInt16          nAutorNo;
    sal_uInt16          nAutorNoPrev;

    SwFltRedline(RedlineType_t   eType_,
                 sal_uInt16          nAutorNo_,
                 const DateTime& rStamp_,
                 RedlineType_t   eTypePrev_    = nsRedlineType_t::REDLINE_INSERT,
                 sal_uInt16          nAutorNoPrev_ = USHRT_MAX,
                 const DateTime* pStampPrev_   = 0)
        : SfxPoolItem(RES_FLTR_REDLINE), aStamp(rStamp_),
        aStampPrev( DateTime::EMPTY ),
        eType(eType_),
        eTypePrev(eTypePrev_), nAutorNo(nAutorNo_), nAutorNoPrev(nAutorNoPrev_)
    {
            if( pStampPrev_ )
                aStampPrev = *pStampPrev_;
    }

    SwFltRedline(const SwFltRedline& rCpy):
        SfxPoolItem(RES_FLTR_REDLINE),
        aStamp(         rCpy.aStamp       ),
        aStampPrev(     rCpy.aStampPrev   ),
        eType(          rCpy.eType        ),
        eTypePrev(      rCpy.eTypePrev    ),
        nAutorNo(       rCpy.nAutorNo     ),
        nAutorNoPrev(   rCpy.nAutorNoPrev )
        {}
    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool operator==(const SfxPoolItem& rItem) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const override;
};

class SW_DLLPUBLIC SwFltBookmark : public SfxPoolItem
{
private:

    long mnHandle;
    OUString maName;
    OUString maVal;
    bool mbIsTOCBookmark;

public:
    SwFltBookmark( const OUString& rNa,
                   const OUString& rVa,
                   long nHand,
                   const bool bIsTOCBookmark = false );
    SwFltBookmark( const SwFltBookmark& );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const override;

    long GetHandle() const              { return mnHandle; }
    const OUString& GetName() const       { return maName; }
    const OUString& GetValSys() const     { return maVal; }
    bool IsTOCBookmark() const
    {
        return mbIsTOCBookmark;
    }
};

class SW_DLLPUBLIC SwFltTOX : public SfxPoolItem
{
    SwTOXBase* pTOXBase;
    sal_uInt16 nCols;
    bool bHadBreakItem; // there was a break item BEFORE insertion of the TOX
    bool bHadPageDescItem;
public:
    SwFltTOX(SwTOXBase* pBase, sal_uInt16 _nCols = 0);
    SwFltTOX(const SwFltTOX&);
    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const override;
    SwTOXBase* GetBase()            { return pTOXBase; }
    void SetHadBreakItem(    bool bVal ) { bHadBreakItem    = bVal; }
    void SetHadPageDescItem( bool bVal ) { bHadPageDescItem = bVal; }
    bool HadBreakItem()    const { return bHadBreakItem; }
    bool HadPageDescItem() const { return bHadPageDescItem; }
};

// Der WWEndStack verhaelt sich wie der WWControlStck, nur dass die Attribute
// auf ihm bis ans Ende des Dokuments gehortet werden, falls auf sie noch
// zugegriffen werden muss (z.B. Book/RefMarks, Index u.s.w.)
class SwFltEndStack : public SwFltControlStack
{
public:
    SwFltEndStack(SwDoc* pDo, sal_uLong nFieldFl)
        :SwFltControlStack(pDo, nFieldFl)
    {
        bIsEndStack = true;
    }
};

SW_DLLPUBLIC void UpdatePageDescs(SwDoc &rDoc, size_t nInPageDescOffset);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

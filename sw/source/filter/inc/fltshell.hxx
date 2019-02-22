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

#include <hintids.hxx>
#include <vcl/keycod.hxx>
#include <tools/datetime.hxx>
#include <editeng/formatbreakitem.hxx>
#include <poolfmt.hxx>
#include <fmtornt.hxx>
#include <mdiexp.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <strings.hrc>
#include <IDocumentRedlineAccess.hxx>

#include <cstddef>
#include <limits>
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
    bool operator==(const SwFltPosition &rOther) const
    {
        return (m_nContent == rOther.m_nContent &&
                m_nNode == rOther.m_nNode);
    }
    void SetPos(SwNodeIndex const &rNode, sal_uInt16 nIdx)
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

// Stack entry for the attributes. It is always pointers to new attributes that are passed.
class SwFltStackEntry
{
private:
    SwFltStackEntry(SwFltStackEntry const&) = delete;
    SwFltStackEntry& operator=(SwFltStackEntry const&) = delete;

public:
    SwFltPosition m_aMkPos;
    SwFltPosition m_aPtPos;

    std::unique_ptr<SfxPoolItem> pAttr;// Format Attribute

    bool bOld;          // to mark Attributes *before* skipping field results
    bool bOpen;     //Entry open, awaiting being closed
    bool bConsumedByField;
    bool m_isAnnotationOnEnd; ///< annotation already moved onto its end pos.

    sal_Int32 mnStartCP;
    sal_Int32 mnEndCP;
    bool bIsParaEnd;

    SW_DLLPUBLIC SwFltStackEntry(const SwPosition & rStartPos, std::unique_ptr<SfxPoolItem> pHt );
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

class SW_DLLPUBLIC SwFltControlStack
{
private:
    SwFltControlStack(SwFltControlStack const&) = delete;
    SwFltControlStack& operator=(SwFltControlStack const&) = delete;

    typedef std::vector<std::unique_ptr<SwFltStackEntry>> Entries;
    Entries m_Entries;

    sal_uLong const nFieldFlags;

    bool bHasSdOD;
    bool bSdODChecked;

protected:
    SwDoc* pDoc;
    bool bIsEndStack;

    virtual void SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry& rEntry);
    virtual sal_Int32 GetCurrAttrCP() const {return -1;}
    virtual bool IsParaEndInCPs(sal_Int32 nStart,sal_Int32 nEnd,bool bSdOD) const;

    //Clear the para end position recorded in reader intermittently for the least impact on loading performance
    virtual void ClearParaEndPosition(){};
    virtual bool CheckSdOD(sal_Int32 nStart,sal_Int32 nEnd);
    bool HasSdOD();

public:
    enum class MoveAttrsMode { DEFAULT, POSTIT_INSERTED };
    void MoveAttrs(const SwPosition& rPos, MoveAttrsMode = MoveAttrsMode::DEFAULT);
    enum Flags
    {
        HYPO,
        TAGS_DO_ID,
        TAGS_VISIBLE,
        BOOK_TO_VAR_REF,
        BOOK_AND_REF,
        TAGS_IN_TEXT,
        ALLOW_FLD_CR
    };

    SwFltControlStack(SwDoc* pDo, sal_uLong nFieldFl);
    virtual ~SwFltControlStack();

    bool IsFlagSet(Flags no) const  { return ::SwFltGetFlag(nFieldFlags, no);}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr );

    virtual SwFltStackEntry* SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId, bool bTstEnde=true, long nHand = LONG_MAX, bool consumedByField=false);

    void StealAttr(const SwNodeIndex& rNode);
    void MarkAllAttrsOld();
    void KillUnlockedAttrs(const SwPosition& pPos);
    SfxPoolItem* GetFormatStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos);
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
    std::unique_ptr<SwFltAnchorClient> pClient;

public:
    SwFltAnchor(SwFrameFormat* pFlyFormat);
    SwFltAnchor(const SwFltAnchor&);
    virtual ~SwFltAnchor() override;

    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = nullptr) const override;
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
    DateTime const        aStamp;
    DateTime const        aStampPrev;
    RedlineType_t const   eType;
    RedlineType_t const   eTypePrev;
    std::size_t const     nAutorNo;
    std::size_t const     nAutorNoPrev;

    static constexpr auto NoPrevAuthor
        = std::numeric_limits<std::size_t>::max();

    SwFltRedline(RedlineType_t   eType_,
                 std::size_t     nAutorNo_,
                 const DateTime& rStamp_,
                 RedlineType_t   eTypePrev_    = nsRedlineType_t::REDLINE_INSERT,
                 std::size_t     nAutorNoPrev_ = NoPrevAuthor)
        : SfxPoolItem(RES_FLTR_REDLINE), aStamp(rStamp_),
        aStampPrev( DateTime::EMPTY ),
        eType(eType_),
        eTypePrev(eTypePrev_), nAutorNo(nAutorNo_), nAutorNoPrev(nAutorNoPrev_)
    {
    }

    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem& rItem) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = nullptr) const override;
};

class SW_DLLPUBLIC SwFltBookmark : public SfxPoolItem
{
private:

    long const mnHandle;
    OUString maName;
    OUString const maVal;
    bool const mbIsTOCBookmark;

public:
    SwFltBookmark( const OUString& rNa,
                   const OUString& rVa,
                   long nHand,
                   const bool bIsTOCBookmark = false );

    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = nullptr) const override;

    long GetHandle() const              { return mnHandle; }
    const OUString& GetName() const       { return maName; }
    const OUString& GetValSys() const     { return maVal; }
    bool IsTOCBookmark() const
    {
        return mbIsTOCBookmark;
    }
};

/// Stores RDF statements on a paragraph (key-value pairs where the subject is the paragraph).
class SW_DLLPUBLIC SwFltRDFMark : public SfxPoolItem
{
    long m_nHandle;
    std::vector< std::pair<OUString, OUString> > m_aAttributes;

public:
    SwFltRDFMark();

    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = nullptr) const override;

    void SetHandle(long nHandle);
    long GetHandle() const;
    void SetAttributes(const std::vector< std::pair<OUString, OUString> >& rAttributes);
    const std::vector< std::pair<OUString, OUString> >& GetAttributes() const;
};

class SW_DLLPUBLIC SwFltTOX : public SfxPoolItem
{
    std::shared_ptr<SwTOXBase> m_xTOXBase;
    bool bHadBreakItem; // there was a break item BEFORE insertion of the TOX
    bool bHadPageDescItem;
public:
    SwFltTOX(SwTOXBase* pBase);
    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* = nullptr) const override;
    const SwTOXBase& GetBase() { return *m_xTOXBase; }
    void SetHadBreakItem(    bool bVal ) { bHadBreakItem    = bVal; }
    void SetHadPageDescItem( bool bVal ) { bHadPageDescItem = bVal; }
    bool HadBreakItem()    const { return bHadBreakItem; }
    bool HadPageDescItem() const { return bHadPageDescItem; }
};

// The WWEndStack behaves like the WWControlStack, except that the attributes
// on it are hoarded to the end of the document if they need to be accessed
// (e.g., book/RefMarks, index, etc.).
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

class ImportProgress
{
private:
    SwDocShell * const m_pDocShell;
public:
    ImportProgress(SwDocShell *pDocShell, long nStartVal, long nEndVal)
        : m_pDocShell(pDocShell)
    {
        ::StartProgress(STR_STATSTR_W4WREAD, nStartVal, nEndVal, m_pDocShell);
    }

    void Update(sal_uInt16 nProgress)
    {
        ::SetProgressState(nProgress, m_pDocShell);    // Update
    }

    ~ImportProgress()
    {
        ::EndProgress(m_pDocShell);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

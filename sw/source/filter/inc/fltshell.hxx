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
#include <svl/listener.hxx>
#include <tools/datetime.hxx>
#include <mdiexp.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <strings.hrc>
#include <IDocumentRedlineAccess.hxx>

#include <cstddef>
#include <limits>
#include <memory>

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

    enum class RegionMode { NoCheck = 0, CheckNodes = 1<<0, CheckFieldmark = 1<<1 };
    SW_DLLPUBLIC void SetEndPos(  const SwPosition & rEndPos);
    SW_DLLPUBLIC bool MakeRegion(SwDoc& rDoc, SwPaM& rRegion, RegionMode eCheck) const;
    SW_DLLPUBLIC static bool MakeRegion(SwDoc& rDoc, SwPaM& rRegion,
        RegionMode eCheck, const SwFltPosition &rMkPos, const SwFltPosition &rPtPos, bool bIsParaEnd=false,
        sal_uInt16 nWhich=0);

    void SetStartCP(sal_Int32 nCP) {mnStartCP = nCP;}
    void SetEndCP(sal_Int32 nCP) {mnEndCP = nCP;}
    sal_Int32 GetStartCP() const {return mnStartCP;}
    sal_Int32 GetEndCP() const {return mnEndCP;}
    bool IsParaEnd() const { return bIsParaEnd;}
    void SetIsParaEnd(bool bArg){ bIsParaEnd = bArg;}
};

template<> struct o3tl::typed_flags<SwFltStackEntry::RegionMode>: o3tl::is_typed_flags<SwFltStackEntry::RegionMode, 0x03> {};

class SW_DLLPUBLIC SwFltControlStack
{
private:
    SwFltControlStack(SwFltControlStack const&) = delete;
    SwFltControlStack& operator=(SwFltControlStack const&) = delete;

    typedef std::vector<std::unique_ptr<SwFltStackEntry>> Entries;
    Entries m_Entries;

    sal_uLong nFieldFlags;

    bool bHasSdOD;
    bool bSdODChecked;

protected:
    SwDoc& rDoc;
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

    SwFltControlStack(SwDoc& rDo, sal_uLong nFieldFl);
    virtual ~SwFltControlStack();

    bool IsFlagSet(Flags no) const  { return ::SwFltGetFlag(nFieldFlags, no);}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr );

    virtual SwFltStackEntry* SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId, bool bTstEnd=true, tools::Long nHand = LONG_MAX, bool consumedByField=false);

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

class SwFltAnchorListener;

class SW_DLLPUBLIC SwFltAnchor: public SfxPoolItem
{
    SwFrameFormat* pFrameFormat;
    std::unique_ptr<SwFltAnchorListener> pListener;

public:
    SwFltAnchor(SwFrameFormat* pFlyFormat);
    SwFltAnchor(const SwFltAnchor&);
    virtual ~SwFltAnchor() override;

    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SwFltAnchor* Clone(SfxItemPool* = nullptr) const override;
    void SetFrameFormat(SwFrameFormat* _pFrameFormat);
    const SwFrameFormat* GetFrameFormat() const { return pFrameFormat; }
          SwFrameFormat* GetFrameFormat() { return pFrameFormat; }
};

class SwFltAnchorListener : public SvtListener
{
    SwFltAnchor* m_pFltAnchor;
    public:
        SwFltAnchorListener(SwFltAnchor* pFltAnchor);
        virtual void Notify(const SfxHint&) override;
};

class SW_DLLPUBLIC SwFltRedline : public SfxPoolItem
{
public:
    DateTime        aStamp;
    RedlineType     eType;
    std::size_t     nAutorNo;

    SwFltRedline(RedlineType   eType_,
                 std::size_t     nAutorNo_,
                 const DateTime& rStamp_)
        : SfxPoolItem(RES_FLTR_REDLINE), aStamp(rStamp_),
        eType(eType_),
        nAutorNo(nAutorNo_)
    {
    }

    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem& rItem) const override;
    virtual SwFltRedline* Clone(SfxItemPool* = nullptr) const override;
};

class SW_DLLPUBLIC SwFltBookmark : public SfxPoolItem
{
private:

    tools::Long mnHandle;
    OUString maName;
    OUString maVal;
    bool mbIsTOCBookmark;

public:
    SwFltBookmark( const OUString& rNa,
                   const OUString& rVa,
                   tools::Long nHand,
                   const bool bIsTOCBookmark = false );

    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SwFltBookmark* Clone(SfxItemPool* = nullptr) const override;

    tools::Long GetHandle() const              { return mnHandle; }
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
    tools::Long m_nHandle;
    std::vector< std::pair<OUString, OUString> > m_aAttributes;

public:
    SwFltRDFMark();

    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SwFltRDFMark* Clone(SfxItemPool* = nullptr) const override;

    void SetHandle(tools::Long nHandle);
    tools::Long GetHandle() const;
    void SetAttributes(const std::vector< std::pair<OUString, OUString> >& rAttributes);
    const std::vector< std::pair<OUString, OUString> >& GetAttributes() const;
};

class SW_DLLPUBLIC SwFltTOX : public SfxPoolItem
{
    std::shared_ptr<SwTOXBase> m_xTOXBase;
    bool bHadBreakItem; // there was a break item BEFORE insertion of the TOX
    bool bHadPageDescItem;
public:
    SwFltTOX(std::shared_ptr<SwTOXBase> xBase);
    // "purely virtual methods" of SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SwFltTOX* Clone(SfxItemPool* = nullptr) const override;
    const SwTOXBase& GetBase() const { return *m_xTOXBase; }
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
    SwFltEndStack(SwDoc& rDo, sal_uLong nFieldFl)
        :SwFltControlStack(rDo, nFieldFl)
    {
        bIsEndStack = true;
    }
};

SW_DLLPUBLIC void UpdatePageDescs(SwDoc &rDoc, size_t nInPageDescOffset);

class ImportProgress
{
private:
    SwDocShell *m_pDocShell;
public:
    ImportProgress(SwDocShell *pDocShell, tools::Long nStartVal, tools::Long nEndVal)
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

// detect if the SwFrameFormat it is watching was deleted
class SW_DLLPUBLIC FrameDeleteWatch final: public SvtListener
{
    SwFrameFormat* m_pFormat;
public:
    FrameDeleteWatch(SwFrameFormat* pFormat);

    virtual void Notify(const SfxHint& rHint) override;

    SwFrameFormat* GetFormat()
    {
        return m_pFormat;
    }

    bool WasDeleted() const
    {
        return !m_pFormat;
    }

    virtual ~FrameDeleteWatch() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

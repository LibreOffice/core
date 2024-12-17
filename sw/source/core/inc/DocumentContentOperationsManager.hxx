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

#pragma once

#include <IDocumentContentOperations.hxx>
#include <ndarr.hxx> //Only for lcl_RstTxtAttr

class SwDoc;
class SwNoTextNode;
class SwFormatColl;
class SwHistory;

namespace sw
{

class DocumentContentOperationsManager final : public IDocumentContentOperations
{
public:
    DocumentContentOperationsManager( SwDoc& i_rSwdoc );

    //Interface methods:
    bool CopyRange(SwPaM&, SwPosition&, SwCopyFlags, sal_uInt32 nMovedID = 0) const override;

    void DeleteSection(SwNode* pNode) override;

    void DeleteRange(SwPaM&) override;

    bool DelFullPara(SwPaM&) override;

    bool DeleteAndJoin(SwPaM&, SwDeleteFlags flags = SwDeleteFlags::Default) override;

    bool MoveRange(SwPaM&, SwPosition&, SwMoveFlags) override;

    bool MoveNodeRange(SwNodeRange&, SwNode&, SwMoveFlags) override;

    void MoveAndJoin(SwPaM&, SwPosition&) override;

    bool Overwrite(const SwPaM &rRg, const OUString& rStr) override;

    bool InsertString(const SwPaM &rRg, const OUString&,
              const SwInsertFlags nInsertMode = SwInsertFlags::EMPTYEXPAND ) override;

    void SetIME(bool bIME) override;

    bool GetIME() const override;

    void TransliterateText(const SwPaM& rPaM, utl::TransliterationWrapper&) override;

    SwFlyFrameFormat* InsertGraphic(const SwPaM &rRg, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet, const SfxItemSet* pGrfAttrSet, SwFrameFormat*) override;

    void ReRead(SwPaM&, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic) override;

    SwDrawFrameFormat* InsertDrawObj( const SwPaM &rRg, SdrObject& rDrawObj, const SfxItemSet& rFlyAttrSet ) override;

    SwFlyFrameFormat* InsertEmbObject(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj, SfxItemSet* pFlyAttrSet) override;

    SwFlyFrameFormat* InsertOLE(const SwPaM &rRg, const OUString& rObjName, sal_Int64 nAspect, const SfxItemSet* pFlyAttrSet,
                           const SfxItemSet* pGrfAttrSet) override;

    bool SplitNode(const SwPosition &rPos, bool bChkTableStart) override;

    bool AppendTextNode(SwPosition& rPos) override;

    bool ReplaceRange(SwPaM& rPam, const OUString& rNewStr,
                              const bool bRegExReplace) override;

    // Add a para for the char attribute exp...
    bool InsertPoolItem(const SwPaM &rRg, const SfxPoolItem&,
                        const SetAttrMode nFlags = SetAttrMode::DEFAULT,
                        SwRootFrame const* pLayout = nullptr,
                        SwTextAttr **ppNewTextAttr = nullptr) override;

    void InsertItemSet (const SwPaM &rRg, const SfxItemSet&,
        const SetAttrMode nFlags = SetAttrMode::DEFAULT,
        SwRootFrame const* pLayout = nullptr) override;

    void RemoveLeadingWhiteSpace(const SwPosition & rPos ) override;
    void RemoveLeadingWhiteSpace(SwPaM& rPaM) override;


    //Non-Interface methods

    void DeleteDummyChar(SwPosition const& rPos, sal_Unicode cDummy);

    void CopyWithFlyInFly( const SwNodeRange& rRg,
                            SwNode& rInsPos,
                            const std::pair<const SwPaM&, const SwPosition&> * pCopiedPaM = nullptr,
                            bool bMakeNewFrames = true,
                            bool bDelRedlines = true,
                            bool bCopyFlyAtFly = false,
                            SwCopyFlags flags = SwCopyFlags::Default) const;
    void CopyFlyInFlyImpl(  const SwNodeRange& rRg,
                            SwPaM const*const pCopiedPaM,
                            SwNode& rStartIdx,
                            const bool bCopyFlyAtFly = false,
                            SwCopyFlags flags = SwCopyFlags::Default,
                            bool bMakeNewFrames = true) const;

    /// Parameters for _Rst and lcl_SetTextFormatColl
    //originallyfrom docfmt.cxx
    struct ParaRstFormat
    {
        SwTextFormatColl* pFormatColl;
        SwHistory* pHistory;
        const SwPosition *pSttNd, *pEndNd;
        const SfxItemSet* pDelSet;
        SwRootFrame const*const pLayout;
        sal_uInt16 nWhich;
        bool bReset;
        bool bResetListAttrs; // #i62575#
        bool bResetAll;
        bool bResetAllCharAttrs;
        bool bInclRefToxMark;
        /// From the attributes included in the range, delete only the ones which have exactly same range. Don't delete the ones which are simply included in the range.
        bool bExactRange;

        ParaRstFormat(const SwPosition* pStart, const SwPosition* pEnd,
                   SwHistory* pHst, const SfxItemSet* pSet = nullptr,
                   SwRootFrame const*const pLay = nullptr)
            : pFormatColl(nullptr)
            , pHistory(pHst)
            , pSttNd(pStart)
            , pEndNd(pEnd)
            , pDelSet(pSet)
            , pLayout(pLay)
            , nWhich(0)
            , bReset(false) // #i62675#
            , bResetListAttrs(false)
            , bResetAll(true)
            , bResetAllCharAttrs(false)
            , bInclRefToxMark(false)
            , bExactRange(false)
        {
        }
    };
    static bool lcl_RstTextAttr( SwNode* pNd, void* pArgs ); //originally from docfmt.cxx
    static std::shared_ptr<SfxItemSet> lcl_createDelSet(SwDoc& rDoc);

    virtual ~DocumentContentOperationsManager() override;

private:
    SwDoc& m_rDoc;

    bool m_bIME = false;

    bool DeleteAndJoinImpl(SwPaM &, SwDeleteFlags);
    bool DeleteAndJoinWithRedlineImpl(SwPaM &, SwDeleteFlags);
    bool DeleteRangeImpl(SwPaM &, SwDeleteFlags);
    bool DeleteRangeImplImpl(SwPaM &, SwDeleteFlags);
    bool ReplaceRangeImpl(SwPaM&, OUString const&, const bool);
    SwFlyFrameFormat* InsNoTextNode( const SwPosition&rPos, SwNoTextNode*,
                                const SfxItemSet* pFlyAttrSet,
                                const SfxItemSet* pGrfAttrSet,
                                SwFrameFormat* );
    /* Copy a range within the same or to another document.
     Position may not lie within range! */
    bool CopyImpl( SwPaM&, SwPosition&,
            SwCopyFlags flags, SwPaM *const pCpyRng /*= 0*/) const;
    bool CopyImplImpl(SwPaM&, SwPosition&,
            SwCopyFlags flags, SwPaM *const pCpyRng /*= 0*/) const;

    DocumentContentOperationsManager(DocumentContentOperationsManager const&) = delete;
    DocumentContentOperationsManager& operator=(DocumentContentOperationsManager const&) = delete;
};


void CopyBookmarks(const SwPaM& rPam, const SwPosition& rTarget,
                   SwCopyFlags flags = SwCopyFlags::Default);

void CalcBreaks(std::vector<std::pair<SwNodeOffset, sal_Int32>> & rBreaks,
        SwPaM const & rPam, bool const isOnlyFieldmarks = false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

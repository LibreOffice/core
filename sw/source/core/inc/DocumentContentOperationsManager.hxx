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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTCONTENTOPERATIONSMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTCONTENTOPERATIONSMANAGER_HXX

#include <IDocumentContentOperations.hxx>
#include <boost/noncopyable.hpp>
#include <ndarr.hxx> //Only for lcl_RstTxtAttr

class SwDoc;
class SwNoTextNode;
class SwFormatColl;
class SwHistory;

namespace sw
{

class DocumentContentOperationsManager : public IDocumentContentOperations,
                                         public ::boost::noncopyable
{
public:
    DocumentContentOperationsManager( SwDoc& i_rSwdoc );

    //Interface methods:
    bool CopyRange(SwPaM&, SwPosition&, const bool bCopyAll, bool bCheckPos ) const override;

    void DeleteSection(SwNode* pNode) override;

    bool DeleteRange(SwPaM&) override;

    bool DelFullPara(SwPaM&) override;

    // Add optional parameter <bForceJoinNext>, default value <false>
    // Needed for hiding of deletion redlines
    bool DeleteAndJoin( SwPaM&,
        const bool bForceJoinNext = false ) override;

    bool MoveRange(SwPaM&, SwPosition&, SwMoveFlags) override;

    bool MoveNodeRange(SwNodeRange&, SwNodeIndex&, SwMoveFlags) override;

    bool MoveAndJoin(SwPaM&, SwPosition&, SwMoveFlags) override;

    bool Overwrite(const SwPaM &rRg, const OUString& rStr) override;

    bool InsertString(const SwPaM &rRg, const OUString&,
              const SwInsertFlags nInsertMode = SwInsertFlags::EMPTYEXPAND ) override;

    void TransliterateText(const SwPaM& rPaM, utl::TransliterationWrapper&) override;

    SwFlyFrameFormat* Insert(const SwPaM &rRg, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet, const SfxItemSet* pGrfAttrSet, SwFrameFormat*) override;

    SwFlyFrameFormat* Insert(const SwPaM& rRg, const GraphicObject& rGrfObj, const SfxItemSet* pFlyAttrSet,
        const SfxItemSet* pGrfAttrSet, SwFrameFormat*) override;

    void ReRead(SwPaM&, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic, const GraphicObject* pGrfObj) override;

    SwDrawFrameFormat* InsertDrawObj( const SwPaM &rRg, SdrObject& rDrawObj, const SfxItemSet& rFlyAttrSet ) override;

    SwFlyFrameFormat* Insert(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj, const SfxItemSet* pFlyAttrSet,
        const SfxItemSet* pGrfAttrSet, SwFrameFormat*) override;

    SwFlyFrameFormat* InsertOLE(const SwPaM &rRg, const OUString& rObjName, sal_Int64 nAspect, const SfxItemSet* pFlyAttrSet,
                           const SfxItemSet* pGrfAttrSet, SwFrameFormat*) override;

    bool SplitNode(const SwPosition &rPos, bool bChkTableStart) override;

    bool AppendTextNode(SwPosition& rPos) override;

    bool ReplaceRange(SwPaM& rPam, const OUString& rNewStr,
                              const bool bRegExReplace) override;

    // Add a para for the char attribute exp...
    bool InsertPoolItem(const SwPaM &rRg, const SfxPoolItem&,
                                const SetAttrMode nFlags = SetAttrMode::DEFAULT, bool bExpandCharToPara=false) override;

    bool InsertItemSet (const SwPaM &rRg, const SfxItemSet&,
        const SetAttrMode nFlags = SetAttrMode::DEFAULT) override;

    void RemoveLeadingWhiteSpace(const SwPosition & rPos ) override;


    //Non-Interface methods

    void CopyWithFlyInFly( const SwNodeRange& rRg,
                            const sal_Int32 nEndContentIndex,
                            const SwNodeIndex& rInsPos,
                            const std::pair<const SwPaM&, const SwPosition&> * pCopiedPaM = nullptr,
                            bool bMakeNewFrames = true,
                            bool bDelRedlines = true,
                            bool bCopyFlyAtFly = false ) const;
    void CopyFlyInFlyImpl(  const SwNodeRange& rRg,
                            const sal_Int32 nEndContentIndex,
                            const SwNodeIndex& rStartIdx,
                            const bool bCopyFlyAtFly = false ) const;

    /// Parameters for _Rst and lcl_SetTextFormatColl
    //originallyfrom docfmt.cxx
    struct ParaRstFormat
    {
        SwFormatColl* pFormatColl;
        SwHistory* pHistory;
        const SwPosition *pSttNd, *pEndNd;
        const SfxItemSet* pDelSet;
        sal_uInt16 nWhich;
        bool bReset;
        bool bResetListAttrs; // #i62575#
        bool bResetAll;
        bool bInclRefToxMark;
        /// From the attributes included in the range, delete only the ones which have exactly same range. Don't delete the ones which are simply included in the range.
        bool bExactRange;

        ParaRstFormat(const SwPosition* pStt, const SwPosition* pEnd,
                   SwHistory* pHst, sal_uInt16 nWhch = 0, const SfxItemSet* pSet = nullptr)
            : pFormatColl(nullptr)
            , pHistory(pHst)
            , pSttNd(pStt)
            , pEndNd(pEnd)
            , pDelSet(pSet)
            , nWhich(nWhch)
            , bReset(false) // #i62675#
            , bResetListAttrs(false)
            , bResetAll(true)
            , bInclRefToxMark(false)
            , bExactRange(false)
        {
        }
    };
    static bool lcl_RstTextAttr( const SwNodePtr& rpNd, void* pArgs ); //originally from docfmt.cxx


    virtual ~DocumentContentOperationsManager();

private:
    SwDoc& m_rDoc;

    bool DeleteAndJoinImpl(SwPaM&, const bool);
    bool DeleteAndJoinWithRedlineImpl(SwPaM&, const bool unused = false);
    bool DeleteRangeImpl(SwPaM&, const bool unused = false);
    bool DeleteRangeImplImpl(SwPaM &);
    bool ReplaceRangeImpl(SwPaM&, OUString const&, const bool);
    SwFlyFrameFormat* _InsNoTextNode( const SwPosition&rPos, SwNoTextNode*,
                                const SfxItemSet* pFlyAttrSet,
                                const SfxItemSet* pGrfAttrSet,
                                SwFrameFormat* = nullptr );
    /* Copy a range within the same or to another document.
     Position may not lie within range! */
    bool CopyImpl( SwPaM&, SwPosition&, const bool MakeNewFrames /*= true */,
            const bool bCopyAll, SwPaM *const pCpyRng /*= 0*/ ) const;
};

}

#endif // INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTCONTENTOPERATIONSMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTREDLINEMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTREDLINEMANAGER_HXX

#include <IDocumentRedlineAccess.hxx>

class SwDoc;

namespace sw
{

class SAL_DLLPUBLIC_RTTI DocumentRedlineManager final : public IDocumentRedlineAccess
{
public:
    DocumentRedlineManager( SwDoc& i_rSwdoc );

    /**
     * Replaced by SwRootFrame::IsHideRedlines() (this is model-level redline
     * hiding) for hide/show.
     */
    virtual RedlineFlags GetRedlineFlags(const SwViewShell* pViewShell = nullptr) const override;

    virtual void SetRedlineFlags_intern(/*[in]*/RedlineFlags eMode, SfxRedlineRecordingMode eRedlineRecordingMode = SfxRedlineRecordingMode::ViewAgnostic, bool bRecordModeChange = false) override;

    virtual void SetRedlineFlags(/*[in]*/RedlineFlags eMode, SfxRedlineRecordingMode eRedlineRecordingMode = SfxRedlineRecordingMode::ViewAgnostic, bool bRecordModeChange = false) override;

    virtual bool IsRedlineOn() const override;

    virtual bool IsIgnoreRedline() const override;

    virtual const SwRedlineTable& GetRedlineTable() const override;
    virtual SwRedlineTable& GetRedlineTable() override;
    virtual const SwExtraRedlineTable& GetExtraRedlineTable() const override;
    virtual SwExtraRedlineTable& GetExtraRedlineTable() override;

    virtual bool IsInRedlines(const SwNode& rNode) const override;

    virtual AppendResult AppendRedline(
        /*[in]*/ SwRangeRedline* pPtr,
        /*[in]*/ bool bCallDelete,
        /*[in]*/ sal_uInt32 nMoveIDToDelete = 0) override;

    virtual bool AppendTableRowRedline(/*[in]*/SwTableRowRedline* pPtr) override;
    virtual bool AppendTableCellRedline(/*[in]*/SwTableCellRedline* pPtr) override;

    virtual bool SplitRedline(/*[in]*/const SwPaM& rPam) override;

    virtual bool DeleteRedline(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/RedlineType nDelType) override;

    virtual bool DeleteRedline(
        /*[in]*/const SwStartNode& rSection,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/RedlineType nDelType) override;

    virtual SwRedlineTable::size_type GetRedlinePos(
        /*[in]*/const SwNode& rNode,
        /*[in]*/RedlineType nType) const override;

    virtual SwRedlineTable::size_type GetRedlineEndPos(
        /*[in]*/ SwRedlineTable::size_type nStartPos,
        /*[in]*/ const SwNode& rNode,
        /*[in]*/ RedlineType nType) const override;

    virtual bool HasRedline(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/RedlineType nType,
        /*[in]*/bool bStartOrEndInRange) const override;

    virtual void CompressRedlines(size_t nStartIdx = 0) override;

    virtual const SwRangeRedline* GetRedline(
        /*[in]*/const SwPosition& rPos,
        /*[in]*/SwRedlineTable::size_type* pFndPos) const override;

    virtual bool IsRedlineMove() const override;

    virtual void SetRedlineMove(/*[in]*/bool bFlag) override;

    virtual bool AcceptRedline(/*[in]*/ SwRedlineTable::size_type nPos, /*[in]*/ bool bCallDelete,
                               /*[in]*/ bool bRange = false) override;

    virtual bool AcceptRedline(/*[in]*/ const SwPaM& rPam, /*[in]*/ bool bCallDelete,
                               /*[in]*/ sal_Int8 nDepth = 0) override;

    virtual void AcceptRedlineParagraphFormatting(/*[in]*/const SwPaM& rPam) override;

    virtual bool RejectRedline(/*[in]*/ SwRedlineTable::size_type nPos, /*[in]*/ bool bCallDelete,
                               /*[in]*/ bool bRange = false) override;

    virtual bool RejectRedline(/*[in]*/ const SwPaM& rPam, /*[in]*/ bool bCallDelete,
                               /*[in]*/ sal_Int8 nDepth = 0) override;

    virtual void AcceptAllRedline(/*[in]*/bool bAcceptReject) override;

    virtual const SwRangeRedline* SelNextRedline(/*[in]*/SwPaM& rPam) const override;

    virtual const SwRangeRedline* SelPrevRedline(/*[in]*/SwPaM& rPam) const override;

    virtual void UpdateRedlineAttr() override;

    virtual std::size_t GetRedlineAuthor() override;

    virtual std::size_t InsertRedlineAuthor(const OUString& rAuthor) override;

    virtual bool SetRedlineComment(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/const OUString& rComment) override;

    virtual const css::uno::Sequence <sal_Int8>& GetRedlinePassword() const override;

    virtual void SetRedlinePassword(
        /*[in]*/const css::uno::Sequence <sal_Int8>& rNewPassword) override;

    // After nodes are removed, m_pContentNode's may not updated
    virtual void UpdateRedlineContentNode(
        /*[in]*/ SwRedlineTable::size_type nStartPos,
        /*[in]*/ SwRedlineTable::size_type nEndPos) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    //Non Interface methods;

    /** Set comment-text for Redline. It then comes in via AppendRedLine.
     Used by AutoFormat. 0-pointer resets mode.
     Sequence number is for conjoining of Redlines by the UI. */
    void SetAutoFormatRedlineComment( const OUString* pText, sal_uInt16 nSeqNo = 0 );

    bool IsHideRedlines() const { return m_bHideRedlines; }
    void SetHideRedlines(bool const bHideRedlines) { m_bHideRedlines = bHideRedlines; }

    void HideAll(bool bDeletion);
    void ShowAll();

    virtual ~DocumentRedlineManager() override;

private:

    bool RejectRedlineRange(SwRedlineTable::size_type nPosOrigin,
                            const SwRedlineTable::size_type& nPosStart,
                            const SwRedlineTable::size_type& nPosEnd, bool bCallDelete);
    bool AcceptRedlineRange(SwRedlineTable::size_type nPosOrigin,
                            const SwRedlineTable::size_type& nPosStart,
                            const SwRedlineTable::size_type& nPosEnd, bool bCallDelete);
    bool AcceptMovedRedlines(sal_uInt32 nMovedID, bool bCallDelete);
    bool RejectMovedRedlines(sal_uInt32 nMovedID, bool bCallDelete);

    DocumentRedlineManager(DocumentRedlineManager const&) = delete;
    DocumentRedlineManager& operator=(DocumentRedlineManager const&) = delete;

    SwDoc& m_rDoc;

    RedlineFlags meRedlineFlags;     //< Current Redline Mode.
    SwRedlineTable maRedlineTable;           //< List of all Ranged Redlines.
    SwExtraRedlineTable maExtraRedlineTable;      //< List of all Extra Redlines.
    std::optional<OUString> moAutoFormatRedlnComment;  //< Comment for Redlines inserted via AutoFormat.
    bool mbIsRedlineMove;    //< true: Redlines are moved into to / out of the section.
    sal_uInt16 mnAutoFormatRedlnCommentNo;  /**< SeqNo for conjoining of AutoFormat-Redlines.
                                         by the UI. Managed by SwAutoFormat! */
    css::uno::Sequence <sal_Int8 > maRedlinePasswd;

    /// this flag is necessary for file import because the ViewShell/layout is
    /// created "too late" and the ShowRedlineChanges item is not below "Views"
    bool m_bHideRedlines = false;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

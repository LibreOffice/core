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

class DocumentRedlineManager : public IDocumentRedlineAccess
{
public:
    DocumentRedlineManager( SwDoc& i_rSwdoc );

    virtual RedlineFlags GetRedlineFlags() const override;

    virtual void SetRedlineFlags_intern(/*[in]*/RedlineFlags eMode) override;

    virtual void SetRedlineFlags(/*[in]*/RedlineFlags eMode) override;

    virtual bool IsRedlineOn() const override;

    virtual bool IsIgnoreRedline() const override;

    virtual const SwRedlineTable& GetRedlineTable() const override;
    virtual SwRedlineTable& GetRedlineTable() override;
    virtual const SwExtraRedlineTable& GetExtraRedlineTable() const override;
    virtual SwExtraRedlineTable& GetExtraRedlineTable() override;
    virtual bool HasExtraRedlineTable() const override;

    virtual bool IsInRedlines(const SwNode& rNode) const override;

    virtual bool AppendRedline(/*[in]*/SwRangeRedline* pPtr, /*[in]*/bool bCallDelete) override;

    virtual bool AppendTableRowRedline(/*[in]*/SwTableRowRedline* pPtr, /*[in]*/bool bCallDelete) override;
    virtual bool AppendTableCellRedline(/*[in]*/SwTableCellRedline* pPtr, /*[in]*/bool bCallDelete) override;

    virtual bool SplitRedline(/*[in]*/const SwPaM& rPam) override;

    virtual bool DeleteRedline(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/sal_uInt16 nDelType) override;

    virtual bool DeleteRedline(
        /*[in]*/const SwStartNode& rSection,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/sal_uInt16 nDelType) override;

    virtual SwRedlineTable::size_type GetRedlinePos(
        /*[in]*/const SwNode& rNode,
        /*[in]*/sal_uInt16 nType) const override;

    virtual void CompressRedlines() override;

    virtual const SwRangeRedline* GetRedline(
        /*[in]*/const SwPosition& rPos,
        /*[in]*/SwRedlineTable::size_type* pFndPos) const override;

    virtual bool IsRedlineMove() const override;

    virtual void SetRedlineMove(/*[in]*/bool bFlag) override;

    virtual bool AcceptRedline(/*[in]*/SwRedlineTable::size_type nPos, /*[in]*/bool bCallDelete) override;

    virtual bool AcceptRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) override;

    virtual bool RejectRedline(/*[in]*/SwRedlineTable::size_type nPos, /*[in]*/bool bCallDelete) override;

    virtual bool RejectRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) override;

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


    //Non Interface methods;

    /** Set comment-text for Redline. It then comes in via AppendRedLine.
     Used by AutoFormat. 0-pointer resets mode.
     Sequence number is for conjoining of Redlines by the UI. */
    void SetAutoFormatRedlineComment( const OUString* pText, sal_uInt16 nSeqNo = 0 );

    void checkRedlining(RedlineFlags& _rReadlineMode);


    virtual ~DocumentRedlineManager() override;

private:

    DocumentRedlineManager(DocumentRedlineManager const&) = delete;
    DocumentRedlineManager& operator=(DocumentRedlineManager const&) = delete;

    SwDoc& m_rDoc;

    RedlineFlags meRedlineFlags;     //< Current Redline Mode.
    SwRedlineTable        *mpRedlineTable;           //< List of all Ranged Redlines.
    SwExtraRedlineTable   *mpExtraRedlineTable;      //< List of all Extra Redlines.
    OUString            *mpAutoFormatRedlnComment;  //< Comment for Redlines inserted via AutoFormat.
    bool mbIsRedlineMove;    //< true: Redlines are moved into to / out of the section.
    bool mbReadlineChecked;    //< true: if the query was already shown
    sal_uInt16 mnAutoFormatRedlnCommentNo;  /**< SeqNo for conjoining of AutoFormat-Redlines.
                                         by the UI. Managed by SwAutoFormat! */
    css::uno::Sequence <sal_Int8 > maRedlinePasswd;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

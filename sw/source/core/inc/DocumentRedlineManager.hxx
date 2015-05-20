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
#include <boost/utility.hpp>

class SwDoc;

namespace sw
{

class DocumentRedlineManager : public IDocumentRedlineAccess,
                               public ::boost::noncopyable
{
public:
    DocumentRedlineManager( SwDoc& i_rSwdoc );

    virtual RedlineMode_t GetRedlineMode() const SAL_OVERRIDE;

    virtual void SetRedlineMode_intern(/*[in]*/RedlineMode_t eMode) SAL_OVERRIDE;

    virtual void SetRedlineMode(/*[in]*/RedlineMode_t eMode) SAL_OVERRIDE;

    virtual bool IsRedlineOn() const SAL_OVERRIDE;

    virtual bool IsIgnoreRedline() const SAL_OVERRIDE;

    virtual const SwRedlineTable& GetRedlineTable() const SAL_OVERRIDE;
    virtual SwRedlineTable& GetRedlineTable() SAL_OVERRIDE;
    virtual const SwExtraRedlineTable& GetExtraRedlineTable() const SAL_OVERRIDE;
    virtual SwExtraRedlineTable& GetExtraRedlineTable() SAL_OVERRIDE;
    virtual bool HasExtraRedlineTable() const SAL_OVERRIDE;

    virtual bool IsInRedlines(const SwNode& rNode) const SAL_OVERRIDE;

    virtual bool AppendRedline(/*[in]*/SwRangeRedline* pPtr, /*[in]*/bool bCallDelete) SAL_OVERRIDE;

    virtual bool AppendTableRowRedline(/*[in]*/SwTableRowRedline* pPtr, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual bool AppendTableCellRedline(/*[in]*/SwTableCellRedline* pPtr, /*[in]*/bool bCallDelete) SAL_OVERRIDE;

    virtual bool SplitRedline(/*[in]*/const SwPaM& rPam) SAL_OVERRIDE;

    virtual bool DeleteRedline(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/sal_uInt16 nDelType) SAL_OVERRIDE;

    virtual bool DeleteRedline(
        /*[in]*/const SwStartNode& rSection,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/sal_uInt16 nDelType) SAL_OVERRIDE;

    virtual sal_uInt16 GetRedlinePos(
        /*[in]*/const SwNode& rNode,
        /*[in]*/sal_uInt16 nType) const SAL_OVERRIDE;

    virtual void CompressRedlines() SAL_OVERRIDE;

    virtual const SwRangeRedline* GetRedline(
        /*[in]*/const SwPosition& rPos,
        /*[in]*/sal_uInt16* pFndPos) const SAL_OVERRIDE;

    virtual bool IsRedlineMove() const SAL_OVERRIDE;

    virtual void SetRedlineMove(/*[in]*/bool bFlag) SAL_OVERRIDE;

    virtual bool AcceptRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete) SAL_OVERRIDE;

    virtual bool AcceptRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) SAL_OVERRIDE;

    virtual bool RejectRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete) SAL_OVERRIDE;

    virtual bool RejectRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) SAL_OVERRIDE;

    virtual const SwRangeRedline* SelNextRedline(/*[in]*/SwPaM& rPam) const SAL_OVERRIDE;

    virtual const SwRangeRedline* SelPrevRedline(/*[in]*/SwPaM& rPam) const SAL_OVERRIDE;

    virtual void UpdateRedlineAttr() SAL_OVERRIDE;

    virtual sal_uInt16 GetRedlineAuthor() SAL_OVERRIDE;

    virtual sal_uInt16 InsertRedlineAuthor(const OUString& rAuthor) SAL_OVERRIDE;

    virtual bool SetRedlineComment(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/const OUString& rComment) SAL_OVERRIDE;

    virtual const ::com::sun::star::uno::Sequence <sal_Int8>& GetRedlinePassword() const SAL_OVERRIDE;

    virtual void SetRedlinePassword(
        /*[in]*/const ::com::sun::star::uno::Sequence <sal_Int8>& rNewPassword) SAL_OVERRIDE;


    //Non Interface methods;

    /** Set comment-text for Redline. It then comes in via AppendRedLine.
     Used by AutoFormat. 0-pointer resets mode.
     Sequence number is for conjoining of Redlines by the UI. */
    void SetAutoFormatRedlineComment( const OUString* pText, sal_uInt16 nSeqNo = 0 );

    void checkRedlining(RedlineMode_t& _rReadlineMode);


    virtual ~DocumentRedlineManager();

private:
    SwDoc& m_rDoc;

    RedlineMode_t meRedlineMode;     //< Current Redline Mode.
    SwRedlineTable        *mpRedlineTable;           //< List of all Ranged Redlines.
    SwExtraRedlineTable   *mpExtraRedlineTable;      //< List of all Extra Redlines.
    OUString            *mpAutoFormatRedlnComment;  //< Comment for Redlines inserted via AutoFormat.
    bool mbIsRedlineMove;    //< true: Redlines are moved into to / out of the section.
    bool mbReadlineChecked;    //< true: if the query was already shown
    sal_uInt16 mnAutoFormatRedlnCommentNo;  /**< SeqNo for conjoining of AutoFormat-Redlines.
                                         by the UI. Managed by SwAutoFormat! */
    ::com::sun::star::uno::Sequence <sal_Int8 > maRedlinePasswd;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

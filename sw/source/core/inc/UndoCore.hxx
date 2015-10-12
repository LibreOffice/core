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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOCORE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOCORE_HXX

#include <undobj.hxx>
#include <calbck.hxx>
#include <rtl/ustring.hxx>
#include <redline.hxx>

#include <memory>
#include <vector>

class SfxItemSet;
class SwFormatColl;
class SwFormatAnchor;
class SdrMarkList;
class SwUndoDelete;
class SwFormat;

namespace sw {
    class UndoManager;
    class IShellCursorSupplier;
}

class SwRedlineSaveData: public SwUndRng, public SwRedlineData, private SwUndoSaveSection
{
public:
    SwRedlineSaveData(
        SwComparePosition eCmpPos,
        const SwPosition& rSttPos,
        const SwPosition& rEndPos,
        SwRangeRedline& rRedl,
        bool bCopyNext );

    ~SwRedlineSaveData();

    void RedlineToDoc( SwPaM& rPam );

    SwNodeIndex* GetMvSttIdx() const
    {
        return SwUndoSaveSection::GetMvSttIdx();
    }

#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 nRedlineCount;
#endif
};

class SwRedlineSaveDatas {
private:
    std::vector<std::unique_ptr<SwRedlineSaveData>> m_Data;

public:
    SwRedlineSaveDatas() : m_Data() {}

    void clear() { m_Data.clear(); }
    bool empty() const { return m_Data.empty(); }
    size_t size() const { return m_Data.size(); }
    void push_back(std::unique_ptr<SwRedlineSaveData> pNew) { m_Data.push_back(std::move(pNew)); }
    const SwRedlineSaveData& operator[](size_t const nIdx) const { return *m_Data[ nIdx ]; }
    SwRedlineSaveData& operator[](size_t const nIdx) { return *m_Data[ nIdx ]; }
};

namespace sw {
class UndoRedoContext
    : public SfxUndoContext
{
public:
    UndoRedoContext(SwDoc & rDoc, IShellCursorSupplier & rCursorSupplier)
        : m_rDoc(rDoc)
        , m_rCursorSupplier(rCursorSupplier)
        , m_pSelFormat(0)
        , m_pMarkList(0)
    { }

    SwDoc & GetDoc() const { return m_rDoc; }

    IShellCursorSupplier & GetCursorSupplier() { return m_rCursorSupplier; }

    void SetSelections(SwFrameFormat *const pSelFormat, SdrMarkList *const pMarkList)
    {
        m_pSelFormat = pSelFormat;
        m_pMarkList = pMarkList;
    }
    void GetSelections(SwFrameFormat *& o_rpSelFormat, SdrMarkList *& o_rpMarkList)
    {
        o_rpSelFormat = m_pSelFormat;
        o_rpMarkList = m_pMarkList;
    }

private:
    SwDoc & m_rDoc;
    IShellCursorSupplier & m_rCursorSupplier;
    SwFrameFormat * m_pSelFormat;
    SdrMarkList * m_pMarkList;
};

class RepeatContext
    : public SfxRepeatTarget
{
public:
    RepeatContext(SwDoc & rDoc, SwPaM & rPaM)
        : m_rDoc(rDoc)
        , m_pCurrentPaM(& rPaM)
        , m_bDeleteRepeated(false)
    { }

    SwDoc & GetDoc() const { return m_rDoc; }

    SwPaM & GetRepeatPaM()
    {
        return *m_pCurrentPaM;
    }

private:
    friend class ::sw::UndoManager;
    friend class ::SwUndoDelete;

    SwDoc & m_rDoc;
    SwPaM * m_pCurrentPaM;
    bool m_bDeleteRepeated; /// has a delete action been repeated?
};

} // namespace sw

class SwUndoFormatColl : public SwUndo, private SwUndRng
{
    OUString aFormatName;
    SwHistory* pHistory;
    SwFormatColl* pFormatColl;
    // for correct <ReDo(..)> and <Repeat(..)>
    // boolean, which indicates that the attributes are reseted at the nodes
    // before the format has been applied.
    const bool mbReset;
    // boolean, which indicates that the list attributes had been reseted at
    // the nodes before the format has been applied.
    const bool mbResetListAttrs;

    void DoSetFormatColl(SwDoc & rDoc, SwPaM & rPaM);

public:
    SwUndoFormatColl( const SwPaM&, SwFormatColl*,
                   const bool bReset,
                   const bool bResetListAttrs );
    virtual ~SwUndoFormatColl();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    /**
       Returns the rewriter for this undo object.

       The rewriter contains one rule:

           $1 -> <name of format collection>

       <name of format collection> is the name of the format
       collection that is applied by the action recorded by this undo
       object.

       @return the rewriter for this undo object
    */
    virtual SwRewriter GetRewriter() const override;

    SwHistory* GetHistory() { return pHistory; }

};

class SwUndoSetFlyFormat : public SwUndo, public SwClient
{
    SwFrameFormat* pFrameFormat;                  // saved FlyFormat
    SwFrameFormat* pOldFormat;
    SwFrameFormat* pNewFormat;
    SfxItemSet* pItemSet;               // the re-/ set attributes
    sal_uLong nOldNode, nNewNode;
    sal_Int32 nOldContent, nNewContent;
    sal_uInt16 nOldAnchorTyp, nNewAnchorTyp;
    bool bAnchorChgd;

    void PutAttr( sal_uInt16 nWhich, const SfxPoolItem* pItem );
    void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
    void GetAnchor( SwFormatAnchor& rAnhor, sal_uLong nNode, sal_Int32 nContent );

public:
    SwUndoSetFlyFormat( SwFrameFormat& rFlyFormat, SwFrameFormat& rNewFrameFormat );
    virtual ~SwUndoSetFlyFormat();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
    void DeRegisterFromFormat( SwFormat& );
};

class SwUndoOutlineLeftRight : public SwUndo, private SwUndRng
{
    short nOffset;

public:
    SwUndoOutlineLeftRight( const SwPaM& rPam, short nOffset );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

const int nUndoStringLength = 20;

/**
   Shortens a string to a maximum length.

   @param rStr      the string to be shortened
   @param nLength   the maximum length for rStr
   @param rFillStr  string to replace cut out characters with

   If rStr has less than nLength characters it will be returned unaltered.

   If rStr has more than nLength characters the following algorithm
   generates the shortened string:

       frontLength = (nLength - length(rFillStr)) / 2
       rearLength = nLength - length(rFillStr) - frontLength
       shortenedString = concat(<first frontLength characters of rStr,
                                rFillStr,
                                <last rearLength characters of rStr>)

   Preconditions:
      - nLength - length(rFillStr) >= 2

   @return the shortened string
 */
OUString
ShortenString(const OUString & rStr, sal_Int32 nLength, const OUString & rFillStr);
/**
   Denotes special characters in a string.

   The rStr is split into parts containing special characters and
   parts not containing special characters. In a part containing
   special characters all characters are equal. These parts are
   maximal.

   @param rStr     the string to denote in

   The resulting string is generated by concatenating the found
   parts. The parts without special characters are surrounded by
   "'". The parts containing special characters are denoted as "n x",
   where n is the length of the part and x is the representation of
   the special character (i. e. "tab(s)").

   @return the denoted string
*/
OUString DenoteSpecialCharacters(const OUString & rStr);

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOCORE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

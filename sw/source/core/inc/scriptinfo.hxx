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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_SCRIPTINFO_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SCRIPTINFO_HXX

#include <vector>
#include <deque>
#include <unordered_set>
#include <swscanner.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include "TextFrameIndex.hxx"

class SwTextNode;
class SwTextFrame;
class Point;
class MultiSelection;
enum class SwFontScript;
namespace sw { struct MergedPara; }

#define SPACING_PRECISION_FACTOR 100

// encapsulates information about script changes
class SwScriptInfo
{
public:
    enum CompType { KANA, SPECIAL_LEFT, SPECIAL_RIGHT, NONE, SPECIAL_MIDDLE};

private:
    //! Records a single change in script type.
    struct ScriptChangeInfo
    {
        TextFrameIndex position; //!< Character position at which we change script
        sal_uInt8       type;     //!< Script type (Latin/Asian/Complex) that we change to.
        ScriptChangeInfo(TextFrameIndex pos, sal_uInt8 typ) : position(pos), type(typ) {};
    };
    //TODO - This is sorted, so should probably be a std::set rather than vector.
    //       But we also use random access (probably unnecessarily).
    std::vector<ScriptChangeInfo> m_ScriptChanges;
    //! Records a single change in direction.
    struct DirectionChangeInfo
    {
        TextFrameIndex position; //!< Character position at which we change direction.
        sal_uInt8       type;     //!< Direction that we change to.
        DirectionChangeInfo(TextFrameIndex pos, sal_uInt8 typ) : position(pos), type(typ) {};
    };
    std::vector<DirectionChangeInfo> m_DirectionChanges;
    std::deque<TextFrameIndex> m_Kashida;
    /// indexes into m_Kashida
    std::unordered_set<size_t> m_KashidaInvalid;
    std::deque<TextFrameIndex> m_NoKashidaLine;
    std::deque<TextFrameIndex> m_NoKashidaLineEnd;
    std::vector<TextFrameIndex> m_HiddenChg;
    //! Records a single change in compression.
    struct CompressionChangeInfo
    {
        TextFrameIndex position; //!< Character position where the change occurs.
        TextFrameIndex length;   //!< Length of the segment.
        CompType  type;     //!< Type of compression that we change to.
        CompressionChangeInfo(TextFrameIndex pos, TextFrameIndex len, CompType typ) : position(pos), length(len), type(typ) {};
    };
    std::vector<CompressionChangeInfo> m_CompressionChanges;
#ifdef DBG_UTIL
    CompType DbgCompType(const TextFrameIndex nPos) const;
#endif

    TextFrameIndex m_nInvalidityPos;
    sal_uInt8 m_nDefaultDir;

    void UpdateBidiInfo( const OUString& rText );
    bool IsKashidaValid(size_t nKashPos) const;
    // returns true if nKashPos is newly marked invalid
    bool MarkKashidaInvalid(size_t nKashPos);
    void ClearKashidaInvalid(size_t nKashPos);
    bool MarkOrClearKashidaInvalid(TextFrameIndex nStt, TextFrameIndex nLen,
            bool bMark, sal_Int32 nMarkCount);
    bool IsKashidaLine(TextFrameIndex nCharIdx) const;
    // examines the range [ nStart, nStart + nEnd ] if there are kanas
    // returns start index of kana entry in array, otherwise SAL_MAX_SIZE
    size_t HasKana(TextFrameIndex nStart, TextFrameIndex nEnd) const;

public:

    SwScriptInfo();
    ~SwScriptInfo();

    // determines script changes
    void InitScriptInfo(const SwTextNode& rNode, sw::MergedPara const* pMerged, bool bRTL);
    void InitScriptInfo(const SwTextNode& rNode, sw::MergedPara const* pMerged);

    // set/get position from which data is invalid
    void SetInvalidityA(const TextFrameIndex nPos)
    {
        if (nPos < m_nInvalidityPos)
            m_nInvalidityPos = nPos;
    }
    TextFrameIndex GetInvalidityA() const
    {
        return m_nInvalidityPos;
    }

    // get default direction for paragraph
    sal_uInt8 GetDefaultDir() const { return m_nDefaultDir; };

    // array operations, nCnt refers to array position
    size_t CountScriptChg() const { return m_ScriptChanges.size(); }
    TextFrameIndex GetScriptChg(const size_t nCnt) const
    {
        assert(nCnt < m_ScriptChanges.size());
        return m_ScriptChanges[nCnt].position;
    }
    sal_uInt8 GetScriptType( const size_t nCnt ) const
    {
        assert( nCnt < m_ScriptChanges.size());
        return m_ScriptChanges[nCnt].type;
    }

    size_t CountDirChg() const { return m_DirectionChanges.size(); }
    TextFrameIndex GetDirChg(const size_t nCnt) const
    {
        assert(nCnt < m_DirectionChanges.size());
        return m_DirectionChanges[ nCnt ].position;
    }
    sal_uInt8 GetDirType( const size_t nCnt ) const
    {
        assert(nCnt < m_DirectionChanges.size());
        return m_DirectionChanges[ nCnt ].type;
    }

    size_t CountKashida() const
    {
        return m_Kashida.size();
    }

    TextFrameIndex GetKashida(const size_t nCnt) const
    {
        assert(nCnt < m_Kashida.size());
        return m_Kashida[nCnt];
    }

    size_t CountCompChg() const { return m_CompressionChanges.size(); };
    TextFrameIndex GetCompStart(const size_t nCnt) const
    {
        assert(nCnt < m_CompressionChanges.size());
        return m_CompressionChanges[ nCnt ].position;
    }
    TextFrameIndex GetCompLen(const size_t nCnt) const
    {
        assert(nCnt < m_CompressionChanges.size());
        return m_CompressionChanges[ nCnt ].length;
    }
    CompType GetCompType( const size_t nCnt ) const
    {
        assert(nCnt < m_CompressionChanges.size());
        return m_CompressionChanges[ nCnt ].type;
    }

    size_t CountHiddenChg() const { return m_HiddenChg.size(); };
    TextFrameIndex GetHiddenChg(const size_t nCnt) const
    {
        assert(nCnt < m_HiddenChg.size());
        return m_HiddenChg[ nCnt ];
    }
    TextFrameIndex NextHiddenChg(TextFrameIndex nPos) const;
    static void CalcHiddenRanges(const SwTextNode& rNode, MultiSelection& rHiddenMulti);
    static void selectHiddenTextProperty(const SwTextNode& rNode, MultiSelection &rHiddenMulti);
    static void selectRedLineDeleted(const SwTextNode& rNode, MultiSelection &rHiddenMulti, bool bSelect=true);

    // "high" level operations, nPos refers to string position
    TextFrameIndex NextScriptChg(TextFrameIndex nPos) const;
    sal_Int16 ScriptType(const TextFrameIndex nPos) const;

    // Returns the position of the next direction level change.
    // If bLevel is set, the position of the next level which is smaller
    // than the level at position nPos is returned. This is required to
    // obtain the end of a SwBidiPortion
    TextFrameIndex NextDirChg(const TextFrameIndex nPos,
                           const sal_uInt8* pLevel = nullptr) const;
    sal_uInt8 DirType(const TextFrameIndex nPos) const;

    // HIDDEN TEXT STUFF START

/** Hidden text range information - static and non-version

    @descr  Determines if a given position is inside a hidden text range. The
            static version tries to obtain a valid SwScriptInfo object
            via the SwTextNode, otherwise it calculates the values from scratch.
            The non-static version uses the internally cached informatio
            for the calculation.

    @param  rNode
                The text node.
    @param  nPos
                The given position that should be checked.
    @param  rnStartPos
                Return parameter for the start position of the hidden range.
                COMPLETE_STRING if nPos is not inside a hidden range.
    @param  rnEndPos
                Return parameter for the end position of the hidden range.
                0 if nPos is not inside a hidden range.
    @param  rnEndPos
                Return parameter that contains all the hidden text ranges. Optional.
    @return
            returns true if there are any hidden characters in this paragraph.

*/
    static bool GetBoundsOfHiddenRange( const SwTextNode& rNode, sal_Int32 nPos,
                                        sal_Int32& rnStartPos, sal_Int32& rnEndPos,
                                        std::vector<sal_Int32>* pList = nullptr );
    bool GetBoundsOfHiddenRange(TextFrameIndex nPos, TextFrameIndex & rnStartPos,
                                TextFrameIndex & rnEndPos) const;

    static bool IsInHiddenRange( const SwTextNode& rNode, sal_Int32 nPos );

/** Hidden text attribute handling

    @descr  Takes a string and either deletes the hidden ranges or sets
            a given character in place of the hidden characters.

    @param  rNode
                The text node.
    @param  rText
                The string to modify.
    @param  cChar
                The character that should replace the hidden characters.
    @param  bDel
                If set, the hidden ranges will be deleted from the text node.
 */
    static sal_Int32 MaskHiddenRanges(
            const SwTextNode& rNode, OUStringBuffer& rText,
                                    const sal_Int32 nStt, const sal_Int32 nEnd,
                                    const sal_Unicode cChar );

/** Hidden text attribute handling

    @descr  Takes a SwTextNode and deletes the hidden ranges from the node.

    @param  rNode
                The text node.
 */
    static void DeleteHiddenRanges( SwTextNode& rNode );

    // HIDDEN TEXT STUFF END

    // modifies the kerning array according to a given compress value
    long Compress( long* pKernArray, TextFrameIndex nIdx, TextFrameIndex nLen,
                   const sal_uInt16 nCompress, const sal_uInt16 nFontHeight,
                   const bool bCentered,
                   Point* pPoint = nullptr ) const;

/** Performs a kashida justification on the kerning array

    @descr  Add some extra space for kashida justification to the
            positions in the kerning array.
    @param  pKernArray
                The printers kerning array. Optional.
    @param  pScrArray
                The screen kerning array. Optional.
    @param  nStt
                Start referring to the paragraph.
    @param  nLen
                The number of characters to be considered.
    @param  nSpaceAdd
                The value which has to be added to a kashida opportunity.
    @return The number of kashida opportunities in the given range
*/
    sal_Int32 KashidaJustify( long* pKernArray, long* pScrArray,
          TextFrameIndex nStt, TextFrameIndex nLen, long nSpaceAdd = 0) const;

/** Clears array of kashidas marked as invalid
 */
    void ClearKashidaInvalid(TextFrameIndex const nStt, TextFrameIndex const nLen)
    {
        MarkOrClearKashidaInvalid(nStt, nLen, false, 0);
    }

/** Marks nCnt kashida positions as invalid
   pKashidaPositions: array of char indices relative to the paragraph
*/
    void MarkKashidasInvalid(sal_Int32 nCnt, const TextFrameIndex* pKashidaPositions);

/** Marks nCnt kashida positions as invalid
    in the given text range
 */
    bool MarkKashidasInvalid(sal_Int32 const nCnt,
            TextFrameIndex const nStt, TextFrameIndex const nLen)
    {
        return MarkOrClearKashidaInvalid(nStt, nLen, true, nCnt);
    }

/** retrieves kashida opportunities for a given text range.

   rKashidaPositions: buffer to receive the char indices of the
                      kashida opportunities relative to the paragraph
*/
    void GetKashidaPositions(TextFrameIndex nStt, TextFrameIndex nLen,
                             std::vector<TextFrameIndex>& rKashidaPosition);

/** Use regular blank justification instead of kashdida justification for the given line of text.
   nStt Start char index of the line referring to the paragraph.
   nLen Number of characters in the line
*/
    void SetNoKashidaLine(TextFrameIndex nStt, TextFrameIndex nLen);

/** Clear forced blank justification for a given line.
   nStt Start char index of the line referring to the paragraph.
   nLen Number of characters in the line
*/
    void ClearNoKashidaLine(TextFrameIndex nStt, TextFrameIndex nLen);

/** Checks if text is Arabic text.

     @descr  Checks if text is Arabic text.
     @param  rText
                 The text to check
     @param  nStt
                 Start index of the text
     @return Returns if the language is an Arabic language
 */
    static bool IsArabicText(const OUString& rText, TextFrameIndex nStt, TextFrameIndex nLen);

/** Performs a thai justification on the kerning array

    @descr  Add some extra space for thai justification to the
            positions in the kerning array.
    @param  rText
                The String
    @param  pKernArray
                The printers kerning array. Optional.
    @param  pScrArray
                The screen kerning array. Optional.
    @param  nIdx
                Start referring to the paragraph.
    @param  nLen
                The number of characters to be considered.
    @param  nSpaceAdd
                The value which has to be added to the cells.
    @return The number of extra spaces in the given range
*/
    static TextFrameIndex ThaiJustify( const OUString& rText, long* pKernArray,
                                  long* pScrArray, TextFrameIndex nIdx,
                                  TextFrameIndex nLen,
                                  TextFrameIndex nNumberOfBlanks = TextFrameIndex(0),
                                  long nSpaceAdd = 0 );

    static TextFrameIndex CountCJKCharacters(const OUString &rText,
            TextFrameIndex nPos, TextFrameIndex nEnd, LanguageType aLang);

    static void CJKJustify( const OUString& rText, long* pKernArray,
                                  long* pScrArray, TextFrameIndex nStt,
                                  TextFrameIndex nLen, LanguageType aLang,
                                  long nSpaceAdd, bool bIsSpaceStop );

    /// return a frame for the node, ScriptInfo is its member...
    /// (many clients need both frame and SI, and both have to match)
    static SwScriptInfo* GetScriptInfo( const SwTextNode& rNode,
                                        SwTextFrame const** o_pFrame = nullptr,
                                        bool bAllowInvalid = false);

    SwFontScript WhichFont(TextFrameIndex nIdx) const;
    static SwFontScript WhichFont(sal_Int32 nIdx, OUString const & rText);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

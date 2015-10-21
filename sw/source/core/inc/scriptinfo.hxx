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

#include <list>
#include <deque>
#include "swscanner.hxx"
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

class SwTextNode;
class Point;
class MultiSelection;
typedef std::list< sal_Int32 > PositionList;

#define SPACING_PRECISION_FACTOR 100

// encapsultes information about script changes
class SwScriptInfo
{
public:
    enum CompType { KANA, SPECIAL_LEFT, SPECIAL_RIGHT, NONE, SPECIAL_MIDDLE};

private:
    //! Records a single change in script type.
    struct ScriptChangeInfo
    {
        sal_Int32 position; //!< Character position at which we change script
        sal_uInt8       type;     //!< Script type (Latin/Asian/Complex) that we change to.
        inline ScriptChangeInfo(sal_Int32 pos, sal_uInt8 typ) : position(pos), type(typ) {};
    };
    //TODO - This is sorted, so should probably be a std::set rather than vector.
    //       But we also use random access (probably unnecessarily).
    std::vector<ScriptChangeInfo> aScriptChanges;
    //! Records a single change in direction.
    struct DirectionChangeInfo
    {
        sal_Int32 position; //!< Character position at which we change direction.
        sal_uInt8       type;     //!< Direction that we change to.
        inline DirectionChangeInfo(sal_Int32 pos, sal_uInt8 typ) : position(pos), type(typ) {};
    };
    std::vector<DirectionChangeInfo> aDirectionChanges;
    std::deque< sal_Int32 > aKashida;
    std::deque< sal_Int32 > aKashidaInvalid;
    std::deque< sal_Int32 > aNoKashidaLine;
    std::deque< sal_Int32 > aNoKashidaLineEnd;
    std::deque< sal_Int32 > aHiddenChg;
    //! Records a single change in compression.
    struct CompressionChangeInfo
    {
        sal_Int32 position; //!< Character position where the change occurs.
        sal_Int32 length;   //!< Length of the segment.
        CompType  type;     //!< Type of compression that we change to.
        inline CompressionChangeInfo(sal_Int32 pos, sal_Int32 len, CompType typ) : position(pos), length(len), type(typ) {};
    };
    std::vector<CompressionChangeInfo> aCompressionChanges;
#ifdef DBG_UTIL
    CompType DbgCompType( const sal_Int32 nPos ) const;
#endif

    sal_Int32 nInvalidityPos;
    sal_uInt8 nDefaultDir;

    void UpdateBidiInfo( const OUString& rText );

    bool IsKashidaValid(sal_Int32 nKashPos) const;
    void MarkKashidaInvalid(sal_Int32 nKashPos);
    void ClearKashidaInvalid(sal_Int32 nKashPos);
    bool MarkOrClearKashidaInvalid(sal_Int32 nStt, sal_Int32 nLen, bool bMark, sal_Int32 nMarkCount);
    bool IsKashidaLine(sal_Int32 nCharIdx) const;
    // examines the range [ nStart, nStart + nEnd ] if there are kanas
    // returns start index of kana entry in array, otherwise SAL_MAX_SIZE
    size_t HasKana( sal_Int32 nStart, const sal_Int32 nEnd ) const;

public:

    SwScriptInfo();
    ~SwScriptInfo();

    // determines script changes
    void InitScriptInfo( const SwTextNode& rNode, bool bRTL );
    void InitScriptInfo( const SwTextNode& rNode );

    // set/get position from which data is invalid
    void SetInvalidityA(const sal_Int32 nPos)
    {
        if (nPos < nInvalidityPos)
            nInvalidityPos = nPos;
    }
    sal_Int32 GetInvalidityA() const
    {
        return nInvalidityPos;
    }

    // get default direction for paragraph
    inline sal_uInt8 GetDefaultDir() const { return nDefaultDir; };

    // array operations, nCnt refers to array position
    size_t CountScriptChg() const { return aScriptChanges.size(); }
    sal_Int32 GetScriptChg( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aScriptChanges.size(),"No ScriptChange today!");
        return aScriptChanges[nCnt].position;
    }
    sal_uInt8 GetScriptType( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aScriptChanges.size(),"No ScriptType today!");
        return aScriptChanges[nCnt].type;
    }

    size_t CountDirChg() const { return aDirectionChanges.size(); }
    sal_Int32 GetDirChg( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aDirectionChanges.size(),"No DirChange today!");
        return aDirectionChanges[ nCnt ].position;
    }
    sal_uInt8 GetDirType( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aDirectionChanges.size(),"No DirType today!");
        return aDirectionChanges[ nCnt ].type;
    }

    size_t CountKashida() const
    {
        return aKashida.size();
    }

    sal_Int32 GetKashida(const size_t nCnt) const
    {
        OSL_ENSURE( nCnt < aKashida.size(),"No Kashidas today!");
        return aKashida[nCnt];
    }

    size_t CountCompChg() const { return aCompressionChanges.size(); };
    sal_Int32 GetCompStart( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aCompressionChanges.size(),"No CompressionStart today!");
        return aCompressionChanges[ nCnt ].position;
    }
    sal_Int32 GetCompLen( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aCompressionChanges.size(),"No CompressionLen today!");
        return aCompressionChanges[ nCnt ].length;
    }
    CompType GetCompType( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aCompressionChanges.size(),"No CompressionType today!");
        return aCompressionChanges[ nCnt ].type;
    }

    size_t CountHiddenChg() const { return aHiddenChg.size(); };
    sal_Int32 GetHiddenChg( const size_t nCnt ) const
    {
        OSL_ENSURE( nCnt < aHiddenChg.size(),"No HiddenChg today!");
        return aHiddenChg[ nCnt ];
    }
    static void CalcHiddenRanges(const SwTextNode& rNode, MultiSelection& rHiddenMulti);
    static void selectHiddenTextProperty(const SwTextNode& rNode, MultiSelection &rHiddenMulti);
    static void selectRedLineDeleted(const SwTextNode& rNode, MultiSelection &rHiddenMulti, bool bSelect=true);

    // "high" level operations, nPos refers to string position
    sal_Int32 NextScriptChg( const sal_Int32 nPos ) const;
    sal_Int16 ScriptType( const sal_Int32 nPos ) const;

    // Returns the position of the next direction level change.
    // If bLevel is set, the position of the next level which is smaller
    // than the level at position nPos is returned. This is required to
    // obtain the end of a SwBidiPortion
    sal_Int32 NextDirChg( const sal_Int32 nPos,
                           const sal_uInt8* pLevel = 0 ) const;
    sal_uInt8 DirType( const sal_Int32 nPos ) const;

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
                                        PositionList* pList = 0 );
    bool GetBoundsOfHiddenRange(  sal_Int32 nPos, sal_Int32& rnStartPos,
                                  sal_Int32& rnEndPos, PositionList* pList = 0 ) const;

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
    long Compress( long* pKernArray, sal_Int32 nIdx, sal_Int32 nLen,
                   const sal_uInt16 nCompress, const sal_uInt16 nFontHeight,
                   const bool bCentered,
                   Point* pPoint = NULL ) const;

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
                              sal_Int32 nStt, sal_Int32 nLen, long nSpaceAdd = 0) const;

/** Clears array of kashidas marked as invalid
 */
    void ClearKashidaInvalid(sal_Int32 nStt, sal_Int32 nLen)
    {
        MarkOrClearKashidaInvalid(nStt, nLen, false, 0);
    }

/** Marks nCnt kashida positions as invalid
   pKashidaPositions: array of char indices relative to the paragraph
*/
    bool MarkKashidasInvalid(sal_Int32 nCnt, sal_Int32* pKashidaPositions);

/** Marks nCnt kashida positions as invalid
    in the given text range
 */
    bool MarkKashidasInvalid(sal_Int32 nCnt, sal_Int32 nStt, sal_Int32 nLen)
    {
        return MarkOrClearKashidaInvalid(nStt, nLen, true, nCnt);
    }

/** retrieves kashida opportunities for a given text range.
   returns the number of kashida positions in the given text range

   pKashidaPositions: buffer to reveive the char indices of the
                      kashida opportunties relative to the paragraph
*/
    sal_Int32 GetKashidaPositions(sal_Int32 nStt, sal_Int32 nLen,
       sal_Int32* pKashidaPosition);

/** Use regular blank justification instead of kashdida justification for the given line of text.
   nStt Start char index of the line referring to the paragraph.
   nLen Number of characters in the line
*/
    void SetNoKashidaLine(sal_Int32 nStt, sal_Int32 nLen);

/** Clear forced blank justification for a given line.
   nStt Start char index of the line referring to the paragraph.
   nLen Number of characters in the line
*/
    void ClearNoKashidaLine(sal_Int32 nStt, sal_Int32 nLen);

/** Checks if text is Arabic text.

     @descr  Checks if text is Arabic text.
     @param  rText
                 The text to check
     @param  nStt
                 Start index of the text
     @return Returns if the language is an Arabic language
 */
    static bool IsArabicText( const OUString& rText, sal_Int32 nStt, sal_Int32 nLen );

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
    static sal_Int32 ThaiJustify( const OUString& rText, long* pKernArray,
                                  long* pScrArray, sal_Int32 nIdx,
                                  sal_Int32 nLen, sal_Int32 nNumberOfBlanks = 0,
                                  long nSpaceAdd = 0 );

    static SwScriptInfo* GetScriptInfo( const SwTextNode& rNode,
                                        bool bAllowInvalid = false );

    static sal_uInt8 WhichFont(sal_Int32 nIdx, const OUString* pText, const SwScriptInfo* pSI);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

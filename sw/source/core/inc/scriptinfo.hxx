/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SCRIPTINFO_HXX
#define _SCRIPTINFO_HXX
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_XUB_STRLEN
#include <svl/svstdarr.hxx>
#endif
#include <i18npool/lang.h>
#include <list>
#include <modeltoviewhelper.hxx>

#include <errhdl.hxx>

class SwTxtNode;
class Point;
class MultiSelection;
class String;
typedef std::list< xub_StrLen > PositionList;

#define SPACING_PRECISION_FACTOR 100

/*************************************************************************
 *                class SwScanner
 * Hilfsklasse, die beim Spellen die Worte im gewuenschten Bereich
 * nacheinander zur Verfuegung stellt.
 *************************************************************************/

class SwScanner
{
    XubString aWord;
    const SwTxtNode& rNode;
    const String& rText;
    const LanguageType* pLanguage;
    const ModelToViewHelper::ConversionMap* pConversionMap;
    xub_StrLen nStartPos;
    xub_StrLen nEndPos;
    xub_StrLen nBegin;
    xub_StrLen nLen;
    LanguageType aCurrLang;
    USHORT nWordType;
    BOOL bClip;

public:
    SwScanner( const SwTxtNode& rNd, const String& rTxt, const LanguageType* pLang,
               const ModelToViewHelper::ConversionMap* pConvMap,
               USHORT nWordType,
               xub_StrLen nStart, xub_StrLen nEnde, BOOL bClip = FALSE );


    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    BOOL NextWord();

    const XubString& GetWord() const    { return aWord; }

    xub_StrLen GetBegin() const         { return nBegin; }
    xub_StrLen GetEnd() const           { return nBegin + nLen; }
    xub_StrLen GetLen() const           { return nLen; }

    LanguageType GetCurrentLanguage() const {return aCurrLang;}
};

/*************************************************************************
 *                      class SwScriptInfo
 *
 * encapsultes information about script changes
 *************************************************************************/

class SwScriptInfo
{
private:
    //! Records a single change in script type.
    struct ScriptChangeInfo
    {
        xub_StrLen position; //!< Character position at which we change script
        BYTE       type;     //!< Script type (Latin/Asian/Complex) that we change to.
        inline ScriptChangeInfo(xub_StrLen pos, BYTE typ) : position(pos), type(typ) {};
    };
    //TODO - This is sorted, so should probably be a std::set rather than vector.
    //       But we also use random access (probably unnecessarily).
    std::vector<ScriptChangeInfo> aScriptChanges;
    //! Records a single change in direction.
    struct DirectionChangeInfo
    {
        xub_StrLen position; //!< Character position at which we change direction.
        BYTE       type;     //!< Direction that we change to.
        inline DirectionChangeInfo(xub_StrLen pos, BYTE typ) : position(pos), type(typ) {};
    };
    std::vector<DirectionChangeInfo> aDirectionChanges;
    SvXub_StrLens aKashida;
    SvXub_StrLens aKashidaInvalid;
    SvXub_StrLens aNoKashidaLine;
    SvXub_StrLens aNoKashidaLineEnd;
    SvXub_StrLens aHiddenChg;
    //! Records a single change in compression.
    struct CompressionChangeInfo
    {
        xub_StrLen position; //!< Character position where the change occurs.
        xub_StrLen length;   //!< Length of the segment.
        BYTE       type;     //!< Type of compression that we change to.
        inline CompressionChangeInfo(xub_StrLen pos, xub_StrLen len, BYTE typ) : position(pos), length(len), type(typ) {};
    };
    std::vector<CompressionChangeInfo> aCompressionChanges;
    xub_StrLen nInvalidityPos;
    BYTE nDefaultDir;

    void UpdateBidiInfo( const String& rTxt );

    sal_Bool IsKashidaValid ( xub_StrLen nKashPos ) const;
    void MarkKashidaInvalid ( xub_StrLen nKashPos );
    void ClearKashidaInvalid ( xub_StrLen nKashPos );
    bool MarkOrClearKashidaInvalid( xub_StrLen nStt, xub_StrLen nLen, bool bMark, xub_StrLen nMarkCount );
    bool IsKashidaLine ( xub_StrLen nCharIdx ) const;

public:
    enum CompType { KANA, SPECIAL_LEFT, SPECIAL_RIGHT, NONE };

    SwScriptInfo();
    ~SwScriptInfo();

    // determines script changes
    void InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL );
    void InitScriptInfo( const SwTxtNode& rNode );

    // set/get position from which data is invalid
    inline void SetInvalidity( const xub_StrLen nPos );
    inline xub_StrLen GetInvalidity() const { return nInvalidityPos; };

    // get default direction for paragraph
    inline BYTE GetDefaultDir() const { return nDefaultDir; };

    // array operations, nCnt refers to array position
    inline USHORT CountScriptChg() const;
    inline xub_StrLen GetScriptChg( const USHORT nCnt ) const;
    inline BYTE GetScriptType( const USHORT nCnt ) const;

    inline USHORT CountDirChg() const;
    inline xub_StrLen GetDirChg( const USHORT nCnt ) const;
    inline BYTE GetDirType( const USHORT nCnt ) const;

    inline USHORT CountKashida() const;
    inline xub_StrLen GetKashida( const USHORT nCnt ) const;

    inline USHORT CountCompChg() const;
    inline xub_StrLen GetCompStart( const USHORT nCnt ) const;
    inline xub_StrLen GetCompLen( const USHORT nCnt ) const;
    inline BYTE GetCompType( const USHORT nCnt ) const;

    inline USHORT CountHiddenChg() const;
    inline xub_StrLen GetHiddenChg( const USHORT nCnt ) const;
    static void CalcHiddenRanges( const SwTxtNode& rNode,
                                                MultiSelection& rHiddenMulti );

    // "high" level operations, nPos refers to string position
    xub_StrLen NextScriptChg( const xub_StrLen nPos ) const;
    BYTE ScriptType( const xub_StrLen nPos ) const;

    // Returns the position of the next direction level change.
    // If bLevel is set, the position of the next level which is smaller
    // than the level at position nPos is returned. This is required to
    // obtain the end of a SwBidiPortion
    xub_StrLen NextDirChg( const xub_StrLen nPos,
                           const BYTE* pLevel = 0 ) const;
    BYTE DirType( const xub_StrLen nPos ) const;

#if OSL_DEBUG_LEVEL > 1
    BYTE CompType( const xub_StrLen nPos ) const;
#endif

    //
    // HIDDEN TEXT STUFF START
    //

/** Hidden text range information - static and non-version

    @descr  Determines if a given position is inside a hidden text range. The
            static version tries to obtain a valid SwScriptInfo object
            via the SwTxtNode, otherwise it calculates the values from scratch.
            The non-static version uses the internally cached informatio
            for the calculation.

    @param  rNode
                The text node.
    @param  nPos
                The given position that should be checked.
    @param  rnStartPos
                Return parameter for the start position of the hidden range.
                STRING_LEN if nPos is not inside a hidden range.
    @param  rnEndPos
                Return parameter for the end position of the hidden range.
                0 if nPos is not inside a hidden range.
    @param  rnEndPos
                Return parameter that contains all the hidden text ranges. Optional.
    @return
            returns true if there are any hidden characters in this paragraph.

*/
    static bool GetBoundsOfHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos,
                                        xub_StrLen& rnStartPos, xub_StrLen& rnEndPos,
                                        PositionList* pList = 0 );
    bool GetBoundsOfHiddenRange(  xub_StrLen nPos, xub_StrLen& rnStartPos,
                                  xub_StrLen& rnEndPos, PositionList* pList = 0 ) const;

    static bool IsInHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos );

/** Hidden text attribute handling

    @descr  Takes a string and either deletes the hidden ranges or sets
            a given character in place of the hidden characters.

    @param  rNode
                The text node.
    @param  nPos
                The string to modify.
    @param  cChar
                The character that should replace the hidden characters.
    @param  bDel
                If set, the hidden ranges will be deleted from the text node.
 */
    static USHORT MaskHiddenRanges( const SwTxtNode& rNode, XubString& rText,
                                    const xub_StrLen nStt, const xub_StrLen nEnd,
                                    const xub_Unicode cChar );

/** Hidden text attribute handling

    @descr  Takes a SwTxtNode and deletes the hidden ranges from the node.

    @param  rNode
                The text node.
 */
    static void DeleteHiddenRanges( SwTxtNode& rNode );

    //
    // HIDDEN TEXT STUFF END
    //

    // examines the range [ nStart, nStart + nEnd ] if there are kanas
    // returns start index of kana entry in array, otherwise USHRT_MAX
    USHORT HasKana( xub_StrLen nStart, const xub_StrLen nEnd ) const;

    // modifies the kerning array according to a given compress value
    long Compress( sal_Int32* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                   const USHORT nCompress, const USHORT nFontHeight,
                   Point* pPoint = NULL ) const;

/** Performes a kashida justification on the kerning array

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
    USHORT KashidaJustify( sal_Int32* pKernArray, sal_Int32* pScrArray,
                           xub_StrLen nStt, xub_StrLen nLen,
                           long nSpaceAdd = 0) const;

/** Clears array of kashidas marked as invalid
 */
    inline void ClearKashidaInvalid ( xub_StrLen nStt, xub_StrLen nLen ) { MarkOrClearKashidaInvalid( nStt, nLen, false, 0 ); }

/** Marks nCnt kashida positions as invalid
   pKashidaPositions: array of char indices relative to the paragraph
*/
   bool MarkKashidasInvalid ( xub_StrLen nCnt, xub_StrLen* pKashidaPositions );

/** Marks nCnt kashida positions as invalid
    in the given text range
 */
   inline bool MarkKashidasInvalid ( xub_StrLen nCnt, xub_StrLen nStt, xub_StrLen nLen )
       { return MarkOrClearKashidaInvalid( nStt, nLen, true, nCnt ); }

/** retrieves kashida opportunities for a given text range.
   returns the number of kashida positions in the given text range

   pKashidaPositions: buffer to reveive the char indices of the
                      kashida opportunties relative to the paragraph
*/
   USHORT GetKashidaPositions ( xub_StrLen nStt, xub_StrLen nLen,
                             xub_StrLen* pKashidaPosition );




/** Use regular blank justification instead of kashdida justification for the given line of text.
   nStt Start char index of the line referring to the paragraph.
   nLen Number of characters in the line
*/
   void SetNoKashidaLine ( xub_StrLen nStt, xub_StrLen nLen );

/** Clear forced blank justification for a given line.
   nStt Start char index of the line referring to the paragraph.
   nLen Number of characters in the line
*/
   void ClearNoKashidaLine ( xub_StrLen nStt, xub_StrLen nLen );

/** Checks if text is Arabic text.

     @descr  Checks if text is Arabic text.
     @param  rTxt
                 The text to check
     @param  nStt
                 Start index of the text
     @return Returns if the language is an Arabic language
 */
    static sal_Bool IsArabicText( const XubString& rTxt, xub_StrLen nStt, xub_StrLen nLen );

/** Performes a thai justification on the kerning array

    @descr  Add some extra space for thai justification to the
            positions in the kerning array.
    @param  rTxt
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
    static USHORT ThaiJustify( const XubString& rTxt, sal_Int32* pKernArray,
                               sal_Int32* pScrArray, xub_StrLen nIdx,
                               xub_StrLen nLen, xub_StrLen nNumberOfBlanks = 0,
                               long nSpaceAdd = 0 );

    static SwScriptInfo* GetScriptInfo( const SwTxtNode& rNode,
                                        sal_Bool bAllowInvalid = sal_False );

    static BYTE WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI );
};

inline void SwScriptInfo::SetInvalidity( const xub_StrLen nPos )
{
    if ( nPos < nInvalidityPos )
        nInvalidityPos = nPos;
};

inline USHORT SwScriptInfo::CountScriptChg() const { return aScriptChanges.size(); }
inline xub_StrLen SwScriptInfo::GetScriptChg( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aScriptChanges.size(),"No ScriptChange today!");
    return aScriptChanges[nCnt].position;
}
inline BYTE SwScriptInfo::GetScriptType( const xub_StrLen nCnt ) const
{
    OSL_ENSURE( nCnt < aScriptChanges.size(),"No ScriptType today!");
    return aScriptChanges[nCnt].type;
}

inline USHORT SwScriptInfo::CountDirChg() const { return aDirectionChanges.size(); }
inline xub_StrLen SwScriptInfo::GetDirChg( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aDirectionChanges.size(),"No DirChange today!");
    return aDirectionChanges[ nCnt ].position;
}
inline BYTE SwScriptInfo::GetDirType( const xub_StrLen nCnt ) const
{
    OSL_ENSURE( nCnt < aDirectionChanges.size(),"No DirType today!");
    return aDirectionChanges[ nCnt ].type;
}

inline USHORT SwScriptInfo::CountKashida() const { return aKashida.Count(); }
inline xub_StrLen SwScriptInfo::GetKashida( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aKashida.Count(),"No Kashidas today!");
    return aKashida[ nCnt ];
}

inline USHORT SwScriptInfo::CountCompChg() const { return aCompressionChanges.size(); };
inline xub_StrLen SwScriptInfo::GetCompStart( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aCompressionChanges.size(),"No CompressionStart today!");
    return aCompressionChanges[ nCnt ].position;
}
inline xub_StrLen SwScriptInfo::GetCompLen( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aCompressionChanges.size(),"No CompressionLen today!");
    return aCompressionChanges[ nCnt ].length;
}

inline BYTE SwScriptInfo::GetCompType( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aCompressionChanges.size(),"No CompressionType today!");
    return aCompressionChanges[ nCnt ].type;
}

inline USHORT SwScriptInfo::CountHiddenChg() const { return aHiddenChg.Count(); };
inline xub_StrLen SwScriptInfo::GetHiddenChg( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aHiddenChg.Count(),"No HiddenChg today!");
    return aHiddenChg[ nCnt ];
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

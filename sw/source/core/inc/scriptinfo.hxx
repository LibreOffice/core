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
#define _SVSTDARR_SHORTS
#define _SVSTDARR_BYTES
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
    sal_uInt16 nWordType;
    sal_Bool bClip;

public:
    SwScanner( const SwTxtNode& rNd, const String& rTxt, const LanguageType* pLang,
               const ModelToViewHelper::ConversionMap* pConvMap,
               sal_uInt16 nWordType,
               xub_StrLen nStart, xub_StrLen nEnde, sal_Bool bClip = sal_False );


    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    sal_Bool NextWord();

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
    SvXub_StrLens aScriptChg;
    SvBytes aScriptType;
    SvXub_StrLens aDirChg;
    SvBytes aDirType;
    SvXub_StrLens aKashida;
    SvXub_StrLens aKashidaInvalid;
    SvXub_StrLens aNoKashidaLine;
    SvXub_StrLens aNoKashidaLineEnd;
    SvXub_StrLens aCompChg;
    SvXub_StrLens aCompLen;
    SvXub_StrLens aHiddenChg;
    SvBytes aCompType;
    xub_StrLen nInvalidityPos;
    sal_uInt8 nDefaultDir;

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
    inline sal_uInt8 GetDefaultDir() const { return nDefaultDir; };

    // array operations, nCnt refers to array position
    inline size_t CountScriptChg() const;
    inline xub_StrLen GetScriptChg( const size_t nCnt ) const;
    inline sal_uInt8 GetScriptType( const sal_uInt16 nCnt ) const;

    inline size_t CountDirChg() const;
    inline xub_StrLen GetDirChg( const size_t nCnt ) const;
    inline sal_uInt8 GetDirType( const size_t nCnt ) const;

    inline size_t CountKashida() const;
    inline xub_StrLen GetKashida( const size_t nCnt ) const;

    inline size_t CountCompChg() const;
    inline xub_StrLen GetCompStart( const size_t nCnt ) const;
    inline xub_StrLen GetCompLen( const size_t nCnt ) const;
    inline sal_uInt8 GetCompType( const size_t nCnt ) const;

    inline size_t CountHiddenChg() const;
    inline xub_StrLen GetHiddenChg( const size_t nCnt ) const;
    static void CalcHiddenRanges( const SwTxtNode& rNode,
                                                MultiSelection& rHiddenMulti );

    // "high" level operations, nPos refers to string position
    xub_StrLen NextScriptChg( const xub_StrLen nPos ) const;
    sal_uInt8 ScriptType( const xub_StrLen nPos ) const;

    // Returns the position of the next direction level change.
    // If bLevel is set, the position of the next level which is smaller
    // than the level at position nPos is returned. This is required to
    // obtain the end of a SwBidiPortion
    xub_StrLen NextDirChg( const xub_StrLen nPos,
                           const sal_uInt8* pLevel = 0 ) const;
    sal_uInt8 DirType( const xub_StrLen nPos ) const;

#if OSL_DEBUG_LEVEL > 1
    sal_uInt8 CompType( const xub_StrLen nPos ) const;
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
    static sal_uInt16 MaskHiddenRanges( const SwTxtNode& rNode, XubString& rText,
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
    sal_uInt16 HasKana( xub_StrLen nStart, const xub_StrLen nEnd ) const;

    // modifies the kerning array according to a given compress value
    long Compress( sal_Int32* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                   const sal_uInt16 nCompress, const sal_uInt16 nFontHeight,
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
    sal_uInt16 KashidaJustify( sal_Int32* pKernArray, sal_Int32* pScrArray,
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
   sal_uInt16 GetKashidaPositions ( xub_StrLen nStt, xub_StrLen nLen,
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
    static sal_uInt16 ThaiJustify( const XubString& rTxt, sal_Int32* pKernArray,
                               sal_Int32* pScrArray, xub_StrLen nIdx,
                               xub_StrLen nLen, xub_StrLen nNumberOfBlanks = 0,
                               long nSpaceAdd = 0 );

    static SwScriptInfo* GetScriptInfo( const SwTxtNode& rNode,
                                        sal_Bool bAllowInvalid = sal_False );

    static sal_uInt8 WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI );
};

inline void SwScriptInfo::SetInvalidity( const xub_StrLen nPos )
{
    if ( nPos < nInvalidityPos )
        nInvalidityPos = nPos;
};
inline size_t SwScriptInfo::CountScriptChg() const { return aScriptChg.size(); }
inline xub_StrLen SwScriptInfo::GetScriptChg( const size_t nCnt ) const
{
    ASSERT( nCnt < aScriptChg.size(),"No ScriptChange today!");
    return aScriptChg[ nCnt ];
}
inline sal_uInt8 SwScriptInfo::GetScriptType( const xub_StrLen nCnt ) const
{
    ASSERT( nCnt < aScriptType.size(),"No ScriptType today!");
    return aScriptType[ nCnt ];
}

inline size_t SwScriptInfo::CountDirChg() const { return aDirChg.size(); }
inline xub_StrLen SwScriptInfo::GetDirChg( const size_t nCnt ) const
{
    ASSERT( nCnt < aDirChg.size(),"No DirChange today!");
    return aDirChg[ nCnt ];
}
inline sal_uInt8 SwScriptInfo::GetDirType( const size_t nCnt ) const
{
    ASSERT( nCnt < aDirType.size(),"No DirType today!");
    return aDirType[ nCnt ];
}

inline size_t SwScriptInfo::CountKashida() const { return aKashida.size(); }
inline xub_StrLen SwScriptInfo::GetKashida( const size_t nCnt ) const
{
    ASSERT( nCnt < aKashida.size(),"No Kashidas today!");
    return aKashida[ nCnt ];
}

inline size_t SwScriptInfo::CountCompChg() const { return aCompChg.size(); };
inline xub_StrLen SwScriptInfo::GetCompStart( const size_t nCnt ) const
{
    ASSERT( nCnt < aCompChg.size(),"No CompressionStart today!");
    return aCompChg[ nCnt ];
}
inline xub_StrLen SwScriptInfo::GetCompLen( const size_t nCnt ) const
{
    ASSERT( nCnt < aCompLen.size(),"No CompressionLen today!");
    return aCompLen[ nCnt ];
}

inline sal_uInt8 SwScriptInfo::GetCompType( const size_t nCnt ) const
{
    ASSERT( nCnt < aCompType.size(),"No CompressionType today!");
    return aCompType[ nCnt ];
}

inline size_t SwScriptInfo::CountHiddenChg() const { return aHiddenChg.size(); };
inline xub_StrLen SwScriptInfo::GetHiddenChg( const size_t nCnt ) const
{
    ASSERT( nCnt < aHiddenChg.size(),"No HiddenChg today!");
    return aHiddenChg[ nCnt ];
}


#endif

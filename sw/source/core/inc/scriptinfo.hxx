/*************************************************************************
 *
 *  $RCSfile: scriptinfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-30 10:18:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SCRIPTINFO_HXX
#define _SCRIPTINFO_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_SHORTS
#define _SVSTDARR_BYTES
#define _SVSTDARR_USHORTS
#define _SVSTDARR_XUB_STRLEN
#include <svtools/svstdarr.hxx>
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

class SwWrongList;
class SwTxtNode;
class Point;

/*************************************************************************
 *                class SwScanner
 * Hilfsklasse, die beim Spellen die Worte im gewuenschten Bereich
 * nacheinander zur Verfuegung stellt.
 *************************************************************************/

class SwScanner
{
    XubString aWord;
    const SwWrongList* pWrong;
    const SwTxtNode& rNode;
    xub_StrLen nEndPos;
    xub_StrLen nBegin;
    xub_StrLen nLen;
    LanguageType aCurrLang;
    USHORT nWordType;
    BOOL bReverse;
    BOOL bStart;
    BOOL bIsOnlineSpell;

public:
    SwScanner( const SwTxtNode& rNd, const SwWrongList* pWrng, USHORT nWordType,
               xub_StrLen nStart, xub_StrLen nEnde, BOOL bRev, BOOL bOS );

    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    BOOL NextWord();
    BOOL NextWord( LanguageType aLang );

    const XubString& GetWord() const    { return aWord; }

    xub_StrLen GetBegin() const         { return nBegin; }
    xub_StrLen GetEnd() const           { return nBegin + nLen; }
    xub_StrLen GetLen() const           { return nLen; }
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
    SvXub_StrLens aCompChg;
    SvXub_StrLens aCompLen;
    SvBytes aCompType;
    xub_StrLen nInvalidityPos;
    BYTE nDefaultDir;

    void UpdateBidiInfo( const String& rTxt );

public:
    enum CompType { KANA, SPECIAL_LEFT, SPECIAL_RIGHT, NONE };

    inline SwScriptInfo() : nInvalidityPos( 0 ), nDefaultDir( 0 ) {};

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

    BYTE CompType( const xub_StrLen nPos ) const;

    // examines the range [ nStart, nStart + nEnd ] if there are kanas
    // returns start index of kana entry in array, otherwise USHRT_MAX
    USHORT HasKana( xub_StrLen nStart, const xub_StrLen nEnd ) const;

    // modifies the kerning array according to a given compress value
    long Compress( long* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                   const USHORT nCompress, const USHORT nFontHeight,
                   Point* pPoint = NULL ) const;

/** Performes a kashida justification on the kerning array

    @descr  Add some extra space for kashida justification to the
            positions in the kerning array.
    @param  pKernArray
                The printers kerning array. Optional.
    @param  pScrArray
                The screen kerning array. Optional.
    @param  nIdx
                Start referring to the paragraph.
    @param  nLen
                The number of characters to be considered.
    @param  nSpace
                The value which has to be added to a kashida opportunity.
    @return The number of kashida opportunities in the given range
*/
    USHORT KashidaJustify( long* pKernArray ,long* pScrArray,
                           xub_StrLen nIdx, xub_StrLen nLen,
                           USHORT nSpace = 0 ) const;

/** Checks if language is one of the 16 Arabic languages

    @descr  Checks if language is one of the 16 Arabic languages
    @param  aLang
                The language which has to be checked.
    @return Returns if the language is an Arabic language
*/
    static BOOL IsArabicLanguage( LanguageType aLang );

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
    @param  nSpace
                The value which has to be added to the cells.
    @return The number of extra spaces in the given range
*/
    static USHORT ThaiJustify( const XubString& rTxt, long* pKernArray,
                               long* pScrArray, xub_StrLen nIdx,
                               xub_StrLen nLen, USHORT nSpace = 0 );

    static SwScriptInfo* GetScriptInfo( const SwTxtNode& rNode,
                                        sal_Bool bAllowInvalid = sal_False );

    static BYTE WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI );
};

inline void SwScriptInfo::SetInvalidity( const xub_StrLen nPos )
{
    if ( nPos < nInvalidityPos )
        nInvalidityPos = nPos;
};
inline USHORT SwScriptInfo::CountScriptChg() const { return aScriptChg.Count(); }
inline xub_StrLen SwScriptInfo::GetScriptChg( const USHORT nCnt ) const
{
    ASSERT( nCnt < aScriptChg.Count(),"No ScriptChange today!");
    return aScriptChg[ nCnt ];
}
inline BYTE SwScriptInfo::GetScriptType( const xub_StrLen nCnt ) const
{
    ASSERT( nCnt < aScriptChg.Count(),"No ScriptType today!");
    return aScriptType[ nCnt ];
}

inline USHORT SwScriptInfo::CountDirChg() const { return aDirChg.Count(); }
inline xub_StrLen SwScriptInfo::GetDirChg( const USHORT nCnt ) const
{
    ASSERT( nCnt < aDirChg.Count(),"No DirChange today!");
    return aDirChg[ nCnt ];
}
inline BYTE SwScriptInfo::GetDirType( const xub_StrLen nCnt ) const
{
    ASSERT( nCnt < aDirChg.Count(),"No DirType today!");
    return aDirType[ nCnt ];
}

inline USHORT SwScriptInfo::CountKashida() const { return aKashida.Count(); }
inline xub_StrLen SwScriptInfo::GetKashida( const USHORT nCnt ) const
{
    ASSERT( nCnt < aKashida.Count(),"No Kashidas today!");
    return aKashida[ nCnt ];
}

inline USHORT SwScriptInfo::CountCompChg() const { return aCompChg.Count(); };
inline xub_StrLen SwScriptInfo::GetCompStart( const USHORT nCnt ) const
{
    ASSERT( nCnt < aCompChg.Count(),"No CompressionStart today!");
    return aCompChg[ nCnt ];
}
inline xub_StrLen SwScriptInfo::GetCompLen( const USHORT nCnt ) const
{
    ASSERT( nCnt < aCompChg.Count(),"No CompressionLen today!");
    return aCompLen[ nCnt ];
}

inline BYTE SwScriptInfo::GetCompType( const USHORT nCnt ) const
{
    ASSERT( nCnt < aCompChg.Count(),"No CompressionType today!");
    return aCompType[ nCnt ];
}

#endif

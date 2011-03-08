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

#ifndef _MySVXACORR_HXX
#define _MySVXACORR_HXX

#include <com/sun/star/embed/XStorage.hpp>
#include <tools/rtti.hxx>
#include <svl/svarray.hxx>
#include <i18npool/lang.h>
#include <tools/time.hxx>
#include <tools/date.hxx>
#include <tools/string.hxx>
#include <editeng/swafopt.hxx>
#include "editeng/editengdllapi.h"

class CharClass;
class SfxPoolItem;
class SvxAutoCorrect;
class SvStringsISortDtor;
class SfxObjectShell;
class SvxAutoCorrLanguageTable_Impl;
class SvxAutoCorrLastFileAskTable_Impl;
class SotStorageRef;
class SotStorage;
class Window;

// Auto correct flags
const long CptlSttSntnc     = 0x00000001;   // Capital letters at the beginning of a sentence
const long CptlSttWrd       = 0x00000002;   // not two Capital letters at the beginning of a word
const long AddNonBrkSpace   = 0x00000004;   // Add non breaking space before :;?!
const long ChgOrdinalNumber = 0x00000008;   // Ordinal-Number 1st, 2nd,..
const long ChgToEnEmDash    = 0x00000010;   // - -> Endash/Emdash
const long ChgWeightUnderl  = 0x00000020;   // * -> Bold, _ -> Underscore
const long SetINetAttr      = 0x00000040;   // Set INetAttribut
const long Autocorrect      = 0x00000080;   // Call AutoCorrect
const long ChgQuotes        = 0x00000100;   // replace double quotes
const long SaveWordCplSttLst= 0x00000200;   // Save Auto correction of Capital letter at beginning of sentence.
const long SaveWordWrdSttLst= 0x00000400;   // Save Auto correction of 2 Capital letter at beginning of word.
const long IgnoreDoubleSpace= 0x00000800;   // Ignore 2 Spaces
const long ChgSglQuotes     = 0x00001000;   // Replace simple quotes
const long CorrectCapsLock  = 0x00002000;   // Correct accidental use of cAPS LOCK key

const long ChgWordLstLoad   = 0x20000000;   // Replacement list loaded
const long CplSttLstLoad    = 0x40000000;   // Exception list for Capital letters Start loaded
const long WrdSttLstLoad    = 0x80000000;   // Exception list for Word Start loaded

// TODO: handle unicodes > U+FFFF and check users of this class

// only a mapping class
class EDITENG_DLLPUBLIC SvxAutoCorrDoc
{
public:
    SvxAutoCorrDoc() {}
    virtual ~SvxAutoCorrDoc();

    virtual BOOL Delete( xub_StrLen nStt, xub_StrLen nEnd ) = 0;
    virtual BOOL Insert( xub_StrLen nPos, const String& rTxt ) = 0;
    virtual BOOL Replace( xub_StrLen nPos, const String& rTxt ) = 0;

    virtual BOOL SetAttr( xub_StrLen nStt, xub_StrLen nEnd, USHORT nSlotId,
                            SfxPoolItem& ) = 0;

    virtual BOOL SetINetAttr( xub_StrLen nStt, xub_StrLen nEnd, const String& rURL ) = 0;

    // Return the text of a previous paragraph. This must not be empty!
    // If no paragraph exits or just an empty one, then return 0.
    // The flag indicates:
    //      TRUE: before the normal insertion position (TRUE)
    //      FALSE: in which the corrected word was inserted.
    //             (Does not to have to be the same paragraph !!!!)
    virtual const String* GetPrevPara( BOOL bAtNormalPos ) = 0;

    virtual BOOL ChgAutoCorrWord( xub_StrLen& rSttPos, xub_StrLen nEndPos,
                                  SvxAutoCorrect& rACorrect,
                                  const String** ppPara ) = 0;
    // Is called after the change of the signs by the functions
    //  - FnCptlSttWrd
    //  - FnCptlSttSntnc
    // As an option, the words can then be inserted into the exception lists.
    virtual void SaveCpltSttWord( ULONG nFlag, xub_StrLen nPos,
                                    const String& rExceptWord,
                                    sal_Unicode cChar );

    // which language at the position?
    virtual LanguageType GetLanguage( xub_StrLen nPos, BOOL bPrevPara = FALSE ) const;
};


class EDITENG_DLLPUBLIC SvxAutocorrWord
{
    String sShort, sLong;
    BOOL bIsTxtOnly;                // Is pure ASCII - Text
public:
    SvxAutocorrWord( const String& rS, const String& rL, BOOL bFlag = TRUE )
        : sShort( rS ), sLong( rL ), bIsTxtOnly( bFlag )
    {}

    const String& GetShort() const                  { return sShort; }
    const String& GetLong() const                   { return sLong; }
    BOOL IsTextOnly() const                         { return bIsTxtOnly; }
};

typedef SvxAutocorrWord* SvxAutocorrWordPtr;
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvxAutocorrWordList, SvxAutocorrWordPtr, 10, 10 , EDITENG_DLLPUBLIC)

class EDITENG_DLLPUBLIC SvxAutoCorrectLanguageLists
{
    String sShareAutoCorrFile, sUserAutoCorrFile;
    // If the AutoCorr file is newer
    Date aModifiedDate;
    Time aModifiedTime, aLastCheckTime;

    LanguageType            eLanguage; //LANGUAGE_DONTKNOW use for all languages
    SvStringsISortDtor*     pCplStt_ExcptLst;
    SvStringsISortDtor*     pWrdStt_ExcptLst;
    SvxAutocorrWordList*    pAutocorr_List;
    SvxAutoCorrect&         rAutoCorrect;

    long nFlags;

    BOOL IsFileChanged_Imp();
    void LoadXMLExceptList_Imp( SvStringsISortDtor*& rpLst,
                                const sal_Char* pStrmName,
                                SotStorageRef& rStg);
    void SaveExceptList_Imp( const SvStringsISortDtor& rLst,
                                const sal_Char* pStrmName,
                                SotStorageRef& rStg,
                                BOOL bConvert = FALSE);

    BOOL MakeBlocklist_Imp( SotStorage& rStg );
    void RemoveStream_Imp( const String& rName );
    void MakeUserStorage_Impl();

public:
    SvxAutoCorrectLanguageLists( SvxAutoCorrect& rParent,
            const String& rShareAutoCorrectFile,
            const String& rUserAutoCorrectFile,
            LanguageType eLang);
    ~SvxAutoCorrectLanguageLists();

    // Load, Set, Get - the replacement list
    SvxAutocorrWordList* LoadAutocorrWordList();
    void SetAutocorrWordList( SvxAutocorrWordList* pList );
    const SvxAutocorrWordList* GetAutocorrWordList();

    // Load, Set, Get - the exception list for Capital letter at the
    // beginning of a sentence
    SvStringsISortDtor* LoadCplSttExceptList();
    void SaveCplSttExceptList();
    void SetCplSttExceptList( SvStringsISortDtor* pList );
    SvStringsISortDtor* GetCplSttExceptList();
    BOOL AddToCplSttExceptList(const String& rNew);

    // Load, Set, Get the exception list for 2 Capital letters at the
    // begining of a word.
    SvStringsISortDtor* LoadWrdSttExceptList();
    void SaveWrdSttExceptList();
    void SetWrdSttExceptList( SvStringsISortDtor* pList );
    SvStringsISortDtor* GetWrdSttExceptList();
    BOOL AddToWrdSttExceptList(const String& rNew);

    // Save word substitutions:
    //      Store these directly in the storage. The word list is updated
    //      accordingly!
    //  - pure Text
    BOOL PutText( const String& rShort, const String& rLong );
    //  - Text with attribution (only the SWG - SWG format!)
    BOOL PutText( const String& rShort, SfxObjectShell& );
    //  - Deleting an entry
    BOOL DeleteText( const String& rShort );
};


class EDITENG_DLLPUBLIC SvxAutoCorrect
{
    friend class SvxAutoCorrectLanguageLists;

    String sShareAutoCorrFile, sUserAutoCorrFile;

    SvxSwAutoFmtFlags aSwFlags;     // StarWriter AutoFormat Flags

    // all languages in a table
    SvxAutoCorrLanguageTable_Impl* pLangTable;
    SvxAutoCorrLastFileAskTable_Impl* pLastFileTable;
    CharClass* pCharClass;

    bool bRunNext;

    LanguageType eCharClassLang;

    long nFlags;
    sal_Unicode cStartDQuote, cEndDQuote, cStartSQuote, cEndSQuote,
                cEmDash, cEnDash;


    // private methods
    SvxAutoCorrectLanguageLists& _GetLanguageList( LanguageType eLang );

    void _GetCharClass( LanguageType eLang );

protected:
    //  - Text with attribution (only the SWG - SWG format!)
    //      rShort is the stream name - encrypted!
    virtual BOOL PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const String& rFileName, const String& rShort, SfxObjectShell& ,
                            String& );

    // required language in the table add if possible only when the file exists
    BOOL    CreateLanguageFile(LanguageType eLang, BOOL bNewFile = TRUE);
    //  - Return the replacement text (only for SWG format, all others can be
    //      taken from the word list!)
    //      rShort is the stream name - encrypted!
public:

    sal_Unicode GetQuote( sal_Unicode cInsChar, BOOL bSttQuote,
                            LanguageType eLang ) const;
    virtual BOOL GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const String& rFileName, const String& rShort, String& rLong );

    TYPEINFO();

    SvxAutoCorrect( const String& rShareAutocorrFile,
                    const String& rUserAutocorrFile );
    SvxAutoCorrect( const SvxAutoCorrect& );
    virtual ~SvxAutoCorrect();

    // Execute an AutoCorrect.
    // Returns what has been executed, according to the above flags
    ULONG AutoCorrect( SvxAutoCorrDoc& rDoc, const String& rTxt,
                        xub_StrLen nPos, sal_Unicode cInsChar, BOOL bInsert, Window* pFrameWin = NULL );

    // Return for the autotext expansion the previous word,
    // AutoCorrect - corresponding algorithm
    BOOL GetPrevAutoCorrWord( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen nPos, String& rWord ) const;

    // Search for or or the words in the replacement table.
    // rText - check in this text the words of the list
    // rStt - the detected starting position
    // nEnd - to check position - as of this item forward
    // rLang - Input: in which language is searched
    //         Output: in which "language list" was it found
    const SvxAutocorrWord* SearchWordsInList( const String& rTxt,
                                    xub_StrLen& rStt, xub_StrLen nEndPos,
                                    SvxAutoCorrDoc& rDoc,
                                    LanguageType& rLang );

    // Query/Set the Character for the Quote substitution
    sal_Unicode GetStartSingleQuote() const         { return cStartSQuote; }
    sal_Unicode GetEndSingleQuote() const           { return cEndSQuote; }
    sal_Unicode GetStartDoubleQuote() const         { return cStartDQuote; }
    sal_Unicode GetEndDoubleQuote() const           { return cEndDQuote; }

    void SetStartSingleQuote( const sal_Unicode cStart )    { cStartSQuote = cStart; }
    void SetEndSingleQuote( const sal_Unicode cEnd )        { cEndSQuote = cEnd; }
    void SetStartDoubleQuote( const sal_Unicode cStart )    { cStartDQuote = cStart; }
    void SetEndDoubleQuote( const sal_Unicode cEnd )        { cEndDQuote = cEnd; }

    String GetQuote( SvxAutoCorrDoc& rDoc, xub_StrLen nInsPos,
                    sal_Unicode cInsChar, BOOL bSttQuote );
    void InsertQuote( SvxAutoCorrDoc& rDoc, xub_StrLen nInsPos,
                    sal_Unicode cInsChar, BOOL bSttQuote, BOOL bIns );

    // Query/Set the name of the AutoCorrect file
    // the default is "autocorr.dat"
    String GetAutoCorrFileName( LanguageType eLang = LANGUAGE_SYSTEM,
                                BOOL bNewFile = FALSE,
                                BOOL bTstUserExist = FALSE ) const;
    void SetUserAutoCorrFileName( const String& rNew );
    void SetShareAutoCorrFileName( const String& rNew );

    // Query/Set the current settings of AutoCorrect
    long GetFlags() const                       { return nFlags; }
    inline SvxSwAutoFmtFlags&   GetSwFlags()    { return aSwFlags;}
    BOOL IsAutoCorrFlag( long nFlag ) const
                                { return nFlags & nFlag ? TRUE : FALSE; }
    void SetAutoCorrFlag( long nFlag, BOOL bOn = TRUE );

    // Load, Set, Get - the replacement list
    SvxAutocorrWordList* LoadAutocorrWordList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        { return _GetLanguageList( eLang ).LoadAutocorrWordList(); }
    const SvxAutocorrWordList* GetAutocorrWordList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        { return _GetLanguageList( eLang ).GetAutocorrWordList(); }

    // Save word substitutions:
    //      Save these directly in the storage. The word list is updated
    //      accordingly!
    //  - pure Text
    BOOL PutText( const String& rShort, const String& rLong, LanguageType eLang = LANGUAGE_SYSTEM );
    //  - Text with attribution (only in the SWG - SWG format!)
    BOOL PutText( const String& rShort, SfxObjectShell& rShell,
                 LanguageType eLang = LANGUAGE_SYSTEM )
        { return _GetLanguageList( eLang ).PutText(rShort, rShell ); }

    //  - Delete a entry
    BOOL DeleteText( const String& rShort, LanguageType eLang = LANGUAGE_SYSTEM);

    // Load, Set, Get - the exception list for capital letters at the
    // beginning of a sentence
    void SaveCplSttExceptList( LanguageType eLang = LANGUAGE_SYSTEM );
    SvStringsISortDtor* LoadCplSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM)
        {   return _GetLanguageList( eLang ).LoadCplSttExceptList(); }
    const SvStringsISortDtor* GetCplSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        {   return _GetLanguageList( eLang ).GetCplSttExceptList(); }

    // Adds a single word. The list will be immediately written to the file!
    BOOL AddCplSttException( const String& rNew,
                                LanguageType eLang = LANGUAGE_SYSTEM );

    // Load, Set, Get the exception list for 2 Capital letters at the
    // beginning of a word.
    void SaveWrdSttExceptList( LanguageType eLang = LANGUAGE_SYSTEM );
    SvStringsISortDtor* LoadWrdSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        {   return _GetLanguageList( eLang ).LoadWrdSttExceptList(); }
    const SvStringsISortDtor* GetWrdSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        {   return _GetLanguageList( eLang ).GetWrdSttExceptList(); }
    // Adds a single word. The list will be immediately written to the file!
    BOOL AddWrtSttException( const String& rNew, LanguageType eLang = LANGUAGE_SYSTEM);

    // Search through the Languages for the entry
    BOOL FindInWrdSttExceptList( LanguageType eLang, const String& sWord );
    BOOL FindInCplSttExceptList( LanguageType eLang, const String& sWord,
                                    BOOL bAbbreviation = FALSE);

    // Methods for the auto-correction
    BOOL FnCptlSttWrd( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    BOOL FnChgOrdinalNumber( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    BOOL FnChgToEnEmDash( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    BOOL FnAddNonBrkSpace( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    BOOL FnSetINetAttr( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    BOOL FnChgWeightUnderl( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    BOOL FnCptlSttSntnc( SvxAutoCorrDoc&, const String&, BOOL bNormalPos,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang  = LANGUAGE_SYSTEM);
    bool FnCorrectCapsLock( SvxAutoCorrDoc&, const String&,
                            xub_StrLen nSttPos, xub_StrLen nEndPos,
                            LanguageType eLang  = LANGUAGE_SYSTEM );

    bool                HasRunNext() { return bRunNext; }

    static long         GetDefaultFlags();

// returns TRUE for charcters where the function
// 'SvxAutoCorrect::AutoCorrect' should be called.
// (used to avoid occasional 'collisions' with (Thai) input-sequence-checking)
    static sal_Bool     IsAutoCorrectChar( sal_Unicode cChar );

    sal_Bool NeedsHardspaceAutocorr( sal_Unicode cChar );

    CharClass& GetCharClass( LanguageType eLang )
    {
        if( !pCharClass || eLang != eCharClassLang )
            _GetCharClass( eLang );
        return *pCharClass;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

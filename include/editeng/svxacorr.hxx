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

#ifndef EE_SVXACORR_HXX
#define EE_SVXACORR_HXX

#include <com/sun/star/embed/XStorage.hpp>

#include <o3tl/sorted_vector.hxx>
#include <tools/rtti.hxx>
#include <i18nlangtag/lang.h>
#include <tools/time.hxx>
#include <tools/date.hxx>
#include <tools/string.hxx>
#include <editeng/swafopt.hxx>
#include "editeng/editengdllapi.h"

#include <map>
#include <set>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_map.hpp>

class CharClass;
class SfxPoolItem;
class SvxAutoCorrect;
class SfxObjectShell;
class SotStorageRef;
class SotStorage;
class Window;

struct CompareSvStringsISortDtor
{
    bool operator()( String* const& lhs, String* const& rhs ) const
    {
        return lhs->CompareIgnoreCaseToAscii( *rhs ) == COMPARE_LESS;
    }
};

class SvStringsISortDtor
    : public o3tl::sorted_vector<String*, CompareSvStringsISortDtor>
{
public:
    ~SvStringsISortDtor() { DeleteAndDestroyAll(); }
};

// Auto correct flags
const long CptlSttSntnc     = 0x00000001;   // Capital letters at the beginning of a sentence
const long CptlSttWrd       = 0x00000002;   // not two Capital letters at the beginning of a word
const long AddNonBrkSpace   = 0x00000004;   // Add non breaking space before :;?!%
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

    virtual sal_Bool Delete( xub_StrLen nStt, xub_StrLen nEnd ) = 0;
    virtual sal_Bool Insert( xub_StrLen nPos, const String& rTxt ) = 0;
    virtual sal_Bool Replace( xub_StrLen nPos, const String& rTxt ) = 0;
    virtual sal_Bool ReplaceRange( xub_StrLen nPos, xub_StrLen nLen, const String& rTxt ) = 0;

    virtual sal_Bool SetAttr( xub_StrLen nStt, xub_StrLen nEnd, sal_uInt16 nSlotId,
                            SfxPoolItem& ) = 0;

    virtual sal_Bool SetINetAttr( xub_StrLen nStt, xub_StrLen nEnd, const OUString& rURL ) = 0;

    // Return the text of a previous paragraph. This must not be empty!
    // If no paragraph exits or just an empty one, then return 0.
    // The flag indicates:
    //      TRUE: before the normal insertion position (TRUE)
    //      FALSE: in which the corrected word was inserted.
    //             (Does not to have to be the same paragraph !!!!)
    virtual const String* GetPrevPara( sal_Bool bAtNormalPos ) = 0;

    virtual sal_Bool ChgAutoCorrWord( xub_StrLen& rSttPos, xub_StrLen nEndPos,
                                  SvxAutoCorrect& rACorrect,
                                  const String** ppPara ) = 0;
    // Is called after the change of the signs by the functions
    //  - FnCptlSttWrd
    //  - FnCptlSttSntnc
    // As an option, the words can then be inserted into the exception lists.
    virtual void SaveCpltSttWord( sal_uLong nFlag, xub_StrLen nPos,
                                    const String& rExceptWord,
                                    sal_Unicode cChar );

    // which language at the position?
    virtual LanguageType GetLanguage( xub_StrLen nPos, sal_Bool bPrevPara = sal_False ) const;
};


class EDITENG_DLLPUBLIC SvxAutocorrWord
{
    String sShort, sLong;
    sal_Bool bIsTxtOnly;                // Is pure ASCII - Text
public:
    SvxAutocorrWord( const String& rS, const String& rL, sal_Bool bFlag = sal_True )
        : sShort( rS ), sLong( rL ), bIsTxtOnly( bFlag )
    {}

    const String& GetShort() const                  { return sShort; }
    const String& GetLong() const                   { return sLong; }
    sal_Bool IsTextOnly() const                     { return bIsTxtOnly; }
};

struct CompareSvxAutocorrWordList
{
  bool operator()( SvxAutocorrWord* const& lhs, SvxAutocorrWord* const& rhs ) const;
};

typedef std::set<SvxAutocorrWord*, CompareSvxAutocorrWordList> SvxAutocorrWordList_Set;
typedef ::boost::unordered_map< OUString, SvxAutocorrWord *,
                                OUStringHash >          SvxAutocorrWordList_Hash;

class EDITENG_DLLPUBLIC SvxAutocorrWordList
{
    // only one of these contains the data
    mutable SvxAutocorrWordList_Set  maSet;
    mutable SvxAutocorrWordList_Hash maHash; // key is 'Short'

    const SvxAutocorrWord* WordMatches(const SvxAutocorrWord *pFnd,
                                       const String &rTxt,
                                       xub_StrLen &rStt,
                                       xub_StrLen nEndPos) const;
public:
                           // free any objects still in the set
                           ~SvxAutocorrWordList();
    void                   DeleteAndDestroyAll();
    bool                   Insert(SvxAutocorrWord *pWord) const;
    SvxAutocorrWord*       FindAndRemove(SvxAutocorrWord *pWord);
    void                   LoadEntry(String sWrong, String sRight, sal_Bool bOnlyTxt);
    bool                   empty() const;

    typedef std::vector<SvxAutocorrWord *> Content;
    Content                getSortedContent() const;

    const SvxAutocorrWord* SearchWordsInList(const String& rTxt, xub_StrLen& rStt, xub_StrLen nEndPos) const;
};

class EDITENG_DLLPUBLIC SvxAutoCorrectLanguageLists
{
    String sShareAutoCorrFile, sUserAutoCorrFile;
    // If the AutoCorr file is newer
    Date aModifiedDate;
    Time aModifiedTime, aLastCheckTime;

    SvStringsISortDtor*     pCplStt_ExcptLst;
    SvStringsISortDtor*     pWrdStt_ExcptLst;
    SvxAutocorrWordList*    pAutocorr_List;
    SvxAutoCorrect&         rAutoCorrect;

    long nFlags;

    sal_Bool IsFileChanged_Imp();
    void LoadXMLExceptList_Imp( SvStringsISortDtor*& rpLst,
                                const sal_Char* pStrmName,
                                SotStorageRef& rStg);
    void SaveExceptList_Imp( const SvStringsISortDtor& rLst,
                                const sal_Char* pStrmName,
                                SotStorageRef& rStg,
                                sal_Bool bConvert = sal_False);

    sal_Bool MakeBlocklist_Imp( SotStorage& rStg );
    void RemoveStream_Imp( const String& rName );
    void MakeUserStorage_Impl();

public:
    SvxAutoCorrectLanguageLists( SvxAutoCorrect& rParent,
            const String& rShareAutoCorrectFile,
            const String& rUserAutoCorrectFile);
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
    sal_Bool AddToCplSttExceptList(const String& rNew);

    // Load, Set, Get the exception list for 2 Capital letters at the
    // beginning of a word.
    SvStringsISortDtor* LoadWrdSttExceptList();
    void SaveWrdSttExceptList();
    void SetWrdSttExceptList( SvStringsISortDtor* pList );
    SvStringsISortDtor* GetWrdSttExceptList();
    sal_Bool AddToWrdSttExceptList(const String& rNew);

    // Save word substitutions:
    //      Store these directly in the storage. The word list is updated
    //      accordingly!
    //  - pure Text
    sal_Bool PutText( const String& rShort, const String& rLong );
    //  - Text with attribution (only the SWG - SWG format!)
    sal_Bool PutText( const String& rShort, SfxObjectShell& );
    //  - Deleting an entry
    sal_Bool DeleteText( const String& rShort );
    //  - Make combined changes in one pass
    sal_Bool MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries, std::vector<SvxAutocorrWord>& aDeleteEntries );
};

class EDITENG_DLLPUBLIC SvxAutoCorrect
{
    friend class SvxAutoCorrectLanguageLists;

    String sShareAutoCorrFile, sUserAutoCorrFile;

    SvxSwAutoFmtFlags aSwFlags;     // StarWriter AutoFormat Flags

    // all languages in a table
    boost::ptr_map<LanguageType, SvxAutoCorrectLanguageLists>* pLangTable;
    std::map<LanguageType, long> aLastFileTable;
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
    virtual sal_Bool PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg,
                              const String& rFileName, const String& rShort, SfxObjectShell&, OUString& );

    // required language in the table add if possible only when the file exists
    sal_Bool    CreateLanguageFile(LanguageType eLang, sal_Bool bNewFile = sal_True);
    //  - Return the replacement text (only for SWG format, all others can be
    //      taken from the word list!)
    //      rShort is the stream name - encrypted!
public:

    sal_Unicode GetQuote( sal_Unicode cInsChar, sal_Bool bSttQuote,
                            LanguageType eLang ) const;
    virtual sal_Bool GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg,
                                  const String& rFileName, const String& rShort, OUString& rLong );

    TYPEINFO();

    SvxAutoCorrect( const String& rShareAutocorrFile,
                    const String& rUserAutocorrFile );
    SvxAutoCorrect( const SvxAutoCorrect& );
    virtual ~SvxAutoCorrect();

    // Execute an AutoCorrect.
    // Returns what has been executed, according to the above flags
    // FIXME: this has the horrible flaw that the rTxt must be a reference
    // to the actual SwTxtNode/EditNode string because it inserts the character
    // in rDoc and expects that to side-effect rTxt
    sal_uLong DoAutoCorrect( SvxAutoCorrDoc& rDoc, const String& rTxt,
                           xub_StrLen nPos, sal_Unicode cInsChar, sal_Bool bInsert, Window* pFrameWin = NULL );

    // Return for the autotext expansion the previous word,
    // AutoCorrect - corresponding algorithm
    sal_Bool GetPrevAutoCorrWord( SvxAutoCorrDoc& rDoc, const String& rTxt,
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
                    sal_Unicode cInsChar, sal_Bool bSttQuote );
    void InsertQuote( SvxAutoCorrDoc& rDoc, xub_StrLen nInsPos,
                    sal_Unicode cInsChar, sal_Bool bSttQuote, sal_Bool bIns );

    // Query/Set the name of the AutoCorrect file
    // the default is "autocorr.dat"
    OUString GetAutoCorrFileName( LanguageType eLang = LANGUAGE_SYSTEM,
                                sal_Bool bNewFile = sal_False,
                                sal_Bool bTstUserExist = sal_False ) const;

    // Query/Set the current settings of AutoCorrect
    long GetFlags() const                       { return nFlags; }
    inline SvxSwAutoFmtFlags&   GetSwFlags()    { return aSwFlags;}
    sal_Bool IsAutoCorrFlag( long nFlag ) const
                                { return nFlags & nFlag ? sal_True : sal_False; }
    void SetAutoCorrFlag( long nFlag, sal_Bool bOn = sal_True );

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
    sal_Bool PutText( const String& rShort, const String& rLong, LanguageType eLang = LANGUAGE_SYSTEM );
    //  - Text with attribution (only in the SWG - SWG format!)
    sal_Bool PutText( const String& rShort, SfxObjectShell& rShell,
                 LanguageType eLang = LANGUAGE_SYSTEM )
        { return _GetLanguageList( eLang ).PutText(rShort, rShell ); }

    sal_Bool MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries,
                                  std::vector<SvxAutocorrWord>& aDeleteEntries,
                                  LanguageType eLang = LANGUAGE_SYSTEM );

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
    sal_Bool AddCplSttException( const String& rNew,
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
    sal_Bool AddWrtSttException( const String& rNew, LanguageType eLang = LANGUAGE_SYSTEM);

    // Search through the Languages for the entry
    sal_Bool FindInWrdSttExceptList( LanguageType eLang, const String& sWord );
    sal_Bool FindInCplSttExceptList( LanguageType eLang, const String& sWord,
                                    sal_Bool bAbbreviation = sal_False);

    // Methods for the auto-correction
    sal_Bool FnCptlSttWrd( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    sal_Bool FnChgOrdinalNumber( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    sal_Bool FnChgToEnEmDash( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    sal_Bool FnAddNonBrkSpace( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    sal_Bool FnSetINetAttr( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    sal_Bool FnChgWeightUnderl( SvxAutoCorrDoc&, const String&,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang = LANGUAGE_SYSTEM );
    sal_Bool FnCptlSttSntnc( SvxAutoCorrDoc&, const String&, sal_Bool bNormalPos,
                                xub_StrLen nSttPos, xub_StrLen nEndPos,
                                LanguageType eLang  = LANGUAGE_SYSTEM);
    bool FnCorrectCapsLock( SvxAutoCorrDoc&, const String&,
                            xub_StrLen nSttPos, xub_StrLen nEndPos,
                            LanguageType eLang  = LANGUAGE_SYSTEM );

    bool                HasRunNext() { return bRunNext; }

    static long         GetDefaultFlags();

// returns sal_True for characters where the function
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

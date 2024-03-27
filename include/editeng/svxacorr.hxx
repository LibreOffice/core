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

#ifndef INCLUDED_EDITENG_SVXACORR_HXX
#define INCLUDED_EDITENG_SVXACORR_HXX

#include <o3tl/sorted_vector.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <o3tl/string_view.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/time.hxx>
#include <tools/date.hxx>
#include <editeng/swafopt.hxx>
#include <editeng/editengdllapi.h>

#include <optional>
#include <map>
#include <memory>
#include <string_view>
#include <utility>

class CharClass;
class SfxPoolItem;
class SotStorage;
class SvxAutoCorrect;
class SfxObjectShell;
namespace vcl { class Window; }
namespace com::sun::star::embed { class XStorage; }
namespace tools { template <typename T> class SvRef; }

struct CompareSvStringsISortDtor
{
    bool operator()( std::u16string_view lhs, std::u16string_view rhs ) const
    {
        return o3tl::compareToIgnoreAsciiCase( lhs, rhs ) < 0;
    }
};

class SvStringsISortDtor
    : public o3tl::sorted_vector<OUString, CompareSvStringsISortDtor>
{
};

// Auto correct flags
enum class ACFlags : sal_uInt32 {
    NONE                 = 0x00000000,
    CapitalStartSentence = 0x00000001,   // Capital letters at the beginning of a sentence
    CapitalStartWord     = 0x00000002,   // not two Capital letters at the beginning of a word
    AddNonBrkSpace       = 0x00000004,   // Add non breaking space before :,?!%
    ChgOrdinalNumber     = 0x00000008,   // Ordinal-Number 1st, 2nd,..
    ChgToEnEmDash        = 0x00000010,   // - -> Endash/Emdash
    ChgWeightUnderl      = 0x00000020,   // * -> Bold, _ -> Underscore
    SetINetAttr          = 0x00000040,   // Set INetAttribut
    Autocorrect          = 0x00000080,   // Call AutoCorrect
    ChgQuotes            = 0x00000100,   // replace double quotes
    SaveWordCplSttLst    = 0x00000200,   // Save Auto correction of Capital letter at beginning of sentence.
    SaveWordWordStartLst    = 0x00000400,   // Save Auto correction of TWo INitial CApitals or sMALL iNITIAL.
    IgnoreDoubleSpace    = 0x00000800,   // Ignore 2 Spaces
    ChgSglQuotes         = 0x00001000,   // Replace simple quotes
    CorrectCapsLock      = 0x00002000,   // Correct accidental use of cAPS LOCK key
    TransliterateRTL     = 0x00004000,   // Transliterate RTL text
    ChgAngleQuotes       = 0x00008000,   // >>, << -> angle quotes in some languages

    ChgWordLstLoad       = 0x20000000,   // Replacement list loaded
    CplSttLstLoad        = 0x40000000,   // Exception list for Capital letters Start loaded
    WordStartLstLoad        = 0x80000000,   // Exception list for Word Start loaded
};
namespace o3tl {
    template<> struct typed_flags<ACFlags> : is_typed_flags<ACFlags, 0xe000ffff> {};
}

enum class ACQuotes
{
    NONE,
    NonBreakingSpace,
    CapitalizeIAm,
    DoubleAngleQuote,
    UseApostrophe,
};

// TODO: handle code points > U+FFFF and check users of this class

// only a mapping class
class EDITENG_DLLPUBLIC SvxAutoCorrDoc
{
public:
    virtual ~SvxAutoCorrDoc();

    virtual bool Delete( sal_Int32 nStt, sal_Int32 nEnd ) = 0;
    virtual bool Insert( sal_Int32 nPos, const OUString& rTxt ) = 0;
    virtual bool Replace( sal_Int32 nPos, const OUString& rTxt ) = 0;
    virtual bool ReplaceRange( sal_Int32 nPos, sal_Int32 nLen, const OUString& rTxt ) = 0;

    virtual void SetAttr( sal_Int32 nStt, sal_Int32 nEnd, sal_uInt16 nSlotId,
                            SfxPoolItem& ) = 0;

    virtual bool SetINetAttr( sal_Int32 nStt, sal_Int32 nEnd, const OUString& rURL ) = 0;

    // Return the text of a previous paragraph.
    // If no paragraph exits or just an empty one, then return an empty string.
    // The flag indicates:
    //      TRUE: before the normal insertion position (TRUE)
    //      FALSE: in which the corrected word was inserted.
    //             (Does not to have to be the same paragraph !!!!)
    virtual OUString const* GetPrevPara(bool bAtNormalPos) = 0;

    virtual bool ChgAutoCorrWord( sal_Int32& rSttPos, sal_Int32 nEndPos,
                                  SvxAutoCorrect& rACorrect,
                                  OUString* pPara ) = 0;
    virtual bool TransliterateRTLWord( sal_Int32& rSttPos, sal_Int32 nEndPos,
                                  bool bApply = false ) = 0;

    // Is called after the change of the signs by the functions
    //  - FnCapitalStartWord
    //  - FnCapitalStartSentence
    // As an option, the words can then be inserted into the exception lists.
    virtual void SaveCpltSttWord( ACFlags nFlag, sal_Int32 nPos,
                                    const OUString& rExceptWord,
                                    sal_Unicode cChar );

    // which language at the position?
    virtual LanguageType GetLanguage( sal_Int32 nPos ) const;
};


class EDITENG_DLLPUBLIC SvxAutocorrWord
{
    OUString sShort, sLong;
    bool bIsTxtOnly;                // Is pure ASCII - Text
public:
    SvxAutocorrWord( OUString aS, OUString aL, bool bFlag = true )
        : sShort(std::move( aS )), sLong(std::move( aL )), bIsTxtOnly( bFlag )
    {}

    const OUString& GetShort() const                  { return sShort; }
    const OUString& GetLong() const                   { return sLong; }
    bool IsTextOnly() const                           { return bIsTxtOnly; }
};

class EDITENG_DLLPUBLIC SvxAutocorrWordList
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

    SvxAutocorrWordList( const SvxAutocorrWordList& ) = delete;
    const SvxAutocorrWordList& operator= ( const SvxAutocorrWordList& ) = delete;

    const SvxAutocorrWord* WordMatches(const SvxAutocorrWord *pFnd,
                                       std::u16string_view rTxt,
                                       sal_Int32 &rStt,
                                       sal_Int32 nEndPos) const;
public:
    SvxAutocorrWordList();
                           // free any objects still in the set
                           ~SvxAutocorrWordList();
    void                   DeleteAndDestroyAll();
    const SvxAutocorrWord* Insert(SvxAutocorrWord aWord) const;
    std::optional<SvxAutocorrWord> FindAndRemove(const SvxAutocorrWord *pWord);
    void                   LoadEntry(const OUString& sWrong, const OUString& sRight, bool bOnlyTxt);
    bool                   empty() const;

    struct CompareSvxAutocorrWordList;
    typedef std::vector<SvxAutocorrWord> AutocorrWordSetType;
    const AutocorrWordSetType & getSortedContent() const;

    const SvxAutocorrWord* SearchWordsInList(std::u16string_view rTxt, sal_Int32& rStt, sal_Int32 nEndPos) const;
};

class EDITENG_DLLPUBLIC SvxAutoCorrectLanguageLists
{
    OUString sShareAutoCorrFile, sUserAutoCorrFile;
    // If the AutoCorr file is newer
    Date aModifiedDate;
    tools::Time aModifiedTime, aLastCheckTime;

    std::unique_ptr<SvStringsISortDtor> pCplStt_ExcptLst;
    std::unique_ptr<SvStringsISortDtor> pWordStart_ExcptLst;
    std::unique_ptr<SvxAutocorrWordList> pAutocorr_List;
    SvxAutoCorrect&         rAutoCorrect;

    ACFlags nFlags;

    bool IsFileChanged_Imp();
    void LoadXMLExceptList_Imp( std::unique_ptr<SvStringsISortDtor>& rpLst,
                                const OUString& sStrmName,
                                tools::SvRef<SotStorage>& rStg);
    static void SaveExceptList_Imp( const SvStringsISortDtor& rLst,
                                const OUString& sStrmName,
                                tools::SvRef<SotStorage> const & rStg,
                                bool bConvert = false);

    bool MakeBlocklist_Imp( SotStorage& rStg );
    void RemoveStream_Imp( const OUString& rName );
    void MakeUserStorage_Impl();

public:
    SvxAutoCorrectLanguageLists( SvxAutoCorrect& rParent,
            OUString aShareAutoCorrectFile,
            OUString aUserAutoCorrectFile);
    ~SvxAutoCorrectLanguageLists();

    // Load, Set, Get - the replacement list
    SvxAutocorrWordList* LoadAutocorrWordList();
    const SvxAutocorrWordList* GetAutocorrWordList();

    // Load, Set, Get - the exception list for Capital letter at the
    // beginning of a sentence
    SvStringsISortDtor* LoadCplSttExceptList();
    void SaveCplSttExceptList();
    SvStringsISortDtor* GetCplSttExceptList();
    bool AddToCplSttExceptList(const OUString& rNew);

    // Load, Set, Get the exception list for TWo INitial CApitals or sMALL iNITIAL
    SvStringsISortDtor* LoadWordStartExceptList();
    void SaveWordStartExceptList();
    SvStringsISortDtor* GetWordStartExceptList();
    bool AddToWordStartExceptList(const OUString& rNew);

    // Save word substitutions:
    //      Store these directly in the storage. The word list is updated
    //      accordingly!
    //  - pure Text
    bool PutText( const OUString& rShort, const OUString& rLong );
    //  - Text with attribution (only the SWG - SWG format!)
    void PutText( const OUString& rShort, SfxObjectShell& );
    //  - Make combined changes in one pass
    bool MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries, std::vector<SvxAutocorrWord>& aDeleteEntries );
};

class EDITENG_DLLPUBLIC SvxAutoCorrect
{
    friend class SvxAutoCorrectLanguageLists;

    OUString sShareAutoCorrFile, sUserAutoCorrFile;

    SvxSwAutoFormatFlags aSwFlags;     // StarWriter AutoFormat Flags

    // all languages in a table
    std::map<LanguageTag, SvxAutoCorrectLanguageLists> m_aLangTable;
    std::map<LanguageTag, sal_Int64> aLastFileTable;
    std::unique_ptr<CharClass> pCharClass;

    LanguageType eCharClassLang;

    ACFlags nFlags;
    sal_Unicode cStartDQuote, cEndDQuote, cStartSQuote, cEndSQuote;


    // private methods
    SvxAutoCorrectLanguageLists& GetLanguageList_( LanguageType eLang );

    void GetCharClass_( LanguageType eLang );

protected:
    //  - Text with attribution (only the SWG - SWG format!)
    //      rShort is the stream name - encrypted!
    virtual bool PutText( const css::uno::Reference < css::embed::XStorage >& rStg,
                              const OUString& rFileName, const OUString& rShort, SfxObjectShell&, OUString& );

    // required language in the table add if possible only when the file exists
    bool    CreateLanguageFile(const LanguageTag& rLanguageTag, bool bNewFile = true);
    //  - Return the replacement text (only for SWG format, all others can be
    //      taken from the word list!)
    //      rShort is the stream name - encrypted!
public:

    sal_Unicode GetQuote( sal_Unicode cInsChar, bool bSttQuote,
                            LanguageType eLang ) const;
    virtual bool GetLongText( const OUString& rShort, OUString& rLong );

    virtual void refreshBlockList( const css::uno::Reference < css::embed::XStorage >& rStg);

    SvxAutoCorrect( OUString aShareAutocorrFile,
                    OUString aUserAutocorrFile );
    SvxAutoCorrect( const SvxAutoCorrect& );
    virtual ~SvxAutoCorrect();

    /** Execute an AutoCorrect.
        Returns what has been executed, according to the above auto correct flags.
        @param  io_bNbspRunNext
                Remembers if a NO-BREAK SPACE was added (eg. in "fr" language)
                (set to <TRUE/>) at the last character input that may have to
                be removed again depending on what character is following.
     */
    // FIXME: this has the horrible flaw that the rTxt must be a reference
    // to the actual SwTxtNode/EditNode string because it inserts the character
    // in rDoc and expects that to side-effect rTxt
    void DoAutoCorrect( SvxAutoCorrDoc& rDoc, const OUString& rTxt,
                           sal_Int32 nPos, sal_Unicode cInsChar, bool bInsert, bool& io_bNbspRunNext,
                           vcl::Window const * pFrameWin = nullptr );

    // Return for the autotext expansion the previous word,
    // AutoCorrect - corresponding algorithm
    OUString GetPrevAutoCorrWord(SvxAutoCorrDoc const& rDoc, const OUString& rTxt, sal_Int32 nPos);

    // Returns vector candidates for AutoText name match, starting with the longest string between
    // 3 and 9 characters long, that is a chunk of text starting with a whitespace or with a word's
    // first character, and ending at the current cursor position or empty string if no such string
    // exists
    static std::vector<OUString> GetChunkForAutoText(std::u16string_view rTxt, sal_Int32 nPos);

    // Search for the words in the replacement table.
    // rText - check in this text the words of the list
    // rStt - the detected starting position
    // nEnd - to check position - as of this item forward
    // rLang - Input: in which language is searched
    //         Output: in which "language list" was it found
    const SvxAutocorrWord* SearchWordsInList( std::u16string_view rTxt,
                                    sal_Int32& rStt, sal_Int32 nEndPos,
                                    SvxAutoCorrDoc& rDoc,
                                    LanguageTag& rLang );

    // Query/Set the Character for the Quote substitution
    sal_Unicode GetStartSingleQuote() const         { return cStartSQuote; }
    sal_Unicode GetEndSingleQuote() const           { return cEndSQuote; }
    sal_Unicode GetStartDoubleQuote() const         { return cStartDQuote; }
    sal_Unicode GetEndDoubleQuote() const           { return cEndDQuote; }

    void SetStartSingleQuote( const sal_Unicode cStart )    { cStartSQuote = cStart; }
    void SetEndSingleQuote( const sal_Unicode cEnd )        { cEndSQuote = cEnd; }
    void SetStartDoubleQuote( const sal_Unicode cStart )    { cStartDQuote = cStart; }
    void SetEndDoubleQuote( const sal_Unicode cEnd )        { cEndDQuote = cEnd; }

    OUString GetQuote( SvxAutoCorrDoc const & rDoc, sal_Int32 nInsPos,
                    sal_Unicode cInsChar, bool bSttQuote );
    void InsertQuote( SvxAutoCorrDoc& rDoc, sal_Int32 nInsPos,
                    sal_Unicode cInsChar, bool bSttQuote, bool bIns,
                    LanguageType eLang, ACQuotes eType ) const;

    // Query/Set the name of the AutoCorrect file
    // the default is "autocorr.dat"
    OUString GetAutoCorrFileName( const LanguageTag& rLanguageTag /* = LANGUAGE_SYSTEM */ ,
                                bool bNewFile = false,
                                bool bTstUserExist = false,
                                bool bUnlocalized = false ) const;

    // Query/Set the current settings of AutoCorrect
    ACFlags GetFlags() const                { return nFlags; }
    SvxSwAutoFormatFlags&   GetSwFlags()    { return aSwFlags;}
    const SvxSwAutoFormatFlags& GetSwFlags() const { return aSwFlags; }
    bool IsAutoCorrFlag( ACFlags nFlag ) const
                                { return bool(nFlags & nFlag); }
    void SetAutoCorrFlag( ACFlags nFlag, bool bOn = true );

    // Load, Set, Get - the replacement list
    SvxAutocorrWordList* LoadAutocorrWordList( LanguageType eLang )
        { return GetLanguageList_( eLang ).LoadAutocorrWordList(); }

    // Save word substitutions:
    //      Save these directly in the storage. The word list is updated
    //      accordingly!
    //  - pure Text
    bool PutText( const OUString& rShort, const OUString& rLong, LanguageType eLang );
    //  - Text with attribution (only in the SWG - SWG format!)
    void PutText( const OUString& rShort, SfxObjectShell& rShell, LanguageType eLang )
        { GetLanguageList_( eLang ).PutText(rShort, rShell ); }

    void MakeCombinedChanges( std::vector<SvxAutocorrWord>& aNewEntries,
                                  std::vector<SvxAutocorrWord>& aDeleteEntries,
                                  LanguageType eLang );

    // Load, Set, Get - the exception list for capital letters at the
    // beginning of a sentence
    void SaveCplSttExceptList( LanguageType eLang );
    SvStringsISortDtor* LoadCplSttExceptList(LanguageType eLang)
        {   return GetLanguageList_( eLang ).LoadCplSttExceptList(); }
    const SvStringsISortDtor* GetCplSttExceptList( LanguageType eLang )
        {   return GetLanguageList_( eLang ).GetCplSttExceptList(); }

    // Adds a single word. The list will be immediately written to the file!
    bool AddCplSttException( const OUString& rNew, LanguageType eLang );

    // Load, Set, Get the exception list for TWo INitial CApitals or sMALL iNITIAL
    void SaveWordStartExceptList( LanguageType eLang );
    SvStringsISortDtor* LoadWordStartExceptList( LanguageType eLang )
        {   return GetLanguageList_( eLang ).LoadWordStartExceptList(); }
    const SvStringsISortDtor* GetWordStartExceptList( LanguageType eLang )
        {   return GetLanguageList_( eLang ).GetWordStartExceptList(); }
    // Adds a single word. The list will be immediately written to the file!
    bool AddWordStartException( const OUString& rNew, LanguageType eLang);

    // Search through the Languages for the entry
    bool FindInWordStartExceptList( LanguageType eLang, const OUString& sWord );
    bool FindInCplSttExceptList( LanguageType eLang, const OUString& sWord,
                                    bool bAbbreviation = false);

    // Methods for the auto-correction
    void FnCapitalStartWord( SvxAutoCorrDoc&, const OUString&,
                                sal_Int32 nSttPos, sal_Int32 nEndPos,
                                LanguageType eLang );
    bool FnChgOrdinalNumber( SvxAutoCorrDoc&, const OUString&,
                                sal_Int32 nSttPos, sal_Int32 nEndPos,
                                LanguageType eLang );
    bool FnChgToEnEmDash( SvxAutoCorrDoc&, const OUString&,
                                sal_Int32 nSttPos, sal_Int32 nEndPos,
                                LanguageType eLang );
    bool FnAddNonBrkSpace( SvxAutoCorrDoc&, std::u16string_view,
                                sal_Int32 nEndPos,
                                LanguageType eLang, bool& io_bNbspRunNext );
    bool FnSetINetAttr( SvxAutoCorrDoc&, const OUString&,
                                sal_Int32 nSttPos, sal_Int32 nEndPos,
                                LanguageType eLang );
    bool FnChgWeightUnderl( SvxAutoCorrDoc&, const OUString&,
                                sal_Int32 nEndPos );
    void FnCapitalStartSentence( SvxAutoCorrDoc&, const OUString&, bool bNormalPos,
                                sal_Int32 nSttPos, sal_Int32 nEndPos,
                                LanguageType eLang);
    bool FnCorrectCapsLock( SvxAutoCorrDoc&, const OUString&,
                            sal_Int32 nSttPos, sal_Int32 nEndPos,
                            LanguageType eLang );

    static ACFlags  GetDefaultFlags();

// returns sal_True for characters where the function
// 'SvxAutoCorrect::AutoCorrect' should be called.
// (used to avoid occasional 'collisions' with (Thai) input-sequence-checking)
    static bool     IsAutoCorrectChar( sal_Unicode cChar );

    static bool NeedsHardspaceAutocorr( sal_Unicode cChar );

    CharClass& GetCharClass( LanguageType eLang )
    {
        if( !pCharClass || eLang != eCharClassLang )
            GetCharClass_( eLang );
        return *pCharClass;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

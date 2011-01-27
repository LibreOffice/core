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

// Flags fuer die AutoKorrekt-Flags
const long CptlSttSntnc     = 0x00000001;   // Gross-Buchstaben am SatzAnfang
const long CptlSttWrd       = 0x00000002;   // keine 2 Gr.-Buchst. am WordAnfang
const long AddNonBrkSpace   = 0x00000004;   // Add non breaking space before :;?!
const long ChgOrdinalNumber = 0x00000008;   // Ordinal-Number 1st, 2nd,..
const long ChgToEnEmDash    = 0x00000010;   // - -> Endash/Emdash
const long ChgWeightUnderl  = 0x00000020;   // * -> Fett, _ -> unterstreichen
const long SetINetAttr      = 0x00000040;   // INetAttribut setzen
const long Autocorrect      = 0x00000080;   // Autokorrektur aufrufen
const long ChgQuotes        = 0x00000100;   // doppelte Quotes ersetzen
const long SaveWordCplSttLst= 0x00000200;   // GrB. am SatzAnf. auto. aufnehmen
const long SaveWordWrdSttLst= 0x00000400;   // 2 GrB. am WortAnf. auto. aufnehmen
const long IgnoreDoubleSpace= 0x00000800;   // 2 Spaces ignorieren
const long ChgSglQuotes     = 0x00001000;   // einfache Quotes ersetzen
const long CorrectCapsLock  = 0x00002000;   // Correct accidental use of cAPS LOCK key

const long ChgWordLstLoad   = 0x20000000;   // Ersetzungsliste geladen
const long CplSttLstLoad    = 0x40000000;   // Exceptionlist fuer CplStart geladen
const long WrdSttLstLoad    = 0x80000000;   // Exceptionlist fuer WordStart geladen

// TODO: handle unicodes > U+FFFF and check users of this class

// nur eine Mappingklasse
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

    // returne den Text eines vorherigen Absatzes.
    // Dieser darf nicht leer sein!
    // Gibt es diesen nicht oder gibt es davor nur Leere, dann returne 0
    // Das Flag gibt an:
    //      TRUE: den, vor der normalen Einfuegeposition (TRUE)
    //      FALSE: den, in den das korrigierte Wort eingfuegt wurde.
    //              (Muss nicht der gleiche Absatz sein!!!!)
    virtual const String* GetPrevPara( BOOL bAtNormalPos ) = 0;

    virtual BOOL ChgAutoCorrWord( xub_StrLen& rSttPos, xub_StrLen nEndPos,
                                  SvxAutoCorrect& rACorrect,
                                  const String** ppPara ) = 0;
    // wird nach dem austauschen der Zeichen von den Funktionen
    //  - FnCptlSttWrd
    //  - FnCptlSttSntnc
    // gerufen. Dann koennen die Worte ggfs. in die Ausnahmelisten
    // aufgenommen werden.
    virtual void SaveCpltSttWord( ULONG nFlag, xub_StrLen nPos,
                                    const String& rExceptWord,
                                    sal_Unicode cChar );

    // welche Sprache gilt an der Position?
    virtual LanguageType GetLanguage( xub_StrLen nPos, BOOL bPrevPara = FALSE ) const;
};


class EDITENG_DLLPUBLIC SvxAutocorrWord
{
    String sShort, sLong;
    BOOL bIsTxtOnly;                // ist reiner ASCII - Text
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
    // falls die AutoCorr Datei neuer ist
    Date aModifiedDate;
    Time aModifiedTime, aLastCheckTime;

    LanguageType            eLanguage; //LANGUAGE_DONTKNOW fuer alle Sprachen verwenden
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

    // Lade, Setze, Gebe - die Ersetzungsliste
    SvxAutocorrWordList* LoadAutocorrWordList();
    void SetAutocorrWordList( SvxAutocorrWordList* pList );
    const SvxAutocorrWordList* GetAutocorrWordList();

    // Lade, Setze, Gebe - die Ausnahmeliste fuer Grossbuchstabe am
    // Satzanfang
    SvStringsISortDtor* LoadCplSttExceptList();
    void SaveCplSttExceptList();
    void SetCplSttExceptList( SvStringsISortDtor* pList );
    SvStringsISortDtor* GetCplSttExceptList();
    BOOL AddToCplSttExceptList(const String& rNew);

    // Lade, Setze, Gebe die Ausnahmeliste fuer 2 Grossbuchstaben am
    // Wortanfang
    SvStringsISortDtor* LoadWrdSttExceptList();
    void SaveWrdSttExceptList();
    void SetWrdSttExceptList( SvStringsISortDtor* pList );
    SvStringsISortDtor* GetWrdSttExceptList();
    BOOL AddToWrdSttExceptList(const String& rNew);

    // Speichern von Wortersetzungen:
    //      Diese speichern direkt im Storage. Die Wortliste wird
    //      entsprechend aktualisiert!
    //  - purer Text
    BOOL PutText( const String& rShort, const String& rLong );
    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
    BOOL PutText( const String& rShort, SfxObjectShell& );
    //  - loesche einen Eintrag
    BOOL DeleteText( const String& rShort );
};


class EDITENG_DLLPUBLIC SvxAutoCorrect
{
    friend class SvxAutoCorrectLanguageLists;

    String sShareAutoCorrFile, sUserAutoCorrFile;

    SvxSwAutoFmtFlags aSwFlags;     // StarWriter AutoFormat-Flags

    // alle Sprachen in einer Tabelle
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
    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
    //      rShort ist der Stream-Name - gecryptet!
    virtual BOOL PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const String& rFileName, const String& rShort, SfxObjectShell& ,
                            String& );

    // geforderte Sprache in die Tabelle eintragen gfs. nur wenn das file existiert
    BOOL    CreateLanguageFile(LanguageType eLang, BOOL bNewFile = TRUE);
    //  - return den Ersetzungstext (nur fuer SWG-Format, alle anderen
    //      koennen aus der Wortliste herausgeholt werden!)
    //      rShort ist der Stream-Name - gecryptet!
public:

    sal_Unicode GetQuote( sal_Unicode cInsChar, BOOL bSttQuote,
                            LanguageType eLang ) const;
    virtual BOOL GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const String& rFileName, const String& rShort, String& rLong );

    TYPEINFO();

    SvxAutoCorrect( const String& rShareAutocorrFile,
                    const String& rUserAutocorrFile );
    SvxAutoCorrect( const SvxAutoCorrect& );
    virtual ~SvxAutoCorrect();

    // fuehre eine AutoKorrektur aus.
    // returnt was ausgefuehrt wurde; entsprechend den obigen Flags
    ULONG AutoCorrect( SvxAutoCorrDoc& rDoc, const String& rTxt,
                        xub_StrLen nPos, sal_Unicode cInsChar, BOOL bInsert, Window* pFrameWin = NULL );

    // return fuer die Autotext Expandierung das vorherige Wort, was dem
    // AutoCorrect - Algorythmus entspricht.
    BOOL GetPrevAutoCorrWord( SvxAutoCorrDoc& rDoc, const String& rTxt,
                                xub_StrLen nPos, String& rWord ) const;

    // suche das oder die Worte in der ErsetzungsTabelle.
    // rText - ueberpruefe in diesem Text die Worte der Liste
    // rStt - die gefundene Startposition
    // nEnd - zu ueberpruefende Position - ab dieser Pos nach vorne
    // rLang - Input: in welcher Sprache wird gesucht
    //          Output: in welcher "Sprach-Liste" wurde es gefunden
    const SvxAutocorrWord* SearchWordsInList( const String& rTxt,
                                    xub_StrLen& rStt, xub_StrLen nEndPos,
                                    SvxAutoCorrDoc& rDoc,
                                    LanguageType& rLang );

    // erfrage / setze die Zeichen fuer die Quote-Ersetzung
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

    // erfrage / setze den Namen der AutoKorrektur-Datei
    // defaultet auf "autocorr.dat"
    String GetAutoCorrFileName( LanguageType eLang = LANGUAGE_SYSTEM,
                                BOOL bNewFile = FALSE,
                                BOOL bTstUserExist = FALSE ) const;
    void SetUserAutoCorrFileName( const String& rNew );
    void SetShareAutoCorrFileName( const String& rNew );

    // erfrage / setze die aktuellen Einstellungen der AutoKorrektur
    long GetFlags() const                       { return nFlags; }
    inline SvxSwAutoFmtFlags&   GetSwFlags()    { return aSwFlags;}
    BOOL IsAutoCorrFlag( long nFlag ) const
                                { return nFlags & nFlag ? TRUE : FALSE; }
    void SetAutoCorrFlag( long nFlag, BOOL bOn = TRUE );

    // Lade, Setze, Gebe - die Ersetzungsliste
    SvxAutocorrWordList* LoadAutocorrWordList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        { return _GetLanguageList( eLang ).LoadAutocorrWordList(); }
    const SvxAutocorrWordList* GetAutocorrWordList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        { return _GetLanguageList( eLang ).GetAutocorrWordList(); }

    // Speichern von Wortersetzungen:
    //      Diese speichern direkt im Storage. Die Wortliste wird
    //      entsprechend aktualisiert!
    //  - purer Text
    BOOL PutText( const String& rShort, const String& rLong, LanguageType eLang = LANGUAGE_SYSTEM );
    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
    BOOL PutText( const String& rShort, SfxObjectShell& rShell,
                 LanguageType eLang = LANGUAGE_SYSTEM )
        { return _GetLanguageList( eLang ).PutText(rShort, rShell ); }

    //  - loesche einen Eintrag
    BOOL DeleteText( const String& rShort, LanguageType eLang = LANGUAGE_SYSTEM);

    // Lade, Setze, Gebe - die Ausnahmeliste fuer Grossbuchstabe am
    // Satzanfang
    void SaveCplSttExceptList( LanguageType eLang = LANGUAGE_SYSTEM );
    SvStringsISortDtor* LoadCplSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM)
        {   return _GetLanguageList( eLang ).LoadCplSttExceptList(); }
    const SvStringsISortDtor* GetCplSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        {   return _GetLanguageList( eLang ).GetCplSttExceptList(); }

    // fuegt ein einzelnes Wort hinzu. Die Liste wird sofort
    // in die Datei geschrieben!
    BOOL AddCplSttException( const String& rNew,
                                LanguageType eLang = LANGUAGE_SYSTEM );

    // Lade, Setze, Gebe die Ausnahmeliste fuer 2 Grossbuchstaben am
    // Wortanfang
    void SaveWrdSttExceptList( LanguageType eLang = LANGUAGE_SYSTEM );
    SvStringsISortDtor* LoadWrdSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        {   return _GetLanguageList( eLang ).LoadWrdSttExceptList(); }
    const SvStringsISortDtor* GetWrdSttExceptList(
                                    LanguageType eLang = LANGUAGE_SYSTEM )
        {   return _GetLanguageList( eLang ).GetWrdSttExceptList(); }
    // fuegt ein einzelnes Wort hinzu. Die Liste wird sofort
    // in die Datei geschrieben!
    BOOL AddWrtSttException( const String& rNew, LanguageType eLang = LANGUAGE_SYSTEM);

    //ueber die Sprachen nach dem Eintrag suchen
    BOOL FindInWrdSttExceptList( LanguageType eLang, const String& sWord );
    BOOL FindInCplSttExceptList( LanguageType eLang, const String& sWord,
                                    BOOL bAbbreviation = FALSE);

    // die einzelnen Methoden fuer die Autokorrektur
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

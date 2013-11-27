/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_SPLWRAP_HXX
#define _SVX_SPLWRAP_HXX

// include ---------------------------------------------------------------

#include <editeng/svxenum.hxx>
#include <tools/string.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "editeng/editengdllapi.h"

// forward ---------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XDictionary;
    class XSpellChecker1;
    class XHyphenator;
}}}}

class Window;
//IAccessibility2 Impplementaton 2009-----
class SdrObject;
// misc functions ---------------------------------------------------------------

void EDITENG_DLLPUBLIC SvxPrepareAutoCorrect( String &rOldText, String &rNewText );

/*--------------------------------------------------------------------
     Beschreibung: Der SpellWrapper
 --------------------------------------------------------------------*/

class EDITENG_DLLPUBLIC SvxSpellWrapper {
private:
    friend class SvxSpellCheckDialog;
    friend class SvxHyphenWordDialog;
    friend class SvxHyphenWordDialog_Impl;

    Window*     pWin;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface >             xLast;  // result of last spelling/hyphenation attempt
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > xSpell;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    xHyph;
//IAccessibility2 Impplementaton 2009-----
    SdrObject* mpTextObj;
    sal_uInt16  nOldLang;       // Sprache merken, nur bei Aenderung SetLanguage rufen
    sal_Bool    bOtherCntnt : 1; // gesetzt => Sonderbereiche zunaechst pruefen
    sal_Bool    bDialog     : 1; // Ist pWin der Svx...Dialog?
    sal_Bool    bHyphen     : 1; // Trennen statt Spellen
    sal_Bool    bAuto       : 1; // Autokorrektur vorhanden?
    sal_Bool    bReverse    : 1; // Rueckwaerts Spellen
    sal_Bool    bStartDone  : 1; // Vorderen Teil bereits korrigiert
    sal_Bool    bEndDone    : 1; // Hinteren Teil bereits korrigiert
    sal_Bool    bStartChk   : 1; // Vorderen Teil pruefen
    sal_Bool    bRevAllowed : 1; // Niemals rueckwaerts spellen
    sal_Bool    bAllRight   : 1; // falsche Woerter in geignetes Woerterbuch
                             // aufnehmen und nicht den Dialog starten.

    EDITENG_DLLPRIVATE sal_Bool     SpellNext();        // naechsten Bereich anwaehlen
    sal_Bool    FindSpellError();   // Suche nach Fehlern ( ueber Bereiche hinweg )

public:
    SvxSpellWrapper( Window* pWn,
                     ::com::sun::star::uno::Reference<
                         ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpellChecker,
                     const sal_Bool bStart = sal_False, const sal_Bool bIsAllRight = sal_False,
                     const sal_Bool bOther = sal_False, const sal_Bool bRevAllow = sal_True );
    SvxSpellWrapper( Window* pWn,
                     ::com::sun::star::uno::Reference<
                         ::com::sun::star::linguistic2::XHyphenator >  &xHyphenator,
                     const sal_Bool bStart = sal_False, const sal_Bool bOther = sal_False );

    virtual ~SvxSpellWrapper();

    static sal_Int16    CheckSpellLang(
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::linguistic2::XSpellChecker1 >  xSpell,
                            sal_Int16 nLang );
    static sal_Int16    CheckHyphLang(
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::linguistic2::XHyphenator >  xHyph,
                            sal_Int16 nLang );

    static void         ShowLanguageErrors();

    void            SpellDocument();        // Rechtschreibpruefung durchfuehren
    inline sal_Bool IsStartDone(){ return bStartDone; }
    inline sal_Bool IsEndDone(){ return bEndDone; }
    inline sal_Bool IsReverse(){ return bReverse; }
    inline sal_Bool IsDialog(){ return bDialog; } // SvxSpellCheckDialog OnScreen
    inline sal_Bool IsHyphen(){ return bHyphen; } // Trennen statt Spellen
    inline void     SetHyphen( const sal_Bool bNew = sal_True ){ bHyphen = bNew; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                    GetXSpellChecker() { return xSpell; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >
                    GetXHyphenator()    { return xHyph; }
    inline sal_Bool             IsAllRight()        { return bAllRight; }
    inline Window*  GetWin() { return pWin; }
    // kann evtl entfallen in ONE_LINGU:
    inline void     SetOldLang( const sal_uInt16 nNew ){ nOldLang = nNew; }
    // kann evtl entfallen in ONE_LINGU:
    inline void     ChangeLanguage( const sal_uInt16 nNew ) // rufe ggf. SetLanguage
        { if ( nNew != nOldLang ) { SetLanguage( nNew ); nOldLang = nNew; } }
    inline void     EnableAutoCorrect() { bAuto = sal_True; }

protected:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface >
                     GetLast()      { return xLast; }
    void             SetLast(const ::com::sun::star::uno::Reference<
                                ::com::sun::star::uno::XInterface >  &xNewLast)
                            { xLast = xNewLast; }
    virtual sal_Bool SpellMore();               // weitere Dokumente pruefen?
    virtual sal_Bool HasOtherCnt();             // gibt es ueberhaupt Sonderbereiche
    virtual void     SpellStart( SvxSpellArea eSpell ); // Bereich vorbereiten
    virtual sal_Bool SpellContinue();           // Bereich pruefen
                                            // Ergebnis mit GetLast verfuegbar
    virtual void ReplaceAll( const String &rNewText, sal_Int16 nLanguage ); // Wort aus Replace-Liste ersetzen
    virtual void StartThesaurus( const String &rWord, sal_uInt16 nLang );   // Thesaurus starten
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >
                 GetAllRightDic() const;
    virtual void SpellEnd();                        // Bereich abschliessen
    virtual void ScrollArea();                      // ScrollArea einstellen
    // Wort ersetzen
    virtual void ChangeWord( const String& rNewWord, const sal_uInt16 nLang );
    virtual String GetThesWord();
    // Wort via Thesaurus ersetzen
    virtual void ChangeThesWord( const String& rNewWord );
    virtual void SetLanguage( const sal_uInt16 nLang ); // Sprache aendern
    virtual void AutoCorrect( const String& rAktStr, const String& rNewStr );
    virtual void InsertHyphen( const sal_uInt16 nPos ); // Hyphen einfuegen

//IAccessibility2 Impplementaton 2009-----
    void SetCurTextObj( SdrObject* pObj ) { mpTextObj = pObj; }
    SdrObject* GetCurTextObj() { return mpTextObj; }
//-----IAccessibility2 Impplementaton 2009
};

#endif

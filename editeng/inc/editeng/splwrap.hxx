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
#ifndef _SVX_SPLWRAP_HXX
#define _SVX_SPLWRAP_HXX

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

// misc functions ---------------------------------------------------------------

void EDITENG_DLLPUBLIC SvxPrepareAutoCorrect( String &rOldText, String &rNewText );

/*--------------------------------------------------------------------
     Description: The SpellWrapper
 --------------------------------------------------------------------*/

class EDITENG_DLLPUBLIC SvxSpellWrapper {
private:
    friend class SvxSpellCheckDialog;
    friend class SvxHyphenWordDialog;
    friend struct SvxHyphenWordDialog_Impl;

    Window*     pWin;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface >             xLast;  // result of last spelling/hyphenation attempt
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > xSpell;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    xHyph;
    sal_uInt16  nOldLang;        // Set Language, only call SetLanguage on changes
    sal_Bool    bOtherCntnt : 1; // set => Check special sections initially
    sal_Bool    bDialog     : 1; // Is pWin the Svx...Dialog?
    sal_Bool    bHyphen     : 1; // Split instead of spell checking
    sal_Bool    bAuto       : 1; // AutoCorrect available?
    sal_Bool    bReverse    : 1; // Reverse spell check
    sal_Bool    bStartDone  : 1; // Beginning already corrected
    sal_Bool    bEndDone    : 1; // End part already corrected
    sal_Bool    bStartChk   : 1; // Examine the beginning
    sal_Bool    bRevAllowed : 1; // Reverse spell check prohibited
    sal_Bool    bAllRight   : 1; // Record wrong words in the dedicated
                                 // dictionary and do not start the dialog.

    EDITENG_DLLPRIVATE sal_Bool     SpellNext();        // select next area
    sal_Bool    FindSpellError();   // Check for errors (over areas)

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

    void            SpellDocument();        // Perform Spell Checking
    inline sal_Bool IsStartDone(){ return bStartDone; }
    inline sal_Bool IsEndDone(){ return bEndDone; }
    inline sal_Bool IsReverse(){ return bReverse; }
    inline sal_Bool IsDialog(){ return bDialog; } // SvxSpellCheckDialog OnScreen
    inline sal_Bool IsHyphen(){ return bHyphen; } // Split instead of Spell check
    inline void     SetHyphen( const sal_Bool bNew = sal_True ){ bHyphen = bNew; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                    GetXSpellChecker() { return xSpell; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >
                    GetXHyphenator()    { return xHyph; }
    inline sal_Bool             IsAllRight()        { return bAllRight; }
    inline Window*  GetWin() { return pWin; }
    // can possibly be omitted in ONE_LINGU:
    inline void     SetOldLang( const sal_uInt16 nNew ){ nOldLang = nNew; }
    // can possibly be omitted in ONE_LINGU:
    inline void     ChangeLanguage( const sal_uInt16 nNew ) // call SetLanguage if needed.
        { if ( nNew != nOldLang ) { SetLanguage( nNew ); nOldLang = nNew; } }
    inline void     EnableAutoCorrect() { bAuto = sal_True; }

protected:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface >
                     GetLast()      { return xLast; }
    void             SetLast(const ::com::sun::star::uno::Reference<
                                ::com::sun::star::uno::XInterface >  &xNewLast)
                            { xLast = xNewLast; }
    virtual sal_Bool SpellMore();               // examine further documents?
    virtual sal_Bool HasOtherCnt();             // Are there any special areas?
    virtual void     SpellStart( SvxSpellArea eSpell ); // Preparing the area
    virtual sal_Bool SpellContinue();     // Check Areas
                                          // Result avaliable through GetLast
    virtual void ReplaceAll( const String &rNewText, sal_Int16 nLanguage ); //Replace word from the replace list
    virtual void StartThesaurus( const String &rWord, sal_uInt16 nLang );
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >
                 GetAllRightDic() const;
    virtual void SpellEnd();                        // Finish area
    virtual void ScrollArea();                      // Set ScrollArea
    // Replace word
    virtual void ChangeWord( const String& rNewWord, const sal_uInt16 nLang );
    virtual String GetThesWord();
    // Wort via Thesaurus ersetzen
    virtual void ChangeThesWord( const String& rNewWord );
    virtual void SetLanguage( const sal_uInt16 nLang ); // Change Language
    virtual void AutoCorrect( const String& rAktStr, const String& rNewStr );
    virtual void InsertHyphen( const sal_uInt16 nPos ); // Insert hyphen

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

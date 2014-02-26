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
#ifndef INCLUDED_EDITENG_SPLWRAP_HXX
#define INCLUDED_EDITENG_SPLWRAP_HXX

#include <editeng/editengdllapi.h>
#include <editeng/svxenum.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>

// forward ---------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XDictionary;
    class XSpellChecker1;
    class XHyphenator;
}}}}

class Window;
class SdrObject;
// misc functions ---------------------------------------------------------------

void EDITENG_DLLPUBLIC SvxPrepareAutoCorrect( OUString &rOldText, const OUString &rNewText );

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
    SdrObject*  mpTextObj;
    bool        bOtherCntnt : 1; // set => Check special sections initially
    bool        bDialog     : 1; // Is pWin the Svx...Dialog?
    bool        bHyphen     : 1; // Split instead of spell checking
    bool        bAuto       : 1; // AutoCorrect available?
    bool        bReverse    : 1; // Reverse spell check
    bool        bStartDone  : 1; // Beginning already corrected
    bool        bEndDone    : 1; // End part already corrected
    bool        bStartChk   : 1; // Examine the beginning
    bool        bRevAllowed : 1; // Reverse spell check prohibited
    bool        bAllRight   : 1; // Record wrong words in the dedicated
                                 // dictionary and do not start the dialog.

    EDITENG_DLLPRIVATE bool     SpellNext();        // select next area
    bool    FindSpellError();   // Check for errors (over areas)

public:
    SvxSpellWrapper( Window* pWn,
                     ::com::sun::star::uno::Reference<
                         ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpellChecker,
                     const bool bStart = false, const bool bIsAllRight = false,
                     const bool bOther = false, const bool bRevAllow = true );
    SvxSpellWrapper( Window* pWn,
                     ::com::sun::star::uno::Reference<
                         ::com::sun::star::linguistic2::XHyphenator >  &xHyphenator,
                     const bool bStart = false, const bool bOther = false );

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
    inline bool     IsStartDone(){ return bStartDone; }
    inline bool     IsEndDone(){ return bEndDone; }
    inline bool     IsReverse(){ return bReverse; }
    inline bool     IsDialog(){ return bDialog; } // SvxSpellCheckDialog OnScreen
    inline bool     IsHyphen(){ return bHyphen; } // Split instead of Spell check
    inline void     SetHyphen( const bool bNew = true ){ bHyphen = bNew; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                    GetXSpellChecker() { return xSpell; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >
                    GetXHyphenator()    { return xHyph; }
    inline bool     IsAllRight()        { return bAllRight; }
    inline Window*  GetWin() { return pWin; }
    inline void     EnableAutoCorrect() { bAuto = true; }

protected:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface >
                     GetLast()      { return xLast; }
    void             SetLast(const ::com::sun::star::uno::Reference<
                                ::com::sun::star::uno::XInterface >  &xNewLast)
                            { xLast = xNewLast; }
    virtual bool SpellMore();               // examine further documents?
    virtual bool HasOtherCnt();             // Are there any special areas?
    virtual void SpellStart( SvxSpellArea eSpell ); // Preparing the area
    virtual bool SpellContinue();     // Check Areas
                                          // Result available through GetLast
    virtual void ReplaceAll( const OUString &rNewText, sal_Int16 nLanguage ); //Replace word from the replace list
    virtual void StartThesaurus( const OUString &rWord, sal_uInt16 nLang );
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >
                 GetAllRightDic() const;
    virtual void SpellEnd();                        // Finish area
    virtual void ScrollArea();                      // Set ScrollArea
    // Replace word
    virtual void ChangeWord( const OUString& rNewWord, const sal_uInt16 nLang );
    // Wort via Thesaurus ersetzen
    virtual void ChangeThesWord( const OUString& rNewWord );
    virtual void SetLanguage( const sal_uInt16 nLang ); // Change Language
    virtual void AutoCorrect( const OUString& rAktStr, const OUString& rNewStr );
    virtual void InsertHyphen( const sal_uInt16 nPos ); // Insert hyphen

    void SetCurTextObj( SdrObject* pObj ) { mpTextObj = pObj; }
    SdrObject* GetCurTextObj() { return mpTextObj; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <vcl/vclptr.hxx>

// forward ---------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XDictionary;
    class XSpellChecker1;
    class XHyphenator;
}}}}

namespace vcl { class Window; }
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

    VclPtr<vcl::Window>     pWin;
    css::uno::Reference<
        css::uno::XInterface >             xLast;  // result of last spelling/hyphenation attempt
    css::uno::Reference<
        css::linguistic2::XSpellChecker1 > xSpell;
    css::uno::Reference<
        css::linguistic2::XHyphenator >    xHyph;
    SdrObject*  mpTextObj;
    bool        bOtherCntnt : 1; // set => Check special sections initially
    bool        bDialog     : 1; // Is pWin the Svx...Dialog?
    bool        bHyphen     : 1; // Split instead of spell checking
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
    SvxSpellWrapper( vcl::Window* pWn,
                     css::uno::Reference< css::linguistic2::XSpellChecker1 >  &xSpellChecker,
                     const bool bStart = false, const bool bIsAllRight = false,
                     const bool bOther = false, const bool bRevAllow = true );
    SvxSpellWrapper( vcl::Window* pWn,
                     css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphenator,
                     const bool bStart = false, const bool bOther = false );

    virtual ~SvxSpellWrapper();

    static sal_Int16    CheckSpellLang(
                            css::uno::Reference< css::linguistic2::XSpellChecker1 >  xSpell,
                            sal_Int16 nLang );
    static sal_Int16    CheckHyphLang(
                            css::uno::Reference< css::linguistic2::XHyphenator >  xHyph,
                            sal_Int16 nLang );

    static void         ShowLanguageErrors();

    void            SpellDocument();        // Perform Spell Checking
    inline bool     IsStartDone(){ return bStartDone; }
    inline bool     IsEndDone(){ return bEndDone; }
    inline bool     IsHyphen(){ return bHyphen; } // Split instead of Spell check
    inline void     SetHyphen() { bHyphen = true; }
    inline bool     IsAllRight()        { return bAllRight; }

protected:
    css::uno::Reference< css::uno::XInterface >
                     GetLast()      { return xLast; }
    void             SetLast(const css::uno::Reference< css::uno::XInterface >  &xNewLast)
                            { xLast = xNewLast; }
    virtual bool SpellMore();               // examine further documents?
    virtual bool HasOtherCnt();             // Are there any special areas?
    virtual void SpellStart( SvxSpellArea eSpell ); // Preparing the area
    virtual void SpellContinue();     // Check Areas
                                          // Result available through GetLast
    virtual void ReplaceAll( const OUString &rNewText, sal_Int16 nLanguage ); //Replace word from the replace list
    static css::uno::Reference< css::linguistic2::XDictionary >
                 GetAllRightDic();
    virtual void SpellEnd();                        // Finish area
    virtual void InsertHyphen( const sal_Int32 nPos ); // Insert hyphen

    void SetCurTextObj( SdrObject* pObj ) { mpTextObj = pObj; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

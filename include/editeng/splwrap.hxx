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
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <vcl/weld.hxx>

// forward ---------------------------------------------------------------

namespace com::sun::star::linguistic2 {
    class XDictionary;
    class XSpellChecker1;
    class XHyphenator;
}

// misc functions ---------------------------------------------------------------

void EDITENG_DLLPUBLIC SvxPrepareAutoCorrect( OUString &rOldText, std::u16string_view rNewText );

/*--------------------------------------------------------------------
     Description: The SpellWrapper
 --------------------------------------------------------------------*/

class EDITENG_DLLPUBLIC SvxSpellWrapper {
private:
    friend class SvxSpellCheckDialog;
    friend class SvxHyphenWordDialog;
    friend struct SvxHyphenWordDialog_Impl;

    weld::Widget* pWin;
    std::unique_ptr<weld::WaitObject> xWait;
    css::uno::Reference<
        css::uno::XInterface >             xLast;  // result of last spelling/hyphenation attempt
    css::uno::Reference<
        css::linguistic2::XHyphenator >    xHyph;
    bool        bOtherCntnt : 1; // set => Check special sections initially
    bool        bReverse    : 1; // Reverse spell check
    bool        bStartDone  : 1; // Beginning already corrected
    bool        bEndDone    : 1; // End part already corrected
    bool        bStartChk   : 1; // Examine the beginning
    bool        bRevAllowed : 1; // Reverse spell check prohibited
    bool        bAllRight   : 1; // Record wrong words in the dedicated
                                 // dictionary and do not start the dialog.

    SAL_DLLPRIVATE bool     SpellNext();        // select next area
    bool    FindSpellError();   // Check for errors (over areas)

    SvxSpellWrapper(SvxSpellWrapper const &) = delete;
    void operator =(SvxSpellWrapper const &) = delete;

public:
    SvxSpellWrapper( weld::Widget* pWn,
                     const bool bStart, const bool bIsAllRight );
    SvxSpellWrapper( weld::Widget* pWn,
                     css::uno::Reference< css::linguistic2::XHyphenator > const &xHyphenator,
                     const bool bStart, const bool bOther );

    virtual ~SvxSpellWrapper();

    static sal_Int16    CheckSpellLang(
                            css::uno::Reference< css::linguistic2::XSpellChecker1 > const & xSpell,
                            LanguageType nLang );
    static sal_Int16    CheckHyphLang(
                            css::uno::Reference< css::linguistic2::XHyphenator >const & xHyph,
                            LanguageType nLang );

    static void         ShowLanguageErrors();

    void            SpellDocument();        // Perform Spell Checking
    bool     IsStartDone() const { return bStartDone; }
    bool     IsEndDone() const { return bEndDone; }
    bool     IsAllRight() const { return bAllRight; }

protected:
    const css::uno::Reference< css::uno::XInterface >&
                     GetLast() const { return xLast; }
    void             SetLast(const css::uno::Reference< css::uno::XInterface >  &xNewLast)
                            { xLast = xNewLast; }
    virtual bool SpellMore();               // examine further documents?
    virtual void SpellStart( SvxSpellArea eSpell ); // Preparing the area
    virtual void SpellContinue();     // Check Areas
                                          // Result available through GetLast
    virtual void ReplaceAll( const OUString &rNewText ); //Replace word from the replace list
    static css::uno::Reference< css::linguistic2::XDictionary >
                 GetAllRightDic();
    virtual void SpellEnd();                        // Finish area
    virtual void InsertHyphen( const sal_Int32 nPos ); // Insert hyphen
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

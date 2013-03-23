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
#pragma once
#if 1

#include <editeng/splwrap.hxx>
#include <editeng/svxacorr.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <editeng/hangulhanja.hxx>

class EditView;

class TextConvWrapper : public editeng::HangulHanjaConversion
{
    rtl::OUString   aConvText;      // convertible text part found last time
    LanguageType    nConvTextLang;  // language of aConvText
    sal_uInt16          nLastPos;       // starting position of the last found text portion (word)
    sal_uInt16          nUnitOffset;    // offset of current unit in the current text portion (word)

    ESelection      aConvSel;       // selection to be converted if
                                    // 'HasRange' is true, other conversion
                                    // starts from the cursor position

    EditView *      pEditView;
    Window *        pWin;

    sal_Bool        bStartChk;
    sal_Bool        bStartDone;
    sal_Bool        bEndDone;
    sal_Bool        bAllowChange;   // storage for _bAllowImplicitChangesForNotConvertibleText
                                    // parameters value of function GetNextPortion.
                                    // used to transport the value to where it is needed.


    // from SvxSpellWrapper copied and modified
    sal_Bool    ConvNext_impl();        // former SpellNext
    sal_Bool    FindConvText_impl();    // former FindSpellError
    sal_Bool    ConvMore_impl();        // former SpellMore

    // from EditSpellWrapper copied and modified
    void        ConvStart_impl( SvxSpellArea eSpell );   // former SpellStart
    void        ConvEnd_impl();                          // former SpellEnd
    sal_Bool    ConvContinue_impl();                     // former SpellContinue

    void        SelectNewUnit_impl( const sal_Int32 nUnitStart,
                                    const sal_Int32 nUnitEnd );

    void        ChangeText( const String &rNewText,
                            const ::rtl::OUString& rOrigText,
                            const ::com::sun::star::uno::Sequence< sal_Int32 > *pOffsets,
                            ESelection *pESelection );
    void        ChangeText_impl( const String &rNewText, sal_Bool bKeepAttributes );

    // Forbidden and not implemented.
    TextConvWrapper (const TextConvWrapper &);
    TextConvWrapper & operator= (const TextConvWrapper &);

protected:
    virtual void    GetNextPortion( ::rtl::OUString& /* [out] */ rNextPortion,
                        LanguageType& /* [out] */ rLangOfPortion,
                        sal_Bool /* [in] */ _bAllowImplicitChangesForNotConvertibleText );
    virtual void    HandleNewUnit( const sal_Int32 nUnitStart,
                                   const sal_Int32 nUnitEnd );
    virtual void    ReplaceUnit(
                        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
                        const ::rtl::OUString& rOrigText,
                        const ::rtl::OUString& rReplaceWith,
                        const ::com::sun::star::uno::Sequence< sal_Int32 > &rOffsets,
                        ReplacementAction eAction,
                        LanguageType *pNewUnitLanguage );

    virtual sal_Bool    HasRubySupport() const;

    void SetLanguageAndFont( const ESelection &rESel,
                            LanguageType nLang, sal_uInt16 nLangWhichId,
                            const Font *pFont,  sal_uInt16 nFontWhichId );


public:
    TextConvWrapper( Window* pWindow,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const ::com::sun::star::lang::Locale& rSourceLocale,
            const ::com::sun::star::lang::Locale& rTargetLocale,
            const Font* pTargetFont,
            sal_Int32 nOptions,
            sal_Bool bIsInteractive,
            sal_Bool bIsStart, EditView* pView );

    virtual ~TextConvWrapper();

    void    Convert();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

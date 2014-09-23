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
#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_TEXTCONV_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_TEXTCONV_HXX

#include <editeng/splwrap.hxx>
#include <editeng/svxacorr.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <editeng/hangulhanja.hxx>

class EditView;

class TextConvWrapper : public editeng::HangulHanjaConversion
{
    OUString        m_aConvText;      // convertible text part found last time
    LanguageType    m_nConvTextLang;  // language of aConvText
    sal_uInt16      m_nUnitOffset;    // offset of current unit in the current text portion (word)
    sal_uInt16      m_nLastPos;       // starting position of the last found text portion (word)

    ESelection      m_aConvSel;       // selection to be converted if
                                      // 'HasRange' is true, other conversion
                                      // starts from the cursor position

    EditView *      m_pEditView;
    vcl::Window *        m_pWin;

    bool            m_bStartChk;
    bool            m_bStartDone;
    bool            m_bEndDone;
    bool            m_bAllowChange;   // storage for _bAllowImplicitChangesForNotConvertibleText
                                      // parameters value of function GetNextPortion.
                                      // used to transport the value to where it is needed.


    // from SvxSpellWrapper copied and modified
    bool        ConvNext_impl();        // former SpellNext
    bool        FindConvText_impl();    // former FindSpellError
    bool        ConvMore_impl();        // former SpellMore

    // from EditSpellWrapper copied and modified
    void        ConvStart_impl( SvxSpellArea eSpell );   // former SpellStart
    void        ConvEnd_impl();                          // former SpellEnd
    bool        ConvContinue_impl();                     // former SpellContinue

    void        SelectNewUnit_impl( const sal_Int32 nUnitStart,
                                    const sal_Int32 nUnitEnd );

    void        ChangeText( const OUString &rNewText,
                            const OUString& rOrigText,
                            const ::com::sun::star::uno::Sequence< sal_Int32 > *pOffsets,
                            ESelection *pESelection );
    void        ChangeText_impl( const OUString &rNewText, bool bKeepAttributes );

    // Forbidden and not implemented.
    TextConvWrapper (const TextConvWrapper &);
    TextConvWrapper & operator= (const TextConvWrapper &);

protected:
    virtual void    GetNextPortion( OUString& /* [out] */ rNextPortion,
                        LanguageType& /* [out] */ rLangOfPortion,
                        bool /* [in] */ _bAllowImplicitChangesForNotConvertibleText ) SAL_OVERRIDE;
    virtual void    HandleNewUnit( const sal_Int32 nUnitStart,
                                   const sal_Int32 nUnitEnd ) SAL_OVERRIDE;
    virtual void    ReplaceUnit(
                        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
                        const OUString& rOrigText,
                        const OUString& rReplaceWith,
                        const ::com::sun::star::uno::Sequence< sal_Int32 > &rOffsets,
                        ReplacementAction eAction,
                        LanguageType *pNewUnitLanguage ) SAL_OVERRIDE;

    virtual bool    HasRubySupport() const SAL_OVERRIDE;

    void SetLanguageAndFont( const ESelection &rESel,
                            LanguageType nLang, sal_uInt16 nLangWhichId,
                            const vcl::Font *pFont,  sal_uInt16 nFontWhichId );


public:
    TextConvWrapper( vcl::Window* pWindow,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const ::com::sun::star::lang::Locale& rSourceLocale,
            const ::com::sun::star::lang::Locale& rTargetLocale,
            const vcl::Font* pTargetFont,
            sal_Int32 nOptions,
            bool bIsInteractive,
            bool bIsStart, EditView* pView );

    virtual ~TextConvWrapper();

    void    Convert();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

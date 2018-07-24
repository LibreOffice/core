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
#ifndef INCLUDED_SW_INC_HHCWRP_HXX
#define INCLUDED_SW_INC_HHCWRP_HXX

#include <editeng/hangulhanja.hxx>
#include "swdllapi.h"

class SwView;
namespace vcl { class Window; }
class SwWrtShell;
struct SwConversionArgs;
class SwPaM;

class SW_DLLPUBLIC SwHHCWrapper : public editeng::HangulHanjaConversion
{
    SwView *    m_pView;
    VclPtr<vcl::Window>     m_pWin;
    SwWrtShell &m_rWrtShell;

    std::unique_ptr<SwConversionArgs> m_pConvArgs;    /**< object for arguments (and results) needed
                                                       to find of next convertible text portion */

    sal_Int32       m_nLastPos;       /**< starting position of the last found text part
                                       (needs to be sth that gets not moved like
                                       SwPaM or SwPosition by replace operations!) */
    sal_Int32       m_nUnitOffset;

    sal_uInt16      m_nPageCount;     ///< page count for progress bar
    sal_uInt16      m_nPageStart;     ///< first checked page

    bool        m_bIsDrawObj;
    bool        m_bIsOtherContent;
    bool        m_bStartChk;
    bool        m_bIsSelection;       ///< true if only the selected text should be converted
    bool        m_bStartDone;
    bool        m_bEndDone;

    /// from SvxSpellWrapper copied and modified
    bool        ConvNext_impl();        ///< former SpellNext
    void        FindConvText_impl();    ///< former FindSpellError

    void        ConvStart_impl( SwConversionArgs *pConvArgs, SvxSpellArea eSpell );   ///< former SpellStart
    void        ConvEnd_impl( SwConversionArgs const *pConvArgs );                          ///< former SpellEnd
    bool        ConvContinue_impl( SwConversionArgs *pConvArgs );                     ///< former SpellContinue

    void        SelectNewUnit_impl( const sal_Int32 nUnitStart,
                                    const sal_Int32 nUnitEnd );
    void        ChangeText( const OUString &rNewText,
                            const OUString& rOrigText,
                            const css::uno::Sequence< sal_Int32 > *pOffsets,
                            SwPaM *pCursor );
    void        ChangeText_impl( const OUString &rNewText, bool bKeepAttributes );

protected:
    virtual void    GetNextPortion( OUString& rNextPortion,
                            LanguageType& rLangOfPortion,
                            bool bAllowImplicitChangesForNotConvertibleText ) override;
    virtual void    HandleNewUnit( const sal_Int32 nUnitStart,
                                   const sal_Int32 nUnitEnd ) override;
    virtual void    ReplaceUnit(
                        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
                        const OUString& rOrigText,
                        const OUString& rReplaceWith,
                        const css::uno::Sequence< sal_Int32 > &rOffsets,
                        ReplacementAction eAction,
                        LanguageType *pNewUnitLanguage ) override;

    virtual bool    HasRubySupport() const override;

public:
    SwHHCWrapper(
        SwView* pView,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        LanguageType nSourceLanguage, LanguageType nTargetLanguage,
        const vcl::Font *pTargetFont,
        sal_Int32 nConvOptions, bool bIsInteractive,
        bool bStart, bool bOther, bool bSelection );

    virtual ~SwHHCWrapper() override;

    void    Convert();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

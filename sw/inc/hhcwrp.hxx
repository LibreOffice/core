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
#ifndef _HHCWRP_HXX
#define _HHCWRP_HXX

#include <editeng/hangulhanja.hxx>

class SwView;
class Window;
class SwWrtShell;
struct SwConversionArgs;
class SwPaM;

//////////////////////////////////////////////////////////////////////

class SwHHCWrapper : public editeng::HangulHanjaConversion
{
    SwView *    m_pView;
    Window*     m_pWin;
    SwWrtShell &m_rWrtShell;

    SwConversionArgs *m_pConvArgs;    /**< object for arguments (and results) needed
                                       to find of next convertible text portion */

    xub_StrLen      m_nLastPos;       /**< starting position of the last found text part
                                       (needs to be sth that gets not moved like
                                       SwPaM or SwPosition by replace operations!) */
    sal_Int32       m_nUnitOffset;

    sal_uInt16      m_nPageCount;     ///< page count for progress bar
    sal_uInt16      m_nPageStart;     ///< first checked page

    sal_Bool    m_bIsDrawObj;
    sal_Bool    m_bIsOtherCntnt;
    sal_Bool    m_bStartChk;
    sal_Bool    m_bIsSelection;       ///< true if only the selected text should be converted
    sal_Bool    m_bStartDone;
    sal_Bool    m_bEndDone;

    /// from SvxSpellWrapper copied and modified
    sal_Bool    ConvNext_impl();        ///< former SpellNext
    sal_Bool    FindConvText_impl();    ///< former FindSpellError

    /// from SwSpellWrapper copied and modified
    sal_Bool    HasOtherCnt_impl();
    void        ConvStart_impl( SwConversionArgs *pConvArgs, SvxSpellArea eSpell );   ///< former SpellStart
    void        ConvEnd_impl( SwConversionArgs *pConvArgs );                          ///< former SpellEnd
    sal_Bool    ConvContinue_impl( SwConversionArgs *pConvArgs );                     ///< former SpellContinue

    void        SelectNewUnit_impl( const sal_Int32 nUnitStart,
                                    const sal_Int32 nUnitEnd );
    void        ChangeText( const String &rNewText,
                            const OUString& rOrigText,
                            const ::com::sun::star::uno::Sequence< sal_Int32 > *pOffsets,
                            SwPaM *pCrsr );
    void        ChangeText_impl( const String &rNewText, sal_Bool bKeepAttributes );

    inline sal_Bool IsDrawObj()             { return m_bIsDrawObj; }
    inline void SetDrawObj( sal_Bool bNew ) { m_bIsDrawObj = bNew; }

protected:
    virtual void    GetNextPortion( OUString& rNextPortion,
                            LanguageType& rLangOfPortion,
                            sal_Bool bAllowImplicitChangesForNotConvertibleText );
    virtual void    HandleNewUnit( const sal_Int32 nUnitStart,
                                   const sal_Int32 nUnitEnd );
    virtual void    ReplaceUnit(
                        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
                        const OUString& rOrigText,
                        const OUString& rReplaceWith,
                        const ::com::sun::star::uno::Sequence< sal_Int32 > &rOffsets,
                        ReplacementAction eAction,
                        LanguageType *pNewUnitLanguage );

    virtual sal_Bool    HasRubySupport() const;

public:
    SwHHCWrapper(
        SwView* pView,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
        LanguageType nSourceLanguage, LanguageType nTargetLanguage,
        const Font *pTargetFont,
        sal_Int32 nConvOptions, sal_Bool bIsInteractive,
        sal_Bool bStart, sal_Bool bOther, sal_Bool bSelection );

    virtual ~SwHHCWrapper();

    void    Convert();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef _HHCWRP_HXX
#define _HHCWRP_HXX

#include <editeng/hangulhanja.hxx>
#include <pam.hxx>

class SwView;
class Window;
class SwWrtShell;
struct SwConversionArgs;

//////////////////////////////////////////////////////////////////////

class SwHHCWrapper : public editeng::HangulHanjaConversion
{
    SwView *    pView;
    Window*     pWin;
    SwWrtShell &rWrtShell;

    SwConversionArgs *pConvArgs;    // object for arguments (and results) needed
                                    // to find of next convertible text portion

    xub_StrLen      nLastPos;   // starting position of the last found text part
                                // (needs to be sth that gets not moved like
                                // SwPaM or SwPosition by replace operations!)
    sal_Int32       nUnitOffset;

    sal_uInt16      nPageCount;     // page count for progress bar
    sal_uInt16      nPageStart;     // first checked page

    sal_Bool    bIsDrawObj;
    sal_Bool    bIsStart;
    sal_Bool    bIsOtherCntnt;
    sal_Bool    bStartChk;
    sal_Bool    bIsSelection;   // true if only the selected text should be converted
    sal_Bool    bInfoBox;       // true if message should be displayed at the end
    sal_Bool    bIsConvSpecial; // true if special regions: header, footer, ... should be converted
    sal_Bool    bStartDone;
    sal_Bool    bEndDone;
//    sal_Bool    bLastRet;

    // from SvxSpellWrapper copied and modified
    sal_Bool    ConvNext_impl();        // former SpellNext
    sal_Bool    FindConvText_impl();    // former FindSpellError

    // from SwSpellWrapper copied and modified
    sal_Bool    HasOtherCnt_impl();
    void        ConvStart_impl( SwConversionArgs *pConvArgs, SvxSpellArea eSpell );   // former SpellStart
    void        ConvEnd_impl( SwConversionArgs *pConvArgs );                          // former SpellEnd
    sal_Bool    ConvContinue_impl( SwConversionArgs *pConvArgs );                     // former SpellContinue

    void        SelectNewUnit_impl( const sal_Int32 nUnitStart,
                                    const sal_Int32 nUnitEnd );
    void        ChangeText( const String &rNewText,
                            const ::rtl::OUString& rOrigText,
                            const ::com::sun::star::uno::Sequence< sal_Int32 > *pOffsets,
                            SwPaM *pCrsr );
    void        ChangeText_impl( const String &rNewText, sal_Bool bKeepAttributes );

    inline sal_Bool IsDrawObj()             { return bIsDrawObj; }
    inline void SetDrawObj( sal_Bool bNew ) { bIsDrawObj = bNew; }

protected:
    virtual void    GetNextPortion( ::rtl::OUString& rNextPortion,
                            LanguageType& rLangOfPortion,
                            sal_Bool bAllowImplicitChangesForNotConvertibleText );
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

public:
    SwHHCWrapper(
        SwView* pView,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF,
        LanguageType nSourceLanguage, LanguageType nTargetLanguage,
        const Font *pTargetFont,
        sal_Int32 nConvOptions, sal_Bool bIsInteractive,
        sal_Bool bStart, sal_Bool bOther, sal_Bool bSelection );

    virtual ~SwHHCWrapper();

    void    Convert();
};


#endif


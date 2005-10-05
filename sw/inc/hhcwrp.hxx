/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hhcwrp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 13:18:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _HHCWRP_HXX
#define _HHCWRP_HXX

#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include <svx/hangulhanja.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>
#endif

class SwView;
class Window;
class SwWrtShell;
struct SwConversionArgs;

//////////////////////////////////////////////////////////////////////

class SwHHCWrapper : public svx::HangulHanjaConversion
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

    USHORT      nPageCount;     // page count for progress bar
    USHORT      nPageStart;     // first checked page

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
    void        ChangeText( const String &rNewText );

    inline BOOL IsDrawObj()             { return bIsDrawObj; }
    inline void SetDrawObj( BOOL bNew ) { bIsDrawObj = bNew; }

protected:
    virtual void    GetNextPortion( ::rtl::OUString& rNextPortion,
                            LanguageType& rLangOfPortion,
                            sal_Bool bAllowImplicitChangesForNotConvertibleText );
    virtual void    HandleNewUnit( const sal_Int32 nUnitStart,
                                   const sal_Int32 nUnitEnd );
    virtual void    ReplaceUnit(
                        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
                        const ::rtl::OUString& rReplaceWith,
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


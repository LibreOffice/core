/*************************************************************************
 *
 *  $RCSfile: hhcwrp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-04-04 08:13:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    inline BOOL IsDrawObj()             { return bIsDrawObj; }
    inline void SetDrawObj( BOOL bNew ) { bIsDrawObj = bNew; }

protected:
    virtual void    GetNextPortion( ::rtl::OUString& rNextPortion, LanguageType& rLangOfPortion );
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


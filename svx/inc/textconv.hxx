/*************************************************************************
 *
 *  $RCSfile: textconv.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 15:44:35 $
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
#ifndef _TEXTCONV_HXX
#define _TEXTCONV_HXX

#include <splwrap.hxx>
#include <svxacorr.hxx>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include "hangulhanja.hxx"
#endif

class EditView;
class ImpEditEngine;
class ContentNode;

class TextConvWrapper : public svx::HangulHanjaConversion
{
    rtl::OUString   aConvText;      // convertible text part found last time
    USHORT          nLastPos;       // starting position of the last found text portion (word)
    USHORT          nUnitOffset;    // offset of current unit in the current text portion (word)

    ESelection      aConvSel;       // selection to be converted if
                                    // 'HasRange' is true, other conversion
                                    // starts from the cursor position

    EditView *      pEditView;
    Window *        pWin;

    LanguageType    nLang;
    sal_Bool        bStartChk;
    sal_Bool        bStartDone;
    sal_Bool        bEndDone;


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

protected:
    virtual void    GetNextPortion( ::rtl::OUString& /* [out] */ rNextPortion );
    virtual void    HandleNewUnit( const sal_Int32 nUnitStart,
                                   const sal_Int32 nUnitEnd );
    virtual void    ReplaceUnit(
                        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
                        const ::rtl::OUString& rReplaceWith,
                        ReplacementAction eAction );

   virtual sal_Bool    HasRubySupport() const;

public:
    TextConvWrapper( Window* pWindow,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF,
            const ::com::sun::star::lang::Locale& rLocale,
            BOOL bIsStart, EditView* pView );

    virtual ~TextConvWrapper();

    void    Convert();
};

#endif


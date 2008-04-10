/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hyphen.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SVX_HYPHEN_HXX
#define _SVX_HYPHEN_HXX

// include ---------------------------------------------------------------

#include <vcl/edit.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>
#include <com/sun/star/uno/Reference.hxx>
// forward ---------------------------------------------------------------

namespace com{namespace sun{namespace star{
namespace linguistic2{
    class XHyphenator;
    class XPossibleHyphens;
}}}}

class SvxSpellWrapper;

// class SvxHyphenEdit ---------------------------------------------------

class SvxHyphenEdit : public Edit
{
public:
    SvxHyphenEdit( Window* pParent, const ResId& rResId );

protected:
    virtual void    KeyInput( const KeyEvent &rKEvt );
};

// class SvxHyphenWordDialog ---------------------------------------------

class SvxHyphenWordDialog : public SfxModalDialog
{
public:
    SvxHyphenWordDialog( const String &rWord, LanguageType nLang,
                         Window* pParent,
                         ::com::sun::star::uno::Reference<
                             ::com::sun::star::linguistic2::XHyphenator >  &xHyphen,
                         SvxSpellWrapper* pWrapper );

    void            SelLeft();
    void            SelRight();

private:
    FixedText           aWordFT;
    SvxHyphenEdit       aWordEdit;
    ImageButton         aLeftBtn;
    ImageButton         aRightBtn;
    OKButton            aOkBtn;
    CancelButton        aCancelBtn;
    PushButton          aContBtn;
    PushButton          aDelBtn;
    HelpButton          aHelpBtn;
    String              aLabel;
    SvxSpellWrapper*    pHyphWrapper;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >        xHyphenator;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XPossibleHyphens >   xPossHyph;
    String              aActWord;       // actual (to be displayed) word
    LanguageType        nActLanguage;   // and language
    sal_uInt16              nMaxHyphenationPos; // right most valid hyphenation pos
    sal_uInt16              nHyphPos;
    sal_uInt16              nOldPos;
    sal_Bool                bBusy;

#ifdef _SVX_HYPHEN_CXX
    void            EnableLRBtn_Impl();
    //void          EnableCutBtn_Impl();
    void            SetLabel_Impl( LanguageType nLang );
    String          EraseUnusableHyphens_Impl(
                        ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XPossibleHyphens >  &rxPossHyph,
                        sal_uInt16 nMaxHyphenationPos );

    void            InitControls_Impl();
    void            ContinueHyph_Impl( sal_uInt16 nInsPos = 0 );
    sal_uInt16          GetHyphIndex_Impl();

    DECL_LINK( CutHdl_Impl, Button* );
    DECL_LINK( DeleteHdl_Impl, Button* );
    DECL_LINK( ContinueHdl_Impl, Button* );
    DECL_LINK( CancelHdl_Impl, Button* );
    DECL_LINK( Left_Impl, Button* );
    DECL_LINK( Right_Impl, Button* );
    DECL_LINK( GetFocusHdl_Impl, Edit* );
    DECL_LINK( LangError_Impl, void* );
#endif
};


#endif


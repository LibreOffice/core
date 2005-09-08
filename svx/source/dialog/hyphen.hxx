/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyphen.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:17:50 $
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
#ifndef _SVX_HYPHEN_HXX
#define _SVX_HYPHEN_HXX

// include ---------------------------------------------------------------

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
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


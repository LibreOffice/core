/*************************************************************************
 *
 *  $RCSfile: hyphen.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 09:55:13 $
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
#ifndef _SVX_HYPHEN_HXX
#define _SVX_HYPHEN_HXX

// include ---------------------------------------------------------------

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
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
    SvxHyphenEdit       aWordEdit;
    ImageButton         aLeftBtn;
    ImageButton         aRightBtn;
    GroupBox            aWordBox;
    PushButton          aContBtn;
    PushButton          aDelBtn;
    OKButton            aOkBtn;
    CancelButton        aCancelBtn;
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


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chinese_translationdialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:58:13 $
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

#ifndef _TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDLG_HXX
#define _TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDLG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class RadioButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class ChineseDictionaryDialog;

class ChineseTranslationDialog : public ModalDialog
{
public:
    ChineseTranslationDialog( Window* pParent );
    virtual ~ChineseTranslationDialog();

    void getSettings( sal_Bool& rbDirectionToSimplified
                    , sal_Bool& rbUseCharacterVariants
                    , sal_Bool& rbTranslateCommonTerms ) const;

private:
    DECL_LINK( DictionaryHdl, void* );
    DECL_LINK( DirectionHdl, void* );
    DECL_LINK( CommonTermsHdl, void* );
    DECL_LINK( OkHdl, void* );
    void impl_UpdateVariantsCheckBox();

private:
    FixedLine   m_aFL_Direction;

    RadioButton m_aRB_To_Simplified;
    RadioButton m_aRB_To_Traditional;
    CheckBox    m_aCB_Use_Variants;

    FixedLine   m_aFL_Commonterms;

    CheckBox    m_aCB_Translate_Commonterms;
    PushButton  m_aPB_Editterms;

    FixedLine   m_aFL_Bottomline;

    OKButton        m_aBP_OK;
    CancelButton    m_aBP_Cancel;
    HelpButton      m_aBP_Help;

    ChineseDictionaryDialog* m_pDictionaryDialog;
};

//.............................................................................
} //end namespace
//.............................................................................
#endif

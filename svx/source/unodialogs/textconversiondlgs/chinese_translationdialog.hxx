/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDLG_HXX
#define _TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDLG_HXX

#include <vcl/dialog.hxx>
// header for class FixedLine
#include <vcl/fixed.hxx>
// header for class RadioButton
#include <vcl/button.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

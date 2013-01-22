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
                    , sal_Bool& rbTranslateCommonTerms ) const;

private:
    DECL_LINK( DictionaryHdl, void* );
    DECL_LINK( CommonTermsHdl, void* );
    DECL_LINK( OkHdl, void* );
    void impl_UpdateVariantsCheckBox();

private:
    FixedLine   m_aFL_Direction;

    RadioButton m_aRB_To_Simplified;
    RadioButton m_aRB_To_Traditional;

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

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

#ifndef INCLUDED_SVX_SOURCE_UNODIALOGS_TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDIALOG_HXX
#define INCLUDED_SVX_SOURCE_UNODIALOGS_TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>


namespace textconversiondlgs
{



/**
*/
class ChineseDictionaryDialog;

class ChineseTranslationDialog : public ModalDialog
{
public:
    ChineseTranslationDialog( vcl::Window* pParent );
    virtual ~ChineseTranslationDialog();

    void getSettings( bool& rbDirectionToSimplified
                    , bool& rbTranslateCommonTerms ) const;

private:
    DECL_LINK( DictionaryHdl, void* );
    DECL_LINK( CommonTermsHdl, void* );
    DECL_LINK( OkHdl, void* );
    void impl_UpdateVariantsCheckBox();

private:
    RadioButton* m_pRB_To_Simplified;
    RadioButton* m_pRB_To_Traditional;

    CheckBox*   m_pCB_Translate_Commonterms;
    PushButton* m_pPB_Editterms;

    OKButton*   m_pBP_OK;

    ChineseDictionaryDialog* m_pDictionaryDialog;
};


} //end namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

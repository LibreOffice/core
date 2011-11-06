/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDLG_HXX
#define _TEXTCONVERSIONDLGS_CHINESE_TRANSLATIONDLG_HXX

#include <vcl/dialog.hxx>
// header for class FixedLine
#include <vcl/fixed.hxx>
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

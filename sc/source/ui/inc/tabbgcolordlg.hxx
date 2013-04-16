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



#ifndef SC_TABBGCOLORDLG_HXX
#define SC_TABBGCOLORDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svx/SvxColorValueSet.hxx>

//------------------------------------------------------------------------

class ScTabBgColorDlg : public ModalDialog
{
public:
    ScTabBgColorDlg( Window* pParent,
                     const String& rTitle,
                     const String& rTabBgColorNoColorText,
                     const Color& rDefaultColor,
                     const rtl::OString& nHelpId );
    ~ScTabBgColorDlg();

    void GetSelectedColor( Color& rColor ) const;

private:
    class ScTabBgColorValueSet : public SvxColorValueSet
    {
    public:
        ScTabBgColorValueSet(Control* pParent, const ResId& rResId, ScTabBgColorDlg* pTabBgColorDlg);

        virtual void KeyInput( const KeyEvent& rKEvt );
    private:
        ScTabBgColorDlg* aTabBgColorDlg;
    };

    Control                 aBorderWin;
    ScTabBgColorValueSet    aTabBgColorSet;
    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;
    Color                   aTabBgColor;
    const String            aTabBgColorNoColorText;
    rtl::OString            msHelpId;

    void            FillColorValueSets_Impl();

    DECL_LINK( TabBgColorDblClickHdl_Impl, ValueSet* );
    DECL_LINK( TabBgColorOKHdl_Impl, OKButton* pBtn );
};

#endif // SC_TABBGCOLORDLG_HXX

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



#ifndef SC_SOLVEROPTIONS_HXX
#define SC_SOLVEROPTIONS_HXX

#include <vcl/dialog.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <svx/checklbx.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }



class ScSolverOptionsDialog : public ModalDialog
{
    FixedText       maFtEngine;
    ListBox         maLbEngine;
    FixedText       maFtSettings;
    SvxCheckListBox maLbSettings;
    PushButton      maBtnEdit;
    FixedLine       maFlButtons;
    HelpButton      maBtnHelp;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

    SvLBoxButtonData* mpCheckButtonData;
    com::sun::star::uno::Sequence<rtl::OUString> maImplNames;
    com::sun::star::uno::Sequence<rtl::OUString> maDescriptions;
    String          maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

    DECL_LINK( EngineSelectHdl, ListBox* );
    DECL_LINK( SettingsSelHdl, SvxCheckListBox* );
    DECL_LINK( SettingsDoubleClickHdl, SvTreeListBox* );
    DECL_LINK( ButtonHdl, PushButton* );

    void    ReadFromComponent();
    void    FillListBox();
    void    EditOption();

public:
    ScSolverOptionsDialog( Window* pParent,
                           const com::sun::star::uno::Sequence<rtl::OUString>& rImplNames,
                           const com::sun::star::uno::Sequence<rtl::OUString>& rDescriptions,
                           const String& rEngine,
                           const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties );
    ~ScSolverOptionsDialog();

    const String& GetEngine() const;
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& GetProperties();
};


class ScSolverIntegerDialog : public ModalDialog
{
    FixedText       maFtName;
    NumericField    maNfValue;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

public:
    ScSolverIntegerDialog( Window * pParent );
    ~ScSolverIntegerDialog();

    void        SetOptionName( const String& rName );
    void        SetValue( sal_Int32 nValue );
    sal_Int32   GetValue() const;
};

class ScSolverValueDialog : public ModalDialog
{
    FixedText       maFtName;
    Edit            maEdValue;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

public:
    ScSolverValueDialog( Window * pParent );
    ~ScSolverValueDialog();

    void        SetOptionName( const String& rName );
    void        SetValue( double fValue );
    double      GetValue() const;
};

#endif


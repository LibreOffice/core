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



#ifndef SC_SCENDLG_HXX
#define SC_SCENDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/ctrlbox.hxx>

//===================================================================

class ScNewScenarioDlg : public ModalDialog
{
public:
        ScNewScenarioDlg( Window* pParent, const String& rName, sal_Bool bEdit = sal_False, sal_Bool bSheetProtected = sal_False );
        ~ScNewScenarioDlg();

    void SetScenarioData( const String& rName, const String& rComment,
                            const Color& rColor, sal_uInt16 nFlags );

    void GetScenarioData( String& rName, String& rComment,
                            Color& rColor, sal_uInt16& rFlags ) const;

private:
    FixedLine           aFlName;
    Edit                aEdName;
    FixedLine           aFlComment;
    MultiLineEdit       aEdComment;
    FixedLine           aFlOptions;
    CheckBox            aCbShowFrame;
    ColorListBox        aLbColor;
    //CheckBox          aCbPrintFrame;
    CheckBox            aCbTwoWay;
    //CheckBox          aCbAttrib;
    //CheckBox          aCbValue;
    CheckBox            aCbCopyAll;
    CheckBox            aCbProtect;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    const String        aDefScenarioName;
    sal_Bool                bIsEdit;

    DECL_LINK( OkHdl, OKButton * );
    DECL_LINK( EnableHdl, CheckBox * );
};


#endif // SC_SCENDLG_HXX



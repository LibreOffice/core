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



#ifndef SC_PFILTDLG_HXX
#define SC_PFILTDLG_HXX

#ifndef _SV_HXX
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#include <svtools/stdctrl.hxx>
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#include "address.hxx"
#include "queryparam.hxx"

//------------------------------------------------------------------

class ScViewData;
class ScDocument;
class ScQueryItem;
class TypedScStrCollection;

//==================================================================

class ScPivotFilterDlg : public ModalDialog
{
public:
                    ScPivotFilterDlg( Window* pParent,
                                      const SfxItemSet& rArgSet, SCTAB nSourceTab );
                    ~ScPivotFilterDlg();

    const ScQueryItem&  GetOutputItem();

private:
    FixedLine       aFlCriteria;
    //----------------------------
    ListBox         aLbField1;
    ListBox         aLbCond1;
    ComboBox        aEdVal1;
    //----------------------------
    ListBox         aLbConnect1;
    ListBox         aLbField2;
    ListBox         aLbCond2;
    ComboBox        aEdVal2;
    //----------------------------
    ListBox         aLbConnect2;
    ListBox         aLbField3;
    ListBox         aLbCond3;
    ComboBox        aEdVal3;
    //----------------------------
    FixedText       aFtConnect;
    FixedText       aFtField;
    FixedText       aFtCond;
    FixedText       aFtVal;

    FixedLine       aFlOptions;
    CheckBox        aBtnCase;
    CheckBox        aBtnRegExp;
    CheckBox        aBtnUnique;
    FixedText       aFtDbAreaLabel;
    FixedInfo       aFtDbArea;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    MoreButton      aBtnMore;
    const String    aStrUndefined;
    const String    aStrNoName;
    const String    aStrNone;
    const String    aStrEmpty;
    const String    aStrNotEmpty;
    const String    aStrRow;
    const String    aStrColumn;

    const sal_uInt16        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    sal_uInt16              nFieldCount;
    ComboBox*           aValueEdArr[3];
    ListBox*            aFieldLbArr[3];
    ListBox*            aCondLbArr[3];

    TypedScStrCollection*   pEntryLists[MAXCOLCOUNT];

#ifdef _PFILTDLG_CXX
private:
    void    Init            ( const SfxItemSet& rArgSet );
    void    FillFieldLists  ();
    void    UpdateValueList ( sal_uInt16 nList );
    void    ClearValueList  ( sal_uInt16 nList );
    sal_uInt16  GetFieldSelPos  ( SCCOL nField );

    // Handler:
    DECL_LINK( LbSelectHdl, ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK( CheckBoxHdl,  CheckBox* );
#endif
};


#endif // SC_PFILTDLG_HXX


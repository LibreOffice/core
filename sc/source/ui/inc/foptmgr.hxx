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



#ifndef SC_FOPTMGR_HXX
#define SC_FOPTMGR_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

//----------------------------------------------------------------------------

class FixedInfo;
namespace formula
{
    class RefButton;
}
class MoreButton;
class Dialog;
struct ScQueryParam;
class ScDocument;
class ScViewData;

//----------------------------------------------------------------------------

class ScFilterOptionsMgr
{
public:
            ScFilterOptionsMgr( Dialog*             ptrDlg,
                                ScViewData*         ptrViewData,
                                const ScQueryParam& refQueryData,
                                MoreButton&         refBtnMore,
                                CheckBox&           refBtnCase,
                                CheckBox&           refBtnRegExp,
                                CheckBox&           refBtnHeader,
                                CheckBox&           refBtnUnique,
                                CheckBox&           refBtnCopyResult,
                                CheckBox&           refBtnDestPers,
                                ListBox&            refLbCopyArea,
                                Edit&               refEdCopyArea,
                                formula::RefButton&     refRbCopyArea,
                                FixedText&          refFtDbAreaLabel,
                                FixedInfo&          refFtDbArea,
                                FixedLine&          refFlOptions,
                                const String&       refStrNoName,
                                const String&       refStrUndefined );
            ~ScFilterOptionsMgr();

    sal_Bool    VerifyPosStr ( const String& rPosStr ) const;

private:
    Dialog*         pDlg;
    ScViewData*     pViewData;
    ScDocument*     pDoc;

    MoreButton&     rBtnMore;
    CheckBox&       rBtnCase;
    CheckBox&       rBtnRegExp;
    CheckBox&       rBtnHeader;
    CheckBox&       rBtnUnique;
    CheckBox&       rBtnCopyResult;
    CheckBox&       rBtnDestPers;
    ListBox&        rLbCopyPos;
    Edit&           rEdCopyPos;
    formula::RefButton& rRbCopyPos;
    FixedText&      rFtDbAreaLabel;
    FixedInfo&      rFtDbArea;
    FixedLine&      rFlOptions;

    const String&   rStrNoName;
    const String&   rStrUndefined;

    const ScQueryParam& rQueryData;

#ifdef _FOPTMGR_CXX
private:
    void Init();

    // Handler:
    DECL_LINK( EdPosModifyHdl,      Edit* );
    DECL_LINK( LbPosSelHdl,         ListBox* );
    DECL_LINK( BtnCopyResultHdl,    CheckBox* );
#endif
};



#endif // SC_FOPTMGR_HXX

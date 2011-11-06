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



#ifndef SC_HIGHRED_HXX
#define SC_HIGHRED_HXX

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>


#include "rangenam.hxx"
#include "anyrefdg.hxx"

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#include <vcl/lstbox.hxx>

#ifndef _SVX_ACREDLIN_HXX
#include <svx/ctredlin.hxx>
#endif
#include <svx/simptabl.hxx>
#include "chgtrack.hxx"
#include "chgviset.hxx"

class ScViewData;
class ScDocument;

#ifndef FLT_DATE_BEFORE
#define FLT_DATE_BEFORE     0
#define FLT_DATE_SINCE      1
#define FLT_DATE_EQUAL      2
#define FLT_DATE_NOTEQUAL   3
#define FLT_DATE_BETWEEN    4
#define FLT_DATE_SAVE       5
#endif

//==================================================================

class ScHighlightChgDlg : public ScAnyRefDlg
{
private:

    CheckBox                aHighlightBox;
    FixedLine               aFlFilter;
    SvxTPFilter             aFilterCtr;
    CheckBox                aCbAccept;
    CheckBox                aCbReject;

    OKButton                aOkButton;
    CancelButton            aCancelButton;
    HelpButton              aHelpButton;

    formula::RefEdit                aEdAssign;
    formula::RefButton              aRbAssign;

    ScViewData*             pViewData;
    ScDocument*             pDoc;
    ScRangeName             aLocalRangeName;
    Selection               theCurSel;
    Size                    MinSize;
    ScRangeList             aRangeList;
    ScChangeViewSettings    aChangeViewSet;

    void                    Init();

    DECL_LINK( RefHandle, SvxTPFilter* );
    DECL_LINK(HighLightHandle, CheckBox*);
    DECL_LINK(OKBtnHdl, PushButton*);


protected:

    virtual void    RefInputDone( sal_Bool bForced = sal_False );

public:
                    ScHighlightChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData);

                    ~ScHighlightChgDlg();

    virtual void    SetActive();
    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    Close();
    virtual sal_Bool    IsRefInputMode() const;

};


#endif // SC_NAMEDLG_HXX


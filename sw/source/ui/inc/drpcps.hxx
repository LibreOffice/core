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


#ifndef _DRPCPS_HXX
#define _DRPCPS_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class SwWrtShell;

// class SwDropCapsDlg ******************************************************

class SwDropCapsDlg : public SfxSingleTabDialog
{

public:

     SwDropCapsDlg(Window *pParent, const SfxItemSet &rSet );
    ~SwDropCapsDlg();

};

class SwDropCapsPict;

// class SwDropCapsPage *****************************************************

class SwDropCapsPage : public SfxTabPage
{
friend class SwDropCapsPict;

    FixedLine       aSettingsFL;
    CheckBox        aDropCapsBox;
    CheckBox        aWholeWordCB;
    FixedText       aSwitchText;
    NumericField    aDropCapsField;
    FixedText       aLinesText;
    NumericField    aLinesField;
    FixedText       aDistanceText;
    MetricField     aDistanceField;

    FixedLine       aContentFL;
    FixedText       aTextText;
    Edit            aTextEdit;
    FixedText       aTemplateText;
    ListBox         aTemplateBox;

    SwDropCapsPict  *pPict;

    sal_Bool            bModified;
    sal_Bool            bFormat;
    sal_Bool            bHtmlMode;

    SwWrtShell &rSh;

     SwDropCapsPage(Window *pParent, const SfxItemSet &rSet);
    ~SwDropCapsPage();

    virtual int     DeactivatePage(SfxItemSet *pSet);
    void    FillSet( SfxItemSet &rSet );

    DECL_LINK( ClickHdl, Button * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( WholeWordHdl, CheckBox * );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool FillItemSet(      SfxItemSet &rSet);
    virtual void Reset      (const SfxItemSet &rSet);

    void    SetFormat(sal_Bool bSet){bFormat = bSet;}
};

#endif


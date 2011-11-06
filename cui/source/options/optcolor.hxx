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


#ifndef _SVX_OPTCOLOR_HXX
#define _SVX_OPTCOLOR_HXX

// include ---------------------------------------------------------------

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

/* -----------------------------25.03.2002 10:40------------------------------

 ---------------------------------------------------------------------------*/
namespace svtools {class EditableColorConfig;class EditableExtendedColorConfig;}
class ColorConfigCtrl_Impl;
class AbstractSvxNameDialog; //CHINA001 class SvxNameDialog;
class SvxColorOptionsTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

    FixedLine               aColorSchemeFL;
    FixedText               aColorSchemeFT;
    ListBox                 aColorSchemeLB;
    PushButton              aSaveSchemePB;
    PushButton              aDeleteSchemePB;

    FixedLine               aCustomColorsFL;

    sal_Bool                    bFillItemSetCalled;

    svtools::EditableColorConfig* pColorConfig;
    svtools::EditableExtendedColorConfig* pExtColorConfig;
    ColorConfigCtrl_Impl*   pColorConfigCT;

    DECL_LINK(SchemeChangedHdl_Impl, ListBox*);
    DECL_LINK(SaveDeleteHdl_Impl, PushButton*);
    DECL_LINK(CheckNameHdl_Impl, AbstractSvxNameDialog*);
    void UpdateColorConfig();

public:
    SvxColorOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxColorOptionsTabPage(  );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual int         DeactivatePage( SfxItemSet* pSet );
    virtual void        FillUserData();

};

#endif


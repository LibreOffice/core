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


#ifndef _SVX_OPTASIAN_HXX
#define _SVX_OPTASIAN_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <svx/langbox.hxx>
struct SvxAsianLayoutPage_Impl;
class SvxAsianLayoutPage : public SfxTabPage
{
    FixedLine    aKerningGB;
    RadioButton aCharKerningRB;
    RadioButton aCharPunctKerningRB;

    FixedLine    aCharDistGB;
    RadioButton aNoCompressionRB;
    RadioButton aPunctCompressionRB;
    RadioButton aPunctKanaCompressionRB;

    FixedLine    aStartEndGB;

    FixedText       aLanguageFT;
    SvxLanguageBox  aLanguageLB;
    CheckBox        aStandardCB;

    FixedText   aStartFT;
    Edit        aStartED;
    FixedText   aEndFT;
    Edit        aEndED;
    FixedText   aHintFT;

    SvxAsianLayoutPage_Impl* pImpl;

    DECL_LINK(LanguageHdl, SvxLanguageBox*);
    DECL_LINK(ChangeStandardHdl, CheckBox*);
    DECL_LINK(ModifyHdl, Edit*);

    SvxAsianLayoutPage( Window* pParent, const SfxItemSet& rSet );
public:

    virtual ~SvxAsianLayoutPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static sal_uInt16*      GetRanges();
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif


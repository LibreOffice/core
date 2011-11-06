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


#ifndef _SFX_MGETEMPL_HXX
#define _SFX_MGETEMPL_HXX

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

#include <sfx2/tabdlg.hxx>

#include <svtools/svmedit2.hxx>
#include <svtools/svmedit.hxx>

/* erwartet:
    SID_TEMPLATE_NAME   :   In: StringItem, Name der Vorlage
    SID_TEMPLATE_FAMILY :   In: Familie der Vorlage
*/

class SfxStyleFamilies;
class SfxStyleFamilyItem;
class SfxStyleSheetBase;

#ifdef _SFXDLG_HXX
#define FixedInfo FixedText
#endif

class SfxManageStyleSheetPage : public SfxTabPage
{
    FixedText aNameFt;
    Edit aNameEd;
    ExtMultiLineEdit aNameMLE;

    CheckBox    aAutoCB;

    FixedText aFollowFt;
    ListBox aFollowLb;

    FixedText aBaseFt;
    ListBox aBaseLb;

    FixedText aFilterFt;
    ListBox aFilterLb;

    FixedLine aDescGb;
    FixedInfo aDescFt;
    MultiLineEdit aDescED;

    SfxStyleSheetBase *pStyle;
    SfxStyleFamilies *pFamilies;
    const SfxStyleFamilyItem *pItem;
    String aBuf;
    sal_Bool bModified;

        // initiale Daten des Styles
    String aName;
    String aFollow;
    String aParent;
    sal_uInt16 nFlags;

private:
friend class SfxStyleDialog;

//#if 0 // _SOLAR__PRIVATE
    DECL_LINK( GetFocusHdl, Edit * );
    DECL_LINK( LoseFocusHdl, Edit * );

    void    UpdateName_Impl(ListBox *, const String &rNew);
    void    SetDescriptionText_Impl();
//#endif

    SfxManageStyleSheetPage(Window *pParent, const SfxItemSet &rAttrSet );
    ~SfxManageStyleSheetPage();

    static SfxTabPage*  Create(Window *pParent, const SfxItemSet &rAttrSet );

protected:
    virtual sal_Bool        FillItemSet(SfxItemSet &);
    virtual void        Reset(const SfxItemSet &);

    using TabPage::ActivatePage;
        virtual void        ActivatePage(const SfxItemSet &);
        using TabPage::DeactivatePage;
    virtual int     DeactivatePage(SfxItemSet * = 0);
};

#ifdef FixedInfo
#undef FixedInfo
#endif

#endif


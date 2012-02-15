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



#ifndef _SD_PRNTOPTS_HXX
#define _SD_PRNTOPTS_HXX

#include <vcl/group.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

/*
enum PrintType
{
    PT_DRAWING,
    PT_NOTES,
    PT_HANDOUT,
    PT_OUTLINE
};
*/
class SdModule;
class SdPrintOptions : public SfxTabPage
{
 friend class SdOptionsDlg;
 friend class SdModule;

private:
    FixedLine           aGrpPrint;
    CheckBox            aCbxDraw;
    CheckBox            aCbxNotes;
    CheckBox            aCbxHandout;
    CheckBox            aCbxOutline;

    FixedLine           aSeparator1FL;
    FixedLine           aGrpOutput;
    RadioButton         aRbtColor;
    RadioButton         aRbtGrayscale;
    RadioButton         aRbtBlackWhite;

    FixedLine           aGrpPrintExt;
    CheckBox            aCbxPagename;
    CheckBox            aCbxDate;
    CheckBox            aCbxTime;
    CheckBox            aCbxHiddenPages;

    FixedLine           aSeparator2FL;
    FixedLine           aGrpPageoptions;
    RadioButton         aRbtDefault;
    RadioButton         aRbtPagesize;
    RadioButton         aRbtPagetile;
    RadioButton         aRbtBooklet;
    CheckBox            aCbxFront;
    CheckBox            aCbxBack;

    CheckBox            aCbxPaperbin;

    const SfxItemSet&   rOutAttrs;

    DECL_LINK( ClickCheckboxHdl, CheckBox * );
    DECL_LINK( ClickBookletHdl, CheckBox * );

    void updateControls();

    using OutputDevice::SetDrawMode;
public:
            SdPrintOptions( Window* pParent, const SfxItemSet& rInAttrs);
            ~SdPrintOptions();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    void    SetDrawMode();
    virtual void        PageCreated (SfxAllItemSet aSet);
};



#endif // _SD_PRNTOPTS_HXX



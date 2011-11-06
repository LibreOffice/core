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


#ifndef _SWFLDDOK_HXX
#define _SWFLDDOK_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>

#include "numfmtlb.hxx"
#include "fldpage.hxx"

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldDokPage : public SwFldPage
{
    FixedText           aTypeFT;
    ListBox             aTypeLB;
    FixedText           aSelectionFT;
    ListBox             aSelectionLB;
    FixedText           aValueFT;
    Edit                aValueED;
    NumericField        aLevelED;
    NumericField        aDateOffsetED;
    FixedText           aFormatFT;
    ListBox             aFormatLB;
    NumFormatListBox    aNumFormatLB;
    CheckBox            aFixedCB;

    String              sDateOffset;
    String              sTimeOffset;
    Bitmap              aRootOpened;
    Bitmap              aRootClosed;

    sal_uInt16              nOldSel;
    sal_uLong               nOldFormat;

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( FormatHdl, ListBox* pLB = 0 );
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );

    void                AddSubType(sal_uInt16 nTypeId);
    sal_uInt16              FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldDokPage(Window* pWindow, const SfxItemSet& rSet);

                        ~SwFldDokPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif


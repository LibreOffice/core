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


#ifndef _SWLINENUM_HXX
#define _SWLINENUM_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <numberingtypelistbox.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class Window;
class SfxItemSet;
class SwView;
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung: SingleTabDialog
 --------------------------------------------------------------------*/

class SwLineNumberingDlg : public SfxSingleTabDialog
{
    SwWrtShell* pSh;

    DECL_LINK( OKHdl, Button * );

public:
    inline SwWrtShell* GetWrtShell() const { return pSh; }

     SwLineNumberingDlg(SwView *pVw);
    ~SwLineNumberingDlg();
};

/*--------------------------------------------------------------------
   Beschreibung: TabPage
 --------------------------------------------------------------------*/

class SwLineNumberingPage : public SfxTabPage
{
    CheckBox        aNumberingOnCB;
    FixedLine        aDisplayFL;
    FixedText       aCharStyleFT;
    ListBox         aCharStyleLB;
    FixedText       aFormatFT;
    SwNumberingTypeListBox   aFormatLB;
    FixedText       aPosFT;
    ListBox         aPosLB;
    FixedText       aOffsetFT;
    MetricField     aOffsetMF;
    FixedText       aNumIntervalFT;
    NumericField    aNumIntervalNF;
    FixedText       aNumRowsFT;
    FixedLine        aDivisorFL;
    FixedText       aDivisorFT;
    Edit            aDivisorED;
    FixedText       aDivIntervalFT;
    NumericField    aDivIntervalNF;
    FixedText       aDivRowsFT;
    FixedLine        aCountFL;
    CheckBox        aCountEmptyLinesCB;
    CheckBox        aCountFrameLinesCB;
    CheckBox        aRestartEachPageCB;

    SwWrtShell*     pSh;

    SwLineNumberingPage( Window* pParent, const SfxItemSet& rSet );
    ~SwLineNumberingPage();

    DECL_LINK( LineOnOffHdl, CheckBox *pCB = 0 );
    DECL_LINK( ModifyHdl, Edit *pED = 0 );

public:

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif



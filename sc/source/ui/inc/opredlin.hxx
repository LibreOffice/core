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


#ifndef _SC_OPREDLIN_HXX
#define _SC_OPREDLIN_HXX

#include <sfx2/tabdlg.hxx>

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/ctrlbox.hxx>
#include <svx/fntctrl.hxx>
#include <svx/strarray.hxx>

/*-----------------------------------------------------------------------
    Beschreibung: Redlining-Optionen
 -----------------------------------------------------------------------*/

class ScRedlineOptionsTabPage : public SfxTabPage
{
    FixedText           aContentFT;
    ColorListBox        aContentColorLB;
    FixedText           aRemoveFT;
    ColorListBox        aRemoveColorLB;
    FixedText           aInsertFT;
    ColorListBox        aInsertColorLB;
    FixedText           aMoveFT;
    ColorListBox        aMoveColorLB;
    FixedLine           aChangedGB;
    String              aAuthorStr;
    DECL_LINK( ColorHdl, ColorListBox *pColorLB );


public:

    ScRedlineOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~ScRedlineOptionsTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};
#endif



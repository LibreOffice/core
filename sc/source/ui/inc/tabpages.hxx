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



#ifndef SC_TABPAGES_HXX
#define SC_TABPAGES_HXX

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>

//========================================================================

class ScTabPageProtection : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rAttrSet );
    static  sal_uInt16*     GetRanges       ();
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreAttrs );
    virtual void        Reset           ( const SfxItemSet& );

protected:
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
                ScTabPageProtection( Window*            pParent,
                                     const SfxItemSet&  rCoreAttrs );
                ~ScTabPageProtection();

private:
    FixedLine   aFlProtect;
    TriStateBox aBtnHideCell;
    TriStateBox aBtnProtect;
    TriStateBox aBtnHideFormula;
    FixedInfo   aTxtHint;

    FixedLine   aFlPrint;
    TriStateBox aBtnHidePrint;
    FixedInfo   aTxtHint2;

                                    // aktueller Status:
    sal_Bool        bTriEnabled;        //  wenn vorher Dont-Care
    sal_Bool        bDontCare;          //  alles auf TriState
    sal_Bool        bProtect;           //  einzelne Einstellungen ueber TriState sichern
    sal_Bool        bHideForm;
    sal_Bool        bHideCell;
    sal_Bool        bHidePrint;

    // Handler:
    DECL_LINK( ButtonClickHdl, TriStateBox* pBox );
    void        UpdateButtons();
};



#endif // SC_TABPAGES_HXX

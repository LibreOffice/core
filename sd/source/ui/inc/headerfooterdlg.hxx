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


#ifndef _SD_HEADERFOOTER_DIALOG_HXX
#define _SD_HEADERFOOTER_DIALOG_HXX

#include <vcl/tabdlg.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

#include "headerfooterdlg.hrc"
#include "sdpage.hxx"

class SfxObjectShell;
class SdUndoGroup;

namespace sd
{
class ViewShell;

class HeaderFooterTabPage;

class HeaderFooterDialog : public TabDialog
{
private:
    DECL_LINK( ActivatePageHdl, TabControl * );
    DECL_LINK( DeactivatePageHdl, TabControl * );

    TabControl      maTabCtrl;

    HeaderFooterTabPage*    mpSlideTabPage;
    HeaderFooterTabPage*    mpNotesHandoutsTabPage;

    HeaderFooterSettings    maSlideSettings;
    HeaderFooterSettings    maNotesHandoutSettings;
    bool                    mbNotOnTitle;

    SdDrawDocument*         mpDoc;
    SdPage*                 mpCurrentPage;
    ViewShell*              mpViewShell;

    void apply( bool bToAll, bool bForceSlides );
    void change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings );

public:
    HeaderFooterDialog( ViewShell* pViewShell, ::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    ~HeaderFooterDialog();

    void ApplyToAll( TabPage* pPage );
    void Apply( TabPage* pPage );
    void Cancel( TabPage* pPage );

    virtual short Execute();
};

}

#endif


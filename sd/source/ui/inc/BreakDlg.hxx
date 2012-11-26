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



#ifndef SD_BREAK_DLG_HXX
#define SD_BREAK_DLG_HXX

#include <vcl/group.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svx/dlgctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <svtools/prgsbar.hxx>
#include <vcl/edit.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>

class SvdProgressInfo;
class SfxProgress;

namespace sd {

class DrawDocShell;
class DrawView;

/*************************************************************************
|*
|* Dialog zum aufbrechen von Metafiles
|*
\************************************************************************/
class BreakDlg
    : public SfxModalDialog
{
public:
    BreakDlg (
        ::Window* pWindow,
        DrawView* pDrView,
        DrawDocShell* pShell,
        sal_uLong nSumActionCount,
        sal_uLong nObjCount);
    virtual ~BreakDlg();

    short Execute();

private:
    FixedText       aFtObjInfo;
    FixedText       aFtActInfo;
    FixedText       aFtInsInfo;

    FixedInfo       aFiObjInfo;
    FixedInfo       aFiActInfo;
    FixedInfo       aFiInsInfo;

    CancelButton    aBtnCancel;
    DrawView*   pDrView;

    bool            bCancel;

    Timer           aTimer;
    SvdProgressInfo *pProgrInfo;
    Link            aLink;
    SfxProgress     *mpProgress;

    DECL_LINK( CancelButtonHdl, void* );
    DECL_LINK( UpDate, void* );
    DECL_LINK( InitialUpdate, Timer* );
};

} // end of namespace sd

#endif

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


#ifndef _SFXNEW_HXX
#define _SFXNEW_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <sfx2/basedlgs.hxx>

//=========================================================================

class SfxObjectShellLock;
class SfxObjectShell;
class MoreButton;

//=========================================================================

#define SFXWB_PREVIEW 0x0003
#define SFXWB_LOAD_TEMPLATE 0x0004

#define SFX_LOAD_TEXT_STYLES    0x0001
#define SFX_LOAD_FRAME_STYLES   0x0002
#define SFX_LOAD_PAGE_STYLES    0x0004
#define SFX_LOAD_NUM_STYLES     0x0008
#define SFX_MERGE_STYLES        0x0010

#define RET_TEMPLATE_LOAD       100

class SFX2_DLLPUBLIC SfxPreviewWin: public Window
{
    SfxObjectShellLock &rDocShell;
protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    /** state whether a system setting for high contrast should be evaluated
        and taken into account for this window.

        The default implementation uses the accessibility option IsForDrawings
      */
    virtual bool    UseHighContrastSetting() const;

public:
    SfxPreviewWin( Window* pParent,
                   const ResId& rResId,
                   SfxObjectShellLock &rDocSh );
};

class SfxNewFileDialog_Impl;
class SFX2_DLLPUBLIC SfxNewFileDialog : public SfxModalDialog
{
    friend class SfxNewFileDialog_Impl;

private:
    SfxNewFileDialog_Impl* pImpl;

public:

    SfxNewFileDialog(Window *pParent, sal_uInt16 nFlags = 0);
    ~SfxNewFileDialog();

        // Liefert sal_False, wenn '- Keine -' als Vorlage eingestellt ist
        // Nur wenn IsTemplate() sal_True liefert, koennen Vorlagennamen
        // erfragt werden
    sal_Bool IsTemplate() const;
    String GetTemplateRegion() const;
    String GetTemplateName() const;
    String GetTemplateFileName() const;

    // load template methods
    sal_uInt16  GetTemplateFlags()const;
    void    SetTemplateFlags(sal_uInt16 nSet);
};

#endif

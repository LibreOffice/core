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


#ifndef _SFX_TEMPLDLG_HXX
#define _SFX_TEMPLDLG_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <vcl/ctrl.hxx>
#include <rsc/rscsfx.hxx>

#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>

class SfxTemplateDialog_Impl;
class SfxTemplateCatalog_Impl;

// class ISfxTemplateCommon ----------------------------------------------

class ISfxTemplateCommon
{
public:
    virtual SfxStyleFamily GetActualFamily() const = 0;
    virtual String GetSelectedEntry() const = 0;
};

// class SfxTemplateDialog -----------------------------------------------

class SfxTemplateDialog : public SfxDockingWindow
{
private:
friend class SfxTemplateDialogWrapper;
friend class SfxTemplateDialog_Impl;

    SfxTemplateDialog_Impl*     pImpl;

    virtual void                DataChanged( const DataChangedEvent& _rDCEvt );
    virtual void                Resize();
    virtual SfxChildAlignment   CheckAlignment( SfxChildAlignment, SfxChildAlignment );
    virtual void                StateChanged( StateChangedType nStateChange );

public:
    SfxTemplateDialog( SfxBindings*, SfxChildWindow*, Window* );
    ~SfxTemplateDialog();

    virtual void                Update();

    ISfxTemplateCommon*         GetISfxTemplateCommon();
    void                        SetParagraphFamily();
};

// class SfxTemplateCatalog ----------------------------------------------

class SfxTemplateCatalog : public SfxModalDialog
{
private:
    SfxTemplateCatalog_Impl *pImpl;

public:
    SfxTemplateCatalog(Window * pParent, SfxBindings *pBindings);
    ~SfxTemplateCatalog();
    friend class SfxTemplateCatalog_Impl;
};

// class SfxTemplateDialogWrapper ----------------------------------------

class SFX2_DLLPUBLIC SfxTemplateDialogWrapper : public SfxChildWindow
{
public:
                SfxTemplateDialogWrapper
                    (Window*,sal_uInt16,SfxBindings*,SfxChildWinInfo*);
                SFX_DECL_CHILDWINDOW(SfxTemplateDialogWrapper);

    void            SetParagraphFamily();
};

// class SfxTemplatePanelControl -----------------------------------------

class SFX2_DLLPUBLIC SfxTemplatePanelControl : public DockingWindow
{
public:
    SfxTemplatePanelControl (SfxBindings* pBindings, Window* pParentWindow);
    ~SfxTemplatePanelControl (void);

    virtual void                Update();
    virtual void                DataChanged( const DataChangedEvent& _rDCEvt );
    virtual void                Resize();
    virtual SfxChildAlignment   CheckAlignment( SfxChildAlignment, SfxChildAlignment );
    virtual void                StateChanged( StateChangedType nStateChange );
    virtual void                FreeResource (void);

    ISfxTemplateCommon*         GetISfxTemplateCommon();
    void                        SetParagraphFamily();

private:
    SfxTemplateDialog_Impl*     pImpl;
    SfxBindings* mpBindings;
};


#endif


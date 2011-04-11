/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX_TEMPLDLG_HXX
#define _SFX_TEMPLDLG_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

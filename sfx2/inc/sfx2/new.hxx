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
#ifndef _SFXNEW_HXX
#define _SFXNEW_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

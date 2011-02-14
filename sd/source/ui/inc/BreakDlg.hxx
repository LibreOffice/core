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

    sal_Bool            bCancel;

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

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

#ifndef _SV_TABDLG_HXX
#define _SV_TABDLG_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/dialog.hxx>

class FixedLine;
class TabControl;

// ----------------------
// - TabDialog -
// ----------------------

class VCL_DLLPUBLIC TabDialog : public Dialog
{
private:
    FixedLine*          mpFixedLine;
    Window*             mpViewWindow;
    WindowAlign         meViewAlign;
    sal_Bool                mbPosControls;

    SAL_DLLPRIVATE void ImplInitTabDialogData();
    SAL_DLLPRIVATE void ImplPosControls();

public:
                        TabDialog( Window* pParent,
                                   WinBits nStyle = WB_STDTABDIALOG );
                        TabDialog( Window* pParent, const ResId& rResId );
                        ~TabDialog();

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );

    void                AdjustLayout();

    void                SetViewWindow( Window* pWindow ) { mpViewWindow = pWindow; }
    Window*             GetViewWindow() const { return mpViewWindow; }
    void                SetViewAlign( WindowAlign eAlign ) { meViewAlign = eAlign; }
    WindowAlign         GetViewAlign() const { return meViewAlign; }
};

#endif  // _SV_TABDLG_HXX

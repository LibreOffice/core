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
#ifndef _SVTOOLS_TEMPLDLG_HXX
#define _SVTOOLS_TEMPLDLG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

namespace binfilter
{

struct SvtTmplDlg_Impl;

// class SvtDocumentTemplateDialog ---------------------------------------

class SvtTemplateWindow;

class  SvtDocumentTemplateDialog : public ModalDialog
{
private:
    FixedLine			aLine;
    PushButton			aManageBtn;
    PushButton			aEditBtn;
    OKButton			aOKBtn;
    CancelButton		aCancelBtn;
    HelpButton			aHelpBtn;

    SvtTmplDlg_Impl*	pImpl;

    DECL_DLLPRIVATE_LINK(			SelectHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(			DoubleClickHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(			NewFolderHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(			SendFocusHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(			OKHdl_Impl, PushButton* );
    DECL_DLLPRIVATE_LINK(			OrganizerHdl_Impl, PushButton* );
    DECL_DLLPRIVATE_LINK(			UpdateHdl_Impl, Timer* );

public:
    SvtDocumentTemplateDialog( Window* pParent );

    /** ctor for calling the dialog for <em>selection</em> only, not for <em>opening</em> a document
        <p>If you use this ctor, the dialog will behave differently in the following areas:
        <ul><li>The <em>Edit</em> button will be hidden.</li>
            <li>Upon pressing em>Open</em>, the selected file will not be opened. Instead, it's
                URL is available (see <method>GetSelectedFileURL</method>).</li>
        </ul>

    */
    struct SelectOnly { };
    SvtDocumentTemplateDialog( Window* _pParent, SelectOnly );

    ~SvtDocumentTemplateDialog();

    sal_Bool	IsFileSelected( ) const;
    String		GetSelectedFileURL( ) const;

    void        SelectTemplateFolder();

private:
    void InitImpl( );
    sal_Bool CanEnableEditBtn() const;
};

}

#endif // _SVTOOLS_TEMPLDLG_HXX


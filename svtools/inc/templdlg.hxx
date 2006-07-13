/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: templdlg.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 12:06:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVTOOLS_TEMPLDLG_HXX
#define _SVTOOLS_TEMPLDLG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
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

struct SvtTmplDlg_Impl;

// class SvtDocumentTemplateDialog ---------------------------------------

class SvtTemplateWindow;

class SVT_DLLPUBLIC SvtDocumentTemplateDialog : public ModalDialog
{
private:
    FixedLine           aLine;
    PushButton          aManageBtn;
    PushButton          aEditBtn;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    SvtTmplDlg_Impl*    pImpl;

    DECL_DLLPRIVATE_LINK(           SelectHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(           DoubleClickHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(           NewFolderHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(           SendFocusHdl_Impl, SvtTemplateWindow* );
    DECL_DLLPRIVATE_LINK(           OKHdl_Impl, PushButton* );
    DECL_DLLPRIVATE_LINK(           OrganizerHdl_Impl, PushButton* );
    DECL_DLLPRIVATE_LINK(           UpdateHdl_Impl, Timer* );

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

    sal_Bool    IsFileSelected( ) const;
    String      GetSelectedFileURL( ) const;

    void        SelectTemplateFolder();

private:
    SVT_DLLPRIVATE void InitImpl( );
    SVT_DLLPRIVATE sal_Bool CanEnableEditBtn() const;
};

#endif // _SVTOOLS_TEMPLDLG_HXX


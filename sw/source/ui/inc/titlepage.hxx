/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _SWTITLEPAGE_HXX
#define _SWTITLEPAGE_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <numberingtypelistbox.hxx>

class Window;
class SwWrtShell;
class SwPageDesc;

class SwTitlePageDlg : public SfxModalDialog
{
private:
    RadioButton* m_pUseExistingPagesRB;
    NumericField* m_pPageCountNF;

    RadioButton* m_pDocumentStartRB;
    RadioButton* m_pPageStartRB;
    NumericField* m_pPageStartNF;

    CheckBox* m_pRestartNumberingCB;
    NumericField* m_pRestartNumberingNF;
    CheckBox* m_pSetPageNumberCB;
    NumericField* m_pSetPageNumberNF;

    ListBox* m_pPagePropertiesLB;
    PushButton* m_pPagePropertiesPB;

    OKButton* m_pOkPB;

    SwWrtShell *mpSh;

    const SwFmtPageDesc *mpPageFmtDesc;

    SwPageDesc *mpTitleDesc;
    const SwPageDesc *mpIndexDesc;
    const SwPageDesc *mpNormalDesc;

    void FillList();

    sal_uInt16 GetInsertPosition() const;

    DECL_LINK( OKHdl, Button * );
    DECL_LINK( EditHdl, Button * );
    DECL_LINK(RestartNumberingHdl, void *);
    DECL_LINK(SetPageNumberHdl, void *);
    DECL_LINK(UpHdl, void *);
    DECL_LINK(DownHdl, void *);
    DECL_LINK(StartPageHdl, void *);
public:
    SwTitlePageDlg( Window *pParent );
    ~SwTitlePageDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

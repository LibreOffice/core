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

#ifndef SC_SORTDLG_HXX
#define SC_SORTDLG_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include "scui_def.hxx"

class ScSortDlg : public SfxTabDialog
{
public:
    ScSortDlg( Window*           pParent,
            const SfxItemSet* pArgSet );
    virtual ~ScSortDlg();

    void    SetHeaders( sal_Bool bHeaders );
    void    SetByRows ( sal_Bool bByRows );
    sal_Bool    GetHeaders() const;
    sal_Bool    GetByRows () const;


private:
    sal_Bool    bIsHeaders;
    sal_Bool    bIsByRows;
};

inline void ScSortDlg::SetHeaders( sal_Bool bHeaders )  { bIsHeaders = bHeaders; }
inline void ScSortDlg::SetByRows ( sal_Bool bByRows  )  { bIsByRows = bByRows; }
inline sal_Bool ScSortDlg::GetHeaders() const           { return bIsHeaders; }
inline sal_Bool ScSortDlg::GetByRows () const           { return bIsByRows; }

class ScSortWarningDlg : public ModalDialog
{
public:
    ScSortWarningDlg( Window* pParent, const String& rExtendText,const String& rCurrentText );
            ~ScSortWarningDlg();
    DECL_LINK( BtnHdl, PushButton* );
private:
    FixedText       aFtText;
    FixedText       aFtTip;
    PushButton      aBtnExtSort;
    PushButton      aBtnCurSort;
    CancelButton    aBtnCancel;
};

#endif // SC_SORTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

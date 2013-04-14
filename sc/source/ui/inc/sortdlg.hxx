/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    FixedText*       aFtText;
    PushButton*      aBtnExtSort;
    PushButton*      aBtnCurSort;
};

#endif // SC_SORTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

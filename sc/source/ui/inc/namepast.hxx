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

#ifndef SC_NAMEPAST_HXX
#define SC_NAMEPAST_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include "namemgrtable.hxx"

#include <vector>

#include "scui_def.hxx"
class ScRangeName;
class ScDocShell;

class ScNamePasteDlg : public ModalDialog
{
    DECL_LINK( ButtonHdl, Button * );

private:
    PushButton      maBtnPasteAll;
    PushButton      maBtnPaste;
    HelpButton      maHelpButton;
    PushButton      maBtnClose;
    FixedLine       maFlDiv;

    ScRangeManagerCtrl maCtrl;

    ScRangeManagerTable* mpTable;

    std::vector<rtl::OUString> maSelectedNames;
    boost::ptr_map<rtl::OUString, ScRangeName> maRangeMap;
public:
    ScNamePasteDlg( Window * pParent, ScDocShell* pShell, bool bInsList=true );

    virtual ~ScNamePasteDlg();

    std::vector<rtl::OUString> GetSelectedNames() const;
    bool            IsAllSelected() const;
};


#endif //SC_NAMEPAST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

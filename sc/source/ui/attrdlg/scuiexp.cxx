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

#undef SC_DLLIMPLEMENTATION

#include "scdlgfact.hxx"
#include "corodlg.hxx"
#include "condformatmgr.hxx"
#include "dapitype.hxx"
#include "dapidata.hxx"
#include "datafdlg.hxx"
#include "delcodlg.hxx"
#include "delcldlg.hxx"
#include "dpgroupdlg.hxx"
#include "filldlg.hxx"
#include "groupdlg.hxx"
#include "linkarea.hxx"
#include "lbseldlg.hxx"
#include "inscldlg.hxx"
#include "instbdlg.hxx"
#include "inscodlg.hxx"
#include "mtrindlg.hxx"
#include "mvtabdlg.hxx"
#include "namecrea.hxx"
#include "namepast.hxx"
#include "pfiltdlg.hxx"
#include "pvfundlg.hxx"
#include "shtabdlg.hxx"
#include "scendlg.hxx"
#include "scuiasciiopt.hxx"
#include "scuiautofmt.hxx"
#include "scuiimoptdlg.hxx"
#include "sortdlg.hxx"
#include "strindlg.hxx"
#include "tabbgcolordlg.hxx"
#include "textimportoptions.hxx"
#include <sal/types.h>

namespace scui
{
    static ScAbstractDialogFactory_Impl* pFactory=nullptr;
    ScAbstractDialogFactory_Impl* GetFactory()
    {
        if ( !pFactory )
            pFactory = new ScAbstractDialogFactory_Impl;
        //if ( !pSwResMgr)
        //  ScDialogsResMgr::GetResMgr();
        return pFactory;
    }
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT ScAbstractDialogFactory* ScCreateDialogFactory()
    {
        return ::scui::GetFactory();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

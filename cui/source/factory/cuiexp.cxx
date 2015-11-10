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

#include "sfx2/basedlgs.hxx"
#include "cuigaldlg.hxx"
#include "hlmarkwn.hxx"
#include "srchxtra.hxx"
#include "cuisrchdlg.hxx"
#include "transfrm.hxx"
#include "labdlg.hxx"
#include "cuitbxform.hxx"
#include "optdict.hxx"
#include "dlgname.hxx"
#include "cuiimapwnd.hxx"
#include "multipat.hxx"
#include "cuifmsearch.hxx"
#include "cuigrfflt.hxx"
#include "cuitabarea.hxx"
#include "insdlg.hxx"
#include "pastedlg.hxx"
#include "linkdlg.hxx"
#include "postdlg.hxx"
#include "passwdomdlg.hxx"
#include "cuihyperdlg.hxx"
#include "selector.hxx"
#include "SpellDialog.hxx"
#include "zoom.hxx"
#include "showcols.hxx"
#include "hyphen.hxx"
#include "thesdlg.hxx"
#include "hangulhanjadlg.hxx"
#include "dstribut.hxx"
#include "dlgfact.hxx"
#include "sal/types.h"

namespace cui
{
    static AbstractDialogFactory_Impl* pFactory=nullptr;
    AbstractDialogFactory_Impl* GetFactory()
    {
        if ( !pFactory )
            pFactory = new AbstractDialogFactory_Impl;
        return pFactory;
    }
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT VclAbstractDialogFactory* CreateDialogFactory()
    {
        return ::cui::GetFactory();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

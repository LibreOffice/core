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

#include "swdlgfact.hxx"

#include <abstract.hxx>
#include <ascfldlg.hxx>
#include <break.hxx>
#include <colwd.hxx>
#include <convert.hxx>
#include <srtdlg.hxx>
#include <dbinsdlg.hxx>
#include <DropDownFieldDialog.hxx>
#include <DropDownFormFieldDialog.hxx>
#include <fldtdlg.hxx>
#include <glossary.hxx>
#include <inpdlg.hxx>
#include <insfnote.hxx>
#include <instable.hxx>
#include <javaedit.hxx>
#include <label.hxx>
#include <mailmrge.hxx>
#include <mailmergewizard.hxx>
#include <mergetbl.hxx>
#include <multmrk.hxx>
#include <regionsw.hxx>
#include <rowht.hxx>
#include <selglos.hxx>
#include <splittbl.hxx>
#include <tautofmt.hxx>
#include <swmodalredlineacceptdlg.hxx>
#include <swrenamexnameddlg.hxx>
#include <swuiidxmrk.hxx>
#include <swuicnttab.hxx>
#include <wordcountdialog.hxx>
#include <swuiexp.hxx>

namespace swui
{
    SwAbstractDialogFactory& GetFactory()
    {
        static SwAbstractDialogFactory_Impl aFactory;
        return aFactory;
    }
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT SwAbstractDialogFactory* SwCreateDialogFactory()
    {
        return &::swui::GetFactory();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

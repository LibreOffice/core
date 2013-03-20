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

#pragma once
#if 1

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>

//------------------------------------------------------------------------

class ScDataPilotSourceTypeDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    RadioButton     aBtnSelection;
    RadioButton     aBtnNamedRange;
    RadioButton     aBtnDatabase;
    RadioButton     aBtnExternal;
    ListBox         aLbNamedRange;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotSourceTypeDlg( Window* pParent, sal_Bool bEnableExternal );
            ~ScDataPilotSourceTypeDlg();

    bool IsDatabase() const;
    bool IsExternal() const;
    bool IsNamedRange() const;
    rtl::OUString GetSelectedNamedRange() const;
    void AppendNamedRange(const rtl::OUString& rNames);

private:
    DECL_LINK( RadioClickHdl, RadioButton * );
};


class ScDataPilotServiceDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    FixedText       aFtService;
    ListBox         aLbService;
    FixedText       aFtSource;
    Edit            aEdSource;
    FixedText       aFtName;
    Edit            aEdName;
    FixedText       aFtUser;
    Edit            aEdUser;
    FixedText       aFtPasswd;
    Edit            aEdPasswd;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotServiceDlg( Window* pParent,
                    const com::sun::star::uno::Sequence<rtl::OUString>& rServices );
            ~ScDataPilotServiceDlg();

    String  GetServiceName() const;
    String  GetParSource() const;
    String  GetParName() const;
    String  GetParUser() const;
    String  GetParPass() const;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

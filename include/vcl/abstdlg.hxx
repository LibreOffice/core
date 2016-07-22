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
#ifndef INCLUDED_VCL_ABSTDLG_HXX
#define INCLUDED_VCL_ABSTDLG_HXX

#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>

namespace vcl { class Window; }
class ResId;
class Dialog;

class VCL_DLLPUBLIC VclAbstractDialog
{
public:
    virtual             ~VclAbstractDialog();

    virtual short       Execute() = 0;
};

class VCL_DLLPUBLIC VclAbstractDialog2
{
public:
    virtual             ~VclAbstractDialog2();

    virtual void        StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) = 0;
    virtual long        GetResult() = 0;
};

class VCL_DLLPUBLIC VclAbstractTerminatedDialog : public VclAbstractDialog
{
public:
    virtual void        EndDialog(long nResult) = 0;
};

class VCL_DLLPUBLIC VclAbstractRefreshableDialog : public VclAbstractDialog
{
public:
    virtual void        Update() = 0;
};

class VCL_DLLPUBLIC AbstractPasswordToOpenModifyDialog : public VclAbstractDialog
{
public:
    virtual OUString    GetPasswordToOpen() const   = 0;
    virtual OUString    GetPasswordToModify() const = 0;
    virtual bool        IsRecommendToOpenReadonly() const = 0;
};

class VCL_DLLPUBLIC VclAbstractDialogFactory
{
public:
    virtual             ~VclAbstractDialogFactory();    // needed for export of vtable
    static VclAbstractDialogFactory* Create();
    // nDialogId was previously a ResId without ResMgr; the ResourceId is now
    // an implementation detail of the factory
    virtual VclAbstractDialog* CreateVclDialog( vcl::Window* pParent, sal_uInt32 nResId ) = 0;

    // creates instance of PasswordToOpenModifyDialog from cui
    virtual AbstractPasswordToOpenModifyDialog* CreatePasswordToOpenModifyDialog( vcl::Window * pParent, sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

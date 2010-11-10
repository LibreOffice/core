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
#ifndef _VCL_ABSTDLG_HXX
#define _VCL_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <tools/string.hxx>
#include <vcl/dllapi.h>

class Window;
class ResId;
class Link;

class VCL_DLLPUBLIC VclAbstractDialog
{
public:
    virtual short           Execute() = 0;
    //virtual void          Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = 0 ) = 0;
    virtual                 ~VclAbstractDialog();
};

class VCL_DLLPUBLIC VclAbstractDialog2
{
public:
    virtual void            StartExecuteModal( const Link& rEndDialogHdl ) = 0;
    virtual long            GetResult() = 0;
    virtual                 ~VclAbstractDialog2();
};

class VCL_DLLPUBLIC VclAbstractTerminatedDialog : public VclAbstractDialog
{
public:
    virtual void            EndDialog(long nResult =0) = 0;
};

class VCL_DLLPUBLIC VclAbstractRefreshableDialog : public VclAbstractDialog
{
public:
    virtual void            Update() = 0;
    virtual void            Sync() = 0;
};

class VCL_DLLPUBLIC AbstractPasswordToOpenModifyDialog : public VclAbstractDialog
{
public:
    virtual String  GetPasswordToOpen() const   = 0;
    virtual String  GetPasswordToModify() const = 0;
    virtual bool    IsRecommendToOpenReadonly() const = 0;
};

//-------------------------------------------------------------

class VCL_DLLPUBLIC VclAbstractDialogFactory
{
public:
                                        virtual ~VclAbstractDialogFactory();    // needed for export of vtable
    static VclAbstractDialogFactory*    Create();
    // nDialogId was previously a ResId without ResMgr; the ResourceId is now
    // an implementation detail of the factory
    virtual VclAbstractDialog*          CreateVclDialog( Window* pParent, sal_uInt32 nResId ) = 0;

    // creates instance of PasswordToOpenModifyDialog from cui
    virtual AbstractPasswordToOpenModifyDialog *    CreatePasswordToOpenModifyDialog( Window * pParent, sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify ) = 0;
};

#endif


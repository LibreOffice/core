/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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


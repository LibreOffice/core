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

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclptr.hxx>
#include <vcl/vclreferencebase.hxx>
#include <vector>
#include <functional>
#include <memory>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/XModel.hpp>

namespace vcl { class Window; }
class Dialog;
class Bitmap;
class BitmapEx;
namespace weld
{
    class DialogController;
    class Window;
}

/**
* Some things multiple-inherit from VclAbstractDialog and OutputDevice,
* so we need to use virtual inheritance to keep the referencing counting
* OK.
*/
class VCL_DLLPUBLIC VclAbstractDialog : public virtual VclReferenceBase
{
protected:
    virtual             ~VclAbstractDialog() override;
public:
    virtual short       Execute() = 0;

    struct AsyncContext {
        VclPtr<VclReferenceBase> mxOwner;
        std::shared_ptr<weld::DialogController> mxOwnerDialog;
        std::function<void(sal_Int32)> maEndDialogFn;
        bool isSet() { return !!maEndDialogFn; }
    };

    bool StartExecuteAsync(const std::function<void(sal_Int32)> &rEndDialogFn)
    {
        AsyncContext aCtx;
        aCtx.mxOwner = this;
        aCtx.maEndDialogFn = rEndDialogFn;
        return StartExecuteAsync(aCtx);
    }

    /// Commence execution of a modal dialog.
    virtual bool StartExecuteAsync(AsyncContext &);

    // Screenshot interface
    virtual std::vector<OString> getAllPageUIXMLDescriptions() const;
    virtual bool selectPageByUIXMLDescription(const OString& rUIXMLDescription);
    virtual BitmapEx createScreenshot() const;
    virtual OString GetScreenshotId() const { return OString(); };
};

class VCL_DLLPUBLIC VclAbstractDialog2 : public virtual VclReferenceBase
{
protected:
    virtual             ~VclAbstractDialog2() override;
public:
    virtual void        StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) = 0;
    virtual sal_Int32   GetResult() = 0;
};

class VCL_DLLPUBLIC VclAbstractTerminatedDialog : public VclAbstractDialog
{
protected:
    virtual             ~VclAbstractTerminatedDialog() override = default;
public:
    virtual void        EndDialog(sal_Int32 nResult) = 0;
};

class VCL_DLLPUBLIC VclAbstractRefreshableDialog : public VclAbstractDialog
{
protected:
    virtual             ~VclAbstractRefreshableDialog() override = default;
public:
    virtual void        Update() = 0;
};

class VCL_DLLPUBLIC AbstractPasswordToOpenModifyDialog : public VclAbstractDialog
{
protected:
    virtual             ~AbstractPasswordToOpenModifyDialog() override = default;
public:
    virtual OUString    GetPasswordToOpen() const   = 0;
    virtual OUString    GetPasswordToModify() const = 0;
    virtual bool        IsRecommendToOpenReadonly() const = 0;
};

class VCL_DLLPUBLIC AbstractScreenshotAnnotationDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScreenshotAnnotationDlg() override = default;
};

class VCL_DLLPUBLIC AbstractSignatureLineDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractSignatureLineDialog() override = default;
};

class VCL_DLLPUBLIC AbstractSignSignatureLineDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractSignSignatureLineDialog() override = default;
};

class VCL_DLLPUBLIC VclAbstractDialogFactory
{
public:
    virtual             ~VclAbstractDialogFactory();    // needed for export of vtable
    static VclAbstractDialogFactory* Create();
    // The Id is an implementation detail of the factory
    virtual VclPtr<VclAbstractDialog> CreateVclDialog(vcl::Window* pParent, sal_uInt32 nId) = 0;

    // creates instance of PasswordToOpenModifyDialog from cui
    virtual VclPtr<AbstractPasswordToOpenModifyDialog> CreatePasswordToOpenModifyDialog(weld::Window * pParent, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify) = 0;

    // creates instance of SignatureDialog from cui
    virtual VclPtr<AbstractSignatureLineDialog>
    CreateSignatureLineDialog(weld::Window* pParent,
                              const css::uno::Reference<css::frame::XModel> xModel,
                              bool bEditExisting)
        = 0;

    // creates instance of SignSignatureDialog from cui
    virtual VclPtr<AbstractSignSignatureLineDialog>
    CreateSignSignatureLineDialog(weld::Window* pParent,
                                  const css::uno::Reference<css::frame::XModel> xModel)
        = 0;

    // creates instance of ScreenshotAnnotationDlg from cui
    virtual VclPtr<AbstractScreenshotAnnotationDlg> CreateScreenshotAnnotationDlg(
        vcl::Window* pParent,
        Dialog& rParentDialog) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef _ACCESSIBLESVXFINDREPLACEDIALOG_HXX_
#define _ACCESSIBLESVXFINDREPLACEDIALOG_HXX_

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <toolkit/awt/vclxwindows.hxx>


class VCLXAccessibleSvxFindReplaceDialog : public VCLXAccessibleComponent
{
public:
    VCLXAccessibleSvxFindReplaceDialog(VCLXWindow* pVCLXindow);
    virtual ~VCLXAccessibleSvxFindReplaceDialog();
    virtual void FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet ) override;
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

};


class VCLXSvxFindReplaceDialog : public VCLXDialog
{
public:
    VCLXSvxFindReplaceDialog(vcl::Window* pSplDlg)
    {
        SetWindow(pSplDlg);
    }
    virtual ~VCLXSvxFindReplaceDialog()
    {};
private:
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >  CreateAccessibleContext() override
    {
        return new VCLXAccessibleSvxFindReplaceDialog(this);
    }
};
#endif // _ACCESSIBLESVXFINDREPLACEDIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

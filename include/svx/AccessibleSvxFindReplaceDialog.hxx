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
    virtual void FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception);

};


class VCLXSvxFindReplaceDialog : public VCLXDialog
{
public:
    VCLXSvxFindReplaceDialog(Window* pSplDlg)
    {
        SetWindow(pSplDlg);
    }
    virtual ~VCLXSvxFindReplaceDialog()
    {};
private:
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext >  CreateAccessibleContext()
    {
        return new VCLXAccessibleSvxFindReplaceDialog(this);
    }
};
#endif // _ACCESSIBLESVXFINDREPLACEDIALOG_HXX_

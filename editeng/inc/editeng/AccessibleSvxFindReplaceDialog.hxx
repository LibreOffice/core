/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
*  Copyright IBM Corporation 2010.
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
#ifndef _ACCESSIBLESVXFINDREPLACEDIALOG_HXX_
#define _ACCESSIBLESVXFINDREPLACEDIALOG_HXX_
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX_
#include <toolkit/awt/vclxwindows.hxx>
#endif


class VCLXAccessibleSvxFindReplaceDialog : public VCLXAccessibleComponent
{
public:
    VCLXAccessibleSvxFindReplaceDialog(VCLXWindow* pVCLXindow);
    virtual ~VCLXAccessibleSvxFindReplaceDialog();
    virtual void FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

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

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

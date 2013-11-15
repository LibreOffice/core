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

#ifndef _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_
#define _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include "toolkit/awt/vclxwindow.hxx"
#endif
class HeaderBar;

//  ----------------------------------------------------
//  class VCLXAccessibleHeaderBar
//  ----------------------------------------------------

typedef std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > >
    ListItems;

class VCLXAccessibleHeaderBar : public VCLXAccessibleComponent
{

public:
    HeaderBar*  m_pHeadBar;
    virtual ~VCLXAccessibleHeaderBar();

    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    VCLXAccessibleHeaderBar( VCLXWindow* pVCLXindow );

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);


    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

public:
    virtual void SAL_CALL disposing (void);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateChild(sal_Int32 i);

private:
    ListItems m_aAccessibleChildren;


};

class VCLXHeaderBar :  public VCLXWindow
{
public:
    VCLXHeaderBar(Window* pHeaderBar);
    virtual ~VCLXHeaderBar();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > CreateAccessibleContext();

};

#endif // _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_


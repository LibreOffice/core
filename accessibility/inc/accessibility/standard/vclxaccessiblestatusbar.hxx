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



#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBAR_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBAR_HXX

#include <toolkit/awt/vclxaccessiblecomponent.hxx>

#include <vector>

class StatusBar;

//  ----------------------------------------------------
//  class VCLXAccessibleStatusBar
//  ----------------------------------------------------

class VCLXAccessibleStatusBar : public VCLXAccessibleComponent
{
private:
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > AccessibleChildren;

    AccessibleChildren      m_aAccessibleChildren;
    StatusBar*              m_pStatusBar;

protected:
    void                    UpdateShowing( sal_Int32 i, sal_Bool bShowing );
    void                    UpdateItemName( sal_Int32 i );
    void                    UpdateItemText( sal_Int32 i );

    void                    InsertChild( sal_Int32 i );
    void                    RemoveChild( sal_Int32 i );

    virtual void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL   disposing();

public:
    VCLXAccessibleStatusBar( VCLXWindow* pVCLXWindow );
    ~VCLXAccessibleStatusBar();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
};


#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBAR_HXX


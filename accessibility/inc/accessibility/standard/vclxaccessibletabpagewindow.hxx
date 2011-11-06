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



#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETABPAGEWINDOW_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLETABPAGEWINDOW_HXX

#include <toolkit/awt/vclxaccessiblecomponent.hxx>


class TabControl;
class TabPage;


//  ----------------------------------------------------
//  class VCLXAccessibleTabPageWindow
//  ----------------------------------------------------

class VCLXAccessibleTabPageWindow : public VCLXAccessibleComponent
{
private:
    TabControl*             m_pTabControl;
    TabPage*                m_pTabPage;
    sal_uInt16              m_nPageId;

protected:
    // OCommonAccessibleComponent
    virtual ::com::sun::star::awt::Rectangle SAL_CALL   implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL   disposing();

public:
    VCLXAccessibleTabPageWindow( VCLXWindow* pVCLXWindow );
    ~VCLXAccessibleTabPageWindow();

    // XAccessibleContext
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
};


#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLETABPAGEWINDOW_HXX

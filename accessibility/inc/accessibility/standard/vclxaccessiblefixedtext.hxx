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



#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDTEXT_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDTEXT_HXX

#include <accessibility/standard/vclxaccessibletextcomponent.hxx>

//  ----------------------------------------------------
//  class VCLXAccessibleFixedText
//  ----------------------------------------------------

class VCLXAccessibleFixedText : public VCLXAccessibleTextComponent
{
protected:
    virtual ~VCLXAccessibleFixedText();

    virtual void FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );
    virtual void implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );

public:
    VCLXAccessibleFixedText( VCLXWindow* pVCLXindow );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEFIXEDTEXT_HXX


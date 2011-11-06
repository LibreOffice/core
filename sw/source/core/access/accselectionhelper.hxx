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


#ifndef _ACCSELECTIONHELPER_HXX_
#define _ACCSELECTIONHELPER_HXX_
class SwAccessibleContext;
class SwFEShell;
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

class SwAccessibleSelectionHelper
{
    /// the context on which this helper works
    SwAccessibleContext& rContext;


    /// get FE-Shell
    SwFEShell* GetFEShell();

    void throwIndexOutOfBoundsException()
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );

public:

    SwAccessibleSelectionHelper( SwAccessibleContext& rContext );
    ~SwAccessibleSelectionHelper();


    //=====  XAccessibleSelection  ============================================

    void selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    sal_Bool isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
    void clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    void selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    sal_Int32 getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    // --> OD 2004-11-16 #111714# - index has to be treated as global child index.
    void deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
};


#endif


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


#ifndef _ACCEMBEDDED_HXX
#define _ACCEMBEDDED_HXX
#include "accnotextframe.hxx"

//IAccessibility2 Implementation 2009-----
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
//-----IAccessibility2 Implementation 2009
class SwAccessibleEmbeddedObject : public   SwAccessibleNoTextFrame
            , public ::com::sun::star::accessibility::XAccessibleExtendedAttributes

{

protected:

    virtual ~SwAccessibleEmbeddedObject();

public:

    SwAccessibleEmbeddedObject( SwAccessibleMap* pInitMap,
                                const SwFlyFrm* pFlyFrm );

    //IAccessibility2 Implementation 2009-----
    //=====  XInterface  ======================================================

    virtual com::sun::star::uno::Any SAL_CALL
        queryInterface (const com::sun::star::uno::Type & rType)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        acquire (void)
        throw ();

    virtual void SAL_CALL
        release (void)
        throw ();
    //-----IAccessibility2 Implementation 2009
    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ====================================================
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //IAccessibility2 Implementation 2009-----
    // ====== XAccessibleExtendedAttributes =====================================
        virtual ::com::sun::star::uno::Any SAL_CALL getExtendedAttributes()
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) ;
    //-----IAccessibility2 Implementation 2009
};


#endif

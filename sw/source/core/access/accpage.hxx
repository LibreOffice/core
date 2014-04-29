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


#ifndef _ACCPAGE_HXX
#define _ACCPAGE_HXX

#ifndef _ACCCONTEXT_HXX
#include "acccontext.hxx"
#endif


/**
 * accessibility implementation for the page (SwPageFrm)
 * The page is _only_ visible in the page preview. For the regular
 * document view, it doesn't make sense to add this additional element
 * into the hierarchy. For the page preview, however, the page is the
 * important.
 */
class SwAccessiblePage : public SwAccessibleContext
{
    sal_Bool    bIsSelected;    // protected by base class mutex

    sal_Bool    IsSelected();

    using SwAccessibleFrame::GetBounds;

protected:

    // return the bounding box for the page in page preview mode
    SwRect GetBounds( /* const SwFrm *pFrm =0 */ );

    // Set states for getAccessibleStateSet.
    // This drived class additionally sets
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

    virtual ~SwAccessiblePage();

public:
    // convenience constructor to avoid typecast;
    // may only be called with SwPageFrm argument
    SwAccessiblePage( SwAccessibleMap* pInitMap, const SwFrm* pFrame );



    //
    // XAccessibleContext methods that need to be overridden
    //

    virtual ::rtl::OUString SAL_CALL getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    //
    // XServiceInfo
    //

    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService (
        const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ====================================================
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool HasCursor();   // required by map to remember that object
};

#endif


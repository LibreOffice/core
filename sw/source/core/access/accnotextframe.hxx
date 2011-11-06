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


#ifndef _ACCNOTEXTFRAME_HXX
#define _ACCNOTEXTFRAME_HXX
#include "accframebase.hxx"
#include <com/sun/star/accessibility/XAccessibleImage.hpp>

class SwFlyFrm;
class SwNoTxtNode;

class SwAccessibleNoTextFrame : public  SwAccessibleFrameBase,
                                public ::com::sun::star::accessibility::XAccessibleImage
{
    SwDepend        aDepend;
    // --> OD 2009-07-14 #i73249#
    ::rtl::OUString msTitle;
    // <--
    ::rtl::OUString msDesc;

protected:

    virtual ~SwAccessibleNoTextFrame();

    const SwNoTxtNode *GetNoTxtNode() const;

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:

    SwAccessibleNoTextFrame( SwAccessibleMap* pInitMap,
                             sal_Int16 nInitRole,
                             const SwFlyFrm *pFlyFrm );

    //=====  XAccessibleContext  ==============================================

    // --> OD 2009-07-14 #i73249#
    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // <--

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XInterface  ======================================================

    // XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAcessibleContext and
    // XAccessibleImage.

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire(  ) throw ()
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw ()
        { SwAccessibleContext::release(); };

    //====== XTypeProvider ====================================================
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleImage  ================================================

    virtual ::rtl::OUString SAL_CALL
        getAccessibleImageDescription(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
        getAccessibleImageHeight(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
        getAccessibleImageWidth(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( sal_Bool bRecursive = sal_False );
};


#endif

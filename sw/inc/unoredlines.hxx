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


#ifndef _UNOREDLINES_HXX
#define _UNOREDLINES_HXX

#include <unocoll.hxx>
#include <unobaseclass.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>

class SwRedline;
namespace com{ namespace sun{ namespace star{
        namespace beans{
            class XPropertySet;
        }
}}}
typedef
cppu::WeakImplHelper3
<
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::lang::XServiceInfo
>
SwRedlinesBaseClass;
class SwXRedlines : public SwRedlinesBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXRedlines();
public:
    SwXRedlines(SwDoc* pDoc);


    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::beans::XPropertySet*           GetObject( SwRedline& rRedline, SwDoc& rDoc );
};
/* -----------------------------12.01.01 14:58--------------------------------

 ---------------------------------------------------------------------------*/
class SwXRedlineEnumeration
    : public SwSimpleEnumeration_Base
    , public SwClient
{
    SwDoc* pDoc;
    sal_uInt16 nCurrentIndex;
protected:
    virtual ~SwXRedlineEnumeration();
public:
    SwXRedlineEnumeration(SwDoc& rDoc);


    //XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
protected:
    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};


#endif


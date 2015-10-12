/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_UNOREDLINES_HXX
#define INCLUDED_SW_INC_UNOREDLINES_HXX

#include <unocoll.hxx>
#include <unobaseclass.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>

class SwRangeRedline;
namespace com{ namespace sun{ namespace star{
        namespace beans{
            class XPropertySet;
        }
}}}
typedef
cppu::WeakImplHelper
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
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XEnumerationAccess - was: XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    static ::com::sun::star::beans::XPropertySet*           GetObject( SwRangeRedline& rRedline, SwDoc& rDoc );
};

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
    virtual sal_Bool SAL_CALL hasMoreElements() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement() throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
protected:
    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

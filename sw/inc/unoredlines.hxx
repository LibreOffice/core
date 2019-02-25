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

#include <sal/config.h>

#include "docary.hxx"
#include "unocoll.hxx"
#include "unobaseclass.hxx"
#include <svl/listener.hxx>
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
    css::container::XIndexAccess,
    css::container::XEnumerationAccess,
    css::lang::XServiceInfo
>
SwRedlinesBaseClass;
class SwXRedlines : public SwRedlinesBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXRedlines() override;
public:
    SwXRedlines(SwDoc* pDoc);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XEnumerationAccess - was: XParagraphEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createEnumeration() override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static css::beans::XPropertySet*           GetObject( SwRangeRedline& rRedline, SwDoc& rDoc );
};

class SwXRedlineEnumeration
    : public SwSimpleEnumeration_Base
    , public SvtListener
{
    SwDoc* pDoc;
    SwRedlineTable::size_type nCurrentIndex;
protected:
    virtual ~SwXRedlineEnumeration() override;
public:
    SwXRedlineEnumeration(SwDoc& rDoc);

    //XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    virtual void Notify( const SfxHint& ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

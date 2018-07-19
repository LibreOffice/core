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

#ifndef INCLUDED_COMPHELPER_ENUMHELPER_HXX
#define INCLUDED_COMPHELPER_ENUMHELPER_HXX

#include <vector>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

struct OEnumerationLock
{
    public:
        ::osl::Mutex m_aLock;
};

/** provides an com.sun.star.container::XEnumeration access based
    on an object implementing the com.sun.star.container::XNameAccess interface
*/
class COMPHELPER_DLLPUBLIC OEnumerationByName : private OEnumerationLock
                         , public ::cppu::WeakImplHelper< css::container::XEnumeration ,
                                                          css::lang::XEventListener    >
{
    css::uno::Sequence< OUString > const                m_aNames;
    sal_Int32                                           m_nPos;
    css::uno::Reference< css::container::XNameAccess >    m_xAccess;
    bool                                            m_bListening;

public:
    OEnumerationByName(const css::uno::Reference< css::container::XNameAccess >& _rxAccess);
    OEnumerationByName(const css::uno::Reference< css::container::XNameAccess >& _rxAccess,
                       const css::uno::Sequence< OUString >&             _aNames  );
    virtual ~OEnumerationByName() override;

    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

private:
    COMPHELPER_DLLPRIVATE void impl_startDisposeListening();
    COMPHELPER_DLLPRIVATE void impl_stopDisposeListening();
};

/** provides an com.sun.star.container::XEnumeration access based
    on an object implementing the com.sun.star.container::XNameAccess interface
*/
class COMPHELPER_DLLPUBLIC OEnumerationByIndex : private OEnumerationLock
                          , public ::cppu::WeakImplHelper< css::container::XEnumeration ,
                                                           css::lang::XEventListener    >
{
    sal_Int32                                         m_nPos;
    css::uno::Reference< css::container::XIndexAccess > m_xAccess;
    bool                                          m_bListening;

public:
    OEnumerationByIndex(const css::uno::Reference< css::container::XIndexAccess >& _rxAccess);
    virtual ~OEnumerationByIndex() override;

    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

private:
    COMPHELPER_DLLPRIVATE void impl_startDisposeListening();
    COMPHELPER_DLLPRIVATE void impl_stopDisposeListening();
};

// this is the way that works for ENABLE_LTO with MSVC 2013
class SAL_DLLPUBLIC_TEMPLATE OAnyEnumeration_BASE
    : public ::cppu::WeakImplHelper<css::container::XEnumeration> {};

/** provides an com.sun.star.container::XEnumeration
    for an outside set vector of Any's.

*/
class COMPHELPER_DLLPUBLIC OAnyEnumeration : private OEnumerationLock
                                           , public OAnyEnumeration_BASE
{
    sal_Int32                         m_nPos;
    css::uno::Sequence< css::uno::Any > m_lItems;

public:
    OAnyEnumeration(const css::uno::Sequence< css::uno::Any >& lItems);
    virtual ~OAnyEnumeration() override;

    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;

};

}

#endif // INCLUDED_COMPHELPER_ENUMHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

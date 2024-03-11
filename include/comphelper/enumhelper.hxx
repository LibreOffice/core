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

#include <config_options.h>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <comphelper/comphelperdllapi.h>
#include <cppuhelper/implbase.hxx>
#include <mutex>
#include <variant>
#include <vector>

namespace com::sun::star::container { class XIndexAccess; }
namespace com::sun::star::container { class XNameAccess; }

namespace comphelper
{

/** provides a com.sun.star.container::XEnumeration access based
    on an object implementing the com.sun.star.container::XNameAccess interface
*/
class UNLESS_MERGELIBS_MORE(COMPHELPER_DLLPUBLIC) OEnumerationByName final :
                         public ::cppu::WeakImplHelper< css::container::XEnumeration ,
                                                          css::lang::XEventListener    >
{
    std::variant<css::uno::Sequence< OUString >, std::vector<OUString>> m_aNames;
    css::uno::Reference< css::container::XNameAccess >  m_xAccess;
    sal_Int32                                           m_nPos;
    bool                                                m_bListening;
    std::mutex m_aLock;

public:
    OEnumerationByName(css::uno::Reference< css::container::XNameAccess > _xAccess);
    OEnumerationByName(css::uno::Reference< css::container::XNameAccess > _xAccess,
                       std::vector<OUString>             _aNames  );
    virtual ~OEnumerationByName() override;

    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

private:
    sal_Int32 getLength() const;
    const OUString& getElement(sal_Int32 nIndex) const;
    COMPHELPER_DLLPRIVATE void impl_startDisposeListening();
    COMPHELPER_DLLPRIVATE void impl_stopDisposeListening();
};

/** provides a com.sun.star.container::XEnumeration access based
    on an object implementing the com.sun.star.container::XNameAccess interface
*/
class COMPHELPER_DLLPUBLIC OEnumerationByIndex final :
                          public ::cppu::WeakImplHelper< css::container::XEnumeration ,
                                                           css::lang::XEventListener    >
{
    css::uno::Reference< css::container::XIndexAccess > m_xAccess;
    sal_Int32                                         m_nPos;
    bool                                          m_bListening;
    std::mutex m_aLock;

public:
    OEnumerationByIndex(css::uno::Reference< css::container::XIndexAccess > _xAccess);
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

/** provides a com.sun.star.container::XEnumeration
    for an outside set vector of Any's.

*/
class UNLESS_MERGELIBS_MORE(COMPHELPER_DLLPUBLIC) OAnyEnumeration final :
                                           public OAnyEnumeration_BASE
{
    sal_Int32                         m_nPos;
    css::uno::Sequence< css::uno::Any > m_lItems;
    std::mutex m_aLock;

public:
    OAnyEnumeration(const css::uno::Sequence< css::uno::Any >& lItems);
    virtual ~OAnyEnumeration() override;

    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;

};

}

#endif // INCLUDED_COMPHELPER_ENUMHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

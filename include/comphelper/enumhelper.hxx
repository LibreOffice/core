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

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#define _COMPHELPER_ENUMHELPER_HXX_

#include <vector>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace starcontainer     = ::com::sun::star::container;
    namespace staruno           = ::com::sun::star::uno;
    namespace starlang          = ::com::sun::star::lang;

//==================================================================
//= OEnumerationLock
//==================================================================
struct OEnumerationLock
{
    public:
        ::osl::Mutex m_aLock;
};

//==================================================================
//= OEnumerationByName
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type> access based
    on an object implementing the <type scope="com.sun.star.container">XNameAccess</type> interface
*/
class COMPHELPER_DLLPUBLIC OEnumerationByName : private OEnumerationLock
                         , public ::cppu::WeakImplHelper2< starcontainer::XEnumeration ,
                                                           starlang::XEventListener    >
{
    staruno::Sequence< OUString >                m_aNames;
    sal_Int32                                           m_nPos;
    staruno::Reference< starcontainer::XNameAccess >    m_xAccess;
    sal_Bool                                            m_bListening;

public:
    OEnumerationByName(const staruno::Reference< starcontainer::XNameAccess >& _rxAccess);
    OEnumerationByName(const staruno::Reference< starcontainer::XNameAccess >& _rxAccess,
                       const staruno::Sequence< OUString >&             _aNames  );
    virtual ~OEnumerationByName();

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

    virtual void SAL_CALL disposing(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);

private:
    COMPHELPER_DLLPRIVATE void impl_startDisposeListening();
    COMPHELPER_DLLPRIVATE void impl_stopDisposeListening();
};

//==================================================================
//= OEnumerationByIndex
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type> access based
    on an object implementing the <type scope="com.sun.star.container">XNameAccess</type> interface
*/
class COMPHELPER_DLLPUBLIC OEnumerationByIndex : private OEnumerationLock
                          , public ::cppu::WeakImplHelper2< starcontainer::XEnumeration ,
                                                            starlang::XEventListener    >
{
    sal_Int32                                         m_nPos;
    staruno::Reference< starcontainer::XIndexAccess > m_xAccess;
    sal_Bool                                          m_bListening;

public:
    OEnumerationByIndex(const staruno::Reference< starcontainer::XIndexAccess >& _rxAccess);
    virtual ~OEnumerationByIndex();

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

    virtual void SAL_CALL disposing(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);

private:
    COMPHELPER_DLLPRIVATE void impl_startDisposeListening();
    COMPHELPER_DLLPRIVATE void impl_stopDisposeListening();
};

//==================================================================
//= OAnyEnumeration
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type>
    for an outside set vector of Any's.

*/
class COMPHELPER_DLLPUBLIC OAnyEnumeration : private OEnumerationLock
                                           , public  ::cppu::WeakImplHelper1< starcontainer::XEnumeration >
{
    sal_Int32                         m_nPos;
    staruno::Sequence< staruno::Any > m_lItems;

public:
    OAnyEnumeration(const staruno::Sequence< staruno::Any >& lItems);
    virtual ~OAnyEnumeration();

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

};

//.........................................................................
}
//... namespace comphelper .......................................................

#endif // _COMPHELPER_ENUMHELPER_HXX_



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

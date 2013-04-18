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
#ifndef _COMPHELPER_WEAK_HXX_
#define _COMPHELPER_WEAK_HXX_

#include "comphelper/comphelperdllapi.h"

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>

namespace comphelper
{
/** Base class to implement an UNO object supporting types and weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    In addition to the features from cppu::OWeakObject, derivations from this class can
    also used as a base class for ::cppu::ImplInheritanceHelper?
*/
class COMPHELPER_DLLPUBLIC OWeakTypeObject : public ::cppu::OWeakObject, public ::com::sun::star::lang::XTypeProvider
{
public:
    OWeakTypeObject();
    virtual ~OWeakTypeObject();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & rType )  throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

};

}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

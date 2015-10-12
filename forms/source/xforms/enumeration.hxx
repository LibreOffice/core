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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_ENUMERATION_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_ENUMERATION_HXX


#include <cppuhelper/implbase.hxx>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace uno { class Any; }
    namespace container { class NoSuchElementException; }
    namespace lang { class WrappedTargetException; }
    namespace uno { class RuntimeException; }
} } }

/** implement XEnumeration based on container::XIndexAccess */
class Enumeration
    : public cppu::WeakImplHelper<css::container::XEnumeration>
{
    css::uno::Reference<css::container::XIndexAccess> mxContainer;
    sal_Int32 mnIndex;

public:
    explicit Enumeration( css::container::XIndexAccess* );

    virtual sal_Bool SAL_CALL hasMoreElements()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL nextElement()
        throw( css::container::NoSuchElementException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

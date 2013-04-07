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

#ifndef INCLUDED_stoc_source_registry_tdprovider_methoddescription_hxx
#define INCLUDED_stoc_source_registry_tdprovider_methoddescription_hxx

#include "functiondescription.hxx"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace reflection {
        class XParameter;
    }
} } }
namespace typereg { class Reader; }

namespace stoc { namespace registry_tdprovider {

class MethodDescription: public FunctionDescription {
public:
    MethodDescription(
        com::sun::star::uno::Reference<
            com::sun::star::container::XHierarchicalNameAccess > const &
            manager,
        OUString const & name,
        com::sun::star::uno::Sequence< sal_Int8 > const & bytes,
        sal_uInt16 index);

    ~MethodDescription();

    OUString getName() const { return m_name; }

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XParameter > >
    getParameters() const;

private:
    MethodDescription(MethodDescription &); // not implemented
    void operator =(MethodDescription); // not implemented

    OUString m_name;
    mutable com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::reflection::XParameter > > m_parameters;
    mutable bool m_parametersInit;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

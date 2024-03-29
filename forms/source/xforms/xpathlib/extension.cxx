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


#include <com/sun/star/xml/xpath/Libxml2ExtensionHandle.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "extension.hxx"
#include "xpathlib.hxx"

namespace com::sun::star::uno {
    class XComponentContext;
}

using namespace com::sun::star::uno;
using namespace com::sun::star::xforms;
using namespace com::sun::star::xml::xpath;
using namespace com::sun::star::beans;

Libxml2ExtensionHandle SAL_CALL CLibxml2XFormsExtension::getLibxml2ExtensionHandle()
{
    Libxml2ExtensionHandle aHandle;
    aHandle.functionLookupFunction = reinterpret_cast< sal_Int64 >( &xforms_lookupFunc );
    aHandle.functionData = reinterpret_cast< sal_Int64 >( this );
    aHandle.variableLookupFunction = sal_Int64(0);
    aHandle.variableData = sal_Int64(0);
    return aHandle;
}

void SAL_CALL CLibxml2XFormsExtension::initialize(const Sequence< Any >& aSequence)
{
    if (aSequence.getLength() == 2
        && (aSequence[0] >>= m_aModel)
        && (aSequence[1] >>= m_aContextNode))
    {
        return;
    }

    NamedValue aValue;
    for (const Any& rArg : aSequence)
    {
        if (! (rArg >>= aValue))
            throw RuntimeException();
        if ( aValue.Name == "Model" )
            aValue.Value >>= m_aModel;
        else if ( aValue.Name == "ContextNode" )
            aValue.Value >>= m_aContextNode;
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_xml_xpath_XFormsExtension_get_implementation(css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new CLibxml2XFormsExtension());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

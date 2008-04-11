/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: extension.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include <stdio.h>
#include <com/sun/star/xml/xpath/Libxml2ExtensionHandle.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "extension.hxx"
#include "xpathlib.hxx"
#include "frm_module.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xforms;
using namespace com::sun::star::xml::xpath;
using namespace com::sun::star::beans;
using com::sun::star::xml::dom::XNode;

Reference< XInterface > SAL_CALL CLibxml2XFormsExtension::Create(
    const Reference< XMultiServiceFactory >& /*aFactory*/)
{
    // printf("_create_\n");
    Reference< XInterface > aInstance(static_cast< XXPathExtension* >(new CLibxml2XFormsExtension(/*aFactory*/)));
    return aInstance;
}

OUString SAL_CALL CLibxml2XFormsExtension::getImplementationName_Static()
{
    // printf("_implname_\n");
    return OUString::createFromAscii("com.sun.star.comp.xml.xpath.XFormsExtension");
}

Sequence< OUString > SAL_CALL CLibxml2XFormsExtension::getSupportedServiceNames_Static()
{
    // printf("_services_\n");
    Sequence< OUString > aSequence(1);
    aSequence[0] = OUString::createFromAscii("com.sun.star.xml.xpath.XPathExtension");
    return aSequence;
}

Libxml2ExtensionHandle SAL_CALL CLibxml2XFormsExtension::getLibxml2ExtensionHandle() throw (RuntimeException)
{
    Libxml2ExtensionHandle aHandle;
    aHandle.functionLookupFunction = reinterpret_cast< sal_Int64 >( &xforms_lookupFunc );
    aHandle.functionData = reinterpret_cast< sal_Int64 >( this );
    aHandle.variableLookupFunction = (sal_Int64)0;
    aHandle.variableData = (sal_Int64)0;
    return aHandle;
}

void SAL_CALL CLibxml2XFormsExtension::initialize(const Sequence< Any >& aSequence) throw (RuntimeException)
{
    NamedValue aValue;
    for (sal_Int32 i = 0; i < aSequence.getLength(); i++)
    {
        if (! (aSequence[i] >>= aValue))
            throw RuntimeException();
        if (aValue.Name.equalsAscii("Model"))
            aValue.Value >>= m_aModel;
        else if (aValue.Name.equalsAscii("ContextNode"))
            aValue.Value >>= m_aContextNode;
    }
}

Reference< XModel > CLibxml2XFormsExtension::getModel()
{
    return m_aModel;
}

Reference< XNode > CLibxml2XFormsExtension::getContextNode()
{
    return m_aContextNode;
}

extern "C" void SAL_CALL createRegistryInfo_CLibxml2XFormsExtension()
{
    static frm::OMultiInstanceAutoRegistration< CLibxml2XFormsExtension >   aRegistration;
}

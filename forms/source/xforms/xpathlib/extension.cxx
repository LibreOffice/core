/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include <stdio.h>
#include <com/sun/star/xml/xpath/Libxml2ExtensionHandle.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "extension.hxx"
#include "xpathlib.hxx"
#include "frm_module.hxx"

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

::rtl::OUString SAL_CALL CLibxml2XFormsExtension::getImplementationName_Static()
{
    // printf("_implname_\n");
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.xml.xpath.XFormsExtension");
}

Sequence< ::rtl::OUString > SAL_CALL CLibxml2XFormsExtension::getSupportedServiceNames_Static()
{
    // printf("_services_\n");
    Sequence< ::rtl::OUString > aSequence(1);
    aSequence[0] = ::rtl::OUString::createFromAscii("com.sun.star.xml.xpath.XPathExtension");
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

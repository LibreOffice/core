#include <stdio.h>
#include <com/sun/star/xml/xpath/Libxml2ExtensionHandle.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "extension.hxx"
#include "xpathlib.hxx"

#ifndef FRM_MODULE_HXX
#include "frm_module.hxx"
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xforms;
using namespace com::sun::star::xml::xpath;
using namespace com::sun::star::beans;
using com::sun::star::xml::dom::XNode;

Reference< XInterface > SAL_CALL CLibxml2XFormsExtension::Create(
    const Reference< XMultiServiceFactory >& aFactory)
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
    aHandle.functionLookupFunction = (sal_Int64)&xforms_lookupFunc;
    aHandle.functionData = (sal_Int64)this;
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

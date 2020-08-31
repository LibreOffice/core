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

#include <sal/config.h>

#include <codemaker/commoncpp.hxx>
#include <codemaker/global.hxx>

#include "skeletoncommon.hxx"
#include "skeletoncpp.hxx"

#include <iostream>

using namespace ::codemaker::cpp;

namespace skeletonmaker::cpp {

static void generateIncludes(std::ostream & o,
         const std::set< OUString >& interfaces,
         const OUString & propertyhelper, const bool serviceobject,
         const bool supportxcomponent)
{
    o << "#include \"sal/config.h\"\n";
    if (serviceobject) {
        o << "#include \"cppuhelper/factory.hxx\"\n"
             "#include \"cppuhelper/implementationentry.hxx\"\n";
    } else {
        o << "#include \"com/sun/star/uno/XComponentContext.hpp\"\n";
    }
    if (supportxcomponent) {
        o << "#include \"cppuhelper/compbase" << interfaces.size() << ".hxx\"\n";
        o << "#include \"cppuhelper/basemutex.hxx\"\n";
    } else {
        o << "#include \"cppuhelper/implbase" << interfaces.size() << ".hxx\"\n";
    }

    if (propertyhelper.getLength() > 1) {
        if (propertyhelper == "_")
            o << "#include \"cppuhelper/rpopshlp.hxx\"\n";
        else
            o << "#include \"cppuhelper/propertysetmixin.hxx\"\n";
    }

    for (const auto& rIface : interfaces)
    {
        o << "#include \""
          << rIface.replace('.', '/')
          << ".hpp\"\n";
    }
}

static short generateNamespace(std::ostream & o,
                        const OString & implname,
                        bool serviceobject,
                        OString & nm)
{
    short count=0;
    sal_Int32 index = implname.lastIndexOf('.');
    if (serviceobject) {
        o << "\n\n// component helper namespace\n";
    } else {
        o << "\n";
    }
    OStringBuffer buf;
    if (index == -1) {
        if (serviceobject) {
            buf.append("comp_");
            buf.append(implname);
            nm = buf.makeStringAndClear();
            o << "namespace comp_" << implname << " {\n\n";
            count=1;
        } else {
            nm.clear();
        }
    } else {
        sal_Int32 nPos=0;
        do {
            OString token(implname.getToken(0, '.', nPos));
            if (nPos < 0 && serviceobject) {
                buf.append("::comp_");
                buf.append(token);
                o << "namespace comp_" << token << " { ";
                count++;
            } else {
                buf.append("::");
                buf.append(token);
                o << "namespace " << token << " { ";
                count++;
            }
        } while( nPos <= index );
        nm = buf.makeStringAndClear();
        o << "\n\n";
    }
    return count;
}

static OString generateCompHelperDeclaration(std::ostream & o,
                                      const OString & implname)
{
    OString nm;
    short nbrackets = generateNamespace(o, implname, true, nm);

    o << "namespace css = ::com::sun::star;\n\n";

    // generate component/service helper functions
    o << "// component and service helper functions:\n"
        "OUString SAL_CALL _getImplementationName();\n"
        "css::uno::Sequence< OUString > SAL_CALL "
        "_getSupportedServiceNames();\n"
        "css::uno::Reference< css::uno::XInterface > SAL_CALL _create("
        " css::uno::Reference< css::uno::XComponentContext > const & "
        "context );\n\n";

    // close namespace
    for (short i=0; i < nbrackets; i++)
        o << "} ";
    o << "// closing component helper namespace\n\n";

    return nm;
}

static void generateCompHelperDefinition(std::ostream & o,
         const OString & implname,
         const OString & classname,
         const std::set< OUString >& services)
{
    OString nm;
    short nbrackets = generateNamespace(o, implname, true, nm);

    o << "OUString SAL_CALL _getImplementationName() {\n"
         "    return OUString(\n"
         "        \"" << implname << "\");\n}\n\n";

    o << "css::uno::Sequence< OUString > SAL_CALL "
        "_getSupportedServiceNames()\n{\n    css::uno::Sequence< "
        "OUString > s(" << services.size() << ");\n";

    short i=0;
    for (const auto& rService : services)
    {
        o << "    s[" << i++ << "] = OUString(\""
          << rService << "\");\n";
    }
    o << "    return s;\n}\n\n";

    o << "css::uno::Reference< css::uno::XInterface > SAL_CALL _create("
         "\n    const css::uno::Reference< css::uno::XComponentContext > & "
         "context)\n{\n"
         "    return static_cast< ::cppu::OWeakObject * >(new "
      << classname <<  "(context));\n}\n\n";

    // close namespace
    for (short j=0; j < nbrackets; j++)
        o << "} ";
    o << "// closing component helper namespace\n\n";

}

static void generateCompFunctions(std::ostream & o, const OString & nmspace)
{
    o << "static ::cppu::ImplementationEntry const entries[] = {\n"
         "    { &" << nmspace << "::_create,\n      &"
      << nmspace << "::_getImplementationName,\n      &"
      << nmspace << "::_getSupportedServiceNames,\n"
         "      &::cppu::createSingleComponentFactory, 0, 0 },\n"
         "    { 0, 0, 0, 0, 0, 0 }\n};\n\n";

    o << "extern \"C\" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(\n"
         "    const char * implName, void * serviceManager, void * registryKey)\n{\n"
         "    return ::cppu::component_getFactoryHelper(\n"
         "        implName, serviceManager, registryKey, entries);\n}\n\n";

    o << "extern \"C\" sal_Bool SAL_CALL component_writeInfo(\n"
         "    void * serviceManager, void * registryKey)\n{\n"
         "    return ::cppu::component_writeInfoHelper("
         "serviceManager, registryKey, entries);\n}\n";
}

void generateXPropertySetBodies(std::ostream& o,
                                const OString & classname,
                                const OString & propertyhelper)
{
    o << "// com.sun.star.beans.XPropertySet:\n";

    o << "css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL "
      << classname << "getPropertySetInfo() throw ("
        "css::uno::RuntimeException)\n{\n    return ::cppu::PropertySetMixin< "
      << propertyhelper
      << " >::getPropertySetInfo();\n}\n\n";

    o << "void SAL_CALL " << classname << "setPropertyValue(const OUString"
        " & aPropertyName, const css::uno::Any & aValue) throw ("
        "css::uno::RuntimeException, css::beans::UnknownPropertyException, "
        "css::beans::PropertyVetoException, css::lang::IllegalArgumentException, "
        "css::lang::WrappedTargetException)\n{\n    ::cppu::PropertySetMixin< "
      << propertyhelper << " >::setPropertyValue(aPropertyName, aValue);\n}\n\n";


    o << "css::uno::Any SAL_CALL " << classname << "getPropertyValue(const "
        "OUString & aPropertyName) throw (css::uno::RuntimeException, "
        "css::beans::UnknownPropertyException, css::lang::WrappedTargetException)"
        "\n{\n    return ::cppu::PropertySetMixin< "
      << propertyhelper << " >::getPropertyValue(aPropertyName);\n}\n\n";

    o << "void SAL_CALL " << classname << "addPropertyChangeListener(const "
        "OUString & aPropertyName, const css::uno::Reference< "
        "css::beans::XPropertyChangeListener > & xListener) throw ("
        "css::uno::RuntimeException, css::beans::UnknownPropertyException, "
        "css::lang::WrappedTargetException)\n{\n    ::cppu::PropertySetMixin< "
      << propertyhelper
      << " >::addPropertyChangeListener(aPropertyName, xListener);\n}\n\n";

    o << "void SAL_CALL " << classname << "removePropertyChangeListener(const "
        "OUString & aPropertyName, const css::uno::Reference< "
        "css::beans::XPropertyChangeListener > & xListener) throw ("
        "css::uno::RuntimeException, css::beans::UnknownPropertyException, "
        "css::lang::WrappedTargetException)\n{\n    ::cppu::PropertySetMixin< "
      << propertyhelper
      << " >::removePropertyChangeListener(aPropertyName, xListener);\n}\n\n";

    o << "void SAL_CALL " << classname << "addVetoableChangeListener(const "
        "OUString & aPropertyName, const css::uno::Reference< "
        "css::beans::XVetoableChangeListener > & xListener) throw ("
        "css::uno::RuntimeException, css::beans::UnknownPropertyException, "
        "css::lang::WrappedTargetException)\n{\n    ::cppu::PropertySetMixin< "
      << propertyhelper
      << " >::addVetoableChangeListener(aPropertyName, xListener);\n}\n\n";

    o << "void SAL_CALL " << classname << "removeVetoableChangeListener(const "
        "OUString & aPropertyName, const css::uno::Reference< "
        "css::beans::XVetoableChangeListener > & xListener) throw ("
        "css::uno::RuntimeException, css::beans::UnknownPropertyException, "
        "css::lang::WrappedTargetException)\n{\n    ::cppu::PropertySetMixin< "
      << propertyhelper
      << " >::removeVetoableChangeListener(aPropertyName, xListener);\n}\n\n";
}

void generateXFastPropertySetBodies(std::ostream& o,
                                    const OString & classname,
                                    const OString & propertyhelper)
{
    o << "// com.sun.star.beans.XFastPropertySet:\n";

    o << "void SAL_CALL " << classname << "setFastPropertyValue( ::sal_Int32 "
        "nHandle, const css::uno::Any& aValue ) throw ("
        "css::beans::UnknownPropertyException, css::beans::PropertyVetoException, "
        "css::lang::IllegalArgumentException, css::lang::WrappedTargetException, "
        "css::uno::RuntimeException)\n{\n    ::cppu::PropertySetMixin< "
      << propertyhelper << " >::setFastPropertyValue(nHandle, aValue);\n}\n\n";


    o << "css::uno::Any SAL_CALL " << classname << "getFastPropertyValue( "
        "::sal_Int32 nHandle ) throw (css::beans::UnknownPropertyException, "
        "css::lang::WrappedTargetException, css::uno::RuntimeException)\n{\n"
        "    return ::cppu::PropertySetMixin< "
      << propertyhelper << " >::getFastPropertyValue(nHandle);\n}\n\n";
}

void generateXPropertyAccessBodies(std::ostream& o,
                                   const OString & classname,
                                   const OString & propertyhelper)
{
    o << "    // com.sun.star.beans.XPropertyAccess:\n";

    o << "css::uno::Sequence< css::beans::PropertyValue > SAL_CALL "
      << classname << "getPropertyValues(  ) throw ("
        "css::uno::RuntimeException)\n{\n"
        "    return ::cppu::PropertySetMixin< "
      << propertyhelper << " >::getPropertyValues();\n}\n\n";

    o << "void SAL_CALL " << classname << "setPropertyValues( const "
        "css::uno::Sequence< css::beans::PropertyValue >& aProps ) throw ("
        "css::beans::UnknownPropertyException, css::beans::PropertyVetoException, "
        "css::lang::IllegalArgumentException, css::lang::WrappedTargetException, "
        "css::uno::RuntimeException)\n{\n"
        "    ::cppu::PropertySetMixin< "
      << propertyhelper << " >::setPropertyValues(aProps);\n}\n\n";
}

void generateXLocalizable(std::ostream& o, const OString & classname)
{
    o << "// css::lang::XLocalizable:\n"
        "void SAL_CALL " << classname << "setLocale(const css::lang::"
        "Locale & eLocale) throw (css::uno::RuntimeException)\n{\n"
        "     m_locale = eLocale;\n}\n\n"
        "css::lang::Locale SAL_CALL " << classname << "getLocale() "
        "throw (css::uno::RuntimeException)\n{\n    return m_locale;\n}\n\n";
}

void generateXAddInBodies(std::ostream& o, const OString & classname)
{
    o << "// css::sheet::XAddIn:\n";

    o << "OUString SAL_CALL " << classname << "getProgrammaticFunctionName("
        "const OUString & aDisplayName) throw (css::uno::RuntimeException)"
        "\n{\n    OUString ret;\n    try {\n        css::uno::Reference< "
        "css::container::XNameAccess > xNAccess(m_xHAccess, css::uno::UNO_QUERY);\n"
        "        css::uno::Sequence< OUString > functions = "
        "xNAccess->getElementNames();\n        sal_Int32 len = functions."
        "getLength();\n        OUString sDisplayName;\n"
        "        for (sal_Int32 i=0; i < len; ++i) {\n"
        "            sDisplayName = getAddinProperty(functions[i], "
        "OUString(),\n                                           "
        "sDISPLAYNAME);\n            if (sDisplayName.equals(aDisplayName))\n"
        "                return functions[i];\n        }\n    }\n"
        "     catch ( const css::uno::RuntimeException & e ) {\n        throw e;\n    }\n"
        "     catch ( css::uno::Exception & ) {\n    }\n    return ret;\n}\n\n";

    o << "OUString SAL_CALL " << classname << "getDisplayFunctionName(const "
        "OUString & aProgrammaticName) throw (css::uno::RuntimeException)\n"
        "{\n    return getAddinProperty(aProgrammaticName, OUString(), "
        "sDISPLAYNAME);\n}\n\n";

    o << "OUString SAL_CALL " << classname << "getFunctionDescription(const "
        "OUString & aProgrammaticName) throw (css::uno::RuntimeException)\n"
        "{\n    return getAddinProperty(aProgrammaticName, OUString(), "
        "sDESCRIPTION);\n}\n\n";

    o << "OUString SAL_CALL " << classname << "getDisplayArgumentName(const "
        "OUString & aProgrammaticFunctionName, ::sal_Int32 nArgument) throw "
        "(css::uno::RuntimeException)\n{\n    return getAddinProperty("
        "aProgrammaticFunctionName,\n                            m_functionMap["
        "aProgrammaticFunctionName][nArgument],\n"
        "                            sDISPLAYNAME);\n}\n\n";

    o << "OUString SAL_CALL " << classname << "getArgumentDescription(const "
        "OUString & aProgrammaticFunctionName, ::sal_Int32 nArgument) throw "
        "(css::uno::RuntimeException)\n{\n    return getAddinProperty("
        "aProgrammaticFunctionName,\n                            "
        "m_functionMap[aProgrammaticFunctionName][nArgument],\n"
        "                            sDESCRIPTION);\n}\n\n";

    o << "OUString SAL_CALL " << classname << "getProgrammaticCategoryName("
        "const OUString & aProgrammaticFunctionName) throw ("
        "css::uno::RuntimeException)\n{\n    return getAddinProperty("
        "aProgrammaticFunctionName, OUString(), sCATEGORY);\n}\n\n";

    o << "OUString SAL_CALL " << classname << "getDisplayCategoryName(const "
        "OUString & aProgrammaticFunctionName) throw ("
        "css::uno::RuntimeException)\n{\n    return getAddinProperty("
        "aProgrammaticFunctionName, OUString(), "
        "sCATEGORYDISPLAYNAME);\n}\n\n";
}

void generateXCompatibilityNamesBodies(std::ostream& o, const OString & classname)
{
    o << "// css::sheet::XCompatibilityNames:\n"
        "css::uno::Sequence< css::sheet::LocalizedName > SAL_CALL " << classname
      << "getCompatibilityNames(const OUString & aProgrammaticName) throw "
        "(css::uno::RuntimeException)\n{\n    css::uno::Sequence< "
        "css::sheet::LocalizedName > seqLocalizedNames;\n    try {\n        "
        "OUStringBuffer buf("
        "aProgrammaticName);\n        buf.appendAscii(\"/CompatibilityName\");\n"
        "        OUString hname(buf.makeStringAndClear());\n\n        "
        "if ( m_xCompAccess->hasByHierarchicalName(hname) ) {\n"
        "            css::uno::Reference< css::container::XNameAccess > "
        "xNameAccess(\n"
        "                m_xCompAccess->getByHierarchicalName(hname), "
        "css::uno::UNO_QUERY);\n\n            css::uno::Sequence< OUString"
        " > elems = \n            xNameAccess->getElementNames();"
        "\n            ::sal_Int32 len = elems.getLength();\n\n            "
        "seqLocalizedNames.realloc(len);\n\n            OUString "
        "sCompatibilityName;\n            for (::sal_Int32 i=0; i < len; ++i) {\n"
        "                OUString sLocale(elems[i]);\n                "
        "xNameAccess->getByName(sLocale) >>= sCompatibilityName;\n\n"
        "                css::lang::Locale aLocale;\n                "
        "::sal_Int32 nIndex = 0, nToken = 0;\n                "
        /* FIXME-BCP47: this will break. */
        "do {\n                    OUString aToken = sLocale.getToken(0, '-', "
        "nIndex);\n                    switch (nToken++) {\n                    "
        "case 0:\n                        aLocale.Language = aToken;\n"
        "                        break;\n                    case 1:\n"
        "                        aLocale.Country = aToken;\n                    "
        "    break;\n                    default:\n                        "
        "aLocale.Variant = sLocale.copy(nIndex-aToken.getLength()-1);\n"
        "                        nIndex = -1;\n                    }\n"
        "                } while ( nIndex >= 0 );\n\n                "
        "seqLocalizedNames[i].Locale = aLocale;\n                "
        "seqLocalizedNames[i].Name = sCompatibilityName;\n            }"
        "\n        }\n    }\n    catch ( const css::uno::RuntimeException & e ) {\n        "
        "throw e;\n    }\n    catch ( css::uno::Exception & ) {\n    }\n\n"
        "    return seqLocalizedNames;\n}\n\n";
}

void generateXInitialization(std::ostream& o, const OString & classname)
{
    o << "// css::lang::XInitialization:\n"
        "void SAL_CALL " << classname << "initialize( const css::uno::Sequence< "
        "css::uno::Any >& aArguments ) "
        "throw (css::uno::Exception, css::uno::RuntimeException)\n{\n"
        "    css::uno::Reference < css::frame::XFrame > xFrame;\n"
        "    if ( aArguments.getLength() ) {\n        aArguments[0] >>= xFrame;\n"
        "        m_xFrame = xFrame;\n    }\n}\n\n";
}

void generateXDispatch(std::ostream& o,
                       const OString & classname,
                       const ProtocolCmdMap & protocolCmdMap)
{
    // com.sun.star.frame.XDispatch
    // dispatch
    o << "// css::frame::XDispatch:\n"
        "void SAL_CALL " << classname << "dispatch( const css::util::URL& aURL, const "
        "css::uno::Sequence< css::beans::PropertyValue >& aArguments ) throw"
        "(css::uno::RuntimeException)\n{\n";

    for (const auto& rEntry : protocolCmdMap) {
        o << "    if ( aURL.Protocol.equalsAscii(\"" << rEntry.first
          << "\") == 0 )\n    {\n";

        for (const auto& rCmd : rEntry.second) {
            o << "        if ( aURL.Path.equalsAscii(\"" << rCmd << "\") )\n"
                "        {\n                // add your own code here\n"
                "                return;\n        }\n";
        }

        o << "    }\n";
    }
    o << "}\n\n";

    // addStatusListener
    o << "void SAL_CALL " << classname << "addStatusListener( const css::uno::Reference< "
        "css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) "
        "throw (css::uno::RuntimeException)\n{\n"
        "    // add your own code here\n}\n\n";

    // removeStatusListener
    o << "void SAL_CALL " << classname << "removeStatusListener( const css::uno::Reference"
        "< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) "
        "throw (css::uno::RuntimeException)\n{\n"
        "    // add your own code here\n}\n\n";
}

void generateXDispatchProvider(std::ostream& o,
                               const OString & classname,
                               const ProtocolCmdMap & protocolCmdMap)
{

    // com.sun.star.frame.XDispatchProvider
    // queryDispatch
    o << "// css::frame::XDispatchProvider:\n"
        "css::uno::Reference< css::frame::XDispatch > SAL_CALL " << classname
      << "queryDispatch( const css::util::URL& aURL,"
        " const OUString& sTargetFrameName, sal_Int32 nSearchFlags ) "
        "throw(css::uno::RuntimeException)\n{\n    css::uno::Reference< "
        "css::frame::XDispatch > xRet;\n"
        "    if ( !m_xFrame.is() )\n        return 0;\n\n";

    for (const auto& rEntry : protocolCmdMap) {
        o << "    if ( aURL.Protocol.equalsAscii(\"" << rEntry.first
          << "\") == 0 )\n    {\n";

        for (const auto& rCmd : rEntry.second) {
            o << "        if ( aURL.Path.equalsAscii(\"" << rCmd << "\") == 0 )\n"
                "            xRet = this;\n";
        }

        o << "    }\n";
    }
    o << "    return xRet;\n}\n\n";

    // queryDispatches
    o << "css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL "
      << classname << "queryDispatches( const css::uno::Sequence< "
        "css::frame::DispatchDescriptor >& seqDescripts ) throw("
        "css::uno::RuntimeException)\n{\n"
        "    sal_Int32 nCount = seqDescripts.getLength();\n"
        "    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > "
        "lDispatcher(nCount);\n\n"
        "    for( sal_Int32 i=0; i<nCount; ++i ) {\n"
        "        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL,\n"
        "                                        seqDescripts[i].FrameName,\n"
        "                                        seqDescripts[i].SearchFlags );\n"
        "    }\n\n    return lDispatcher;\n}\n\n";
}

static void generateAddinConstructorAndHelper(std::ostream& o,
         ProgramOptions const & options,
         rtl::Reference< TypeManager > const & manager, const OString & classname,
         const std::set< OUString >& interfaces)
{
    o << classname << "::" << classname
      << "(css::uno::Reference< css::uno::XComponentContext > const & context) :\n"
         "    m_xContext(context),    m_locale()\n{\n";

    if (options.backwardcompatible) {
        o << "     try {\n";

        generateFunctionParameterMap(o, options, manager, interfaces);

        o << "        css::uno::Reference< css::lang::XMultiServiceFactory > xProvider"
            "(\n             m_xContext->getServiceManager()->createInstanceWithContext"
            "(\n                 OUString(\n    "
            "                 \"com.sun.star.configuration.ConfigurationProvider\"),"
            "\n                 m_xContext ), css::uno::UNO_QUERY );\n\n";

        o << "        OUString sReadOnlyView(\n"
            "                \"com.sun.star.configuration.ConfigurationAccess\");\n\n";

        o << "        OUStringBuffer sPath(OUString(\n"
            "             \"/org.openoffice.Office.CalcAddIns/AddInInfo/\"));\n"
            "        sPath.appendAscii(sADDIN_SERVICENAME);\n"
            "        sPath.appendAscii(\"/AddInFunctions\");\n\n"
            "        // create arguments: nodepath\n"
            "        css::beans::PropertyValue aArgument;\n"
            "        aArgument.Name = OUString(\"nodepath\");\n"
            "        aArgument.Value <<= sPath.makeStringAndClear();\n\n"
            "        css::uno::Sequence< css::uno::Any > aArguments(1);\n"
            "        aArguments[0] <<= aArgument;\n\n";

        o << "        // create the default view using default UI locale\n"
            "        css::uno::Reference< css::uno::XInterface > xIface =\n"
            "            xProvider->createInstanceWithArguments(sReadOnlyView, "
            "aArguments);\n\n"
            "         m_xHAccess.set(xIface, css::uno::UNO_QUERY);"
            "\n\n";

        o << "        // extend arguments to create a view for all locales to get "
            "simple\n        // access to the compatibilityname property\n"
            "        aArgument.Name = OUString(\"locale\");\n"
            "        aArgument.Value <<= OUString(\"*\");\n"
            "        aArguments.realloc(2);\n"
            "        aArguments[1] <<= aArgument;\n\n"
            "        // create view for all locales\n"
            "        xIface = xProvider->createInstanceWithArguments(sReadOnlyView, "
            "aArguments);\n\n"
            "        m_xCompAccess.set(xIface, css::uno::UNO_QUERY);\n";

        o << "    }\n    catch ( css::uno::Exception & ) {\n    }\n}\n\n";

        o << "// addin configuration property helper function:\nOUString "
            "SAL_CALL " << classname << "::getAddinProperty(const OUString &"
            " funcName, const OUString & paramName, const char * propName) "
            "throw (css::uno::RuntimeException)\n{\n"
            "    OUString ret;\n    try {\n        "
            "OUStringBuffer buf(funcName);\n"
            "        if (!paramName.isEmpty()) {\n"
            "            buf.appendAscii(\"/Parameters/\");\n"
            "            buf.append(paramName);\n        }\n\n"
            "        css::uno::Reference< css::beans::XPropertySet > xPropSet(\n"
            "            m_xHAccess->getByHierarchicalName(\n"
            "                buf.makeStringAndClear()), css::uno::UNO_QUERY);\n"
            "        xPropSet->getPropertyValue(\n            "
            "OUString(propName)) >>= ret;\n    }\n"
            "     catch ( const css::uno::RuntimeException & e ) {\n        throw e;\n    }\n"
            "     catch ( css::uno::Exception & ) {\n    }\n    return ret;\n";
    }
    o <<"}\n\n";
}

static void generateMemberInitialization(std::ostream& o,
                                  ProgramOptions const & options,
                                  rtl::Reference< TypeManager > const & manager,
                                  AttributeInfo const & members)
{
    for (const auto& rMember : members)
    {
        sal_Int32 rank;
        if ((manager->decompose(rMember.type, true, nullptr, &rank, nullptr, nullptr)
             <= codemaker::UnoType::Sort::Char)
            && rank == 0)
        {
            o << ",\n    m_" << rMember.name << "(";
            printType(o, options, manager, rMember.type, 16, true);
            o << ")";
        }
    }
}

static void generateMemberDeclaration(std::ostream& o,
                               ProgramOptions const & options,
                               rtl::Reference< TypeManager > const & manager,
                               AttributeInfo const & members)
{
    for (const auto& rMember : members)
    {
        o << "    ";
        printType(o, options, manager, rMember.type, 1);
        o << " m_" << rMember.name << ";\n";
    }
}

static OString generateClassDefinition(std::ostream& o,
         ProgramOptions const & options,
         rtl::Reference< TypeManager > const & manager,
         OString const & classname,
         std::set< OUString > const & interfaces,
         AttributeInfo const & properties,
         AttributeInfo const & attributes,
         std::set< OUString > const & propinterfaces,
         OUString const & propertyhelper, bool supportxcomponent)
{
    OStringBuffer parentname(64);
    o << "class " << classname << ":\n";

    if (!interfaces.empty()) {
        if (supportxcomponent) {
            parentname.append("::cppu::WeakComponentImplHelper");
            parentname.append(static_cast<sal_Int32>(interfaces.size()));
            o << "    private ::cppu::BaseMutex,\n"
                 "    public ::cppu::WeakComponentImplHelper"
              << interfaces.size() << "<";
        } else {
            parentname.append("::cppu::WeakImplHelper");
            parentname.append(static_cast<sal_Int32>(interfaces.size()));
            o << "    public ::cppu::WeakImplHelper" << interfaces.size() << "<";
        }

        std::set< OUString >::const_iterator iter = interfaces.begin();
        while (iter != interfaces.end())
        {
            o << "\n        " << scopedCppName(u2b(*iter));
            ++iter;
            if (iter != interfaces.end())
                o << ",";
            else
                o << ">";
        }
    }

    if (propertyhelper.getLength() > 1) {
        o << ",\n    public ::cppu::PropertySetMixin< "
          << scopedCppName(u2b(propertyhelper)) << " >";
    }

    o << "\n{\npublic:\n"
         "    explicit " << classname << "("
         "css::uno::Reference< css::uno::XComponentContext > const & context);\n\n";

    // generate component/service helper functions
//     o << "    // component and service helper functions:\n"
//       << "    static OUString SAL_CALL _getImplementationName();\n"
//       << "    static css::uno::Sequence< OUString > SAL_CALL "
//       << "_getSupportedServiceNames();\n"
//       << "    static css::uno::Reference< css::uno::XInterface > SAL_CALL _create("
//       << "\n        css::uno::Reference< css::uno::XComponentContext > const & "
//       << "context);\n\n";

    // override queryInterface
    if (propertyhelper.getLength() > 1) {
        o << "    // css::uno::XInterface:\n"
            "    virtual css::uno::Any SAL_CALL queryInterface("
            "css::uno::Type const & type) throw ("
            "css::uno::RuntimeException);\n";

        OStringBuffer buffer(256);
        buffer.append(parentname.toString());
        buffer.append("< ");
        std::set< OUString >::const_iterator iter = interfaces.begin();
        while (iter != interfaces.end())
        {
            buffer.append(scopedCppName(u2b(*iter)));
            ++iter;
            if (iter != interfaces.end())
                buffer.append(", ");
            else
                buffer.append(" >");
        }
        OString parent(buffer.makeStringAndClear());
        o << "    virtual void SAL_CALL acquire() throw ()\n        { "
          << parent << "::acquire(); }\n";
        o << "    virtual void SAL_CALL release() throw ()\n        { "
          << parent << "::release(); }\n\n";
    }

    codemaker::GeneratedTypeSet generated;
    for (const auto& rIface : interfaces)
    {
        printMethods(o, options, manager, rIface, generated, "", "", "    ",
                     true, propertyhelper);
    }

    o << "private:\n    " << classname << "(const " << classname << " &); // not defined\n"
         "    " << classname << "& operator=(const " << classname << " &); // not defined\n\n"
         "    // destructor is private and will be called indirectly by the release call"
         "    virtual ~" << classname << "() {}\n\n";

    if (options.componenttype == 2) {
        o << "    typedef boost::unordered_map< ::sal_Int32, OUString, "
            "boost::hash<::sal_Int32> > ParamMap;\n"
            "    typedef boost::unordered_map< OUString, ParamMap, "
            "OUStringHash > FunctionMap;\n\n"
            "    OUString SAL_CALL getAddinProperty(const OUString & "
            "funcName, const OUString & paramName, const char * propName) "
            "throw (css::uno::RuntimeException);\n\n";
    }

    if (supportxcomponent) {
        o << "    // override WeakComponentImplHelperBase::disposing()\n"
            "    // This function is called upon disposing the component,\n"
            "    // if your component needs special work when it becomes\n"
            "    // disposed, do it here.\n"
            "    virtual void SAL_CALL disposing();\n\n";
    }

    // members
    o << "    css::uno::Reference< css::uno::XComponentContext > m_xContext;\n";
    if (!supportxcomponent && !attributes.empty())
        o << "   mutable ::osl::Mutex m_aMutex;\n";

    // additional member for add-ons
    if (options.componenttype == 3) {
        o << "    css::uno::Reference< css::frame::XFrame > m_xFrame;\n";
    }

    if (options.componenttype == 2) {
        if (options.backwardcompatible) {
            o <<"    css::uno::Reference< css::container::XHierarchicalNameAccess > "
                "m_xHAccess;\n"
                "    css::uno::Reference< css::container::XHierarchicalNameAccess > "
                "m_xCompAccess;\n"
                "    FunctionMap m_functionMap;\n";
        }
        o << "    css::lang::Locale m_locale;\n";
    }

    generateMemberDeclaration(o, options, manager, properties);
    generateMemberDeclaration(o, options, manager, attributes);

//     if (!properties.empty())
//     {
//         AttributeInfo::const_iterator iter = properties.begin();
//         while (iter != properties.end())
//         {
//             o << "    ";
//             printType(o, options, manager, iter->second.first.replace('.','/'),
//                       1, false);
//             o << " m_" << iter->first << ";\n";
//             ++iter;
//         }
//     }
//     if (!attributes.empty())
//     {
//         AttributeInfo::const_iterator iter = attributes.begin();
//         while (iter != attributes.end())
//         {
//             o << "    ";
//             printType(o, options, manager, iter->second.first.replace('.','/'),
//                       1, false);
//             o << " m_" << iter->first << ";\n";
//             ++iter;
//         }
//     }

    o << "};\n\n";

    // generate constructor
    if (options.componenttype == 2) {
        generateAddinConstructorAndHelper(o, options, manager,
                                          classname, interfaces);
    } else {
        o << classname << "::" << classname
          << "(css::uno::Reference< css::uno::XComponentContext > const & context) :\n";
        if (supportxcomponent) {
            o << "    ::cppu::WeakComponentImplHelper" << interfaces.size() << "<";
            std::set< OUString >::const_iterator iter = interfaces.begin();
            while (iter != interfaces.end()) {
                o << "\n        " << scopedCppName(u2b(*iter));
                ++iter;
                if (iter != interfaces.end())
                    o << ",";
                else
                    o << ">(m_aMutex),\n";
            }
        }
        if (propertyhelper.getLength() > 1) {
            o << "    ::cppu::PropertySetMixin< "
              << scopedCppName(u2b(propertyhelper)) << " >(\n"
                 "        context, static_cast< Implements >(\n            ";
            OStringBuffer buffer(128);
            if (propinterfaces.find("com/sun/star/beans/XPropertySet")
                != propinterfaces.end()) {
                buffer.append("IMPLEMENTS_PROPERTY_SET");
            }
            if (propinterfaces.find("com/sun/star/beans/XFastPropertySet")
                != propinterfaces.end()) {
                if (!buffer.isEmpty())
                    buffer.append(" | IMPLEMENTS_FAST_PROPERTY_SET");
                else
                    buffer.append("IMPLEMENTS_FAST_PROPERTY_SET");
            }
            if (propinterfaces.find("com/sun/star/beans/XPropertyAccess")
                != propinterfaces.end()) {
                if (!buffer.isEmpty())
                    buffer.append(" | IMPLEMENTS_PROPERTY_ACCESS");
                else
                    buffer.append("IMPLEMENTS_PROPERTY_ACCESS");
            }
            o << buffer.makeStringAndClear()
              << "), css::uno::Sequence< OUString >()),\n";
        }
        o << "    m_xContext(context)";

        generateMemberInitialization(o, options, manager, properties);
        generateMemberInitialization(o, options, manager, attributes);

        o << "\n{}\n\n";
    }

    // generate service/component helper function implementations
//     generateServiceHelper(o, options.implname, classname, services);

    if (supportxcomponent) {
        o << "// override WeakComponentImplHelperBase::disposing()\n"
            "// This function is called upon disposing the component,\n"
            "// if your component needs special work when it becomes\n"
            "// disposed, do it here.\n"
            "void SAL_CALL " << classname << "::disposing()\n{\n\n}\n\n";
    }

    return parentname.makeStringAndClear();
}

static void generateXServiceInfoBodies(std::ostream& o,
                                OString const & classname,
                                OString const & comphelpernamespace)
{
    o << "// com.sun.star.uno.XServiceInfo:\n"
         "OUString SAL_CALL " << classname << "getImplementationName() "
         "throw (css::uno::RuntimeException)\n{\n    "
         "return " << comphelpernamespace << "::_getImplementationName();\n}\n\n";

    o << "sal_Bool SAL_CALL " << classname
      << "supportsService(OUString const & "
         "serviceName) throw (css::uno::RuntimeException)\n{\n    "
         "css::uno::Sequence< OUString > serviceNames = "
      << comphelpernamespace << "::_getSupportedServiceNames();\n    "
         "for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {\n    "
         "    if (serviceNames[i] == serviceName)\n            return sal_True;\n"
         "    }\n    return sal_False;\n}\n\n";

    o << "css::uno::Sequence< OUString > SAL_CALL " << classname
      << "getSupportedServiceNames() throw (css::uno::RuntimeException)\n{\n    "
         "return " << comphelpernamespace
      << "::_getSupportedServiceNames();\n}\n\n";
}


static void generateMethodBodies(std::ostream& o,
        ProgramOptions const & options,
        rtl::Reference< TypeManager > const & manager,
        std::set< OUString > const & interfaces,
        OString const & classname,
        OString const & comphelpernamespace,
        OUString const & propertyhelper)
{
    OString name(classname.concat("::"));
    codemaker::GeneratedTypeSet generated;
    for (const auto& rIface : interfaces) {
        if ( rIface == "com.sun.star.lang.XServiceInfo" ) {
            generateXServiceInfoBodies(o, name, comphelpernamespace);
            generated.add(u2b(rIface));
        } else {
            printMethods(o, options, manager, rIface, generated, "_",
                         name, "", true, propertyhelper);
        }
    }
}

static void generateQueryInterface(std::ostream& o,
                            ProgramOptions const & options,
                            rtl::Reference< TypeManager > const & manager,
                            const std::set< OUString >& interfaces,
                            OString const & parentname,
                            OString const & classname,
                            OUString const & propertyhelper)
{
    if (propertyhelper.isEmpty())
        return;

    o << "css::uno::Any " << classname
      << "::queryInterface(css::uno::Type const & type) throw ("
        "css::uno::RuntimeException)\n{\n    ";

    if (!propertyhelper.isEmpty())
        o << "return ";
    else
        o << "css::uno::Any a(";

    o   << parentname << "<";
    std::set< OUString >::const_iterator iter = interfaces.begin();
    while (iter != interfaces.end())
    {
        o << "\n        " << scopedCppName(u2b(*iter));
        ++iter;
        if (iter != interfaces.end())
            o << ",";
        else
            o << ">";
    }

    if (!propertyhelper.isEmpty()) {
        o << "::queryInterface(type);\n";
    } else {
        o << "::queryInterface(type));\n";
        o << "    return a.hasValue() ? a\n        : (";
        if (propertyhelper == "_") {
            o << "::cppu::OPropertySetHelper::queryInterface(type));\n";
        } else {
            o << "::cppu::PropertySetMixin<\n            ";
            printType(o, options, manager, propertyhelper, 0);
            o << " >::queryInterface(\n               type));\n";
        }
    }
    o << "}\n\n";
}

void generateSkeleton(ProgramOptions const & options,
                      rtl::Reference< TypeManager > const & manager,
                      std::vector< OString > const & types)
{
    // special handling of calc add-ins
    if (options.componenttype == 2) {
        generateCalcAddin(options, manager, types);
        return;
    }

    std::set< OUString > interfaces;
    std::set< OUString > services;
    AttributeInfo properties;
    AttributeInfo attributes;
    std::set< OUString > propinterfaces;
    bool serviceobject = false;
    bool supportxcomponent = false;

    for (const auto& rType : types) {
        checkType(manager, b2u(rType), interfaces, services, properties);
    }

    if (options.componenttype == 3) {
        // the Protocolhandler service is mandatory for a protocol handler add-on,
        // so it is defaulted. The XDispatchProvider provides Dispatch objects for
        // certain functions and the generated impl object implements XDispatch
        // directly for simplicity reasons.
        checkType(manager, "com.sun.star.frame.ProtocolHandler",
                  interfaces, services, properties);
        checkType(manager, "com.sun.star.frame.XDispatch",
                  interfaces, services, properties);
    }

    // check if service object or simple UNO object
    if (!services.empty())
        serviceobject = true;

    OUString propertyhelper = checkPropertyHelper(
        options, manager, services, interfaces, attributes, propinterfaces);

    checkDefaultInterfaces(interfaces, services, propertyhelper);

    if (interfaces.size() > 12)
        throw CannotDumpException(
            "the skeletonmaker supports components with 12 interfaces "
            "only (limitation of the UNO implementation helpers)!");


    supportxcomponent = checkXComponentSupport(manager, interfaces);

    OString compFileName;
    OString tmpFileName;
    std::ostream* pofs = nullptr;
    bool standardout = getOutputStream(options, ".cxx",
                                       &pofs, compFileName, tmpFileName);

    try {
        if (!standardout && options.license) {
            printLicenseHeader(*pofs, compFileName);
        }

        generateIncludes(*pofs, interfaces, propertyhelper, serviceobject,
                         supportxcomponent);

        if (options.componenttype == 3) {
            *pofs << "#include \"com/sun/star/frame/XFrame.hpp\"\n";
        }

        // namespace
        OString nmspace;
        short nm = 0;

        if (serviceobject) {
            nmspace = generateCompHelperDeclaration(*pofs, options.implname);

            *pofs <<
                "\n\n/// anonymous implementation namespace\nnamespace {\n\n"
                "namespace css = ::com::sun::star;\n\n";
        } else {
            nm = generateNamespace(*pofs, options.implname, false, nmspace);
            *pofs << "namespace css = ::com::sun::star;\n\n";
        }

        sal_Int32 index = 0;
        OString classname(options.implname);
        if ((index = classname.lastIndexOf('.')) > 0)
            classname = classname.copy(index+1);

        OString parentname(
            generateClassDefinition(*pofs,
                options, manager, classname, interfaces, properties,
                attributes, propinterfaces, propertyhelper, supportxcomponent));

        generateQueryInterface(*pofs, options, manager, interfaces, parentname,
                               classname, propertyhelper);

        generateMethodBodies(*pofs, options, manager, interfaces, classname,
                             nmspace, propertyhelper);

        if (serviceobject) {
            // close namespace
            *pofs << "} // closing anonymous implementation namespace\n\n";

            generateCompHelperDefinition(*pofs, options.implname,
                                         classname, services);
            generateCompFunctions(*pofs, nmspace);
        } else {
            // close namespace
            for (short i=0; i < nm; i++)
                *pofs << "} ";
            *pofs << (nm > 0 ? "// closing namespace\n\n" : "\n");
        }

        if (!standardout)
        {
            if (static_cast<std::ofstream*>(pofs)->is_open())
                static_cast<std::ofstream*>(pofs)->close();
            delete pofs;
            OSL_VERIFY(makeValidTypeFile(compFileName, tmpFileName, false));
        }
    } catch (CannotDumpException & e) {
        std::cerr << "ERROR: " << e.getMessage() << "\n";
        if ( !standardout ) {
            if (static_cast<std::ofstream*>(pofs)->is_open())
                static_cast<std::ofstream*>(pofs)->close();
            delete pofs;
            // remove existing type file if something goes wrong to ensure
            // consistency
            if (fileExists(compFileName))
                removeTypeFile(compFileName);

            // remove tmp file if something goes wrong
            removeTypeFile(tmpFileName);
        }
    }
}

void generateCalcAddin(ProgramOptions const & options,
                       rtl::Reference< TypeManager > const & manager,
                       std::vector< OString > const & types)
{
    std::set< OUString > interfaces;
    std::set< OUString > services;
    AttributeInfo properties;
    AttributeInfo attributes;
    std::set< OUString > propinterfaces;
    bool serviceobject = false;
    bool supportxcomponent = false;


    for (const auto& rType : types) {
        checkType(manager, b2u(rType), interfaces, services, properties);
    }

    OUString sAddinService;
    if (services.size() != 1) {
        throw CannotDumpException(
            "for calc add-in components one and only one service type is necessary!"
            " Please reference a valid type with the '-t' option.");
    }


    // get the one and only add-in service for later use
    std::set< OUString >::const_iterator iter2 = services.begin();
    sAddinService = *iter2;
    if (sAddinService == "com.sun.star.sheet.AddIn") {
        sAddinService = *(++iter2);
    }

    // if backwardcompatible==true the AddIn service needs to be added to the
    // supported service list, the necessary interfaces are mapped to the add-in
    // configuration. Since OO.org 2.0.4 this is obsolete and the add-in is
    // taken from the configuration from Calc directly, this simplifies the
    // add-in code
    if (options.backwardcompatible) {
        checkType(manager, "com.sun.star.sheet.AddIn",
                  interfaces, services, properties);
    } else {
        // special case for the optional XLocalization interface. It should be
        // implemented always. But it is parent of the XAddIn and we need it only
        // if backwardcompatible is false.
        interfaces.insert("com.sun.star.lang.XLocalizable");
    }

    OUString propertyhelper = checkPropertyHelper(
        options, manager, services, interfaces, attributes, propinterfaces);

    if (!propertyhelper.isEmpty())
        std::cerr << "WARNING: interfaces specifying calc add-in functions "
            "shouldn't support attributes!\n";

    checkDefaultInterfaces(interfaces, services, propertyhelper);

    if (interfaces.size() > 12) {
        throw CannotDumpException(
            "the skeletonmaker supports components with 12 interfaces "
            "only (limitation of the UNO implementation helpers)!");
    }

    // check if service object or simple UNO object
    if (!services.empty())
        serviceobject = true;

    supportxcomponent = checkXComponentSupport(manager, interfaces);
    if (supportxcomponent)
        std::cerr << "WARNING: add-ins shouldn't support "
            "com.sun.star.uno.XComponent!\n";

    OString compFileName;
    OString tmpFileName;
    std::ostream* pofs = nullptr;
    bool standardout = getOutputStream(options, ".cxx",
                                       &pofs, compFileName, tmpFileName);

    try {
        if (!standardout && options.license) {
            printLicenseHeader(*pofs, compFileName);
        }

        generateIncludes(*pofs, interfaces, propertyhelper, serviceobject,
                         supportxcomponent);

        *pofs <<
            "#include \"com/sun/star/beans/PropertyValue.hpp\"\n"
            "#include \"com/sun/star/beans/XPropertySet.hpp\"\n"
            "#include \"com/sun/star/container/XNameAccess.hpp\"\n"
            "#include \"com/sun/star/container/XHierarchicalNameAccess.hpp\"\n\n"
            "#include \"rtl/ustrbuf.hxx\"\n\n"
            "#include <boost/unordered_map.hpp>\n"
            "#include <set>\n";

        // namespace
        OString nmspace(generateCompHelperDeclaration(*pofs, options.implname));

        *pofs <<
            "\n\n// anonymous implementation namespace\nnamespace {\n\n"
            "namespace css = ::com::sun::star;\n\n";

        sal_Int32 index = 0;
        OString classname(options.implname);
        if ((index = classname.lastIndexOf('.')) > 0) {
            classname = classname.copy(index+1);
        }

        if (options.backwardcompatible) {
            *pofs << "static const char * sADDIN_SERVICENAME = \""
                  << sAddinService << "\";\n\n";
            *pofs << "static const char * sDISPLAYNAME = \"DisplayName\";\n"
                "static const char * sDESCRIPTION = \"Description\";\n"
                "static const char * sCATEGORY = \"Category\";\n"
                "static const char * sCATEGORYDISPLAYNAME = \"CategoryDisplayName\";"
                "\n\n";
        }

        OString parentname(
            generateClassDefinition(*pofs,
                options, manager, classname, interfaces, properties,
                attributes, propinterfaces, propertyhelper, supportxcomponent));

        generateQueryInterface(*pofs, options, manager, interfaces, parentname,
                               classname, propertyhelper);

        generateMethodBodies(*pofs, options, manager, interfaces, classname,
                             nmspace, propertyhelper);

        // close namespace
        *pofs << "} // closing anonymous implementation namespace\n\n";

        generateCompHelperDefinition(*pofs, options.implname, classname,
                                     services);

        generateCompFunctions(*pofs, nmspace);

        if (!standardout)
        {
            if (static_cast<std::ofstream*>(pofs)->is_open())
                static_cast<std::ofstream*>(pofs)->close();
            delete pofs;
            OSL_VERIFY(makeValidTypeFile(compFileName, tmpFileName, false));
        }
    } catch (CannotDumpException & e) {
        std::cerr << "ERROR: " << e.getMessage() << "\n";
        if ( !standardout ) {
            if (static_cast<std::ofstream*>(pofs)->is_open())
                static_cast<std::ofstream*>(pofs)->close();
            delete pofs;
            // remove existing type file if something goes wrong to ensure
            // consistency
            if (fileExists(compFileName))
                removeTypeFile(compFileName);

            // remove tmp file if something goes wrong
            removeTypeFile(tmpFileName);
        }
    }
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

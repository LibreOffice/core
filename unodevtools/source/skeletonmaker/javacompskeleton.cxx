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

#include "sal/config.h"

#include "codemaker/commonjava.hxx"
#include "codemaker/global.hxx"
#include "rtl/strbuf.hxx"

#include "skeletoncommon.hxx"
#include "skeletonjava.hxx"

#include <iostream>

using namespace ::rtl;
using namespace ::codemaker::java;

namespace skeletonmaker { namespace java {

void generatePackage(std::ostream & o, const OString & implname)
{
    sal_Int32 index = implname.lastIndexOf('.');
    if (index != -1)
        o << "package " << implname.copy(0, index) << ";\n\n";
}

void generateImports(std::ostream & o, ProgramOptions const & options,
         const OUString & propertyhelper,
         bool serviceobject, bool supportxcomponent)
{
    if (options.componenttype == 3)
        o << "import com.sun.star.uno.UnoRuntime;\n";
    o << "import com.sun.star.uno.XComponentContext;\n";
    if (serviceobject) {
        o << "import com.sun.star.lib.uno.helper.Factory;\n";
        o << "import com.sun.star.lang.XSingleComponentFactory;\n";
        o << "import com.sun.star.registry.XRegistryKey;\n";
    }

    if  (!propertyhelper.equals("_")) {
        if (supportxcomponent)
            o << "import com.sun.star.lib.uno.helper.ComponentBase;\n";
        else
            o << "import com.sun.star.lib.uno.helper.WeakBase;\n";
    }
    if (!propertyhelper.isEmpty()) {
        if (propertyhelper.equals("_")) {
            o << "import com.sun.star.lib.uno.helper.PropertySet;\n";
            o << "import com.sun.star.beans.PropertyAttribute;\n";
        } else {
            o << "import com.sun.star.uno.Type;\n";
            o << "import com.sun.star.uno.Any;\n";
            o << "import com.sun.star.beans.Ambiguous;\n";
            o << "import com.sun.star.beans.Defaulted;\n";
            o << "import com.sun.star.beans.Optional;\n";
            o << "import com.sun.star.lib.uno.helper.PropertySetMixin;\n";
        }
    }
}

void generateCompFunctions(std::ostream & o, const OString & classname)
{
    o << "    public static XSingleComponentFactory __getComponentFactory("
        " String sImplementationName ) {\n"
        "        XSingleComponentFactory xFactory = null;\n\n"
        "        if ( sImplementationName.equals( m_implementationName ) )\n"
        "            xFactory = Factory.createComponentFactory("
      << classname << ".class, m_serviceNames);\n"
        "        return xFactory;\n    }\n\n";

    o << "    public static boolean __writeRegistryServiceInfo("
        " XRegistryKey xRegistryKey ) {\n"
        "        return Factory.writeRegistryServiceInfo(m_implementationName,\n"
        "                                                m_serviceNames,\n"
        "                                                xRegistryKey);\n"
        "    }\n\n";
}

void generateXServiceInfoBodies(std::ostream& o)
{
    o << "    // com.sun.star.lang.XServiceInfo:\n";
    o << "    public String getImplementationName() {\n"
      << "         return m_implementationName;\n    }\n\n";

    o << "    public boolean supportsService( String sService ) {\n"
      << "        int len = m_serviceNames.length;\n\n"
      << "        for( int i=0; i < len; i++) {\n"
      << "            if (sService.equals(m_serviceNames[i]))\n"
      << "                return true;\n"
      << "        }\n        return false;\n    }\n\n";

    o << "    public String[] getSupportedServiceNames() {\n"
      << "        return m_serviceNames;\n    }\n\n";
}

void generateXPropertySetBodies(std::ostream& o)
{
    o << "    // com.sun.star.beans.XPropertySet:\n";
    o << "    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()\n"
      "    {\n        return m_prophlp.getPropertySetInfo();\n    }\n\n";

    o << "    public void setPropertyValue(String aPropertyName, "
        "Object aValue) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.beans.PropertyVetoException, "
        "com.sun.star.lang.IllegalArgumentException,"
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.setPropertyValue(aPropertyName, aValue);\n    }\n\n";

    o << "    public Object getPropertyValue(String "
        "aPropertyName) throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        return "
        "m_prophlp.getPropertyValue(aPropertyName);\n    }\n\n";

    o << "    public void addPropertyChangeListener(String aPropertyName"
        ", com.sun.star.beans.XPropertyChangeListener xListener) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.addPropertyChangeListener(aPropertyName, xListener);\n    }\n\n";

    o << "    public void removePropertyChangeListener(String "
        "aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener) "
        "throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.removePropertyChangeListener(aPropertyName, xListener);\n"
        "    }\n\n";

    o << "    public void addVetoableChangeListener(String aPropertyName"
        ", com.sun.star.beans.XVetoableChangeListener xListener) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.addVetoableChangeListener(aPropertyName, xListener);\n    }\n\n";

    o << "    public void removeVetoableChangeListener(String "
        "aPropertyName, com.sun.star.beans.XVetoableChangeListener xListener) "
        "throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.removeVetoableChangeListener(aPropertyName, xListener);\n }\n\n";
}

void generateXFastPropertySetBodies(std::ostream& o)
{
    o << "    // com.sun.star.beans.XFastPropertySet:\n";

    o << "    public void setFastPropertyValue(int nHandle, Object "
        "aValue) throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.beans.PropertyVetoException, "
        "com.sun.star.lang.IllegalArgumentException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.setFastPropertyValue(nHandle, aValue);\n    }\n\n";

    o << "    public Object getFastPropertyValue(int nHandle) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        return "
        "m_prophlp.getFastPropertyValue(nHandle);\n    }\n\n";
}

void generateXPropertyAccessBodies(std::ostream& o)
{
    o << "    // com.sun.star.beans.XPropertyAccess:\n";

    o << "    public com.sun.star.beans.PropertyValue[] getPropertyValues()\n"
        " {\n        return m_prophlp.getPropertyValues();\n    }\n\n";

    o << "    public void setPropertyValues(com.sun.star.beans.PropertyValue[] "
        "aProps) throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.beans.PropertyVetoException, "
        "com.sun.star.lang.IllegalArgumentException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.setPropertyValues(aProps);\n    }\n\n";
}


bool checkAttribute(
    OStringBuffer& attributeValue,
    unoidl::AccumulationBasedServiceEntity::Property::Attributes attribute)
{
    bool cast = false;
    sal_uInt16 attributes[9] = {
        /* com::sun::star::beans::PropertyValue::MAYBEVOID */ 1,
        /* com::sun::star::beans::PropertyValue::BOUND */ 2,
        /* com::sun::star::beans::PropertyValue::CONSTRAINED */ 4,
        /* com::sun::star::beans::PropertyValue::TRANSIENT */ 8,
        /* com::sun::star::beans::PropertyValue::READONLY */ 16,
        /* com::sun::star::beans::PropertyValue::MAYBEAMBIGIOUS */ 32,
        /* com::sun::star::beans::PropertyValue::MAYBEDEFAULT */ 64,
        /* com::sun::star::beans::PropertyValue::REMOVABLE */ 128,
        /* com::sun::star::beans::PropertyValue::OPTIONAL */ 256 };

    for (sal_uInt16 i = 0; i < 9; i++)
    {
        if (attribute & attributes[i]) {
            if (!attributeValue.isEmpty()) {
                cast |= true;
                attributeValue.append("|");
            }
            switch (attributes[i])
            {
            case 1:
                attributeValue.append("PropertyAttribute.MAYBEVOID");
                break;
            case 2:
                attributeValue.append("PropertyAttribute.BOUND");
                break;
            case 4:
                attributeValue.append("PropertyAttribute.CONSTRAINED");
                break;
            case 8:
                attributeValue.append("PropertyAttribute.TRANSIENT");
                break;
            case 16:
                attributeValue.append("PropertyAttribute.READONLY");
                break;
            case 32:
                attributeValue.append("PropertyAttribute.MAYBEAMBIGIOUS");
                break;
            case 64:
                attributeValue.append("PropertyAttribute.MAYBEDEFAULT");
                break;
            case 128:
                attributeValue.append("PropertyAttribute.REMOVABLE");
                break;
            case 256:
                attributeValue.append("PropertyAttribute.OPTIONAL");
                break;
            }
        }
    }
    if (cast) {
        attributeValue.insert(0, '(');
        attributeValue.append(')');
    }

    return cast;
}

void registerProperties(std::ostream& o,
                        const AttributeInfo& properties,
                        const OString& indentation)
{
    if (!properties.empty()) {
        bool cast = false;
        OStringBuffer attributeValue;
        for (AttributeInfo::const_iterator i(properties.begin());
             i != properties.end(); ++i)
        {
            if (i->attributes != 0) {
                cast = checkAttribute(attributeValue, i->attributes);
            } else {
                cast = true;
                attributeValue.append('0');
            }

            o << indentation << "registerProperty(\"" << i->name
              << "\", \"m_" << i->name << "\",\n"
              << indentation << "      ";
            if (cast)
                o << "(short)";

            o << attributeValue.makeStringAndClear() << ");\n";
        }
    }
}

void generateXLocalizableBodies(std::ostream& o) {
    // com.sun.star.lang.XLocalizable:
    // setLocale
    o << "    // com.sun.star.lang.XLocalizable:\n"
        "    public void setLocale(com.sun.star.lang.Locale eLocale)\n    {\n"
        "        m_locale = eLocale;\n    }\n\n";

    // getLocale
    o << "    public com.sun.star.lang.Locale getLocale()\n    {\n"
        "        return m_locale;\n    }\n\n";
}

void generateXAddInBodies(std::ostream& o, ProgramOptions const &)
{
    // com.sun.star.sheet.XAddIn:
    // getProgrammaticFuntionName
    o << "    // com.sun.star.sheet.XAddIn:\n"
        "    public String getProgrammaticFuntionName(String "
        "aDisplayName)\n    {\n"
        "        try {\n"
        "            com.sun.star.container.XNameAccess xNAccess =\n"
        "                (com.sun.star.container.XNameAccess)UnoRuntime."
        "queryInterface(\n"
        "                    com.sun.star.container.XNameAccess.class, m_xHAccess);"
        "\n            String functions[] = xNAccess.getElementNames();\n"
        "            String sDisplayName = \"\";\n"
        "            int len = functions.length;\n"
        "            for (int i=0; i < len; ++i) {\n"
        "                sDisplayName = com.sun.star.uno.AnyConverter.toString(\n"
        "                    getAddinProperty(functions[i], \"\", sDISPLAYNAME));\n"
        "                if (sDisplayName.equals(aDisplayName))\n"
        "                    return functions[i];\n            }\n"
        "        }\n        catch ( com.sun.star.uno.RuntimeException e ) {\n"
        "            throw e;\n        }\n"
        "        catch ( com.sun.star.uno.Exception e ) {\n        }\n\n"
        "        return \"\";\n    }\n\n";

    // getDisplayFunctionName
    o << "    public String getDisplayFunctionName(String "
        "aProgrammaticName)\n    {\n"
        "        return getAddinProperty(aProgrammaticName, \"\", sDISPLAYNAME);\n"
        "    }\n\n";

    // getFunctionDescription
    o << "    public String getFunctionDescription(String "
        "aProgrammaticName)\n    {\n"
        "        return getAddinProperty(aProgrammaticName, \"\", sDESCRIPTION);\n"
        "    }\n\n";

    // getDisplayArgumentName
    o << "    public String getDisplayArgumentName(String "
        "aProgrammaticFunctionName, int nArgument)\n    {\n";
    o << "        return getAddinProperty(aProgrammaticFunctionName,\n"
        "                                m_functionMap.get(\n"
        "                                    aProgrammaticFunctionName).get("
        "nArgument),\n"
        "                                sDISPLAYNAME);\n    }\n\n";

    // getArgumentDescription
    o << "    public String getArgumentDescription(String "
        "aProgrammaticFunctionName, int nArgument)\n    {\n";
    o << "        return getAddinProperty(aProgrammaticFunctionName,\n"
        "                                m_functionMap.get(\n"
        "                                    aProgrammaticFunctionName).get("
        "nArgument),\n"
        "                                sDESCRIPTION);\n    }\n\n";

    // getProgrammaticCategoryName
    o << "    public String getProgrammaticCategoryName(String "
        "aProgrammaticFunctionName)\n    {\n"
        "        return getAddinProperty(aProgrammaticFunctionName, \"\", "
        "sCATEGORY);\n    }\n\n";

    // getDisplayCategoryName
    o << "    public String getDisplayCategoryName(String "
        "aProgrammaticFunctionName)\n    {\n"
        "        return getAddinProperty(aProgrammaticFunctionName, \"\", "
        "sCATEGORYDISPLAYNAME);\n    }\n\n";
}

void generateXCompatibilityNamesBodies(std::ostream& o)
{
    o << "    // com.sun.star.sheet.XCompatibilityNames:\n"
        "    public com.sun.star.sheet.LocalizedName[] getCompatibilityNames("
        "String aProgrammaticName)\n    {\n"
        "        com.sun.star.sheet.LocalizedName[] seqLocalizedNames =\n"
        "            new com.sun.star.sheet.LocalizedName[0];\n\n        try {\n";

    o << "            StringBuffer path = new StringBuffer(aProgrammaticName);\n"
        "            path.append(\"/CompatibilityName\");\n"
        "            String hname = path.toString();\n\n";

    o << "            if ( m_xCompAccess.hasByHierarchicalName(hname) ) {\n"
        "                com.sun.star.container.XNameAccess xNameAccess =\n"
        "                    (com.sun.star.container.XNameAccess)UnoRuntime."
        "queryInterface(\n"
        "                        com.sun.star.container.XNameAccess.class,\n"
        "                        m_xCompAccess.getByHierarchicalName(hname));\n\n"
        "                String elems[] = xNameAccess.getElementNames();\n"
        "                int len = elems.length;\n"
        "                seqLocalizedNames = new com.sun.star.sheet.LocalizedName"
        "[len];\n                String sCompatibilityName = \"\";\n\n";

    o << "                for (int i=0; i < len; ++i) {\n"
        "                    String sLocale = elems[i];\n"
        "                    sCompatibilityName = com.sun.star.uno.AnyConverter."
        "toString(\n                        xNameAccess.getByName(sLocale));\n\n"
        "                    com.sun.star.lang.Locale aLocale = \n"
        "                        new com.sun.star.lang.Locale();\n\n"
        /* FIXME-BCP47: this will break. */
        "                    String tokens[] = sLocale.split(\"-\");\n"
        "                    int nToken = tokens.length;\n"
        "                    if (nToken >= 1) aLocale.Language = tokens[0];\n"
        "                    if (nToken >= 2) aLocale.Country = tokens[1];\n"
        "                    if (nToken >= 3)  {\n"
        "                        StringBuffer buf = \n"
        "                            new StringBuffer(tokens[2]);\n"
        "                        for (int t=3; t < nToken; ++t)\n"
        "                            buf.append(tokens[t]);\n\n"
        "                        aLocale.Variant = buf.toString();\n"
        "                    }\n\n"
        "                    seqLocalizedNames[i].Locale = aLocale;\n"
        "                    seqLocalizedNames[i].Name = sCompatibilityName;\n"
        "                }\n        }\n        }\n"
        "        catch ( com.sun.star.uno.RuntimeException e ) {\n"
        "            throw e;\n        }\n"
        "        catch ( com.sun.star.uno.Exception e ) {\n        }\n\n"
        "        return seqLocalizedNames;\n    }\n\n";
}

void generateXInitializationBodies(std::ostream& o)
{
    o << "    // com.sun.star.lang.XInitialization:\n"
        "    public void initialize( Object[] object )\n"
        "        throws com.sun.star.uno.Exception\n    {\n"
        "        if ( object.length > 0 )\n        {\n"
        "            m_xFrame = (com.sun.star.frame.XFrame)UnoRuntime.queryInterface(\n"
        "                com.sun.star.frame.XFrame.class, object[0]);\n        }\n    }\n\n";
}

void generateXDispatchBodies(std::ostream& o, ProgramOptions const & options)
{
    // com.sun.star.frame.XDispatch
    // dispatch
    o << "    // com.sun.star.frame.XDispatch:\n"
        "     public void dispatch( com.sun.star.util.URL aURL,\n"
        "                           com.sun.star.beans.PropertyValue[] aArguments )\n    {\n";

    ProtocolCmdMap::const_iterator iter = options.protocolCmdMap.begin();
    while (iter != options.protocolCmdMap.end()) {
        o << "         if ( aURL.Protocol.compareTo(\"" << (*iter).first
          << "\") == 0 )\n        {\n";

        for (std::vector< OString >::const_iterator i = (*iter).second.begin();
             i != (*iter).second.end(); ++i) {
            o << "            if ( aURL.Path.compareTo(\"" << (*i) << "\") == 0 )\n"
                "            {\n                // add your own code here\n"
                "                return;\n            }\n";
        }

        o << "        }\n";
        ++iter;
    }
    o << "    }\n\n";

    // addStatusListener
    o << "    public void addStatusListener( com.sun.star.frame.XStatusListener xControl,\n"
        "                                    com.sun.star.util.URL aURL )\n    {\n"
        "        // add your own code here\n    }\n\n";

    // com.sun.star.frame.XDispatch
    o << "    public void removeStatusListener( com.sun.star.frame.XStatusListener xControl,\n"
        "                                       com.sun.star.util.URL aURL )\n    {\n"
        "        // add your own code here\n    }\n\n";
}

void generateXDispatchProviderBodies(std::ostream& o, ProgramOptions const & options)
{
    // com.sun.star.frame.XDispatchProvider
    // queryDispatch
    o << "    // com.sun.star.frame.XDispatchProvider:\n"
        "    public com.sun.star.frame.XDispatch queryDispatch( com.sun.star.util.URL aURL,\n"
        "                                                       String sTargetFrameName,\n"
        "                                                       int iSearchFlags )\n    {\n";

    ProtocolCmdMap::const_iterator iter = options.protocolCmdMap.begin();
    while (iter != options.protocolCmdMap.end()) {
        o << "        if ( aURL.Protocol.compareTo(\"" << (*iter).first
          << "\") == 0 )\n        {\n";

        for (std::vector< OString >::const_iterator i = (*iter).second.begin();
             i != (*iter).second.end(); ++i) {
            o << "            if ( aURL.Path.compareTo(\"" << (*i) << "\") == 0 )\n"
                "                return this;\n";
        }

        o << "        }\n";
        ++iter;
    }
    o << "        return null;\n    }\n\n";

    // queryDispatches
    o << "    // com.sun.star.frame.XDispatchProvider:\n"
        "    public com.sun.star.frame.XDispatch[] queryDispatches(\n"
        "         com.sun.star.frame.DispatchDescriptor[] seqDescriptors )\n    {\n"
        "        int nCount = seqDescriptors.length;\n"
        "        com.sun.star.frame.XDispatch[] seqDispatcher =\n"
        "            new com.sun.star.frame.XDispatch[seqDescriptors.length];\n\n"
        "        for( int i=0; i < nCount; ++i )\n        {\n"
        "            seqDispatcher[i] = queryDispatch(seqDescriptors[i].FeatureURL,\n"
        "                                             seqDescriptors[i].FrameName,\n"
        "                                             seqDescriptors[i].SearchFlags );\n"
        "        }\n        return seqDispatcher;\n    }\n\n";
}

void generateMethodBodies(std::ostream& o,
         ProgramOptions const & options,
         rtl::Reference< TypeManager > const & manager,
         const std::set< OUString >& interfaces,
         const OString& indentation, bool usepropertymixin)
{
    std::set< OUString >::const_iterator iter = interfaces.begin();
    codemaker::GeneratedTypeSet generated;
    while (iter != interfaces.end()) {
        OUString type(*iter);
        ++iter;
        if (type.equals("com.sun.star.lang.XServiceInfo")) {
            generateXServiceInfoBodies(o);
            generated.add(u2b(type));
        } else {
            if (options.componenttype == 2) {
                if (type.equals("com.sun.star.lang.XServiceName")) {
                    o << "    // com.sun.star.lang.XServiceName:\n"
                        "    public String getServiceName() {\n"
                        "        return sADDIN_SERVICENAME;\n    }\n";
                    generated.add(u2b(type));
                    continue;
                } else if (type.equals("com.sun.star.sheet.XAddIn")) {
                    generateXAddInBodies(o, options);
                    generated.add(u2b(type));

                    // special handling of XLocalizable -> parent of XAddIn
                    if (!generated.contains("com.sun.star.lang.XLocalizable")) {
                        generateXLocalizableBodies(o);
                        generated.add("com.sun.star.lang.XLocalizable");
                    }
                    continue;
                } else if (type.equals("com.sun.star.lang.XLocalizable")) {
                    generateXLocalizableBodies(o);
                    generated.add(u2b(type));
                    continue;
                } else if (type.equals("com.sun.star.sheet.XCompatibilityNames")) {
                    generateXCompatibilityNamesBodies(o);
                    generated.add(u2b(type));
                    continue;
                }
            }
            if (options.componenttype == 3) {
                if (type.equals("com.sun.star.lang.XInitialization")) {
                    generateXInitializationBodies(o);
                    generated.add(u2b(type));
                    continue;
                } else if (type.equals("com.sun.star.frame.XDispatch")) {
                    generateXDispatchBodies(o, options);
                    generated.add(u2b(type));
                    continue;
                } else if (type.equals("com.sun.star.frame.XDispatchProvider")) {
                    generateXDispatchProviderBodies(o, options);
                    generated.add(u2b(type));
                    continue;
                }
            }
            printMethods(o, options, manager, type, generated, "_",
                         indentation, true, usepropertymixin);
        }
    }
}

static const char* propcomment=
"        // use the last parameter of the PropertySetMixin constructor\n"
"        // for your optional attributes if necessary. See the documentation\n"
"        // of the PropertySetMixin helper for further information.\n"
"        // Ensure that your attributes are initialized correctly!\n";


void generateAddinConstructorAndHelper(std::ostream& o,
         ProgramOptions const & options,
         rtl::Reference< TypeManager > const & manager, const OString & classname,
         const std::set< OUString >& services,
         const std::set< OUString >& interfaces)
{
    o << "    private com.sun.star.lang.Locale m_locale = "
        "new com.sun.star.lang.Locale();\n";

    if (!options.backwardcompatible) {
        // Constructor
        o << "\n    public " << classname << "( XComponentContext context )\n"
            "    {\n        m_xContext = context;\n    }\n\n";
        return;
    }


    // get the one and only add-in service for later use
    std::set< OUString >::const_iterator iter = services.begin();
    OUString sAddinService = *iter;
    if (sAddinService == "com.sun.star.sheet.AddIn") {
        sAddinService = *(++iter);
    }


    // add-in specific fields
    o << "\n    private static final String sADDIN_SERVICENAME = \""
      << sAddinService << "\";\n\n";
    o << "    private static final String sDISPLAYNAME = "
        "\"DisplayName\";\n"
        "    private static final String sDESCRIPTION = "
        "\"Description\";\n"
        "    private static final String sCATEGORY = \"Category\";\n"
        "    private static final String sCATEGORYDISPLAYNAME = "
        "\"CategoryDisplayName\";\n\n";

    o << "    private com.sun.star.container.XHierarchicalNameAccess  "
        "m_xHAccess = null;\n"
        "    private com.sun.star.container.XHierarchicalNameAccess  "
        "m_xCompAccess = null;\n";
    o << "    private java.util.Hashtable<\n        String, "
        "java.util.Hashtable< Integer, String> > m_functionMap = null;\n\n";

    // Constructor
    o << "\n    public " << classname << "( XComponentContext context )\n    {\n"
        "        m_xContext = context;\n\n"
        "        try {\n";

    o << "        m_functionMap = new java.util.Hashtable<\n"
        "            String, java.util.Hashtable< Integer, String > >();\n\n";

    generateFunctionParameterMap(o, options,  manager, interfaces);

    o << "        com.sun.star.lang.XMultiServiceFactory xProvider = \n"
        "            (com.sun.star.lang.XMultiServiceFactory)UnoRuntime."
        "queryInterface(\n"
        "                com.sun.star.lang.XMultiServiceFactory.class,\n"
        "                m_xContext.getServiceManager().createInstanceWithContext("
        "\n                    \"com.sun.star.configuration.ConfigurationProvider\""
        ",\n                    m_xContext));\n\n";

    o << "        String sReadOnlyView = "
        "\"com.sun.star.configuration.ConfigurationAccess\";\n\n";

    o << "        StringBuffer sPath = new StringBuffer(\n"
        "             \"/org.openoffice.Office.CalcAddIns/AddInInfo/\");\n"
        "        sPath.append(sADDIN_SERVICENAME);\n"
        "        sPath.append(\"/AddInFunctions\");\n\n";

    o << "        // create arguments: nodepath\n"
        "         com.sun.star.beans.PropertyValue aArgument = \n"
        "             new com.sun.star.beans.PropertyValue();\n"
        "         aArgument.Name = \"nodepath\";\n"
        "         aArgument.Value = new com.sun.star.uno.Any(\n"
        "             com.sun.star.uno.Type.STRING, sPath.toString());\n\n";

    o << "        Object aArguments[] = new Object[1];\n"
        "        aArguments[0] = new com.sun.star.uno.Any("
        " new com.sun.star.uno.Type(\n"
        "            com.sun.star.beans.PropertyValue.class), aArgument);\n\n";

    o << "        // create the default view using default UI locale\n"
        "         Object xIface = \n"
        "             xProvider.createInstanceWithArguments(sReadOnlyView, "
        "aArguments);\n\n";

    o << "        m_xHAccess = (com.sun.star.container.XHierarchicalNameAccess)\n"
        "             UnoRuntime.queryInterface(\n"
        "                 com.sun.star.container.XHierarchicalNameAccess.class, "
        "xIface);\n\n";

    o << "        // extends arguments to create a view for all locales to get "
        "simple\n        // access to the compatibilityname property\n"
        "        aArguments = new Object[2];\n"
        "        aArguments[0] = new com.sun.star.uno.Any( "
        "new com.sun.star.uno.Type(\n"
        "            com.sun.star.beans.PropertyValue.class), aArgument);\n"
        "        aArgument.Name = \"locale\";\n"
        "        aArgument.Value = new com.sun.star.uno.Any(\n"
        "            com.sun.star.uno.Type.STRING, \"*\");\n"
        "        aArguments[1] = new com.sun.star.uno.Any( "
        " new com.sun.star.uno.Type(\n"
        "            com.sun.star.beans.PropertyValue.class), aArgument);\n\n";

    o << "        // create view for all locales\n"
        "        xIface = xProvider.createInstanceWithArguments(sReadOnlyView, "
        "aArguments);\n\n"
        "        m_xCompAccess = (com.sun.star.container.XHierarchicalNameAccess)\n"
        "            UnoRuntime.queryInterface(\n"
        "                com.sun.star.container.XHierarchicalNameAccess.class, "
        "xIface);\n        }\n"
        "        catch ( com.sun.star.uno.Exception e ) {\n        }\n    }\n\n";

    // add-in helper function
    o << "    // addin configuration property helper function:\n"
        "    String getAddinProperty(String funcName, "
        "String paramName, String propName)\n    {\n"
        "        try {\n            StringBuffer buf = "
        "new StringBuffer(funcName);\n\n"
        "            if (paramName.length() > 0) {\n"
        "                buf.append(\"/Parameters/\");\n"
        "                buf.append(paramName);\n            }\n\n";

    o << "            com.sun.star.beans.XPropertySet xPropSet =\n"
        "                (com.sun.star.beans.XPropertySet)UnoRuntime."
        "queryInterface(\n"
        "                    com.sun.star.beans.XPropertySet.class,\n"
        "                    m_xHAccess.getByHierarchicalName(buf.toString()));\n\n"
        "            return com.sun.star.uno.AnyConverter.toString(\n"
        "                xPropSet.getPropertyValue(propName));\n        }\n"
        "        catch ( com.sun.star.uno.RuntimeException e ) {\n"
        "            throw e;\n        }\n"
        "        catch ( com.sun.star.uno.Exception e ) {\n        }\n"
        "        return \"\";\n    }\n\n";
}


void generateClassDefinition(std::ostream& o,
         ProgramOptions const & options,
         rtl::Reference< TypeManager > const & manager,
         const OString & classname,
         const std::set< OUString >& services,
         const std::set< OUString >& interfaces,
         const AttributeInfo& properties,
         const AttributeInfo& attributes,
         const OUString& propertyhelper, bool supportxcomponent)
{
    o << "\n\npublic final class " << classname << " extends ";

    if (!interfaces.empty()) {
        if (propertyhelper.equals("_")) {
                o << "PropertySet\n";
        } else {
            if (supportxcomponent)
                o << "ComponentBase\n";
            else
                o << "WeakBase\n";
        }
        o << "   implements ";
        std::set< OUString >::const_iterator iter = interfaces.begin();
        while (iter != interfaces.end()) {
            o << (*iter);
            ++iter;
            if (iter != interfaces.end())
                o << ",\n              ";
        }
    }
    o << "\n{\n";

    o << "    private final XComponentContext m_xContext;\n";

    // additional member for add-ons
    if (options.componenttype == 3) {
        o << "    private com.sun.star.frame.XFrame m_xFrame;\n";
    }

    // check property helper
    if (propertyhelper.getLength() > 1)
        o << "    private final PropertySetMixin m_prophlp;\n";

    o << "    private static final String m_implementationName = "
      << classname << ".class.getName();\n";

    if (!services.empty()) {
        o << "    private static final String[] m_serviceNames = {\n";
        std::set< OUString >::const_iterator iter = services.begin();
        while (iter != services.end()) {
            o << "        \"" << (*iter).replace('/','.') << "\"";
            ++iter;
            if (iter != services.end())
                o << ",\n";
            else
                o << " };\n\n";
        }
    }

    // attribute/property members
    if (!properties.empty()) {
        AttributeInfo::const_iterator iter =
            properties.begin();
        o << "    // properties\n";
        while (iter != properties.end()) {
            o << "    protected ";
            printType(o, options, manager, iter->type, false, false);
            o << " m_" << iter->name << ";\n";
            ++iter;
        }
    } else if (!attributes.empty()) {
        AttributeInfo::const_iterator iter =
            attributes.begin();
        o << "    // attributes\n";
        while (iter != attributes.end()) {
            o << "    private ";
            printType(o, options, manager, iter->type, false, false);
            o << " m_" << iter->name << " = ";
            printType(o, options, manager, iter->type, false, true);
            o <<";\n";
            ++iter;
        }
    }

    // special handling of calc add-ins
    if (options.componenttype == 2)
    {
        generateAddinConstructorAndHelper(o, options, manager, classname,
                                          services, interfaces);
    } else {
        o << "\n    public " << classname << "( XComponentContext context )\n"
            "    {\n        m_xContext = context;\n";
        if (propertyhelper.equals("_")) {
            registerProperties(o, properties, "        ");
        } else {
            if (propertyhelper.getLength() > 1) {
                o << propcomment
                  << "        m_prophlp = new PropertySetMixin(m_xContext, this,\n"
                  << "            new Type(" << propertyhelper
                  << ".class), null);\n";
            }
        }
        o << "    };\n\n";

    }

    if (!services.empty())
        generateCompFunctions(o, classname);

    generateMethodBodies(o, options, manager, interfaces,
                         "    ", propertyhelper.getLength() > 1);

    // end of class definition
    o << "}\n";
}

void generateSkeleton(ProgramOptions const & options,
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

    std::vector< OString >::const_iterator iter = types.begin();
    while (iter != types.end()) {
        checkType(manager, b2u(*iter), interfaces, services, properties);
        ++iter;
    }

    if (options.componenttype == 3) {
        // the Protocolhandler service is mandatory for an protocol handler add-on,
        // so it is defaulted. The XDispatchProvider provides Dispatch objects for
        // certain functions and the generated impl object implements XDispatch
        // directly for simplicity reasons.
        checkType(manager, "com.sun.star.frame.ProtocolHandler",
                  interfaces, services, properties);
        checkType(manager, "com.sun.star.frame.XDispatch",
                  interfaces, services, properties);
    }

    if (options.componenttype == 2) {
        if (services.size() != 1) {
            throw CannotDumpException(
                "for calc add-in components one and only one service type is "
                "necessary! Please reference a valid type with the '-t' option.");
        }

        // if backwardcompatible==true the AddIn service needs to be added to the
        // suported service list, the necessary intefaces are mapped to the add-in
        // configuration. Since OO.org 2.0.4 this is obsolete and the add-in is
        // take form the configuration from Calc directly, this simplifies the
        // add-in code
        if (options.backwardcompatible) {
            checkType(manager, "com.sun.star.sheet.AddIn",
                      interfaces, services, properties);
        } else {
            // special case for the optional XLocalization interface. It should be
            // implemented always. But it is parent of the XAddIn and we need it only
            // if backwardcompatible is false.
            if (interfaces.find("com.sun.star.lang.XLocalizable") == interfaces.end()) {
                interfaces.insert("com.sun.star.lang.XLocalizable");
            }
        }
    }


    // check if service object or simple UNO object
    if (!services.empty())
        serviceobject = true;

    OUString propertyhelper = checkPropertyHelper(
        options, manager, services, interfaces, attributes, propinterfaces);
    checkDefaultInterfaces(interfaces, services, propertyhelper);

    if (options.componenttype == 2) {
        if (!propertyhelper.isEmpty())
            std::cerr << "WARNING: interfaces specifying calc add-in functions "
                "shouldn't support attributes!\n";
    }

    supportxcomponent = checkXComponentSupport(manager, interfaces);

    OString compFileName;
    OString tmpFileName;
    std::ostream* pofs = NULL;
    bool standardout = getOutputStream(options, ".java",
                                       &pofs, compFileName, tmpFileName);

    try {
        if (!standardout && options.license) {
            printLicenseHeader(*pofs, compFileName);
        }

        generatePackage(*pofs, options.implname);

        generateImports(*pofs, options, propertyhelper,
                        serviceobject, supportxcomponent);

        OString classname(options.implname);
        sal_Int32 index = 0;
        if ((index = classname.lastIndexOf('.')) > 0)
            classname = classname.copy(index+1);

        generateClassDefinition(*pofs, options, manager, classname, services,
                                interfaces, properties, attributes, propertyhelper,
                                supportxcomponent);

        if ( !standardout && pofs && ((std::ofstream*)pofs)->is_open()) {
            ((std::ofstream*)pofs)->close();
            delete pofs;
            OSL_VERIFY(makeValidTypeFile(compFileName, tmpFileName, sal_False));
        }
    } catch (CannotDumpException & e) {
        std::cerr << "ERROR: " << e.getMessage() << "\n";
        if ( !standardout ) {
            if (pofs && ((std::ofstream*)pofs)->is_open()) {
                ((std::ofstream*)pofs)->close();
                delete pofs;
            }
            // remove existing type file if something goes wrong to ensure
            // consistency
            if (fileExists(compFileName))
                removeTypeFile(compFileName);

            // remove tmp file if something goes wrong
            removeTypeFile(tmpFileName);
        }
    }
}

} }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

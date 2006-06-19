/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javacompskeleton.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:50:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "codemaker/commonjava.hxx"

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

void generateImports(std::ostream & o,
         const std::hash_set< OString, OStringHash >& /*interfaces*/,
         const OString & propertyhelper,
         bool serviceobject, bool supportxcomponent)
{
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
    if (propertyhelper.getLength() > 0) {
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

//     std::hash_set< OString, OStringHash >::const_iterator iter =
//                    interfaces.begin();
//     while (iter != interfaces.end())
//     {
//         o << "import " << ((*iter).getStr()) << ";\n";
//         iter++;
//     }
}

void generateCompFunctions(std::ostream & o, const OString & classname)
{
    o << "    public static XSingleComponentFactory __getComponentFactory("
        "String sImplementationName ) {\n"
        "        XSingleComponentFactory xFactory = null;\n\n"
        "        if ( sImplementationName.equals( m_implementationName ) )\n"
        "            xFactory = Factory.createComponentFactory("
      << classname << ".class, m_serviceNames);\n"
        "        return xFactory;\n    }\n\n";

    o << "    public static boolean __writeRegistryServiceInfo("
        "XRegistryKey xRegistryKey ) {\n"
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


bool checkAttribute(OStringBuffer& attributeValue, sal_uInt16 attribute)
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
        /* com::sun::star::beans::PropertyValue::REMOVEABLE */ 128,
        /* com::sun::star::beans::PropertyValue::OPTIONAL */ 256 };

    for (sal_uInt16 i = 0; i < 9; i++)
    {
        if (attribute & attributes[i]) {
            if (attributeValue.getLength() > 0) {
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
                attributeValue.append("PropertyAttribute.REMOVEABLE");
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
                        TypeManager const & /*manager*/,
                        const AttributeInfo& properties,
                        const OString& indentation)
{
    if (!properties.empty()) {
        bool cast = false;
        OStringBuffer attributeValue;
        for (AttributeInfo::const_iterator i(properties.begin());
             i != properties.end(); ++i)
        {
            if (i->second.second > 0) {
                cast = checkAttribute(attributeValue, i->second.second);
            } else {
                cast = true;
                attributeValue.append('0');
            }

            o << indentation << "registerProperty(\"" << i->first
              << "\", \"m_" << i->first << "\",\n"
              << indentation << "      ";
            if (cast)
                o << "(short)";

            o << attributeValue.makeStringAndClear() << ");\n";
        }
    }
}

void generateXAddInBodies(std::ostream& o, ProgramOptions const & options)
{
    // com.sun.star.lang.XLocalizable:
    // setLocale
    o << "    // com.sun.star.lang.XLocalizable:\n"
        "    public void setLocale(com.sun.star.lang.Locale eLocale)\n    {\n"
        "        m_locale = eLocale;\n    }\n\n";

    // getLocale
    o << "    public com.sun.star.lang.Locale getLocale()\n    {\n"
        "        return m_locale;\n    }\n\n";

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
    if (options.java5) {
        o << "        return getAddinProperty(aProgrammaticFunctionName,\n"
            "                                m_functionMap.get(\n"
            "                                    aProgrammaticFunctionName).get("
            "nArgument),\n"
            "                                sDISPLAYNAME);\n    }\n\n";
    } else {
        o << "        return getAddinProperty(aProgrammaticFunctionName, (String)\n"
            "                                ((java.util.Hashtable)m_functionMap."
            "get(\n                                    aProgrammaticFunctionName))."
            "get(\n                                        new Integer(nArgument))"
            ", sDISPLAYNAME);\n    }\n\n";
    }

    // getArgumentDescription
    o << "    public String getArgumentDescription(String "
        "aProgrammaticFunctionName, int nArgument)\n    {\n";
    if (options.java5) {
        o << "        return getAddinProperty(aProgrammaticFunctionName,\n"
            "                                m_functionMap.get(\n"
            "                                    aProgrammaticFunctionName).get("
            "nArgument),\n"
            "                                sDESCRIPTION);\n    }\n\n";
    } else {
        o << "        return getAddinProperty(aProgrammaticFunctionName, (String)\n"
            "                                ((java.util.Hashtable)m_functionMap."
            "get(\n                                    aProgrammaticFunctionName))."
            "get(\n                                        new Integer(nArgument))"
            ", sDESCRIPTION);\n    }\n\n";
    }
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

void generateMethodBodies(std::ostream& o,
         ProgramOptions const & options,
         TypeManager const & manager,
         const std::hash_set< OString, OStringHash >& interfaces,
         const OString& indentation, bool usepropertymixin)
{
    std::hash_set< OString, OStringHash >::const_iterator iter =
        interfaces.begin();
    codemaker::GeneratedTypeSet generated;
    while (iter != interfaces.end()) {
        OString type(*iter);
        iter++;
        if (type.equals("com.sun.star.lang.XServiceInfo")) {
            generateXServiceInfoBodies(o);
        } else {
            if (options.componenttype == 2) {
                if (type.equals("com.sun.star.lang.XServiceName")) {
                    o << "    // com.sun.star.lang.XServiceName:\n"
                        "    public String getServiceName() {\n"
                        "        return sADDIN_SERVICENAME;\n    }\n";
                    generated.add(type);
                    continue;
                } else if (type.equals("com.sun.star.sheet.XAddIn")) {
                    generateXAddInBodies(o, options);
                    generated.add(type);
                    continue;
                } else if (type.equals("com.sun.star.sheet.XCompatibilityNames")) {
                    generateXCompatibilityNamesBodies(o);
                    generated.add(type);
                    continue;
                }
            }

            typereg::Reader reader(manager.getTypeReader(type.replace('.','/')));
            printMethods(o, options, manager, reader, generated, "_",
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
         TypeManager const & manager, const OString & classname,
         const std::hash_set< OString, OStringHash >& services,
         const std::hash_set< OString, OStringHash >& interfaces)
{
    // get the one and only add-in service for later use
    OString sAddinService = (*services.begin()).replace('/', '.');

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

    o << "    private com.sun.star.lang.Locale m_locale = "
        "new com.sun.star.lang.Locale();\n"
        "    private com.sun.star.container.XHierarchicalNameAccess  "
        "m_xHAccess = null;\n"
        "    private com.sun.star.container.XHierarchicalNameAccess  "
        "m_xCompAccess = null;\n";
    if (options.java5) {
        o << "    private java.util.Hashtable<\n        String, "
            "java.util.Hashtable< Integer, String> > m_functionMap = null;\n\n";
    } else {
        o << "    private java.util.Hashtable m_functionMap = null;\n\n";
    }
    // Constructor
    o << "\n    public " << classname << "( XComponentContext context )\n    {\n"
        "        m_xContext = context;\n\n"
        "        try {\n";

    if (options.java5) {
        o << "        m_functionMap = new java.util.Hashtable<\n"
            "            String, java.util.Hashtable< Integer, "
            "String > >();\n\n";
    } else {
        o << "        m_functionMap = new java.util.Hashtable();\n\n";
    }

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
        "             \"/org.openoffice.Office.Sheet.CalcAddIns/AddInInfo/\");\n"
        "        sPath.append(sADDIN_SERVICENAME);\n"
        "        sPath.append(\"/AddInFunctions\");\n\n";

    o << "        // create arguments: nodepath\n"
        "         com.sun.star.beans.PropertyValue aArgument = \n"
        "             new com.sun.star.beans.PropertyValue();\n"
        "         aArgument.Name = \"nodepath\";\n"
        "         aArgument.Value = new com.sun.star.uno.Any(\n"
        "             com.sun.star.uno.Type.STRING, sPath.toString());\n\n";

    o << "        Object aArguments[] = new Object[1];\n"
        "        aArguments[0] = new com.sun.star.uno.Any(\n"
        "            com.sun.star.beans.PropertyValue.class, aArgument);\n\n";

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
        "        aArguments[0] = new com.sun.star.uno.Any(\n"
        "            com.sun.star.beans.PropertyValue.class, aArgument);\n"
        "        aArgument.Name = \"locale\";\n"
        "        aArgument.Value = new com.sun.star.uno.Any(\n"
        "            com.sun.star.uno.Type.STRING, \"*\");\n"
        "        aArguments[1] = new com.sun.star.uno.Any(\n"
        "            com.sun.star.beans.PropertyValue.class, aArgument);\n\n";

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
         TypeManager const & manager,
         const OString & classname,
         const std::hash_set< OString, OStringHash >& services,
         const std::hash_set< OString, OStringHash >& interfaces,
         const AttributeInfo& properties,
         const AttributeInfo& attributes,
         const OString& propertyhelper, bool supportxcomponent)
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
        std::hash_set< OString, OStringHash >::const_iterator iter =
            interfaces.begin();
        while (iter != interfaces.end()) {
            o << (*iter);
            iter++;
            if (iter != interfaces.end())
                o << ",\n              ";
        }
    }
    o << "\n{\n";

    o << "    private final XComponentContext m_xContext;\n";

    // check property helper
    if (propertyhelper.getLength() > 1)
        o << "    private final PropertySetMixin m_prophlp;\n";

    o << "    private static final String m_implementationName = "
      << classname << ".class.getName();\n";

    if (!services.empty()) {
        o << "    private static final String[] m_serviceNames = {\n";
        std::hash_set< OString, OStringHash >::const_iterator iter =
            services.begin();
        while (iter != services.end()) {
            o << "        \"" << (*iter).replace('/','.') << "\"";
            iter++;
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
            printType(o, options, manager, iter->second.first.replace('.','/'),
                      false, false);
            o << " m_" << iter->first << ";\n";
            iter++;
        }
    } else if (!attributes.empty()) {
        AttributeInfo::const_iterator iter =
            attributes.begin();
        o << "    // attributes\n";
        while (iter != attributes.end()) {
            o << "    private ";
            printType(o, options, manager, iter->second.first.replace('.','/'),
                      false, false);
            o << " m_" << iter->first << " = ";
            printType(o, options, manager, iter->second.first.replace('.','/'),
                      false, true);
            o <<";\n";
            iter++;
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
            registerProperties(o, manager, properties, "        ");
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
                      TypeManager const & manager,
                      std::vector< OString > const & types,
                      OString const & /*delegate*/)
{
    std::hash_set< OString, OStringHash > interfaces;
    std::hash_set< OString, OStringHash > services;
    AttributeInfo properties;
    AttributeInfo attributes;
    std::hash_set< OString, OStringHash > propinterfaces;
    bool serviceobject = false;
    bool supportxcomponent = false;

    std::vector< OString >::const_iterator iter = types.begin();
    while (iter != types.end()) {
        checkType(manager, *iter, interfaces, services, properties);
        iter++;
    }

    if (options.componenttype == 2) {
        if (services.size() != 1) {
            throw CannotDumpException(
                "for calc add-in components one and only one service type is "
                "necessary! Please reference a valid type with the '-t' option.");
        }

        // add AddIn in suported service list, this service is currently necessary
        // to identify all calc add-ins and to support the necessary add-in helper
        // interfaces.
        // This becomes obsolete in the future when this information is collected
        // from the configuration
        checkType(manager, "com.sun.star.sheet.AddIn",
                  interfaces, services, properties);
        // special case for XLocalization, it is exlicitly handled and will be
        // necessary in the future as well
//         if (interfaces.find("com.sun.star.lang.XLocalizable") ==
//             interfaces.end())
//             interfaces.insert("com.sun.star.lang.XLocalizable");
    }


    // check if service object or simple UNO object
    if (!services.empty())
        serviceobject = true;

    OString propertyhelper = checkPropertyHelper(
        options, manager, services, interfaces, attributes, propinterfaces);
    checkDefaultInterfaces(interfaces, services, propertyhelper);

    if (options.componenttype == 2) {
        if (propertyhelper.getLength() > 0)
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

        generateImports(*pofs, interfaces, propertyhelper,
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
    } catch(CannotDumpException& e) {

        std::cerr << "ERROR: " << e.m_message.getStr() << "\n";
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



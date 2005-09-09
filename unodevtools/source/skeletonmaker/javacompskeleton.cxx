/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javacompskeleton.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jsc $ $Date: 2005-09-09 13:50:29 $
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

#include <codemaker/commonjava.hxx>

#include "skeletonjava.hxx"

#include <iostream>

using namespace ::rtl;
using namespace ::codemaker::java;

namespace skeletonmaker { namespace java {

void generatePackage(std::ostream & o, const OString & implname)
{
    short count=0;
    sal_Int32 index = implname.lastIndexOf('.');
    if (index != -1) {
        o << "package " << implname.copy(0, index) << ";\n\n";
    }
}

void generateImports(std::ostream & o,
         const std::hash_set< OString, OStringHash >& interfaces,
         const OString & propertyhelper,
         bool serviceobject, bool supportxcomponent)
{
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
//     while (iter != interfaces.end()) {
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
    o << "    /* com.sun.star.lang.XServiceInfo */\n";
    o << "    public java.lang.String getImplementationName() {\n"
      << "         return m_implementationName;\n    }\n\n";

    o << "    public boolean supportsService( java.lang.String sService ) {\n"
      << "        int len = m_serviceNames.length;\n\n"
      << "        for( int i=0; i < len; i++) {\n"
      << "            if (sService.equals(m_serviceNames[i]))\n"
      << "                return true;\n"
      << "        }\n        return false;\n    }\n\n";

    o << "    public java.lang.String[] getSupportedServiceNames() {\n"
      << "        return m_serviceNames;\n    }\n\n";
}

void generateXPropertySetBodies(std::ostream& o)
{
    o << "    /* com.sun.star.lang.XPropertySet */\n";
    o << "    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()\n"
      "    {\n        return m_prophlp.getPropertySetInfo();\n    }\n\n";

    o << "    public void setPropertyValue(java.lang.String aPropertyName, "
        "java.lang.Object aValue) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.beans.PropertyVetoException, "
        "com.sun.star.lang.IllegalArgumentException,"
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.setPropertyValue(aPropertyName, aValue);\n    }\n\n";

    o << "    public java.lang.Object getPropertyValue(java.lang.String "
        "aPropertyName) throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        return "
        "m_prophlp.getPropertyValue(aPropertyName);\n    }\n\n";

    o << "    public void addPropertyChangeListener(java.lang.String aPropertyName"
        ", com.sun.star.beans.XPropertyChangeListener xListener) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.addPropertyChangeListener(aPropertyName, xListener);\n    }\n\n";

    o << "    public void removePropertyChangeListener(java.lang.String "
        "aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener) "
        "throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.removePropertyChangeListener(aPropertyName, xListener);\n"
        "    }\n\n";

    o << "    public void addVetoableChangeListener(java.lang.String aPropertyName"
        ", com.sun.star.beans.XVetoableChangeListener xListener) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.addVetoableChangeListener(aPropertyName, xListener);\n    }\n\n";

    o << "    public void removeVetoableChangeListener(java.lang.String "
        "aPropertyName, com.sun.star.beans.XVetoableChangeListener xListener) "
        "throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.removeVetoableChangeListener(aPropertyName, xListener);\n }\n\n";
}

void generateXFastPropertySetBodies(std::ostream& o)
{
    o << "    /* com.sun.star.lang.XFastPropertySet */\n";

    o << "    public void setFastPropertyValue(int nHandle, java.lang.Object "
        "aValue) throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.beans.PropertyVetoException, "
        "com.sun.star.lang.IllegalArgumentException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.setFastPropertyValue(nHandle, aValue);\n    }\n\n";

    o << "    public java.lang.Object getFastPropertyValue(int nHandle) throws "
        "com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        return "
        "m_prophlp.getFastPropertyValue(nHandle);\n    }\n\n";
}

void generateXPropertyAccessBodies(std::ostream& o)
{
    o << "    /* com.sun.star.lang.XPropertyAccess */\n";

    o << "    public com.sun.star.beans.PropertyValue[] getPropertyValues()\n"
        " {\n        return m_prophlp.getPropertyValues();\n    }\n\n";

    o << "    public void setPropertyValues(com.sun.star.beans.PropertyValue[] "
        "aProps) throws com.sun.star.beans.UnknownPropertyException, "
        "com.sun.star.beans.PropertyVetoException, "
        "com.sun.star.lang.IllegalArgumentException, "
        "com.sun.star.lang.WrappedTargetException\n    {\n        "
        "m_prophlp.setPropertyValues(aProps);\n    }\n\n";
}


bool checkAttribute(OStringBuffer& attributeValue, sal_uInt16 attribute) {
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

    for (sal_uInt16 i = 0; i < 9; i++) {
        if (attribute & attributes[i]) {
            if (attributeValue.getLength() > 0) {
                cast |= true;
                attributeValue.append("|");
            }
            switch (attributes[i]) {
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
                        TypeManager const & manager,
                        const StringPairHashMap& properties,
                        const OString& indentation)
{
    if (!properties.empty()) {
        StringPairHashMap::const_iterator iter =
            properties.begin();
        bool cast = false;
        OStringBuffer attributeValue;
        while (iter != properties.end()) {
            if (iter->second.second > 0) {
                cast = checkAttribute(attributeValue, iter->second.second);
            } else {
                cast = true;
                attributeValue.append('0');
            }

            o << indentation << "registerProperty(\"" << iter->first
              << "\", \"m_" << iter->first << "\",\n"
              << indentation << "      ";
            if (cast)
                o << "(short)";

            o << attributeValue.makeStringAndClear() << ");\n";
            iter++;
        }
    }
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
        if ( (*iter).equals("com.sun.star.lang.XServiceInfo")) {
                generateXServiceInfoBodies(o);
        } else {
            typereg::Reader reader(manager.getTypeReader((*iter).replace('.','/')));
            printMethods(o, options, manager, reader, generated, "_",
                         indentation, true, usepropertymixin);
        }
        iter++;
    }
}

static const char* propcomment=
"        // use the last parameter of the PropertySetMixin constructor\n"
"        // for your optional attributes if necessary. See the documentation\n"
"        // of the PropertySetMixin helper for further information.\n"
"        // Ensure that your attributes are initialized correctly!\n";

void generateClassDefinition(std::ostream& o,
         ProgramOptions const & options,
         TypeManager const & manager,
         const OString & classname,
         const std::hash_set< OString, OStringHash >& services,
         const std::hash_set< OString, OStringHash >& interfaces,
         const StringPairHashMap& properties,
         const StringPairHashMap& attributes,
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
        o << "    implements ";
        std::hash_set< OString, OStringHash >::const_iterator iter =
            interfaces.begin();
        while (iter != interfaces.end()) {
            o << (*iter);
            iter++;
            if (iter != interfaces.end())
                o << ",\n               ";
        }
    }
    o << "\n{\n";

    o << "    private final XComponentContext m_context;\n";

    // check property helper
    if (propertyhelper.getLength() > 1) {
        o << "    private final PropertySetMixin m_prophlp;\n";
    }

    o << "    private static final java.lang.String m_implementationName = "
      << classname << ".class.getName();\n";

    if (!services.empty()) {
        o << "    private static final java.lang.String[] m_serviceNames = {\n";
        std::hash_set< OString, OStringHash >::const_iterator iter =
            services.begin();
        while (iter != services.end()) {
            o << "        \"" << (*iter).replace('/','.') << "\"";
            iter++;
            if (iter != services.end())
                o << ",\n";
            else
                o << " };\n";
        }
    }
    o << "\n    public " << classname << "( XComponentContext context ) {\n"
      << "        m_context = context;\n";
    if (propertyhelper.equals("_")) {
        registerProperties(o, manager, properties, "        ");
    } else {
        if (propertyhelper.getLength() > 1) {
            o << propcomment
              << "        m_prophlp = new PropertySetMixin(m_context, this,\n"
              << "            new Type(" << propertyhelper << ".class), null);\n";
        }
    }
    o << "    };\n\n";

    if (!services.empty())
        generateCompFunctions(o, classname);

    generateMethodBodies(o, options, manager, interfaces,
                         "    ", propertyhelper.getLength() > 1);

    if (!properties.empty()) {
        StringPairHashMap::const_iterator iter =
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
        StringPairHashMap::const_iterator iter =
            attributes.begin();
        o << "    // attributes\n";
        while (iter != attributes.end()) {
            o << "    private ";
            printType(o, options, manager, iter->second.first.replace('.','/'),
                      false, false);
            o << " m_" << iter->first << ";\n";
            iter++;
        }
    }


    // end of class definition
    o << "}\n";
}

void generateSkeleton(ProgramOptions const & options,
                      TypeManager const & manager,
                      std::vector< OString > const & types,
                      OString const & delegate)
{
    std::hash_set< OString, OStringHash > interfaces;
    std::hash_set< OString, OStringHash > services;
    StringPairHashMap properties;
    StringPairHashMap attributes;
    std::hash_set< OString, OStringHash > propinterfaces;
    bool serviceobject = false;
    bool supportxcomponent = false;

    std::vector< OString >::const_iterator iter = types.begin();
    while (iter != types.end()) {
        checkType(manager, *iter, interfaces, services, properties);
        iter++;
    }

    // check if service object or simple UNO object
    if (!services.empty())
        serviceobject = true;

    OString propertyhelper = checkPropertyHelper(manager, services,
                                                 attributes, propinterfaces);
    checkDefaultInterfaces(interfaces, services, propertyhelper);

    supportxcomponent = checkXComponentSupport(manager, interfaces);

    OString compFileName(createFileNameFromType(
                                  options.outputpath,
                                  options.implname.replace('.','/'),
                                  ".java"));

    OString tmpDir = getTempDir(compFileName);
    FileStream file;
    file.createTempFile(tmpDir);
    OString tmpFileName;

    if(!file.isValid())
    {
        OString message("cannot open ");
        message += compFileName + " for writing";
        throw CannotDumpException(message);
    } else {
        tmpFileName = file.getName();
    }
    file.close();
    std::ofstream oFile(tmpFileName.getStr(), std::ios_base::binary);

    try {
        generatePackage(oFile, options.implname);

        generateImports(oFile, interfaces, propertyhelper,
                        serviceobject, supportxcomponent);

        OString classname(options.implname);
        sal_Int32 index = 0;
        if ((index = classname.lastIndexOf('.')) > 0)
            classname = classname.copy(index+1);

        generateClassDefinition(oFile, options, manager, classname, services,
                                interfaces, properties, attributes, propertyhelper,
                                supportxcomponent);

        oFile.close();
        OSL_VERIFY(makeValidTypeFile(compFileName, tmpFileName, sal_False));
    } catch(CannotDumpException& e) {

        std::cout << "ERROR: " << e.m_message.getStr() << "\n";
        // remove existing type file if something goes wrong to ensure consistency
        if (fileExists(compFileName))
            removeTypeFile(compFileName);

        // remove tmp file if something goes wrong
        removeTypeFile(tmpFileName);
    }
}

} }



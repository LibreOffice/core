/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cppcompskeleton.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jsc $ $Date: 2005-10-21 13:52:34 $
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

#include <codemaker/commoncpp.hxx>

#include "skeletoncpp.hxx"

#include <iostream>

using namespace ::rtl;
using namespace ::codemaker::cpp;

namespace skeletonmaker { namespace cpp {

void generateIncludes(std::ostream & o,
         const std::hash_set< OString, OStringHash >& interfaces,
         const StringPairHashMap& properties,
         OString propertyhelper, bool serviceobject,
         bool supportxcomponent)
{
    if (serviceobject) {
        o << "#include <cppuhelper/factory.hxx>\n"
          << "#include <cppuhelper/implementationentry.hxx>\n";
    }
    if (supportxcomponent) {
        o << "#include <cppuhelper/compbase" << interfaces.size() << ".hxx>\n";
        o << "#include <cppuhelper/basemutex.hxx>\n";
    } else
        o << "#include <cppuhelper/implbase" << interfaces.size() << ".hxx>\n";


    if (propertyhelper.getLength() > 0) {
        if (propertyhelper.equals("_"))
            o << "#include <cppuhelper/rpopshlp.hxx>\n";
        else
            o << "#include <cppuhelper/propertysetmixin.hxx>\n";
    }

    std::hash_set< OString, OStringHash >::const_iterator iter = interfaces.begin();
    while (iter != interfaces.end()) {
        o << "#include <"
          << ((*iter).replace('.', '/').getStr())
          << ".hpp>\n";
        iter++;
    }
}

short generateNamespace(std::ostream & o, const OString & implname)
{
    short count=0;
    sal_Int32 index = implname.lastIndexOf('.');
    if (index == -1) {
        o << "namespace {\n\n";
        count=1;
    } else {
        sal_Int32 nPos=0;
        do
        {
            o << "namespace " << implname.getToken(0, '.', nPos) << " { ";
            count++;
        } while( nPos <= index );
        o << "\n\n";
    }
    return count;
}

void generateServiceHelper(std::ostream & o,
         const OString & implname,
         const std::hash_set< OString, OStringHash >& services)
{
    o << "::rtl::OUString SAL_CALL _getImplementationName() {\n"
      << "    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\n"
      << "        \"" << implname << "\"));\n}\n\n";

    o << "css::uno::Sequence< rtl::OUString > SAL_CALL _getSupportedServiceNames()\n{\n"
      << "    css::uno::Sequence< rtl::OUString > s(" << services.size() << ");\n";

    std::hash_set< OString, OStringHash >::const_iterator iter = services.begin();
    short i=0;
    while (iter != services.end()) {
        o << "    s[" << i++ << "] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\n"
          << "         \"" << (*iter).replace('/','.') << "\"));\n";
        iter++;
    }
    o << "    return s;\n}\n\n";
}

void generateCreateFunction(std::ostream & o, const OString & classname)
{
    o << "css::uno::Reference< css::uno::XInterface > SAL_CALL _create(\n"
      << "    css::uno::Reference< css::uno::XComponentContext > const & context)\n"
      << "        SAL_THROW((css::uno::Exception))\n{\n"
      << "    return static_cast< cppu::OWeakObject * >(new "
      << classname <<  "(context));\n}\n\n";
}

void generateCompFunctions(std::ostream & o, const OString & nmspace)
{
    o << "static struct cppu::ImplementationEntry entries[] = {\n"
      << "    { &" << nmspace << "_create, &"
      << nmspace << "_getImplementationName,\n      &"
      << nmspace << "_getSupportedServiceNames,\n"
      << "      &cppu::createSingleComponentFactory, 0, 0 },\n"
      << "    { 0, 0, 0, 0, 0, 0 }\n};\n\n";

    o << "extern \"C\" void SAL_CALL component_getImplementationEnvironment(\n"
      << "    char const ** envTypeName, uno_Environment **)\n{\n"
      << "    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;\n}\n\n";

    o << "extern \"C\" void * SAL_CALL component_getFactory(\n"
      << "    char const * implName, void * serviceManager, void * registryKey)\n{\n"
      << "    return cppu::component_getFactoryHelper(\n"
      << "        implName, serviceManager, registryKey, entries);\n}\n\n";

    o << "extern \"C\" sal_Bool SAL_CALL component_writeInfo(\n"
      << "    void * serviceManager, void * registryKey)\n{\n"
      << "    return cppu::component_writeInfoHelper("
      << "serviceManager, registryKey, entries);\n}\n";
}

OString generateClassDefinition(std::ostream& o,
         ProgramOptions const & options,
         TypeManager const & manager,
         const OString & classname,
         const std::hash_set< OString, OStringHash >& interfaces,
         const StringPairHashMap& properties,
         const StringPairHashMap& attributes,
         const std::hash_set< OString, OStringHash >& propinterfaces,
         const OString& propertyhelper, bool supportxcomponent)
{
    OStringBuffer parentname(64);
    o << "class " << classname << ":\n";
    if (propertyhelper.getLength() > 1)
        o << "    public cppu::PropertySetMixin< "
          << scopedCppName(propertyhelper, false, true) << " >,\n";

    if (!interfaces.empty()) {
        if (supportxcomponent) {
            parentname.append("cppu::WeakComponentImplHelper");
            parentname.append(static_cast<sal_Int32>(interfaces.size()));
            o << "    private cppu::BaseMutex,\n"
              << "    public cppu::WeakComponentImplHelper"
              << interfaces.size() << "<";
        } else {
            parentname.append("cppu::WeakImplHelper");
            parentname.append(static_cast<sal_Int32>(interfaces.size()));
            o << "    public cppu::WeakImplHelper" << interfaces.size() << "<";
        }
        std::hash_set< OString, OStringHash >::const_iterator iter =
            interfaces.begin();
        while (iter != interfaces.end()) {
            o << "\n        " << scopedCppName(*iter, false, true);
            iter++;
            if (iter != interfaces.end())
                o << ",";
            else
                o << ">\n";
        }
    }
    o << "{\npublic:\n"
      << "    explicit " << classname << "("
      << "css::uno::Reference< css::uno::XComponentContext > const & context);\n\n";

    // overload queryInterface
    if (propertyhelper.getLength() > 0) {
        o << "    /* ::com::sun::star::uno::XInterface */\n"
            "    virtual css::uno::Any SAL_CALL queryInterface("
            "css::uno::Type const & type) throw ("
            "css::uno::RuntimeException);\n";

        OStringBuffer buffer(256);
        buffer.append(parentname);
        buffer.append("< ");
        std::hash_set< OString, OStringHash >::const_iterator iter =
            interfaces.begin();
        while (iter != interfaces.end()) {
            buffer.append(scopedCppName(*iter, false, true));
            iter++;
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

    std::hash_set< OString, OStringHash >::const_iterator iter =
        interfaces.begin();
    codemaker::GeneratedTypeSet generated;
    while (iter != interfaces.end()) {
        typereg::Reader reader(manager.getTypeReader((*iter).replace('.','/')));
        printMethods(o, options, manager, reader, generated, "", "", "    ",
                     true, true);
        o << "\n";
        iter++;
    }

    o << "private:\n    "
      << classname << "(" << classname << " &); // not defined\n"
      << "    void operator =(" << classname << " &); // not defined\n\n"
      << "    virtual ~" << classname << "() {}\n\n";

    // members
    o << "    css::uno::Reference< css::uno::XComponentContext >  m_xContext;\n";

    if (!properties.empty()) {
        StringPairHashMap::const_iterator iter = properties.begin();
        while (iter != properties.end()) {
            o << "    ";
            printType(o, options, manager, iter->second.first.replace('.','/'),
                      1, false, false);
            o << " m_" << iter->first << ";\n";
            iter++;
        }
    } else if (!attributes.empty()) {
        StringPairHashMap::const_iterator iter = attributes.begin();
        while (iter != attributes.end()) {
            o << "    ";
            printType(o, options, manager, iter->second.first.replace('.','/'),
                      1, false, false);
            o << " m_" << iter->first << ";\n";
            iter++;
        }
    }

    o << "};\n\n";


    // generate constructor
    o << classname << "::" << classname
      << "(css::uno::Reference< css::uno::XComponentContext > const & context) :\n";
    if (supportxcomponent) {
        o << "    cppu::WeakComponentImplHelper" << interfaces.size() << "<";
        std::hash_set< OString, OStringHash >::const_iterator iter =
            interfaces.begin();
        while (iter != interfaces.end()) {
            o << "\n        " << scopedCppName(*iter, false, true);
            iter++;
            if (iter != interfaces.end())
                o << ",";
            else
                o << ">(m_aMutex),\n";
        }
    }
    if (propertyhelper.getLength() > 0) {
        o << "    cppu::PropertySetMixin< "
          << scopedCppName(propertyhelper, false, true) << " >(\n"
          << "        context, static_cast< Implements >(\n            ";
        OStringBuffer buffer(128);
        if (propinterfaces.find("com/sun/star/beans/XPropertySet")
            != propinterfaces.end())
            buffer.append("IMPLEMENTS_PROPERTY_SET");
        if (propinterfaces.find("com/sun/star/beans/XFastPropertySet")
            != propinterfaces.end()) {
            if (buffer.getLength() > 0)
                buffer.append(" | IMPLEMENTS_FAST_PROPERTY_SET");
            else
                buffer.append("IMPLEMENTS_FAST_PROPERTY_SET");
        }
        if (propinterfaces.find("com/sun/star/beans/XPropertyAccess")
            != propinterfaces.end()) {
            if (buffer.getLength() > 0)
                buffer.append(" | IMPLEMENTS_PROPERTY_ACCESS");
            else
                buffer.append("IMPLEMENTS_PROPERTY_ACCESS");
        }
        o << buffer.makeStringAndClear()
          << "), css::uno::Sequence< ::rtl::OUString >()),\n";
    }

    o << "    m_xContext(context)\n{}\n\n";

    return parentname.makeStringAndClear();
}

void generateXServiceInfoBodies(std::ostream& o,
                                   const OString & classname)
{
    o << "/* com.sun.star.uno.XServiceInfo */\n"
      << "rtl::OUString  SAL_CALL " << classname << "getImplementationName() "
      << "throw (css::uno::RuntimeException)\n{\n    "
      << "return _getImplementationName();\n}\n\n";

    o << "sal_Bool  SAL_CALL " << classname
      << "supportsService(rtl::OUString const & "
      << "serviceName) throw (css::uno::RuntimeException)\n{\n    "
      << "css::uno::Sequence< rtl::OUString > serviceNames = "
      << "_getSupportedServiceNames();\n    "
      << "for (sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {\n    "
      << "    if (serviceNames[i] == serviceName)\n            return true;\n"
      << "    }\n    return false;\n}\n\n";

    o << "css::uno::Sequence< rtl::OUString >  SAL_CALL " << classname
      << "getSupportedServiceNames() throw (css::uno::RuntimeException)\n{\n    "
      << "return _getSupportedServiceNames();\n}\n\n";
}


void generateMethodBodies(std::ostream& o,
        ProgramOptions const & options,
        TypeManager const & manager,
        const OString & classname,
        const std::hash_set< OString, OStringHash >& interfaces,
        bool usepropertymixin)
{
    OString name(classname.concat("::"));
    std::hash_set< OString, OStringHash >::const_iterator iter =
        interfaces.begin();
    codemaker::GeneratedTypeSet generated;
    while (iter != interfaces.end()) {
        if ( (*iter).equals("com.sun.star.lang.XServiceInfo") ) {
            generateXServiceInfoBodies(o, name);
        } else {
            typereg::Reader reader(manager.getTypeReader((*iter).replace('.','/')));
            printMethods(o, options, manager, reader, generated, "_",
                         name, "", true, true, usepropertymixin);
        }
        iter++;
    }
}

void generateQueryInterface(std::ostream& o,
                            ProgramOptions const & options,
                            TypeManager const & manager,
                            const std::hash_set< OString, OStringHash >& interfaces,
                            OString const & parentname,
                            OString const & classname,
                            OString const & propertyhelper)
{
    if (propertyhelper.getLength() == 0)
        return;

    o << "css::uno::Any " << classname
      << "::queryInterface(css::uno::Type const & type) throw ("
        "css::uno::RuntimeException)\n{\n    ";

    if (propertyhelper.getLength() == 0)
        o << "return ";
    else
        o << "css::uno::Any a(";

    o   << parentname << "<";
    std::hash_set< OString, OStringHash >::const_iterator iter =
        interfaces.begin();
    while (iter != interfaces.end()) {
        o << "\n        " << scopedCppName(*iter, false, true);
        iter++;
        if (iter != interfaces.end())
            o << ",";
        else
            o << ">";
    }

    if (propertyhelper.getLength() == 0) {
        o << "::queryInterface(type);\n";
    } else {
        o << "::queryInterface(type));\n";
        o << "    return a.hasValue() ? a\n        : (";
        if (propertyhelper.equals("_")) {
            o << "cppu::OPropertySetHelper::queryInterface(type));\n";
        } else {
            o << "cppu::PropertySetMixin<\n            ";
            printType(o, options, manager, propertyhelper.replace('.', '/'),
                      0, false, false);
            o << " >::queryInterface(\n               type));\n";
        }
    }
    o << "}\n\n";
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

    OString propertyhelper = checkPropertyHelper(manager, services,
                                                 attributes, propinterfaces);

    checkDefaultInterfaces(interfaces, services, propertyhelper);

    if (interfaces.size() > 12)
        std::cout << "ERROR: the skeletonmaker supports components with only 12 interfaces!\n";

    // check if service object or simple UNO object
    if (!services.empty())
        serviceobject = true;

    supportxcomponent = checkXComponentSupport(manager, interfaces);

    OString compFileName(createFileNameFromType(
                                  options.outputpath,
                                  options.implname.replace('.','/'),
                                  ".cxx"));

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
        generateIncludes(oFile, interfaces, properties, propertyhelper,
                         serviceobject, supportxcomponent);
        // namespace
        oFile << "\n\nnamespace css = ::com::sun::star;\n\n";
        short nm = generateNamespace(oFile, options.implname);
        sal_Int32 index = 0;
        OString nmspace;
        OString classname(options.implname);
        if ((index = classname.lastIndexOf('.')) > 0) {
            nmspace = scopedCppName(classname.copy(0, index));
            nmspace = nmspace.concat("::");
            classname = classname.copy(index+1);
        }

        generateServiceHelper(oFile, options.implname, services);

        OString parentname(
            generateClassDefinition(
                oFile, options, manager,classname, interfaces, properties,
                attributes, propinterfaces, propertyhelper, supportxcomponent));

        generateQueryInterface(oFile, options, manager, interfaces, parentname,
                               classname, propertyhelper);

        generateMethodBodies(oFile, options, manager, classname, interfaces,
                             propertyhelper.getLength() > 1);

        generateCreateFunction(oFile, classname);

        // close namepsace
        for (short i=0; i < nm; i++)
            oFile << "} ";
        oFile << "// closing namespace\n\n";

        generateCompFunctions(oFile, nmspace);

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



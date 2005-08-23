/*************************************************************************
 *
 *  $RCSfile: cppcompskeleton.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2005-08-23 08:29:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "skeletoncpp.hxx"

using namespace ::codemaker::cpp;

namespace skeletonmaker { namespace cpp {

void generateIncludes(std::ostream & o,
         const std::hash_set< rtl::OString, rtl::OStringHash >& interfaces,
         const StringPairHashMap& properties,
         bool attributes, bool propertyhelper, bool serviceobject,
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


    if (attributes && propertyhelper)
        o << "#include <cppuhelper/propertysetmixin.hxx>\n";

    std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter = interfaces.begin();
    while (iter != interfaces.end()) {
        o << "#include <"
          << ((*iter).replace('.', '/').getStr())
          << ".hpp>\n";
        iter++;
    }
}

short generateNamespace(std::ostream & o, const rtl::OString & implname)
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
         const rtl::OString & implname,
         const std::hash_set< rtl::OString, rtl::OStringHash >& services)
{
    o << "rtl::OUString SAL_CALL _getImplementationName() {\n"
      << "    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\n"
      << "        \"" << implname << "\"));\n}\n\n";

    o << "css::uno::Sequence< rtl::OUString > SAL_CALL _getSupportedServiceNames()\n{\n"
      << "    css::uno::Sequence< rtl::OUString > s(" << services.size() << ");\n";

    std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter = services.begin();
    short i=0;
    while (iter != services.end()) {
        o << "    s[" << i++ << "] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\n"
          << "         \"" << (*iter).replace('/','.') << "\"));\n";
        iter++;
    }
    o << "    return s;\n}\n\n";
}

void generateCreateFunction(std::ostream & o, const rtl::OString & classname)
{
    o << "css::uno::Reference< css::uno::XInterface > SAL_CALL _create(\n"
      << "    css::uno::Reference< css::uno::XComponentContext > const & context)\n"
      << "        SAL_THROW((css::uno::Exception))\n{\n"
      << "    return static_cast< cppu::OWeakObject * >(new "
      << classname <<  "(context));\n}\n\n";
}

void generateCompFunctions(std::ostream & o, const rtl::OString & nmspace)
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

void generateClassDefinition(std::ostream& o,
         ProgramOptions const & options,
         TypeManager const & manager,
         const rtl::OString & classname,
         const std::hash_set< rtl::OString, rtl::OStringHash >& interfaces,
         const StringPairHashMap& properties,
         const rtl::OString& propertyhelper, bool supportxcomponent)
{
    o << "class " << classname << ":\n";
    if (propertyhelper.getLength() > 0)
        o << "    public cppuhelper::PropertySetMixin< "
          << scopedCppName(propertyhelper, false, true) << " >,\n";

    if (!interfaces.empty()) {
        if (supportxcomponent)
            o << "    private cppu::BaseMutex,\n"
              << "    public cppu::WeakComponentImplHelper"
              << interfaces.size() << "<";
        else
            o << "    public cppu::WeakImplHelper" << interfaces.size() << "<";
        std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter =
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

    std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter =
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
    }

    o << "};\n\n";


    // generate constructor
    o << classname << "::" << classname
      << "(css::uno::Reference< css::uno::XComponentContext > const & context) :\n";
    if (supportxcomponent) {
        o << "    cppu::WeakComponentImplHelper" << interfaces.size() << "<";
        std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter =
            interfaces.begin();
        while (iter != interfaces.end()) {
            o << "\n        " << scopedCppName(*iter, false, true);
            iter++;
            if (iter != interfaces.end())
                o << ",";
            else
                o << ">(m_mutex),\n";
        }
    }
    if (propertyhelper.getLength() > 0)
        o << "    cppuhelper::PropertySetMixin< "
          << scopedCppName(propertyhelper, false, true) << " >(\n"
          << "        context, static_cast< Implements >(\n            "
          << "IMPLEMENTS_PROPERTY_SET | IMPLEMENTS_FAST_PROPERTY_SET "
          << "| IMPLEMENTS_PROPERTY_ACCESS)),\n";

    o << "    m_xContext(context)\n{}\n\n";
}

void generateXServiceInfoBodies(std::ostream& o,
                                   const rtl::OString & classname)
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
        const rtl::OString & classname,
        const std::hash_set< rtl::OString, rtl::OStringHash >& interfaces)
{
    rtl::OString name(classname.concat("::"));
    std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter =
        interfaces.begin();
    codemaker::GeneratedTypeSet generated;
    while (iter != interfaces.end()) {
        if ( (*iter).equals("com.sun.star.lang.XServiceInfo") ) {
            generateXServiceInfoBodies(o, name);
        } else {
            typereg::Reader reader(manager.getTypeReader((*iter).replace('.','/')));
            printMethods(o, options, manager, reader, generated, "_",
                         name, "", true, true);
        }
        iter++;
    }
}

void generateSkeleton(ProgramOptions const & options,
                         TypeManager const & manager,
                         std::vector< rtl::OString > const & types,
                         rtl::OString const & delegate)
{
    std::hash_set< rtl::OString, rtl::OStringHash > interfaces;
    std::hash_set< rtl::OString, rtl::OStringHash > services;
    StringPairHashMap properties;
    bool attributes = false;
    bool serviceobject = false;
    bool supportxcomponent = false;

    std::vector< rtl::OString >::const_iterator iter = types.begin();
    while (iter != types.end()) {
        checkType(manager, *iter, interfaces, services, properties, attributes);
        iter++;
    }

    rtl::OString propertyhelper = checkPropertyHelper(manager, services);

    checkDefaultInterfaces(interfaces, services, propertyhelper);

    if (interfaces.size() > 12)
        std::cout << "ERROR: the skeletonmaker supports components with only 12 interfaces!\n";

    // check if service object or simple UNO object
    if (!services.empty())
        serviceobject = true;

    supportxcomponent = checkXComponentSupport(manager, interfaces);

    rtl::OString compFileName(createFileNameFromType(
                                  options.outputpath,
                                  options.implname.replace('.','/'),
                                  ".cxx"));

    rtl::OString tmpDir = getTempDir(compFileName);
    FileStream file;
    file.createTempFile(tmpDir);
    rtl::OString tmpFileName;

    if(!file.isValid())
    {
        rtl::OString message("cannot open ");
        message += compFileName + " for writing";
        throw CannotDumpException(message);
    } else {
        tmpFileName = file.getName();
    }
    file.close();
    std::ofstream oFile(tmpFileName.getStr(), std::ios_base::binary);

    try {
        generateIncludes(oFile, interfaces, properties, attributes,
                         false, serviceobject, supportxcomponent);
        // namespace
        oFile << "\n\nnamespace css = ::com::sun::star;\n\n";
        short nm = generateNamespace(oFile, options.implname);
        sal_Int32 index = 0;
        rtl::OString nmspace;
        rtl::OString classname(options.implname);
        if ((index = classname.lastIndexOf('.')) > 0) {
            nmspace = scopedCppName(classname.copy(0, index));
            nmspace = nmspace.concat("::");
            classname = classname.copy(index+1);
        }

        generateServiceHelper(oFile, options.implname, services);

        generateClassDefinition(oFile, options, manager, classname,
                                interfaces, properties, propertyhelper,
                                supportxcomponent);

        generateMethodBodies(oFile, options, manager, classname, interfaces);

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



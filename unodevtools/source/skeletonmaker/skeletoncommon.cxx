/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: skeletoncommon.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jsc $ $Date: 2005-09-09 13:50:32 $
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

#include <osl/thread.hxx>

#include <codemaker/commonjava.hxx>
#include <codemaker/commoncpp.hxx>

#include "skeletoncommon.hxx"

using namespace ::rtl;
using namespace ::codemaker::cpp;

namespace skeletonmaker {

codemaker::UnoType::Sort decomposeResolveAndCheck(
    TypeManager const & manager, OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, OString * name, sal_Int32 * rank,
    std::vector< OString > * arguments)
{
    codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
        manager, type, resolveTypedefs, allowVoid, allowExtraEntities,
        typeClass, name, rank, arguments);
    for (std::vector< OString >::iterator i(arguments->begin());
         i != arguments->end(); ++i)
    {
        RTTypeClass typeClass2;
        OString name2;
        sal_Int32 rank2;
        std::vector< OString > arguments2;
        decomposeResolveAndCheck(
            manager, *i, true, false, false, &typeClass2, &name2, &rank2,
            &arguments2);
    }
    return sort;
}

// collect attributes including inherited attributes
void checkAttributes(TypeManager const & manager,
                     const typereg::Reader& reader,
                     StringPairHashMap& attributes,
                     std::hash_set< OString, OStringHash >& propinterfaces)
{
    OString typeName = codemaker::convertString(reader.getTypeName());
    if (typeName.equals("com/sun/star/beans/XPropertySet") ||
        typeName.equals("com/sun/star/beans/XFastPropertySet") ||
        typeName.equals("com/sun/star/beans/XMultiPropertySet") ||
        typeName.equals("com/sun/star/beans/XPropertyAccess") )
    {
        propinterfaces.insert(typeName);
    }

    for (sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i) {
        typereg::Reader supertype(manager.getTypeReader(
                                  codemaker::convertString(
                                      reader.getSuperTypeName(i))));
        if (!supertype.isValid()) {
            throw CannotDumpException(
                "Bad type library entity "
                + codemaker::convertString(reader.getSuperTypeName(i)));
        }
        checkAttributes(manager, supertype, attributes, propinterfaces);
    }

    for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
        OString fieldName(
            codemaker::convertString(reader.getFieldName(i)).
            replace('/', '.'));
        OString fieldType(
            codemaker::convertString(reader.getFieldTypeName(i)).
            replace('/', '.'));
        attributes.insert(StringPairHashMap::value_type(
                              fieldName,
                              std::pair<OString, sal_Int16>(
                                  fieldType, reader.getFieldFlags(i))));
    }
}

void checkType(TypeManager const & manager,
               OString const & type,
               std::hash_set< OString, OStringHash >& interfaceTypes,
               std::hash_set< OString, OStringHash >& serviceTypes,
               StringPairHashMap& properties)
{

    OString binType(type.replace('.', '/'));
    typereg::Reader reader(manager.getTypeReader(binType));
    if (!reader.isValid()) {
        throw CannotDumpException("Bad type library entity " + binType);
    }

    switch (reader.getTypeClass()) {
    case RT_TYPE_INTERFACE:
    {
        // com/sun/star/lang/XComponent should be also not in the list
        // but it will be used for checking the impl helper and will be
        // removed later if necessary.
        if ( binType.equals("com/sun/star/lang/XTypeProvider") ||
             binType.equals("com/sun/star/uno/XWeak") )
            return;
        if (interfaceTypes.find(type) == interfaceTypes.end()) {
            interfaceTypes.insert(type);
        }
    }
        break;
    case RT_TYPE_SERVICE:
        if (serviceTypes.find(binType) == serviceTypes.end()) {
            serviceTypes.insert(binType);

            if (reader.getSuperTypeCount() > 0) {
                OString supername(codemaker::convertString(
                    reader.getSuperTypeName(0).replace('/', '.')));
                if (interfaceTypes.find(supername) == interfaceTypes.end()) {
                    interfaceTypes.insert(supername);

                    typereg::Reader supertype(manager.getTypeReader(
                                  codemaker::convertString(
                                      reader.getSuperTypeName(0))));
                    if (!supertype.isValid()) {
                        throw CannotDumpException(
                            "Bad type library entity "
                            + codemaker::convertString(reader.getSuperTypeName(0)));
                    }
                }

                // check if constructors are specified, if yes automatically
                // support of XInitialization
                if (reader.getMethodCount() > 0) {
                    OString s("com.sun.star.lang.XInitialization");
                    if (interfaceTypes.find(s) == interfaceTypes.end())
                        interfaceTypes.insert(s);
                }
            } else {
                for (sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i) {
                    OString referenceType(
                        codemaker::convertString(
                            reader.getReferenceTypeName(i)).replace('/', '.'));

                    if ( reader.getReferenceSort(i) == RT_REF_SUPPORTS ) {
                        checkType(manager, referenceType, interfaceTypes,
                                  serviceTypes, properties);
                    } else if ( reader.getReferenceSort(i) == RT_REF_EXPORTS ) {
                        checkType(manager, referenceType, interfaceTypes,
                                  serviceTypes, properties);
                    }
                }

                for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
                    OString fieldName(
                        codemaker::convertString(reader.getFieldName(i)).
                        replace('/', '.'));
                    OString fieldType(
                        codemaker::convertString(reader.getFieldTypeName(i)).
                        replace('/', '.'));

                    properties.insert(StringPairHashMap::value_type(fieldName,
                        std::pair<OString, sal_Int16>(
                             fieldType, reader.getFieldFlags(i))));
                }
            }
        }
        break;
    default:
        OSL_ASSERT(false);
        break;
    }
}

void checkDefaultInterfaces(
         std::hash_set< OString, OStringHash >& interfaces,
         const std::hash_set< OString, OStringHash >& services,
       const OString & propertyhelper)
{
    if (services.empty()) {
        if (interfaces.find("com.sun.star.lang.XServiceInfo") != interfaces.end())
            interfaces.erase("com.sun.star.lang.XServiceInfo");
    } else {
        if (interfaces.find("com.sun.star.lang.XServiceInfo") == interfaces.end())
            interfaces.insert("com.sun.star.lang.XServiceInfo");
    }

    if (propertyhelper.equals("_")) {
        if (interfaces.find("com.sun.star.beans.XPropertySet")
            != interfaces.end())
            interfaces.erase("com.sun.star.beans.XPropertySet");
        if (interfaces.find("com.sun.star.beans.XFastPropertySet")
            != interfaces.end())
            interfaces.erase("com.sun.star.beans.XFastPropertySet");
        if (interfaces.find("com.sun.star.beans.XPropertyAccess")
            != interfaces.end())
            interfaces.erase("com.sun.star.beans.XPropertyAccess");
    }
}

OString checkPropertyHelper(
    TypeManager const & manager,
    const std::hash_set< OString, OStringHash >& services,
    StringPairHashMap& attributes,
    std::hash_set< OString, OStringHash >& propinterfaces)
{
    std::hash_set< OString, OStringHash >::const_iterator iter =
        services.begin();
    bool oldStyleWithProperties = false;
    while (iter != services.end()) {
        typereg::Reader reader(manager.getTypeReader((*iter).replace('.', '/')));

        if (reader.getSuperTypeCount() > 0) {
            typereg::Reader supertype(
                manager.getTypeReader(
                    codemaker::convertString(
                        reader.getSuperTypeName(0))));
            if (!supertype.isValid()) {
                throw CannotDumpException(
                    "Bad type library entity "
                    + codemaker::convertString(
                        reader.getSuperTypeName(0)));
            }

            checkAttributes(manager, supertype, attributes, propinterfaces);

            if (!(attributes.empty() || propinterfaces.empty()))
                return OUStringToOString(supertype.getTypeName().replace('/', '.'),
                                              osl_getThreadTextEncoding());
        } else {
            if (reader.getFieldCount() > 0)
                oldStyleWithProperties = true;
        }
        iter++;
    }

    return (oldStyleWithProperties ? "_" : "");
}


bool checkXComponentSupport(TypeManager const & manager,
                            typereg::Reader const & reader)
{
    static OUString s(RTL_CONSTASCII_USTRINGPARAM(
                               "com/sun/star/lang/XComponent"));
    if (reader.getTypeName().equals(s))
        return true;

    for (sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i) {
        typereg::Reader super(
            manager.getTypeReader(
                codemaker::convertString(
                    reader.getSuperTypeName(i))));
        if (!super.isValid()) {
            throw CannotDumpException(
                "Bad type library entity "
                + codemaker::convertString(
                    reader.getSuperTypeName(i)));
        }
        if (checkXComponentSupport(manager, super))
            return true;
    }

    return false;
}


// if XComponent is directly specified, return true and remove it from the
// supported interfaces list
bool checkXComponentSupport(TypeManager const & manager,
         std::hash_set< OString, OStringHash >& interfaces)
{
    if (interfaces.empty())
        return false;

    std::hash_set< OString, OStringHash >::const_iterator iter =
        interfaces.begin();
    while (iter != interfaces.end()) {
        if ((*iter).equals("com.sun.star.lang.XComponent")) {
            interfaces.erase("com.sun.star.lang.XComponent");
            return true;
        }
        typereg::Reader reader(manager.getTypeReader((*iter).replace('.', '/')));
        if (checkXComponentSupport(manager, reader))
            return true;
        iter++;
    }

    return false;
}

sal_uInt16 checkAdditionalPropertyFlags(typereg::Reader const & reader,
                                        sal_uInt16 field, sal_uInt16 method)
{
    sal_uInt16 flags = 0;
    bool getterSupportsUnknown = false;

    OUString su(RTL_CONSTASCII_USTRINGPARAM(
                   "com/sun/star/beans/UnknownPropertyException"));
    if (method < reader.getMethodCount()
        && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_GET
        && reader.getMethodName(method) == reader.getFieldName(field))
    {
        if (reader.getMethodExceptionCount(method) > 0) {
            for (sal_uInt16 i = 0; i < reader.getMethodExceptionCount(method); ++i)
            {
                if (su.equals(reader.getMethodExceptionTypeName(method, i)))
                    getterSupportsUnknown = true;
            }
        }
        method++;
    }
    if (method < reader.getMethodCount()
        && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_SET
        && reader.getMethodName(method) == reader.getFieldName(field))
    {
        if (reader.getMethodExceptionCount(method) > 0) {
            OUString s(RTL_CONSTASCII_USTRINGPARAM(
                           "com/sun/star/beans/PropertyVetoException"));
            for (sal_uInt16 i = 0; i < reader.getMethodExceptionCount(method); ++i)
            {
                if (s.equals(reader.getMethodExceptionTypeName(method, i)))
                    flags |= RT_ACCESS_CONSTRAINED;
                if (getterSupportsUnknown &&
                    su.equals(reader.getMethodExceptionTypeName(method, i)))
                    flags |= RT_ACCESS_OPTIONAL;
            }
        }
    }
    return flags;
}

}


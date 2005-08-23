/*************************************************************************
 *
 *  $RCSfile: skeletoncommon.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2005-08-23 08:30:44 $
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

#include <osl/thread.hxx>
#include "skeletonmaker.hxx"
#include "skeletoncommon.hxx"

using namespace ::codemaker::cpp;

namespace skeletonmaker {

codemaker::UnoType::Sort decomposeResolveAndCheck(
    TypeManager const & manager, rtl::OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, rtl::OString * name, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments)
{
    codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
        manager, type, resolveTypedefs, allowVoid, allowExtraEntities,
        typeClass, name, rank, arguments);
    for (std::vector< rtl::OString >::iterator i(arguments->begin());
         i != arguments->end(); ++i)
    {
        RTTypeClass typeClass2;
        rtl::OString name2;
        sal_Int32 rank2;
        std::vector< rtl::OString > arguments2;
        decomposeResolveAndCheck(
            manager, *i, true, false, false, &typeClass2, &name2, &rank2,
            &arguments2);
    }
    return sort;
}

void checkType(TypeManager const & manager,
               rtl::OString const & type,
               std::hash_set< rtl::OString, rtl::OStringHash >& interfaceTypes,
               std::hash_set< rtl::OString, rtl::OStringHash >& serviceTypes,
               StringPairHashMap& properties,
               bool& attributes) {

    rtl::OString binType(type.replace('.', '/'));
    typereg::Reader reader(manager.getTypeReader(binType));
    if (!reader.isValid()) {
        throw CannotDumpException("Bad type library entity " + binType);
    }

    switch (reader.getTypeClass()) {
    case RT_TYPE_INTERFACE:
    {
        // com/sun/star/lang/XComponent should also not in the list
        // but it will be used for checking the impl helper and will be
        // removed later if necessary.
        if ( binType.equals("com/sun/star/lang/XTypeProvider") ||
             binType.equals("com/sun/star/uno/XWeak") )
            return;
        if (interfaceTypes.find(type) == interfaceTypes.end()) {
            interfaceTypes.insert(type);
            if (reader.getFieldCount() > 0)
                attributes |= true;
        }
    }
        break;
    case RT_TYPE_SERVICE:
        if (serviceTypes.find(binType) == serviceTypes.end()) {
            serviceTypes.insert(binType);

            if (reader.getSuperTypeCount() > 0) {
                rtl::OString superType(codemaker::convertString(
                    reader.getSuperTypeName(0).replace('/', '.')));
                if (interfaceTypes.find(superType) == interfaceTypes.end())
                    interfaceTypes.insert(superType);
            } else {
                for (sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i) {
                    rtl::OString referenceType(
                        codemaker::convertString(
                            reader.getReferenceTypeName(i)).replace('/', '.'));

                    if ( reader.getReferenceSort(i) == RT_REF_SUPPORTS ) {
                        checkType(manager, referenceType, interfaceTypes,
                                  serviceTypes, properties, attributes);
                    } else if ( reader.getReferenceSort(i) == RT_REF_EXPORTS ) {
                        checkType(manager, referenceType, interfaceTypes,
                                  serviceTypes, properties, attributes);
                    }
                }

                for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
                    rtl::OString fieldName(
                        codemaker::convertString(reader.getFieldName(i)).
                        replace('/', '.'));
                    rtl::OString fieldType(
                        codemaker::convertString(reader.getFieldTypeName(i)).
                        replace('/', '.'));

//                     std::pair<rtl::OString, sal_Int16> prop(
//                               fieldType, reader.getFieldFlags(i));

//                     properties[fieldName] = std::pair<rtl::OString, sal_Int16>(
//                               fieldType, reader.getFieldFlags(i));
                    properties.insert(StringPairHashMap::value_type(fieldName,
                        std::pair<rtl::OString, sal_Int16>(
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
         std::hash_set< rtl::OString, rtl::OStringHash >& interfaces,
         const std::hash_set< rtl::OString, rtl::OStringHash >& services,
       const rtl::OString & propertyhelper)
{
    if (services.empty()) {
        if (interfaces.find("com.sun.star.lang.XServiceInfo") != interfaces.end())
            interfaces.erase("com.sun.star.lang.XServiceInfo");
    } else {
        if (interfaces.find("com.sun.star.lang.XServiceInfo") == interfaces.end())
            interfaces.insert("com.sun.star.lang.XServiceInfo");
    }

    if (propertyhelper.getLength() > 0) {
        if (interfaces.find("com.sun.star.beans.XPropertySet") != interfaces.end())
            interfaces.erase("com.sun.star.beans.XPropertySet");
        if (interfaces.find("com.sun.star.beans.XFastPropertySet") != interfaces.end())
            interfaces.erase("com.sun.star.beans.XFastPropertySet");
        if (interfaces.find("com.sun.star.beans.XMultiPropertySet") != interfaces.end())
            interfaces.erase("com.sun.star.beans.XMultiPropertySet");
    }
}

rtl::OString checkPropertyHelper(TypeManager const & manager,
         const std::hash_set< rtl::OString, rtl::OStringHash >& services)
{
    std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter =
        services.begin();
    bool oldStyleWithProperties = false;
    while (iter != services.end()) {
        typereg::Reader reader(manager.getTypeReader((*iter).replace('.', '/')));

        if (reader.getSuperTypeCount() > 0) {
            typereg::Reader super(
                manager.getTypeReader(
                    codemaker::convertString(
                        reader.getSuperTypeName(0))));
            if (!super.isValid()) {
                throw CannotDumpException(
                    "Bad type library entity "
                    + codemaker::convertString(
                        reader.getSuperTypeName(0)));
            }

            if (super.getFieldCount() > 0)
                return rtl::OUStringToOString(super.getTypeName().replace('/', '.'),
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
    static rtl::OUString s(RTL_CONSTASCII_USTRINGPARAM("com/sun/star/lang/XComponent"));
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
         std::hash_set< rtl::OString, rtl::OStringHash >& interfaces)
{
    if (interfaces.empty())
        return false;

    std::hash_set< rtl::OString, rtl::OStringHash >::const_iterator iter =
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

}


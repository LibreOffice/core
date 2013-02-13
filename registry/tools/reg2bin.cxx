/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "osl/endian.h"
#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "registry/reader.hxx"
#include "registry/refltype.hxx"
#include "registry/registry.hxx"
#include "registry/regtype.h"
#include "registry/types.h"
#include "registry/version.h"
#include "rtl/process.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/textcvt.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/main.h"

namespace {

rtl::OUString getArgumentUrl(sal_uInt32 argument) {
    rtl::OUString arg;
    rtl_getAppCommandArg(argument, &arg.pData);
    rtl::OUString url;
    osl::FileBase::RC e1 = osl::FileBase::getFileURLFromSystemPath(arg, url);
    if (e1 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot convert \"" << arg << "\" to file URL, error code "
            << +e1 << "\n";
        std::exit(EXIT_FAILURE);
    }
    rtl::OUString cwd;
    oslProcessError e2 = osl_getProcessWorkingDir(&cwd.pData);
    if (e2 != osl_Process_E_None) {
        std::cerr
            << "Cannot obtain working directory, error code " << +e2 << "\n";
        std::exit(EXIT_FAILURE);
    }
    rtl::OUString abs;
    e1 = osl::FileBase::getAbsoluteFileURL(cwd, url, abs);
    if (e1 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot make \"" << url
            << "\" into an absolute file URL, error code " << +e1 << "\n";
        std::exit(EXIT_FAILURE);
    }
    return abs;
}

enum Type {
    TYPE_MODULE,
    TYPE_ENUM,
    TYPE_STRUCT,
    TYPE_POLYSTRUCT,
    TYPE_EXCEPTION,
    TYPE_INTERFACE,
    TYPE_TYPEDEF,
    TYPE_CONSTANTS,
    TYPE_NEWSERVICE,
    TYPE_OLDSERVICE,
    TYPE_NEWSINGLETON,
    TYPE_OLDSINGLETON
};

struct Field {
    Field(rtl::OUString const & theName, sal_Int32 theValue):
        name(theName), value(theValue) {}

    Field(rtl::OUString const & theName, rtl::OUString const & theType):
        name(theName), type(theType) {}

    Field(
        rtl::OUString const & theName, rtl::OUString const & theType,
        bool parameterized):
        name(theName), type(theType), bound(parameterized) {}

    Field(
        rtl::OUString const & theName, rtl::OUString const & theType,
        bool theBound, bool theReadOnly,
        std::vector< rtl::OUString > theGetExceptions,
        std::vector< rtl::OUString > theSetExceptions):
        name(theName), type(theType), bound(theBound), readOnly(theReadOnly),
        getExceptions(theGetExceptions), setExceptions(theSetExceptions) {}

    Field(
        rtl::OUString const & theName, rtl::OUString const & theType,
        sal_Int32 flags):
        name(theName), type(theType), value(flags) {}

    rtl::OUString name;
    rtl::OUString type;
    sal_Int32 value;
    bool bound; // aka polymorphic struct type template of parameterized type
    bool readOnly;
    std::vector< rtl::OUString > getExceptions;
    std::vector< rtl::OUString > setExceptions;
};

struct Parameter {
    Parameter(
        rtl::OUString const & theName, rtl::OUString const & theType,
        RTParamMode theMode):
        name(theName), type(theType), mode(theMode) {}

    rtl::OUString name;
    rtl::OUString type;
    RTParamMode mode;
};

struct Method {
    Method(
        rtl::OUString const & theName, rtl::OUString const & theReturnType,
        std::vector< Parameter > const & theParameters,
        std::vector< rtl::OUString > const & theExceptions):
        name(theName), returnType(theReturnType), parameters(theParameters),
        exceptions(theExceptions) {}

    Method(
        rtl::OUString const & theName,
        std::vector< Parameter > const & theParameters,
        std::vector< rtl::OUString > const & theExceptions):
        name(theName), parameters(theParameters), exceptions(theExceptions) {}

    rtl::OUString name;
    rtl::OUString returnType;
    std::vector< Parameter > parameters;
    std::vector< rtl::OUString > exceptions;
};

enum ConstantType {
    CONSTANT_TYPE_BOOLEAN,
    CONSTANT_TYPE_BYTE,
    CONSTANT_TYPE_SHORT,
    CONSTANT_TYPE_UNSIGNED_SHORT,
    CONSTANT_TYPE_LONG,
    CONSTANT_TYPE_UNSIGNED_LONG,
    CONSTANT_TYPE_HYPER,
    CONSTANT_TYPE_UNSIGNED_HYPER,
    CONSTANT_TYPE_FLOAT,
    CONSTANT_TYPE_DOUBLE
};

struct Item {
    Type type;
    std::map< rtl::OUString, Item > elements;
    bool published;
    bool deprecated;
    std::vector< rtl::OUString > supertypes;
    std::vector< rtl::OUString > optionalSupertypes;
    std::vector< Field > fields;
    std::vector< Method > methods;
    std::vector< rtl::OUString > references;
    std::vector< rtl::OUString > optionalReferences;
    ConstantType constantType;
    union {
        bool bo;
        sal_Int8 by;
        sal_Int16 s;
        sal_uInt16 us;
        sal_Int32 l;
        sal_uInt32 ul;
        sal_Int64 h;
        sal_uInt64 uh;
        float f;
        double d;
    } constantValue;
    bool defaultConstructor;
    sal_uInt64 nameOffset;
    sal_uInt64 dataOffset;
};

void insert(
    rtl::OUString const & name, Item const & item,
    std::map< rtl::OUString, Item > * map)
{
    assert(map != 0);
    if(!map->insert(std::make_pair(name, item)).second)
    {
        assert(false);
    }
}

typereg::Reader getReader(RegistryKey & key, std::vector< char > * buffer) {
    assert(buffer != 0);
    RegValueType type;
    sal_uInt32 size;
    RegError e = key.getValueInfo("", &type, &size);
    if (e != REG_NO_ERROR) {
        std::cerr
            << "Cannot get value info about key \"" << key.getName()
            << "\" in registry \"" << key.getRegistryName() << "\", error code "
            << +e << "\n";
        std::exit(EXIT_FAILURE);
    }
    if (type != RG_VALUETYPE_BINARY) {
        std::cerr
            << "Unexpected value type " << +type << " of key \""
            << key.getName() << "\" in registry \"" << key.getRegistryName()
            << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    if (size == 0
        /* || size > std::numeric_limits< std::vector< char >::size_type >::max() */)
    {
        std::cerr
            << "Bad binary value size " << size << " of key \"" << key.getName()
            << "\" in registry \"" << key.getRegistryName() << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    buffer->resize(static_cast< std::vector< char >::size_type >(size));
    e = key.getValue("", &(*buffer)[0]);
    if (e != REG_NO_ERROR) {
        std::cerr
            << "Cannot get binary value of key \"" << key.getName()
            << "\" in registry \"" << key.getRegistryName() << "\", error code "
            << +e << "\n";
        std::exit(EXIT_FAILURE);
    }
    typereg::Reader reader(&(*buffer)[0], size, false, TYPEREG_VERSION_1);
    if (!reader.isValid()) {
        std::cerr
            << "Malformed binary value of key \"" << key.getName()
            << "\" in registry \"" << key.getRegistryName() << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    return reader;
}

void setDeprecated(rtl::OUString const & documentation, Item * item) {
    assert(item != 0);
    item->deprecated = documentation.indexOf("@deprecated") != -1;
        //TODO: this check is somewhat crude
}

void setFlags(typereg::Reader const & reader, Item * item) {
    assert(item != 0);
    item->published = reader.isPublished();
    setDeprecated(reader.getDocumentation(), item);
}

void readModule(
    std::vector< RegistryKey > & roots, RegistryKey & key,
    std::map< rtl::OUString, Item > * map)
{
    RegistryKeyNames names;
    RegError e = key.getKeyNames("", names);
    if (e != REG_NO_ERROR) {
        std::cerr
            << "Cannot get sub-key names of \"" << key.getName()
            << "\" in registry \"" << roots[0].getRegistryName()
            << "\", error code " << +e << "\n";
        std::exit(EXIT_FAILURE);
    }
    for (sal_uInt32 i = 0; i != names.getLength(); ++i) {
        RegistryKey sub;
        e = roots[0].openKey(names.getElement(i), sub);
        if (e != REG_NO_ERROR) {
            std::cerr
                << "Cannot get key \"" << names.getElement(i)
                << "\" in registry \"" << roots[0].getRegistryName()
                << "\", error code " << +e << "\n";
            std::exit(EXIT_FAILURE);
        }
        std::vector< char > buf;
        typereg::Reader reader(getReader(sub, &buf));
        assert(sub.getName().match(key.getName() + "/"));
        rtl::OUString name(
            sub.getName().copy(key.getName().getLength() + std::strlen("/")));
        switch (reader.getTypeClass()) {
        case RT_TYPE_INTERFACE:
            {
                Item item;
                item.type = TYPE_INTERFACE;
                setFlags(reader, &item);
                sal_uInt16 n = reader.getSuperTypeCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    item.supertypes.push_back(
                        reader.getSuperTypeName(j).replace('/', '.'));
                }
                n = reader.getReferenceCount(); // optional base types
                for (sal_uInt16 j = 0; j != n; ++j) {
                    item.optionalSupertypes.push_back(
                        reader.getReferenceTypeName(j).replace('/', '.'));
                }
                sal_uInt16 methodCount = reader.getMethodCount();
                n = reader.getFieldCount(); // attributes
                for (sal_uInt16 j = 0; j != n; ++j) {
                    rtl::OUString attrName(reader.getFieldName(j));
                    std::vector< rtl::OUString > getExcs;
                    std::vector< rtl::OUString > setExcs;
                    for (sal_uInt16 k = 0; k != methodCount; ++k) {
                        if (reader.getMethodName(k) == attrName) {
                            switch (reader.getMethodFlags(k)) {
                            case RT_MODE_ATTRIBUTE_GET:
                                {
                                    sal_uInt16 m
                                        = reader.getMethodExceptionCount(k);
                                    for (sal_uInt16 l = 0; l != m; ++l) {
                                        getExcs.push_back(
                                            reader.
                                            getMethodExceptionTypeName(k, l).
                                            replace('/', '.'));
                                    }
                                    break;
                                }
                            case RT_MODE_ATTRIBUTE_SET:
                                {
                                    sal_uInt16 m
                                        = reader.getMethodExceptionCount(k);
                                    for (sal_uInt16 l = 0; l != m; ++l) {
                                        setExcs.push_back(
                                            reader.
                                            getMethodExceptionTypeName(k, l).
                                            replace('/', '.'));
                                    }
                                    break;
                                }
                            default:
                                std::cerr
                                    << "Method and attribute with same name \""
                                    << attrName
                                    << "\" in interface type with key \""
                                    << sub.getName() << "\" in registry \""
                                    << roots[0].getRegistryName() << "\"\n";
                                std::exit(EXIT_FAILURE);
                            }
                        }
                    }
                    RTFieldAccess flags = reader.getFieldFlags(j);
                    item.fields.push_back(
                        Field(
                            attrName,
                            reader.getFieldTypeName(j).replace('/', '.'),
                            (flags & RT_ACCESS_BOUND) != 0,
                            (flags & RT_ACCESS_READONLY) != 0, getExcs,
                            setExcs));
                }
                for (sal_uInt16 j = 0; j != methodCount; ++j) {
                    RTMethodMode flags = reader.getMethodFlags(j);
                    if (flags != RT_MODE_ATTRIBUTE_GET
                        && flags != RT_MODE_ATTRIBUTE_SET)
                    {
                        std::vector< Parameter > params;
                        sal_uInt16 m = reader.getMethodParameterCount(j);
                        for (sal_uInt16 k = 0; k != m; ++k) {
                            RTParamMode mode = reader.getMethodParameterFlags(
                                j, k);
                            switch (mode) {
                            case RT_PARAM_IN:
                            case RT_PARAM_OUT:
                            case RT_PARAM_INOUT:
                                break;
                            default:
                                std::cerr
                                    << "Unexpected mode " << +mode
                                    << " of parameter \""
                                    << reader.getMethodParameterName(j, k)
                                    << "\" of method \""
                                    << reader.getMethodName(j)
                                    << "\" in interface type with key \""
                                    << sub.getName() << "\" in registry \""
                                    << roots[0].getRegistryName() << "\"\n";
                                std::exit(EXIT_FAILURE);
                            }
                            params.push_back(
                                Parameter(
                                    reader.getMethodParameterName(j, k),
                                    (reader.getMethodParameterTypeName(j, k).
                                     replace('/', '.')),
                                    mode));
                        }
                        std::vector< rtl::OUString > excs;
                        m = reader.getMethodExceptionCount(j);
                        for (sal_uInt16 k = 0; k != m; ++k) {
                            excs.push_back(
                                reader.getMethodExceptionTypeName(j, k).replace(
                                    '/', '.'));
                        }
                        item.methods.push_back(
                            Method(
                                reader.getMethodName(j),
                                reader.getMethodReturnTypeName(j).replace(
                                    '/', '.'),
                                params, excs));
                    }
                }
                insert(name, item, map);
                break;
            }
        case RT_TYPE_MODULE:
            {
                Item item;
                item.type = TYPE_MODULE;
                readModule(roots, sub, &item.elements);
                insert(name, item, map);
                break;
            }
        case RT_TYPE_STRUCT:
            {
                Item item;
                sal_uInt32 n = reader.getReferenceCount();
                if (n == 0) {
                    item.type = TYPE_STRUCT;
                    setFlags(reader, &item);
                    switch (reader.getSuperTypeCount()) {
                    case 0:
                        break;
                    case 1:
                        item.supertypes.push_back(
                            reader.getSuperTypeName(0).replace('/', '.'));
                        break;
                    default:
                        std::cerr
                            << "Unexpected number "
                            << reader.getSuperTypeCount()
                            << (" of super-types of plain struct type with key"
                                " \"")
                            << sub.getName() << "\" in registry \""
                            << roots[0].getRegistryName() << "\"\n";
                        std::exit(EXIT_FAILURE);
                    }
                    n = reader.getFieldCount();
                    for (sal_uInt16 j = 0; j != n; ++j) {
                        item.fields.push_back(
                            Field(
                                reader.getFieldName(j),
                                reader.getFieldTypeName(j).replace('/', '.')));
                    }
                } else {
                    item.type = TYPE_POLYSTRUCT;
                    setFlags(reader, &item);
                    if (reader.getSuperTypeCount() != 0) {
                        std::cerr
                            << "Unexpected number "
                            << reader.getSuperTypeCount()
                            << (" of super-types of polymorphic struct type"
                                " template with key \"")
                            << sub.getName() << "\" in registry \""
                            << roots[0].getRegistryName() << "\"\n";
                        std::exit(EXIT_FAILURE);
                    }
                    for (sal_uInt16 j = 0; j != n; ++j) {
                        item.references.push_back(
                            reader.getReferenceTypeName(j).replace('/', '.'));
                    }
                    n = reader.getFieldCount();
                    for (sal_uInt16 j = 0; j != n; ++j) {
                        item.fields.push_back(
                            Field(
                                reader.getFieldName(j),
                                reader.getFieldTypeName(j).replace('/', '.'),
                                ((reader.getFieldFlags(j)
                                  & RT_ACCESS_PARAMETERIZED_TYPE)
                                 != 0)));
                    }
                }
                insert(name, item, map);
                break;
            }
        case RT_TYPE_ENUM:
            {
                Item item;
                item.type = TYPE_ENUM;
                setFlags(reader, &item);
                sal_uInt16 n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    RTConstValue v(reader.getFieldValue(j));
                    if (v.m_type != RT_TYPE_INT32) {
                        std::cerr
                            << "Unexpected type " << +v.m_type
                            << " of value of field \"" << reader.getFieldName(j)
                            << "\" of enum type with key \"" << sub.getName()
                            << "\" in registry \"" << roots[0].getRegistryName()
                            << "\"\n";
                        std::exit(EXIT_FAILURE);
                    }
                    item.fields.push_back(
                        Field(reader.getFieldName(j), v.m_value.aLong));
                }
                insert(name, item, map);
                break;
            }
        case RT_TYPE_EXCEPTION:
            {
                Item item;
                item.type = TYPE_EXCEPTION;
                setFlags(reader, &item);
                switch (reader.getSuperTypeCount()) {
                case 0:
                    break;
                case 1:
                    item.supertypes.push_back(
                        reader.getSuperTypeName(0).replace('/', '.'));
                    break;
                default:
                    std::cerr
                        << "Unexpected number " << reader.getSuperTypeCount()
                        << " of super-types of exception type with key \""
                        << sub.getName() << "\" in registry \""
                        << roots[0].getRegistryName() << "\"\n";
                    std::exit(EXIT_FAILURE);
                }
                sal_uInt16 n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    item.fields.push_back(
                        Field(
                            reader.getFieldName(j),
                            reader.getFieldTypeName(j).replace('/', '.')));
                }
                insert(name, item, map);
                break;
            }
        case RT_TYPE_TYPEDEF:
            {
                Item item;
                item.type = TYPE_TYPEDEF;
                setFlags(reader, &item);
                if (reader.getSuperTypeCount() != 1) {
                    std::cerr
                        << "Unexpected number " << reader.getSuperTypeCount()
                        << " of super-types of typedef with key \""
                        << sub.getName() << "\" in registry \""
                        << roots[0].getRegistryName() << "\"\n";
                    std::exit(EXIT_FAILURE);
                }
                item.supertypes.push_back(
                    reader.getSuperTypeName(0).replace('/', '.'));
                insert(name, item, map);
                break;
            }
        case RT_TYPE_SERVICE:
            {
                Item item;
                switch (reader.getSuperTypeCount()) {
                case 0:
                    {
                        item.type = TYPE_OLDSERVICE;
                        setFlags(reader, &item);
                        sal_uInt16 n = reader.getReferenceCount();
                        for (sal_uInt16 j = 0; j != n; ++j) {
                            rtl::OUString refName(
                                reader.getReferenceTypeName(j).replace(
                                    '/', '.'));
                            switch (reader.getReferenceSort(j)) {
                            case RT_REF_SUPPORTS:
                                if ((reader.getReferenceFlags(j)
                                     & RT_ACCESS_OPTIONAL)
                                    != 0)
                                {
                                    item.optionalSupertypes.push_back(refName);
                                } else {
                                    item.supertypes.push_back(refName);
                                }
                                break;
                            case RT_REF_EXPORTS:
                                if ((reader.getReferenceFlags(j)
                                     & RT_ACCESS_OPTIONAL)
                                    != 0)
                                {
                                    item.optionalReferences.push_back(refName);
                                } else {
                                    item.references.push_back(refName);
                                }
                                break;
                            default:
                                std::cerr
                                    << "Unexpected mode "
                                    << +reader.getReferenceSort(j)
                                    << " of reference \""
                                    << reader.getReferenceTypeName(j)
                                    << "\" in service with key \""
                                    << sub.getName() << "\" in registry \""
                                    << roots[0].getRegistryName() << "\"\n";
                                std::exit(EXIT_FAILURE);
                            }
                        }
                        n = reader.getFieldCount();
                        for (sal_uInt16 j = 0; j != n; ++j) {
                            RTFieldAccess acc = reader.getFieldFlags(j);
                            sal_Int32 v = 0;
                            if ((acc & RT_ACCESS_READONLY) != 0) {
                                v |= 0x0010;
                            }
                            if ((acc & RT_ACCESS_OPTIONAL) != 0) {
                                v |= 0x0100;
                            }
                            if ((acc & RT_ACCESS_MAYBEVOID) != 0) {
                                v |= 0x0001;
                            }
                            if ((acc & RT_ACCESS_BOUND) != 0) {
                                v |= 0x0002;
                            }
                            if ((acc & RT_ACCESS_CONSTRAINED) != 0) {
                                v |= 0x0004;
                            }
                            if ((acc & RT_ACCESS_TRANSIENT) != 0) {
                                v |= 0x0008;
                            }
                            if ((acc & RT_ACCESS_MAYBEAMBIGUOUS) != 0) {
                                v |= 0x0020;
                            }
                            if ((acc & RT_ACCESS_MAYBEDEFAULT) != 0) {
                                v |= 0x0040;
                            }
                            if ((acc & RT_ACCESS_REMOVEABLE) != 0) {
                                v |= 0x0080;
                            }
                            item.fields.push_back(
                                Field(
                                    reader.getFieldName(j),
                                    reader.getFieldTypeName(j).replace(
                                        '/', '.'),
                                    v));
                        }
                        break;
                    }
                case 1:
                    {
                        item.type = TYPE_NEWSERVICE;
                        setFlags(reader, &item);
                        item.supertypes.push_back(
                            reader.getSuperTypeName(0).replace('/', '.'));
                        sal_uInt16 n = reader.getMethodCount();
                        if (n == 1 && reader.getMethodFlags(0) == 0
                            && reader.getMethodName(0).isEmpty()
                            && reader.getMethodReturnTypeName(0) == "void"
                            && reader.getMethodParameterCount(0) == 0
                            && reader.getMethodExceptionCount(0) == 0)
                        {
                            item.defaultConstructor = true;
                        } else {
                            item.defaultConstructor = false;
                            for (sal_uInt16 j = 0; j != n; ++j) {
                                if (reader.getMethodFlags(j) != RT_MODE_TWOWAY)
                                {
                                    std::cerr
                                        << "Unexpected mode "
                                        << +reader.getMethodFlags(j)
                                        << " of constructor \""
                                        << reader.getMethodName(j)
                                        << "\" in service with key \""
                                        << sub.getName() << "\" in registry \""
                                        << roots[0].getRegistryName() << "\"\n";
                                    std::exit(EXIT_FAILURE);
                                }
                                std::vector< Parameter > params;
                                sal_uInt16 m = reader.getMethodParameterCount(
                                    j);
                                for (sal_uInt16 k = 0; k != m; ++k) {
                                    RTParamMode mode
                                        = reader.getMethodParameterFlags(j, k);
                                    if ((mode & ~RT_PARAM_REST) != RT_PARAM_IN)
                                    {
                                        std::cerr
                                            << "Unexpected mode " << +mode
                                            << " of parameter \""
                                            << reader.getMethodParameterName(
                                                j, k)
                                            << "\" of constructor \""
                                            << reader.getMethodName(j)
                                            << "\" in service with key \""
                                            << sub.getName()
                                            << "\" in registry \""
                                            << roots[0].getRegistryName()
                                            << "\"\n";
                                        std::exit(EXIT_FAILURE);
                                    }
                                    if ((mode & RT_PARAM_REST) != 0
                                        && !(m == 1
                                             && ((reader.
                                                  getMethodParameterTypeName(
                                                      j, 1))
                                                 == "any")))
                                    {
                                        std::cerr
                                            << "Bad rest parameter \""
                                            << reader.getMethodParameterName(
                                                j, k)
                                            << "\" of constructor \""
                                            << reader.getMethodName(j)
                                            << "\" in service with key \""
                                            << sub.getName()
                                            << "\" in registry \""
                                            << roots[0].getRegistryName()
                                            << "\"\n";
                                        std::exit(EXIT_FAILURE);
                                    }
                                    params.push_back(
                                        Parameter(
                                            reader.getMethodParameterName(j, k),
                                            (reader.
                                             getMethodParameterTypeName(j, k).
                                             replace('/', '.')),
                                            mode));
                                }
                                std::vector< rtl::OUString > excs;
                                m = reader.getMethodExceptionCount(j);
                                for (sal_uInt16 k = 0; k != m; ++k) {
                                    excs.push_back(
                                        reader.getMethodExceptionTypeName(j, k).
                                        replace('/', '.'));
                                }
                                item.methods.push_back(
                                    Method(
                                        reader.getMethodName(j), params, excs));
                            }
                        }
                        break;
                    }
                default:
                    std::cerr
                        << "Unexpected number " << reader.getSuperTypeCount()
                        << " of super-types of service with key \""
                        << sub.getName() << "\" in registry \""
                        << roots[0].getRegistryName() << "\"\n";
                    std::exit(EXIT_FAILURE);
                }
                insert(name, item, map);
                break;
            }
        case RT_TYPE_SINGLETON:
            {
                Item item;
                if (reader.getSuperTypeCount() != 1) {
                    std::cerr
                        << "Unexpected number " << reader.getSuperTypeCount()
                        << " of super-types of singleton with key \""
                        << sub.getName() << "\" in registry \""
                        << roots[0].getRegistryName() << "\"\n";
                    std::exit(EXIT_FAILURE);
                }
                rtl::OUString path("/UCR/" + reader.getSuperTypeName(0));
                RegistryKey key2;
                for (std::vector< RegistryKey >::iterator j(roots.begin());
                     j != roots.end(); ++j)
                {
                    e = j->openKey(path, key2);
                    switch (e) {
                    case REG_NO_ERROR:
                        {
                            std::vector< char > buf2;
                            typereg::Reader reader2(getReader(key2, &buf2));
                            switch (reader2.getTypeClass()) {
                            case RT_TYPE_INTERFACE:
                                item.type = TYPE_NEWSINGLETON;
                                goto done;
                            case RT_TYPE_SERVICE:
                                item.type = TYPE_OLDSINGLETON;
                                goto done;
                            default:
                                std::cerr
                                    << "Unexpected type class "
                                    << +reader2.getTypeClass()
                                    << " of super-type with key \""
                                    << key2.getName() << "\" in registry \""
                                    << j->getRegistryName()
                                    << "\" of singleton with key \""
                                    << sub.getName() << "\" in registry \""
                                    << roots[0].getRegistryName() << "\"\n";
                                std::exit(EXIT_FAILURE);
                            }
                            break;
                        }
                    case REG_KEY_NOT_EXISTS:
                        break;
                    default:
                        std::cerr
                            << "Cannot get key \"" << path
                            << "\" in registry \"" << j->getRegistryName()
                            << "\", error code " << +e << "\n";
                        std::exit(EXIT_FAILURE);
                    }
                }
                std::cerr
                    << "Unknown super-type with key \"" << path
                    << "\" of singleton with key \"" << sub.getName()
                    << "\" in registry \"" << roots[0].getRegistryName()
                    << "\"\n";
                std::exit(EXIT_FAILURE);
            done:
                setFlags(reader, &item);
                item.supertypes.push_back(
                    reader.getSuperTypeName(0).replace('/', '.'));
                insert(name, item, map);
                break;
            }
        case RT_TYPE_CONSTANTS:
            {
                Item item;
                item.type = TYPE_CONSTANTS;
                setFlags(reader, &item);
                sal_uInt16 n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    Item item2;
                    setDeprecated(reader.getFieldDocumentation(j), &item2);
                    RTConstValue v(reader.getFieldValue(j));
                    switch (v.m_type) {
                    case RT_TYPE_BOOL:
                        item2.constantType = CONSTANT_TYPE_BOOLEAN;
                        item2.constantValue.bo = v.m_value.aBool;
                        break;
                    case RT_TYPE_BYTE:
                        item2.constantType = CONSTANT_TYPE_BYTE;
                        item2.constantValue.by = v.m_value.aByte;
                        break;
                    case RT_TYPE_INT16:
                        item2.constantType = CONSTANT_TYPE_SHORT;
                        item2.constantValue.s = v.m_value.aShort;
                        break;
                    case RT_TYPE_UINT16:
                        item2.constantType = CONSTANT_TYPE_UNSIGNED_SHORT;
                        item2.constantValue.us = v.m_value.aUShort;
                        break;
                    case RT_TYPE_INT32:
                        item2.constantType = CONSTANT_TYPE_LONG;
                        item2.constantValue.l = v.m_value.aLong;
                        break;
                    case RT_TYPE_UINT32:
                        item2.constantType = CONSTANT_TYPE_UNSIGNED_LONG;
                        item2.constantValue.ul = v.m_value.aULong;
                        break;
                    case RT_TYPE_INT64:
                        item2.constantType = CONSTANT_TYPE_HYPER;
                        item2.constantValue.h = v.m_value.aHyper;
                        break;
                    case RT_TYPE_UINT64:
                        item2.constantType = CONSTANT_TYPE_UNSIGNED_HYPER;
                        item2.constantValue.uh = v.m_value.aUHyper;
                        break;
                    case RT_TYPE_FLOAT:
                        item2.constantType = CONSTANT_TYPE_FLOAT;
                        item2.constantValue.f = v.m_value.aFloat;
                        break;
                    case RT_TYPE_DOUBLE:
                        item2.constantType = CONSTANT_TYPE_DOUBLE;
                        item2.constantValue.d = v.m_value.aDouble;
                        break;
                    default:
                        std::cerr
                            << "Unexpected type " << +v.m_type
                            << " of value of field \"" << reader.getFieldName(j)
                            << "\" of constant group with key \""
                            << sub.getName() << "\" in registry \""
                            << roots[0].getRegistryName() << "\"\n";
                        std::exit(EXIT_FAILURE);
                    }
                    insert(reader.getFieldName(j), item2, &item.elements);
                }
                insert(name, item, map);
                break;
            }
        default:
            std::cerr
                << "Unexpected type class " << +reader.getTypeClass()
                << " of key \"" << sub.getName() << "\" in registry \""
                << roots[0].getRegistryName() << "\"\n";
            std::exit(EXIT_FAILURE);
        }
    }
}

sal_uInt64 getOffset(osl::File & file) {
    sal_uInt64 off;
    osl::FileBase::RC e = file.getPos(off);
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot determine current position in \"" << file.getURL()
            << "\", error code " << +e << "\n";
        std::exit(EXIT_FAILURE);
    }
    return off;
}

void write(osl::File & file, void const * buffer, sal_uInt64 size) {
    sal_uInt64 n;
    osl::FileBase::RC e = file.write(buffer, size, n);
    if (e != osl::FileBase::E_None) {
        std::cerr
            << "Cannot write to \"" << file.getURL() << "\", error code " << +e
            << "\n";
        std::exit(EXIT_FAILURE);
    }
    if (n != size) {
        std::cerr
            << "Bad write of " << n << " instead of " << size << " bytes to \""
            << file.getURL() << "\"\n";
        std::exit(EXIT_FAILURE);
    }
}

void write8(osl::File & file, sal_uInt64 value) {
    if (value > 0xFF) {
        std::cerr << "Cannot write value >= 2^8; input is too large\n";
        std::exit(EXIT_FAILURE);
    }
    unsigned char buf[1];
    buf[0] = value & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void write16(osl::File & file, sal_uInt64 value) {
    if (value > 0xFFFF) {
        std::cerr << "Cannot write value >= 2^16; input is too large\n";
        std::exit(EXIT_FAILURE);
    }
    unsigned char buf[2];
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void write32(osl::File & file, sal_uInt64 value) {
    if (value > 0xFFFFFFFF) {
        std::cerr << "Cannot write value >= 2^32; input is too large\n";
        std::exit(EXIT_FAILURE);
    }
    unsigned char buf[4];
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void write64(osl::File & file, sal_uInt64 value) {
    unsigned char buf[8];
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
    buf[3] = (value >> 32) & 0xFF;
    buf[3] = (value >> 40) & 0xFF;
    buf[3] = (value >> 48) & 0xFF;
    buf[3] = (value >> 56) & 0xFF;
    write(file, buf, SAL_N_ELEMENTS(buf));
}

void writeIso60599Binary32(osl::File & file, float value) {
    union {
        unsigned char buf[4];
        float f; // assuming float is ISO 60599 binary32
    } sa;
    sa.f = value;
#if defined OSL_BIGENDIAN
    std::swap(sa.buf[0], sa.buf[3]);
    std::swap(sa.buf[1], sa.buf[2]);
#endif
    write(file, sa.buf, SAL_N_ELEMENTS(sa.buf));
}

void writeIso60599Binary64(osl::File & file, double value) {
    union {
        unsigned char buf[8];
        float d; // assuming double is ISO 60599 binary64
    } sa;
    sa.d = value;
#if defined OSL_BIGENDIAN
    std::swap(sa.buf[0], sa.buf[7]);
    std::swap(sa.buf[1], sa.buf[6]);
    std::swap(sa.buf[2], sa.buf[5]);
    std::swap(sa.buf[3], sa.buf[4]);
#endif
    write(file, sa.buf, SAL_N_ELEMENTS(sa.buf));
}

rtl::OString toAscii(rtl::OUString const & name) {
    rtl::OString ascii;
    if (!name.convertToString(
            &ascii, RTL_TEXTENCODING_ASCII_US,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        std::cerr << "Cannot convert \"" << name << "\" to US ASCII\n";
        std::exit(EXIT_FAILURE);
    }
    return ascii;
}

sal_uInt64 writeNameNul(osl::File & file, rtl::OUString const & name) {
    rtl::OString ascii(toAscii(name));
    if (ascii.indexOf('\0') != -1) {
        std::cerr << "Name \"" << ascii << "\" contains NUL characters\n";
        std::exit(EXIT_FAILURE);
    }
    sal_uInt64 off = getOffset(file);
    write(file, ascii.getStr(), ascii.getLength() + 1);
    return off;
}

void writeNameLen(osl::File & file, rtl::OUString const & name) {
    static std::map< rtl::OUString, sal_uInt64 > reuse;
    std::map< rtl::OUString, sal_uInt64 >::iterator i(reuse.find(name));
    if (i == reuse.end()) {
        reuse.insert(std::make_pair(name, getOffset(file)));
        rtl::OString ascii(toAscii(name));
        assert(
            (static_cast< sal_uInt64 >(ascii.getLength()) & 0x80000000) == 0);
        write32(
            file, static_cast< sal_uInt64 >(ascii.getLength()) | 0x80000000);
        write(file, ascii.getStr(), ascii.getLength());
    } else {
        write32(file, i->second);
    }
}

void writeType(osl::File & file, Item const & item, bool flag = false) {
    assert(item.type != TYPE_MODULE);
    sal_uInt64 v = item.type;
    if (item.published) {
        v |= 0x80;
    }
    if (item.deprecated) {
        v |= 0x40;
    }
    if (flag) {
        v |= 0x20;
    }
    write8(file, v);
}

sal_uInt64 writeMap(
    osl::File & file, std::map< rtl::OUString, Item > & map, bool root)
{
    for (std::map< rtl::OUString, Item >::iterator i(map.begin());
         i != map.end(); ++i)
    {
        switch (i->second.type) {
        case TYPE_MODULE:
            i->second.dataOffset = writeMap(file, i->second.elements, false);
            break;
        case TYPE_ENUM:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            write32(file, i->second.fields.size());
            for (std::vector< Field >::const_iterator j(
                     i->second.fields.begin());
                 j != i->second.fields.end(); ++j)
            {
                writeNameLen(file, j->name);
                write32(file, static_cast< sal_uInt32 >(j->value));
            }
            break;
        case TYPE_STRUCT:
        case TYPE_EXCEPTION:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second, !i->second.supertypes.empty());
            if (!i->second.supertypes.empty()) {
                writeNameLen(file, i->second.supertypes[0]);
            }
            write32(file, i->second.fields.size());
            for (std::vector< Field >::const_iterator j(
                     i->second.fields.begin());
                 j != i->second.fields.end(); ++j)
            {
                writeNameLen(file, j->name);
                writeNameLen(file, j->type);
            }
            break;
        case TYPE_POLYSTRUCT:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            write32(file, i->second.references.size());
            for (std::vector< rtl::OUString >::const_iterator j(
                     i->second.references.begin());
                 j != i->second.references.end(); ++j)
            {
                writeNameLen(file, *j);
            }
            write32(file, i->second.fields.size());
            for (std::vector< Field >::const_iterator j(
                     i->second.fields.begin());
                 j != i->second.fields.end(); ++j)
            {
                sal_uInt64 f = 0;
                if (j->bound) {
                    f |= 0x01;
                }
                write8(file, f);
                writeNameLen(file, j->name);
                writeNameLen(file, j->type);
            }
            break;
        case TYPE_INTERFACE:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            write32(file, i->second.supertypes.size());
            for (std::vector< rtl::OUString >::const_iterator j(
                     i->second.supertypes.begin());
                 j != i->second.supertypes.end(); ++j)
            {
                writeNameLen(file, *j);
            }
            write32(file, i->second.optionalSupertypes.size());
            for (std::vector< rtl::OUString >::const_iterator j(
                     i->second.optionalSupertypes.begin());
                 j != i->second.optionalSupertypes.end(); ++j)
            {
                writeNameLen(file, *j);
            }
            write32(file, i->second.fields.size());
            for (std::vector< Field >::const_iterator j(
                     i->second.fields.begin());
                 j != i->second.fields.end(); ++j)
            {
                sal_uInt64 f = 0;
                if (j->bound) {
                    f |= 0x01;
                }
                if (j->readOnly) {
                    f |= 0x02;
                }
                write8(file, f);
                writeNameLen(file, j->name);
                writeNameLen(file, j->type);
                write32(file, j->getExceptions.size());
                for (std::vector< rtl::OUString >::const_iterator k(
                         j->getExceptions.begin());
                     k != j->getExceptions.end(); ++k)
                {
                    writeNameLen(file, *k);
                }
                if (!j->readOnly) {
                    write32(file, j->setExceptions.size());
                    for (std::vector< rtl::OUString >::const_iterator k(
                             j->setExceptions.begin());
                         k != j->setExceptions.end(); ++k)
                    {
                        writeNameLen(file, *k);
                    }
                }
            }
            write32(file, i->second.methods.size());
            for (std::vector< Method >::const_iterator j(
                     i->second.methods.begin());
                 j != i->second.methods.end(); ++j)
            {
                writeNameLen(file, j->name);
                writeNameLen(file, j->returnType);
                write32(file, j->parameters.size());
                for (std::vector< Parameter >::const_iterator k(
                         j->parameters.begin());
                     k != j->parameters.end(); ++k)
                {
                    sal_uInt64 f = sal_uInt64();
                    switch (k->mode) {
                    case RT_PARAM_IN:
                        f = 0;
                        break;
                    case RT_PARAM_OUT:
                        f = 1;
                        break;
                    case RT_PARAM_INOUT:
                        f = 2;
                        break;
                    default:
                        std::abort(); // this cannot happen
                    }
                    write8(file, f);
                    writeNameLen(file, k->name);
                    writeNameLen(file, k->type);
                }
                write32(file, j->exceptions.size());
                for (std::vector< rtl::OUString >::const_iterator k(
                         j->exceptions.begin());
                     k != j->exceptions.end(); ++k)
                {
                    writeNameLen(file, *k);
                }
            }
            break;
        case TYPE_TYPEDEF:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            assert(i->second.supertypes.size() == 1);
            writeNameLen(file, i->second.supertypes[0]);
            break;
        case TYPE_CONSTANTS:
            for (std::map< rtl::OUString, Item >::iterator j(
                     i->second.elements.begin());
                 j != i->second.elements.end(); ++j)
            {
                j->second.dataOffset = getOffset(file);
                sal_uInt64 v = j->second.constantType;
                if (j->second.deprecated) {
                    v |= 0x80;
                }
                write8(file, v);
                switch (j->second.constantType) {
                case CONSTANT_TYPE_BOOLEAN:
                    write8(file, j->second.constantValue.bo ? 1 : 0);
                    break;
                case CONSTANT_TYPE_BYTE:
                    write8(
                        file,
                        static_cast< sal_uInt8 >(j->second.constantValue.by));
                    break;
                case CONSTANT_TYPE_SHORT:
                    write16(
                        file,
                        static_cast< sal_uInt16 >(j->second.constantValue.s));
                    break;
                case CONSTANT_TYPE_UNSIGNED_SHORT:
                    write16(file, j->second.constantValue.us);
                    break;
                case CONSTANT_TYPE_LONG:
                    write32(
                        file,
                        static_cast< sal_uInt32 >(j->second.constantValue.l));
                    break;
                case CONSTANT_TYPE_UNSIGNED_LONG:
                    write32(file, j->second.constantValue.ul);
                    break;
                case CONSTANT_TYPE_HYPER:
                    write64(
                        file,
                        static_cast< sal_uInt64 >(j->second.constantValue.h));
                    break;
                case CONSTANT_TYPE_UNSIGNED_HYPER:
                    write64(file, j->second.constantValue.uh);
                    break;
                case CONSTANT_TYPE_FLOAT:
                    writeIso60599Binary32(file, j->second.constantValue.f);
                    break;
                case CONSTANT_TYPE_DOUBLE:
                    writeIso60599Binary64(file, j->second.constantValue.d);
                    break;
                default:
                    std::abort(); // this cannot happen
                }
            }
            for (std::map< rtl::OUString, Item >::iterator j(
                     i->second.elements.begin());
                 j != i->second.elements.end(); ++j)
            {
                j->second.nameOffset = writeNameNul(file, j->first);
            }
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            write32(file, i->second.elements.size());
                // overflow from std::map::size_type -> sal_uInt64 is
                // unrealistic
            for (std::map< rtl::OUString, Item >::iterator j(
                     i->second.elements.begin());
                 j != i->second.elements.end(); ++j)
            {
                write32(file, j->second.nameOffset);
                write32(file, j->second.dataOffset);
            }
            break;
        case TYPE_NEWSERVICE:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second, i->second.defaultConstructor);
            assert(i->second.supertypes.size() == 1);
            writeNameLen(file, i->second.supertypes[0]);
            if (!i->second.defaultConstructor) {
                write32(file, i->second.methods.size());
                for (std::vector< Method >::const_iterator j(
                         i->second.methods.begin());
                     j != i->second.methods.end(); ++j)
                {
                    writeNameLen(file, j->name);
                    write32(file, j->parameters.size());
                    for (std::vector< Parameter >::const_iterator k(
                             j->parameters.begin());
                         k != j->parameters.end(); ++k)
                    {
                        sal_uInt64 f = 0;
                        if ((k->mode & RT_PARAM_REST) != 0) {
                            f |= 0x04;
                        }
                        write8(file, f);
                        writeNameLen(file, k->name);
                        writeNameLen(file, k->type);
                    }
                    write32(file, j->exceptions.size());
                    for (std::vector< rtl::OUString >::const_iterator k(
                             j->exceptions.begin());
                         k != j->exceptions.end(); ++k)
                    {
                        writeNameLen(file, *k);
                    }
                }
            }
            break;
        case TYPE_OLDSERVICE:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            write32(file, i->second.references.size());
            for (std::vector< rtl::OUString >::const_iterator j(
                     i->second.references.begin());
                 j != i->second.references.end(); ++j)
            {
                writeNameLen(file, *j);
            }
            write32(file, i->second.optionalReferences.size());
            for (std::vector< rtl::OUString >::const_iterator j(
                     i->second.optionalReferences.begin());
                 j != i->second.optionalReferences.end(); ++j)
            {
                writeNameLen(file, *j);
            }
            write32(file, i->second.supertypes.size());
            for (std::vector< rtl::OUString >::const_iterator j(
                     i->second.supertypes.begin());
                 j != i->second.supertypes.end(); ++j)
            {
                writeNameLen(file, *j);
            }
            write32(file, i->second.optionalSupertypes.size());
            for (std::vector< rtl::OUString >::const_iterator j(
                     i->second.optionalSupertypes.begin());
                 j != i->second.optionalSupertypes.end(); ++j)
            {
                writeNameLen(file, *j);
            }
            write32(file, i->second.fields.size());
            for (std::vector< Field >::const_iterator j(
                     i->second.fields.begin());
                 j != i->second.fields.end(); ++j)
            {
                write16(file, static_cast< sal_uInt16 >(j->value));
                writeNameLen(file, j->name);
                writeNameLen(file, j->type);
            }
            break;
        case TYPE_NEWSINGLETON:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            assert(i->second.supertypes.size() == 1);
            writeNameLen(file, i->second.supertypes[0]);
            break;
        case TYPE_OLDSINGLETON:
            i->second.dataOffset = getOffset(file);
            writeType(file, i->second);
            assert(i->second.supertypes.size() == 1);
            writeNameLen(file, i->second.supertypes[0]);
            break;
        }
    }
    for (std::map< rtl::OUString, Item >::iterator i(map.begin());
         i != map.end(); ++i)
    {
        i->second.nameOffset = writeNameNul(file, i->first);
    }
    sal_uInt64 off = getOffset(file);
    if (!root) {
        write8(file, 0); // TYPE_MODULE
        write32(file, map.size());
            // overflow from std::map::size_type -> sal_uInt64 is unrealistic
    }
    for (std::map< rtl::OUString, Item >::iterator i(map.begin());
         i != map.end(); ++i)
    {
        write32(file, i->second.nameOffset);
        write32(file, i->second.dataOffset);
    }
    return off;
}

}

SAL_IMPLEMENT_MAIN() {
    sal_uInt32 args = rtl_getAppCommandArgCount();
    if (args < 2) {
        std::cerr
            << ("Usage: reg2bin <.bin file> <.rdb file> <additional .rdb files>"
                "\n");
        std::exit(EXIT_FAILURE);
    }
    std::vector< RegistryKey > roots;
    for (sal_uInt32 i = 1; i != args; ++i) {
        rtl::OUString url(getArgumentUrl(i));
        Registry reg;
        RegError e = reg.open(url, REG_READONLY);
        if (e != REG_NO_ERROR) {
            std::cerr
                << "Cannot open registry \"" << url << "\", error code " << +e
                << "\n";
            std::exit(EXIT_FAILURE);
        }
        RegistryKey root;
        e = reg.openRootKey(root);
        if (e != REG_NO_ERROR) {
            std::cerr
                << "Cannot get root key of registry \"" << url
                << "\", error code " << +e << "\n";
            std::exit(EXIT_FAILURE);
        }
        roots.push_back(root);
    }
    RegistryKey ucr;
    RegError e1 = roots[0].openKey("UCR", ucr);
    if (e1 != REG_NO_ERROR) {
        std::cerr
            << "Cannot get /UCR key of registry \""
            << roots[0].getRegistryName() << "\", error code " << +e1 << "\n";
        std::exit(EXIT_FAILURE);
    }
    std::map< rtl::OUString, Item > map;
    readModule(roots, ucr, &map);
    osl::File f(getArgumentUrl(0));
    osl::FileBase::RC e2 = f.open(osl_File_OpenFlag_Write);
    if (e2 == osl::FileBase::E_NOENT) {
        e2 = f.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);
    }
    if (e2 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot open \"" << f.getURL() << "\" for writing, error code "
            << +e2 << "\n";
        std::exit(EXIT_FAILURE);
    }
    write(f, "UNOIDL\0\xFF", 8);
    write32(f, 0); // root map offset
    write32(f, 0); // root map size
    sal_uInt64 off = writeMap(f, map, true);
    e2 = f.setSize(getOffset(f)); // truncate in case it already existed
    if (e2 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot set size of \"" << f.getURL() << "\", error code "
            << +e2 << "\n";
        std::exit(EXIT_FAILURE);
    }

    e2 = f.setPos(osl_Pos_Absolut, 8);
    if (e2 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot rewind current position in \"" << f.getURL()
            << "\", error code " << +e2 << "\n";
        std::exit(EXIT_FAILURE);
    }
    write32(f, off);
    write32(f, map.size());
        // overflow from std::map::size_type -> sal_uInt64 is unrealistic
    e2 = f.close();
    if (e2 != osl::FileBase::E_None) {
        std::cerr
            << "Cannot close \"" << f.getURL()
            << "\" after writing, error code " << +e2 << "\n";
        std::exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

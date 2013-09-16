/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>
#include <cstring>
#include <vector>

#include "registry/reader.hxx"
#include "registry/registry.hxx"
#include "registry/regtype.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "unoidl/unoidl.hxx"

#include "legacyprovider.hxx"

namespace unoidl { namespace detail {

namespace {

std::vector< OUString > translateAnnotations(OUString const & documentation) {
    std::vector< OUString > ans;
    if (documentation.indexOf("@deprecated") != -1) {
        //TODO: this check is somewhat crude
        ans.push_back("deprecated");
    }
    return ans;
}

ConstantValue translateConstantValue(
    RegistryKey & key, RTConstValue const & value)
{
    switch (value.m_type) {
    case RT_TYPE_BOOL:
        return ConstantValue(static_cast< bool >(value.m_value.aBool));
    case RT_TYPE_BYTE:
        return ConstantValue(value.m_value.aByte);
    case RT_TYPE_INT16:
        return ConstantValue(value.m_value.aShort);
    case RT_TYPE_UINT16:
        return ConstantValue(value.m_value.aUShort);
    case RT_TYPE_INT32:
        return ConstantValue(value.m_value.aLong);
    case RT_TYPE_UINT32:
        return ConstantValue(value.m_value.aULong);
    case RT_TYPE_INT64:
        return ConstantValue(value.m_value.aHyper);
    case RT_TYPE_UINT64:
        return ConstantValue(value.m_value.aUHyper);
    case RT_TYPE_FLOAT:
        return ConstantValue(value.m_value.aFloat);
    case RT_TYPE_DOUBLE:
        return ConstantValue(value.m_value.aDouble);
    default:
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: unexpected type " + OUString::number(value.m_type)
             + " of value of a field  of constant group with key "
             + key.getName()));
    }
}

rtl::Reference< Entity > readEntity(
    rtl::Reference< Manager > const & manager, RegistryKey & ucr,
    RegistryKey & key, OUString const & path, bool probe);

class Cursor: public MapCursor {
public:
    Cursor(
        rtl::Reference< Manager > const & manager, RegistryKey const & ucr,
        RegistryKey const & key);

private:
    virtual ~Cursor() throw () {}

    virtual rtl::Reference< Entity > getNext(OUString * name);

    rtl::Reference< Manager > manager_;
    RegistryKey ucr_;
    RegistryKey key_;
    OUString prefix_;
    RegistryKeyNames names_;
    sal_uInt32 index_;
};

Cursor::Cursor(
    rtl::Reference< Manager > const & manager, RegistryKey const & ucr,
    RegistryKey const & key):
    manager_(manager), ucr_(ucr), key_(key), index_(0)
{
    if (ucr_.isValid()) {
        prefix_ = key_.getName();
        if (!prefix_.endsWith("/")) {
            prefix_ += "/";
        }
        RegError e = key_.getKeyNames("", names_);
        if (e != REG_NO_ERROR) {
            throw FileFormatException(
                key_.getRegistryName(),
                ("legacy format: cannot get sub-key names of " + key_.getName()
                 + ": " + OUString::number(e)));
        }
    }
}

rtl::Reference< Entity > Cursor::getNext(OUString * name) {
    assert(name != 0);
    rtl::Reference< Entity > ent;
    if (index_ != names_.getLength()) {
        OUString path(names_.getElement(index_));
        assert(path.match(prefix_));
        *name = path.copy(prefix_.getLength());
        ent = readEntity(manager_, ucr_, key_, *name, false);
        assert(ent.is());
        ++index_;
    }
    return ent;
}

class Module: public ModuleEntity {
public:
    Module(
        rtl::Reference< Manager > const & manager, RegistryKey const & ucr,
        RegistryKey const & key):
        manager_(manager), ucr_(ucr), key_(key)
    {}

private:
    virtual ~Module() throw () {}

    virtual std::vector< OUString > getMemberNames() const;

    virtual rtl::Reference< MapCursor > createCursor() const
    { return new Cursor(manager_, ucr_, key_); }

    rtl::Reference< Manager > manager_;
    RegistryKey ucr_;
    mutable RegistryKey key_;
};

std::vector< OUString > Module::getMemberNames() const {
    RegistryKeyNames names;
    RegError e = key_.getKeyNames("", names);
    if (e != REG_NO_ERROR) {
        throw FileFormatException(
            key_.getRegistryName(),
            ("legacy format: cannot get sub-key names of " + key_.getName()
             + ": " + OUString::number(e)));
    }
    std::vector< OUString > ns;
    for (sal_uInt32 i = 0; i != names.getLength(); ++i) {
        ns.push_back(names.getElement(i));
    }
    return ns;
}

typereg::Reader getReader(RegistryKey & key, std::vector< char > * buffer) {
    assert(buffer != 0);
    RegValueType type;
    sal_uInt32 size;
    RegError e = key.getValueInfo("", &type, &size);
    if (e != REG_NO_ERROR) {
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: cannot get value info about key " + key.getName()
             + ": " + OUString::number(e)));
    }
    if (type != RG_VALUETYPE_BINARY) {
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: unexpected value type " + OUString::number(type)
             + " of key " + key.getName()));
    }
    if (size == 0
        /*TODO: || size > std::numeric_limits< std::vector< char >::size_type >::max() */)
    {
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: bad binary value size " + OUString::number(size)
             + " of key " + key.getName()));
    }
    buffer->resize(static_cast< std::vector< char >::size_type >(size));
    e = key.getValue("", &(*buffer)[0]);
    if (e != REG_NO_ERROR) {
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: cannot get binary value of key " + key.getName()
             + ": " + OUString::number(e)));
    }
    typereg::Reader reader(&(*buffer)[0], size, false, TYPEREG_VERSION_1);
    if (!reader.isValid()) {
        throw FileFormatException(
            key.getRegistryName(),
            "legacy format: malformed binary value of key " + key.getName());
    }
    return reader;
}

rtl::Reference< Entity > readEntity(
    rtl::Reference< Manager > const & manager, RegistryKey & ucr,
    RegistryKey & key, OUString const & path, bool probe)
{
    assert(manager.is());
    RegistryKey sub;
    RegError e = key.openKey(path, sub);
    switch (e) {
    case REG_NO_ERROR:
        break;
    case REG_KEY_NOT_EXISTS:
        if (probe) {
            return rtl::Reference< Entity >();
        }
        // fall through
    default:
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: cannot open sub-key " + path + " of "
             + key.getName() + ": " + OUString::number(e)));
    }
    std::vector< char > buf;
    typereg::Reader reader(getReader(sub, &buf));
    switch (reader.getTypeClass()) {
    case RT_TYPE_INTERFACE:
        {
            std::vector< AnnotatedReference > mandBases;
            sal_uInt16 n = reader.getSuperTypeCount();
            for (sal_uInt16 j = 0; j != n; ++j) {
                mandBases.push_back(
                    AnnotatedReference(
                        reader.getSuperTypeName(j).replace('/', '.'),
                        std::vector< OUString >()));
            }
            std::vector< AnnotatedReference > optBases;
            n = reader.getReferenceCount();
            for (sal_uInt16 j = 0; j != n; ++j) {
                optBases.push_back(
                    AnnotatedReference(
                        reader.getReferenceTypeName(j).replace('/', '.'),
                        translateAnnotations(
                            reader.getReferenceDocumentation(j))));
            }
            sal_uInt16 methodCount = reader.getMethodCount();
            std::vector< InterfaceTypeEntity::Attribute > attrs;
            n = reader.getFieldCount(); // attributes
            for (sal_uInt16 j = 0; j != n; ++j) {
                OUString attrName(reader.getFieldName(j));
                std::vector< OUString > getExcs;
                std::vector< OUString > setExcs;
                for (sal_uInt16 k = 0; k != methodCount; ++k) {
                    if (reader.getMethodName(k) == attrName) {
                        switch (reader.getMethodFlags(k)) {
                        case RT_MODE_ATTRIBUTE_GET:
                            {
                                sal_uInt16 m
                                    = reader.getMethodExceptionCount(k);
                                for (sal_uInt16 l = 0; l != m; ++l) {
                                    getExcs.push_back(
                                        reader.getMethodExceptionTypeName(k, l).
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
                                        reader.getMethodExceptionTypeName(k, l).
                                        replace('/', '.'));
                                }
                                break;
                            }
                        default:
                            throw FileFormatException(
                                key.getRegistryName(),
                                ("legacy format: method and attribute with same"
                                 " name " + attrName
                                 + " in interface type with key "
                                 + sub.getName()));
                        }
                    }
                }
                RTFieldAccess flags = reader.getFieldFlags(j);
                attrs.push_back(
                    InterfaceTypeEntity::Attribute(
                        attrName, reader.getFieldTypeName(j).replace('/', '.'),
                        (flags & RT_ACCESS_BOUND) != 0,
                        (flags & RT_ACCESS_READONLY) != 0, getExcs, setExcs,
                        translateAnnotations(reader.getFieldDocumentation(j))));
            }
            std::vector< InterfaceTypeEntity::Method > meths;
            for (sal_uInt16 j = 0; j != methodCount; ++j) {
                RTMethodMode flags = reader.getMethodFlags(j);
                if (flags != RT_MODE_ATTRIBUTE_GET
                    && flags != RT_MODE_ATTRIBUTE_SET)
                {
                    std::vector< InterfaceTypeEntity::Method::Parameter >
                        params;
                    sal_uInt16 m = reader.getMethodParameterCount(j);
                    for (sal_uInt16 k = 0; k != m; ++k) {
                        RTParamMode mode = reader.getMethodParameterFlags(j, k);
                        InterfaceTypeEntity::Method::Parameter::Direction dir;
                        switch (mode) {
                        case RT_PARAM_IN:
                            dir = InterfaceTypeEntity::Method::Parameter::DIRECTION_IN;
                            break;
                        case RT_PARAM_OUT:
                            dir = InterfaceTypeEntity::Method::Parameter::DIRECTION_OUT;
                            break;
                        case RT_PARAM_INOUT:
                            dir = InterfaceTypeEntity::Method::Parameter::DIRECTION_IN_OUT;
                            break;
                        default:
                            throw FileFormatException(
                                key.getRegistryName(),
                                ("legacy format: unexpected mode "
                                 + OUString::number(mode) + " of parameter "
                                 + reader.getMethodParameterName(j, k)
                                 + " of method " + reader.getMethodName(j)
                                 + " in interface type with key "
                                 + sub.getName()));
                        }
                        params.push_back(
                            InterfaceTypeEntity::Method::Parameter(
                                reader.getMethodParameterName(j, k),
                                (reader.getMethodParameterTypeName(j, k).
                                 replace('/', '.')),
                                dir));
                    }
                    std::vector< OUString > excs;
                    m = reader.getMethodExceptionCount(j);
                    for (sal_uInt16 k = 0; k != m; ++k) {
                        excs.push_back(
                            reader.getMethodExceptionTypeName(j, k).replace(
                                '/', '.'));
                    }
                    meths.push_back(
                        InterfaceTypeEntity::Method(
                            reader.getMethodName(j),
                            reader.getMethodReturnTypeName(j).replace('/', '.'),
                            params, excs,
                            translateAnnotations(
                                reader.getMethodDocumentation(j))));
                }
            }
            return new InterfaceTypeEntity(
                reader.isPublished(), mandBases, optBases, attrs, meths,
                translateAnnotations(reader.getDocumentation()));
        }
    case RT_TYPE_MODULE:
        return new Module(manager, ucr, sub);
    case RT_TYPE_STRUCT:
        {
            sal_uInt32 n = reader.getReferenceCount();
            if (n == 0) {
                OUString base;
                switch (reader.getSuperTypeCount()) {
                case 0:
                    break;
                case 1:
                    base = reader.getSuperTypeName(0).replace('/', '.');
                    break;
                default:
                    FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unexpected number "
                         + OUString::number(reader.getSuperTypeCount())
                         + " of super-types of plain struct type with key "
                         + sub.getName()));
                }
                std::vector< PlainStructTypeEntity::Member > mems;
                n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    mems.push_back(
                        PlainStructTypeEntity::Member(
                            reader.getFieldName(j),
                            reader.getFieldTypeName(j).replace('/', '.'),
                            translateAnnotations(
                                reader.getFieldDocumentation(j))));
                }
                return new PlainStructTypeEntity(
                    reader.isPublished(), base, mems,
                    translateAnnotations(reader.getDocumentation()));
            } else {
                if (reader.getSuperTypeCount() != 0) {
                    FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unexpected number "
                         + OUString::number(reader.getSuperTypeCount())
                         + " of super-types of polymorphic struct type template"
                         " with key " + sub.getName()));
                }
                std::vector< OUString > params;
                for (sal_uInt16 j = 0; j != n; ++j) {
                    params.push_back(
                        reader.getReferenceTypeName(j).replace('/', '.'));
                }
                std::vector< PolymorphicStructTypeTemplateEntity::Member > mems;
                n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    mems.push_back(
                        PolymorphicStructTypeTemplateEntity::Member(
                            reader.getFieldName(j),
                            reader.getFieldTypeName(j).replace('/', '.'),
                            ((reader.getFieldFlags(j)
                              & RT_ACCESS_PARAMETERIZED_TYPE)
                             != 0),
                            translateAnnotations(
                                reader.getFieldDocumentation(j))));
                }
                return new PolymorphicStructTypeTemplateEntity(
                    reader.isPublished(), params, mems,
                    translateAnnotations(reader.getDocumentation()));
            }
        }
    case RT_TYPE_ENUM:
        {
            std::vector< EnumTypeEntity::Member > mems;
            sal_uInt16 n = reader.getFieldCount();
            for (sal_uInt16 j = 0; j != n; ++j) {
                RTConstValue v(reader.getFieldValue(j));
                if (v.m_type != RT_TYPE_INT32) {
                    FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unexpected type "
                         + OUString::number(v.m_type) + " of value of field "
                         + reader.getFieldName(j) + " of enum type with key "
                         + sub.getName()));
                }
                mems.push_back(
                    EnumTypeEntity::Member(
                        reader.getFieldName(j), v.m_value.aLong,
                        translateAnnotations(reader.getFieldDocumentation(j))));

            }
            return new EnumTypeEntity(
                reader.isPublished(), mems,
                translateAnnotations(reader.getDocumentation()));
        }
    case RT_TYPE_EXCEPTION:
        {
            OUString base;
            switch (reader.getSuperTypeCount()) {
            case 0:
                break;
            case 1:
                base = reader.getSuperTypeName(0).replace('/', '.');
                break;
            default:
                throw FileFormatException(
                    key.getRegistryName(),
                    ("legacy format: unexpected number "
                     + OUString::number(reader.getSuperTypeCount())
                     + " of super-types of exception type with key "
                     + sub.getName()));
            }
            std::vector< ExceptionTypeEntity::Member > mems;
            sal_uInt16 n = reader.getFieldCount();
            for (sal_uInt16 j = 0; j != n; ++j) {
                mems.push_back(
                    ExceptionTypeEntity::Member(
                        reader.getFieldName(j),
                        reader.getFieldTypeName(j).replace('/', '.'),
                        translateAnnotations(reader.getFieldDocumentation(j))));
            }
            return new ExceptionTypeEntity(
                reader.isPublished(), base, mems,
                translateAnnotations(reader.getDocumentation()));
        }
    case RT_TYPE_TYPEDEF:
        if (reader.getSuperTypeCount() != 1) {
            throw FileFormatException(
                key.getRegistryName(),
                ("legacy format: unexpected number "
                 + OUString::number(reader.getSuperTypeCount())
                 + " of super-types of typedef with key " + sub.getName()));
        }
        return new TypedefEntity(
            reader.isPublished(), reader.getSuperTypeName(0).replace('/', '.'),
            translateAnnotations(reader.getDocumentation()));
    case RT_TYPE_SERVICE:
        switch (reader.getSuperTypeCount()) {
        case 0:
            {
                std::vector< AnnotatedReference > mandServs;
                std::vector< AnnotatedReference > optServs;
                std::vector< AnnotatedReference > mandIfcs;
                std::vector< AnnotatedReference > optIfcs;
                sal_uInt16 n = reader.getReferenceCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    AnnotatedReference base(
                        reader.getReferenceTypeName(j).replace('/', '.'),
                        translateAnnotations(
                            reader.getReferenceDocumentation(j)));
                    switch (reader.getReferenceSort(j)) {
                    case RT_REF_EXPORTS:
                        if ((reader.getReferenceFlags(j) & RT_ACCESS_OPTIONAL)
                            == 0)
                        {
                            mandServs.push_back(base);
                        } else {
                            optServs.push_back(base);
                        }
                        break;
                    case RT_REF_SUPPORTS:
                        if ((reader.getReferenceFlags(j) & RT_ACCESS_OPTIONAL)
                            == 0)
                        {
                            mandIfcs.push_back(base);
                        } else {
                            optIfcs.push_back(base);
                        }
                        break;
                    default:
                        throw FileFormatException(
                            key.getRegistryName(),
                            ("legacy format: unexpected mode "
                             + OUString::number(reader.getReferenceSort(j))
                             + " of reference " + reader.getReferenceTypeName(j)
                             + " in service with key " + sub.getName()));
                    }
                }
                std::vector< AccumulationBasedServiceEntity::Property > props;
                n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    RTFieldAccess acc = reader.getFieldFlags(j);
                    int attrs = 0;
                    if ((acc & RT_ACCESS_READONLY) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_READ_ONLY;
                    }
                    if ((acc & RT_ACCESS_OPTIONAL) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_OPTIONAL;
                    }
                    if ((acc & RT_ACCESS_MAYBEVOID) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_MAYBE_VOID;
                    }
                    if ((acc & RT_ACCESS_BOUND) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_BOUND;
                    }
                    if ((acc & RT_ACCESS_CONSTRAINED) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_CONSTRAINED;
                    }
                    if ((acc & RT_ACCESS_TRANSIENT) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_TRANSIENT;
                    }
                    if ((acc & RT_ACCESS_MAYBEAMBIGUOUS) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_MAYBE_AMBIGUOUS;
                    }
                    if ((acc & RT_ACCESS_MAYBEDEFAULT) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_MAYBE_DEFAULT;
                    }
                    if ((acc & RT_ACCESS_REMOVABLE) != 0) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_REMOVABLE;
                    }
                    props.push_back(
                        AccumulationBasedServiceEntity::Property(
                            reader.getFieldName(j),
                            reader.getFieldTypeName(j).replace('/', '.'),
                            static_cast<
                                AccumulationBasedServiceEntity::Property::
                                Attributes >(attrs),
                            translateAnnotations(
                                reader.getFieldDocumentation(j))));
                }
                return new AccumulationBasedServiceEntity(
                    reader.isPublished(), mandServs, optServs, mandIfcs,
                    optIfcs, props,
                    translateAnnotations(reader.getDocumentation()));
            }
        case 1:
            {
                std::vector< SingleInterfaceBasedServiceEntity::Constructor >
                    ctors;
                sal_uInt16 n = reader.getMethodCount();
                if (n == 1 && reader.getMethodFlags(0) == RT_MODE_TWOWAY
                    && reader.getMethodName(0).isEmpty()
                    && reader.getMethodReturnTypeName(0) == "void"
                    && reader.getMethodParameterCount(0) == 0
                    && reader.getMethodExceptionCount(0) == 0)
                {
                    ctors.push_back(
                        SingleInterfaceBasedServiceEntity::Constructor());
                } else {
                    for (sal_uInt16 j = 0; j != n; ++j) {
                        if (reader.getMethodFlags(j) != RT_MODE_TWOWAY) {
                            throw FileFormatException(
                                key.getRegistryName(),
                                ("legacy format: unexpected mode "
                                 + OUString::number(reader.getMethodFlags(j))
                                 + " of constructor " + reader.getMethodName(j)
                                 + " in service with key " + sub.getName()));
                        }
                        std::vector<
                            SingleInterfaceBasedServiceEntity::Constructor::
                            Parameter > params;
                        sal_uInt16 m = reader.getMethodParameterCount(j);
                        for (sal_uInt16 k = 0; k != m; ++k) {
                            RTParamMode mode
                                = reader.getMethodParameterFlags(j, k);
                            if ((mode & ~RT_PARAM_REST) != RT_PARAM_IN) {
                                throw FileFormatException(
                                    key.getRegistryName(),
                                    ("legacy format: unexpected mode "
                                     + OUString::number(mode)
                                     + " of parameter "
                                     + reader.getMethodParameterName(j, k)
                                     + " of constructor "
                                     + reader.getMethodName(j)
                                     + " in service with key "
                                     + sub.getName()));
                            }
                            if ((mode & RT_PARAM_REST) != 0
                                && !(m == 1
                                     && ((reader.getMethodParameterTypeName(
                                              j, 0))
                                         == "any")))
                            {
                                throw FileFormatException(
                                    key.getRegistryName(),
                                    ("legacy format: bad rest parameter "
                                     + reader.getMethodParameterName(j, k)
                                     + " of constructor "
                                     + reader.getMethodName(j)
                                     + " in service with key "
                                     + sub.getName()));
                            }
                            params.push_back(
                                SingleInterfaceBasedServiceEntity::Constructor::
                                Parameter(
                                    reader.getMethodParameterName(j, k),
                                    (reader.getMethodParameterTypeName(j, k).
                                     replace('/', '.')),
                                    (mode & RT_PARAM_REST) != 0));
                        }
                        std::vector< OUString > excs;
                        m = reader.getMethodExceptionCount(j);
                        for (sal_uInt16 k = 0; k != m; ++k) {
                            excs.push_back(
                                reader.getMethodExceptionTypeName(j, k).replace(
                                    '/', '.'));
                        }
                        ctors.push_back(
                            SingleInterfaceBasedServiceEntity::Constructor(
                                reader.getMethodName(j), params, excs,
                                translateAnnotations(
                                    reader.getMethodDocumentation(j))));
                    }
                }
                return new SingleInterfaceBasedServiceEntity(
                    reader.isPublished(),
                    reader.getSuperTypeName(0).replace('/', '.'), ctors,
                    translateAnnotations(reader.getDocumentation()));
            }
        default:
            throw FileFormatException(
                key.getRegistryName(),
                ("legacy format: unexpected number "
                 + OUString::number(reader.getSuperTypeCount())
                 + " of super-types of service with key " + sub.getName()));
        }
    case RT_TYPE_SINGLETON:
        {
            if (reader.getSuperTypeCount() != 1) {
                throw FileFormatException(
                    key.getRegistryName(),
                    ("legacy format: unexpected number "
                     + OUString::number(reader.getSuperTypeCount())
                     + " of super-types of singleton with key "
                     + sub.getName()));
            }
            OUString basePath(reader.getSuperTypeName(0));
            OUString baseName(basePath.replace('/', '.'));
            bool newStyle;
            rtl::Reference< Entity > base(manager->findEntity(baseName));
            if (base.is()) {
                switch (base->getSort()) {
                case Entity::SORT_INTERFACE_TYPE:
                    newStyle = true;
                    break;
                case Entity::SORT_ACCUMULATION_BASED_SERVICE:
                    newStyle = false;
                    break;
                default:
                    throw FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unexpected sort "
                         + OUString::number(base->getSort()) + " of base "
                         + baseName + " of singleton with key "
                         + sub.getName()));
                }
            } else {
                RegistryKey key2;
                e = ucr.openKey(basePath, key2);
                switch (e) {
                case REG_NO_ERROR:
                    break;
                case REG_KEY_NOT_EXISTS:
                    throw FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unknown super-type " + basePath
                         + " of super-type with key " + sub.getName()));
                default:
                    throw FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: cannot open ucr sub-key " + basePath
                         + ": " + OUString::number(e)));
                }
                std::vector< char > buf2;
                typereg::Reader reader2(getReader(key2, &buf2));
                switch (reader2.getTypeClass()) {
                case RT_TYPE_INTERFACE:
                    newStyle = true;
                    break;
                case RT_TYPE_SERVICE:
                    newStyle = false;
                    break;
                default:
                    throw FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unexpected type class "
                         + OUString::number(reader2.getTypeClass())
                         + " of super-type with key " + key2.getName()
                         + " of singleton with key " + sub.getName()));
                }
            }
            return newStyle
                ? rtl::Reference< Entity >(
                    new InterfaceBasedSingletonEntity(
                        reader.isPublished(), baseName,
                        translateAnnotations(reader.getDocumentation())))
                : rtl::Reference< Entity >(
                    new ServiceBasedSingletonEntity(
                        reader.isPublished(), baseName,
                        translateAnnotations(reader.getDocumentation())));
        }
    case RT_TYPE_CONSTANTS:
        {
            std::vector< ConstantGroupEntity::Member > mems;
            sal_uInt16 n = reader.getFieldCount();
            for (sal_uInt16 j = 0; j != n; ++j) {
                mems.push_back(
                    ConstantGroupEntity::Member(
                        reader.getFieldName(j),
                        translateConstantValue(sub, reader.getFieldValue(j)),
                        translateAnnotations(reader.getFieldDocumentation(j))));
            }
            return new ConstantGroupEntity(
                reader.isPublished(), mems,
                translateAnnotations(reader.getDocumentation()));
        }
    default:
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: unexpected type class "
             + OUString::number(reader.getTypeClass()) + " of key "
             + sub.getName()));
    }
}

}

LegacyProvider::LegacyProvider(
    rtl::Reference< Manager > const & manager, OUString const & uri):
    manager_(manager)
{
    Registry reg;
    RegError e = reg.open(uri, REG_READONLY);
    switch (e) {
    case REG_NO_ERROR:
        break;
    case REG_REGISTRY_NOT_EXISTS:
        throw NoSuchFileException(uri);
    default:
        throw FileFormatException(
            uri, "cannot open legacy file: " + OUString::number(e));
    }
    RegistryKey root;
    e = reg.openRootKey(root);
    if (e != REG_NO_ERROR) {
        throw FileFormatException(
            uri, "legacy format: cannot open root key: " + OUString::number(e));
    }
    e = root.openKey("UCR", ucr_);
    switch (e) {
    case REG_NO_ERROR:
    case REG_KEY_NOT_EXISTS: // such effectively empty files exist in the wild
        break;
    default:
        throw FileFormatException(
            uri, "legacy format: cannot open UCR key: " + OUString::number(e));
    }
}

rtl::Reference< MapCursor > LegacyProvider::createRootCursor() const {
    return new Cursor(manager_, ucr_, ucr_);
}

rtl::Reference< Entity > LegacyProvider::findEntity(OUString const & name)
    const
{
    return ucr_.isValid()
        ? readEntity(manager_, ucr_, ucr_, name.replace('.', '/'), true)
        : rtl::Reference< Entity >();
}

LegacyProvider::~LegacyProvider() throw () {}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

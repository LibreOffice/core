/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <vector>

#include <registry/reader.hxx>
#include <registry/registry.hxx>
#include <registry/regtype.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unoidl/unoidl.hxx>

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
        return ConstantValue(value.m_value.aBool);
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
    virtual ~Cursor() throw () override {}

    virtual rtl::Reference< Entity > getNext(OUString * name) override;

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
        if (e != RegError::NO_ERROR) {
            throw FileFormatException(
                key_.getRegistryName(),
                ("legacy format: cannot get sub-key names of " + key_.getName()
                 + ": " + OUString::number(static_cast<int>(e))));
        }
    }
}

rtl::Reference< Entity > Cursor::getNext(OUString * name) {
    assert(name != nullptr);
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
    virtual ~Module() throw () override {}

    virtual std::vector< OUString > getMemberNames() const override;

    virtual rtl::Reference< MapCursor > createCursor() const override
    { return new Cursor(manager_, ucr_, key_); }

    rtl::Reference< Manager > manager_;
    RegistryKey const ucr_;
    mutable RegistryKey key_;
};

std::vector< OUString > Module::getMemberNames() const {
    RegistryKeyNames names;
    RegError e = key_.getKeyNames("", names);
    if (e != RegError::NO_ERROR) {
        throw FileFormatException(
            key_.getRegistryName(),
            ("legacy format: cannot get sub-key names of " + key_.getName()
             + ": " + OUString::number(static_cast<int>(e))));
    }
    std::vector< OUString > ns;
    for (sal_uInt32 i = 0; i != names.getLength(); ++i) {
        ns.push_back(names.getElement(i));
    }
    return ns;
}

typereg::Reader getReader(RegistryKey & key, std::vector< char > * buffer) {
    assert(buffer != nullptr);
    RegValueType type;
    sal_uInt32 size;
    RegError e = key.getValueInfo("", &type, &size);
    if (e != RegError::NO_ERROR) {
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: cannot get value info about key " + key.getName()
             + ": " + OUString::number(static_cast<int>(e))));
    }
    if (type != RegValueType::BINARY) {
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: unexpected value type " + OUString::number(static_cast<int>(type))
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
    if (e != RegError::NO_ERROR) {
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: cannot get binary value of key " + key.getName()
             + ": " + OUString::number(static_cast<int>(e))));
    }
    typereg::Reader reader(&(*buffer)[0], size);
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
    case RegError::NO_ERROR:
        break;
    case RegError::KEY_NOT_EXISTS:
        if (probe) {
            return rtl::Reference< Entity >();
        }
        [[fallthrough]];
    default:
        throw FileFormatException(
            key.getRegistryName(),
            ("legacy format: cannot open sub-key " + path + " of "
             + key.getName() + ": " + OUString::number(static_cast<int>(e))));
    }
    std::vector< char > buf;
    typereg::Reader reader(getReader(sub, &buf));
    switch (reader.getTypeClass()) {
    case RT_TYPE_INTERFACE:
        {
            std::vector< AnnotatedReference > mandBases;
            sal_uInt16 n = reader.getSuperTypeCount();
            for (sal_uInt16 j = 0; j != n; ++j) {
                mandBases.emplace_back(
                    reader.getSuperTypeName(j).replace('/', '.'),
                    std::vector< OUString >());
            }
            std::vector< AnnotatedReference > optBases;
            n = reader.getReferenceCount();
            for (sal_uInt16 j = 0; j != n; ++j) {
                optBases.emplace_back(
                    reader.getReferenceTypeName(j).replace('/', '.'),
                    translateAnnotations(reader.getReferenceDocumentation(j)));
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
                        case RTMethodMode::ATTRIBUTE_GET:
                            {
                                sal_uInt16 m
                                    = reader.getMethodExceptionCount(k);
                                // cid#1213376 unhelpfully warns about an
                                // untrusted loop bound here:
                                // coverity[tainted_data] - trusted data source
                                for (sal_uInt16 l = 0; l != m; ++l) {
                                    getExcs.push_back(
                                        reader.getMethodExceptionTypeName(k, l).
                                        replace('/', '.'));
                                }
                                break;
                            }
                        case RTMethodMode::ATTRIBUTE_SET:
                            {
                                sal_uInt16 m
                                    = reader.getMethodExceptionCount(k);
                                // cid#1213376 unhelpfully warns about an
                                // untrusted loop bound here:
                                // coverity[tainted_data] - trusted data source
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
                attrs.emplace_back(
                    attrName, reader.getFieldTypeName(j).replace('/', '.'),
                    bool(flags & RTFieldAccess::BOUND),
                    bool(flags & RTFieldAccess::READONLY), getExcs, setExcs,
                    translateAnnotations(reader.getFieldDocumentation(j)));
            }
            std::vector< InterfaceTypeEntity::Method > meths;
            for (sal_uInt16 j = 0; j != methodCount; ++j) {
                RTMethodMode flags = reader.getMethodFlags(j);
                if (flags != RTMethodMode::ATTRIBUTE_GET
                    && flags != RTMethodMode::ATTRIBUTE_SET)
                {
                    std::vector< InterfaceTypeEntity::Method::Parameter >
                        params;
                    sal_uInt16 m = reader.getMethodParameterCount(j);
                    // cid#1213376 unhelpfully warns about an untrusted loop
                    // bound here:
                    // coverity[tainted_data] - trusted data source
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
                        params.emplace_back(
                            reader.getMethodParameterName(j, k),
                            (reader.getMethodParameterTypeName(j, k).
                             replace('/', '.')),
                            dir);
                    }
                    std::vector< OUString > excs;
                    m = reader.getMethodExceptionCount(j);
                    // cid#1213376 unhelpfully warns about an untrusted loop
                    // bound here:
                    // coverity[tainted_data] - trusted data source
                    for (sal_uInt16 k = 0; k != m; ++k) {
                        excs.push_back(
                            reader.getMethodExceptionTypeName(j, k).replace(
                                '/', '.'));
                    }
                    meths.emplace_back(
                        reader.getMethodName(j),
                        reader.getMethodReturnTypeName(j).replace('/', '.'),
                        params, excs,
                        translateAnnotations(
                            reader.getMethodDocumentation(j)));
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
            sal_uInt16 n = reader.getReferenceCount();
            if (n == 0) {
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
                         + " of super-types of plain struct type with key "
                         + sub.getName()));
                }
                std::vector< PlainStructTypeEntity::Member > mems;
                n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    mems.emplace_back(
                        reader.getFieldName(j),
                        reader.getFieldTypeName(j).replace('/', '.'),
                        translateAnnotations(reader.getFieldDocumentation(j)));
                }
                return new PlainStructTypeEntity(
                    reader.isPublished(), base, mems,
                    translateAnnotations(reader.getDocumentation()));
            } else {
                if (reader.getSuperTypeCount() != 0) {
                    throw FileFormatException(
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
                    mems.emplace_back(
                        reader.getFieldName(j),
                        reader.getFieldTypeName(j).replace('/', '.'),
                        bool(
                            reader.getFieldFlags(j)
                            & RTFieldAccess::PARAMETERIZED_TYPE),
                        translateAnnotations(reader.getFieldDocumentation(j)));
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
                    throw FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unexpected type "
                         + OUString::number(v.m_type) + " of value of field "
                         + reader.getFieldName(j) + " of enum type with key "
                         + sub.getName()));
                }
                mems.emplace_back(
                    reader.getFieldName(j), v.m_value.aLong,
                    translateAnnotations(reader.getFieldDocumentation(j)));

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
                mems.emplace_back(
                    reader.getFieldName(j),
                    reader.getFieldTypeName(j).replace('/', '.'),
                    translateAnnotations(reader.getFieldDocumentation(j)));
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
                    AnnotatedReference base{
                        reader.getReferenceTypeName(j).replace('/', '.'),
                        translateAnnotations(
                            reader.getReferenceDocumentation(j))};
                    switch (reader.getReferenceSort(j)) {
                    case RTReferenceType::EXPORTS:
                        if (!(reader.getReferenceFlags(j) & RTFieldAccess::OPTIONAL))
                        {
                            mandServs.push_back(base);
                        } else {
                            optServs.push_back(base);
                        }
                        break;
                    case RTReferenceType::SUPPORTS:
                        if (!(reader.getReferenceFlags(j) & RTFieldAccess::OPTIONAL))
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
                             + OUString::number(static_cast<int>(reader.getReferenceSort(j)))
                             + " of reference " + reader.getReferenceTypeName(j)
                             + " in service with key " + sub.getName()));
                    }
                }
                std::vector< AccumulationBasedServiceEntity::Property > props;
                n = reader.getFieldCount();
                for (sal_uInt16 j = 0; j != n; ++j) {
                    RTFieldAccess acc = reader.getFieldFlags(j);
                    int attrs = 0;
                    if (acc & RTFieldAccess::READONLY) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_READ_ONLY;
                    }
                    if (acc & RTFieldAccess::OPTIONAL) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_OPTIONAL;
                    }
                    if (acc & RTFieldAccess::MAYBEVOID) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_MAYBE_VOID;
                    }
                    if (acc & RTFieldAccess::BOUND) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_BOUND;
                    }
                    if (acc & RTFieldAccess::CONSTRAINED) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_CONSTRAINED;
                    }
                    if (acc & RTFieldAccess::TRANSIENT) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_TRANSIENT;
                    }
                    if (acc & RTFieldAccess::MAYBEAMBIGUOUS) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_MAYBE_AMBIGUOUS;
                    }
                    if (acc & RTFieldAccess::MAYBEDEFAULT) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_MAYBE_DEFAULT;
                    }
                    if (acc & RTFieldAccess::REMOVABLE) {
                        attrs |= AccumulationBasedServiceEntity::Property::
                            ATTRIBUTE_REMOVABLE;
                    }
                    props.emplace_back(
                        reader.getFieldName(j),
                        reader.getFieldTypeName(j).replace('/', '.'),
                        static_cast<
                            AccumulationBasedServiceEntity::Property::
                                Attributes >(attrs),
                        translateAnnotations(reader.getFieldDocumentation(j)));
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
                if (n == 1 && reader.getMethodFlags(0) == RTMethodMode::TWOWAY
                    && reader.getMethodName(0).isEmpty()
                    && reader.getMethodReturnTypeName(0) == "void"
                    && reader.getMethodParameterCount(0) == 0
                    && reader.getMethodExceptionCount(0) == 0)
                {
                    ctors.push_back(
                        SingleInterfaceBasedServiceEntity::Constructor());
                } else {
                    for (sal_uInt16 j = 0; j != n; ++j) {
                        if (reader.getMethodFlags(j) != RTMethodMode::TWOWAY) {
                            throw FileFormatException(
                                key.getRegistryName(),
                                ("legacy format: unexpected mode "
                                 + OUString::number(static_cast<int>(reader.getMethodFlags(j)))
                                 + " of constructor " + reader.getMethodName(j)
                                 + " in service with key " + sub.getName()));
                        }
                        std::vector<
                            SingleInterfaceBasedServiceEntity::Constructor::
                            Parameter > params;
                        sal_uInt16 m = reader.getMethodParameterCount(j);
                        // cid#1213376 unhelpfully warns about an untrusted
                        // loop bound here:
                        // coverity[tainted_data] - trusted data source
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
                            params.emplace_back(
                                reader.getMethodParameterName(j, k),
                                (reader.getMethodParameterTypeName(j, k).
                                 replace('/', '.')),
                                (mode & RT_PARAM_REST) != 0);
                        }
                        std::vector< OUString > excs;
                        m = reader.getMethodExceptionCount(j);
                        // cid#1213376 unhelpfully warns about an untrusted
                        // loop bound here:
                        // coverity[tainted_data] - trusted data source
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
                case RegError::NO_ERROR:
                    break;
                case RegError::KEY_NOT_EXISTS:
                    throw FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: unknown super-type " + basePath
                         + " of super-type with key " + sub.getName()));
                default:
                    throw FileFormatException(
                        key.getRegistryName(),
                        ("legacy format: cannot open ucr sub-key " + basePath
                         + ": " + OUString::number(static_cast<int>(e))));
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
                mems.emplace_back(
                    reader.getFieldName(j),
                    translateConstantValue(sub, reader.getFieldValue(j)),
                    translateAnnotations(reader.getFieldDocumentation(j)));
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

LegacyProvider::LegacyProvider(Manager & manager, OUString const & uri):
    manager_(manager)
{
    Registry reg;
    RegError e = reg.open(uri, RegAccessMode::READONLY);
    switch (e) {
    case RegError::NO_ERROR:
        break;
    case RegError::REGISTRY_NOT_EXISTS:
        throw NoSuchFileException(uri);
    default:
        throw FileFormatException(
            uri, "cannot open legacy file: " + OUString::number(static_cast<int>(e)));
    }
    RegistryKey root;
    e = reg.openRootKey(root);
    if (e != RegError::NO_ERROR) {
        throw FileFormatException(
            uri, "legacy format: cannot open root key: " + OUString::number(static_cast<int>(e)));
    }
    e = root.openKey("UCR", ucr_);
    switch (e) {
    case RegError::NO_ERROR:
    case RegError::KEY_NOT_EXISTS: // such effectively empty files exist in the wild
        break;
    default:
        throw FileFormatException(
            uri, "legacy format: cannot open UCR key: " + OUString::number(static_cast<int>(e)));
    }
}

rtl::Reference< MapCursor > LegacyProvider::createRootCursor() const {
    return new Cursor(&manager_, ucr_, ucr_);
}

rtl::Reference< Entity > LegacyProvider::findEntity(OUString const & name)
    const
{
    return ucr_.isValid()
        ? readEntity(&manager_, ucr_, ucr_, name.replace('.', '/'), true)
        : rtl::Reference< Entity >();
}

LegacyProvider::~LegacyProvider() throw () {}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <codemaker/commoncpp.hxx>
#include <codemaker/global.hxx>
#include <codemaker/typemanager.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/process.h>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/textcvt.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/main.h>
#include <sal/types.h>
#include <unoidl/unoidl.hxx>

namespace
{
void badUsage()
{
    std::cerr
        << "Usage:\n\n"
           "  embindmaker <name> <cpp-output> <hpp-output> <js-output> <registries>\n\n"
           "where each <registry> is '+' (primary) or ':' (secondary), followed by: either a\n"
           "new- or legacy-format .rdb file, a single .idl file, or a root directory of an\n"
           ".idl file tree.  For all primary registries, Embind code is written to\n"
           "<cpp-output>/<hpp-output> and corresponding JavaScript scaffolding code is\n"
           "written to <js-output>.  The <name> is used as part of some of the identifiers\n"
           "in those generated files.\n";
    std::exit(EXIT_FAILURE);
}

std::string getPathnameArgument(sal_uInt32 argument)
{
    OUString arg;
    rtl_getAppCommandArg(argument, &arg.pData);
    OString path;
    auto const enc = osl_getThreadTextEncoding();
    if (!arg.convertToString(&path, enc,
                             RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                                 | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
    {
        std::cerr << "Cannot convert \"" << arg << "\" to system encoding " << enc << "\n";
        std::exit(EXIT_FAILURE);
    }
    return std::string(path);
}

std::pair<OUString, bool> parseRegistryArgument(sal_uInt32 argument)
{
    OUString arg;
    rtl_getAppCommandArg(argument, &arg.pData);
    bool primary;
    if (arg.startsWith(u"+", &arg))
    {
        primary = true;
    }
    else if (arg.startsWith(u":", &arg))
    {
        primary = false;
    }
    else
    {
        std::cerr << "Bad registry argument \"" << arg << "\"\n";
        std::exit(EXIT_FAILURE);
    }
    OUString url;
    auto const e1 = osl::FileBase::getFileURLFromSystemPath(arg, url);
    if (e1 != osl::FileBase::E_None)
    {
        std::cerr << "Cannot convert \"" << arg << "\" to file URL, error code " << +e1 << "\n";
        std::exit(EXIT_FAILURE);
    }
    OUString cwd;
    auto const e2 = osl_getProcessWorkingDir(&cwd.pData);
    if (e2 != osl_Process_E_None)
    {
        std::cerr << "Cannot obtain working directory, error code " << +e2 << "\n";
        std::exit(EXIT_FAILURE);
    }
    OUString abs;
    auto const e3 = osl::FileBase::getAbsoluteFileURL(cwd, url, abs);
    if (e3 != osl::FileBase::E_None)
    {
        std::cerr << "Cannot make \"" << url << "\" into an absolute file URL, error code " << +e3
                  << "\n";
        std::exit(EXIT_FAILURE);
    }
    return { abs, primary };
}

struct Module
{
    std::map<OUString, std::shared_ptr<Module>> modules;
    std::vector<std::pair<OUString, OUString>> mappings;
};

OUString
getServiceConstructorName(unoidl::SingleInterfaceBasedServiceEntity::Constructor const& constructor)
{
    return constructor.defaultConstructor ? u"create"_ustr : constructor.name;
}

OUString jsName(OUString const& name) { return name.replace('.', '$'); }

OUString
jsServiceConstructor(OUString const& service,
                     unoidl::SingleInterfaceBasedServiceEntity::Constructor const& constructor)
{
    return "uno_Function_" + jsName(service) + "$$" + getServiceConstructorName(constructor);
}

OUString jsSingleton(OUString const& singleton) { return "uno_Function_" + jsName(singleton); }

void scan(rtl::Reference<unoidl::MapCursor> const& cursor, std::u16string_view prefix,
          Module* module, std::vector<OUString>& enums, std::vector<OUString>& structs,
          std::vector<OUString>& exceptions, std::vector<OUString>& interfaces,
          std::vector<OUString>& services, std::vector<OUString>& singletons)
{
    assert(cursor.is());
    assert(module != nullptr);
    for (;;)
    {
        OUString id;
        auto const ent = cursor->getNext(&id);
        if (!ent.is())
        {
            break;
        }
        OUString name(prefix + id);
        switch (ent->getSort())
        {
            case unoidl::Entity::SORT_MODULE:
            {
                auto& sub = module->modules[id];
                if (!sub)
                {
                    sub = std::make_shared<Module>();
                }
                scan(static_cast<unoidl::ModuleEntity*>(ent.get())->createCursor(),
                     Concat2View(name + "."), sub.get(), enums, structs, exceptions, interfaces,
                     services, singletons);
                break;
            }
            case unoidl::Entity::SORT_ENUM_TYPE:
                module->mappings.emplace_back(id, "instance.uno_Type_" + jsName(name));
                enums.emplace_back(name);
                break;
            case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
                module->mappings.emplace_back(id, "instance.uno_Type_" + jsName(name));
                structs.emplace_back(name);
                break;
            case unoidl::Entity::SORT_EXCEPTION_TYPE:
                module->mappings.emplace_back(id, "instance.uno_Type_" + jsName(name));
                exceptions.emplace_back(name);
                break;
            case unoidl::Entity::SORT_INTERFACE_TYPE:
                module->mappings.emplace_back(id, "instance.uno_Type_" + jsName(name));
                interfaces.emplace_back(name);
                break;
            case unoidl::Entity::SORT_CONSTANT_GROUP:
            {
                auto const& members
                    = static_cast<unoidl::ConstantGroupEntity*>(ent.get())->getMembers();
                if (!members.empty())
                {
                    auto sub = std::make_shared<Module>();
                    for (auto const& member : members)
                    {
                        OUString value;
                        switch (member.value.type)
                        {
                            case unoidl::ConstantValue::TYPE_BOOLEAN:
                                value = member.value.booleanValue ? u"true"_ustr : u"false"_ustr;
                                break;
                            case unoidl::ConstantValue::TYPE_BYTE:
                                value = OUString::number(member.value.byteValue);
                                break;
                            case unoidl::ConstantValue::TYPE_SHORT:
                                value = OUString::number(member.value.shortValue);
                                break;
                            case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                                value = OUString::number(member.value.unsignedShortValue);
                                break;
                            case unoidl::ConstantValue::TYPE_LONG:
                                value = OUString::number(member.value.longValue);
                                break;
                            case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                                value = OUString::number(member.value.unsignedLongValue);
                                break;
                            case unoidl::ConstantValue::TYPE_HYPER:
                                value = OUString::number(member.value.hyperValue) + "n";
                                break;
                            case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                                value = OUString::number(member.value.unsignedHyperValue) + "n";
                                break;
                            case unoidl::ConstantValue::TYPE_FLOAT:
                                value = OUString::number(member.value.floatValue);
                                break;
                            case unoidl::ConstantValue::TYPE_DOUBLE:
                                value = OUString::number(member.value.doubleValue);
                                break;
                        }
                        sub->mappings.emplace_back(member.name, value);
                    }
                    module->modules[id] = sub;
                }
                break;
            }
            case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
            {
                auto const& ctors
                    = static_cast<unoidl::SingleInterfaceBasedServiceEntity*>(ent.get())
                          ->getConstructors();
                if (!ctors.empty())
                {
                    auto sub = std::make_shared<Module>();
                    for (auto const& ctor : ctors)
                    {
                        sub->mappings.emplace_back(getServiceConstructorName(ctor),
                                                   "instance." + jsServiceConstructor(name, ctor));
                    }
                    module->modules[id] = sub;
                    services.emplace_back(name);
                }
            }
            break;
            case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
                module->mappings.emplace_back(id, "instance." + jsSingleton(name));
                singletons.emplace_back(name);
                break;
            default:
                break;
        }
    }
}

OUString cppName(OUString const& name) { return "::" + name.replaceAll(u".", u"::"); }

OUString resolveOuterTypedefs(rtl::Reference<TypeManager> const& manager, OUString const& name)
{
    for (OUString n(name);;)
    {
        rtl::Reference<unoidl::Entity> ent;
        if (manager->getSort(n, &ent) != codemaker::UnoType::Sort::Typedef)
        {
            return n;
        }
        n = dynamic_cast<unoidl::TypedefEntity&>(*ent).getType();
    }
}

OUString resolveAllTypedefs(rtl::Reference<TypeManager> const& manager, std::u16string_view name)
{
    sal_Int32 k1;
    OUString n(b2u(codemaker::UnoType::decompose(u2b(name), &k1)));
    for (;;)
    {
        rtl::Reference<unoidl::Entity> ent;
        if (manager->getSort(n, &ent) != codemaker::UnoType::Sort::Typedef)
        {
            break;
        }
        sal_Int32 k2;
        n = b2u(codemaker::UnoType::decompose(
            u2b(static_cast<unoidl::TypedefEntity*>(ent.get())->getType()), &k2));
        k1 += k2; //TODO: overflow
    }
    OUStringBuffer b;
    for (sal_Int32 i = 0; i != k1; ++i)
    {
        b.append("[]");
    }
    b.append(n);
    return b.makeStringAndClear();
}

bool passByReference(rtl::Reference<TypeManager> const& manager, OUString const& name)
{
    switch (manager->getSort(resolveOuterTypedefs(manager, name)))
    {
        case codemaker::UnoType::Sort::Boolean:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
        case codemaker::UnoType::Sort::Float:
        case codemaker::UnoType::Sort::Double:
        case codemaker::UnoType::Sort::Char:
        case codemaker::UnoType::Sort::Enum:
            return false;
        case codemaker::UnoType::Sort::String:
        case codemaker::UnoType::Sort::Type:
        case codemaker::UnoType::Sort::Any:
        case codemaker::UnoType::Sort::Sequence:
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
        case codemaker::UnoType::Sort::Interface:
            return true;
        default:
            throw CannotDumpException("unexpected entity \"" + name
                                      + "\" in call to passByReference");
    }
}

void dumpType(std::ostream& out, rtl::Reference<TypeManager> const& manager,
              std::u16string_view name)
{
    sal_Int32 k;
    std::vector<OString> args;
    OUString n(
        b2u(codemaker::UnoType::decompose(u2b(resolveAllTypedefs(manager, name)), &k, &args)));
    for (sal_Int32 i = 0; i != k; ++i)
    {
        out << "::com::sun::star::uno::Sequence<";
    }
    switch (manager->getSort(n))
    {
        case codemaker::UnoType::Sort::Void:
            out << "void";
            break;
        case codemaker::UnoType::Sort::Boolean:
            out << "::sal_Bool";
            break;
        case codemaker::UnoType::Sort::Byte:
            out << "::sal_Int8";
            break;
        case codemaker::UnoType::Sort::Short:
            out << "::sal_Int16";
            break;
        case codemaker::UnoType::Sort::UnsignedShort:
            out << "::sal_uInt16";
            break;
        case codemaker::UnoType::Sort::Long:
            out << "::sal_Int32";
            break;
        case codemaker::UnoType::Sort::UnsignedLong:
            out << "::sal_uInt32";
            break;
        case codemaker::UnoType::Sort::Hyper:
            out << "::sal_Int64";
            break;
        case codemaker::UnoType::Sort::UnsignedHyper:
            out << "::sal_uInt64";
            break;
        case codemaker::UnoType::Sort::Float:
            out << "float";
            break;
        case codemaker::UnoType::Sort::Double:
            out << "double";
            break;
        case codemaker::UnoType::Sort::Char:
            out << "::sal_Unicode";
            break;
        case codemaker::UnoType::Sort::String:
            out << "::rtl::OUString";
            break;
        case codemaker::UnoType::Sort::Type:
            out << "::com::sun::star::uno::Type";
            break;
        case codemaker::UnoType::Sort::Any:
            out << "::com::sun::star::uno::Any";
            break;
        case codemaker::UnoType::Sort::Enum:
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::Exception:
            out << cppName(n);
            break;
        case codemaker::UnoType::Sort::PolymorphicStructTemplate:
            out << cppName(n);
            if (!args.empty())
            {
                out << "<";
                bool first = true;
                for (auto const& arg : args)
                {
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        out << ", ";
                    }
                    dumpType(out, manager, b2u(arg));
                }
                out << ">";
            }
            break;
        case codemaker::UnoType::Sort::Interface:
            out << "::com::sun::star::uno::Reference<";
            out << cppName(n);
            out << ">";
            break;
        default:
            throw CannotDumpException(OUString::Concat("unexpected entity \"") + name
                                      + "\" in call to dumpType");
    }
    for (sal_Int32 i = 0; i != k; ++i)
    {
        out << ">";
    }
}

void dumpStructMembers(std::ostream& out, rtl::Reference<TypeManager> const& manager,
                       OUString const& name, rtl::Reference<unoidl::PlainStructTypeEntity> struc)
{
    auto const& base = struc->getDirectBase();
    if (!base.isEmpty())
    {
        auto const ent = manager->getManager()->findEntity(base);
        if (!ent.is() || ent->getSort() != unoidl::Entity::SORT_PLAIN_STRUCT_TYPE)
        {
            throw CannotDumpException("bad struct base \"" + base + "\"");
        }
        dumpStructMembers(out, manager, name,
                          static_cast<unoidl::PlainStructTypeEntity*>(ent.get()));
    }
    for (auto const& mem : struc->getDirectMembers())
    {
        out << "\n        .field(\"" << mem.name << "\", &" << cppName(name) << "::" << mem.name
            << ")";
    }
}

void dumpExceptionMembers(std::ostream& out, rtl::Reference<TypeManager> const& manager,
                          OUString const& name,
                          rtl::Reference<unoidl::ExceptionTypeEntity> exception)
{
    auto const& base = exception->getDirectBase();
    if (!base.isEmpty())
    {
        auto const ent = manager->getManager()->findEntity(base);
        if (!ent.is() || ent->getSort() != unoidl::Entity::SORT_EXCEPTION_TYPE)
        {
            throw CannotDumpException("bad exception base \"" + base + "\"");
        }
        dumpExceptionMembers(out, manager, name,
                             static_cast<unoidl::ExceptionTypeEntity*>(ent.get()));
    }
    for (auto const& mem : exception->getDirectMembers())
    {
        out << "\n        .field(\"" << mem.name << "\", &" << cppName(name) << "::" << mem.name
            << ")";
    }
}

void dumpAttributes(std::ostream& out, rtl::Reference<TypeManager> const& manager,
                    OUString const& name, rtl::Reference<unoidl::InterfaceTypeEntity> const& entity,
                    std::list<OUString> const& baseTrail)
{
    for (auto const& attr : entity->getDirectAttributes())
    {
        out << "        .function(\"get" << attr.name << "\", ";
        if (baseTrail.empty())
        {
            out << "&" << cppName(name) << "::get" << attr.name;
        }
        else
        {
            out << "+[](::com::sun::star::uno::Reference<" << cppName(name)
                << "> const & the_self) { return ";
            for (auto const& base : baseTrail)
            {
                out << "static_cast<" << cppName(base) << " *>(";
            }
            out << "the_self.get()";
            for (std::size_t i = 0; i != baseTrail.size(); ++i)
            {
                out << ")";
            }
            out << "->get" << attr.name << "(); }";
        }
        out << ", ::emscripten::pure_virtual())\n";
        if (!attr.readOnly)
        {
            out << "        .function(\"set" << attr.name << "\", ";
            if (baseTrail.empty())
            {
                out << "&" << cppName(name) << "::set" << attr.name;
            }
            else
            {
                out << "+[](::com::sun::star::uno::Reference<" << cppName(name)
                    << "> const & the_self, ";
                dumpType(out, manager, attr.type);
                if (passByReference(manager, attr.type))
                {
                    out << " const &";
                }
                out << " the_value) { ";
                for (auto const& base : baseTrail)
                {
                    out << "static_cast<" << cppName(base) << " *>(";
                }
                out << "the_self.get()";
                for (std::size_t i = 0; i != baseTrail.size(); ++i)
                {
                    out << ")";
                }
                out << "->set" << attr.name << "(the_value); }";
            }
            out << ", ::emscripten::pure_virtual())\n";
        }
    }
}

bool hasInOutParameters(unoidl::InterfaceTypeEntity::Method const& method)
{
    return std::any_of(method.parameters.begin(), method.parameters.end(),
                       [](auto const& parameter) {
                           return parameter.direction
                                  != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN;
                       });
}

void dumpParameters(std::ostream& out, rtl::Reference<TypeManager> const& manager,
                    unoidl::InterfaceTypeEntity::Method const& method, bool declarations)
{
    bool first = true;
    for (auto const& param : method.parameters)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ", ";
        }
        bool wrap = false;
        if (param.direction != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
        {
            switch (manager->getSort(resolveOuterTypedefs(manager, param.type)))
            {
                case codemaker::UnoType::Sort::Boolean:
                case codemaker::UnoType::Sort::Byte:
                case codemaker::UnoType::Sort::Short:
                case codemaker::UnoType::Sort::UnsignedShort:
                case codemaker::UnoType::Sort::Long:
                case codemaker::UnoType::Sort::UnsignedLong:
                case codemaker::UnoType::Sort::Hyper:
                case codemaker::UnoType::Sort::UnsignedHyper:
                case codemaker::UnoType::Sort::Float:
                case codemaker::UnoType::Sort::Double:
                case codemaker::UnoType::Sort::Char:
                case codemaker::UnoType::Sort::Enum:
                    wrap = true;
                    break;
                case codemaker::UnoType::Sort::String:
                case codemaker::UnoType::Sort::Type:
                case codemaker::UnoType::Sort::Any:
                case codemaker::UnoType::Sort::Sequence:
                case codemaker::UnoType::Sort::PlainStruct:
                case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
                case codemaker::UnoType::Sort::Interface:
                    break;
                default:
                    throw CannotDumpException("unexpected entity \"" + param.type
                                              + "\" as parameter type");
            }
        }
        if (declarations)
        {
            if (wrap)
            {
                out << "::unoembindhelpers::UnoInOutParam<";
            }
            dumpType(out, manager, param.type);
            if (wrap)
            {
                out << ">";
            }
            if (param.direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
            {
                if (passByReference(manager, param.type))
                {
                    out << " const &";
                }
            }
            else
            {
                out << " *";
            }
            out << " ";
        }
        else if (param.direction != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN
                 && !wrap)
        {
            out << "*";
        }
        out << param.name;
        if (!declarations && wrap)
        {
            out << "->value";
        }
    }
}

void dumpWrapper(std::ostream& out, rtl::Reference<TypeManager> const& manager,
                 OUString const& interfaceName, unoidl::InterfaceTypeEntity::Method const& method,
                 std::list<OUString> const& baseTrail)
{
    out << "        .function(\"" << method.name << "\", +[](::com::sun::star::uno::Reference<"
        << cppName(interfaceName);
    out << "> const & the_self";
    if (!method.parameters.empty())
    {
        out << ", ";
    }
    dumpParameters(out, manager, method, true);
    out << ") { return ";
    for (auto const& base : baseTrail)
    {
        out << "static_cast<" << cppName(base) << " *>(";
    }
    out << "the_self";
    if (!baseTrail.empty())
    {
        out << ".get()";
    }
    for (std::size_t i = 0; i != baseTrail.size(); ++i)
    {
        out << ")";
    }
    out << "->" << method.name << "(";
    dumpParameters(out, manager, method, false);
    out << "); }";
    if (hasInOutParameters(method))
    {
        out << ", ::emscripten::allow_raw_pointers()";
    }
    out << ", ::emscripten::pure_virtual())\n";
}

void dumpMethods(std::ostream& out, rtl::Reference<TypeManager> const& manager,
                 OUString const& name, rtl::Reference<unoidl::InterfaceTypeEntity> const& entity,
                 std::list<OUString> const& baseTrail)
{
    for (auto const& meth : entity->getDirectMethods())
    {
        if (!baseTrail.empty() || hasInOutParameters(meth))
        {
            dumpWrapper(out, manager, name, meth, baseTrail);
        }
        else
        {
            out << "        .function(\"" << meth.name << "\", &" << cppName(name)
                << "::" << meth.name << ", ::emscripten::pure_virtual())\n";
        }
    }
}

rtl::Reference<unoidl::InterfaceTypeEntity>
resolveInterface(rtl::Reference<TypeManager> const& manager, OUString const& name)
{
    auto const ent = manager->getManager()->findEntity(name);
    if (!ent.is() || ent->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
    {
        throw CannotDumpException("bad interface \"" + name + "\"");
    }
    return static_cast<unoidl::InterfaceTypeEntity*>(ent.get());
}

void recordVisitedBases(rtl::Reference<TypeManager> const& manager, OUString const& name,
                        std::set<OUString>& visitedBases)
{
    auto const ent = resolveInterface(manager, name);
    for (auto const& base : ent->getDirectMandatoryBases())
    {
        if (visitedBases.insert(base.name).second)
        {
            recordVisitedBases(manager, base.name, visitedBases);
        }
    }
}

void dumpBase(std::ostream& out, rtl::Reference<TypeManager> const& manager,
              OUString const& interface, OUString const& name, std::set<OUString>& visitedBases,
              std::list<OUString> const& baseTrail)
{
    auto const ent = resolveInterface(manager, name);
    for (auto const& base : ent->getDirectMandatoryBases())
    {
        if (visitedBases.insert(base.name).second)
        {
            auto trail = baseTrail;
            trail.push_front(base.name);
            dumpBase(out, manager, interface, base.name, visitedBases, trail);
        }
    }
    dumpAttributes(out, manager, interface, ent, baseTrail);
    dumpMethods(out, manager, interface, ent, baseTrail);
}

void dumpWrapperClassMembers(std::ostream& out, rtl::Reference<TypeManager> const& manager,
                             OUString const& interface, OUString const& name,
                             std::set<OUString>& visitedBases)
{
    auto const ent = resolveInterface(manager, name);
    for (auto const& base : ent->getDirectMandatoryBases())
    {
        if (visitedBases.insert(base.name).second)
        {
            dumpWrapperClassMembers(out, manager, interface, base.name, visitedBases);
        }
    }
    for (auto const& attr : ent->getDirectAttributes())
    {
        out << "    ";
        dumpType(out, manager, attr.type);
        out << " get" << attr.name << "() override { return call<";
        dumpType(out, manager, attr.type);
        out << ">(\"get" << attr.name << "\"); }\n";
        if (!attr.readOnly)
        {
            out << "    void set" << attr.name << "(";
            dumpType(out, manager, attr.type);
            switch (manager->getSort(resolveOuterTypedefs(manager, attr.type)))
            {
                case codemaker::UnoType::Sort::Boolean:
                case codemaker::UnoType::Sort::Byte:
                case codemaker::UnoType::Sort::Short:
                case codemaker::UnoType::Sort::UnsignedShort:
                case codemaker::UnoType::Sort::Long:
                case codemaker::UnoType::Sort::UnsignedLong:
                case codemaker::UnoType::Sort::Hyper:
                case codemaker::UnoType::Sort::UnsignedHyper:
                case codemaker::UnoType::Sort::Float:
                case codemaker::UnoType::Sort::Double:
                case codemaker::UnoType::Sort::Char:
                case codemaker::UnoType::Sort::Enum:
                    break;
                case codemaker::UnoType::Sort::String:
                case codemaker::UnoType::Sort::Type:
                case codemaker::UnoType::Sort::Any:
                case codemaker::UnoType::Sort::Sequence:
                case codemaker::UnoType::Sort::PlainStruct:
                case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
                case codemaker::UnoType::Sort::Interface:
                    out << " const &";
                    break;
                default:
                    throw CannotDumpException("unexpected entity \"" + attr.type
                                              + "\" as attribute type");
            }
            out << " the_value) override { return call<void>(\"set" << attr.name
                << "\", the_value); }\n";
        }
    }
    for (auto const& meth : ent->getDirectMethods())
    {
        out << "    ";
        dumpType(out, manager, meth.returnType);
        out << " " << meth.name << "(";
        bool first = true;
        for (auto const& param : meth.parameters)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ", ";
            }
            dumpType(out, manager, param.type);
            if (param.direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
            {
                switch (manager->getSort(resolveOuterTypedefs(manager, param.type)))
                {
                    case codemaker::UnoType::Sort::Boolean:
                    case codemaker::UnoType::Sort::Byte:
                    case codemaker::UnoType::Sort::Short:
                    case codemaker::UnoType::Sort::UnsignedShort:
                    case codemaker::UnoType::Sort::Long:
                    case codemaker::UnoType::Sort::UnsignedLong:
                    case codemaker::UnoType::Sort::Hyper:
                    case codemaker::UnoType::Sort::UnsignedHyper:
                    case codemaker::UnoType::Sort::Float:
                    case codemaker::UnoType::Sort::Double:
                    case codemaker::UnoType::Sort::Char:
                    case codemaker::UnoType::Sort::Enum:
                        break;
                    case codemaker::UnoType::Sort::String:
                    case codemaker::UnoType::Sort::Type:
                    case codemaker::UnoType::Sort::Any:
                    case codemaker::UnoType::Sort::Sequence:
                    case codemaker::UnoType::Sort::PlainStruct:
                    case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
                    case codemaker::UnoType::Sort::Interface:
                        out << " const &";
                        break;
                    default:
                        throw CannotDumpException("unexpected entity \"" + param.type
                                                  + "\" as parameter type");
                }
            }
            else
            {
                out << " &";
            }
            out << " " << param.name;
        }
        out << ") override { return call<";
        dumpType(out, manager, meth.returnType);
        out << ">(\"" << meth.name << "\"";
        for (auto const& param : meth.parameters)
        {
            out << ", " << param.name;
        }
        out << "); }\n";
    }
}

void dumpRegisterFunctionProlog(std::ostream& out, unsigned long long& counter)
{
    out << "static void __attribute__((noinline)) register" << counter << "() {\n";
}

void dumpRegisterFunctionEpilog(std::ostream& out, unsigned long long& counter)
{
    out << "}\n";
    ++counter;
    if (counter == 0)
    {
        std::cerr << "Emitting too many register functions\n";
        std::exit(EXIT_FAILURE);
    }
}

void recordSequenceTypes(rtl::Reference<TypeManager> const& manager, OUString const& type,
                         std::set<OUString>& sequences)
{
    auto const res = resolveAllTypedefs(manager, type);
    if (manager->getSort(res) == codemaker::UnoType::Sort::Sequence)
    {
        sequences.insert(res);
    }
}

void writeJsMap(std::ostream& out, Module const& module, std::string const& prefix)
{
    auto comma = false;
    for (auto const & [ id, to ] : module.mappings)
    {
        if (comma)
        {
            out << ",\n";
        }
        out << prefix << "'" << id << "': " << to;
        comma = true;
    }
    for (auto const & [ id, sub ] : module.modules)
    {
        if (comma)
        {
            out << ",\n";
        }
        out << prefix << "'" << id << "': {\n";
        writeJsMap(out, *sub, prefix + "    ");
        out << prefix << "}";
        comma = true;
    }
    if (comma)
    {
        out << "\n";
    }
}
}

SAL_IMPLEMENT_MAIN()
{
    try
    {
        auto const args = rtl_getAppCommandArgCount();
        if (args < 4)
        {
            badUsage();
        }
        OUString name;
        rtl_getAppCommandArg(0, &name.pData);
        auto const cppPathname = getPathnameArgument(1);
        auto const hppPathname = getPathnameArgument(2);
        auto const jsPathname = getPathnameArgument(3);
        rtl::Reference<TypeManager> mgr(new TypeManager);
        for (sal_uInt32 i = 4; i != args; ++i)
        {
            auto const & [ uri, primary ] = parseRegistryArgument(i);
            try
            {
                mgr->loadProvider(uri, primary);
            }
            catch (unoidl::NoSuchFileException&)
            {
                std::cerr << "Input <" << uri << "> does not exist\n";
                std::exit(EXIT_FAILURE);
            }
        }
        auto const module = std::make_shared<Module>();
        std::vector<OUString> enums;
        std::vector<OUString> structs;
        std::vector<OUString> exceptions;
        std::vector<OUString> interfaces;
        std::vector<OUString> services;
        std::vector<OUString> singletons;
        for (auto const& prov : mgr->getPrimaryProviders())
        {
            scan(prov->createRootCursor(), u"", module.get(), enums, structs, exceptions,
                 interfaces, services, singletons);
        }
        std::ofstream cppOut(cppPathname, std::ios_base::out | std::ios_base::trunc);
        if (!cppOut)
        {
            std::cerr << "Cannot open \"" << cppPathname << "\" for writing\n";
            std::exit(EXIT_FAILURE);
        }
        cppOut << "#include <emscripten/bind.h>\n"
                  "#include <com/sun/star/uno/Any.hxx>\n"
                  "#include <com/sun/star/uno/Reference.hxx>\n"
                  "#include <static/unoembindhelpers/PrimaryBindings.hxx>\n";
        for (auto const& enm : enums)
        {
            cppOut << "#include <" << enm.replace('.', '/') << ".hpp>\n";
        }
        for (auto const& str : structs)
        {
            cppOut << "#include <" << str.replace('.', '/') << ".hpp>\n";
        }
        for (auto const& exc : exceptions)
        {
            cppOut << "#include <" << exc.replace('.', '/') << ".hpp>\n";
        }
        for (auto const& ifc : interfaces)
        {
            cppOut << "#include <" << ifc.replace('.', '/') << ".hpp>\n";
        }
        for (auto const& srv : services)
        {
            cppOut << "#include <" << srv.replace('.', '/') << ".hpp>\n";
        }
        for (auto const& sng : singletons)
        {
            cppOut << "#include <" << sng.replace('.', '/') << ".hpp>\n";
        }
        cppOut << "\n"
                  "// TODO: This is a temporary workaround that likely causes the Embind UNO\n"
                  "// bindings to leak memory. Reference counting and cloning mechanisms of\n"
                  "// Embind should be investigated to figure out what exactly we need here:\n"
                  "namespace emscripten::internal {\n";
        for (auto const& ifc : interfaces)
        {
            cppOut << "    template<> void raw_destructor<" << cppName(ifc) << ">(" << cppName(ifc)
                   << " *) {}\n";
        }
        cppOut << "}\n\n";
        unsigned long long n = 0;
        for (auto const& enm : enums)
        {
            auto const ent = mgr->getManager()->findEntity(enm);
            assert(ent.is());
            assert(ent->getSort() == unoidl::Entity::SORT_ENUM_TYPE);
            rtl::Reference const enmEnt(static_cast<unoidl::EnumTypeEntity*>(ent.get()));
            dumpRegisterFunctionProlog(cppOut, n);
            cppOut << "    ::emscripten::enum_<" << cppName(enm) << ">(\"uno_Type_" << jsName(enm)
                   << "\")";
            for (auto const& mem : enmEnt->getMembers())
            {
                cppOut << "\n        .value(\"" << mem.name << "\", " << cppName(enm) << "_"
                       << mem.name << ")";
            }
            cppOut << ";\n";
            cppOut << "    ::unoembindhelpers::registerUnoType<" << cppName(enm) << ">();\n";
            dumpRegisterFunctionEpilog(cppOut, n);
        }
        std::set<OUString> sequences;
        for (auto const& str : structs)
        {
            auto const ent = mgr->getManager()->findEntity(str);
            assert(ent.is());
            assert(ent->getSort() == unoidl::Entity::SORT_PLAIN_STRUCT_TYPE);
            rtl::Reference const strEnt(static_cast<unoidl::PlainStructTypeEntity*>(ent.get()));
            dumpRegisterFunctionProlog(cppOut, n);
            cppOut << "    ::emscripten::value_object<" << cppName(str) << ">(\"uno_Type_"
                   << jsName(str) << "\")";
            dumpStructMembers(cppOut, mgr, str, strEnt);
            cppOut << ";\n";
            cppOut << "    ::unoembindhelpers::registerUnoType<" << cppName(str) << ">();\n";
            dumpRegisterFunctionEpilog(cppOut, n);
            for (auto const& mem : strEnt->getDirectMembers())
            {
                recordSequenceTypes(mgr, mem.type, sequences);
            }
        }
        for (auto const& exc : exceptions)
        {
            auto const ent = mgr->getManager()->findEntity(exc);
            assert(ent.is());
            assert(ent->getSort() == unoidl::Entity::SORT_EXCEPTION_TYPE);
            rtl::Reference const excEnt(static_cast<unoidl::ExceptionTypeEntity*>(ent.get()));
            dumpRegisterFunctionProlog(cppOut, n);
            cppOut << "    ::emscripten::value_object<" << cppName(exc) << ">(\"uno_Type_"
                   << jsName(exc) << "\")";
            dumpExceptionMembers(cppOut, mgr, exc, excEnt);
            cppOut << ";\n";
            cppOut << "    ::unoembindhelpers::registerUnoType<" << cppName(exc) << ">();\n";
            dumpRegisterFunctionEpilog(cppOut, n);
            for (auto const& mem : excEnt->getDirectMembers())
            {
                recordSequenceTypes(mgr, mem.type, sequences);
            }
        }
        for (auto const& ifc : interfaces)
        {
            auto const ent = mgr->getManager()->findEntity(ifc);
            assert(ent.is());
            assert(ent->getSort() == unoidl::Entity::SORT_INTERFACE_TYPE);
            rtl::Reference const ifcEnt(static_cast<unoidl::InterfaceTypeEntity*>(ent.get()));
            {
                auto i = ifc.lastIndexOf('.');
                auto j = i + 1;
                if (i == -1)
                {
                    i = 0;
                }
                cppOut << "namespace the_wrappers" << cppName(ifc.copy(0, i)) << " {\n"
                       << "struct " << ifc.copy(j) << " final: public ::emscripten::wrapper<"
                       << cppName(ifc) << "> {\n"
                       << "    EMSCRIPTEN_WRAPPER(" << ifc.copy(j) << ");\n";
                std::set<OUString> visitedBases;
                dumpWrapperClassMembers(cppOut, mgr, ifc, ifc, visitedBases);
                cppOut << "};\n}\n";
            }
            dumpRegisterFunctionProlog(cppOut, n);
            cppOut << "    ::emscripten::class_<" << cppName(ifc);
            //TODO: Embind only supports single inheritance, so use that support at least for a UNO
            // interface's first base, and explicitly spell out the attributes and methods of any
            // remaining bases:
            auto const& bases = ifcEnt->getDirectMandatoryBases();
            if (bases.size() != 0)
            {
                cppOut << ", ::emscripten::base<" << cppName(bases[0].name) << ">";
            }
            cppOut << ">(\"uno_Type_" << jsName(ifc)
                   << "\")\n"
                      "        .allow_subclass<the_wrappers"
                   << cppName(ifc) << ">(\"uno_Wrapper_" << jsName(ifc)
                   << "\")\n"
                      "        .smart_ptr<::com::sun::star::uno::Reference<"
                   << cppName(ifc) << ">>(\"uno_Reference_" << jsName(ifc)
                   << "\")\n"
                      "        .class_function(\"reference\", +[]("
                   << cppName(ifc)
                   << " * the_interface) { return ::com::sun::star::uno::Reference(the_interface); "
                      "}, ::emscripten::allow_raw_pointers())\n"
                      "        "
                      ".constructor(+[](::com::sun::star::uno::Reference<::com::sun::star::uno::"
                      "XInterface> const & the_object) { return ::com::sun::star::uno::Reference<"
                   << cppName(ifc)
                   << ">(the_object, ::com::sun::star::uno::UNO_QUERY); })\n"
                      "        .constructor(+[](::com::sun::star::uno::Any const & the_object, "
                      "[[maybe_unused]] ::unoembindhelpers::uno_Reference) { return "
                      "::com::sun::star::uno::Reference<"
                   << cppName(ifc)
                   << ">(the_object, ::com::sun::star::uno::UNO_QUERY); })\n"
                      "        .function(\"$is\", +[](::com::sun::star::uno::Reference<"
                   << cppName(ifc)
                   << "> const & the_self) { return the_self.is(); })\n"
                      "        .function(\"$query\", +[](::com::sun::star::uno::Reference<"
                   << cppName(ifc)
                   << "> const & the_self) { return "
                      "::com::sun::star::uno::Reference<::com::sun::star::uno::XInterface>(the_"
                      "self); })\n"
                      "        .function(\"$equals\", +[](::com::sun::star::uno::Reference<"
                   << cppName(ifc)
                   << "> const & the_self, "
                      "::com::sun::star::uno::Reference<::com::sun::star::uno::XInterface> const & "
                      "the_other) { return the_self == the_other; })\n";
            if (bases.size() > 1)
            {
                std::set<OUString> visitedBases;
                recordVisitedBases(mgr, bases[0].name, visitedBases);
                for (std::size_t i = 1; i != bases.size(); ++i)
                {
                    dumpBase(cppOut, mgr, ifc, bases[i].name, visitedBases, { bases[i].name });
                }
            }
            dumpAttributes(cppOut, mgr, ifc, ifcEnt, {});
            dumpMethods(cppOut, mgr, ifc, ifcEnt, {});
            cppOut << "        ;\n"
                      "    ::unoembindhelpers::registerUnoType<::com::sun::star::uno::Reference<"
                   << cppName(ifc) << ">>();\n";
            dumpRegisterFunctionEpilog(cppOut, n);
            for (auto const& attr : ifcEnt->getDirectAttributes())
            {
                recordSequenceTypes(mgr, attr.type, sequences);
            }
            for (auto const& meth : ifcEnt->getDirectMethods())
            {
                for (auto const& param : meth.parameters)
                {
                    recordSequenceTypes(mgr, param.type, sequences);
                }
                recordSequenceTypes(mgr, meth.returnType, sequences);
            }
        }
        for (auto const& srv : services)
        {
            auto const ent = mgr->getManager()->findEntity(srv);
            assert(ent.is());
            assert(ent->getSort() == unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE);
            rtl::Reference const srvEnt(
                static_cast<unoidl::SingleInterfaceBasedServiceEntity*>(ent.get()));
            dumpRegisterFunctionProlog(cppOut, n);
            for (auto const& ctor : srvEnt->getConstructors())
            {
                cppOut << "    ::emscripten::function(\"" << jsServiceConstructor(srv, ctor)
                       << "\", &" << cppName(srv) << "::" << getServiceConstructorName(ctor)
                       << ");\n";
            }
            dumpRegisterFunctionEpilog(cppOut, n);
        }
        for (auto const& sng : singletons)
        {
            dumpRegisterFunctionProlog(cppOut, n);
            cppOut << "    ::emscripten::function(\"" << jsSingleton(sng) << "\", &" << cppName(sng)
                   << "::get);\n";
            dumpRegisterFunctionEpilog(cppOut, n);
        }
        cppOut << "void init_unoembind_" << name << "() {\n";
        for (unsigned long long i = 0; i != n; ++i)
        {
            cppOut << "    register" << i << "();\n";
        }
        for (auto const& seq : sequences)
        {
            cppOut << "    ::unoembindhelpers::registerSequence<";
            assert(seq.startsWith("[]"));
            dumpType(cppOut, mgr, seq.copy(2));
            cppOut << ">(\"uno_Sequence";
            sal_Int32 k;
            auto const nuc = b2u(codemaker::UnoType::decompose(u2b(seq), &k));
            assert(k >= 1);
            if (k > 1)
            {
                cppOut << k;
            }
            cppOut << "_" << jsName(nuc) << "\");\n";
        }
        cppOut << "}\n";
        cppOut.close();
        if (!cppOut)
        {
            std::cerr << "Failed to write \"" << cppPathname << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        std::ofstream hppOut(hppPathname, std::ios_base::out | std::ios_base::trunc);
        if (!hppOut)
        {
            std::cerr << "Cannot open \"" << hppPathname << "\" for writing\n";
            std::exit(EXIT_FAILURE);
        }
        hppOut << "void init_unoembind_" << name << "();\n";
        hppOut.close();
        if (!hppOut)
        {
            std::cerr << "Failed to write \"" << hppPathname << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        std::ofstream jsOut(jsPathname, std::ios_base::out | std::ios_base::trunc);
        if (!jsOut)
        {
            std::cerr << "Cannot open \"" << jsPathname << "\" for writing\n";
            std::exit(EXIT_FAILURE);
        }
        jsOut << "function init_unoembind_" << name
              << "(instance) {\n"
                 "    return {\n";
        writeJsMap(jsOut, *module, "        ");
        jsOut << "    };\n"
                 "};\n";
        jsOut.close();
        if (!jsOut)
        {
            std::cerr << "Failed to write \"" << jsPathname << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        return EXIT_SUCCESS;
    }
    catch (unoidl::FileFormatException const& e)
    {
        std::cerr << "Bad input <" << e.getUri() << ">: " << e.getDetail() << "\n";
        std::exit(EXIT_FAILURE);
    }
    catch (CannotDumpException const& e)
    {
        std::cerr << "Failure: " << e.getMessage() << "\n";
        std::exit(EXIT_FAILURE);
    }
    catch (std::exception const& e)
    {
        std::cerr << "Failure: " << e.what() << "\n";
        std::exit(EXIT_FAILURE);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

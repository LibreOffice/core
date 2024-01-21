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
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <utility>
#include <vector>

#include <codemaker/global.hxx>
#include <codemaker/typemanager.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/process.h>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
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
           "  embindmaker <prefix> <registries>\n\n"
           "where each <registry> is '+' (primary) or ':' (secondary), followed by: either a\n"
           "new- or legacy-format .rdb file, a single .idl file, or a root directory of an\n"
           ".idl file tree.  Embind code for all primary registries is written to stdout,"
           "with <prefix> used as part of the name passed to EMSCRIPTEN_BINDINGS.\n";
    std::exit(EXIT_FAILURE);
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

void scan(rtl::Reference<unoidl::MapCursor> const& cursor, std::u16string_view prefix,
          std::vector<OUString>& interfaces)
{
    assert(cursor.is());
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
                scan(static_cast<unoidl::ModuleEntity*>(ent.get())->createCursor(),
                     Concat2View(name + "."), interfaces);
                break;
            case unoidl::Entity::SORT_INTERFACE_TYPE:
                interfaces.emplace_back(name);
                break;
            default:
                break;
        }
    }
}

OUString cppName(OUString const& name) { return "::" + name.replaceAll(u".", u"::"); }

OUString jsName(OUString const& name) { return name.replace('.', '$'); }

void dumpAttributes(OUString const& name, rtl::Reference<unoidl::InterfaceTypeEntity> const& entity)
{
    for (auto const& attr : entity->getDirectAttributes())
    {
        std::cout << "        .function(\"get" << attr.name << "\", &" << cppName(name) << "::get"
                  << attr.name << ")\n";
        if (!attr.readOnly)
        {
            std::cout << "        .function(\"set" << attr.name << "\", &" << cppName(name)
                      << "::set" << attr.name << ")\n";
        }
    }
}

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

void dumpType(rtl::Reference<TypeManager> const& manager, std::u16string_view name, bool isConst)
{
    sal_Int32 k;
    std::vector<OString> args;
    OUString n(
        b2u(codemaker::UnoType::decompose(u2b(resolveAllTypedefs(manager, name)), &k, &args)));
    if (isConst)
    {
        std::cout << "const ";
    }
    for (sal_Int32 i = 0; i != k; ++i)
    {
        std::cout << "::css::uno::Sequence<";
    }
    switch (manager->getSort(n))
    {
        case codemaker::UnoType::Sort::Void:
            std::cout << "void";
            break;
        case codemaker::UnoType::Sort::Boolean:
            std::cout << "::sal_Bool";
            break;
        case codemaker::UnoType::Sort::Byte:
            std::cout << "::sal_Int8";
            break;
        case codemaker::UnoType::Sort::Short:
            std::cout << "::sal_Int16";
            break;
        case codemaker::UnoType::Sort::UnsignedShort:
            std::cout << "::sal_uInt16";
            break;
        case codemaker::UnoType::Sort::Long:
            std::cout << "::sal_Int32";
            break;
        case codemaker::UnoType::Sort::UnsignedLong:
            std::cout << "::sal_uInt32";
            break;
        case codemaker::UnoType::Sort::Hyper:
            std::cout << "::sal_Int64";
            break;
        case codemaker::UnoType::Sort::UnsignedHyper:
            std::cout << "::sal_uInt64";
            break;
        case codemaker::UnoType::Sort::Float:
            std::cout << "float";
            break;
        case codemaker::UnoType::Sort::Double:
            std::cout << "double";
            break;
        case codemaker::UnoType::Sort::Char:
            std::cout << "::sal_Unicode";
            break;
        case codemaker::UnoType::Sort::String:
            std::cout << "::rtl::OUString";
            break;
        case codemaker::UnoType::Sort::Type:
            std::cout << "::css::uno::Type";
            break;
        case codemaker::UnoType::Sort::Any:
            std::cout << "::css::uno::Any";
            break;
        case codemaker::UnoType::Sort::Enum:
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::Exception:
            std::cout << cppName(n);
            break;
        case codemaker::UnoType::Sort::PolymorphicStructTemplate:
            std::cout << cppName(n);
            if (!args.empty())
            {
                std::cout << "<";
                bool first = true;
                for (auto const& arg : args)
                {
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        std::cout << ", ";
                    }
                    dumpType(manager, b2u(arg), false);
                }
                std::cout << ">";
            }
            break;
        case codemaker::UnoType::Sort::Interface:
            std::cout << "::css::uno::Reference<";
            std::cout << cppName(n);
            std::cout << ">";
            break;
        default:
            throw CannotDumpException(OUString::Concat("unexpected entity \"") + name
                                      + "\" in call to dumpType");
    }
    for (sal_Int32 i = 0; i != k; ++i)
    {
        std::cout << ">";
    }
}

void dumpParameters(rtl::Reference<TypeManager> const& manager,
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
            std::cout << ", ";
        }
        bool isConst;
        bool isRef;
        if (param.direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
        {
            isConst = passByReference(manager, param.type);
            isRef = isConst;
        }
        else
        {
            isConst = false;
            isRef = true;
        }
        // For the embind wrapper, we define a pointer instead of a reference:
        if (declarations)
        {
            dumpType(manager, param.type, isConst);
            std::cout << " ";
        }
        if (isRef)
        {
            std::cout << "*";
        }
        if (declarations)
        {
            std::cout << " ";
        }
        std::cout << param.name;
    }
}

void dumpWrapper(rtl::Reference<TypeManager> const& manager, OUString const& interfaceName,
                 unoidl::InterfaceTypeEntity::Method const& method, bool forReference)
{
    std::cout << "        .function(\"" << method.name << "\", +[](";
    if (forReference)
    {
        std::cout << "::com::sun::star::uno::Reference<";
    }
    std::cout << cppName(interfaceName);
    if (forReference)
    {
        std::cout << ">";
    }
    std::cout << " * the_self";
    if (!method.parameters.empty())
    {
        std::cout << ", ";
    }
    dumpParameters(manager, method, true);
    std::cout << ") { return the_self->";
    if (forReference)
    {
        std::cout << "get()->";
    }
    std::cout << method.name << "(";
    dumpParameters(manager, method, false);
    std::cout << "); }, ::emscripten::allow_raw_pointers())\n";
}

void dumpMethods(rtl::Reference<TypeManager> const& manager, OUString const& name,
                 rtl::Reference<unoidl::InterfaceTypeEntity> const& entity, bool forReference)
{
    for (auto const& meth : entity->getDirectMethods())
    {
        if (forReference)
        {
            dumpWrapper(manager, name, meth, true);
        }
        else if (std::any_of(
                     meth.parameters.begin(), meth.parameters.end(), [](auto const& parameter) {
                         return parameter.direction
                                != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN;
                     }))
        {
            dumpWrapper(manager, name, meth, false);
        }
        else
        {
            std::cout << "        .function(\"" << meth.name << "\", &" << cppName(name)
                      << "::" << meth.name << ")\n";
        }
    }
}
}

SAL_IMPLEMENT_MAIN()
{
    try
    {
        auto const args = rtl_getAppCommandArgCount();
        if (args == 0)
        {
            badUsage();
        }
        OUString prefix;
        rtl_getAppCommandArg(0, &prefix.pData);
        rtl::Reference<TypeManager> mgr(new TypeManager);
        for (sal_uInt32 i = 1; i != args; ++i)
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
        std::vector<OUString> interfaces;
        for (auto const& prov : mgr->getPrimaryProviders())
        {
            scan(prov->createRootCursor(), u"", interfaces);
        }
        std::cout << "#include <emscripten/bind.h>\n"
                     "#include <com/sun/star/uno/Any.hxx>\n"
                     "#include <com/sun/star/uno/Reference.hxx>\n";
        for (auto const& ifc : interfaces)
        {
            std::cout << "#include <" << ifc.replace('.', '/') << ".hpp>\n";
        }
        std::cout << "\n"
                     "// TODO: This is a temporary workaround that likely causes the Embind UNO\n"
                     "// bindings to leak memory. Reference counting and cloning mechanisms of\n"
                     "// Embind should be investigated to figure out what exactly we need here:\n"
                     "namespace emscripten::internal {\n";
        for (auto const& ifc : interfaces)
        {
            std::cout << "    template<> void raw_destructor<" << cppName(ifc) << ">("
                      << cppName(ifc) << " *) {}\n";
        }
        std::cout << "}\n\n"
                     "EMSCRIPTEN_BINDINGS(uno_bindings_"
                  << prefix << ") {\n";
        for (auto const& ifc : interfaces)
        {
            auto const ent = mgr->getManager()->findEntity(ifc);
            assert(ent.is());
            assert(ent->getSort() == unoidl::Entity::SORT_INTERFACE_TYPE);
            rtl::Reference const ifcEnt(static_cast<unoidl::InterfaceTypeEntity*>(ent.get()));
            std::cout << "    ::emscripten::class_<" << cppName(ifc) << ">(\"" << jsName(ifc)
                      << "\")\n";
            dumpAttributes(ifc, ifcEnt);
            dumpMethods(mgr, ifc, ifcEnt, false);
            std::cout << "        ;\n"
                         "    ::emscripten::class_<::com::sun::star::uno::Reference<"
                      << cppName(ifc)
                      << ">, ::emscripten::base<::com::sun::star::uno::BaseReference>>(\""
                      << jsName(ifc)
                      << "Ref\")\n"
                         "        .constructor<>()\n"
                         "        .constructor<::com::sun::star::uno::BaseReference, "
                         "::com::sun::star::uno::UnoReference_Query>()\n"
                         "        .function(\"is\", &::com::sun::star::uno::Reference<"
                      << cppName(ifc)
                      << ">::is)\n"
                         "        .function(\"get\", &::com::sun::star::uno::Reference<"
                      << cppName(ifc)
                      << ">::get, ::emscripten::allow_raw_pointers())\n"
                         "        .function(\"set\", "
                         "::emscripten::select_overload<bool(::com::sun::star::uno::Any const "
                         "&, "
                         "com::sun::star::uno::UnoReference_Query)>(&::com::sun::star::uno::"
                         "Reference<"
                      << cppName(ifc) << ">::set))\n";
            dumpAttributes(ifc, ifcEnt);
            dumpMethods(mgr, ifc, ifcEnt, true);
            std::cout << "        ;\n";
        }
        std::cout << "}\n";
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

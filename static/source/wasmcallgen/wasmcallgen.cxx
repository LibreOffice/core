/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <codemaker/global.hxx>
#include <codemaker/typemanager.hxx>
#include <codemaker/unotype.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/temporary.hxx>
#include <o3tl/unreachable.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/character.hxx>
#include <rtl/process.h>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/main.h>
#include <sal/types.h>
#include <unoidl/unoidl.hxx>

namespace
{
[[noreturn]] void badUsage()
{
    std::cerr
        << "Usage:\n\n"
           "  wasmcallgen <cpp-output> <asm-output> <exp-output> <registries>\n\n"
           "where each <registry> is '+' (primary) or ':' (secondary), followed by: either a\n"
           "new- or legacy-format .rdb file, a single .idl file, or a root directory of an\n"
           ".idl file tree.  For all primary registries, Wasm UNO bridge code is written to\n"
           "<cpp-output>/<asm-output>, and to-be-exported exception RTTI symbols are written\n"
           "to <exp-output>.\n";
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

enum class StructKind
{
    Empty,
    I32,
    I64,
    F32,
    F64,
    General
};

StructKind getKind(rtl::Reference<TypeManager> const& manager, std::u16string_view type)
{
    std::vector<OUString> args;
    rtl::Reference<unoidl::Entity> ent;
    OUString singleMemberType;
    switch (manager->decompose(type, true, nullptr, nullptr, &args, &ent))
    {
        case codemaker::UnoType::Sort::PlainStruct:
        {
            auto const strct = static_cast<unoidl::PlainStructTypeEntity const*>(ent.get());
            if (strct->getDirectMembers().size() > 1)
            {
                return StructKind::General;
            }
            auto k = StructKind::Empty;
            if (!strct->getDirectBase().isEmpty())
            {
                k = getKind(manager, strct->getDirectBase());
            }
            if (strct->getDirectMembers().empty())
            {
                return k;
            }
            if (k != StructKind::Empty)
            {
                return StructKind::General;
            }
            singleMemberType = strct->getDirectMembers()[0].type;
            break;
        }
        case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
        {
            auto const strct
                = static_cast<unoidl::PolymorphicStructTypeTemplateEntity const*>(ent.get());
            switch (strct->getMembers().size())
            {
                case 0:
                    return StructKind::Empty;
                case 1:
                    if (strct->getMembers()[0].parameterized)
                    {
                        auto const i = std::find(strct->getTypeParameters().begin(),
                                                 strct->getTypeParameters().end(),
                                                 strct->getMembers()[0].type);
                        if (i == strct->getTypeParameters().end())
                        {
                            throw CannotDumpException("bad type parameter \""
                                                      + strct->getMembers()[0].type
                                                      + "\" in call to getKind");
                        }
                        auto const n = i - strct->getTypeParameters().begin();
                        if (o3tl::make_unsigned(n) > args.size())
                        {
                            throw CannotDumpException("bad type parameter \""
                                                      + strct->getMembers()[0].type
                                                      + "\" in call to getKind");
                        }
                        singleMemberType = args[n];
                    }
                    else
                    {
                        singleMemberType = strct->getMembers()[0].type;
                    }
                    break;
                default:
                    return StructKind::General;
            }
            break;
        }
        default:
            throw CannotDumpException(OUString::Concat("unexpected entity \"") + type
                                      + "\" in call to getKind");
    }
    switch (manager->getSort(resolveAllTypedefs(manager, singleMemberType)))
    {
        case codemaker::UnoType::Sort::Boolean:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Char:
        case codemaker::UnoType::Sort::Enum:
            return StructKind::I32;
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
            return StructKind::I64;
        case codemaker::UnoType::Sort::Float:
            return StructKind::F32;
        case codemaker::UnoType::Sort::Double:
            return StructKind::F64;
        default:
            return StructKind::General;
    }
}

void appendCallSignatureReturnType(OStringBuffer& buffer,
                                   rtl::Reference<TypeManager> const& manager, OUString const& type)
{
    switch (manager->getSort(resolveAllTypedefs(manager, type)))
    {
        case codemaker::UnoType::Sort::Void:
            buffer.append('v');
            break;
        case codemaker::UnoType::Sort::Boolean:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Char:
        case codemaker::UnoType::Sort::Enum:
            buffer.append('i');
            break;
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
            buffer.append('j');
            break;
        case codemaker::UnoType::Sort::Float:
            buffer.append('f');
            break;
        case codemaker::UnoType::Sort::Double:
            buffer.append('d');
            break;
        case codemaker::UnoType::Sort::String:
        case codemaker::UnoType::Sort::Type:
        case codemaker::UnoType::Sort::Any:
        case codemaker::UnoType::Sort::Sequence:
        case codemaker::UnoType::Sort::Interface:
            buffer.append("vi");
            break;
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
        {
            switch (getKind(manager, type))
            {
                case StructKind::Empty:
                    break;
                case StructKind::I32:
                    buffer.append('i');
                    break;
                case StructKind::I64:
                    buffer.append('j');
                    break;
                case StructKind::F32:
                    buffer.append('f');
                    break;
                case StructKind::F64:
                    buffer.append('d');
                    break;
                case StructKind::General:
                    buffer.append("vi");
                    break;
            }
            break;
        }
        default:
            throw CannotDumpException("unexpected entity \"" + type
                                      + "\" in call to appendCallSignatureReturnType");
    }
}

void appendCallSignatureParameter(
    OStringBuffer& buffer, rtl::Reference<TypeManager> const& manager,
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction, OUString const& type)
{
    if (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
    {
        switch (manager->getSort(resolveAllTypedefs(manager, type)))
        {
            case codemaker::UnoType::Sort::Boolean:
            case codemaker::UnoType::Sort::Byte:
            case codemaker::UnoType::Sort::Short:
            case codemaker::UnoType::Sort::UnsignedShort:
            case codemaker::UnoType::Sort::Long:
            case codemaker::UnoType::Sort::UnsignedLong:
            case codemaker::UnoType::Sort::Char:
            case codemaker::UnoType::Sort::Enum:
            case codemaker::UnoType::Sort::String:
            case codemaker::UnoType::Sort::Type:
            case codemaker::UnoType::Sort::Any:
            case codemaker::UnoType::Sort::Sequence:
            case codemaker::UnoType::Sort::PlainStruct:
            case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
            case codemaker::UnoType::Sort::Interface:
                buffer.append('i');
                break;
            case codemaker::UnoType::Sort::Hyper:
            case codemaker::UnoType::Sort::UnsignedHyper:
                buffer.append('j');
                break;
            case codemaker::UnoType::Sort::Float:
                buffer.append('f');
                break;
            case codemaker::UnoType::Sort::Double:
                buffer.append('d');
                break;
            default:
                throw CannotDumpException("unexpected entity \"" + type
                                          + "\" in call to appendCallSignatureParameter");
        }
    }
    else
    {
        buffer.append('i');
    }
}

OString computeGetterCallSignature(rtl::Reference<TypeManager> const& manager,
                                   unoidl::InterfaceTypeEntity::Attribute const& attribute)
{
    OStringBuffer buf;
    appendCallSignatureReturnType(buf, manager, attribute.type);
    buf.append('i');
    return buf.makeStringAndClear();
}

OString computeSetterCallSignature(rtl::Reference<TypeManager> const& manager,
                                   unoidl::InterfaceTypeEntity::Attribute const& attribute)
{
    OStringBuffer buf("vi");
    appendCallSignatureParameter(
        buf, manager, unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN, attribute.type);
    return buf.makeStringAndClear();
}

OString computeMethodCallSignature(rtl::Reference<TypeManager> const& manager,
                                   unoidl::InterfaceTypeEntity::Method const& method)
{
    OStringBuffer buf;
    appendCallSignatureReturnType(buf, manager, method.returnType);
    buf.append('i');
    for (auto const& param : method.parameters)
    {
        appendCallSignatureParameter(buf, manager, param.direction, param.type);
    }
    return buf.makeStringAndClear();
}

void appendSlotSignatureOffsets(OStringBuffer& buffer, sal_Int32 functionOffset,
                                sal_Int32 vtableOffset)
{
    buffer.append(OString::number(functionOffset) + "_" + OString::number(vtableOffset));
}

void appendSlotSignatureReturnType(OStringBuffer& buffer,
                                   rtl::Reference<TypeManager> const& manager, OUString const& type)
{
    switch (manager->getSort(resolveAllTypedefs(manager, type)))
    {
        case codemaker::UnoType::Sort::Void:
            buffer.append('v');
            break;
        case codemaker::UnoType::Sort::Boolean:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Char:
        case codemaker::UnoType::Sort::Enum:
            buffer.append('i');
            break;
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
            buffer.append('j');
            break;
        case codemaker::UnoType::Sort::Float:
            buffer.append('f');
            break;
        case codemaker::UnoType::Sort::Double:
            buffer.append('d');
            break;
        case codemaker::UnoType::Sort::String:
        case codemaker::UnoType::Sort::Type:
        case codemaker::UnoType::Sort::Any:
        case codemaker::UnoType::Sort::Sequence:
        case codemaker::UnoType::Sort::Interface:
            buffer.append('I');
            break;
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
        {
            switch (getKind(manager, type))
            {
                case StructKind::Empty:
                    break;
                case StructKind::I32:
                    buffer.append('i');
                    break;
                case StructKind::I64:
                    buffer.append('j');
                    break;
                case StructKind::F32:
                    buffer.append('f');
                    break;
                case StructKind::F64:
                    buffer.append('d');
                    break;
                case StructKind::General:
                    buffer.append('I');
                    break;
            }
            break;
        }
        default:
            throw CannotDumpException("unexpected entity \"" + type
                                      + "\" in call to appendSlotSignatureReturnType");
    }
}

void appendSlotSignatureParameter(
    OStringBuffer& buffer, rtl::Reference<TypeManager> const& manager,
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction, OUString const& type)
{
    if (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
    {
        switch (manager->getSort(resolveAllTypedefs(manager, type)))
        {
            case codemaker::UnoType::Sort::Boolean:
            case codemaker::UnoType::Sort::Byte:
            case codemaker::UnoType::Sort::Short:
            case codemaker::UnoType::Sort::UnsignedShort:
            case codemaker::UnoType::Sort::Long:
            case codemaker::UnoType::Sort::UnsignedLong:
            case codemaker::UnoType::Sort::Char:
            case codemaker::UnoType::Sort::Enum:
            case codemaker::UnoType::Sort::String:
            case codemaker::UnoType::Sort::Type:
            case codemaker::UnoType::Sort::Any:
            case codemaker::UnoType::Sort::Sequence:
            case codemaker::UnoType::Sort::PlainStruct:
            case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
            case codemaker::UnoType::Sort::Interface:
                buffer.append('i');
                break;
            case codemaker::UnoType::Sort::Hyper:
            case codemaker::UnoType::Sort::UnsignedHyper:
                buffer.append('j');
                break;
            case codemaker::UnoType::Sort::Float:
                buffer.append('f');
                break;
            case codemaker::UnoType::Sort::Double:
                buffer.append('d');
                break;
            default:
                throw CannotDumpException("unexpected entity \"" + type
                                          + "\" in call to appendSlotSignatureParameter");
        }
    }
    else
    {
        buffer.append('i');
    }
}

OString computeGetterSlotSignature(rtl::Reference<TypeManager> const& manager,
                                   unoidl::InterfaceTypeEntity::Attribute const& attribute,
                                   sal_Int32 functionOffset, sal_Int32 vtableOffset)
{
    OStringBuffer buf;
    appendSlotSignatureOffsets(buf, functionOffset, vtableOffset);
    appendSlotSignatureReturnType(buf, manager, attribute.type);
    return buf.makeStringAndClear();
}

OString computeSetterSlotSignature(rtl::Reference<TypeManager> const& manager,
                                   unoidl::InterfaceTypeEntity::Attribute const& attribute,
                                   sal_Int32 functionOffset, sal_Int32 vtableOffset)
{
    OStringBuffer buf;
    appendSlotSignatureOffsets(buf, functionOffset, vtableOffset);
    buf.append('v');
    appendSlotSignatureParameter(
        buf, manager, unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN, attribute.type);
    return buf.makeStringAndClear();
}

OString computeMethodSlotSignature(rtl::Reference<TypeManager> const& manager,
                                   unoidl::InterfaceTypeEntity::Method const& method,
                                   sal_Int32 functionOffset, sal_Int32 vtableOffset)
{
    OStringBuffer buf;
    appendSlotSignatureOffsets(buf, functionOffset, vtableOffset);
    appendSlotSignatureReturnType(buf, manager, method.returnType);
    for (auto const& param : method.parameters)
    {
        appendSlotSignatureParameter(buf, manager, param.direction, param.type);
    }
    return buf.makeStringAndClear();
}

void computeSlotSignatures(rtl::Reference<TypeManager> const& manager,
                           unoidl::InterfaceTypeEntity const& interface, sal_Int32& functionOffset,
                           sal_Int32& vtableOffset, std::set<OString>& slotSignatures)
{
    auto const orgVtableOffset = vtableOffset;
    auto firstBase = true;
    for (auto const& base : interface.getDirectMandatoryBases())
    {
        auto const ent = manager->getManager()->findEntity(base.name);
        if (!ent.is() || ent->getSort() != unoidl::Entity::SORT_INTERFACE_TYPE)
        {
            throw CannotDumpException("unexpected base type \"" + base.name
                                      + "\" in call to computeSlotSignatures");
        }
        sal_Int32 freshFunctionOffset = 0;
        computeSlotSignatures(manager, *static_cast<unoidl::InterfaceTypeEntity const*>(ent.get()),
                              firstBase ? functionOffset : freshFunctionOffset, vtableOffset,
                              slotSignatures);
        vtableOffset += 4;
        firstBase = false;
    }
    for (auto const& attr : interface.getDirectAttributes())
    {
        slotSignatures.insert(
            computeGetterSlotSignature(manager, attr, functionOffset, orgVtableOffset));
        ++functionOffset;
        if (!attr.readOnly)
        {
            slotSignatures.insert(
                computeSetterSlotSignature(manager, attr, functionOffset, orgVtableOffset));
            ++functionOffset;
        }
    }
    for (auto const& meth : interface.getDirectMethods())
    {
        slotSignatures.insert(
            computeMethodSlotSignature(manager, meth, functionOffset, orgVtableOffset));
        ++functionOffset;
    }
}

void appendRttiSymbolSegment(OStringBuffer& buffer, OUString const& id)
{
    OString s(OUStringToOString(id, RTL_TEXTENCODING_ASCII_US));
    buffer.append(OString::number(s.getLength()) + s);
}

OString computeRttiSymbol(std::vector<OUString> const& path, OUString const& id)
{
    OStringBuffer buf("__ZTI");
    if (!path.empty())
    {
        buf.append('N');
        for (auto const& i : path)
        {
            appendRttiSymbolSegment(buf, i);
        }
    }
    appendRttiSymbolSegment(buf, id);
    if (!path.empty())
    {
        buf.append('E');
    }
    return buf.makeStringAndClear();
}

void scan(rtl::Reference<TypeManager> const& manager,
          rtl::Reference<unoidl::MapCursor> const& cursor, std::vector<OUString>& path,
          std::set<OString>& callSignatures, std::set<OString>& slotSignatures,
          std::set<OString>& rttis)
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
        switch (ent->getSort())
        {
            case unoidl::Entity::SORT_MODULE:
                path.push_back(id);
                scan(manager, static_cast<unoidl::ModuleEntity*>(ent.get())->createCursor(), path,
                     callSignatures, slotSignatures, rttis);
                path.pop_back();
                break;
            case unoidl::Entity::SORT_EXCEPTION_TYPE:
                rttis.insert(computeRttiSymbol(path, id));
                break;
            case unoidl::Entity::SORT_INTERFACE_TYPE:
            {
                auto const ite = static_cast<unoidl::InterfaceTypeEntity const*>(ent.get());
                for (auto const& attr : ite->getDirectAttributes())
                {
                    callSignatures.insert(computeGetterCallSignature(manager, attr));
                    if (!attr.readOnly)
                    {
                        callSignatures.insert(computeSetterCallSignature(manager, attr));
                    }
                }
                for (auto const& meth : ite->getDirectMethods())
                {
                    callSignatures.insert(computeMethodCallSignature(manager, meth));
                }
                computeSlotSignatures(manager, *ite, o3tl::temporary<sal_Int32>(0),
                                      o3tl::temporary<sal_Int32>(0), slotSignatures);
                break;
            }
            default:
                break;
        }
    }
}
}

SAL_IMPLEMENT_MAIN()
{
    try
    {
        auto const args = rtl_getAppCommandArgCount();
        if (args < 3)
        {
            badUsage();
        }
        auto const cppPathname = getPathnameArgument(0);
        auto const asmPathname = getPathnameArgument(1);
        auto const expPathname = getPathnameArgument(2);
        rtl::Reference<TypeManager> mgr(new TypeManager);
        for (sal_uInt32 i = 3; i != args; ++i)
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
        std::vector<OUString> path;
        std::set<OString> callSignatures;
        std::set<OString> slotSignatures;
        std::set<OString> rttis;
        for (auto const& prov : mgr->getPrimaryProviders())
        {
            scan(mgr, prov->createRootCursor(), path, callSignatures, slotSignatures, rttis);
        }
        std::ofstream cppOut(cppPathname, std::ios_base::out | std::ios_base::trunc);
        if (!cppOut)
        {
            std::cerr << "Cannot open \"" << cppPathname << "\" for writing\n";
            std::exit(EXIT_FAILURE);
        }
        cppOut << "#include <sal/config.h>\n"
                  "#include <bit>\n"
                  "#include <string_view>\n"
                  "#include <com/sun/star/uno/RuntimeException.hpp>\n"
                  "#include <rtl/ustring.hxx>\n"
                  "#include <sal/types.h>\n"
                  "#include <wasm/generated.hxx>\n";
        for (auto const& sig : callSignatures)
        {
            cppOut << "extern \"C\" void callVirtualFunction_" << sig
                   << "(sal_uInt32 target, sal_uInt64 const * arguments, void * returnValue);\n";
        }
        cppOut << "void callVirtualFunction(std::string_view signature, sal_uInt32 target, "
                  "sal_uInt64 const * arguments, void * returnValue) {\n";
        for (auto const& sig : callSignatures)
        {
            cppOut << "    if (signature == \"" << sig << "\") {\n"
                   << "        callVirtualFunction_" << sig << "(target, arguments, returnValue);\n"
                   << "        return;\n"
                   << "    }\n";
        }
        cppOut << "    throw css::uno::RuntimeException(\"Wasm bridge cannot call virtual function "
                  "with signature \" + OUString::fromUtf8(signature));\n"
                  "}\n";
        if (!slotSignatures.empty())
        {
            cppOut << "namespace {\n";
        }
        for (auto const& sig : slotSignatures)
        {
            auto const i1 = sig.indexOf('_');
            assert(i1 != -1);
            sal_Int32 i2 = i1 + 1;
            for (;; ++i2)
            {
                assert(i2 < sig.getLength());
                if (!rtl::isAsciiDigit(static_cast<unsigned char>(sig[i2])))
                {
                    break;
                }
            }
            assert(i2 != i1);
            cppOut << "extern \"C\" ";
            switch (sig[i2])
            {
                case 'd':
                    cppOut << "double";
                    break;
                case 'f':
                    cppOut << "float";
                    break;
                case 'i':
                    cppOut << "unsigned";
                    break;
                case 'j':
                    cppOut << "unsigned long long";
                    break;
                default:
                    cppOut << "void";
                    break;
            }
            cppOut << " vtableSlotFunction_" << sig << "(";
            if (sig[i2] == 'I')
            {
                cppOut << "unsigned indirectRet, ";
            }
            cppOut << "unsigned thisPtr";
            for (sal_Int32 i = i2 + 1; i != sig.getLength(); ++i)
            {
                cppOut << ", ";
                switch (sig[i])
                {
                    case 'd':
                        cppOut << "double";
                        break;
                    case 'f':
                        cppOut << "float";
                        break;
                    case 'i':
                        cppOut << "unsigned";
                        break;
                    case 'j':
                        cppOut << "unsigned long long";
                        break;
                    default:
                        O3TL_UNREACHABLE;
                }
                cppOut << " arg" << (i - i2);
            }
            cppOut << ") { ";
            switch (sig[i2])
            {
                case 'd':
                    cppOut << "return std::bit_cast<double>(static_cast<unsigned long long>(";
                    break;
                case 'f':
                    cppOut << "return std::bit_cast<float>(static_cast<unsigned>(";
                    break;
                case 'i':
                    cppOut << "return static_cast<unsigned>(";
                    break;
                case 'j':
                    cppOut << "return static_cast<unsigned long long>(";
                    break;
            }
            cppOut << "vtableCall(" << sig.subView(0, i1) << ", "
                   << sig.subView(i1 + 1, i2 - (i1 + 1)) << ", thisPtr, {";
            for (sal_Int32 i = i2 + 1; i != sig.getLength(); ++i)
            {
                if (i != i2 + 1)
                {
                    cppOut << ", ";
                }
                cppOut << "sal_uInt64(";
                switch (sig[i])
                {
                    case 'd':
                        cppOut << "std::bit_cast<unsigned long long>(";
                        break;
                    case 'f':
                        cppOut << "std::bit_cast<unsigned>(";
                        break;
                }
                cppOut << "arg" << (i - i2) << ")";
                switch (sig[i])
                {
                    case 'd':
                    case 'f':
                        cppOut << ")";
                        break;
                }
            }
            cppOut << "}, "
                   << (sig[i2] == 'I' ? "indirectRet" : "reinterpret_cast<unsigned>(nullptr)")
                   << ")";
            switch (sig[i2])
            {
                case 'd':
                case 'f':
                    cppOut << "))";
                    break;
                case 'i':
                case 'j':
                    cppOut << ")";
                    break;
            }
            cppOut << "; }\n";
        }
        if (!slotSignatures.empty())
        {
            cppOut << "}\n";
        }
        cppOut << "void const * getVtableSlotFunction(std::string_view signature) {\n";
        for (auto const& sig : slotSignatures)
        {
            cppOut << "    if (signature == \"" << sig << "\") {\n"
                   << "        return reinterpret_cast<void const*>(vtableSlotFunction_" << sig
                   << ");\n"
                   << "    }\n";
        }
        cppOut << "    throw css::uno::RuntimeException(\"Wasm bridge cannot fill virtual function "
                  "slot with signature \" + OUString::fromUtf8(signature));\n"
                  "}\n";
        cppOut.close();
        if (!cppOut)
        {
            std::cerr << "Failed to write \"" << cppPathname << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        std::ofstream asmOut(asmPathname, std::ios_base::out | std::ios_base::trunc);
        if (!asmOut)
        {
            std::cerr << "Cannot open \"" << asmPathname << "\" for writing\n";
            std::exit(EXIT_FAILURE);
        }
        asmOut << "\t.text\n"
                  "\t.tabletype __indirect_function_table, funcref\n";
        for (auto const& sig : callSignatures)
        {
            asmOut << "\t.functype callVirtualFunction_" << sig << " (i32, i32, i32) -> ()\n";
        }
        for (auto const& sig : callSignatures)
        {
            asmOut << "\t.section .text.callVirtualFunction_" << sig
                   << ",\"\",@\n"
                      "\t.globl callVirtualFunction_"
                   << sig
                   << "\n"
                      "\t.type callVirtualFunction_"
                   << sig
                   << ",@function\n"
                      "callVirtualFunction_"
                   << sig
                   << ":\n"
                      "\t.functype callVirtualFunction_"
                   << sig << " (i32, i32, i32) -> ()\n";
            if (sig[0] != 'v')
            {
                asmOut << "\tlocal.get 2\n";
            }
            unsigned off = 0;
            for (auto c : sig.subView(1))
            {
                asmOut << "\tlocal.get 1\n"
                          "\t";
                switch (c)
                {
                    case 'd':
                        asmOut << "f64";
                        break;
                    case 'f':
                        asmOut << "f32";
                        break;
                    case 'i':
                        asmOut << "i32";
                        break;
                    case 'j':
                        asmOut << "i64";
                        break;
                    default:
                        assert(false);
                }
                asmOut << ".load " << off << "\n";
                off += 8; //TODO: overflow
            }
            asmOut << "\tlocal.get 0\n"
                      "\tcall_indirect (";
            auto first = true;
            for (auto c : sig.subView(1))
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    asmOut << ", ";
                }
                switch (c)
                {
                    case 'd':
                        asmOut << "f64";
                        break;
                    case 'f':
                        asmOut << "f32";
                        break;
                    case 'i':
                        asmOut << "i32";
                        break;
                    case 'j':
                        asmOut << "i64";
                        break;
                    default:
                        assert(false);
                }
            }
            asmOut << ") -> (";
            switch (sig[0])
            {
                case 'd':
                    asmOut << "f64";
                    break;
                case 'f':
                    asmOut << "f32";
                    break;
                case 'i':
                    asmOut << "i32";
                    break;
                case 'j':
                    asmOut << "i64";
                    break;
                case 'v':
                    break;
                default:
                    assert(false);
            }
            asmOut << ")\n";
            if (sig[0] != 'v')
            {
                asmOut << "\t";
                switch (sig[0])
                {
                    case 'd':
                        asmOut << "f64";
                        break;
                    case 'f':
                        asmOut << "f32";
                        break;
                    case 'i':
                        asmOut << "i32";
                        break;
                    case 'j':
                        asmOut << "i64";
                        break;
                    default:
                        assert(false);
                }
                asmOut << ".store 0\n";
            }
            asmOut << "\tend_function\n";
        }
        asmOut.close();
        if (!asmOut)
        {
            std::cerr << "Failed to write \"" << asmPathname << "\"\n";
            std::exit(EXIT_FAILURE);
        }
        std::ofstream expOut(expPathname, std::ios_base::out | std::ios_base::trunc);
        if (!expOut)
        {
            std::cerr << "Cannot open \"" << expPathname << "\" for writing\n";
            std::exit(EXIT_FAILURE);
        }
        for (auto const& rtti : rttis)
        {
            expOut << rtti.getStr() << "\n";
        }
        expOut.close();
        if (!expOut)
        {
            std::cerr << "Failed to write \"" << expPathname << "\"\n";
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

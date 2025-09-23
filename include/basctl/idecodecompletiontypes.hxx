/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>
#include <map>
#include <optional>
#include <memory>
#include <string_view>

#include <rtl/ustring.hxx>
#include <basic/sbxdef.hxx>

namespace basctl
{
enum class IdeSymbolKind
{
    UNKNOWN,
    VARIABLE,
    CONSTANT,
    PARAMETER,
    FUNCTION,
    SUB,
    PROPERTY_GET,
    PROPERTY_LET,
    PROPERTY_SET,
    UDT, // User Defined Type (Struct)
    UDT_MEMBER,
    ENUM,
    ENUM_MEMBER,
    MODULE,
    NAMESPACE,
    LIBRARY,
    CLASS_MODULE, // BASIC Class Module
    // UNO specific kinds
    UNO_SERVICE,
    UNO_INTERFACE,
    UNO_STRUCT,
    UNO_ENUM,
    UNO_CONSTANTS, // A constants group
    UNO_EXCEPTION,
    UNO_TYPE, // Generic UNO type
    UNO_TYPEDEF,
    UNO_METHOD,
    UNO_PROPERTY,
    UNO_FIELD, // For struct/exception members
    // Built-in specific kinds
    BUILTIN_FUNCTION,
    BUILTIN_PROPERTY,
    GLOBAL_CONSTANT_UNO, // e.g. StarDesktop
    // Logical groupings for the browser UI
    ROOT_APPLICATION_LIBS,
    ROOT_DOCUMENT_LIBS,
    ROOT_UNO_APIS,
    ROOT_BASIC_BUILTINS,
    UNO_NAMESPACE,
    PLACEHOLDER // For temporary UI nodes like "[Loading...]"
};

enum class IdeScopeKind
{
    UNKNOWN,
    GLOBAL_APPLICATION,
    GLOBAL_DOCUMENT,
    LIBRARY_PUBLIC,
    MODULE_PUBLIC,
    MODULE_PRIVATE,
    CLASS_MEMBER_PUBLIC,
    CLASS_MEMBER_PRIVATE,
    PROCEDURE_LOCAL,
    BLOCK_LOCAL,
    WITH_BLOCK_MEMBER
};

enum class IdeAccessModifier
{
    NOT_APPLICABLE,
    PUBLIC,
    PRIVATE
};

struct IdeParamInfo
{
    OUString sName;
    OUString sTypeName;
    OUString sQualifiedTypeName;
    SbxDataType eBasicType = SbxDataType::SbxEMPTY;
    bool bIsByVal = true;
    bool bIsOptional = false;
    bool bIsUnoTypeOrigin = false;
    std::optional<OUString> osDefaultValueExpression;
    bool bIsParamArray = false;
    // For UNO parameters
    bool bIsOut = false;
    bool bIsIn = true;

    IdeParamInfo() = default;
    IdeParamInfo(OUString sName_, OUString sTypeName_, bool bOut, bool bIn);
};

/**
 * @struct IdeDimensionInfo
 * @brief Represents array dimension bounds.
 */
struct IdeDimensionInfo
{
    sal_Int32 nLowerBound = 0;
    sal_Int32 nUpperBound = 0;
    bool bIsDynamic = true;

    IdeDimensionInfo() = default;
    IdeDimensionInfo(sal_Int32 lower, sal_Int32 upper, bool dynamic = false);
};

/**
 * @struct IdeSymbolInfo
 * @brief Comprehensive structure for storing metadata about any program symbol.
 * @comment For mapMembers, the IdeSymbolInfo objects pointed to are owned.
 *          Direct cycles through mapMembers are not expected from BASIC language constructs.
 *          UNO type cycles are handled by name resolution against a global UNO type cache.
 */
struct IdeSymbolInfo
{
    bool bSelectable = true;
    OUString sIdentifier;
    OUString sName;
    OUString sQualifiedName;
    IdeSymbolKind eKind = IdeSymbolKind::UNKNOWN;
    OUString sTypeName;
    OUString sQualifiedTypeName;
    SbxDataType eBasicType = SbxDataType::SbxEMPTY;
    bool bIsUnoTypeOrigin = false;
    IdeScopeKind eScope = IdeScopeKind::UNKNOWN;
    OUString sOriginLibrary;
    OUString sOriginLocation;
    OUString sOriginModule;
    OUString sOriginUnoContainer;
    OUString sParentName;
    sal_Int32 nSourceLine = -1;
    sal_Int32 nSourceColumn = -1;
    std::vector<IdeParamInfo> aParameters;
    OUString sReturnTypeName;
    OUString sQualifiedReturnTypeName;
    bool bIsArray = false;
    std::vector<IdeDimensionInfo> aArrayDimensions;
    IdeAccessModifier eAccessModifier = IdeAccessModifier::NOT_APPLICABLE;
    bool bIsResolved = true;
    bool bHasParseErrors = false;
    std::optional<OUString> osConstantValue;
    std::optional<OUString> osBriefDescription;
    std::optional<OUString> osFullSignatureForDisplay;
    std::optional<OUString> osHelpURL;
    std::map<OUString, std::shared_ptr<IdeSymbolInfo>> mapMembers;

    IdeSymbolInfo() = default;
    IdeSymbolInfo(std::u16string_view rName, IdeSymbolKind eTheKind,
                  std::u16string_view rParentIdentifier);

    void AddMember(std::shared_ptr<IdeSymbolInfo> pMember);
};

using SymbolInfoList = std::vector<std::shared_ptr<IdeSymbolInfo>>;
using ParamInfoList = std::vector<IdeParamInfo>;

OUString CreateRootIdentifier(IdeSymbolKind eKind, std::u16string_view sOptionalPayload = u"");

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

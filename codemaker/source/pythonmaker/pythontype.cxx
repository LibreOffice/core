/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "pythonoptions.hxx"
#include "pythontype.hxx"

#include <codemaker/codemaker.hxx>
#include <codemaker/exceptiontree.hxx>
#include <codemaker/generatedtypeset.hxx>
#include <codemaker/global.hxx>
#include <codemaker/options.hxx>
#include <codemaker/typemanager.hxx>
#include <codemaker/unotype.hxx>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <unoidl/unoidl.hxx>

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_set.h>

namespace codemaker::pythonmaker
{
namespace
{
//All 36 keywords in python which will handle the case where variable names in idl files common with python keywords
constexpr auto PYTHON_KEYWORDS = frozen::make_unordered_set<std::string_view>(
    { "False", "None",     "True",  "and",    "as",   "assert", "async",  "await",    "break",
      "class", "continue", "def",   "del",    "elif", "else",   "except", "finally",  "for",
      "from",  "global",   "if",    "import", "in",   "is",     "lambda", "nonlocal", "not",
      "or",    "pass",     "raise", "return", "try",  "while",  "with",   "yield" });

std::set<OUString> g_initPyiCache;

//generates the proper naming of the import statements (the alias is a '_' that handles the file name conflicting with keywords)
OString generateImportStatementLocal(std::string_view fromModule, std::string_view importName,
                                     std::string_view alias)
{
    OString statement = "from "_ostr + codemaker::convertString(b2u(fromModule)) + " import "_ostr
                        + codemaker::convertString(b2u(importName));
    if (!alias.empty())
    {
        statement += " as "_ostr + codemaker::convertString(b2u(alias));
    }
    return statement;
}
}

//If idl has same variable name as a python keywords this function adds a '_' after it to avoid error.
// e.g variable 'and' changes to 'and_'
OString getSafePythonIdentifier(const OString& unoIdentifier)
{
    if (PYTHON_KEYWORDS.count(unoIdentifier))
    {
        return unoIdentifier + "_"_ostr;
    }
    return unoIdentifier;
}

OString unoNameToPyModulePath(const OUString& unoName)
{
    return codemaker::convertString(unoName).replace(
        '.', '/'); //replaces the com.sun.star.... to com/sun/star/....
}

//Maps the Uno types to its respective Pythontype
OString mapUnoTypeToPythonHint(std::u16string_view unoTypeName, TypeManager const& typeManager,
                               OUString const& currentModuleUnoName,
                               OString const& currentClassName, std::set<OString>& imports,
                               std::set<OUString>& dependentTypes)
{
    if (unoTypeName.empty() || unoTypeName == u"void")
    {
        // UNO 'void' type maps to Python's 'None'
        return "None"_ostr;
    }

    OUString nucleus; //the core ,base name of the type
    sal_Int32 rank; // The number of 'sequence<...>' wrappers (0 for non-sequences)
    std::vector<OUString>
        arguments; // Holds type parameters for generic/polymorphic structs. Empty for non-generics
    codemaker::UnoType::Sort sort = typeManager.decompose(
        unoTypeName, true, &nucleus, &rank, &arguments, nullptr); //fundamental kind of the type

    OString pyNucleusHint; //The fundamental kind of the UNO type

    switch (sort)
    {
        case codemaker::UnoType::Sort::Boolean:
            pyNucleusHint = "bool"_ostr;
            break;
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
            // All UNO integer types map to Python's single, arbitrary-precision 'int' type.
            pyNucleusHint = "int"_ostr;
            break;
        case codemaker::UnoType::Sort::Float:
        case codemaker::UnoType::Sort::Double: //no separate double type in python
            pyNucleusHint = "float"_ostr;
            break;
        case codemaker::UnoType::Sort::Char: //no separate char type in python
        case codemaker::UnoType::Sort::String:
            pyNucleusHint = "str"_ostr;
            break;
        case codemaker::UnoType::Sort::Type:
            // UNO's 'type' concept requires 'Type' from Python's 'typing' module for hints
            imports.insert(generateImportStatementLocal("typing", "Type", ""));
            imports.insert(generateImportStatementLocal("typing", "Any", ""));
            pyNucleusHint = "Type[Any]"_ostr;
            break;
        case codemaker::UnoType::Sort::Any:
            imports.insert(generateImportStatementLocal("typing", "Any", ""));
            pyNucleusHint = "Any"_ostr;
            break;

        case codemaker::UnoType::Sort::Enum:
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::Exception:
        case codemaker::UnoType::Sort::Interface:
        case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
        {
            OString pySimpleNameOfNucleus; // This will hold the final, safe Python class name
            sal_Int32 lastDotInNucleus = nucleus.lastIndexOf('.');
            // Extract the simple name (e.g., "XInterface" from "com.sun.star.uno.XInterface")
            if (lastDotInNucleus != -1)
            {
                pySimpleNameOfNucleus = getSafePythonIdentifier(
                    codemaker::convertString(OUString(nucleus.subView(lastDotInNucleus + 1))));
            }
            else
            {
                pySimpleNameOfNucleus = getSafePythonIdentifier(codemaker::convertString(nucleus));
            }

            // Construct the full UNO name of the class currently being generated for comparison
            OUString currentFullUnoName = currentModuleUnoName.isEmpty()
                                              ? b2u(currentClassName)
                                              : currentModuleUnoName + u"." + b2u(currentClassName);

            // Check for a forward reference (a type referring to itself)
            if (nucleus == currentFullUnoName)
            {
                // If the type hint is for the very class we are defining, use a string literal
                // to prevent a "name not yet defined" error
                pyNucleusHint = "'"_ostr + pySimpleNameOfNucleus + "'"_ostr;
            }
            else
            {
                // The type is defined elsewhere, so we must import it and add it as a dependency
                dependentTypes.insert(nucleus);

                OString fullModulePathForImport = codemaker::convertString(nucleus);
                OString classToImport = pySimpleNameOfNucleus;
                // Create a unique, safe alias to prevent any potential import name collisions
                OString alias = fullModulePathForImport.replace('.', '_') + "_"_ostr;

                imports.insert(
                    generateImportStatementLocal(fullModulePathForImport, classToImport, alias));

                // Use the safe alias as the type hint in the generated code
                pyNucleusHint = alias;
            }

            // Handle generic type parameters, e.g., for Pair<string, long>.
            if (sort == codemaker::UnoType::Sort::InstantiatedPolymorphicStruct
                && !arguments.empty())
            {
                pyNucleusHint += "[";
                for (size_t i = 0; i < arguments.size(); ++i)
                {
                    // Recursively call this same function for each type inside the <...>.
                    // This allows for nesting, like Pair<string, Sequence<long>>.
                    pyNucleusHint
                        += mapUnoTypeToPythonHint(arguments[i], typeManager, currentModuleUnoName,
                                                  currentClassName, imports, dependentTypes);
                    if (i < arguments.size() - 1)
                    {
                        pyNucleusHint += ", "; //add ',' between parameters
                    }
                }
                pyNucleusHint += "]";
            }
            break;
        }
        default:
            imports.insert(generateImportStatementLocal("typing", "Any", ""));
            pyNucleusHint = "Any"_ostr; // Fallback to 'Any' for unhandled or unknown types
            break;
    }

    if (rank > 0)
    {
        // If rank > 0, the UNO type is a sequence (e.g., sequence<long>).
        imports.insert(generateImportStatementLocal("typing", "List", ""));
        OString finalHint = pyNucleusHint;

        // Wrap the hint in "List[...]" for each level of nesting
        // The 'rank' variable indicates how many wrappers are needed
        for (sal_Int32 i = 0; i < rank; ++i)
        {
            finalHint = "List["_ostr + finalHint + "]"_ostr;
        }
        return finalHint;
    }
    return pyNucleusHint;
}

// Create __init__.pyi files for the corresponding directory structure
// __init__.pyi file allows a directory to get identified as a python directory
void ensureInitPyi(const OString& baseOutputDir, const OUString& unoModuleName)
{
    if (unoModuleName.isEmpty() || g_initPyiCache.count(unoModuleName))
    {
        // Optimization: If the module is empty or we've already processed it in this run, do nothing.
        return;
    }

    // Recursively ensure the parent module is also created.
    // e.g., for "com.sun.star.sheet", this will first ensure "com.sun.star".
    sal_Int32 lastDot = unoModuleName.lastIndexOf('.');
    if (lastDot != -1)
    {
        ensureInitPyi(baseOutputDir, unoModuleName.copy(0, lastDot));
    }

    // Now, handle the current module level.
    OString initPyiNameAsType = codemaker::convertString(unoModuleName) + ".__init__"_ostr;
    OString initPyiPath = createFileNameFromType(baseOutputDir, initPyiNameAsType, ".pyi"_ostr);

    // Because of the recursion and cache, we only need to check the final file.
    // The parent directories are guaranteed to exist by the recursive call.
    if (!fileExists(initPyiPath))
    {
        FileStream initFile;
        // The directory should already exist due to the parent's createFileNameFromType call.
        // We just need a temp file to atomically create the __init__.pyi.
        OString parentDir = getTempDir(initPyiPath);
        initFile.createTempFile(parentDir);

        if (initFile.isValid())
        {
            OString tempInitName = initFile.getName();
            initFile << "# Auto-generated __init__.pyi by pythonmaker\n"_ostr;
            initFile.close();
            if (!makeValidTypeFile(initPyiPath, tempInitName, false))
            {
                std::cerr << "Warning: Could not create/finalize __init__.pyi at " << initPyiPath
                          << std::endl;
            }
        }
        else
        {
            std::cerr << "Warning: Could not create temp file for __init__.pyi in " << parentDir
                      << std::endl;
        }
    }

    // Mark this module as processed so we don't do it again.
    g_initPyiCache.insert(unoModuleName);
}

PythonStubGenerator::PythonStubGenerator(OUString const& name,
                                         rtl::Reference<TypeManager> const& manager,
                                         codemaker::GeneratedTypeSet& generatedSet,
                                         PythonOptions const& options)
    : m_unoName(name) // Store the full UNO name of the current type.
    , m_typeManager(manager) // Store the reference to the type manager.
    , m_generatedSet(generatedSet) // Store the reference to the set of generated types.
    , m_options(options) // Store the command-line options.
    , m_indentLevel(0) // Initialize indentation level to 0.
{
    // Retrieve the base output directory from the command-line options (e.g., the path after -O)
    m_baseOutputDir = m_options.getOption("-O"_ostr);
    m_verbose = m_options.isValid("--verbose"_ostr);
    // Deconstruct the full UNO name into its module path and the simple class name
    sal_Int32 lastDot = m_unoName.lastIndexOf('.');
    if (lastDot != -1)
    {
        m_moduleName = m_unoName.copy(0, lastDot);
        m_pyClassName = u2b(m_unoName.subView(lastDot + 1));
    }
    else
    {
        m_moduleName = OUString();
        m_pyClassName = u2b(m_unoName);
    }
    m_pySafeClassName = getSafePythonIdentifier(m_pyClassName);
}

void PythonStubGenerator::generate()
{
    if (m_pyClassName.isEmpty())
    {
        return;
    }

    rtl::Reference<unoidl::Entity> entity;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(m_unoName, &entity);

    m_filePath
        = createFileNameFromType(m_baseOutputDir, unoNameToPyModulePath(m_unoName), ".pyi"_ostr);

    // Get the UNO type to display in the --verbose(-V) option
    if (m_verbose)
    {
        const char* sortName = "unknown";
        switch (sort)
        {
            case codemaker::UnoType::Sort::Enum:
                sortName = "enum";
                break;
            case codemaker::UnoType::Sort::ConstantGroup:
                sortName = "constants";
                break;
            case codemaker::UnoType::Sort::Typedef:
                sortName = "typedef";
                break;
            case codemaker::UnoType::Sort::PlainStruct:
                sortName = "struct";
                break;
            case codemaker::UnoType::Sort::Exception:
                sortName = "exception";
                break;
            case codemaker::UnoType::Sort::PolymorphicStructTemplate:
                sortName = "polystruct";
                break;
            case codemaker::UnoType::Sort::Interface:
                sortName = "interface";
                break;
            case codemaker::UnoType::Sort::SingleInterfaceBasedService:
                sortName = "service";
                break;
            case codemaker::UnoType::Sort::InterfaceBasedSingleton:
                sortName = "singleton";
                break;
            default:
                break;
        }
        //display the message in here
        std::cout << "[" << sortName << "] " << u2b(m_unoName) << " -> " << m_filePath << std::endl;
    }

    // Check if the file already exists to respect the -G (generate if not exists) option
    if (fileExists(m_filePath))
    {
        if (m_options.isValid("-G"_ostr))
            return;
    }

    // Creating a temporary files for safety and atomicity
    FileStream tempFile;
    tempFile.createTempFile(getTempDir(m_filePath));
    if (!tempFile.isValid())
    {
        //if the temporary files aren't created something might be wrong with the permissions
        OUString errorMessage = u"Cannot create temporary file for "_ustr;
        errorMessage += b2u(m_filePath);
        throw CannotDumpException(errorMessage);
    }
    OString tempFilePath = tempFile.getName();

    /*The main dispatch logic. Based on the 'sort' of the UNO type, call the
     appropriate specialized generator method.*/
    switch (sort)
    {
        case codemaker::UnoType::Sort::Enum:
            generateEnum(static_cast<unoidl::EnumTypeEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::ConstantGroup:
            generateConstantGroup(static_cast<unoidl::ConstantGroupEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::Typedef:
            generateTypedef(static_cast<unoidl::TypedefEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::PlainStruct:
            generateStruct(static_cast<unoidl::PlainStructTypeEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::Exception:
            generateException(static_cast<unoidl::ExceptionTypeEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::PolymorphicStructTemplate:
            generatePolyStruct(
                static_cast<unoidl::PolymorphicStructTypeTemplateEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::Interface:
            generateInterface(static_cast<unoidl::InterfaceTypeEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::SingleInterfaceBasedService:
            generateService(static_cast<unoidl::SingleInterfaceBasedServiceEntity*>(entity.get()));
            break;
        case codemaker::UnoType::Sort::InterfaceBasedSingleton:
            generateSingleton(static_cast<unoidl::InterfaceBasedSingletonEntity*>(entity.get()));
            break;
        default:
            // For any UNO type we don't handle explicitly yet, generate a minimal placeholder
            m_buffer.append("class ");
            m_buffer.append(m_pySafeClassName);
            m_buffer.append(":\n");
            indent();
            m_buffer.append(getIndent());
            m_buffer.append("pass\n");
            dedent();
            break;
    }

    // Assemble the final content of the .pyi file in the correct order
    OStringBuffer finalBuffer(
        "# Auto-generated by pythonmaker - DO NOT EDIT.\nfrom __future__ import annotations\n");
    if (!m_imports.empty())
    {
        finalBuffer.append("\n");
    }
    // Add collected imports
    for (const auto& imp : m_imports)
    {
        finalBuffer.append(imp + "\n"_ostr);
    }
    if (!m_imports.empty() || !m_buffer.isEmpty())
    {
        finalBuffer.append("\n");
    }
    finalBuffer.append(m_buffer.makeStringAndClear()); // Append the main content

    // Write the fully assembled content to the temporary file.
    tempFile << finalBuffer;
    tempFile.close();

    //checks if the new overwritten file will be same as the current file
    if (!makeValidTypeFile(m_filePath, tempFilePath, m_options.isValid("-Gc"_ostr)))
    {
        OUString errorMessage = u"Cannot finalize file "_ustr;
        errorMessage += b2u(m_filePath);
        throw CannotDumpException(errorMessage);
    }

    // the -nD option prevent generation of dependent types
    if (!m_options.isValid("-nD"_ostr))
    {
        m_generatedSet.add(u2b(m_unoName));
        for (const auto& dep : m_dependentTypes)
        {
            produce(dep, m_typeManager, m_generatedSet, m_options);
        }
    }
}

void PythonStubGenerator::addImportLine(const OString& importLine)
{
    if (!importLine.isEmpty())
    {
        m_imports.insert(importLine);
    }
}

std::vector<unoidl::PlainStructTypeEntity::Member>
PythonStubGenerator::getAllStructMembers(unoidl::PlainStructTypeEntity* entity)
{
    std::vector<unoidl::PlainStructTypeEntity::Member> members;

    // Get members from base classes first by recursing
    if (!entity->getDirectBase().isEmpty())
    {
        rtl::Reference<unoidl::Entity> baseEntity;
        // Look up the base entity in the TypeManager
        if (m_typeManager->getSort(entity->getDirectBase(), &baseEntity)
            == codemaker::UnoType::Sort::PlainStruct)
        {
            unoidl::PlainStructTypeEntity* baseStructEntity
                = static_cast<unoidl::PlainStructTypeEntity*>(baseEntity.get());
            members = getAllStructMembers(baseStructEntity); // Recursive call
        }
    }

    // Add members from the current entity
    const auto& directMembers = entity->getDirectMembers();
    members.insert(members.end(), directMembers.begin(), directMembers.end());

    return members;
}

void PythonStubGenerator::generatePolyStruct(unoidl::PolymorphicStructTypeTemplateEntity* entity)
{
    addImportLine("from typing import TypeVar, Generic"_ostr); //imports to handle generic types
    OStringBuffer typeVarBuffer;
    OStringBuffer genericParams; // For the Generic[T, K] part
    bool firstParam = true;
    for (const auto& paramName : entity->getTypeParameters())
    {
        OString safeParamName = getSafePythonIdentifier(codemaker::convertString(paramName));
        typeVarBuffer.append(safeParamName + " = TypeVar(\""_ostr);
        typeVarBuffer.append(safeParamName + "\")\n"_ostr);

        if (!firstParam)
        {
            genericParams.append(", ");
        }
        genericParams.append(safeParamName);
        firstParam = false;
    }
    typeVarBuffer.append("\n");
    // Prepend this to the main buffer
    m_buffer.insert(m_buffer.getLength(), typeVarBuffer.makeStringAndClear());

    // Generate the class definition
    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    m_buffer.append("(Generic[");
    m_buffer.append(genericParams.makeStringAndClear());
    m_buffer.append("]):\n");
    indent();

    // Generate the __init__ method
    m_buffer.append(getIndent());
    m_buffer.append("def __init__(self");
    const auto& members = entity->getMembers();
    for (const auto& member : members)
    {
        OString safeMemberName = getSafePythonIdentifier(codemaker::convertString(member.name));
        OString memberTypeHint;
        if (member.parameterized)
        {
            // It's a generic type parameter like 'T'
            memberTypeHint = getSafePythonIdentifier(codemaker::convertString(member.type));
        }
        else
        {
            // It's a concrete type, map it normally
            memberTypeHint = mapUnoTypeToPythonHint(member.type, *m_typeManager, m_moduleName,
                                                    m_pyClassName, m_imports, m_dependentTypes);
        }
        m_buffer.append(", ");
        m_buffer.append(safeMemberName);
        m_buffer.append(": ");
        m_buffer.append(memberTypeHint);
        m_buffer.append(" | None = ...");
    }
    m_buffer.append(") -> None:\n");
    indent();
    m_buffer.append(getIndent());
    m_buffer.append("...\n");
    dedent();
    m_buffer.append("\n");

    // Generate typed attributes
    if (members.empty())
    {
        m_buffer.append(getIndent());
        m_buffer.append("pass\n");
    }
    else
    {
        for (const auto& member : members)
        {
            OString safeMemberName = getSafePythonIdentifier(codemaker::convertString(member.name));
            OString memberTypeHint;
            if (member.parameterized)
            {
                memberTypeHint = getSafePythonIdentifier(codemaker::convertString(member.type));
            }
            else
            {
                memberTypeHint = mapUnoTypeToPythonHint(member.type, *m_typeManager, m_moduleName,
                                                        m_pyClassName, m_imports, m_dependentTypes);
            }
            m_buffer.append(getIndent());
            m_buffer.append(safeMemberName);
            m_buffer.append(": ");
            m_buffer.append(memberTypeHint);
            m_buffer.append("\n");
        }
    }
    dedent();
}

std::vector<unoidl::ExceptionTypeEntity::Member>
PythonStubGenerator::getAllExceptionMembers(unoidl::ExceptionTypeEntity* entity)
{
    std::vector<unoidl::ExceptionTypeEntity::Member> members;

    // Recurse up the inheritance chain
    OUString baseName = entity->getDirectBase();
    if (!baseName.isEmpty())
    {
        // Stop recursion at the root UNO Exception, as its members (Message, Context)
        // are handled specially or are part of Python's built-in Exception.
        if (baseName != u"com.sun.star.uno.Exception")
        {
            rtl::Reference<unoidl::Entity> baseEntity;
            if (m_typeManager->getSort(baseName, &baseEntity)
                == codemaker::UnoType::Sort::Exception)
            {
                unoidl::ExceptionTypeEntity* baseExceptionEntity
                    = static_cast<unoidl::ExceptionTypeEntity*>(baseEntity.get());
                members = getAllExceptionMembers(baseExceptionEntity);
            }
        }
    }

    // Add members from the current entity
    const auto& directMembers = entity->getDirectMembers();
    members.insert(members.end(), directMembers.begin(), directMembers.end());
    return members;
}

void PythonStubGenerator::generateEnum(unoidl::EnumTypeEntity* entity)
{
    //just using the enum package provided in python itself
    addImportLine("import enum"_ostr);
    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    m_buffer.append("(enum.Enum):\n");
    indent();

    const auto& members = entity->getMembers();
    if (members.empty())
    {
        m_buffer.append(getIndent());
        m_buffer.append("pass\n");
    }
    else
    {
        for (const auto& member : members)
        {
            OString safeMemberName = getSafePythonIdentifier(u2b(member.name));
            m_buffer.append(getIndent());
            m_buffer.append(safeMemberName);
            m_buffer.append(" = ");
            m_buffer.append(OString::number(member.value));
            m_buffer.append("\n");
        }
    }
    dedent();
}

void PythonStubGenerator::generateConstantGroup(unoidl::ConstantGroupEntity* entity)
{
    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    m_buffer.append(":\n");
    indent();

    const auto& members = entity->getMembers();
    if (members.empty())
    {
        m_buffer.append(getIndent());
        m_buffer.append("# No constants defined in this group\npass\n"); // Or just an empty file
    }
    else
    {
        for (const auto& member : members)
        {
            OString safeConstName = getSafePythonIdentifier(u2b(member.name));

            OString pyValue;
            OString pyTypeHint;

            switch (member.value.type)
            {
                case unoidl::ConstantValue::TYPE_BOOLEAN:
                    pyValue = member.value.booleanValue ? "True"_ostr : "False"_ostr;
                    pyTypeHint = "bool"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_BYTE:
                    pyValue = OString::number(member.value.byteValue);
                    pyTypeHint = "int"_ostr; // Python bytes are different
                    break;
                case unoidl::ConstantValue::TYPE_SHORT:
                    pyValue = OString::number(member.value.shortValue);
                    pyTypeHint = "int"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
                    pyValue = OString::number(member.value.unsignedShortValue);
                    pyTypeHint = "int"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_LONG:
                    pyValue = OString::number(member.value.longValue);
                    pyTypeHint = "int"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
                    pyValue
                        = OString::number(static_cast<sal_Int32>(member.value.unsignedLongValue));
                    pyTypeHint = "int"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_HYPER:
                    pyValue = OString::number(member.value.hyperValue);
                    pyTypeHint = "int"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
                    pyValue
                        = OString::number(static_cast<sal_Int64>(member.value.unsignedHyperValue));
                    pyTypeHint = "int"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_FLOAT:
                    pyValue = OString::number(member.value.floatValue);
                    pyTypeHint = "float"_ostr;
                    break;
                case unoidl::ConstantValue::TYPE_DOUBLE:
                    pyValue = OString::number(member.value.doubleValue);
                    pyTypeHint = "float"_ostr; // Python float is double precision
                    break;
                default:
                    // Should not happen for valid constant groups
                    pyValue = "...  # Unknown constant type"_ostr;
                    pyTypeHint = "Any"_ostr;
                    addImportLine("from typing import Any"_ostr);
                    break;
            }

            m_buffer.append(getIndent());
            m_buffer.append(safeConstName);
            m_buffer.append(": ");
            m_buffer.append(pyTypeHint);
            m_buffer.append(" = ");
            m_buffer.append(pyValue);
            m_buffer.append("\n");
        }
    }
    dedent();
}

void PythonStubGenerator::generateTypedef(unoidl::TypedefEntity* entity)
{
    OUString originalUnoType = entity->getType();
    OString pythonTypeHintForOriginal = mapUnoTypeToPythonHint(
        originalUnoType, *m_typeManager, m_moduleName,
        m_pyClassName, // The UNO module name of the current typedef file
        m_imports, // Set to collect import strings for this .pyi file
        m_dependentTypes // Set to collect UNO type names that need to be generated
    );

    m_buffer.append(m_pySafeClassName);
    m_buffer.append(" = ");
    m_buffer.append(pythonTypeHintForOriginal);
    m_buffer.append("\n");
}

void PythonStubGenerator::generateStruct(unoidl::PlainStructTypeEntity* entity)
{
    // Handle the base class (inheritance)
    OString baseClass = "object"_ostr; // Default base for Python classes
    if (!entity->getDirectBase().isEmpty())
    {
        baseClass = mapUnoTypeToPythonHint(entity->getDirectBase(), *m_typeManager, m_moduleName,
                                           m_pyClassName, m_imports, m_dependentTypes);
    }

    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    m_buffer.append("(");
    m_buffer.append(baseClass);
    m_buffer.append("):\n");
    indent();

    // Generate the __init__ method signature
    m_buffer.append(getIndent());
    m_buffer.append("def __init__(self");

    // We need to collect all members, from this struct and all its base structs.
    // This requires a helper function to walk the inheritance tree.
    std::vector<unoidl::PlainStructTypeEntity::Member> allMembers = getAllStructMembers(entity);

    for (const auto& member : allMembers)
    {
        OString safeMemberName = getSafePythonIdentifier(codemaker::convertString(member.name));
        OString memberTypeHint = mapUnoTypeToPythonHint(member.type, *m_typeManager, m_moduleName,
                                                        m_pyClassName, m_imports, m_dependentTypes);

        m_buffer.append(", ");
        m_buffer.append(safeMemberName);
        m_buffer.append(": ");
        m_buffer.append(memberTypeHint);
        m_buffer.append(" | None = ...");
    }
    m_buffer.append(") -> None:\n");
    indent();
    m_buffer.append(getIndent());
    m_buffer.append("...\n");
    dedent();
    m_buffer.append("\n");

    // Generate typed attributes for all members
    if (allMembers.empty())
    {
        m_buffer.append(getIndent());
        m_buffer.append("pass\n");
    }
    else
    {
        for (const auto& member : allMembers)
        {
            OString safeMemberName = getSafePythonIdentifier(codemaker::convertString(member.name));
            OString memberTypeHint
                = mapUnoTypeToPythonHint(member.type, *m_typeManager, m_moduleName, m_pyClassName,
                                         m_imports, m_dependentTypes);
            m_buffer.append(getIndent());
            m_buffer.append(safeMemberName);
            m_buffer.append(": ");
            m_buffer.append(memberTypeHint);
            m_buffer.append("\n");
        }
    }
    dedent();
}

void PythonStubGenerator::generateException(unoidl::ExceptionTypeEntity* entity)
{
    // Determine the base class for Python stub
    OString baseClass;
    OUString unoBaseName = entity->getDirectBase();
    bool isRootUnoException = (m_unoName == u"com.sun.star.uno.Exception");
    if (unoBaseName.isEmpty() || isRootUnoException)
    {
        // If it has no base or the base is the root UNO Exception,
        // it should inherit from Python's built-in Exception.
        baseClass = "Exception"_ostr;
    }
    else
    {
        // Inherit from the stub of its direct UNO base exception.
        baseClass = mapUnoTypeToPythonHint(unoBaseName, *m_typeManager, m_moduleName, m_pyClassName,
                                           m_imports, m_dependentTypes);
    }

    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    if (isRootUnoException && m_pySafeClassName == "Exception")
    {
        m_buffer.append("(object):\n");
    }
    else
    {
        m_buffer.append("(");
        m_buffer.append(baseClass);
        m_buffer.append("):\n");
    }
    indent();

    // Get all members, including those from base UNO exceptions
    std::vector<unoidl::ExceptionTypeEntity::Member> allMembers = getAllExceptionMembers(entity);

    m_buffer.append(getIndent());
    m_buffer.append("def __init__(self");

    for (const auto& member : allMembers)
    {
        OString safeMemberName = getSafePythonIdentifier(codemaker::convertString(member.name));
        OString memberTypeHint = mapUnoTypeToPythonHint(member.type, *m_typeManager, m_moduleName,
                                                        m_pyClassName, m_imports, m_dependentTypes);

        m_buffer.append(", ");
        m_buffer.append(safeMemberName);
        m_buffer.append(": ");
        m_buffer.append(memberTypeHint);
        m_buffer.append(" | None = ...");
    }

    m_buffer.append(", *args: Any, **kwargs: Any) -> None:\n");
    addImportLine("from typing import Any"_ostr);

    indent();
    m_buffer.append(getIndent());
    m_buffer.append("...\n");
    dedent();
    m_buffer.append("\n");

    for (const auto& member : allMembers)
    {
        OString safeMemberName = getSafePythonIdentifier(codemaker::convertString(member.name));
        OString memberTypeHint = mapUnoTypeToPythonHint(member.type, *m_typeManager, m_moduleName,
                                                        m_pyClassName, m_imports, m_dependentTypes);

        m_buffer.append(getIndent());
        m_buffer.append(safeMemberName);
        m_buffer.append(": ");
        m_buffer.append(memberTypeHint);
        m_buffer.append("\n");
    }

    dedent();
}

void PythonStubGenerator::generateInterface(unoidl::InterfaceTypeEntity* entity)
{
    // Determine and generate the list of base classes.
    OStringBuffer baseClassesStr;
    const auto& bases = entity->getDirectMandatoryBases();

    // The root UNO interface, XInterface, should inherit from ABC. All others
    // will then inherit it transitively.
    if (m_unoName == u"com.sun.star.uno.XInterface" || bases.empty())
    {
        addImportLine("from abc import ABC, abstractmethod"_ostr);
        baseClassesStr.append("ABC");
    }
    else
    {
        addImportLine(
            "from abc import abstractmethod"_ostr); //no need to import ABC as its already inheriting from XInteface
        bool firstBase = true;
        for (const auto& base : bases)
        {
            if (!firstBase)
            {
                baseClassesStr.append(", ");
            }
            baseClassesStr.append(mapUnoTypeToPythonHint(base.name, *m_typeManager, m_moduleName,
                                                         m_pyClassName, m_imports,
                                                         m_dependentTypes));
            firstBase = false;
        }
    }

    // Write the class definition line.
    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    m_buffer.append("(");
    m_buffer.append(baseClassesStr.makeStringAndClear());
    m_buffer.append("):\n");
    indent();

    const auto& attributes = entity->getDirectAttributes();
    const auto& methods = entity->getDirectMethods();
    bool hasNewMembers = !attributes.empty() || !methods.empty();

    if (!hasNewMembers)
    {
        // Add a helper abstract method to explicitly mark this class as abstract.
        m_buffer.append(getIndent());
        m_buffer.append("@abstractmethod\n");
        m_buffer.append(getIndent());
        m_buffer.append("def __uno_abstract__(self) -> None:\n");
        indent();
        m_buffer.append(getIndent());
        m_buffer.append("...\n");
        dedent();
    }
    else
    {
        // Generate properties and methods for members defined directly on this interface.
        for (const auto& attr : attributes)
        {
            OString safeAttrName = getSafePythonIdentifier(codemaker::convertString(attr.name));
            OString attrTypeHint
                = mapUnoTypeToPythonHint(attr.type, *m_typeManager, m_moduleName, m_pyClassName,
                                         m_imports, m_dependentTypes);

            m_buffer.append(getIndent());
            m_buffer.append("@property\n");
            m_buffer.append(getIndent());
            m_buffer.append("@abstractmethod\n");
            m_buffer.append(getIndent());
            m_buffer.append("def ");
            m_buffer.append(safeAttrName);
            m_buffer.append("(self) -> ");
            m_buffer.append(attrTypeHint);
            m_buffer.append(":\n");
            indent();
            m_buffer.append(getIndent());
            m_buffer.append("...\n");
            dedent();

            if (!attr.readOnly)
            {
                m_buffer.append(getIndent());
                m_buffer.append("@");
                m_buffer.append(safeAttrName);
                m_buffer.append(".setter\n");
                m_buffer.append(getIndent());
                m_buffer.append("@abstractmethod\n");
                m_buffer.append(getIndent());
                m_buffer.append("def ");
                m_buffer.append(safeAttrName);
                m_buffer.append("(self, value: ");
                m_buffer.append(attrTypeHint);
                m_buffer.append(") -> None:\n");
                indent();
                m_buffer.append(getIndent());
                m_buffer.append("...\n");
                dedent();
            }
            m_buffer.append("\n");
        }

        for (const auto& method : methods)
        {
            OString safeMethodName = getSafePythonIdentifier(codemaker::convertString(method.name));
            m_buffer.append(getIndent());
            m_buffer.append("@abstractmethod\n");
            m_buffer.append(getIndent());
            m_buffer.append("def ");
            m_buffer.append(safeMethodName);
            m_buffer.append("(self");

            for (const auto& param : method.parameters)
            {
                OString safeParamName
                    = getSafePythonIdentifier(codemaker::convertString(param.name));
                OString paramTypeHint
                    = mapUnoTypeToPythonHint(param.type, *m_typeManager, m_moduleName,
                                             m_pyClassName, m_imports, m_dependentTypes);
                m_buffer.append(", ");
                m_buffer.append(safeParamName);
                m_buffer.append(": ");
                m_buffer.append(paramTypeHint);
            }

            OString returnTypeHint
                = mapUnoTypeToPythonHint(method.returnType, *m_typeManager, m_moduleName,
                                         m_pyClassName, m_imports, m_dependentTypes);

            m_buffer.append(") -> ");
            m_buffer.append(returnTypeHint);
            m_buffer.append(":\n");
            indent();

            if (!method.exceptions.empty())
            {
                m_buffer.append(getIndent());
                m_buffer.append("\"\"\"Raises:\n");
                for (const auto& exc : method.exceptions)
                {
                    OString excHint
                        = mapUnoTypeToPythonHint(exc, *m_typeManager, m_moduleName, m_pyClassName,
                                                 m_imports, m_dependentTypes);
                    m_buffer.append(getIndent());
                    m_buffer.append("    ");
                    m_buffer.append(excHint);
                    m_buffer.append("\n");
                }
                m_buffer.append(getIndent());
                m_buffer.append("\"\"\"\n");
            }

            m_buffer.append(getIndent());
            m_buffer.append("...\n");
            dedent();
            m_buffer.append("\n");
        }
    }
    dedent();
}

void PythonStubGenerator::generateService(unoidl::SingleInterfaceBasedServiceEntity* entity)
{
    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    m_buffer.append(":\n");
    indent();

    const auto& constructors = entity->getConstructors();
    if (constructors.empty())
    {
        // If the service has no constructors defined in IDL, it cannot be created from Python.
        // We generate a `pass` statement to make it a valid empty class.
        m_buffer.append(getIndent());
        m_buffer.append("pass\n");
    }
    else
    {
        // All factory methods will require an XComponentContext, so we resolve its Python type hint once.
        OString contextTypeHint
            = mapUnoTypeToPythonHint(u"com.sun.star.uno.XComponentContext", *m_typeManager,
                                     m_moduleName, m_pyClassName, m_imports, m_dependentTypes);

        // Iterate through each constructor defined in the IDL. Each constructor
        // will become a static factory method in the Python stub.
        for (const auto& ctor : constructors)
        {
            m_buffer.append(getIndent());
            m_buffer.append("@staticmethod\n");

            OString methodName;
            if (ctor.defaultConstructor)
            {
                methodName = "create"_ostr;
            }
            else
            {
                methodName = getSafePythonIdentifier(codemaker::convertString(ctor.name));
            }

            m_buffer.append(getIndent());
            m_buffer.append("def ");
            m_buffer.append(methodName);
            m_buffer.append("(ctx: ");
            m_buffer.append(contextTypeHint);

            // Iterate through the constructor's parameters to build the method signature.
            for (const auto& param : ctor.parameters)
            {
                if (param.rest)
                {
                    addImportLine("from typing import Any"_ostr);
                    m_buffer.append(", *args: Any");
                    break;
                }
                OString safeParamName
                    = getSafePythonIdentifier(codemaker::convertString(param.name));
                OString paramTypeHint
                    = mapUnoTypeToPythonHint(param.type, *m_typeManager, m_moduleName,
                                             m_pyClassName, m_imports, m_dependentTypes);
                m_buffer.append(", ");
                m_buffer.append(safeParamName);
                m_buffer.append(": ");
                m_buffer.append(paramTypeHint);
            }

            // The return type of the factory method is the service's primary interface.
            OString returnTypeHint
                = mapUnoTypeToPythonHint(entity->getBase(), *m_typeManager, m_moduleName,
                                         m_pyClassName, m_imports, m_dependentTypes);

            m_buffer.append(") -> ");
            m_buffer.append(returnTypeHint);
            m_buffer.append(":\n");
            indent();

            // If the constructor can raise exceptions, document them in a docstring.
            // This is the standard Pythonic way to convey this information in stubs.
            if (!ctor.exceptions.empty())
            {
                m_buffer.append(getIndent());
                m_buffer.append("\"\"\"Raises:\n");
                for (const auto& exc : ctor.exceptions)
                {
                    OString excHint
                        = mapUnoTypeToPythonHint(exc, *m_typeManager, m_moduleName, m_pyClassName,
                                                 m_imports, m_dependentTypes);
                    m_buffer.append(getIndent());
                    m_buffer.append("    ");
                    m_buffer.append(excHint);
                    m_buffer.append("\n");
                }
                m_buffer.append(getIndent());
                m_buffer.append("\"\"\"\n");
            }

            m_buffer.append(getIndent());
            m_buffer.append("...\n");
            dedent();
            m_buffer.append("\n");
        }
    }
    dedent();
}

void PythonStubGenerator::generateSingleton(unoidl::InterfaceBasedSingletonEntity* entity)
{
    // A singleton is represented as a class with a static 'get' method.
    // The class itself is just a namespace for this method.
    m_buffer.append("class ");
    m_buffer.append(m_pySafeClassName);
    m_buffer.append(":\n");
    indent();

    // The get() method is static. This is indicated by the @staticmethod decorator.
    m_buffer.append(getIndent());
    m_buffer.append("@staticmethod\n");

    m_buffer.append(getIndent());
    m_buffer.append("def get(");

    // It takes the component context as a parameter.
    OString contextTypeHint
        = mapUnoTypeToPythonHint(u"com.sun.star.uno.XComponentContext", *m_typeManager,
                                 m_moduleName, m_pyClassName, m_imports, m_dependentTypes);
    m_buffer.append("ctx: ");
    m_buffer.append(contextTypeHint);

    // It returns an instance of its base interface.
    OString returnTypeHint = mapUnoTypeToPythonHint(entity->getBase(), *m_typeManager, m_moduleName,
                                                    m_pyClassName, m_imports, m_dependentTypes);

    m_buffer.append(") -> ");
    m_buffer.append(returnTypeHint);
    m_buffer.append(":\n");
    indent();

    // A singleton's get() method can raise a DeploymentException if it fails.
    // We document this in the docstring.
    OString deploymentExcHint
        = mapUnoTypeToPythonHint(u"com.sun.star.uno.DeploymentException", *m_typeManager,
                                 m_moduleName, m_pyClassName, m_imports, m_dependentTypes);

    m_buffer.append(getIndent());
    m_buffer.append("\"\"\"Raises:\n");
    m_buffer.append(getIndent());
    m_buffer.append("    ");
    m_buffer.append(deploymentExcHint);
    m_buffer.append("\n");
    m_buffer.append(getIndent());
    m_buffer.append("\"\"\"\n");

    m_buffer.append(getIndent());
    m_buffer.append("...\n");
    dedent();
    dedent();
}

// Functions responsible for the indentation and dedentation of the python stubs.
void PythonStubGenerator::indent() { m_indentLevel++; }
void PythonStubGenerator::dedent()
{
    if (m_indentLevel > 0)
        m_indentLevel--;
}
OString PythonStubGenerator::getIndent() const
{
    OStringBuffer sb;
    for (int i = 0; i < m_indentLevel; ++i)
        sb.append("    "); // add a single time indentation
    return sb.makeStringAndClear();
}

void produce(OUString const& name, rtl::Reference<TypeManager> const& manager,
             codemaker::GeneratedTypeSet& generated, PythonOptions const& options)
{
    // On the very first call (the root call), clear the __init__.pyi cache.
    // This is a simple way to reset the state for a new pythonmaker run if it were ever
    // called multiple times in the same process.

    if (name.isEmpty())
    {
        g_initPyiCache.clear();
        rtl::Reference<unoidl::MapCursor> cursor = manager->getManager()->createCursor(OUString());
        OUString memberName;
        while (cursor->getNext(&memberName).is())
        {
            produce(memberName, manager, generated, options);
        }
        return;
    }

    if (generated.contains(u2b(name)))
    {
        return;
    }

    if (!manager->foundAtPrimaryProvider(name))
    {
        return;
    }

    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = manager->getSort(name, &entity, &cursor);

    switch (sort)
    {
        case codemaker::UnoType::Sort::AccumulationBasedService:
        case codemaker::UnoType::Sort::ServiceBasedSingleton:
            // These types do not have a direct Python stub equivalent.
            // Mark as processed and do not generate a file.
            generated.add(u2b(name));
            return; // Exit the function for this type
        default:
            break;
    }

    if (sort == codemaker::UnoType::Sort::Module)
    {
        ensureInitPyi(options.getOption("-O"_ostr), name);
        generated.add(u2b(name)); // Mark module as processed
        OUString memberNameInModule;
        if (cursor.is())
        { // Ensure cursor is valid
            while (cursor->getNext(&memberNameInModule).is())
            {
                produce(name + u"."_ustr + memberNameInModule, manager, generated, options);
            }
        }
    }
    else
    {
        sal_Int32 lastDot = name.lastIndexOf('.');
        if (lastDot != -1)
        {
            ensureInitPyi(options.getOption("-O"_ostr), name.copy(0, lastDot));
        }
        PythonStubGenerator generator(name, manager, generated, options);
        generator.generate();
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

#include <codemaker/typemanager.hxx>

/**
 * Generates Rust source files from UNO type definitions
 */
class RustFile
{
public:
    RustFile(std::string_view directory, std::string_view typeName);

    std::string getPath() const;
    void openFile();
    void closeFile();

    // Static methods for project setup
    static void createLibFile(const std::filesystem::path& outputDir);
    static void createCargoFile(const std::filesystem::path& outputDir,
                                const std::string& crateName = "rustmaker");
    static void createGeneratedModFile(const std::filesystem::path& outputDir);
    static void finalizeModFiles(const std::filesystem::path& outputDir);

    // Code generation helpers
    RustFile& beginBlock();
    RustFile& endBlock();
    RustFile& beginLine();
    RustFile& extraIndent();
    RustFile& append(std::string_view item);
    RustFile& append(std::u16string_view item);
    RustFile& endLine();

private:
    // Complex path resolution - walks up directory tree to find project root
    std::filesystem::path findOutputRoot();

    // Module management - handles mod.rs creation and declarations
    void createModFiles();
    void addFileToModDeclaration();
    void ensureModFileExists(const std::filesystem::path& dirPath);
    static void addModuleDeclaration(const std::filesystem::path& modFile,
                                     const std::string& moduleName);

    // Converts UNO type name to file path (dots become path separators)
    static std::filesystem::path createFilePath(std::string_view dir, std::string_view type);

private:
    std::filesystem::path m_filePath;
    std::ofstream m_fileStream;
    int m_indentLevel;

    // Helper function for recursive directory processing
    static void processDirectoryRecursively(const std::filesystem::path& currentDir);
};

/**
 * Generates C++ bridge files for UNO opaque functions
 */
class CppFile
{
public:
    CppFile(std::string_view directory, std::string_view typeName);
    CppFile(std::string_view directory, std::string_view typeName, std::string_view extension);

    std::string getPathString() const;
    std::filesystem::path getPath() const;
    std::string getExtension() const;
    void openFile();
    void openFileAppend(); // Open for appending (continue writing)
    void closeFile();

    // Code generation helpers
    CppFile& beginBlock();
    CppFile& endBlock();
    CppFile& beginLine();
    CppFile& extraIndent();
    CppFile& append(std::string_view item);
    CppFile& append(std::u16string_view item);
    CppFile& endLine();

    // C++ specific helpers
    CppFile& writeIncludes(const std::vector<std::string>& includes);
    CppFile& writeNamespaceBegin(const std::vector<std::string>& namespaces);
    CppFile& writeNamespaceEnd(const std::vector<std::string>& namespaces);

private:
    static std::filesystem::path createCppFilePath(std::string_view dir, std::string_view type,
                                                   std::string_view extension = ".hxx");

private:
    std::filesystem::path m_filePath;
    std::ofstream m_fileStream;
    int m_indentLevel;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

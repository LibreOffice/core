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
#include <string>
#include <string_view>

#include <codemaker/typemanager.hxx>

class CSharpFile
{
public:
    CSharpFile(std::string_view directory, std::string_view typeName)
        : m_filePath(createFilePath(directory, typeName))
    {
    }

public:
    std::string getPath() const { return m_filePath.string(); }

    void openFile()
    {
        std::filesystem::create_directories(m_filePath.parent_path());
        m_fileStream.open(m_filePath, std::fstream::out | std::fstream::trunc);
        m_indentLevel = 0;
    }
    void closeFile() { m_fileStream.close(); }

    CSharpFile& beginBlock()
    {
        beginLine();
        append("{");
        endLine();
        ++m_indentLevel;
        return *this;
    }
    CSharpFile& endBlock()
    {
        --m_indentLevel;
        beginLine();
        append("}");
        endLine();
        return *this;
    }

    CSharpFile& beginLine()
    {
        for (int i = 0; i < m_indentLevel; i++)
        {
            m_fileStream << "    ";
        }
        return *this;
    }

    CSharpFile& extraIndent()
    {
        m_fileStream << "    ";
        return *this;
    }

    CSharpFile& append(std::string_view item)
    {
        m_fileStream << item;
        return *this;
    }

    CSharpFile& append(std::u16string_view item)
    {
        m_fileStream << u2b(item);
        return *this;
    }

    CSharpFile& endLine()
    {
        m_fileStream << '\n';
        return *this;
    }

private:
    static std::filesystem::path createFilePath(std::string_view dir, std::string_view type)
    {
        std::string subdir(type);
        for (char& c : subdir)
            if (c == '.')
                c = '/';

        std::filesystem::path path(dir);
        path /= subdir + ".cs";
        return path;
    }

private:
    std::filesystem::path m_filePath;
    std::ofstream m_fileStream;
    int m_indentLevel = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

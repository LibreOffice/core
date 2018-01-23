/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>

// #define DEBUG_FILEPICKER_IPC

namespace rtl
{
class OUString;
}
class QString;

enum class Commands : uint16_t
{
    SetTitle,
    SetWinId,
    Execute,
    SetMultiSelectionMode,
    SetDefaultName,
    SetDisplayDirectory,
    GetDisplayDirectory,
    GetSelectedFiles,
    AppendFilter,
    SetCurrentFilter,
    GetCurrentFilter,
    SetValue,
    GetValue,
    EnableControl,
    SetLabel,
    GetLabel,
    AddCheckBox,
    Initialize,
    Quit,
    EnablePickFolderMode,
};

inline std::vector<char> readIpcStringArg(std::istream& stream)
{
    uint32_t length = 0;
    stream >> length;
    stream.ignore(); // skip space separator
    std::vector<char> buffer(length, '\0');
    stream.read(buffer.data(), length);
    return buffer;
}

void readIpcArg(std::istream& stream, rtl::OUString& string);
void readIpcArg(std::istream& stream, QString& string);
void readIpcArg(std::istream& stream, css::uno::Sequence<rtl::OUString>& seq);

inline void readIpcArg(std::istream& stream, Commands& value)
{
    uint16_t v = 0;
    stream >> v;
    stream.ignore(); // skip space
    value = static_cast<Commands>(v);
}

void readIpcArg(std::istream&, sal_Bool) = delete;

inline void readIpcArg(std::istream& stream, bool& value)
{
    stream >> value;
    stream.ignore(); // skip space
}

inline void readIpcArg(std::istream& stream, sal_Int16& value)
{
    stream >> value;
    stream.ignore(); // skip space
}

inline void readIpcArg(std::istream& stream, sal_uIntPtr& value)
{
    stream >> value;
    stream.ignore(); // skip space
}

inline void readIpcArgs(std::istream& /*stream*/)
{
    // end of arguments, nothing to do
}

template <typename T, typename... Args>
inline void readIpcArgs(std::istream& stream, T& arg, Args&... args)
{
    readIpcArg(stream, arg);
    readIpcArgs(stream, args...);
}

void sendIpcArg(std::ostream& stream, const rtl::OUString& string);
void sendIpcArg(std::ostream& stream, const QString& string);

inline void sendIpcStringArg(std::ostream& stream, uint32_t length, const char* string)
{
    stream << length << ' ';
    stream.write(string, length);
    stream << ' ';
}

inline void sendIpcArg(std::ostream& stream, Commands value)
{
    stream << static_cast<uint16_t>(value) << ' ';
}

void sendIpcArg(std::ostream&, sal_Bool) = delete;

inline void sendIpcArg(std::ostream& stream, bool value) { stream << value << ' '; }

inline void sendIpcArg(std::ostream& stream, sal_Int16 value) { stream << value << ' '; }

inline void sendIpcArg(std::ostream& stream, sal_uIntPtr value) { stream << value << ' '; }

inline void sendIpcArgsImpl(std::ostream& stream)
{
    // end of arguments, flush stream
    stream << std::endl;
}

template <typename T, typename... Args>
inline void sendIpcArgsImpl(std::ostream& stream, const T& arg, const Args&... args)
{
    sendIpcArg(stream, arg);
    sendIpcArgsImpl(stream, args...);
}

template <typename T, typename... Args>
inline void sendIpcArgs(std::ostream& stream, const T& arg, const Args&... args)
{
    sendIpcArgsImpl(stream, arg, args...);
#ifdef DEBUG_FILEPICKER_IPC
    std::cerr << "IPC MSG: ";
    sendIpcArgsImpl(std::cerr, arg, args...);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

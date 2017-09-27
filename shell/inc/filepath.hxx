/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SHELL_INC_INTERNAL_FILEPATH_HXX
#define INCLUDED_SHELL_INC_INTERNAL_FILEPATH_HXX


// typedefs to allow using Unicode paths on Windows


#include <string>

#if defined _WIN32
typedef wchar_t Filepath_char_t;
typedef std::wstring Filepath_t;
#else
typedef char Filepath_char_t;
typedef std::string Filepath_t;
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

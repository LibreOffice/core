///
/// This file is part of the LibreOffice project.
///
/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at http://mozilla.org/MPL/2.0/.
///
/// This file incorporates work covered by the following license notice:
///
///   Licensed to the Apache Software Foundation (ASF) under one or more
///   contributor license agreements. See the NOTICE file distributed
///   with this work for additional information regarding copyright
///   ownership. The ASF licenses this file to you under the Apache
///   License, Version 2.0 (the "License"); you may not use this file
///   except in compliance with the License. You may obtain a copy of
///   the License at http://www.apache.org/licenses/LICENSE-2.0 .
///

#include <types.mk>
using namespace gb::types;

// GNU make specific setup
static const Command SHELL;


// gbuild root directorys
static Path SRCDIR;
static Path WORKDIR;

// Expected from configure/environment
static const Integer OSL_DEBUG_LEVEL;
static const List<Path> SOLARINC;
static const Path GBUILDDIR;
static const Path JAVA_HOME;
static const Path UPD;
static const String LIBXML_CFLAGS;
static const String OS;
static const Bool DEBUG;

// gbuild global variables derived from the configure/environment
// some of these are defined per platform
namespace gb
{
    /// building with generated dependencies
    static const Bool FULLDEPS;
    /// command to run awk scripts
    static const Command AWK;
    /// command to compile c source files
    static const Command CC;
    /// command to compile c++ source files
    static const Command CXX;
    /// command to process input with a gcc compatible preprocessor
    static const Command GCCP;
    /// command to link objects on the microsoft toolchain
    static const Command LINK;
    /// command to create a unique temporary file
    static const Command MKTEMP;
    /// debuglevel:
    /// 0=no debugging,
    /// 1=non-product build,
    /// 2=debugging build (either product or nonproduct)
    static const Integer DEBUGLEVEL;
    /// compiler specific optimization flags
    static const List<String> COMPILEROPTFLAGS;
    /// default c compilation compiler flags
    static const List<String> CFLAGS;
    /// compiler specific defines
    static const List<String> COMPILERDEFS;
    /// cpu-specific default defines
    static const List<String> CPUDEFS;
    /// default c++ compilation compiler flags
    static const List<String> CXXFLAGS;
    /// platform- and compilerin dependent default defines
    static const List<String> GLOBALDEFS;
    /// default objective c++ compilation compiler flags
    static const List<String> OBJCXXFLAGS;
    /// platfomspecific default defines
    static const List<String> OSDEFS;
    /// ?
    static const Path SDKDIR;
};

// PTHREAD_CFLAGS (Linux)
// SYSTEM_ICU (Linux)
// SYSTEM_JPEG (Linux)
// SYSTEM_LIBXML (Linux)
// USE_SYSTEM_STL (Linux)

/* vim: set filetype=cpp : */

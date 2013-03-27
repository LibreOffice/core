/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// Methods used by all of executables

#ifndef INCLUDED_L10NTOOLS_SOURCE_COMMON_HXX
#define INCLUDED_L10NTOOLS_SOURCE_COMMON_HXX

#include "sal/config.h"

#include <iostream>
#include <rtl/string.hxx>
#include "po.hxx"

namespace common {

//result type of handleArguments()
struct HandledArgs
{
    OString m_sInputFile;
    OString m_sOutputFile;
    OString m_sMergeSrc;
    OString m_sLanguage;
    bool m_bMergeMode;
    HandledArgs()
        : m_sInputFile( OString() )
        , m_sOutputFile( OString() )
        , m_sMergeSrc( OString() )
        , m_sLanguage( OString() )
        , m_bMergeMode( false )
    {}
};

bool handleArguments(int argc, char * argv[], HandledArgs& o_aHandledArgs);

void writeUsage(const OString& rName, const OString& rFileType);

void writePoEntry(
    const OString& rExecutable, PoOfstream& rPoStream, const OString& rSourceFile,
    const OString& rResType, const OString& rGroupId, const OString& rLocalId,
    const OString& rHelpText, const OString& rText, const PoEntry::TYPE eType = PoEntry::TTEXT );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

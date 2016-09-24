/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/main.h"

#include "common.hxx"
#include "propmerge.hxx"

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    common::HandledArgs aArgs;
    if( !common::handleArguments(argc, argv, aArgs) )
    {
        common::writeUsage("propex","*.properties");
        return 1;
    }

    PropParser aParser(
        aArgs.m_sInputFile, aArgs.m_sLanguage, aArgs.m_bMergeMode );
    if( !aParser.isInitialized() )
    {
        return 1;
    }
    if( aArgs.m_bMergeMode )
    {
        aParser.Merge( aArgs.m_sMergeSrc, aArgs.m_sOutputFile );
    }
    else
    {
        aParser.Extract( aArgs.m_sOutputFile );
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

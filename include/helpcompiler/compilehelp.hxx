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

#ifndef INCLUDED_HELPCOMPILER_COMPILEHELP_HXX
#define INCLUDED_HELPCOMPILER_COMPILEHELP_HXX

#include <sal/config.h>

#include <string_view>

#include <sal/types.h>

#if defined(HELPLINKER_DLLIMPLEMENTATION)
#define HELPLINKER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define HELPLINKER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define HELPLINKER_DLLPRIVATE SAL_DLLPRIVATE

#include <rtl/ustring.hxx>

enum class HelpProcessingErrorClass
{
    NONE,
    General,       // Missing files, options etc.
    XmlParsing     // Errors thrown by libxml
};

struct HelpProcessingErrorInfo
{
    HelpProcessingErrorClass   m_eErrorClass;
    OUString                   m_aErrorMsg;
    OUString                   m_aXMLParsingFile;
    sal_Int32                  m_nXMLParsingLine;

    HelpProcessingErrorInfo()
        : m_eErrorClass( HelpProcessingErrorClass::NONE )
        , m_nXMLParsingLine( -1 )
    {}

    HelpProcessingErrorInfo& operator=( const struct HelpProcessingException& e );
};


// Returns true in case of success, false in case of error
HELPLINKER_DLLPUBLIC bool compileExtensionHelp
(
     const OUString& aOfficeHelpPath,
    std::u16string_view aExtensionName,
    std::u16string_view aExtensionLanguageRoot,
    sal_Int32 nXhpFileCount, const OUString* pXhpFiles,
    std::u16string_view aDestination,
    HelpProcessingErrorInfo& o_rHelpProcessingErrorInfo
);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

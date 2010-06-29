/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef COMPILE_HXX
#define COMPILE_HXX

#include "sal/types.h"

#if defined(HELPLINKER_DLLIMPLEMENTATION)
#define HELPLINKER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define HELPLINKER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define HELPLINKER_DLLPRIVATE SAL_DLLPRIVATE


//#include <helplinkerdllapi.h>
#include <rtl/ustring.hxx>

enum HelpProcessingErrorClass
{
    HELPPROCESSING_NO_ERROR,
    HELPPROCESSING_GENERAL_ERROR,       // Missing files, options etc.
    HELPPROCESSING_INTERNAL_ERROR,      // Unexpected problems
    HELPPROCESSING_XMLPARSING_ERROR     // Errors thrown by libxml
};

struct HelpProcessingErrorInfo
{
    HelpProcessingErrorClass        m_eErrorClass;
    rtl::OUString                   m_aErrorMsg;
    rtl::OUString                   m_aXMLParsingFile;
    sal_Int32                       m_nXMLParsingLine;

    HelpProcessingErrorInfo( void )
        : m_eErrorClass( HELPPROCESSING_NO_ERROR )
        , m_nXMLParsingLine( -1 )
    {}

    HelpProcessingErrorInfo& operator=( const struct HelpProcessingException& e );
};


// Returns true in case of success, false in case of error
HELPLINKER_DLLPUBLIC bool compileExtensionHelp
(
     const rtl::OUString& aOfficeHelpPath,
    const rtl::OUString& aExtensionName,
    const rtl::OUString& aExtensionLanguageRoot,
    sal_Int32 nXhpFileCount, const rtl::OUString* pXhpFiles,
    const rtl::OUString& aDestination,
    HelpProcessingErrorInfo& o_rHelpProcessingErrorInfo
);

#endif

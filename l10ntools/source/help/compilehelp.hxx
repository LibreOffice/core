/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

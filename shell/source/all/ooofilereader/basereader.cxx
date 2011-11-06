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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include "internal/basereader.hxx"

#ifndef XML_PARSER_HXX_INCLUDED
#include "internal/xml_parser.hxx"
#endif

#include "assert.h"
#include <memory>

/**  constructor of CBaseReader.
*/
CBaseReader::CBaseReader(const std::string& DocumentName):
m_ZipFile( DocumentName )
{
}

//------------------------------
//
//------------------------------

CBaseReader::CBaseReader(void * sw, zlib_filefunc_def* fa):
m_ZipFile( sw , fa )
{
}

//------------------------------
//
//------------------------------

CBaseReader::~CBaseReader()
{
}

//------------------------------
//
//------------------------------

void CBaseReader::start_document()
{
}

//------------------------------
//
//------------------------------

void CBaseReader::end_document()
{
}

/** Read interested tag content into respective structure then start parsing process.
    @param ContentName
    the xml file name in the zipped document which we interest.
*/
void CBaseReader::Initialize( const std::string& ContentName)
{
    try
    {
        if (m_ZipContent.empty())
            m_ZipFile.GetUncompressedContent( ContentName, m_ZipContent );

        xml_parser parser;
        parser.set_document_handler(this);  // pass current reader as reader to the sax parser
        parser.parse(&m_ZipContent[0], m_ZipContent.size());
    }
    catch(std::exception&
    #if OSL_DEBUG_LEVEL > 0
        ex
    #endif
        )
    {
        ENSURE( false, ex.what() );
    }
    catch(...)
    {
        ENSURE(false, "Unknown error");
    }
}

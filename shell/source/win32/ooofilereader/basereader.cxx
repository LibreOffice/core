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

#undef OSL_DEBUG_LEVEL


#include <osl/diagnose.h>

#include "internal/basereader.hxx"

#include "internal/xml_parser.hxx"

#include "assert.h"
#include <memory>

/**  constructor of CBaseReader.
*/
CBaseReader::CBaseReader(const std::string& DocumentName):
m_ZipFile( DocumentName )
{
}

//------------------------------

CBaseReader::CBaseReader(StreamInterface * sw):
m_ZipFile( sw )
{
}

//------------------------------

CBaseReader::~CBaseReader()
{
}

//------------------------------

void CBaseReader::start_document()
{
}

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

        if (!m_ZipContent.empty())
        {
            xml_parser parser;
            parser.set_document_handler(this);  // pass current reader as reader to the sax parser
            parser.parse(&m_ZipContent[0], m_ZipContent.size());
        }
    }
    catch(std::exception&
    #if OSL_DEBUG_LEVEL > 0
        ex
    #endif
        )
    {
        OSL_ENSURE( false, ex.what() );
    }
    catch(...)
    {
        OSL_ENSURE(false, "Unknown error");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

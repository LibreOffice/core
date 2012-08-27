/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <basereader.hxx>

#include <xml_parser.hxx>

#include <stdio.h>  // SEEK_SET

#include <assert.h>
#include <memory>

/**  constructor of CBaseReader.
*/
CBaseReader::CBaseReader(const Filepath_t& DocumentName):
m_ZipFile( DocumentName )
{
}


CBaseReader::CBaseReader(StreamInterface * sw):
m_ZipFile( sw )
{
}


CBaseReader::~CBaseReader()
{
}


void CBaseReader::start_document()
{
}


void CBaseReader::end_document()
{
}

/** Read interested tag content into respective structure then start parsing process.
    @param ContentName
    the xml file name in the zipped document which we interest.

    For zipped ODF, reads only the named sub-document (meta.xml or
    content.xml) and parses it.

    For flat ODF (the document is a single XML file - not a zip), the
    ContentName parameter is ignored: the entire document is parsed in one
    pass. The readers' chooseTagReader dispatch already returns CDummyTag
    for tags it does not recognize, so feeding the whole flat-ODF document
    is safe - the metainfo reader picks out tags under <office:meta>, the
    content reader picks out body paragraphs under <office:body>, and
    everything else is no-op.
*/
void CBaseReader::Initialize( const std::string& ContentName)
{
    try
    {
        if (m_ZipContent.empty())
        {
            if (m_ZipFile.IsValid())
            {
                m_ZipFile.GetUncompressedContent( ContentName, m_ZipContent );
            }
            else if (StreamInterface* stream = m_ZipFile.GetStream())
            {
                // Flat ODF: slurp the whole document. Zip detection has
                // moved the read cursor, seek back to start first.
                stream->sseek(0, SEEK_SET);
                unsigned char buf[65536];
                for (;;)
                {
                    unsigned long n = stream->sread(buf, sizeof(buf));
                    if (n == 0)
                        break;
                    m_ZipContent.insert(m_ZipContent.end(),
                                        reinterpret_cast<char*>(buf),
                                        reinterpret_cast<char*>(buf) + n);
                    if (n < sizeof(buf))
                        break;
                }
            }
        }

        if (!m_ZipContent.empty())
        {
            xml_parser parser;
            parser.set_document_handler(this);  // pass current reader as reader to the sax parser
            parser.parse(m_ZipContent.data(), m_ZipContent.size(), true/*IsFinal*/);
        }
    }
    catch(std::exception&)
    {
        // OSL_ENSURE( false, ex.what() );
    }
    catch(...)
    {
        // OSL_ENSURE(false, "Unknown error");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

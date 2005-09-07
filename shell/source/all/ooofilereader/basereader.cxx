/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basereader.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:43:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef BASEREADER_HXX_INCLUDED
#include "internal/basereader.hxx"
#endif

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
    catch(std::exception& ex)
    {
        ENSURE( false, ex.what() );
    }
    catch(...)
    {
        ENSURE(false, "Unknown error");
    }
}

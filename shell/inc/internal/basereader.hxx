/*************************************************************************
 *
 *  $RCSfile: basereader.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-09-08 14:22:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef BASEREADER_HXX_INCLUDED
#define BASEREADER_HXX_INCLUDED

#ifndef GLOBAL_HXX_INCLUDED
#include "internal/global.hxx"
#endif

#ifndef TYPES_HXX_INCLUDED
#include "internal/types.hxx"
#endif

#ifndef CONFIG_HXX_INCLUDED
#include "internal/config.hxx"
#endif

#ifndef UTILITIES_HXX_INCLUDED
#include "internal/utilities.hxx"
#endif

#ifndef I_XML_PARSER_EVENT_HANDLER_HXX_INCLUDED
#include "internal/i_xml_parser_event_handler.hxx"
#endif

#ifndef XML_PARSER_HXX_INCLUDED
#include "internal/xml_parser.hxx"
#endif

#ifndef ZIPFILE_HXX_INCLUDED
#include "internal/zipfile.hxx"
#endif

class CBaseReader : public i_xml_parser_event_handler
{
public:
    virtual ~CBaseReader();

protected: // protected because its only an implementation relevant class
    CBaseReader( const std::string& DocumentName );

    virtual void start_document();

    virtual void end_document();

    virtual void start_element(
        const std::wstring& raw_name,
        const std::wstring& local_name,
        const XmlTagAttributes_t& attributes) = 0;

    virtual void end_element(
        const std::wstring& raw_name, const std::wstring& local_name) = 0;

    virtual void characters(const std::wstring& character) = 0;

    virtual void ignore_whitespace(const std::wstring& /*whitespaces*/){};

    virtual void processing_instruction(
        const std::wstring& /*target*/, const std::wstring& /*data*/){};

    virtual void comment(const std::wstring& /*comment*/){};

    void Initialize( const std::string& /*ContentName*/);

private:
    ZipFile        m_ZipFile;
    ZipFile::ZipContentBuffer_t m_ZipContent;
};

#endif
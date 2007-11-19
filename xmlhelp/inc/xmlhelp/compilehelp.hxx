/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: compilehelp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 12:58:27 $
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

#ifndef COMPILE_HXX
#define COMPILE_HXX

#include <xmlhelp/helplinkerdllapi.h>
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
    const rtl::OUString& aExtensionName,
    const rtl::OUString& aExtensionLanguageRoot,
    sal_Int32 nXhpFileCount, const rtl::OUString* pXhpFiles,
    HelpProcessingErrorInfo& o_rHelpProcessingErrorInfo
);

#endif

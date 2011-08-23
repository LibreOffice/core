/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*****************************************************************************
* Change History
* <<Date>> <<Name of editor>> <<Description>>
2005/2		draft code for implementation of chart stream helpers
****************************************************************************/

/**
 * @file
 *  For LWP filter architecture prototype
*/

#include "xfchartstreamtools.hxx"
#include "xfdrawchart.hxx"
#include "xfparagraph.hxx"

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

/**
* @short   Parse the xml stream
* @param  rInpStream the xml stream
* @param   aHandler	xml handler
*/
void XFChartStreamTools::ParseStream(SvStorageStreamRef rInpStream, Reference<XDocumentHandler> aHandler)
{
    //Create parser
    uno::Reference< xml::sax::XParser > xParser;
    {
        uno::Reference< lang::XMultiServiceFactory>
            xServiceFactory( ::comphelper::getProcessServiceFactory() );
        if( !xServiceFactory.is() )
        {
            assert(false);
            return;
        }
        xParser = uno::Reference< xml::sax::XParser > (
            xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" )),
            uno::UNO_QUERY );

        if( ! xParser.is() )
        {
            assert(false);
            return;
        }
    }

    xParser->setDocumentHandler( aHandler );
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = rInpStream->GetXInputStream();
    xParser->parseStream( aParserInput );
}

/**
* @short	Parse the xml stream, content.xml & styles.xml for chart
* @param	rStyleStream	styles xml stream
* @param	rCntStream	the content xml stream
* @param   pOutputStream the output stream
*/
void XFChartStreamTools::ParseStream(SvStorageStreamRef rCntStream,IXFStream* pOutputStream)
{
    Reference<XDocumentHandler> pHandler( new XFChartXMLHandler(pOutputStream) );
    ParseStream(rCntStream,pHandler);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

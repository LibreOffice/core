/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xml_import.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:08:53 $
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
#if ! defined _XMLSCRIPT_XML_IMPORT_HXX_
#define _XMLSCRIPT_XML_IMPORT_HXX_

#include "com/sun/star/xml/input/XRoot.hpp"
#include "com/sun/star/xml/sax/XExtendedDocumentHandler.hpp"


namespace xmlscript
{

/*##############################################################################

    IMPORTING

##############################################################################*/

/** Creates a document handler to be used for SAX1 parser that can handle
    namespaces.  Namespace URI are mapped to integer ids for performance.
    Implementing the XImporter interface, you will get a startRootElement()
    for the root element of your XML document and subsequent
    startChildElement() callbacks for each sub element.
    Namespaces of tags are identified by their integer value.

    @param xRoot
           initial object being called for root context
    @param bSingleThreadedUse
           flag whether context management is synchronized.
    @return
            document handler for parser
*/
::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL createDocumentHandler(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::input::XRoot > const & xRoot,
    bool bSingleThreadedUse = true )
    SAL_THROW( () );

}

#endif

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

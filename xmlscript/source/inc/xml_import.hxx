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
#ifndef _XMLSCRIPT_XML_IMPORT_HXX_
#define _XMLSCRIPT_XML_IMPORT_HXX_

#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>

#include "com/sun/star/xml/input/XRoot.hpp"
#include "com/sun/star/xml/sax/XExtendedDocumentHandler.hpp"

namespace xmlscript
{


//  IMPORTING

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
    SAL_THROW(());

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_XML_HXX
#define INCLUDED_CONFIGMGR_XML_HXX

#include "sal/config.h"

#include <set>

#include "libxml/parser.h"
#include "sal/types.h"

namespace rtl { class OUString; }

namespace configmgr {

struct Data;

namespace xml {

typedef std::set< rtl::OUString > Dependencies;

struct XmlDoc: private boost::noncopyable {
    xmlDocPtr doc;

    explicit XmlDoc(xmlDocPtr theDoc): doc(theDoc) {}

    ~XmlDoc() { xmlFreeDoc(doc); }
};

bool decodeXml(
    rtl::OUString const & encoded, sal_Int32 begin, sal_Int32 end,
    rtl::OUString * decoded);

xmlDocPtr parseXmlFile(rtl::OUString const & url);

void parseXcsFile(rtl::OUString const & url, int layer, Data * data);

void parseXcuFile(rtl::OUString const & url, int layer, Data * data);

bool parseXcdFile(
    xmlDocPtr doc, int layer, Dependencies const & dependencies, Data * data);

void parseModFile(rtl::OUString const & url, Data * data);

void writeModFile(rtl::OUString const & url, Data const & data);

}

}

#endif

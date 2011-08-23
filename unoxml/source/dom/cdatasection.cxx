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

#include "cdatasection.hxx"

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

namespace DOM
{
    CCDATASection::CCDATASection(const xmlNodePtr aNodePtr)
    {
        m_aNodeType = NodeType_CDATA_SECTION_NODE;
        init_text(aNodePtr);
    }

    void SAL_CALL CCDATASection::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        Reference< XExtendedDocumentHandler > xExtended(i_xHandler, UNO_QUERY);
        if (xExtended.is()) {
            xExtended->startCDATA();
            i_xHandler->characters(getData());
            xExtended->endCDATA();
        }
    }

    OUString SAL_CALL CCDATASection::getNodeName()throw (RuntimeException)
    {
        return OUString::createFromAscii("#cdata-section");
    }

    OUString SAL_CALL CCDATASection::getNodeValue() throw (RuntimeException)
    {
        return CCharacterData::getData();
    }
}

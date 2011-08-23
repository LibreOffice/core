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

#include "comment.hxx"

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

namespace DOM
{
    CComment::CComment(const xmlNodePtr aNodePtr)
    {
        m_aNodeType = NodeType_COMMENT_NODE;
        init_node(aNodePtr);
    }

    void SAL_CALL CComment::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        Reference< XExtendedDocumentHandler > xExtended(i_xHandler, UNO_QUERY);
        if (xExtended.is()) {
            xExtended->comment(getData());
        }
    }

    OUString SAL_CALL CComment::getNodeName()throw (RuntimeException)
    {
        return OUString::createFromAscii("#comment");
    }

    OUString SAL_CALL CComment::getNodeValue() throw (RuntimeException)
    {
        return CCharacterData::getData();
    }
}

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

#include "processinginstruction.hxx"
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <string.h>

namespace DOM
{
    CProcessingInstruction::CProcessingInstruction(const xmlNodePtr aNodePtr)
    {
        m_aNodeType = NodeType_PROCESSING_INSTRUCTION_NODE;
        init_node(aNodePtr);
    }

    void SAL_CALL CProcessingInstruction::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        Reference< XExtendedDocumentHandler > xExtended(i_xHandler, UNO_QUERY);
        if (xExtended.is()) {
            xExtended->processingInstruction(getTarget(), getData());
        }
    }

    /**
    The content of this processing instruction.
    */
    OUString SAL_CALL CProcessingInstruction::getData() throw (RuntimeException)
    {
        // XXX
        return OUString();
    }

    /**
    The target of this processing instruction.
    */
    OUString SAL_CALL CProcessingInstruction::getTarget() throw (RuntimeException)
    {
        // XXX
        return OUString();
    }


    /**
    The content of this processing instruction.
    */
  void SAL_CALL CProcessingInstruction::setData(const OUString& /*data*/) throw (RuntimeException, DOMException)
    {
        // XXX
    }


    OUString SAL_CALL CProcessingInstruction::getNodeName()throw (RuntimeException)
    {
       OUString aName;
        if (m_aNodePtr != NULL)
        {
            const xmlChar* xName = m_aNodePtr->name;
            aName = OUString((sal_Char*)xName, strlen((char*)xName), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }

    OUString SAL_CALL CProcessingInstruction::getNodeValue() throw (RuntimeException)
    {
        return getData();
    }
}

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

#include <text.hxx>


namespace DOM
{
    CText::CText(CDocument const& rDocument,
            NodeType const& reNodeType, xmlNodePtr const& rpNode)
        : CText_Base(rDocument, reNodeType, rpNode)
    {
    }

    CText::CText(CDocument const& rDocument, xmlNodePtr const pNode)
        : CText_Base(rDocument, NodeType_TEXT_NODE, pNode)
    {
    }

    void SAL_CALL CText::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        i_xHandler->characters(getData());
    }

  Reference< XText > SAL_CALL CText::splitText(sal_Int32 /*offset*/)
             throw (RuntimeException)
    {
        return Reference< XText >(this);
    }

    OUString SAL_CALL CText::getNodeName()throw (RuntimeException)
    {
        return OUString::createFromAscii("#text");
    }

    void SAL_CALL CText::fastSaxify( Context& io_rContext )
    {
        if (io_rContext.mxCurrentHandler.is())
        {
            try
            {
                io_rContext.mxCurrentHandler->characters( getData() );
            }
            catch( Exception& )
            {}
        }
    }

}

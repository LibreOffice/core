/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: processinginstruction.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:58:02 $
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

#include "processinginstruction.hxx"
#include <string.h>

namespace DOM
{
    CProcessingInstruction::CProcessingInstruction(const xmlNodePtr aNodePtr)
    {
        m_aNodeType = NodeType_PROCESSING_INSTRUCTION_NODE;
        init_node(aNodePtr);
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

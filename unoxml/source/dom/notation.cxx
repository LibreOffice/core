/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: notation.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:57:51 $
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

#include "notation.hxx"
#include <string.h>

namespace DOM
{
    CNotation::CNotation(const xmlNotationPtr aNotationPtr)
    {
        m_aNodeType = NodeType_NOTATION_NODE;
        m_aNotationPtr = aNotationPtr;
        init_node((xmlNodePtr)aNotationPtr);
    }

    OUString SAL_CALL CNotation::getPublicId() throw (RuntimeException)
    {
        // XXX
        return OUString();
    }

    /**
    The system identifier of this notation.
    */
    OUString SAL_CALL CNotation::getSystemId() throw (RuntimeException)
    {
        // XXX
        return OUString();
    }


    OUString SAL_CALL CNotation::getNodeName()throw (RuntimeException)
    {
       OUString aName;
        if (m_aNodePtr != NULL)
        {
            const xmlChar* xName = m_aNodePtr->name;
            aName = OUString((sal_Char*)xName, strlen((char*)xName), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }
    OUString SAL_CALL CNotation::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}

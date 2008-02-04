/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: entity.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:57:06 $
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

#include "entity.hxx"

#include <string.h>

namespace DOM
{

    CEntity::CEntity(const xmlEntityPtr aEntityPtr)
    {
        m_aNodeType = NodeType_ENTITY_NODE;
        m_aEntityPtr = aEntityPtr;
        init_node((xmlNodePtr)aEntityPtr);
    }

    /**
    For unparsed entities, the name of the notation for the entity.
    */
    OUString SAL_CALL CEntity::getNotationName() throw (RuntimeException)
    {
        // XXX
        return OUString();
    }

    /**
    The public identifier associated with the entity, if specified.
    */
    OUString SAL_CALL CEntity::getPublicId() throw (RuntimeException)
    {
        OUString aID;
        if(m_aEntityPtr != NULL)
        {
            aID = OUString((sal_Char*)m_aEntityPtr->ExternalID, strlen((char*)m_aEntityPtr->ExternalID), RTL_TEXTENCODING_UTF8);
        }
        return aID;
    }

    /**
    The system identifier associated with the entity, if specified.
    */
    OUString SAL_CALL CEntity::getSystemId() throw (RuntimeException)
    {
        OUString aID;
        if(m_aEntityPtr != NULL)
        {
            aID = OUString((sal_Char*)m_aEntityPtr->SystemID, strlen((char*)m_aEntityPtr->SystemID), RTL_TEXTENCODING_UTF8);
        }
        return aID;
    }
    OUString SAL_CALL CEntity::getNodeName()throw (RuntimeException)
    {
       OUString aName;
        if (m_aNodePtr != NULL)
        {
            const xmlChar* xName = m_aNodePtr->name;
            aName = OUString((sal_Char*)xName, strlen((char*)xName), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }
    OUString SAL_CALL CEntity::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}

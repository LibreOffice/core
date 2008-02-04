/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documenttype.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:56:30 $
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

#include "documenttype.hxx"
#include "entitiesmap.hxx"
#include "notationsmap.hxx"

#include <string.h>

namespace DOM
{

    CDocumentType::CDocumentType(const xmlDtdPtr aDtdPtr)
    {
        m_aNodeType = NodeType_DOCUMENT_TYPE_NODE;
        m_aDtdPtr = aDtdPtr;
        init_node((xmlNodePtr)aDtdPtr);
    }

    /**
    A NamedNodeMap containing the general entities, both external and
    internal, declared in the DTD.
    */
    Reference< XNamedNodeMap > SAL_CALL CDocumentType::getEntities() throw (RuntimeException)
    {
        Reference< XNamedNodeMap > aMap;
        if (m_aDtdPtr != NULL)
        {
            aMap = Reference< XNamedNodeMap >(new CEntitiesMap(this));
        }
        return aMap;
    }

    /**
    The internal subset as a string, or null if there is none.
    */
    OUString SAL_CALL CDocumentType::getInternalSubset() throw (RuntimeException)
    {
        // XXX
        return OUString();
    }

    /**
    The name of DTD; i.e., the name immediately following the DOCTYPE
    keyword.
    */
    OUString SAL_CALL CDocumentType::getName() throw (RuntimeException)
    {
        OUString aName;
        if (m_aDtdPtr != NULL)
        {
            aName = OUString((sal_Char*)m_aDtdPtr->name, strlen((char*)m_aDtdPtr->name), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }

    /**
    A NamedNodeMap containing the notations declared in the DTD.
    */
    Reference< XNamedNodeMap > SAL_CALL CDocumentType::getNotations() throw (RuntimeException)
    {
        Reference< XNamedNodeMap > aMap;
        if (m_aDtdPtr != NULL)
        {
            aMap.set(new CNotationsMap(this));
        }
        return aMap;
    }

    /**
    The public identifier of the external subset.
    */
    OUString SAL_CALL CDocumentType::getPublicId() throw (RuntimeException)
    {
        OUString aId;
        if (m_aDtdPtr != NULL)
        {
            aId = OUString((sal_Char*)m_aDtdPtr->name, strlen((char*)m_aDtdPtr->ExternalID), RTL_TEXTENCODING_UTF8);
        }
        return aId;
    }

    /**
    The system identifier of the external subset.
    */
    OUString SAL_CALL CDocumentType::getSystemId() throw (RuntimeException)
    {
        OUString aId;
        if (m_aDtdPtr != NULL)
        {
            aId = OUString((sal_Char*)m_aDtdPtr->name, strlen((char*)m_aDtdPtr->SystemID), RTL_TEXTENCODING_UTF8);
        }
        return aId;
    }
    OUString SAL_CALL CDocumentType::getNodeName()throw (RuntimeException)
    {
        return getName();
    }
    OUString SAL_CALL CDocumentType::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}

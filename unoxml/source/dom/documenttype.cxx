/*************************************************************************
 *
 *  $RCSfile: documenttype.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:31:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "documenttype.hxx"
#include "entitiesmap.hxx"
#include "notationsmap.hxx"


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
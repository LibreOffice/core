/*************************************************************************
 *
 *  $RCSfile: documentbuilder.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:21:00 $
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

#include "documentbuilder.hxx"
#include "node.hxx"
#include "document.hxx"

#include <rtl/alloc.h>
#include <rtl/memory.h>

namespace DOM
{
    extern "C" {
        //char *strdup(const char *s);
        static char* strdupfunc(const char* s)
        {
            sal_Int32 len = 0;
            while (s[len] != '\0') len++;
            char *newStr = (char*)rtl_allocateMemory(len+1);
            if (newStr != NULL)
                rtl_copyMemory(newStr, s, len+1);
            return newStr;
        }
    }
    sal_Bool CDocumentBuilder::m_bXmlInit = sal_False;
    CDocumentBuilder::CDocumentBuilder(const Reference< XMultiServiceFactory >& xFactory)
        : m_aFactory(xFactory)
    {
        // init libxml
        if (! m_bXmlInit)
        {
            xmlMemSetup(rtl_freeMemory, (xmlMallocFunc)rtl_allocateMemory,
                (xmlReallocFunc)rtl_reallocateMemory, strdupfunc);
            xmlInitParser();
            m_bXmlInit = sal_True;
        }
    }

    Reference< XInterface > CDocumentBuilder::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        // XXX
        return static_cast< XDocumentBuilder* >(new CDocumentBuilder(rSMgr));
    }

    const char* CDocumentBuilder::aImplementationName = "com.sun.star.comp.xml.dom.DocumentBuilder";
    const char* CDocumentBuilder::aSupportedServiceNames[] = {
        "com.sun.star.xml.dom.DocumentBuilder",
        NULL
    };

    OUString CDocumentBuilder::_getImplementationName()
    {
        return OUString::createFromAscii(aImplementationName);
    }
    Sequence<OUString> CDocumentBuilder::_getSupportedServiceNames()
    {
        Sequence<OUString> aSequence;
        for (int i=0; aSupportedServiceNames[i]!=NULL; i++) {
            aSequence.realloc(i+1);
            aSequence[i]=(OUString::createFromAscii(aSupportedServiceNames[i]));
        }
        return aSequence;
    }

    Sequence< OUString > SAL_CALL CDocumentBuilder::getSupportedServiceNames()
        throw (RuntimeException)
    {
        return CDocumentBuilder::_getSupportedServiceNames();
    }

    OUString SAL_CALL CDocumentBuilder::getImplementationName()
        throw (RuntimeException)
    {
        return CDocumentBuilder::_getImplementationName();
    }

    sal_Bool SAL_CALL CDocumentBuilder::supportsService(const OUString& aServiceName)
        throw (RuntimeException)
    {
        Sequence< OUString > supported = CDocumentBuilder::_getSupportedServiceNames();
        for (sal_Int32 i=0; i<supported.getLength(); i++)
        {
            if (supported[i] == aServiceName) return sal_True;
        }
        return sal_False;
    }

    Reference< XDOMImplementation > SAL_CALL CDocumentBuilder::getDOMImplementation()
        throw (RuntimeException)
    {

        return Reference< XDOMImplementation >();
    }

    sal_Bool SAL_CALL CDocumentBuilder::isNamespaceAware()
        throw (RuntimeException)
    {
        return sal_True;
    }

    sal_Bool SAL_CALL CDocumentBuilder::isValidating()
        throw (RuntimeException)
    {
        return sal_False;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::newDocument()
        throw (RuntimeException)
    {
        // create a new document
        xmlDocPtr pDocument = xmlNewDoc((const xmlChar*)"1.0");
        return Reference< XDocument >(static_cast< CDocument* >(CNode::get((xmlNodePtr)pDocument)));
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parse(const Reference< XInputStream >& is)
        throw (RuntimeException)
    {
        sal_Int32 bsize = 4096;
        char* buffer = (char*)rtl_allocateMemory(bsize);
        sal_Int32 nbytes = 0;
        sal_Int32 nread = 0;
        sal_Int32 csize = bsize;
        Sequence< sal_Int8 > chunk(csize);
        do
        {
            // read mac csize bytes
            nread = is->readBytes(chunk, csize);
            // grow buffer?
            if ((nbytes + nread) > bsize)
            {
                bsize = 2*bsize;
                char *new_buffer = (char*)rtl_reallocateMemory(buffer, bsize);
                if (new_buffer == 0)
                    throw RuntimeException();
                buffer = new_buffer;
            }
            // copy bytes to buffer
            rtl_copyMemory(buffer+nbytes, chunk.getConstArray(), nread);
            nbytes += nread;
        }
        while(nread == csize);

        // try to parse the buffer
        xmlDocPtr pDoc = xmlParseMemory(buffer, nbytes);
        // XXX error checking
        rtl_freeMemory(buffer);

        return Reference< XDocument >(static_cast< CDocument* >(CNode::get((xmlNodePtr)pDoc)));

    }

}

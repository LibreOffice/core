/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documentbuilder.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-28 10:11:22 $
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

#include "documentbuilder.hxx"
#include "node.hxx"
#include "document.hxx"

namespace DOM
{
    CDocumentBuilder::CDocumentBuilder(const Reference< XMultiServiceFactory >& xFactory)
        : m_aFactory(xFactory)
    {
        // init libxml. libxml will protect itself against multiple
        // initializations so there is no problem here if this gets
        // called multiple times.
        xmlInitParser();
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

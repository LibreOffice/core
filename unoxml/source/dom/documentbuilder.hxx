/*************************************************************************
 *
 *  $RCSfile: documentbuilder.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:31:12 $
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

#ifndef _DOCUMENTBUILDER_HXX
#define _DOCUMENTBUILDER_HXX

#include <sal/types.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "libxml/tree.h"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::io;

namespace DOM
{
    class  CDocumentBuilder
        : public ::cppu::WeakImplHelper2< XDocumentBuilder, XServiceInfo >
    {
    private:
        Reference< XMultiServiceFactory > m_aFactory;

    public:

        // ctor
        CDocumentBuilder(const Reference< XMultiServiceFactory >& xFactory);
        static sal_Bool m_bXmlInit;

        // call for factory
        static Reference< XInterface > getInstance(const Reference < XMultiServiceFactory >& xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(const Reference< XMultiServiceFactory >& rSMgr);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException);

        /**
        Obtain an instance of a DOMImplementation object.
        */
        virtual Reference< XDOMImplementation > SAL_CALL getDOMImplementation()
            throw (RuntimeException);

        /**
        Indicates whether or not this parser is configured to understand 
        namespaces.     
        */
        virtual sal_Bool SAL_CALL isNamespaceAware()
            throw (RuntimeException);

        /**       
        Indicates whether or not this parser is configured to validate XML 
        documents.
        */
        virtual sal_Bool SAL_CALL isValidating()
            throw (RuntimeException);

        /**
        Obtain a new instance of a DOM Document object to build a DOM tree 
        with.          
        */
        virtual Reference< XDocument > SAL_CALL newDocument()
            throw (RuntimeException);

        /**
        Parse the content of the given InputStream as an XML document and 
        return a new DOM Document object.     
        */
        virtual Reference< XDocument > SAL_CALL parse(const Reference< XInputStream >& is)
            throw (RuntimeException);

        /**
        Parse the content of the given URI as an XML document and return 
        a new DOM Document object.
        */
        //virtual XDocument SAL_CALL parse(const string& uri);

        /**
        Specify the EntityResolver to be used to resolve entities present
        in the XML document to be parsed.
        */
        // virtual void SAL_CALL setEntityResolver(const XEntityResolver& er);

        /**
        Specify the ErrorHandler to be used to report errors present in
        the XML document to be parsed.
        */
        //virtual void SAL_CALL setErrorHandler(const XErrorHandler& eh);
    };
}

#endif